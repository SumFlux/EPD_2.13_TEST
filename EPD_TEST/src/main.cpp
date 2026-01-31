#include <Adafruit_GFX.h>
#include <Arduino.h>
#include <GxEPD2_BW.h>
#include <SPI.h>


/*
 * IL3897 (B72) Turbo Speed Edition
 * --------------------------------
 * 优化策略：
 * 1. [BUSY检测] 利用库函数自动等待硬件 BUSY 释放。
 * 2. [微秒级延时] BUSY 释放后只给 20ms 缓冲，取代之前的 200ms。
 * 3. [双重刷新] 坚持 "闪白 -> 写黑" 以保画质。
 *
 * 硬件连接 (ESP32-S3):
 *   MOSI  -> GPIO5
 *   SCLK  -> GPIO6
 *   CS    -> GPIO7
 *   DC    -> GPIO17
 *   RES   -> GPIO18
 *   BUSY  -> GPIO8
 */

// ==========================================
// 1. 硬件引脚定义
// ==========================================
#define PIN_MOSI 5
#define PIN_SCK 6
#define PIN_CS 7
#define PIN_DC 17
#define PIN_RST 18
#define PIN_BUSY 8
#define PIN_POWER_EN 47 // 电源使能引脚（如有需要）

// ==========================================
// 2. 驱动：IL3897 (B72) - 2.13寸墨水屏
// ==========================================
GxEPD2_BW<GxEPD2_213_B72, GxEPD2_213_B72::HEIGHT>
    display(GxEPD2_213_B72(PIN_CS, PIN_DC, PIN_RST, PIN_BUSY));

void setup() {
  // 1. 电源开启（如果硬件有电源使能引脚）
  pinMode(PIN_POWER_EN, OUTPUT);
  digitalWrite(PIN_POWER_EN, HIGH);
  delay(100);

  Serial.begin(115200);
  Serial.println("\n--- TURBO SPEED TEST ---");

  // 2. 初始化 SPI
  SPI.end();
  SPI.begin(PIN_SCK, -1, PIN_MOSI, PIN_CS);

  // 3. 初始化墨水屏
  display.init(115200, true, 2, false);
  display.setRotation(1);

  // ==========================================
  // 第一步：静默单次全刷（清屏）
  // ==========================================
  display.setFullWindow();
  display.firstPage();
  do {
    display.fillScreen(GxEPD_WHITE);
  } while (display.nextPage());

  // 给它一点启动时间
  delay(500);

  // ==========================================
  // 第二步：极速局刷 (带缓冲)
  // ==========================================
  Serial.println(">> Turbo Loop Start...");

  display.setTextSize(4);

  int x = 80;
  int y = 40;
  int w = 60;
  int h = 50;

  display.setPartialWindow(x, y, w, h);

  for (int i = 0; i < 50; i++) {

    // --- 阶段 A: 橡皮擦 (闪白) ---
    display.firstPage();
    do {
      display.fillScreen(GxEPD_WHITE);
    } while (display.nextPage());

    // 【关键优化 1】
    // nextPage() 返回意味着 BUSY 刚刚释放。
    // 我们只给 20ms 让电压稳定，防止指令堆积导致死机。
    // 这比之前的 200ms 极大提升了速度。
    delay(20);

    // --- 阶段 B: 写入 (显黑) ---
    display.firstPage();
    do {
      display.fillScreen(GxEPD_WHITE);
      display.drawRect(x, y, w, h, GxEPD_BLACK);
      display.setCursor(x + 15, y + 10);
      display.setTextColor(GxEPD_BLACK);
      display.print(i % 10);
    } while (display.nextPage());

    Serial.printf("Num: %d\n", i);

    // 【关键优化 2】
    // 再次给 20ms 缓冲，准备下一次循环。
    delay(20);
  }

  Serial.println(">> Done. Sleep.");
  display.hibernate();
}

void loop() {}
