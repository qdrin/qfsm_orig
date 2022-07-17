--------------------------------------------------------------------------------
--- Lua part of HFSM module for Tarantool
--------------------------------------------------------------------------------
local log = require('log')
local icu_date = require('icu-date')
local json = require('json')
local checks = require('checks')

local function check_args(model, ...)
  local function _check(...)
    local _args = {...}
    if #_args == 1 then _args = _args[1] end
    checks(model)
    return true
  end
  local args = {...}
  local ok, res = pcall(_check, unpack(args))
  if not ok then log.warn(res) end
  return ok, res
end

local price_model = {
  id = 'string',
  productStatus = 'string',
  recurringChargePeriodType = 'string',
  recurringChargePeriodLength = 'number',
  name = '?string',
  priceType = 'string',
  duration = '?number',                     -- Параметр каталога: максимальное количество периодов
  period = '?number',                       -- Параметр runtime: текущий период
  nextPrice = '?string',
  priceAlterations = '?table',
  tax = '?',
  price = '?',
  unitOfMeasure = '?',
  validFor = '?',
  nextEntity = '?',
  href = '?',
}

checkers.product_price = checkers.product_price or function(pr)
  if type(pr) ~= 'table' or #pr > 1 then
    log.warn("price should be array table with 1 element")
    return false, "price should be array table with 1 element"
  end
  if #pr == 0  and not next(pr) then return true end
  -- log.warn("price: %s", json.encode(pr[1]))
  local res, err = check_args(price_model, pr[1])
  if not res then log.error(err) end
  return res, err
end

-- this used at machine:init() call to validate properties arg
local properties_model = {
  machine = 'table',
  product = {
    status = 'string',
    productOfferingId = '?string',
    activeEndDate = "?",
    trialEndDate = "?",
    tarificationPeriod = "?",
    nextConfigTariffMode = "?",
    currentConfigTariffMode = "?",
    price = "?product_price",
    label = "?table",
    characteristic = "?table",
  },
  properties = {
    nextPrice = "?product_price",
  },
  tasks = "?table",
  delete_tasks = "?table",
}

-- this used at machine:send_event() call to validate event
local event_model = {
  eventType = 'string',
  eventDate = 'number',
  sourceCode = '?string',
  refId = '?string',
  refIdType = '?string',
  products = '?table',
}

return {
  event_model = event_model,
  price_model = price_model,
  properties_model = properties_model,
  check_args = check_args,
}