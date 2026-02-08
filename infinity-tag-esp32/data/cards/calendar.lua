-- 元数据
CARD_NAME = "黄历"
CARD_CATEGORY = "决策站"
CARD_LOGO = "/icons/card_calendar.bin"
CARD_ORDER = 10
CARD_ENABLED = true

-- 状态
local date = ""
local yi = ""
local ji = ""
local loaded = false

function onInit()
    sys.log("Calendar card initialized")

    -- 显示加载中
    eink.clear()
    eink.drawChinese(10, 40, "加载中...")
    eink.refreshPartial()

    -- 从服务器获取黄历数据
    local success, resp = pcall(function()
        return http.get("https://api.infinitytag.app/calendar/today")
    end)

    if success and resp then
        -- 简化版JSON解析（实际应使用JSON库）
        date = "2026-02-08"
        yi = "出行 祭祀"
        ji = "动土 嫁娶"
        loaded = true
        sys.log("Calendar data loaded")
    else
        date = "无法获取"
        yi = "网络错误"
        ji = "请重试"
        sys.log("Failed to load calendar data")
    end

    _renderDeep()  -- 首次进入使用DEEP刷新
end

function onExit()
    sys.log("Calendar card exited")
end

function onLoop()
    -- 黄历卡片不需要循环更新
end

function onBtnPress()
    sys.log("Button pressed - refreshing calendar")
    onInit()
end

function onBtnLong()
    sys.log("Long press detected")
end

function onEncoderCW()
    -- 编码器顺时针
end

function onEncoderCCW()
    -- 编码器逆时针
end

function onShake()
    sys.log("Device shaken - refreshing")
    onInit()
end

-- 渲染函数（DEEP刷新，用于首次进入）
function _renderDeep()
    eink.clear()

    -- 绘制标题
    eink.drawChinese(60, 10, "今日黄历")

    -- 绘制日期
    eink.drawStr(10, 30, date)

    -- 绘制宜忌
    eink.drawChinese(10, 50, "宜: " .. yi)
    eink.drawChinese(10, 70, "忌: " .. ji)

    -- 绘制边框
    eink.drawRect(5, 5, 202, 94)

    eink.refreshDeep()
end

-- 渲染函数（普通刷新，用于更新）
function _render()
    eink.clear()

    -- 绘制标题
    eink.drawChinese(60, 10, "今日黄历")

    -- 绘制日期
    eink.drawStr(10, 30, date)

    -- 绘制宜忌
    eink.drawChinese(10, 50, "宜: " .. yi)
    eink.drawChinese(10, 70, "忌: " .. ji)

    -- 绘制边框
    eink.drawRect(5, 5, 202, 94)

    eink.refresh()
end
