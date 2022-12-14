/* LCD interface type
   - 0: SPI half duplex (the mosi pin is bidirectional mode)
   - 1: SPI full duplex (write = mosi pin, read = miso pin)
   - 2: paralell 8 bit interface */
#define ILI9488_INTERFACE     1

/* Orientation:
   - 0: 320x480 micro-sd in the top (portrait)
   - 1: 480x320 micro-sd in the left (landscape)
   - 2: 320x480 micro-sd in the bottom (portrait)
   - 3: 480x320 micro-sd in the right (landscape) */
#define ILI9488_ORIENTATION   0

/* To clear the screen before display turning on ?
   - 0: does not clear
   - 1: clear */
#define ILI9488_INITCLEAR     1

/* Color order
   - 0: RGB
   - 1: BGR */
#define ILI9488_COLORMODE     1

/* Draw and read bitdeph (16: RGB565, 24: RGB888)
   note: my SPI ILI9488 LCD can only work in 24/24 bit depth
         my paralell 8 bit ILI9488 LCD can work in 16/16, 16/24, 24/16, 24/24 bit depth */
#define ILI9488_WRITEBITDEPTH 24
#define ILI9488_READBITDEPTH  24

/* ILI9488 Size (physical resolution in default orientation) */
#define  ILI9488_LCD_PIXEL_WIDTH   320
#define  ILI9488_LCD_PIXEL_HEIGHT  480
