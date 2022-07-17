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

g.test_duration_not_ended = function()
  local mstate = table.deepcopy(helper.build_machine_state("active", "paid", "commercial"))
  local price = table.deepcopy(helper.prices.offer1.offer1PriceActive)
  price.priceAlterations = {}
  local new_price = table.deepcopy(price)
  price.period = 1
  new_price.period = 2
  local t0 = os.time()
  local t1 = helper.end_of_price(price)
  local data = {
    machine = mstate.machine,
    product = {
      status = "ACTIVE",
      activeEndDate = t0,
      trialEndDate =t0,
      tarificationPeriod = 5,
      price = {price},
    },
    tasks = {}
  }
  local state = helper.build_machine_state("active", "waitingPayment", "commercial")
  local expected = {
    machine = state.machine,
    product = {
      status = "ACTIVE",
      activeEndDate = t1,
      trialEndDate =t0,
      tarificationPeriod = 6,
      price = {new_price},
    },
    tasks = {
      {
        wakeAtFunction = "send_event",
        extraParams = {
          eventType = "commercial_ended",
        },
        wakeAt = function(real)
          local dt = real - (t1 - helper.config.price_ended_before_time)
          return dt >= -10 and dt < 10
        end,
      },
      {
        wakeAtFunction = "send_event",
        extraParams = {
          eventType = "waiting_pay_ended",
        },
        wakeAt = function(real)
          local dt = real - (os.time() + helper.config.payment_waiting_time)
          return dt >= -10 and dt < 10
        end,
      }
    },
    delete_tasks = {},
  }
  local res, err = helper.send_event("commercial_ended", data, "p1")
  print("result: ", json.encode(res))
  t.assert(res, err)
  helper.assert_result(res, expected)
end

g.test_duration_not_ended_past_activeEndDate = function()
  local mstate = helper.build_machine_state("active", "paid", "commercial")
  local price = table.deepcopy(helper.prices.offer1.offer1PriceActive)
  price.priceAlterations = {}
  local new_price = table.deepcopy(price)
  local t0 = os.time() - 86400
  local t1 = helper.end_of_price(price, t0)
  price.period = 1
  new_price.period = 2

  local data = {
    machine = mstate.machine,
    product = {
      status = "ACTIVE",
      activeEndDate = t0,
      trialEndDate =t0,
      tarificationPeriod = 5,
      price = {price},
    },
    tasks = {}
  }
  local state = helper.build_machine_state("active", "waitingPayment", "commercial")
  local expected = {
    machine = state.machine,
    product = {
      status = "ACTIVE",
      activeEndDate = t1,
      trialEndDate =t0,
      tarificationPeriod = 6,
      price = {new_price},
    },
    tasks = {
      {
        wakeAtFunction = "send_event",
        extraParams = {
          eventType = "commercial_ended",
        },
        wakeAt = function(real)
          local dt = real - (t1 - helper.config.price_ended_before_time)
          return dt >= -10 and dt < 10
        end,
      },
      {
        wakeAtFunction = "send_event",
        extraParams = {
          eventType = "waiting_pay_ended",
        },
        wakeAt = function(real)
          local dt = real - (os.time() + helper.config.payment_waiting_time)
          return dt >= -10 and dt < 10
        end,
      }
    },
    delete_tasks = {},
  }
  
  local res, err = helper.send_event("commercial_ended", data, "p1")
  print("result: ", json.encode(res))
  t.assert(res, err)
  helper.assert_result(res, expected)
end

g.test_from_prolongation = function()
  local mstate = helper.build_machine_state("prolongation", "paid", "commercial")
  local price = helper.prices.offer1.offer1PriceActive
  local new_price = table.deepcopy(price)
  price.period = 2
  local t0 = os.time()
  local t1 = helper.end_of_price(price)
  local data = {
    machine = mstate.machine,
    product = {
      status = "ACTIVE",
      activeEndDate = t0,
      trialEndDate =t0,
      tarificationPeriod = 3,
      price = {price},
    },
    tasks = {}
  }

  print("data", json.encode(data))
  local res, err, err_code = helper.send_event("commercial_ended", data, "p1")
  print("result: ", json.encode(res), "err", err, "error_code", err_code)
  t.assert_not(res)
  t.assert_equals(err_code, -2)
end

g.test_from_resuming = function()
  local mstate = helper.build_machine_state("resuming", "paid", "commercial")
  local price = helper.prices.offer1.offer1PriceActive
  local new_price = table.deepcopy(price)
  price.period = 2
  local t0 = os.time()
  local t1 = helper.end_of_price(price)
  local data = {
    machine = mstate.machine,
    product = {
      status = "ACTIVE",
      activeEndDate = t0,
      trialEndDate =t0,
      tarificationPeriod = 3,
      price = {price},
    },
    tasks = {}
  }

  print("data", json.encode(data))
  local res, err, err_code = helper.send_event("commercial_ended", data, "p1")
  print("result: ", json.encode(res), "err", err, "error_code", err_code)
  t.assert_not(res)
  t.assert_equals(err_code, -2)
end

g.test_duration_ended = function()
  local mstate = helper.build_machine_state("active", "paid", "commercial")
  local price = table.deepcopy(helper.prices.offer1.offer1PriceActive)
  local new_price = table.deepcopy(price)
  new_price.priceAlterations = {}
  price.period = 1
  price.priceAlterations[1].period = 1
  new_price.period = 2
  local t0 = os.time()
  local t1 = helper.end_of_price(price)
  local data = {
    machine = mstate.machine,
    product = {
      status = "ACTIVE",
      activeEndDate = t0,
      trialEndDate =t0,
      tarificationPeriod = 2,
      price = {price},
    },
    tasks = {}
  }
  local state = helper.build_machine_state("active", "paid", "priceChanging")
  local expected = {
    machine = state.machine,
    product = {
      status = "ACTIVE",
      activeEndDate = t0,
      trialEndDate =t0,
      tarificationPeriod = 2,
      price = {price},
    },
    tasks = {
      {
        wakeAtFunction = "send_event",
        extraParams = {
          eventType = "change_price",
        },
        wakeAt = function(real)
          local dt = real - os.time()
          return dt >= -10 and dt < 10
        end,
      },
    },
    delete_tasks = {},
  }
  local res, err = helper.send_event("commercial_ended", data, "p1")
  print("result: ", json.encode(res))
  t.assert(res, err)
  helper.assert_result(res, expected)
end