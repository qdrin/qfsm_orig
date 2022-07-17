local t = require('luatest')
local g = t.group()
local json = require('json')
local uuid = require('uuid')
local fiber = require('fiber')
local qfsm = require('qfsm')
local clock = require('clock')

local helper = require('test.helper')

local load_config = {
  max_events = 10,
  max_retries = 200,
}

g.test_deactivation_started_wrong_price = function()
  if true then return end
  t.assert(false)
  local resp
  local mstate = helper.build_machine_state("resuming", "notPaid", "commercial")
  local res, err
  local props = {
    machine = mstate.machine,
    properties = {
      status = mstate.state,
    },
    tasks = {},
  }
  local expected_state = helper.build_machine_state("pendingDisconnect", "paymentFinal", "priceFinal")
  local expected_state2 = table.deepcopy(expected_state)
  expected_state2.price = {priceOff = 1}

  local wrong_count = 0
  local count = 0
  local t0 = clock.realtime()
  local channel = fiber.channel(load_config.max_events)
  for retry = 1, load_config.max_retries do
    for i = 1, load_config.max_events do
      local p = string.format("p%s_%s", retry, i)
      res, err = fiber.create(helper.send_event_fiberized, "deactivation_started", props, p, channel)
      t.assert(res, err)
      -- res:join()
    end
    -- if retry == 2 then t.assert(false) end
    -- fiber.sleep(1)
    print('retry', retry, 'reading channel')
    for evcount = 1, load_config.max_events do
    -- while not channel:is_empty() do
      resp = channel:get(0.05)
      t.assert(resp)
      count = count + 1
      -- print('resp', json.encode(resp))
      t.assert_equals(resp.machine.productProvision.usage, expected_state.machine.productProvision.usage)
      t.assert_equals(resp.machine.productProvision.payment, expected_state.machine.productProvision.payment)
      -- t.assert_equals(type(resp.machine.productProvision.price), 'table')
      if resp.machine.productProvision.price == 1 then
        print('got wrong price', json.encode(resp))
        wrong_count = wrong_count + 1
        t.assert(false)
      end
      t.assert_equals(wrong_count, 0)
    end
  end
  local t1 = clock.realtime()
  local dt = t1 - t0
  print(count, "processed in", dt, 'seconds', t0, t1)
  t.assert(false)
end
