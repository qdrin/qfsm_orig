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
    send_event = function(self)
      local status, res = self.conn:send_event()
      if status < 0 then return nil, res end
      return res
    end,
    close = function(self)
      local res, error = self.conn:close()
      if res < 0 then return nil, error end
      return res
    end,
    init = function(self)
      log.info("machine.init call function %s", self.conn.init)
      local res, error = self.conn:init()
      if res < 0 then return nil, error end
      return res
    end,
    get = function(self)
      log.info("machine.init call function %s", self.conn.init)
      local status, res = self.conn:get()
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
  log.warn(m)
  local machine = setmetatable({
    conn = m,
}, machine_mt)
  log.info(machine)
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
