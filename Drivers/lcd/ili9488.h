/* LCD SPI half duplex / full duplex mode
   - 0: half duplex (the mosi pin is bidirectional mode)
   - 1: full duplex (write = mosi pin, read = miso pin) */
#define  ILI9488_SPIMODE      1

/* Orientation:
   - 0: 320x480 micro-sd in the top (portrait)
   - 1: 480x320 micro-sd in the left (landscape)
   - 2: 320x480 micro-sd in the bottom (portrait)
   - 3: 480x320 micro-sd in the right (landscape) */
#define  ILI9488_ORIENTATION  0

/* To clear the screen before display turning on ?
   - 0: does not clear
   - 1: clear */
#define  ILI9488_INITCLEAR    1

/* Color order
   - 0: RGB
   - 1: BGR */
#define  ILI9488_COLORMODE    0
