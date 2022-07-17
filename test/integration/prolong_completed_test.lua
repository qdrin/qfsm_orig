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

g.test_to_active = function()
  local mstate = helper.build_machine_state("prolongation", "paid", "commercial")
  local price = helper.prices.offer1.offer1PriceActive
  -- local new_price = table.deepcopy(price)
  price.period = 1
  local t0 = os.time()
  local t1 = helper.end_of_price(price)
  local data = {
    machine = mstate.machine,
    product = {
      status = "ACTIVE",
      activeEndDate = t0,
      trialEndDate =t0,
      tarificationPeriod = 4,
      price = {price},
    },
    tasks = {}
  }
  local state = helper.build_machine_state("active", "paid", "commercial")
  local expected = {
    machine = state.machine,
    product = {
      status = "ACTIVE",
      activeEndDate = t0,
      trialEndDate =t0,
      tarificationPeriod = 4,
      price = {price},
    },
    tasks = {},
    delete_tasks = {},
  }
  local res, err = helper.send_event("prolong_completed", data, "p1")
  print("result: ", json.encode(res))
  t.assert(res, err)
  helper.assert_result(res, expected)
end

g.test_to_active_past_event = function()
  local mstate = helper.build_machine_state("prolongation", "paid", "commercial")
  local price = helper.prices.offer1.offer1PriceActive
  -- local new_price = table.deepcopy(price)
  price.period = 1
  local t0 = os.time()
  local t0past = t0 - 86400*10
  local t1 = helper.end_of_price(price)
  local t1past = helper.end_of_price(price, t0past)
  local data = {
    machine = mstate.machine,
    product = {
      status = "ACTIVE",
      activeEndDate = t1,
      trialEndDate =t0,
      tarificationPeriod = 4,
      price = {price},
    },
    tasks = {}
  }
  local state = helper.build_machine_state("active", "paid", "commercial")
  local expected = {
    machine = state.machine,
    product = {
      status = "ACTIVE",
      activeEndDate = t1,
      trialEndDate =t0,
      tarificationPeriod = 4,
      price = {price},
    },
    tasks = {},
    delete_tasks = {},
  }
  local event = {
    eventType = "prolong_completed",
    eventDate = t0past,
  }
  local res, err = helper.send_event(event, data, "p1")
  print("result: ", json.encode(res))
  t.assert(res, err)
  helper.assert_result(res, expected)
end

g.test_to_trial = function()
  local mstate = helper.build_machine_state("prolongation", "paid", "commercial")
  local price = helper.prices.offer1.offer1PriceTrial
  -- local new_price = table.deepcopy(price)
  price.period = 2
  local t0 = os.time()
  local t1 = helper.end_of_price(price)
  local data = {
    machine = mstate.machine,
    product = {
      status = "ACTIVE_TRIAL",
      activeEndDate = t0,
      trialEndDate =t0,
      tarificationPeriod = 4,
      price = {price},
    },
    tasks = {}
  }
  local state = helper.build_machine_state("activeTrial", "paid", "commercial")
  local expected = {
    machine = state.machine,
    product = {
      status = "ACTIVE_TRIAL",
      activeEndDate = t0,
      trialEndDate =t0,
      tarificationPeriod = 4,
      price = {price},
    },
    tasks = {},
    delete_tasks = {},
  }
  local res, err = helper.send_event("prolong_completed", data, "p1")
  print("result: ", json.encode(res))
  t.assert(res, err)
  helper.assert_result(res, expected)
end

g.test_to_trial_past_event = function()
  local mstate = helper.build_machine_state("prolongation", "paid", "commercial")
  local price = helper.prices.offer1.offer1PriceTrial
  -- local new_price = table.deepcopy(price)
  price.period = 1
  local t0 = os.time()
  local t0past = t0 - 86400*10
  local t1 = helper.end_of_price(price)
  local t1past = helper.end_of_price(price, t0past)
  local data = {
    machine = mstate.machine,
    product = {
      status = "ACTIVE_TRIAL",
      activeEndDate = t1,
      trialEndDate =t0,
      tarificationPeriod = 4,
      price = {price},
    },
    tasks = {}
  }
  local state = helper.build_machine_state("activeTrial", "paid", "commercial")
  local expected = {
    machine = state.machine,
    product = {
      status = "ACTIVE_TRIAL",
      activeEndDate = t1,
      trialEndDate =t0,
      tarificationPeriod = 4,
      price = {price},
    },
    tasks = {},
    delete_tasks = {},
  }
  local event = {
    eventType = "prolong_completed",
    eventDate = t0past,
  }
  local res, err = helper.send_event(event, data, "p1")
  print("result: ", json.encode(res))
  t.assert(res, err)
  helper.assert_result(res, expected)
end