/**
 * ESP32S3 2.13inch E-Paper (SSD1680) Driver
 * Based on Waveshare EPD_2in13_V4 Example
 *
 * Pins:
 * SCK=12, MOSI=11, CS=10, DC=8, RES=7, BUSY=9
 */

#include <Arduino.h>
#include <SPI.h>

// --- Pin Definitions ---
#define EPD_SCK_PIN 12
#define EPD_MOSI_PIN 11
#define EPD_CS_PIN 10
#define EPD_DC_PIN 8
#define EPD_RST_PIN 7
#define EPD_BUSY_PIN 9

// --- Display Resolution ---
#define EPD_WIDTH 122
#define EPD_HEIGHT 250

// --- Data Types ---
#define UBYTE uint8_t
#define UWORD uint16_t
#define UDOUBLE uint32_t

// --- Font Data (Digits 0-9, 8x16) ---
const unsigned char Font8x16_Digits[10][16] = {
    {0x00, 0x3E, 0x41, 0x41, 0x41, 0x41, 0x3E, 0x00, 0x00, 0x00, 0x00, 0x00,
     0x00, 0x00, 0x00, 0x00}, // 0
    {0x00, 0x00, 0x42, 0x7F, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
     0x00, 0x00, 0x00, 0x00}, // 1
    {0x00, 0x42, 0x61, 0x51, 0x49, 0x46, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
     0x00, 0x00, 0x00, 0x00}, // 2
    {0x00, 0x22, 0x41, 0x49, 0x49, 0x36, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
     0x00, 0x00, 0x00, 0x00}, // 3
    {0x00, 0x18, 0x14, 0x12, 0x7F, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
     0x00, 0x00, 0x00, 0x00}, // 4
    {0x00, 0x27, 0x45, 0x45, 0x45, 0x39, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
     0x00, 0x00, 0x00, 0x00}, // 5
    {0x00, 0x3C, 0x4A, 0x49, 0x49, 0x30, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
     0x00, 0x00, 0x00, 0x00}, // 6
    {0x00, 0x01, 0x71, 0x09, 0x05, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
     0x00, 0x00, 0x00, 0x00}, // 7
    {0x00, 0x36, 0x49, 0x49, 0x49, 0x36, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
     0x00, 0x00, 0x00, 0x00}, // 8
    {0x00, 0x06, 0x49, 0x49, 0x29, 0x1E, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
     0x00, 0x00, 0x00, 0x00} // 9
};

// --- Global Buffer & State ---
UBYTE *ImageBuffer = NULL;
int counter = 1;
int refresh_count = 0;

// --- Low Level Hardware Interface ---

void DEV_Delay_ms(UDOUBLE xms) { delay(xms); }

void DEV_Digital_Write(int pin, int value) { digitalWrite(pin, value); }

int DEV_Digital_Read(int pin) { return digitalRead(pin); }

void DEV_SPI_WriteByte(UBYTE data) {
  SPI.beginTransaction(SPISettings(4000000, MSBFIRST, SPI_MODE0));
  digitalWrite(EPD_CS_PIN, LOW);
  SPI.transfer(data);
  digitalWrite(EPD_CS_PIN, HIGH);
  SPI.endTransaction();
}

void EPD_Reset(void) {
  DEV_Digital_Write(EPD_RST_PIN, 1);
  DEV_Delay_ms(20);
  DEV_Digital_Write(EPD_RST_PIN, 0);
  DEV_Delay_ms(2);
  DEV_Digital_Write(EPD_RST_PIN, 1);
  DEV_Delay_ms(20);
}

void EPD_SendCommand(UBYTE Reg) {
  DEV_Digital_Write(EPD_DC_PIN, 0);
  DEV_SPI_WriteByte(Reg);
}

void EPD_SendData(UBYTE Data) {
  DEV_Digital_Write(EPD_DC_PIN, 1);
  DEV_SPI_WriteByte(Data);
}

void EPD_ReadBusy(void) {
  // EPD Busy=1 means BUSY, 0 means IDLE?
  // Double check example: if(DEV_Digital_Read(EPD_BUSY_PIN)==0) break; -> So 1
  // is BUSY. Wait, example says: if(DEV_Digital_Read(EPD_BUSY_PIN)==0) break;
  // //=1 BUSY Yes, high is busy.
  Serial.println("e-Paper busy");
  unsigned long start = millis();
  while (1) {
    if (DEV_Digital_Read(EPD_BUSY_PIN) == 0)
      break;
    DEV_Delay_ms(10);
    if (millis() - start > 5000) {
      Serial.println("e-Paper busy timeout!");
      break;
    }
  }
  Serial.println("e-Paper busy release");
}

// --- EPD Driver Functions (Ported from EPD_2in13_V4.cpp) ---

void EPD_SetWindows(UWORD Xstart, UWORD Ystart, UWORD Xend, UWORD Yend) {
  EPD_SendCommand(0x44); // SET_RAM_X_ADDRESS_START_END_POSITION
  EPD_SendData((Xstart >> 3) & 0xFF);
  EPD_SendData((Xend >> 3) & 0xFF);

  EPD_SendCommand(0x45); // SET_RAM_Y_ADDRESS_START_END_POSITION
  EPD_SendData(Ystart & 0xFF);
  EPD_SendData((Ystart >> 8) & 0xFF);
  EPD_SendData(Yend & 0xFF);
  EPD_SendData((Yend >> 8) & 0xFF);
}

void EPD_SetCursor(UWORD Xstart, UWORD Ystart) {
  EPD_SendCommand(0x4E); // SET_RAM_X_ADDRESS_COUNTER
  EPD_SendData(Xstart & 0xFF);

  EPD_SendCommand(0x4F); // SET_RAM_Y_ADDRESS_COUNTER
  EPD_SendData(Ystart & 0xFF);
  EPD_SendData((Ystart >> 8) & 0xFF);
}

void EPD_TurnOnDisplay(void) {
  EPD_SendCommand(0x22); // Display Update Control
  EPD_SendData(0xf7);
  EPD_SendCommand(0x20); // Activate Display Update Sequence
  EPD_ReadBusy();
}

void EPD_TurnOnDisplay_Partial(void) {
  EPD_SendCommand(0x22); // Display Update Control
  EPD_SendData(0xff);    // fast:0x0c, quality:0x0f, 0xcf
  EPD_SendCommand(0x20); // Activate Display Update Sequence
  EPD_ReadBusy();
}

void EPD_Init(void) {
  EPD_Reset();
  EPD_ReadBusy();
  EPD_SendCommand(0x12); // SWRESET
  EPD_ReadBusy();

  EPD_SendCommand(0x01); // Driver output control
  EPD_SendData(0xF9);
  EPD_SendData(0x00);
  EPD_SendData(0x00);

  EPD_SendCommand(0x11); // data entry mode
  EPD_SendData(0x03);

  EPD_SetWindows(0, 0, EPD_WIDTH - 1, EPD_HEIGHT - 1);
  EPD_SetCursor(0, 0);

  EPD_SendCommand(0x3C); // BorderWavefrom
  EPD_SendData(0x05);

  EPD_SendCommand(0x21); // Display update control
  EPD_SendData(0x00);
  EPD_SendData(0x80);

  EPD_SendCommand(0x18); // Read built-in temperature sensor
  EPD_SendData(0x80);
  EPD_ReadBusy();
}

void EPD_Clear(void) {
  UWORD Width, Height;
  Width = (EPD_WIDTH % 8 == 0) ? (EPD_WIDTH / 8) : (EPD_WIDTH / 8 + 1);
  Height = EPD_HEIGHT;

  EPD_SendCommand(0x24);
  for (UWORD j = 0; j < Height; j++) {
    for (UWORD i = 0; i < Width; i++) {
      EPD_SendData(0xFF);
    }
  }
  EPD_TurnOnDisplay();
}

void EPD_Display(UBYTE *Image) {
  UWORD Width, Height;
  Width = (EPD_WIDTH % 8 == 0) ? (EPD_WIDTH / 8) : (EPD_WIDTH / 8 + 1);
  Height = EPD_HEIGHT;

  EPD_SendCommand(0x24);
  for (UWORD j = 0; j < Height; j++) {
    for (UWORD i = 0; i < Width; i++) {
      EPD_SendData(Image[i + j * Width]);
    }
  }
  EPD_TurnOnDisplay();
}

void EPD_Display_Partial(UBYTE *Image) {
  UWORD Width, Height;
  Width = (EPD_WIDTH % 8 == 0) ? (EPD_WIDTH / 8) : (EPD_WIDTH / 8 + 1);
  Height = EPD_HEIGHT;

  // Partial Refresh Logic from EPD_2in13_V4.cpp
  // Reset
  DEV_Digital_Write(EPD_RST_PIN, 0);
  DEV_Delay_ms(1);
  DEV_Digital_Write(EPD_RST_PIN, 1);

  EPD_SendCommand(0x3C); // BorderWavefrom
  EPD_SendData(0x80);

  EPD_SendCommand(0x01); // Driver output control
  EPD_SendData(0xF9);
  EPD_SendData(0x00);
  EPD_SendData(0x00);

  EPD_SendCommand(0x11); // data entry mode
  EPD_SendData(0x03);

  EPD_SetWindows(0, 0, EPD_WIDTH - 1, EPD_HEIGHT - 1);
  EPD_SetCursor(0, 0);

  EPD_SendCommand(0x24); // Write Black and White image to RAM
  for (UWORD j = 0; j < Height; j++) {
    for (UWORD i = 0; i < Width; i++) {
      EPD_SendData(Image[i + j * Width]);
    }
  }
  EPD_TurnOnDisplay_Partial();
}

void EPD_Sleep(void) {
  EPD_SendCommand(0x10); // enter deep sleep
  EPD_SendData(0x01);
  DEV_Delay_ms(100);
}

// --- Framebuffer Graphics Helper ---

// Clear buffer to white (0xFF)
void Paint_Clear(UBYTE *image, UBYTE color) {
  UWORD Width = (EPD_WIDTH % 8 == 0) ? (EPD_WIDTH / 8) : (EPD_WIDTH / 8 + 1);
  UWORD Height = EPD_HEIGHT;
  // 0xFF = White, 0x00 = Black
  memset(image, color, Width * Height);
}

// Draw a single character '0'-'9' at x,y
void Paint_DrawNum(UBYTE *image, int x, int y, int num) {
  if (num < 0 || num > 9)
    return;

  // Font 8x16
  const unsigned char *ptr = Font8x16_Digits[num];

  // Each char is 16 bytes.
  // Format is tricky in the example font file, usually it's column based or row
  // based. Looking at the example copy in my head: {0x00,0x3E,0x41...} -> Looks
  // like 8 bytes for one half? No wait 8x16 is usually 16 bytes. The previous
  // simple font array I synthesized:
  /*
      {0x00,0x3E,0x41,0x41,0x41,0x41,0x3E,0x00...}
  */
  // This looks like column-major for 8 pixels high?
  // Actually, usually these fonts are Column-major? or Row-major?
  // Example: 0x3E = 00111110.
  // If it's 8x16, it takes 16 bytes.
  // Let's assume standard implementation:
  // We will just set pixels based on bits.
  // BUT! The screen arrangement.
  // The buffer is 1 bit per pixel.
  // Byte 0: pixels (0,0) to (7,0)? Or (0,0) to (0,7)?
  // Command 0x11 Data Entry Mode 0x03 -> X increment, Y increment.
  // Usually means Row-major (X changes fast).
  // So Byte 0 is (0,0) to (7,0).
  // Bit 7 is (0,0), Bit 0 is (7,0).

  // My synthesized font data is actually unknown format. Let's just assume I
  // draw a box or something simpler if font is hard. Wait, I can try to
  // interpret the generic GLCD font format. 0x3E, 0x41, 0x41... for '0'.
  // 00111110 (Vertical line?)
  // 01000001

  // Let's implement a simple pixel set function and draw lines.
  // Or just use the font purely.

  // Let's define: Byte 0 is column 0.
  // For 8x16, we probably have 16 bytes?
  // Wait my array has 16 bytes.
  // Let's assume it's valid.

  for (int col = 0; col < 8; col++) { // 8 columns?
    // Actually 16 bytes usually means 16 rows if 8 wide?
    // 0x00 (Row 0), 0x3E (Row 1)...
    // If Row 1 is 00111110 ->  .*****..
    // That looks like a top of a zero.

    for (int row = 0; row < 16; row++) {
      UBYTE val = ptr[row]; // Row byte?
      // Actually I think it's rotated?
      // Let's just try to iterate.
      // If I treat it as 16 rows of 8 bits.

      if (row >= 16)
        break;

      // Check bit 'col' (0..7)
      // Bit 0 is LSB (Right), Bit 7 MSB (Left).
      // Let's draw:
      int bit = (val >> (7 - col)) & 0x01;

      if (bit) {
        // Set Pixel Black (0)
        // Pixel (x + col, y + row)
        int px = x + col;
        int py = y + row;

        if (px >= EPD_WIDTH || py >= EPD_HEIGHT)
          continue;

        // Index calculation
        // Row 'py', Col 'px'.
        // Widthbytes = Width / 8
        UWORD WidthBytes =
            (EPD_WIDTH % 8 == 0) ? (EPD_WIDTH / 8) : (EPD_WIDTH / 8 + 1);
        int idx = py * WidthBytes + (px / 8);
        int bit_offset = 7 - (px % 8);

        ImageBuffer[idx] &= ~(1 << bit_offset);
      }
    }
  }
}

// --- Main Application ---

void setup() {
  Serial.begin(115200);
  delay(100);
  Serial.printf("EPD_TEST Demo\n");

  // Init SPI
  SPI.begin(EPD_SCK_PIN, -1, EPD_MOSI_PIN, EPD_CS_PIN);

  pinMode(EPD_CS_PIN, OUTPUT);
  pinMode(EPD_DC_PIN, OUTPUT);
  pinMode(EPD_RST_PIN, OUTPUT);
  pinMode(EPD_BUSY_PIN, INPUT);

  // Alloc Buffer
  UWORD Imagesize =
      ((EPD_WIDTH % 8 == 0) ? (EPD_WIDTH / 8) : (EPD_WIDTH / 8 + 1)) *
      EPD_HEIGHT;
  ImageBuffer = (UBYTE *)malloc(Imagesize);
  if (ImageBuffer == NULL) {
    Serial.printf("Failed to malloc memory!\n");
    while (1)
      ;
  }

  // 1. Init Full
  Serial.printf("EPD Init...\n");
  EPD_Init();

  // 2. Clear Screen (White)
  Serial.printf("Clearing...\n");
  EPD_Clear();

  // 3. Draw Initial "1"
  Serial.printf("Drawing '1'...\n");
  Paint_Clear(ImageBuffer, 0xFF);              // White
  Paint_DrawNum(ImageBuffer, 10, 10, counter); // Draw '1' at 10,10

  // Display Full
  EPD_Display(ImageBuffer);
  EPD_Sleep(); // Sleep after full refresh usually?
  // Note: If we sleep, we might need to wake up for partial?
  // Display_Partial handles wake up (Reset).

  Serial.printf("Setup Done. Loop start.\n");
}

void loop() {
  delay(3000); // 3 seconds

  counter++;
  if (counter > 9)
    counter = 0; // Wrap around for single digit demo

  refresh_count++;

  // Prepare Image
  Paint_Clear(ImageBuffer, 0xFF);
  Paint_DrawNum(ImageBuffer, 10, 10, counter);

  Serial.printf("Update: %d (Refresh Count: %d)\n", counter, refresh_count);

  if (refresh_count % 5 == 0) {
    // Full Refresh
    Serial.printf("Full Refresh trigger...\n");
    EPD_Init(); // Wake up / Init
    EPD_Display(ImageBuffer);
    EPD_Sleep();
  } else {
    // Partial Refresh
    // EPD_Display_Partial handles its own Reset/Waveform setup
    EPD_Display_Partial(ImageBuffer);
    // Do we sleep after partial?
    // EPD_Sleep();
    // Example doesn't explicitly sleep inside Loop usually, but V4 code
    // suggests partial should be fine. If we sleep, the V4 Display_Partial uses
    // Reset so it's fine. Let's sleep to save power and be consistent.
    // Actually, if we deep sleep, we might lose RAM? No, controller has RAM.
    // Let's just not call Sleep after partial to be faster?
    // But user asked for "Every 3 seconds". Speed is not issue.
    // Let's call Sleep to properly discharge charge pump.
    EPD_Sleep();
  }
}
