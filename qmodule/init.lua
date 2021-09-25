--------------------------------------------------------------------------------
--- Example of a Lua module for Tarantool
--------------------------------------------------------------------------------

--
-- Dependencies
--

local log = require('log') -- some other Tarantool module

-- C library
local qfsmlib = require('qmodule.qfsmlib')
-- Now you can use exported CPP functions from 'qmodule/qfsmlib.c' submodule in your code
local machines = {}
local callbacks = {
  suspend=function(productId)
    log.info("callback 'suspend' start. productId=%s", productId)
  end,
  prolong=function(productId)
    log.info("callback 'prolong' start. productId=%s", productId)
  end,
}

local function callback_func(id, cb_name)
  local productId = machines[id].productId
  callbacks[cb_name](productId)
end

local machine_mt = {
  __index = {
    send_event = function(self, event_type)
      local status, res = self.conn:is_running()
      if not res then return nil, string.format("machine %s is not running", self.conn) end
      local status, res = self.conn:send_event(self.conn, event_type)
      if status < 0 then return nil, res end
      return res
    end,
    close = function(self)
      local status, res = self.conn:is_running()
      if not res then return nil, string.format("machine %s is not running", self.conn) end
      local res, error = self.conn:close()
      if res < 0 then return nil, error end
      machines[self.conn] = nil
      return res
    end,
    init = function(self, productId, state)
      self.productId = productId
      local status, res = self.conn:is_running()
      if not res then return nil, string.format("machine %s is not running", self.conn) end
      status, res = self.conn:init(state)
      if status < 0 then return nil, res end
      return res
    end,
    get = function(self)
      local status, res = self.conn:is_running()
      if not res then return nil, string.format("machine %s is not running", self.conn) end
      status, res = self.conn:get()
      if status < 0 then return nil, res end
      return res
    end,
    is_running = function(self)
      local status, res = self.conn:is_running()
      if status < 0 then return nil, res end
      return res
    end,
  }
}

local function new(custom_cbfunc)
  log.info("new called")
  local res, m = qfsmlib.new()
  log.info("new result=%s", res)
  if res < 0 then return nil, m end
  local machine = setmetatable({
    conn = m,
}, machine_mt)
  machines[m] = machine
  return machine
end

local function stop()
  log.info("stop called")
  local res, m = qfsmlib.stop()
  log.info("stop result: %s", res)
  if not res then return nil, m end
  return true
end

local function init(conf)
  callbacks = conf.callbacks or callbacks
end

qfsmlib.init({callback_func = callback_func})
return {
  init = init,
  new = new,
  stop = stop,
  qfsmlib = qfsmlib,
  callbacks = callbacks,
}
