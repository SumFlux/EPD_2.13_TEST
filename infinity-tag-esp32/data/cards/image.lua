-- 元数据
CARD_NAME = "图片"
CARD_CATEGORY = "展示"
CARD_LOGO = "/icons/card_image.bin"
CARD_ORDER = 1
CARD_ENABLED = true

-- 状态
local imageUrl = "https://api.infinitytag.app/images/default.bin"
local imageLoaded = false

function onInit()
    sys.log("Image card initialized")
    eink.clear()
    eink.drawStr(10, 50, "Loading image...")
    eink.refreshPartial()

    -- 下载图片
    local bitmap = http.downloadBitmap(imageUrl)
    if bitmap then
        imageLoaded = true
        sys.log("Image loaded successfully")
        eink.refreshDeep()  -- 使用DEEP刷新显示图片
    else
        eink.clear()
        eink.drawStr(10, 50, "Failed to load")
        eink.refreshDeep()
        sys.log("Failed to load image")
    end
end

function onExit()
    sys.log("Image card exited")
end

function onLoop()
    -- 图片卡片不需要循环更新
end

function onBtnPress()
    sys.log("Button pressed - reloading image")
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
    sys.log("Device shaken")
end
