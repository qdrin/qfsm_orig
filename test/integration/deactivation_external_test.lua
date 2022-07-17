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

g.test_from_active = function()
  local mstate = helper.build_machine_state("active", "paid", "commercial")
  local t1 = os.time() + 86400*30
  local data = {
    machine = mstate.machine,
    product = {
      status = "ACTIVE",
      productOfferingId = "offer1",  -- productOfferingId useless but we need test backward compatibility
      activeEndDate = t1,
      trialEndDate = os.time()
    },
    tasks = {}
  }
  local state = helper.build_machine_state("active", "paymentStopping", "commercial")
  local expected = {
    machine = state.machine,
    product = {
      status = "ACTIVE",
      activeEndDate = data.product.activeEndDate,
      trialEndDate = data.product.trialEndDate,
    },
    tasks = {
      {
        wakeAtFunction = "DISCONNECT_EXTERNAL",
        wakeAt = function(real)
          local dt = real - os.time()
          return dt >= 0 and dt < 610
        end,
      }
    },
  }
  local res, err = helper.send_event("deactivation_external", data, "p1")
  print("result: ", json.encode(res))
  helper.assert_result(res, expected)
end

g.test_from_activeTrial = function()
  local mstate = helper.build_machine_state("activeTrial", "paid", "trial")
  local t1 = os.time() + 86400*30
  local data = {
    machine = mstate.machine,
    product = {
      status = "ACTIVE_TRIAL",
      activeEndDate = t1,
      trialEndDate = t1
    },
    tasks = {}
  }
  local state = helper.build_machine_state("activeTrial", "paymentStopping", "trial")
  local expected = {
    machine = state.machine,
    product = {
      status = "ACTIVE_TRIAL",
      activeEndDate = data.product.activeEndDate,
      trialEndDate = data.product.trialEndDate
    },
    tasks = {
      {
        wakeAtFunction = "DISCONNECT_EXTERNAL",
        wakeAt = function(real)
          local dt = real - os.time()
          return dt >= 0 and dt < 610
        end,
      }
    },
  }
  local res, err = helper.send_event("deactivation_external", data, "p1")
  print("result: ", json.encode(res))
  helper.assert_result(res, expected)
end

g.test_from_resuming = function()
  local mstate = helper.build_machine_state("resuming", "paid", "commercial")
  local data = {
    machine = mstate.machine,
    product = {
      status = "SUSPENDED",
      activeEndDate = os.time() + 86400*30,
      trialEndDate = os.time()
    },
    tasks = {}
  }
  local state = helper.build_machine_state("resuming", "paymentStopping", "commercial")
  local expected = {
    machine = state.machine,
    product = {
      status = "SUSPENDED",
      activeEndDate = data.product.activeEndDate,
      trialEndDate = data.product.trialEndDate,
    },
    tasks = {
      {
        wakeAtFunction = "DISCONNECT_EXTERNAL",
        wakeAt = function(real)
          local dt = real - os.time()
          return dt >= 0 and dt < 610
        end,
      }
    },
  }

  local res, err = helper.send_event("deactivation_external", data, "p1")
  print("result", json.encode(res))
  helper.assert_result(res, expected)
end

g.test_from_prolongation = function()
  local mstate = helper.build_machine_state("prolongation", "paid", "commercial")
  local data = {
    machine = mstate.machine,
    product = {
      status = "ACTIVE",
      activeEndDate = os.time() + 86400*30,
      trialEndDate = os.time()
    },
    tasks = {}
  }
  local state = helper.build_machine_state("prolongation", "paymentStopping", "commercial")
  local expected = {
    machine = state.machine,
    product = {
      status = "ACTIVE",
      activeEndDate = data.product.activeEndDate,
      trialEndDate = data.product.trialEndDate,
    },
    tasks = {
      {
        wakeAtFunction = "DISCONNECT_EXTERNAL",
        wakeAt = function(real)
          local dt = real - os.time()
          return dt >= 0 and dt < 610
        end,
      }
    },
  }

  local res, err = helper.send_event("deactivation_external", data, "p1")
  print("result", json.encode(res))
  helper.assert_result(res, expected)
end

g.test_from_suspending = function()
  local mstate = helper.build_machine_state("suspending", "notPaid", "commercial")
  local data = {
    machine = mstate.machine,
    product = {
      status = "ACTIVE",
      activeEndDate = os.time() - 3600,
      trialEndDate = os.time() - 86400*30,
    },
    tasks = {}
  }
  local state = helper.build_machine_state("suspending", "paymentStopping", "commercial")
  local expected = {
    machine = state.machine,
    product ={
      status = "ACTIVE",
      activeEndDate = data.product.activeEndDate,
      trialEndDate = data.product.trialEndDate,
    },
    tasks = {
      {
        wakeAtFunction = "DISCONNECT_EXTERNAL",
        wakeAt = function(real)
          local dt = real - os.time()
          return dt >= 0 and dt < 610
        end,
      }
    },
  }

  local res, err = helper.send_event("deactivation_external", data, "p1")
  print("result", json.encode(res))
  helper.assert_result(res, expected)
end

g.test_from_suspended = function()
  local mstate = helper.build_machine_state("suspended", "notPaid", "commercial")
  local t0 = os.time()
  local t1 = t0 - 86400*30
  local data = {
    machine = mstate.machine,
    product = {
      status = "SUSPENDED",
      activeEndDate = t1,
    },
    tasks = {}
  }
  local state = helper.build_machine_state("suspended", "paymentStopping", "commercial")
  local expected = {
    machine = state.machine,
    product = {
      status = data.product.status,
      activeEndDate = data.product.activeEndDate,
    },
    tasks = {
      {
        wakeAtFunction = "DISCONNECT_EXTERNAL",
        wakeAt = function(real)
          local dt = real - os.time()
          return dt > -10 and dt < 10
        end,
      }
    },
  }

  local res, err = helper.send_event("deactivation_external", data, "p1")
  print("resutl", json.encode(res))
  helper.assert_result(res, expected)
end

g.test_from_waitingPayment_activeTrial = function()
  local mstate = helper.build_machine_state("activeTrial", "waitingPayment", "commercial")
  local data = {
    machine = mstate.machine,
    product = {
      status = "ACTIVE_TRIAL",
      activeEndDate = os.time() + 86400*30,
      trialEndDate = os.time()
    },
    tasks = {}
  }
  local state = helper.build_machine_state("activeTrial", "paymentStopping", "commercial")
  local expected = {
    machine = state.machine,
    product = {
      status = "ACTIVE_TRIAL",
      activeEndDate = data.product.activeEndDate,
      trialEndDate = data.product.trialEndDate,
    },
    tasks = {
      {
        wakeAtFunction = "DISCONNECT_EXTERNAL",
        wakeAt = function(real)
          local dt = real - os.time()
          return dt >= 0 and dt < 610
        end,
      }
    },
  }

  local res, err = helper.send_event("deactivation_external", data, "p1")
  print("result", json.encode(res))
  helper.assert_result(res, expected)
end

g.test_from_waitingPayment_active = function()
  local mstate = helper.build_machine_state("active", "waitingPayment", "commercial")
  local price = helper.prices.offer1.offer1PriceActive
  local t0 = os.time() + 86400*30
  local data = {
    machine = mstate.machine,
    product = {
      status = "ACTIVE",
      activeEndDate = t0,
      trialEndDate = os.time(),
      price = {price}
    },
    tasks = {}
  }
  local state = helper.build_machine_state("active", "paymentStopping", "commercial")
  local expected = {
    machine = state.machine,
    product = {
      status = data.product.status,
      activeEndDate = data.product.activeEndDate,
      trialEndDate = data.product.trialEndDate,
    },
    tasks = {
      {
        wakeAtFunction = "DISCONNECT_EXTERNAL",
        wakeAt = function(real)
          local dt = real - os.time()
          return dt >= 0 and dt < 610
        end,
      }
    },
    delete_tasks = {
      {wakeAtFunction = "send_event", extraParams = {eventType="waiting_pay_ended"}},
    },
  }

  local res, err = helper.send_event("deactivation_external", data, "p1")
  print("result", json.encode(res))
  helper.assert_result(res, expected)
end

g.test_from_price_changed = function()
  local mstate = helper.build_machine_state("activeTrial", "paid", "priceChanged")
  local price = helper.prices.offer1.offer1PriceActive
  local t0 = os.time()
  local data = {
    machine = mstate.machine,
    product = {
      status = "ACTIVE_TRIAL",
      activeEndDate = t0,
      trialEndDate = t0,
      price = {price}
    },
    tasks = {}
  }
  local state = helper.build_machine_state("activeTrial", "paymentStopping", "priceChanged")
  local expected = {
    machine = state.machine,
    product = {
      status = "ACTIVE_TRIAL",
      activeEndDate = data.product.activeEndDate,
    },
    tasks = {
      {
        wakeAtFunction = "DISCONNECT_EXTERNAL",
        extraParams = {eventType = "disconnect"},
        wakeAt = function(real)
          local dt = real - os.time()
          return dt >= 0 and dt < 610
        end,
      }
    },
  }

  local res, err = helper.send_event("deactivation_external", data, "p1")
  print("result", json.encode(res))
  helper.assert_result(res, expected)
end

g.test_from_trial_with_immediate_characteristic = function()
  local mstate = helper.build_machine_state("active", "paid", "commercial")
  local price = helper.prices.offer1.offer1PriceActive
  local t0 = helper.end_of_price(price)
  local data = {
    machine = mstate.machine,
    product = {
      status = "ACTIVE",
      activeEndDate = t0,
      trialEndDate = os.time(),
      tarificationPeriod = 1,
      price = {price},
      characteristic = {
        {
          type = "string",
          refName = "ActiveDeactivationMode",
          value = "Immediate"
        }
      },
    },
    tasks = {}
  }
  local state = helper.build_machine_state("active", "paymentStopping", "commercial")
  local expected = {
    machine = state.machine,
    product = {
      status = "ACTIVE",
      activeEndDate = data.product.activeEndDate,
      trialEndDate = data.product.trialEndDate,
    },
    tasks = {
      {
        wakeAtFunction = "DISCONNECT_EXTERNAL",
        wakeAt = function(real)
          local dt = real - os.time()
          return dt >= 0 and dt < 610
        end,
      }
    },
  }

  print("data", json.encode(data))
  local res, err = helper.send_event("deactivation_external", data, "p1")
  print("result", json.encode(res))
  helper.assert_result(res, expected)
end

g.test_from_wrong_state = function()
  local mstate = helper.build_machine_state("pendingDisconnect", "paymentFinal", "priceFinal")
  local data = {
    machine = mstate.machine,
    product = {
      status = "PENDING_DISCONNECT",
    },
    tasks = {}
  }

  local res, err, err_code = helper.send_event("deactivation_external", data, "p1")
  print("err_code", err_code, "error", err)
  t.assert_not(res)
  t.assert_equals(err_code, -2)
end

