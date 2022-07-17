local t = require('luatest')
local g = t.group()
local json = require('json')
local uuid = require('uuid')
local fiber = require('fiber')
local qfsm = require('qfsm')
local clock = require('clock')
local validators = require('qfsm.validators')

local helper = require('test.helper')

g.before_all(function()
  helper.read_config()
  helper.read_prices()
  print('qfsm.init finished')
end)

g.before_each(function()
end)

g.test_price_model = function()
  local price = table.deepcopy(helper.prices.offer1["offer1PriceTrial"])
  price.validFor = os.time() + 86400
  price.tax = 20.0
  price.price = 100.0
  price.unitOfMeasure = 'RU'
  price.nextEntity = 'next_entity_id'
  price.href = 'some_id'
  -- price.sic_transit_gloria_mundi = "sic"
  print("price", json.encode(price))
  t.assert(price)
  local res, err = validators.check_args(validators.price_model, price)
  t.assert(res, err)
end

g.test_properties_model = function()
  local data = {
    machine = {pendingActivate=1},
    product = {
      status = "PENDING_ACTIVATE",
    },
    tasks = {}
  }
  print("data", json.encode(data))
  local res, err = validators.check_args(validators.properties_model, data)
  t.assert(res, err)
end

g.test_properties_model_with_price = function()
  local price = table.deepcopy(helper.prices.offer1["offer1PriceTrial"])
  -- price.sic_transit_gloria_mundi = "sic"
  local data = {
    machine = {pendingActivate=1},
    product = {
      status = "PENDING_ACTIVATE",
      price = {price},
    },
    tasks = {}
  }
  print("data", json.encode(data))
  local res, err = validators.check_args(validators.properties_model, data)
  t.assert(res, err)
end

g.test_properties_model_with_wrong_price = function()
  local data = {
    machine = {pendingActivate=1},
    product = {
      status = "PENDING_ACTIVATE",
      price = helper.prices.offer1["offer1PriceTrial"],   -- price is not array
    },
    tasks = {}
  }
  print("data", json.encode(data))
  local res, err = validators.check_args(validators.properties_model, data)
  t.assert_not(res)
end