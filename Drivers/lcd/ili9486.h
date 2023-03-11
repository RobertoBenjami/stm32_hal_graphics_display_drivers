/* Orientation
   - 0: 240x320 portrait 0'
   - 1: 320x240 landscape 90'
   - 2: 240x320 portrait 180'
   - 3: 320x240 landscape 270'
*/
#define ILI9486_ORIENTATION       0

/* To clear the screen before display turning on ?
   - 0: does not clear
   - 1: clear */
#define ILI9486_INITCLEAR         0

/* Color mode
   - 0: RGB565 (R:bit15..11, G:bit10..5, B:bit4..0)
   - 1: BRG565 (B:bit15..11, G:bit10..5, R:bit4..0)
*/
#define  ILI9486_COLORMODE        0

/* Draw and read bitdeph (16: RGB565, 24: RGB888) */
#define ILI9486_WRITEBITDEPTH     16
#define ILI9486_READBITDEPTH      16

/* ILI9486 physic resolution in 0 orientation */
#define  ILI9486_LCD_PIXEL_WIDTH  320
#define  ILI9486_LCD_PIXEL_HEIGHT 480
