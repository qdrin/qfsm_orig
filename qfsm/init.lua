--------------------------------------------------------------------------------
--- Lua part of HFSM module for Tarantool
--------------------------------------------------------------------------------
local log = require('log')
local icu_date = require('icu-date')
-- local error_model = require('app.modules.common.error-model')
local json = require('json')
local checks = require('checks')
local validators = require('qfsm.validators')
local check_args = validators.check_args
local properties_model = validators.properties_model
local event_model = validators.event_model

-- CPP Qt library
-- import library changes locale and floating point to floating comma
local locale = os.setlocale()
local qfsmlib = require('qfsm.qfsmlib')
os.setlocale(locale)


local result_codes = {
  [1] = "OK",
  [-1] = "QfsmlibError",
  [-2] = "IllegalEventError",
  [-3] = "IllegalParametersError",
  [-4] = "PoolIsEmptyError",
}
-- Now you can use exported CPP functions from 'qfsm/qfsmlib.cpp' submodule in your code
local machines = {}

local config = {
  prices = {},
  max_machines = 3,
  ms_timeout = 10,
  payment_waiting_time = 7200,
  price_ended_before_time = 3600,
  suspend_duration = 86400*180,
  immediate_task_time = 300,
}

local qfsmlib_error = function(err, status)
  err = err or "Unknown error"
  status = status or -1
  return nil, err, status
end

local machine_mt = {
  __index = {
    send_event = function(self, event)
      local status, res = check_args({'table', event_model}, self, event)
      if not status then return status, res, -3 end
      status, res = self.conn:is_running()
      if not res then return qfsmlib_error(string.format("machine %s is not running", self.conn+1), -1) end
      status, res = self.conn:send_event(json.encode(event))
      if not status or status < 0 then
        log.error("qfsm machine[%s]:send_event failed:  %s", self.conn+1, res)
        return qfsmlib_error(res, status)
      end
      -- The next string is useless when we run the self:get()
      -- res = json.decode(res)
      -- This fixes unaccomplished machine states
      res = self:get()

      for _, t in pairs(res.tasks or {}) do t.productId = self.productId end
      for _, t in pairs(res.deleteTasks or {}) do t.productId = self.productId end
      res.productId = self.productId
      return res
    end,
    release = function(self)
      self.busy = false
      self.productId = nil
      self.tasks = {}
      self.delete_tasks = {}
      self.properties = nil
    end,
    close = function(self)
      local status, res = self.conn:is_running()
      if not res then return qfsmlib_error(string.format("machine %s is not running", self.conn+1), -1) end
      local res, error = self.conn:close()
      if res < 0 then return qfsmlib_error(err) end
      self.conn = -1
      table.remove(machines, self.conn + 1)
      return res
    end,
    -- opts should contain product attributes in plain model
    -- self is representation of product
    -- All the product properties can be the self properties, e.g. productId, label, characteristic, price etc.
    -- state represents the qfsmlib model: product properties -> 'properties' attribute, machine -> 'machine'
    -- config parts should be placed to 'config' attribute
    init = function(self, productId, opts)
      local status, res = check_args({'table', '?string', properties_model}, self, productId, opts)
      if not status then return status, res, -3 end
      local state = {
        machine = opts.machine,
        product = table.deepcopy(opts.product),
        properties = table.deepcopy(opts.properties),
        config = {
          payment_waiting_time = config.payment_waiting_time,
          suspend_duration = config.suspend_duration,
          price_ended_before_time = config.price_ended_before_time,
          immediate_task_time = config.immediate_task_time,
        },
        tasks = opts.tasks,
        delete_tasks = opts.delete_tasks,
      }
      self.productId = productId

      status, res = self.conn:is_running()
      if not res then return qfsmlib_error(string.format("machine %s is not running", self.conn+1), -1) end
      status, res = self.conn:init(json.encode(state))
      if status < 0 then return qfsmlib_error(res, status) end
      res = json.decode(res)
      self.machine = res.machine
      -- for n, v in pairs(res.properties) do self[n] = v end
      log.verbose("qfsm machine[%s].init finished: %s", self.conn+1, json.encode(res))
      return res
    end,
    get = function(self)
      local status, res = self.conn:is_running()
      if not res then return qfsmlib_error(string.format("machine %s is not running", self.conn+1), -1) end
      status, res = self.conn:get()
      if status < 0 then return qfsmlib_error(res, status) end
      res = json.decode(res)
      self.machine = res.machine
      self.properties = res.properties
      return res
    end,
    is_running = function(self)
      local status, res = self.conn:is_running()
      if status < 0 then return qfsmlib_error(res, status) end
      return res
    end,
  }
}

local function new()
  local res, m, err = qfsmlib.new()
  if res < 0 then qfsmlib_error(err) end
  local machine = setmetatable({
    conn = m,
    tasks = {},
    busy = false,
  }, machine_mt)
  log.info("machine created. conn_id %s", machine.conn)
  machine:get()
  return machine
end

local function stop()
  log.info("stop called")
  for i = #machines, 1, -1 do machines[i]:close() end
  local res, m = qfsmlib.stop()
  log.info("stop result: %s", res)
  if not res then qfsmlib_error(m) end
  return true
end

local function lease()
  for _, m in pairs(machines) do
    if not m.busy then
      m.busy = true
      m.tasks = {}
      m.delete_tasks = {}
      return m
    end
  end
  log.error("qfsm: No free machines in pool")
  return qfsmlib_error(-4, "qfsm: No free machines in pool")
end

local function init(conf)
  log.info("qfsm.init started")
  local machine
  conf = conf or {}
  config.max_machines = conf.max_machines or config.max_machines
  config.ms_timeout = conf.ms_timeout or config.ms_timeout
  config.payment_waiting_time = conf.payment_waiting_time or config.payment_waiting_time
  config.price_ended_before_time = conf.price_ended_before_time or config.price_ended_before_time
  config.suspend_duration = conf.suspend_duration or config.suspend_duration
  config.immediate_task_time = conf.immediate_task_time or config.immediate_task_time
  qfsmlib.init({
    max_machines = config.max_machines,
    ms_timeout = config.ms_timeout,
  })
  if #machines == config.max_machines then return end
  for i = #machines, 1, -1 do machines[i]:close() end
  for i = 1, config.max_machines do
    machine = new()
    machines[machine.conn + 1] = machine
  end
end

return {
  init = init,
  lease = lease,
  stop = stop,
  qfsmlib = qfsmlib,
  machines = machines,
  result_codes = result_codes,
}
