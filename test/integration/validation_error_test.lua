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

g.test_bad_init_data = function()
  local mstate = helper.build_machine_state("pendingDisconnect", "paymentFinal", "priceFinal")
  local data = {
    machine = mstate.machine,
    status = "PENDING_DISCONNECT",  -- state is at wrong place
    product = {
      status = "PENDING_DISCONNECT",
    },
    tasks = {}
  }
  local t0 = os.time()
  local state = helper.build_machine_state("disconnection", "paymentFinal", "priceFinal")

  local res, err, err_code = helper.send_event("disconnect", data, "p1")
  print("err_code", err_code, "error", err)
  t.assert_not(res)
  t.assert_equals(err_code, -3)
end

g.test_bad_event_data = function()
  local mstate = helper.build_machine_state("pendingDisconnect", "paymentFinal", "priceFinal")
  local data = {
    machine = mstate.machine,
    product = {
      status = "PENDING_DISCONNECT",
    },
    tasks = {}
  }
  local t0 = os.time()
  local event = {
    eventType="disconnect",
    eventWrongData = "some_fake_data",
  }
  local state = helper.build_machine_state("disconnection", "paymentFinal", "priceFinal")

  local res, err, err_code = helper.send_event(event, data, "p1")
  print("err_code", err_code, "error", err)
  t.assert_not(res)
  t.assert_equals(err_code, -3)
end