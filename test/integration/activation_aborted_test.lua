local t = require('luatest')
local g = t.group()
local json = require('json')
local uuid = require('uuid')
local fiber = require('fiber')
local qfsm = require('qfsm')
local clock = require('clock')

local helper = require('test.helper')

g.before_all(function()
  helper.read_config()
  helper.read_prices()
  qfsm.init(helper.config)
  print('qfsm.init finished')
end)

g.before_each(function()
end)

g.test_trial_price = function()
  local mstate = helper.build_machine_state("pendingActivate")
  local price = helper.prices.offer1.offer1PriceTrial
  local data = {
    machine = mstate.machine,
    product = {
      status = "PENDING_ACTIVATE",
      price = {price},
    },
    tasks = {}
  }
  local state = helper.build_machine_state("aborted")
  local expected = {
    machine = state.machine,
    product = {
      status = "ABORTED",
    },
    tasks = {},
    delete_tasks = {},
  }
  print("data", json.encode(data))
  local res, err = helper.send_event("activation_aborted", data, "p1")
  print("result: ", json.encode(res))
  t.assert(res, err)
  helper.assert_result(res, expected)
end

g.test_commercial_price = function()
  local mstate = helper.build_machine_state("pendingActivate")
  local price = helper.prices.offer1.offer1PriceActive
  local t0 = os.time()
  local t1 = helper.end_of_price(price)
  local data = {
    machine = mstate.machine,
    product = {
      status = "PENDING_ACTIVATE",
      price = {price},
    },
    tasks = {}
  }
  local state = helper.build_machine_state("aborted")
  local expected = {
    machine = state.machine,
    product = {
      status = "ABORTED",
    },
    tasks = {},
    delete_tasks = {},
  }
  print("data", json.encode(data))
  local res, err = helper.send_event("activation_aborted", data, "p1")
  print("result: ", json.encode(res))
  t.assert(res, err)
  helper.assert_result(res, expected)
end

g.test_empty_price = function()
  local mstate = helper.build_machine_state("pendingActivate")
  local t0 = os.time()
  local data = {
    machine = mstate.machine,
    product = {
      status = "PENDING_ACTIVATE",
      price = {},
    },
    tasks = {}
  }
  local state = helper.build_machine_state("aborted")
  local expected = {
    machine = state.machine,
    product = {
      status = "ABORTED",
    },
    tasks = {},
    delete_tasks = {},
  }
  print("data", json.encode(data))
  local res, err = helper.send_event("activation_aborted", data, "p1")
  print("result: ", json.encode(res))
  t.assert(res, err)
  helper.assert_result(res, expected)
end

g.test_null_price = function()
  local mstate = helper.build_machine_state("pendingActivate")
  local t0 = os.time()
  local data = {
    machine = mstate.machine,
    product = {
      status = "PENDING_ACTIVATE",
    },
    tasks = {}
  }
  local state = helper.build_machine_state("aborted")
  local expected = {
    machine = state.machine,
    product = {
      status = "ABORTED",
    },
    tasks = {},
    delete_tasks = {},
  }
  print("data", json.encode(data))
  local res, err = helper.send_event("activation_aborted", data, "p1")
  print("result: ", json.encode(res))
  t.assert(res, err)
  helper.assert_result(res, expected)
end

