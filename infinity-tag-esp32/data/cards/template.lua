-- 元数据
CARD_NAME = "模板"
CARD_CATEGORY = "其他"
CARD_LOGO = "/icons/card_default.bin"
CARD_ORDER = 100
CARD_ENABLED = true

-- 状态变量
local counter = 0
local lastUpdate = 0

-- 初始化
function onInit()
    sys.log("Template card initialized")
    counter = 0

    -- 尝试从NVS加载上次的计数
    local saved = nvs.get("template_counter")
    if saved and saved ~= "" then
        counter = tonumber(saved) or 0
        sys.log("Loaded counter from NVS: " .. counter)
    end

    sys.log("About to call _renderDeep()")
    _renderDeep()  -- 首次进入使用DEEP刷新
    sys.log("_renderDeep() completed")
end

-- 退出
function onExit()
    sys.log("Template card exited")

    -- 保存计数到NVS
    nvs.set("template_counter", tostring(counter))
    sys.log("Saved counter to NVS: " .. counter)
end

-- 主循环（限制10Hz调用）
function onLoop()
    -- 可选：定期更新界面
    local now = sys.millis()
    if now - lastUpdate > 5000 then  -- 每5秒更新一次
        lastUpdate = now
        -- 这里可以添加定期更新逻辑
    end
end

-- 按键事件
function onBtnPress()
    counter = counter + 1
    sys.log("Button pressed - counter: " .. counter)
    _render()
end

function onBtnLong()
    counter = 0
    sys.log("Long press - counter reset")
    _render()
end

-- 编码器事件
function onEncoderCW()
    counter = counter + 1
    sys.log("Encoder CW - counter: " .. counter)
    _render()
end

function onEncoderCCW()
    counter = counter - 1
    sys.log("Encoder CCW - counter: " .. counter)
    _render()
end

-- 摇晃事件
function onShake()
    counter = counter + 10
    sys.log("Device shaken - counter: " .. counter)
    _render()
end

-- 渲染函数（DEEP刷新，用于首次进入）
function _renderDeep()
    sys.log("_renderDeep() started")
    eink.clear()
    sys.log("eink.clear() done")

    -- 绘制标题
    eink.drawStr(10, 10, "Lua Card Template")
    sys.log("Title drawn")

    -- 绘制计数器
    eink.drawStr(10, 30, "Counter: " .. counter)
    sys.log("Counter drawn")

    -- 绘制说明
    eink.drawStr(10, 50, "Press button to +1")
    eink.drawStr(10, 65, "Long press to reset")
    eink.drawStr(10, 80, "Rotate encoder +/-1")
    eink.drawStr(10, 95, "Shake to +10")
    sys.log("Instructions drawn")

    -- 使用深度刷新（最彻底）
    sys.log("About to call eink.refreshDeep()")
    eink.refreshDeep()
    sys.log("eink.refreshDeep() completed")
end

-- 渲染函数（局部刷新，用于更新）
function _render()
    eink.clear()

    -- 绘制标题
    eink.drawStr(10, 10, "Lua Card Template")

    -- 绘制计数器
    eink.drawStr(10, 30, "Counter: " .. counter)

    -- 绘制说明
    eink.drawStr(10, 50, "Press button to +1")
    eink.drawStr(10, 65, "Long press to reset")
    eink.drawStr(10, 80, "Rotate encoder +/-1")
    eink.drawStr(10, 95, "Shake to +10")

    -- 使用局部刷新（更快）
    eink.refreshPartial()
end
