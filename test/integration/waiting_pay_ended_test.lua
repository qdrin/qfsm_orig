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
  local mstate = helper.build_machine_state("active", "waitingPayment", "commercial")
  local price = helper.prices.offer1.offer1PriceActive
  -- local new_price = table.deepcopy(price)
  -- price.period = 1
  local t0 = os.time()
  local t1 = helper.end_of_price(price)
  local data = {
    machine = mstate.machine,
    product = {
      status = "ACTIVE",
      activeEndDate = t1,
      trialEndDate =t0,
      tarificationPeriod = 5,
      price = {price},
    },
    tasks = {}
  }
  local state = helper.build_machine_state("suspending", "notPaid", "commercial")
  local expected = {
    machine = state.machine,
    product = {
      status = "ACTIVE",
      activeEndDate = function(real)
        local dt = real - os.time()
        return dt >= -10 and dt < 10
      end,
      trialEndDate =t0,
      tarificationPeriod = 4,
      price = {price},
    },
    tasks = {
      {
        wakeAtFunction = "SUSPEND",
        wakeAt = function(real)
          local dt = real - os.time()
          return dt >= -10 and dt < 10
        end,
      }
    },
    delete_tasks = {
      {wakeAtFunction = "send_event", extraParams = {eventType = "trial_ended"}},
      {wakeAtFunction = "send_event", extraParams = {eventType = "commercial_ended"}},
      {wakeAtFunction = "send_event", extraParams = {eventType = "waiting_pay_ended"}},
    },
  }
  local res, err = helper.send_event("waiting_pay_ended", data, "p1")
  print("result: ", json.encode(res))
  t.assert(res, err)
  helper.assert_result(res, expected)
end