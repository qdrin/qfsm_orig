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

--
-- Internal functions
--

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
      return res
    end,
    init = function(self, state)
      local status, res = self.conn:is_running()
      if not res then return nil, string.format("machine %s is not running", self.conn) end
      local res, new_state = self.conn:init(state)
      if res < 0 then return nil, new_state end
      return new_state
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

local function new()
  log.info("new called")
  local res, m = qfsmlib.new()
  log.info("new result=%s", res)
  if res < 0 then return nil, m end
  local machine = setmetatable({
    conn = m,
}, machine_mt)
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
    new = new,
    stop = stop,
    qfsmlib = qfsmlib,
}
