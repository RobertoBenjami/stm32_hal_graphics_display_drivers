/* LCD SPI half duplex / full duplex mode
   - 0: half duplex (the mosi pin is bidirectional mode)
   - 1: full duplex (write = mosi pin, read = miso pin) */
#define  ST7735_SPIMODE           0

/* Orientation:
   - 0: 128x160 (portrait)
   - 1: 160x128 (landscape)
   - 2: 128x160 (portrait)
   - 3: 160x128 (landscape) */
#define  ST7735_ORIENTATION       0

/* Color mode
   - 0: RGB565 (R:bit15..11, G:bit10..5, B:bit4..0)
   - 1: BRG565 (B:bit15..11, G:bit10..5, R:bit4..0) */
#define  ST7735_COLORMODE         0   /* 0 = RGB, 1 = BGR */

/* To clear the screen before display turning on ?
   - 0: does not clear
   - 1: clear */
#define  ST7735_INITCLEAR         1

/* Draw and read bitdeph (16: RGB565, 24: RGB888)
   note: my ST7735 LCD only readable if ST7735_READBITDEPTH 24 */
#define  ST7735_WRITEBITDEPTH     16
#define  ST7735_READBITDEPTH      24

/* Physical resolution in default orientation */
#define  ST7735_LCD_PIXEL_WIDTH   128
#define  ST7735_LCD_PIXEL_HEIGHT  160
