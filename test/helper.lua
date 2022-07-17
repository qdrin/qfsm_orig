local fiber = require('fiber')
local json = require('json')
local uuid = require('uuid')
local fio = require('fio')
local yaml = require('yaml')
local qfsm = require('qfsm')
local utils = require('test.utils')
local t = require('luatest')

local helper = {}

helper.send_event_fiberized = function(ev_type, _props, prId, ch)
  local productId = prId or uuid.str()
  local m = qfsm.lease()
  t.assert(m)
  local _res, _err = m:init(productId, _props)
  t.assert(_res, json.encode(_err))
  _res, err = m:send_event({eventType=ev_type})
  t.assert(_res, json.encode(_err))
  m:release()
  t.assert(not ch:is_full(), "channel is full")
  ch:put(_res)
end

helper.send_event = function(ev_data, _props, prId)
  local productId = prId or uuid.str()
  local m = qfsm.lease()
  t.assert(m)
  print("send_event _props", json.encode(_props))
  local _res, _err, _status = m:init(productId, _props)
  if not _res then
    m:release()
    return _res, _err, _status
  end
  local event
  if type(ev_data) == 'table' then
    event = table.deepcopy(ev_data)
  else
    event = {eventType = ev_data}
  end
  event.eventDate = event.eventDate or fiber.time()
  print("event", json.encode(event))
  _res, _err, _status = m:send_event(event)
  m:release()
  if not _res then
    return _res, _err, _status
  end
  return _res
end

helper.build_machine_state = function(_usage, _payment, _price)
  local states = {
    usage = {
      pendingActivate={machine={pendingActivate=1}, state="PENDING_ACTIVATE"},
      aborted={machine={aborted=1}, state="ABORTED"},
      disconnect={machine={disconnect=1}, state="DISCONNECT"},
      activeTrial = {machine={productProvision={usage={usageOn={activated={activeTrial=1}}}}}, state="ACTIVE_TRIAL"},
      active = {machine={productProvision={usage={usageOn={activated={active=1}}}}}, state="ACTIVE"},
      suspending = {machine={productProvision={usage={usageOn={suspending=1}}}}, state="SUSPENDING"},
      suspended = {machine={productProvision={usage={usageOn={suspended=1}}}}, state="SUSPENDED"},
      prolongation = {machine={productProvision={usage={usageOn={prolongation=1}}}}, state="PROLONGATION"},
      resuming = {machine={productProvision={usage={usageOn={resuming=1}}}}, state="RESUMING"},
      pendingDisconnect = {machine={productProvision={usage={usageOff={pendingDisconnect=1}}}}, state="PENDING_DISCONNECT"},
      disconnection = {machine={productProvision={usage={usageOff={disconnection=1}}}}, state="DISCONNECTION"},
      disconnected = {machine={productProvision={usage={usageOff={disconnected=1}}}}, state="DISCONNECTED"},
      usageFinal={productProvision={usage={usageFinal=1}}, state="DISCONNECTED"},
    },
    payment = {
      paid={productProvision={payment={paymentOn={paid=1}}}},
      notPaid={productProvision={payment={paymentOn={notPaid=1}}}},
      waitingPayment={productProvision={payment={paymentOn={waitingPayment=1}}}},
      paymentStopping={productProvision={payment={paymentOff={paymentStopping=1}}}},
      paymentStopped={productProvision={payment={paymentOff={paymentStopped=1}}}},
      paymentFinal={productProvision={payment={paymentFinal=1}}},
    },
    price = {
      trial={productProvision={price={priceOn={trial=1}}}},
      commercial={productProvision={price={priceOn={commercial=1}}}},
      priceChanging={productProvision={price={priceOn={priceChanging=1}}}},
      priceChanged={productProvision={price={priceOn={priceChanged=1}}}},
      priceNotChanged={productProvision={price={priceOn={priceNotChanged=1}}}},
      priceOff={productProvision={price={priceOff=1}}},
      priceFinal={productProvision={price={priceFinal=1}}},
    }
  }
  local u, p, pr, state
  u = states.usage[_usage].machine
  state = states.usage[_usage].state
  if _payment then p = states.payment[_payment] end
  if _price then pr = states.price[_price] end
  if p and pr then
    for pname, pp in pairs(p.productProvision) do u.productProvision[pname] = pp end
    for pname, pp in pairs(pr.productProvision) do u.productProvision[pname] = pp end
  end
  return {machine = u, state = state}
end

function helper.tostring(obj)
  if type(obj) ~= 'table' then return obj end
  return json.encode(obj, {encode_use_tostring=true})
end

helper.covers_recurse = function(fact, expected)
  local pres, res, err, has_element
  local error = ""
  local real = table.deepcopy(fact)
  if #expected > 0 then
    if not(#real > 0) then
      return nil, string.format("array expected: %s", json.encode(expected))
    end
    for i, v in pairs(expected) do
      has_element = false
      for j, r in pairs(real) do
        pres, res, err = pcall(helper.covers_recurse, r, v)
        if not pres then
          err = res
          res = nil
          print("fatal", err)
        end
        if res then
          has_element = true
          break
        end
        error = error .. ": " .. err
      end
      if not has_element then
        return nil, string.format("not found: %s -> %s\n%s", helper.tostring(i), helper.tostring(v), error)
      end
    end
    return real
  else
    for n, v in pairs(expected) do
      -- print("real", real[n], "v", v)
      -- if real[n] == nil then return nil, string.format("key '%s' not found", n) end
      if type(v) == 'table' then
        pres, res, err = pcall(helper.covers_recurse, real[n], v)
        if not res then return nil, err end
      elseif type(v) == 'function' then
        if not v(real[n]) then
          return nil, string.format("function check failed on '%s'", n)
        end
      else
        if v ~= real[n] then
          return nil, string.format("fields '%s' are not equal", n)
        end
      end
    end
    return real
  end
end

helper.assert_result = function(result, expected)
  local res, err = helper.covers_recurse(result, expected)
  t.assert(res, err)
  if expected.delete_tasks then
    t.assert_equals(#res.delete_tasks, #expected.delete_tasks)
  end
  if expected.tasks then
    t.assert_equals(#res.tasks, #expected.tasks)
  end
end

helper.end_of_price = function(price, start)
  start = start or os.time()
  local date_funcs = {
    day = function()
      return start + price.recurringChargePeriodLength * 86400
    end,
    month = function()
      return utils.addmonths(start, price.recurringChargePeriodLength)
    end,
  }
  t.assert(price.recurringChargePeriodType, "no price.recurringChargePeriodType")
  local dfunc = date_funcs[price.recurringChargePeriodType]
  t.assert(dfunc, "no date function found")
  return dfunc()
end

helper.read_config = function(file_name)
  local fname = file_name or "test/data/config.yaml"
  local f = fio.open(fname)
  local ystr = f:read()
  f:close()
  local conf = yaml.decode(ystr)
  helper.config = conf
  return true
end

helper.read_prices = function(file_name)
  local fname = file_name or "test/data/prices.yaml"
  local f = fio.open(fname)
  local ystr = f:read()
  f:close()
  local conf = yaml.decode(ystr)
  helper.prices = conf
  for offer, oprices in pairs(helper.prices) do
    setmetatable(oprices, {
      __index = function(tbl, key)
        if type(key) == 'number' then return rawget(tbl, key) end
        for i, v in pairs(tbl) do
          if v.id == key then return v end
        end
        return nil
      end
    })
  end
  return true
end

return helper