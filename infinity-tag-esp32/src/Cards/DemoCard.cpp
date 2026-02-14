#include "Cards/DemoCard.h"
#include "Utils/ChineseFont.h"
#include "Utils/Logger.h"
#include <LittleFS.h>

// 演示文本（多行中文）
const char* DemoCard::DEMO_TEXTS[] = {
    "欢迎使用墨水屏",
    "这是第二行文字",
    "支持中文显示",
    "可以添加背景图",
    "按下按钮切换",
    "旋转编码器滚动",
    "Framebuffer系统",
    "图层合成技术"
};

const int DemoCard::DEMO_TEXT_COUNT = 8;

DemoCard::DemoCard()
    : _lineIndex(0), _needsRefresh(true), _useBackground(false) {
}

void DemoCard::onEnter() {
    LOG_DEBUG("[DemoCard] Entering demo card");
    _lineIndex = 0;
    _needsRefresh = true;
    _useBackground = false;
}

void DemoCard::onExit() {
    LOG_DEBUG("[DemoCard] Exiting demo card");
}

void DemoCard::onEvent(const Event& event) {
    switch (event.type) {
        case EVENT_BUTTON_RELEASE:
            // 短按：切换背景图片开关
            _useBackground = !_useBackground;
            _needsRefresh = true;
            LOG_PRINTF("[DemoCard] Background: %s\n", _useBackground ? "ON" : "OFF");
            break;

        case EVENT_ENCODER_ROTATE:
            // 旋转编码器：滚动显示行数
            _lineIndex += event.value;
            if (_lineIndex < 0) {
                _lineIndex = 0;
            } else if (_lineIndex > DEMO_TEXT_COUNT - 1) {
                _lineIndex = DEMO_TEXT_COUNT - 1;
            }
            _needsRefresh = true;
            LOG_PRINTF("[DemoCard] Line index: %d\n", _lineIndex);
            break;

        default:
            break;
    }
}

void DemoCard::renderToLayers(LayerManager& layerMgr) {
    // 1. 创建背景层
    auto bgLayer = std::make_shared<BackgroundLayer>(Framebuffer::WHITE);

    // 如果启用背景图片，尝试加载
    if (_useBackground) {
        // 背景图片路径（需要提前准备）
        const char* bgPath = "/images/demo_bg.bin";

        if (LittleFS.exists(bgPath)) {
            File file = LittleFS.open(bgPath, "r");
            if (file) {
                size_t fileSize = file.size();
                // 背景图片应该是 104×212 像素，1位格式 = 2756 字节
                if (fileSize == Framebuffer::BUFFER_SIZE) {
                    uint8_t* bgData = (uint8_t*)malloc(fileSize);
                    if (bgData) {
                        file.read(bgData, fileSize);
                        bgLayer->setImage(bgData, Framebuffer::WIDTH, Framebuffer::HEIGHT);
                        free(bgData);
                        LOG_DEBUG("[DemoCard] Background image loaded");
                    }
                } else {
                    LOG_PRINTF("[DemoCard] Invalid background size: %d (expected %d)\n",
                               fileSize, Framebuffer::BUFFER_SIZE);
                }
                file.close();
            }
        } else {
            LOG_DEBUG("[DemoCard] Background image not found, using white");
        }
    }

    layerMgr.addLayer(bgLayer);

    // 2. 创建文字层
    auto textLayer = std::make_shared<TextLayer>();

    // 计算显示范围（一屏最多显示 10 行，每行 20px 高）
    const int LINE_HEIGHT = 20;
    const int MAX_VISIBLE_LINES = 10;
    const int START_Y = 10;

    int startLine = _lineIndex;
    int endLine = std::min(_lineIndex + MAX_VISIBLE_LINES, DEMO_TEXT_COUNT);

    // 绘制可见的文本行
    for (int i = startLine; i < endLine; i++) {
        int y = START_Y + (i - startLine) * LINE_HEIGHT;

        // 计算文字居中位置
        int16_t textWidth = ChineseFont::getStringWidth(DEMO_TEXTS[i]);
        int16_t x = (Framebuffer::WIDTH - textWidth) / 2;

        // 添加文字到图层
        textLayer->addText(x, y, DEMO_TEXTS[i], Framebuffer::BLACK);
    }

    layerMgr.addLayer(textLayer);

    // 3. 添加滚动指示器（如果有更多内容）
    if (DEMO_TEXT_COUNT > MAX_VISIBLE_LINES) {
        // 创建一个简单的内容层来绘制箭头
        class ArrowLayer : public Layer {
        public:
            ArrowLayer(int lineIndex, int maxLines, int totalLines)
                : Layer(LayerType::CONTENT, 5),
                  _lineIndex(lineIndex),
                  _maxLines(maxLines),
                  _totalLines(totalLines) {}

            void render() override {
                _framebuffer.clear(Framebuffer::WHITE);

                // 绘制上箭头（如果不在顶部）
                if (_lineIndex > 0) {
                    int arrowX = Framebuffer::WIDTH - 10;
                    int arrowY = 5;
                    for (int i = 0; i < 5; i++) {
                        for (int j = -i; j <= i; j++) {
                            _framebuffer.setPixel(arrowX + j, arrowY + i, Framebuffer::BLACK);
                        }
                    }
                }

                // 绘制下箭头（如果不在底部）
                if (_lineIndex + _maxLines < _totalLines) {
                    int arrowX = Framebuffer::WIDTH - 10;
                    int arrowY = Framebuffer::HEIGHT - 10;
                    for (int i = 0; i < 5; i++) {
                        for (int j = -i; j <= i; j++) {
                            _framebuffer.setPixel(arrowX + j, arrowY - i, Framebuffer::BLACK);
                        }
                    }
                }
            }

        private:
            int _lineIndex;
            int _maxLines;
            int _totalLines;
        };

        auto indicatorLayer = std::make_shared<ArrowLayer>(
            _lineIndex, MAX_VISIBLE_LINES, DEMO_TEXT_COUNT
        );
        layerMgr.addLayer(indicatorLayer);
    }

    // 4. 添加提示文字层（底部）
    auto hintLayer = std::make_shared<TextLayer>();
    String hint = _useBackground ? "背景:开" : "背景:关";
    int16_t hintWidth = ChineseFont::getStringWidth(hint);
    int16_t hintX = (Framebuffer::WIDTH - hintWidth) / 2;
    hintLayer->addText(hintX, Framebuffer::HEIGHT - 20, hint, Framebuffer::BLACK);
    layerMgr.addLayer(hintLayer);

    _needsRefresh = false;
}

RefreshMode DemoCard::getRefreshMode() const {
    // 切换背景时使用全屏刷新，滚动时使用局部刷新
    if (_useBackground) {
        return RefreshMode::FULL;
    } else {
        return RefreshMode::PARTIAL;
    }
}
