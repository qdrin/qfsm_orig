local checks = require('checks')
local fiber = require('fiber')
local icu_date = require('icu-date')
local json = require('json')
local log = require('log')

local utils = {}

local default_timezone = 'GMT'

local function date2unix(d_str)
  if type(d_str) ~= 'string' then return d_str end
  -- todo move gsub from here to inventory.select_pg
  d_str = d_str:gsub("(%d%d)( )(%d%d:)", "%1T%3")
  local date = icu_date.new()
  local format = icu_date.formats.iso8601()

  local md = '(%d+%-%d+%-%d+)(.*)'
  local mt = '(T%d+:%d+:%d+%.*%d*)(.*)'
  local mz = '[%-%+Z]%d+:%d+$'
  local tail, date_s, time_s

  date_s, tail = d_str:match(md)
  tail = tail or ''
  time_s, tail = tail:match(mt)
  tail = tail or ''
  local tz_s = tail:match(mz)
  if tz_s == 'Z' then tz_s = 'GMT'
  else tz_s = (tz_s and 'GMT' .. tz_s) or default_timezone
  end
  date:set_time_zone_id(tz_s)
  time_s = time_s or 'T00:00:00.000'
  time_s = time_s:gsub("%..*", "") .. '.000'
  local date_formatted = string.format('%s%s', date_s, time_s)
  date:parse(format, date_formatted)
  return (date:get_millis()/1000)
end

local function unix2date(u_time, tz)
  if type(u_time) ~= 'number' then return u_time end
  local date = icu_date.new()
  tz = tz or default_timezone
  date:set_time_zone_id(tz)
  local format = icu_date.formats.iso8601()
  date:set_millis(u_time*1000)

  return (date:format(format))
end

local function addperiod(start_date, period, sign)
  local start = start_date or fiber.time()
  sign = sign or 1
  local fields = {
    date={
      Y='YEAR',
      M='MONTH',
      D='DAY_OF_MONTH',
      W='WEEK',
    },
    time={
      H='HOUR',
      M='MINUTE',
      S='SECOND',
    }
  }
  local fmt = icu_date.formats.iso8601()
  local dper, tper
  if period:find('T') then
    dper, tper = period:match('P(.*)T(.*)')
  else
    dper = period:match('P(.*)')
    tper = ""
  end
  local date = icu_date:new()
  date:set_millis(start*1000)
  for num, typ in dper:gmatch('([%d]+)([Y,M,D,W])') do
    date:add(icu_date.fields[fields.date[typ]], sign*tonumber(num))
  end
  for num, typ in tper:gmatch('([%d]+)([H,M,S])') do
    date:add(icu_date.fields[fields.time[typ]], sign*tonumber(num))
  end
  return (date:get_millis()/1000)
end

local function addmonths(start, months)
  local date = icu_date.new()
  date:set_millis(start*1000)
  date:add(icu_date.fields.MONTH, months)
  return (date:get_millis()/1000)
end

local function merge_maps(new, current)
  checks('table', 'table')
  local res = current
  for n, v in pairs(new) do
    if type(current[n]) == 'table' and type(v) == 'table' then
      res[n] = merge_maps(v, current[n])
    else res[n] = v
    end
  end
  return res
end

local function string2table(count)
  count = count or 2
  return function(param)
    local spl = param:split(",")
    if #spl ~= count then return nil, "id count mismatch" end
    local t = {}
    for _, s in pairs(spl) do
      table.insert(t,s)
    end
    return t
  end
end

--Парсим реквест по единой схеме. Все, чего нет - nil
local function parse_model(process_order, source, target)
  -- process_order: array of tables like {'source_key', function(key) return source_key_data[key] end}
  log.verbose('Start parse_model')
  local missed = {}  -- отстутствующие параметры
  local defaulted = {}  -- параметры, установленные по умолчанию
  source = source or {}
  local tgt = table.deepcopy(target)
  local result = {}
  local par_current, t_par, path, path_from, fld
  local arr_name, arr_ind -- Name of attribute with array type and it's index (from 1)

  for _, order in pairs(process_order) do
    if source[order[1]] then
      -- source[order[1]] - соответствующая коллекция параметров запроса
      -- par = the parameter name in params table
      -- path == opts.map: defines where to put param
      -- path_from == opts.from or par: defines from where the value is retrieved
      for par, opts in pairs(source[order[1]]) do
        opts = opts or {}
        t_par = tgt
        path = opts.map or par
        path_from = opts.from or par
        fld = par
        for _, v in pairs(path:split('.')) do
          arr_name, arr_ind = v:match('(.*)%[(.*)%]')
          if arr_name then
            arr_ind = tonumber(arr_ind) or #t_par + 1
            t_par = t_par[arr_name][arr_ind]
          elseif type(t_par[v]) == 'table' then t_par = t_par[v]
          else fld = v
          end
        end
        -- если параметр еще не был вытянут раньше или был установлен по дефолту
        if t_par[fld] and not defaulted[path] then
          log.warn(string.format('%s param is already processed', path))
        else
          par_current = order[2](path_from)
          missed[path] = nil
          if par_current ~= nil then
            t_par[fld] = par_current
            defaulted[path] = nil
          elseif opts.default then
            par_current = opts.default
            defaulted[path] = true
          elseif opts.mandatory then
            missed[path] = "Missed"
          end
          if par_current and opts.convert_func then par_current = opts.convert_func(par_current) end
          if par_current and opts.type and type(par_current) ~= opts.type then
            missed[path] = string.format("Parameter '%s' has type '%s', expected '%s'", path, type(par_current), opts.type)
          end
          t_par[fld] = par_current
        end
      end
    else
      log.verbose(string.format('No request params awaiting from %s', order[1]))
    end
  end

  if next(missed) then
    result.missed_params = missed
  end
  result.parsed = tgt
  log.verbose('parse_model finish')
  return result
end

local function remove_empty_lists (items, field_list)  -- Превращаем пустые массивы в null
  for _, v in pairs(items or {}) do
    for _, f in pairs(field_list) do
      if v[f] and v[f] ~= box.NULL and #v[f] == 0 then v[f] = nil end
    end
  end
  return items
end

local function restore_empty_lists(items, field_list)
  for _, v in pairs(items or {}) do
    for _, f in pairs(field_list) do
      if not v[f] or v[f] == box.NULL then v[f] = {} end
    end
  end
  return items
end

utils.date2unix = date2unix
utils.unix2date = unix2date
utils.addmonths = addmonths
utils.addperiod = addperiod
utils.merge_maps = merge_maps
utils.string2table = string2table
utils.parse_model = parse_model
utils.remove_empty_lists = remove_empty_lists
utils.restore_empty_lists = restore_empty_lists
return utils
