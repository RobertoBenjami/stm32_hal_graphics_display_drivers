/* Orientation:
   - 0: 240x320 reset button in the top (portrait)
   - 1: 320x240 reset button in the left (landscape)
   - 2: 240x320 reset button in the bottom (portrait)
   - 3: 320x240 reset button in the right (landscape) */
#define  ILI9325_ORIENTATION       0

/* Color mode
   - 0: RGB565 (R:bit15..11, G:bit10..5, B:bit4..0)
   - 1: BRG565 (B:bit15..11, G:bit10..5, R:bit4..0) */
#define  ILI9325_COLORMODE         0

/* Draw and read bitdeph (16: RGB565, 24: RGB888)
   note: my paralell 8bit ILI9325 LCD only work in 16/16 bit depth */
#define  ILI9325_WRITEBITDEPTH     16
#define  ILI9325_READBITDEPTH      16

/* Physical resolution in default orientation */
#define  ILI9325_LCD_PIXEL_WIDTH   240
#define  ILI9325_LCD_PIXEL_HEIGHT  320
