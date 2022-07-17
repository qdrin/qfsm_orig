local t = require('luatest')
local g = t.group()
local json = require('json')
local uuid = require('uuid')
local fiber = require('fiber')
local qfsm = require('qfsm')
local clock = require('clock')

local helper = require('test.helper')

g.before_all(function()
  qfsm.init()
  print('qfsm.init finished')
end)

g.before_each(function()

end)

g.test_from_pendingDisconnect = function()
  local mstate = helper.build_machine_state("pendingDisconnect", "paymentFinal", "priceFinal")
  local data = {
    machine = mstate.machine,
    product = {
      status = "PENDING_DISCONNECT",
    },
    tasks = {}
  }
  local t0 = os.time()
  local state = helper.build_machine_state("disconnection", "paymentFinal", "priceFinal")
  local expected = {
    machine = state.machine,
    product = {
      status = "PENDING_DISCONNECT",
    },
    tasks = {
      {
        wakeAtFunction = "DISCONNECT",
        wakeAt = function(real)
          local dt = real - os.time()
          return dt >= 0 and dt < 1
        end,
      },
    },
  }

  local res, err = helper.send_event("disconnect", data, "p1")
  print("result", json.encode(res), "error", err)
  t.assert(res)
  helper.assert_result(res, expected)
  -- t.assert(false)
end

g.test_from_wrong_state = function()
  local mstate = helper.build_machine_state("disconnection", "paymentFinal", "priceFinal")
  local data = {
    machine = mstate.machine,
    product = {
      status = "PENDING_DISCONNECT",
    },
    tasks = {},
    -- products = {},
  }

  local res, err, err_code = helper.send_event("disconnect", data, "p1")
  print("err_code", err_code, "error", err)
  t.assert_not(res)
  t.assert_equals(err_code, -2)
end
