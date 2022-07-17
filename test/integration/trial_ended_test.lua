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

g.test_duration_ended = function()
  local mstate = helper.build_machine_state("activeTrial", "paid", "trial")
  local price = helper.prices.offer1.offer1PriceTrial
  price.period = 2
  local t0 = os.time()
  local data = {
    machine = mstate.machine,
    product = {
      status = "ACTIVE_TRIAL",
      activeEndDate = t0,
      trialEndDate =t0,
      price = {price},
    },
    tasks = {}
  }
  local state = helper.build_machine_state("activeTrial", "paid", "priceChanging")
  local expected = {
    machine = state.machine,
    product = {
      status = "ACTIVE_TRIAL",
      activeEndDate = t0,
      trialEndDate =t0,
    },
    tasks = {
      {
        wakeAtFunction = "send_event",
        wakeAt = function(real)
          local dt = real - os.time()
          return dt >= -10 and dt < 10
        end,
        extraParams = {eventType="change_price"},
      }
    },
    delete_tasks = {},
  }
  local res, err = helper.send_event("trial_ended", data, "p1")
  print("result: ", json.encode(res))
  t.assert(res, err)
  helper.assert_result(res, expected)
end

g.test_duration_not_ended = function()
  local mstate = helper.build_machine_state("activeTrial", "paid", "trial")
  local price = helper.prices.offer2.offer2PriceTrial
  local new_price = table.deepcopy(price)
  price.period = 1
  new_price.period = 2
  local t0 = os.time()
  local t1 = helper.end_of_price(price)
  local data = {
    machine = mstate.machine,
    product = {
      status = "ACTIVE_TRIAL",
      activeEndDate = t0,
      tarificationPeriod = 1,
      trialEndDate =t0,
      price = {price},
    },
    tasks = {}
  }
  local state = helper.build_machine_state("activeTrial", "paid", "trial")
  local expected = {
    machine = state.machine,
    product = {
      status = "ACTIVE_TRIAL",
      activeEndDate = t1,
      trialEndDate =t1,
      tarificationPeriod = 2,
      price = {new_price},
    },
    tasks = {
      {
        wakeAtFunction = "send_event",
        extraParams = {
          eventType = "trial_ended",
        },
        wakeAt = function(real)
          local dt = real - (t1 - helper.config.price_ended_before_time)
          return dt >= -10 and dt < 10
        end,
      }
    },
    delete_tasks = {},
  }
  local res, err = helper.send_event("trial_ended", data, "p1")
  print("result: ", json.encode(res))
  t.assert(res, err)
  helper.assert_result(res, expected)
end