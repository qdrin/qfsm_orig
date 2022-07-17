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

g.test_trial_to_trial = function()
  local mstate = helper.build_machine_state("activeTrial", "paid", "priceChanging")
  local price = helper.prices.offer1.offer1PriceTrial
  local t0 = os.time()
  local t1 = helper.end_of_price(price)
  local data = {
    machine = mstate.machine,
    product = {
      status = "ACTIVE_TRIAL",
      activeEndDate = t0,
      trialEndDate =t0,
      price = {price},
    },
    properties = {
      nextPrice = {price},
    },
    tasks = {}
  }
  print("data", json.encode(data))
  local res, err, status = helper.send_event("change_price", data, "p1")
  print("result: ", json.encode(res), "status", status, "error", err)
  t.assert_not(res)
  t.assert_equals(status, -2)
end

g.test_commercialpromo_to_commercialpromo = function()
  local mstate = helper.build_machine_state("active", "paid", "priceChanging")
  local price = helper.prices.offer1.offer1PriceActive
  local t0 = os.time()
  local t1 = helper.end_of_price(price)
  local data = {
    machine = mstate.machine,
    product = {
      status = "ACTIVE_TRIAL",
      activeEndDate = t0,
      trialEndDate =t0,
      price = {price},
    },
    properties = {
      nextPrice = {price},
    },
    tasks = {}
  }
  print("data", json.encode(data))
  local res, err, status = helper.send_event("change_price", data, "p1")
  print("result: ", json.encode(res), "status", status, "error", err)
  t.assert_not(res)
  t.assert_equals(status, -2)
end

g.test_trial_to_commercial = function()
  local mstate = helper.build_machine_state("activeTrial", "paid", "priceChanging")
  local price = helper.prices.offer1.offer1PriceTrial
  local new_price = helper.prices.offer1.offer1PriceActive
  local t0 = os.time()
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
  local state = helper.build_machine_state("activeTrial", "paid", "priceChanged")
  local expected = {
    machine = state.machine,
    product = {
      status = "ACTIVE_TRIAL",
      activeEndDate = t0,
      trialEndDate =t0,
    },
    tasks = {
      {
        wakeAtFunction = "CHANGE_PRICE",
        wakeAt = function(real)
          local dt = real - os.time()
          return dt > -10 and dt < 10
        end,
        extraParams = {
          nextPrice = {new_price},
        }
      }
    },
    delete_tasks = {},
  }
  print("data", json.encode(data))
  local res, err = helper.send_event("change_price", data, "p1")
  print("result: ", json.encode(res))
  t.assert(res, err)
  helper.assert_result(res, expected)
end

g.test_trial_to_commercial_no_next_price = function()
  local mstate = helper.build_machine_state("activeTrial", "paid", "priceChanging")
  local price = helper.prices.offer1.offer1PriceTrial
  local new_price = helper.prices.offer1.offer1PriceActive
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
  local state = helper.build_machine_state("activeTrial", "paid", "priceChanged")
  print("data", json.encode(data))
  local res, err, status = helper.send_event("change_price", data, "p1")
  print("result: ", json.encode(res), "status", status, "error", err)
  t.assert_not(res)
  t.assert_equals(status, -2)
  t.assert(err:match("nextPrice not found"))
  -- t.assert(false)
end

g.test_trial_to_commercialpromo = function()
  local mstate = helper.build_machine_state("activeTrial", "paid", "priceChanging")
  local price = helper.prices.offer1.offer1PriceTrial
  local new_price = helper.prices.offer1.offer1PriceActive
  local t0 = os.time()
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
  local state = helper.build_machine_state("activeTrial", "paid", "priceChanged")
  local expected = {
    machine = state.machine,
    product = {
      status = "ACTIVE_TRIAL",
      activeEndDate = t0,
      trialEndDate =t0,
      price = {price},
    },
    properties = {
      nextPrice = {new_price},
    },
    tasks = {
      {
        wakeAtFunction = "CHANGE_PRICE",
        wakeAt = function(real)
          local dt = real - os.time()
          return dt > -10 and dt < 10
        end,
        extraParams = {
          nextPrice = {new_price},
        }
      }
    },
    delete_tasks = {},
  }
  print("data", json.encode(data))
  local res, err = helper.send_event("change_price", data, "p1")
  print("result: ", json.encode(res))
  t.assert(res, err)
  helper.assert_result(res, expected)
end

g.test_commercial_to_commercialpromo = function()
  local mstate = helper.build_machine_state("active", "paid", "priceChanging")
  local price = table.deepcopy(helper.prices.offer1.offer1PriceActive)
  print('price', json.encode(price))
  price.priceAlterations = {}
  local new_price = table.deepcopy(helper.prices.offer1.offer1PriceActive)
  local t0 = os.time()
  local t1 = helper.end_of_price(new_price)
  local data = {
    machine = mstate.machine,
    product = {
      status = "ACTIVE",
      activeEndDate = t0,
      trialEndDate =t0,
      price = {price},
    },
    properties = {
      nextPrice = {new_price},
    },
    tasks = {}
  }
  local state = helper.build_machine_state("active", "paid", "priceChanged")
  local expected = {
    machine = state.machine,
    product = {
      status = "ACTIVE",
      activeEndDate = t0,
      trialEndDate =t0,
    },
    tasks = {
      {
        wakeAtFunction = "CHANGE_PRICE",
        wakeAt = function(real)
          local dt = real - os.time()
          return dt > -10 and dt < 10
        end,
        extraParams = {
          nextPrice = {new_price},
        }
      }
    },
    delete_tasks = {},
  }
  print("t1", t1, "data", json.encode(data))
  local res, err = helper.send_event("change_price", data, "p1")
  print("result: ", json.encode(res))
  t.assert(res, err)
  helper.assert_result(res, expected)
end

g.test_commercial_to_trial = function()
  local mstate = helper.build_machine_state("active", "paid", "priceChanging")
  local price = helper.prices.offer1.offer1PriceActive
  local new_price = helper.prices.offer1.offer1PriceTrial
  local t0 = os.time()
  local t1 = helper.end_of_price(new_price)
  local data = {
    machine = mstate.machine,
    product = {
      status = "ACTIVE",
      activeEndDate = t0,
      trialEndDate =t0 - 86400*30,
      price = {price},
    },
    properties = {
      nextPrice = {new_price},
    },
    tasks = {}
  }
  local state = helper.build_machine_state("active", "paid", "priceChanged")
  local expected = {
    machine = state.machine,
    product = {
      status = "ACTIVE",
      activeEndDate = data.product.activeEndDate,
      trialEndDate =data.product.trialEndDate,
    },
    tasks = {
      {
        wakeAtFunction = "CHANGE_PRICE",
        wakeAt = function(real)
          print("real", real, "expected", os.time())
          local dt = real - os.time()
          return dt > -10 and dt < 10
        end,
        extraParams = {
          nextPrice = {new_price},
        }
      }
    },
    delete_tasks = {},
  }
  print("nextPrice", json.encode(new_price))
  print("data", json.encode(data))
  local res, err = helper.send_event("change_price", data, "p1")
  print("result: ", json.encode(res))
  t.assert(res, err)
  helper.assert_result(res, expected)
end

g.test_commercialpromo_to_commercial = function()
  local mstate = helper.build_machine_state("active", "paid", "priceChanging")
  local price = table.deepcopy(helper.prices.offer1.offer1PriceActive)
  local new_price = table.deepcopy(helper.prices.offer1.offer1PriceActive)
  new_price.priceAlterations = {}
  local t0 = os.time()
  local t1 = helper.end_of_price(new_price)
  local data = {
    machine = mstate.machine,
    product = {
      status = "ACTIVE",
      activeEndDate = t0,
      trialEndDate =t0,
      price = {price},
    },
    properties = {
      nextPrice = {new_price},
    },
    tasks = {}
  }
  local state = helper.build_machine_state("active", "paid", "priceChanged")
  local expected = {
    machine = state.machine,
    product = {
      status = "ACTIVE",
      activeEndDate = t0,
      trialEndDate =t0,
    },
    tasks = {
      {
        wakeAtFunction = "CHANGE_PRICE",
        wakeAt = function(real)
          local dt = real - os.time()
          return dt > -10 and dt < 10
        end,
        extraParams = {
          nextPrice = {new_price},
        }
      }
    },
    delete_tasks = {},
  }
  print("t1", t1, "data", json.encode(data))
  local res, err = helper.send_event("change_price", data, "p1")
  print("result: ", json.encode(res))
  t.assert(res, err)
  helper.assert_result(res, expected)
end
