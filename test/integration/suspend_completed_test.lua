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

g.test_normal = function()
  local mstate = helper.build_machine_state("suspending", "notPaid", "commercial")
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
  local state = helper.build_machine_state("suspended", "notPaid", "commercial")
  local expected = {
    machine = state.machine,
    product = {
      status = "SUSPENDED",
      activeEndDate = t0,
      trialEndDate =t0,
      tarificationPeriod = 4,
      price = {price},
    },
    tasks = {
      {
        wakeAtFunction = "send_event",
        wakeAt = function(real)
          local dt = real - (os.time() + helper.config.suspend_duration)
          return dt >= -10 and dt < 10
        end,
        extraParams = {
          eventType = "suspend_ended",
        }
      }
    },
    delete_tasks = {},
  }
  local res, err = helper.send_event("suspend_completed", data, "p1")
  print("result: ", json.encode(res))
  t.assert(res, err)
  helper.assert_result(res, expected)
end

g.test_past_event_date = function()
  local mstate = helper.build_machine_state("suspending", "notPaid", "commercial")
  local price = helper.prices.offer1.offer1PriceActive
  -- local new_price = table.deepcopy(price)
  price.period = 1
  local t0 = os.time()
  local t1 = t0 - 86400*10
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
  local state = helper.build_machine_state("suspended", "notPaid", "commercial")
  local expected = {
    machine = state.machine,
    product = {
      status = "SUSPENDED",
      activeEndDate = t0,
      trialEndDate =t0,
      tarificationPeriod = 4,
      price = {price},
    },
    tasks = {
      {
        wakeAtFunction = "send_event",
        wakeAt = function(real)
          local dt = real - (os.time() + helper.config.suspend_duration)
          return dt >= -10 and dt < 10
        end,
        extraParams = {
          eventType = "suspend_ended",
        }
      }
    },
    delete_tasks = {},
  }
  local event = {
    eventType = "suspend_completed",
    eventDate = t1,
  }
  local res, err = helper.send_event("suspend_completed", data, "p1")
  print("result: ", json.encode(res))
  t.assert(res, err)
  helper.assert_result(res, expected)
end