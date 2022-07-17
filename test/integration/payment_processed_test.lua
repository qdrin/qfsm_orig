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

g.test_from_waitingpayment = function()
  local mstate = helper.build_machine_state("active", "waitingPayment", "commercial")
  local price = helper.prices.offer1.offer1PriceActive
  -- local new_price = table.deepcopy(price)
  price.period = 1
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
  local state = helper.build_machine_state("prolongation", "paid", "commercial")
  local expected = {
    machine = state.machine,
    product = {
      status = "ACTIVE",
      activeEndDate = t1,
      trialEndDate =t0,
      tarificationPeriod = 5,
      price = {price},
    },
    tasks = {
      {
        wakeAtFunction = "ACTIVATE",
        wakeAt = function(real)
          local dt = real - os.time()
          return dt >= -10 and dt < 10
        end,
        extraParams = {
          price = {price},
        }
      }
    },
    delete_tasks = {
      {wakeAtFunction = "send_event", extraParams = {eventType = "waiting_pay_ended"}},
    },
  }
  local res, err = helper.send_event("payment_processed", data, "p1")
  print("result: ", json.encode(res))
  t.assert(res, err)
  helper.assert_result(res, expected)
end

-- confirm that past payment_processed event doesn't affect trialEndDate
g.test_from_waitingpayment_past_event = function()
  local mstate = helper.build_machine_state("active", "waitingPayment", "commercial")
  local price = helper.prices.offer1.offer1PriceActive
  -- local new_price = table.deepcopy(price)
  price.period = 1
  local t0 = os.time()  -- helper.config.price_ended_before_time
  local t1 = helper.end_of_price(price, t0)
  local tevent = t0 - 7000
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
  local state = helper.build_machine_state("prolongation", "paid", "commercial")
  local expected = {
    machine = state.machine,
    product = {
      status = "ACTIVE",
      activeEndDate = t1,
      trialEndDate = t0,
      tarificationPeriod = 5,
      price = {price},
    },
    tasks = {
      {
        wakeAtFunction = "ACTIVATE",
        wakeAt = function(real)
          local dt = real - os.time()
          return dt >= -10 and dt < 10
        end,
        extraParams = {
          price = {price},
        }
      }
    },
    delete_tasks = {
      {wakeAtFunction = "send_event", extraParams = {eventType = "waiting_pay_ended"}},
    },
  }
  local event = {
    eventType = "payment_processed",
    eventDate = tevent,
  }
  local res, err = helper.send_event(event, data, "p1")
  print("result: ", json.encode(res))
  t.assert(res, err)
  helper.assert_result(res, expected)
end

g.test_from_waitingpayment_first_tarification = function()
  local mstate = helper.build_machine_state("active", "waitingPayment", "commercial")
  local price = helper.prices.offer1.offer1PriceActive
  price.period = 1
  local t0 = os.time()
  local t1 = helper.end_of_price(price)
  local data = {
    machine = mstate.machine,
    product = {
      status = "ACTIVE",
      activeEndDate = t1,
      trialEndDate =t0,
      tarificationPeriod = 1,
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
      tarificationPeriod = 1,
      price = {price},
    },
    tasks = {},
    delete_tasks = {
      {wakeAtFunction = "send_event", extraParams = {eventType = "waiting_pay_ended"}},
    },
  }
  local res, err = helper.send_event("payment_processed", data, "p1")
  print("result: ", json.encode(res))
  t.assert(res, err)
  helper.assert_result(res, expected)
end

g.test_from_waitingpayment_first_tarification_past_event = function()
  local mstate = helper.build_machine_state("active", "waitingPayment", "commercial")
  local price = helper.prices.offer1.offer1PriceActive
  price.period = 1
  local t0 = os.time()
  local t1 = helper.end_of_price(price)
  local data = {
    machine = mstate.machine,
    product = {
      status = "ACTIVE",
      activeEndDate = t1,
      trialEndDate =t0,
      tarificationPeriod = 1,
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
      tarificationPeriod = 1,
      price = {price},
    },
    tasks = {},
    delete_tasks = {
      {wakeAtFunction = "send_event", extraParams = {eventType = "waiting_pay_ended"}},
    },
  }
  local event = {
    eventType = "payment_processed",
    eventDate = os.time() - 7000,
  }
  local res, err = helper.send_event(event, data, "p1")
  print("result: ", json.encode(res))
  t.assert(res, err)
  helper.assert_result(res, expected)
end

g.test_from_suspended = function()
  local mstate = helper.build_machine_state("suspended", "notPaid", "commercial")
  local price = table.deepcopy(helper.prices.offer1.offer1PriceActive)
  price.priceAlterations = {}
  price.period = 0
  local new_price = table.deepcopy(price)
  new_price.period = 1
  local t0 = os.time() - 86400*10
  local t1 = helper.end_of_price(price, t0)
  local t1_new = helper.end_of_price(price)
  local data = {
    machine = mstate.machine,
    product = {
      status = "SUSPENDED",
      activeEndDate = t1,
      trialEndDate =t0,
      tarificationPeriod = 1,
      price = {price},
    },
    tasks = {}
  }
  -- activeEndDate and commercial_ended task is set according to SYSDATE of event
  local state = helper.build_machine_state("resuming", "paid", "commercial")
  local expected = {
    machine = state.machine,
    product = {
      status = "SUSPENDED",
      activeEndDate = function(real)
        local dt = real - t1_new
        return dt > -10 and dt < 10
      end,
      trialEndDate =t0,
      tarificationPeriod = 2,
      price = {new_price},
    },
    tasks = {
      {
        wakeAtFunction = "send_event",
        extraParams = {eventType = "commercial_ended"},
        wakeAt = function(real)
          local dt = real - (t1_new - helper.config.price_ended_before_time)
          return dt > -10 and dt < 10
        end
      },
      {
        wakeAtFunction = "RESUME",
        wakeAt = function(real)
          local dt = real - os.time()
          return dt > -10 and dt < 10
        end
      }
    },
    delete_tasks = {
      {wakeAtFunction = "send_event", extraParams = {eventType = "suspend_ended"}},
    },
  }
  local res, err = helper.send_event("payment_processed", data, "p1")
  print("result: ", json.encode(res))
  t.assert(res, err)
  helper.assert_result(res, expected)
end

g.test_from_suspended_past_event = function()
  local mstate = helper.build_machine_state("suspended", "notPaid", "commercial")
  local price = table.deepcopy(helper.prices.offer1.offer1PriceActive)
  price.priceAlterations = {}
  local new_price = table.deepcopy(price)
  price.period = 1
  new_price.period = 2
  local t0 = os.time()
  local t1 = helper.end_of_price(price)
  local tpast = os.time() - 86400 * 10
  local t1past = helper.end_of_price(price, tpast)
  local data = {
    machine = mstate.machine,
    product = {
      status = "SUSPENDED",
      activeEndDate = t1,
      trialEndDate =t0,
      tarificationPeriod = 1,
      price = {price},
    },
    tasks = {}
  }
  local state = helper.build_machine_state("resuming", "paid", "commercial")
  local expected = {
    machine = state.machine,
    product = {
      status = "SUSPENDED",
      activeEndDate = t1past,
      trialEndDate =t0,
      tarificationPeriod = 2,
      price = {new_price},
    },
    tasks = {
      {
        wakeAtFunction = "send_event",
        extraParams = {eventType = "commercial_ended"},
        wakeAt = function(real)
          local dt = real - (t1past - helper.config.price_ended_before_time)
          return dt > -10 and dt < 10
        end
      },
      {
        wakeAtFunction = "RESUME",
        wakeAt = function(real)
          local dt = real - os.time()
          return dt > -10 and dt < 10
        end
      }
    },
    delete_tasks = {
      {wakeAtFunction = "send_event", extraParams = {eventType = "suspend_ended"}},
    }
  }

  local event = {
    eventType = "payment_processed",
    eventDate = tpast,
  }
  local res, err = helper.send_event(event, data, "p1")
  print("result: ", json.encode(res))
  t.assert(res, err)
  helper.assert_result(res, expected)
end
