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

g.test_trial_activation_completed = function()
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
  local state = helper.build_machine_state("activeTrial", "paid", "trial")
  local expected = {
    machine = state.machine,
    product = {
      status = "ACTIVE_TRIAL",
      activeEndDate = function(real)
        local dt = real - helper.end_of_price(price)
        return dt >= -10 and dt < 10
      end,
      trialEndDate = function(real)
        local dt = real - helper.end_of_price(price)
        return dt >= -10 and dt < 10
      end,
    },
    tasks = {
      {
        wakeAtFunction = "send_event",
        extraParams = {eventType = "trial_ended"},
        wakeAt = function(real)
          local dt = real - (helper.end_of_price(price) - helper.config.price_ended_before_time)
          return dt >= -10 and dt < 10
        end,
      }
    },
    delete_tasks = {},
  }
  local res, err = helper.send_event("trial_activation_completed", data, "p1")
  print("result: ", json.encode(res))
  t.assert(res, err)
  helper.assert_result(res, expected)
end

g.test_activation_completed = function()
  local mstate = helper.build_machine_state("pendingActivate")
  local price = helper.prices.offer1.offer1PriceActive
  print("price", json.encode(price))
  local data = {
    machine = mstate.machine,
    product = {
      status = "PENDING_ACTIVATE",
      price = {price},
    },
    tasks = {}
  }
  local state = helper.build_machine_state("active", "waitingPayment", "commercial")
  local expected = {
    machine = state.machine,
    product = {
      status = "ACTIVE",
      activeEndDate = function(real)
        local dt = real - helper.end_of_price(price)
        return dt >= -10 and dt < 10
      end,
    },
    tasks = {
      {
        wakeAtFunction = "send_event",
        extraParams = {eventType = "commercial_ended"},
        wakeAt = function(real)
          local dt = real - (helper.end_of_price(price) - helper.config.price_ended_before_time)
          return dt >= -10 and dt < 10
        end,
      },
      {
        wakeAtFunction = "send_event",
        extraParams = {eventType = "waiting_pay_ended"},
        wakeAt = function(real)
          local dt = real - (os.time() + helper.config.payment_waiting_time)
          return dt >= -10 and dt < 10
        end,
      }
    },
    delete_tasks = {},
  }
  local res, err = helper.send_event("activation_completed", data, "p1")
  print("result: ", json.encode(res))
  t.assert(res, err)
  helper.assert_result(res, expected)
end

g.test_trial_activation_completed_past_event_date = function()
  local mstate = helper.build_machine_state("pendingActivate")
  local price = helper.prices.offer1.offer1PriceTrial
  local t0 = os.time() - 86400 * 10
  local t1 = helper.end_of_price(price, t0)
  local data = {
    machine = mstate.machine,
    product = {
      status = "PENDING_ACTIVATE",
      price = {price},
    },
    tasks = {}
  }
  local state = helper.build_machine_state("activeTrial", "paid", "trial")
  local expected = {
    machine = state.machine,
    product = {
      status = "ACTIVE_TRIAL",
      activeEndDate = function(real)
        local dt = real - t1
        return dt >= -10 and dt < 10
      end,
      trialEndDate = function(real)
        local dt = real - t1
        return dt >= -10 and dt < 10
      end,
    },
    tasks = {
      {
        wakeAtFunction = "send_event",
        extraParams = {eventType = "trial_ended"},
        wakeAt = function(real)
          local dt = real - (t1 - helper.config.price_ended_before_time)
          return dt >= -10 and dt < 10
        end,
      }
    },
    delete_tasks = {},
  }
  local event = {
    eventType = "trial_activation_completed",
    eventDate = t0,
  }
  local res, err = helper.send_event(event, data, "p1")
  print("result: ", json.encode(res))
  t.assert(res, err)
  helper.assert_result(res, expected)
end

g.test_activation_completed_past_event_date = function()
  local mstate = helper.build_machine_state("pendingActivate")
  local price = helper.prices.offer1.offer1PriceActive
  local t0 = os.time() - 86400*10
  local t1 = helper.end_of_price(price, t0)
  print("price", json.encode(price))
  local data = {
    machine = mstate.machine,
    product = {
      status = "PENDING_ACTIVATE",
      price = {price},
    },
    tasks = {}
  }
  local state = helper.build_machine_state("active", "waitingPayment", "commercial")
  local expected = {
    machine = state.machine,
    product = {
      status = "ACTIVE",
      activeEndDate = function(real)
        local dt = real - t1
        return dt >= -10 and dt < 10
      end,
    },
    tasks = {
      {
        wakeAtFunction = "send_event",
        extraParams = {eventType = "commercial_ended"},
        wakeAt = function(real)
          local dt = real - (t1 - helper.config.price_ended_before_time)
          return dt >= -10 and dt < 10
        end,
      },
      {
        wakeAtFunction = "send_event",
        extraParams = {eventType = "waiting_pay_ended"},
        wakeAt = function(real)
          local dt = real - (os.time() + helper.config.payment_waiting_time)
          return dt >= -10 and dt < 10
        end,
      }
    },
    delete_tasks = {},
  }
  local event = {
    eventType = "activation_completed",
    eventDate = t0,
  }
  local res, err = helper.send_event(event, data, "p1")
  print("result: ", json.encode(res))
  t.assert(res, err)
  helper.assert_result(res, expected)
end