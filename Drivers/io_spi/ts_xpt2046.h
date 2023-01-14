/*
 * XPT2046 touch driver
 * author: Roberto Benjami
 * v.2023.01
 */

//=============================================================================
/* Setting section (please set the necessary things in this section) */

/* SPI handle select
   - software SPI (set this macro value: -1, set in CUBEMX the TS_CS, TS_SCK, TS_MISO, TS_MOSI, TS_IRQ pin)
   - hardware SPI handle: see in main.c file (default: hspi1, hspi2 ... hspi6) */
#define TS_SPI_HANDLE         hspi1

/* SPI write and read speed (if deleted -> setting in CUBE)
   - software SPI: TS_SCK clock delay (see the TS_IO_Delay function)
   - hardware SPI: clock div fPCLK: 0=/2, 1=/4, 2=/8, 3=/16, 4=/32, 5=/64, 6=/128, 7=/256 */
#define TS_SPI_SPD            4

/* Wait time before reading xpt2046 (see: TS_IO_Transaction and TS_IO_Delay dunctions) */
#define XPT2046_READDELAY     0
