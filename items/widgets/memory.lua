local icons = require("icons")
local colors = require("colors")
local settings = require("settings")

-- Execute the event provider binary which provides the event "memory_load" for
-- the memory usage data, which is fired every 2.0 seconds.
sbar.exec("killall memory_load >/dev/null; $CONFIG_DIR/helpers/event_providers/memory_load/bin/memory_load memory_load 2.0")

local memory = sbar.add("graph", "widgets.memory" , 42, {
  position = "right",
  graph = { color = colors.green },
  background = {
    height = 22,
    color = { alpha = 0 },
    border_color = { alpha = 0 },
    drawing = true,
  },
  icon = { string = icons.memory },
  label = {
    string = "mem ??%",
    font = {
      family = settings.font.numbers,
      style = settings.font.style_map["Bold"],
      size = 9.0,
    },
    align = "right",
    padding_right = 0,
    width = 0,
    y_offset = 4
  },
  padding_right = settings.paddings + 6
})

memory:subscribe("memory_load", function(env)
  -- Also available: env.used_memory, env.free_memory, env.memory_pressure
  local used_percentage = tonumber(env.used_percentage) or 0
  memory:push({ used_percentage / 100. })

  local color = colors.green
  if used_percentage > 60 then
    if used_percentage < 80 then
      color = colors.yellow
    elseif used_percentage < 90 then
      color = colors.orange
    else
      color = colors.red
    end
  end

  memory:set({
    graph = { color = color },
    label = "mem " .. env.used_percentage .. "%",
  })
end)

memory:subscribe("mouse.clicked", function(env)
  sbar.exec("open -a 'Activity Monitor'")
end)

-- Background around the memory item
sbar.add("bracket", "widgets.memory.bracket", { memory.name }, {
  background = { color = colors.bg1 }
})

-- Background around the memory item
sbar.add("item", "widgets.memory.padding", {
  position = "right",
  width = settings.group_paddings
})

return memory
