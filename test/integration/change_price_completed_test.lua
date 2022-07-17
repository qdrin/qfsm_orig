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

g.test_trial_to_active = function()
  local mstate = helper.build_machine_state("activeTrial", "paid", "priceChanged")
  local price = table.deepcopy(helper.prices.offer1.offer1PriceTrial)
  local new_price = table.deepcopy(helper.prices.offer1.offer1PriceActive)
  new_price.priceAlterations = {}
  local t0 = os.time()
  local t1 = helper.end_of_price(new_price)
  local data = {
    machine = mstate.machine,
    product = {
      status = "ACTIVE_TRIAL",
      activeEndDate = t0,
      trialEndDate =t0,
      price = {price},
    },
    properties = {
      nextPrice = {new_price},
    },
    tasks = {}
  }
  local state = helper.build_machine_state("activeTrial", "waitingPayment", "commercial")
  local expected = {
    machine = state.machine,
    product = {
      status = "ACTIVE_TRIAL",
      activeEndDate = t1,
      trialEndDate =t0,
      price = {new_price},
    },
    tasks = {
      {
        wakeAtFunction = "send_event",
        wakeAt = function(real)
          local dt = real - (t1 - helper.config.price_ended_before_time)
          return dt > -10 and dt < 10
        end,
        extraParams = {
          eventType = "commercial_ended",
        }
      },
      {
        wakeAtFunction = "send_event",
        wakeAt = function(real)
          local dt = real - (os.time() + helper.config.payment_waiting_time)
          return dt > -10 and dt < 10
        end,
        extraParams = {
          eventType = "waiting_pay_ended",
        }
      },
    },
    delete_tasks = {},
  }
  print("data", json.encode(data))
  local res, err = helper.send_event("change_price_completed", data, "p1")
  print("result: ", json.encode(res))
  t.assert(res, err)
  helper.assert_result(res, expected)
end

g.test_trial_to_active_past_date = function()
  local mstate = helper.build_machine_state("activeTrial", "paid", "priceChanged")
  local price = helper.prices.offer1.offer1PriceTrial
  local new_price = helper.prices.offer1.offer1PriceActive
  local t0 = os.time() - 86400*10
  local t1 = helper.end_of_price(new_price, t0)
  local data = {
    machine = mstate.machine,
    product = {
      status = "ACTIVE_TRIAL",
      activeEndDate = os.time(),
      trialEndDate =os.time(),
      price = {price},
    },
    properties = {
      nextPrice = {new_price},
    },
    tasks = {}
  }
  local state = helper.build_machine_state("activeTrial", "waitingPayment", "commercial")
  local expected = {
    machine = state.machine,
    product = {
      status = "ACTIVE_TRIAL",
      activeEndDate = t1,
      trialEndDate =data.product.trialEndDate,
    },
    tasks = {
      {
        wakeAtFunction = "send_event",
        wakeAt = function(real)
          local dt = real - (t1 - helper.config.price_ended_before_time)
          return dt > -10 and dt < 10
        end,
        extraParams = {
          eventType = "commercial_ended",
        }
      },
      {
        wakeAtFunction = "send_event",
        wakeAt = function(real)
          local dt = real - (os.time() + helper.config.payment_waiting_time)
          return dt > -10 and dt < 10
        end,
        extraParams = {
          eventType = "waiting_pay_ended",
        }
      },
    },
    delete_tasks = {},
  }
  local event = {
    eventType = "change_price_completed",
    eventDate = t0,
  }
  print("data", json.encode(data))
  local res, err = helper.send_event(event, data, "p1")
  print("result: ", json.encode(res))
  t.assert(res, err)
  helper.assert_result(res, expected)
end

g.test_active_to_trial = function()
  local mstate = helper.build_machine_state("active", "paid", "priceChanged")
  local price = helper.prices.offer1.offer1PriceActive
  local new_price = helper.prices.offer1.offer1PriceTrial
  local t0 = os.time()
  local t1 = helper.end_of_price(new_price)
  local data = {
    machine = mstate.machine,
    product = {
      status = "ACTIVE",
      activeEndDate = t0,
      trialEndDate =t0 - 86400 * 30,
      price = {price},
    },
    properties = {
      nextPrice = {new_price},
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
    },
    tasks = {
      {
        wakeAtFunction = "send_event",
        wakeAt = function(real)
          local dt = real - (t1 - helper.config.price_ended_before_time)
          return dt > -10 and dt < 10
        end,
        extraParams = {
          eventType = "trial_ended",
        }
      }
    },
    delete_tasks = {},
  }
  print("data", json.encode(data))
  local res, err = helper.send_event("change_price_completed", data, "p1")
  print("result: ", json.encode(res))
  t.assert(res, err)
  helper.assert_result(res, expected)
end

g.test_active_to_trial_past_date = function()
  local mstate = helper.build_machine_state("active", "paid", "priceChanged")
  local price = helper.prices.offer1.offer1PriceActive
  local new_price = helper.prices.offer1.offer1PriceTrial
  local t0 = os.time() - 86400*10
  local t1 = helper.end_of_price(new_price, t0)
  local data = {
    machine = mstate.machine,
    product = {
      status = "ACTIVE",
      activeEndDate = os.time(),
      trialEndDate =os.time() - 86400 * 30,
      price = {price},
    },
    properties = {
      nextPrice = {new_price},
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
    },
    tasks = {
      {
        wakeAtFunction = "send_event",
        wakeAt = function(real)
          local dt = real - (t1 - helper.config.price_ended_before_time)
          return dt > -10 and dt < 10
        end,
        extraParams = {
          eventType = "trial_ended",
        }
      }
    },
    delete_tasks = {},
  }
  print("data", json.encode(data))
  local event = {
    eventType = "change_price_completed",
    eventDate = t0,
  }
  local res, err = helper.send_event(event, data, "p1")
  print("result: ", json.encode(res))
  t.assert(res, err)
  helper.assert_result(res, expected)
end

g.test_trial_to_activepromo = function()
  local mstate = helper.build_machine_state("activeTrial", "paid", "priceChanged")
  local price = helper.prices.offer1.offer1PriceTrial
  price.period = 1
  local new_price = helper.prices.offer1.offer1PriceActive
  local t0 = os.time()
  local t1 = helper.end_of_price(new_price)
  local data = {
    machine = mstate.machine,
    product = {
      status = "ACTIVE_TRIAL",
      activeEndDate = t0,
      trialEndDate =t0,
      price = {price},
      tarificationPeriod = 1,
    },
    properties = {
      nextPrice = {new_price},
    },
    tasks = {}
  }
  local state = helper.build_machine_state("activeTrial", "waitingPayment", "commercial")
  local expected = {
    machine = state.machine,
    product = {
      status = "ACTIVE_TRIAL",
      price = {new_price},
      activeEndDate = t1,
      trialEndDate =t0,
    },
    tasks = {
      {
        wakeAtFunction = "send_event",
        wakeAt = function(real)
          local dt = real - (t1 - helper.config.price_ended_before_time)
          return dt > -10 and dt < 10
        end,
        extraParams = {
          eventType = "commercial_ended",
        }
      },
      {
        wakeAtFunction = "send_event",
        wakeAt = function(real)
          local dt = real - (os.time() + helper.config.payment_waiting_time)
          return dt > -10 and dt < 10
        end,
        extraParams = {
          eventType = "waiting_pay_ended",
        }
      },
    },
    delete_tasks = {},
  }
  print("data", json.encode(data))
  local res, err = helper.send_event("change_price_completed", data, "p1")
  print("result: ", json.encode(res))
  t.assert(res, err)
  helper.assert_result(res, expected)
end

g.test_activepromo_to_active = function()
  local mstate = helper.build_machine_state("active", "paid", "priceChanged")
  local price = table.deepcopy(helper.prices.offer1.offer1PriceActive)
  price.period = 1
  local new_price = table.deepcopy(helper.prices.offer1.offer1PriceActive)
  new_price.priceAlterations = {}
  local t0 = os.time() - 86400*30
  local t1 = helper.end_of_price(new_price)
  local data = {
    machine = mstate.machine,
    product = {
      status = "ACTIVE",
      activeEndDate = os.time(),
      trialEndDate =t0,
      price = {price},
      tarificationPeriod = 1,
    },
    properties = {
      nextPrice = {new_price},
    },
    tasks = {}
  }
  local state = helper.build_machine_state("active", "waitingPayment", "commercial")
  local expected = {
    machine = state.machine,
    product = {
      status = "ACTIVE",
      price = {new_price},
      activeEndDate = t1,
      trialEndDate =t0,
    },
    tasks = {
      {
        wakeAtFunction = "send_event",
        wakeAt = function(real)
          local dt = real - (t1 - helper.config.price_ended_before_time)
          return dt > -10 and dt < 10
        end,
        extraParams = {
          eventType = "commercial_ended",
        }
      },
      {
        wakeAtFunction = "send_event",
        wakeAt = function(real)
          local dt = real - (os.time() + helper.config.payment_waiting_time)
          return dt > -10 and dt < 10
        end,
        extraParams = {
          eventType = "waiting_pay_ended",
        }
      },
    },
    delete_tasks = {},
  }
  print("data", json.encode(data))
  local res, err = helper.send_event("change_price_completed", data, "p1")
  print("result: ", json.encode(res))
  t.assert(res, err)
  helper.assert_result(res, expected)
end

g.test_trial_to_active_without_next_price = function()
  local mstate = helper.build_machine_state("activeTrial", "paid", "priceChanged")
  local price = helper.prices.offer1.offer1PriceTrial
  price.period = 1
  local new_price = helper.prices.offer1.offer1PriceActive
  local t0 = os.time()
  local t1 = helper.end_of_price(new_price)
  local data = {
    machine = mstate.machine,
    product = {
      status = "ACTIVE_TRIAL",
      activeEndDate = t0,
      trialEndDate =t0,
      price = {price},
      tarificationPeriod = 1,
    },
    properties = {
    },
    tasks = {}
  }

  print("data", json.encode(data))
  local res, err, status = helper.send_event("change_price_completed", data, "p1")
  print("result: ", json.encode(res), "status", status, "error", err)
  t.assert_not(res)
  t.assert_equals(status, -2)
end

g.test_trial_to_active_without_properties = function()
  local mstate = helper.build_machine_state("activeTrial", "paid", "priceChanged")
  local price = helper.prices.offer1.offer1PriceTrial
  price.period = 1
  local new_price = helper.prices.offer1.offer1PriceActive
  local t0 = os.time()
  local t1 = helper.end_of_price(new_price)
  local data = {
    machine = mstate.machine,
    product = {
      status = "ACTIVE_TRIAL",
      activeEndDate = t0,
      trialEndDate =t0,
      price = {price},
      tarificationPeriod = 1,
    },
    tasks = {}
  }

  print("data", json.encode(data))
  local res, err, status = helper.send_event("change_price_completed", data, "p1")
  print("result: ", json.encode(res), "status", status, "error", err)
  t.assert_not(res)
  t.assert_equals(status, -2)
  -- t.assert(false)
end