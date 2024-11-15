/* Orientation
   - 0: 240x320 portrait (plug in top)
   - 1: 320x240 landscape (plug in left)
   - 2: 240x320 portrait (plug in botton)
   - 3: 320x240 landscape (plug in right) */
#define  ST7789_ORIENTATION             0

/* To clear the screen before display turning on ?
   - 0: does not clear
   - 1: clear */
#define  ST7789_INITCLEAR               1

/* Color order (0 = RGB, 1 = BGR) */
#define  ST7789_COLORMODE               0

/* Draw and read bitdeph (16: RGB565, 24: RGB888) 
   note: my SPI ST7789 LCD only readable if ST7789_READBITDEPTH 24 */
#define  ST7789_WRITEBITDEPTH           16
#define  ST7789_READBITDEPTH            24

// ILI9341 physic resolution (in 0 orientation)
#define  ST7789_LCD_PIXEL_WIDTH         240
#define  ST7789_LCD_PIXEL_HEIGHT        320
