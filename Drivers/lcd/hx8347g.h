/* Orientation:
   - 0: 240x320 reset button in the top (portrait)
   - 1: 320x240 reset button in the left (landscape)
   - 2: 240x320 reset button in the bottom (portrait)
   - 3: 320x240 reset button in the right (landscape) */
#define  HX8347G_ORIENTATION       0

/* Color mode
   - 0: RGB565 (R:bit15..11, G:bit10..5, B:bit4..0)
   - 1: BRG565 (B:bit15..11, G:bit10..5, R:bit4..0) */
#define  HX8347G_COLORMODE         0

/* Draw and read bitdeph (16: RGB565, 24: RGB888)
   note: my paralell 8bit HX8347G LCD can write in 16 and 24 bit depth, can read only in 24 bit depth */
#define  HX8347G_WRITEBITDEPTH     16
#define  HX8347G_READBITDEPTH      24

/* Physical resolution in default orientation */
#define  HX8347G_LCD_PIXEL_WIDTH   240
#define  HX8347G_LCD_PIXEL_HEIGHT  320
