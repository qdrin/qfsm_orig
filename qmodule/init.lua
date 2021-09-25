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
  suspend=function(id)
    local productId = machines[id].productId
    log.info("'suspend' callback start. id=%s, productId=%s", id, productId)
  end,
  prolong=function(id)
    log.info("'prolong' callback start. id=%s", id)
  end,
}

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

local function set_callbacks(custom_callbacks)
  callbacks = custom_callbacks or callbacks
  log.info("qmodule.set_callbacks start")
  local res, m = qfsmlib.set_callbacks(callbacks)
  if res < 0 then return nil, m end
  log.info("qmodule.set_callbacks finished: %s", res)
  return res
end

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

--
-- Exported functions
--
return {
    set_callbacks = set_callbacks,
    new = new,
    stop = stop,
    qfsmlib = qfsmlib,
    callbacks = callbacks,
}
