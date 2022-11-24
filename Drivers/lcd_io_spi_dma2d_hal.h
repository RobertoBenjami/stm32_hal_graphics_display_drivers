//=============================================================================
/* Information section */

/*
 * SPI HAL LCD driver with DMA2D (stm32f4xx, stm32f7xx, stm32h7xx)
 * author: Roberto Benjami
 * v.2022.11
 *
 * When should you use this driver?
 *   If the display can only work in 24-bit (RGB888) mode in SPI mode,
 *   and your controller have a DMA2D peripheral.
 *   It can do the 16bit (RGB565) to 24bit (RGB888) conversion in the background 
     without using the processor (only at write).
 */

/* Features:
   - only hardware SPI and DMA2D with DMA
   - write function 8 and 16bit with DMA (in both fill and bitmap mode)
   - write function with bitdepth convert (16bit RGB565 to 24bit RGB888) with DMA2D
   - all writing functions are possible in both fill and bitmap mode
   - read function 8 and 16bit only without DMA
   - read function with bitdepth convert (24bit RGB888 to 16bit RGB565) only without DMA */

/* Settings in CUBEIDE or CUBEMX
   SPI Parameter Settings
   - Mode: "Full duplex master" or "Half duplex master" os "Transmit only master"
   - Hardware NSS signal: disabled
   - Frame format: Motorola
   - Data Size: 8 bits
   - First bit: MSB first
   - Clock Polarity: High
   - Clock Phase: 2 Edge
   SPI DMA Settings (not required)
   - Add DMA Request SPIn_TX, left at the default value
   GPIO
   - Lcd chip select name: LCD_CS
   - Lcd RS name: LCD_RS
   - Lcd SCK name: LCD_SCK
   - Lcd reset pin name: LCD_RST (not required)
   - Lcd black light pin name: LCD_BL (not required)
   DMA2D
   - Transfer Mode: Memory to Memory With Pixel Format Conversion
   - Color Mode: RGB888
   - DMA2D Input Color Mode: RGB565
   - DMA2D global interrupt enabled (check mark)
*/

//=============================================================================
/* Interface section */

/* Queries whether a DMA operation is in progress
   - 0: not in progress
   - 1: in progress */
uint32_t LCD_IO_DmaBusy(void);

/* If we want to know when the LCD DMA operation is finished, let's create a function in our program:
   This function is called by the driver at the end of DMA operations. */
void LCD_IO_DmaTxCpltCallback(SPI_HandleTypeDef *hspi);

//=============================================================================
/* Setting section (please set the necessary things in this section) */

#ifndef __LCD_IO_SPI_DMA2D_HAL_H__
#define __LCD_IO_SPI_DMA2D_HAL_H__

/* SPI handle select (see in main.c file, default: hspi1, hspi2 ... hspi6) */
#define LCD_SPI_HANDLE    hspi1

/* DMA2D handle select (see in main.c file, default: hdma2d) */
#define LCD_DMA2D_HANDLE  hdma2d

/* SPI mode
   - 0: only TX (write on MOSI pin, no MISO pin)
   - 1: half duplex (MOSI pin is bidirectional)
   - 2: full duplex (write on MOSI pin, read on MISO pin) */
#define LCD_SPI_MODE      0

/* SPI write and read speed (if deleted -> setting in CUBE)
   - hardware SPI clock div fPCLK: 0=/2, 1=/4, 2=/8, 3=/16, 4=/32, 5=/64, 6=/128, 7=/256 */
#define LCD_SPI_SPD_WRITE 2
#define LCD_SPI_SPD_READ  4

/* Backlight control (the logical level of the active state) */
#define LCD_BLON          0

/* When data direction change (OUT->IN) there is a display that requires extra clock
   example ST7735: 1, ILI9341: 0, ILI9488: 0 */
#define LCD_SCK_EXTRACLK  0

/* Waiting mode until the end of the previous DMA transaction
   - 0: while(dmastatus.txstatus);
   - 1: freertos (also works with freertos cmsis v1 and cmsis v2) */
#define LCD_DMA_WAITMODE  0

/* In dma mode the bitmap drawing function is completed before the actual drawing.
   When should we wait for the previous DMA operation to complete? (see the readme.me file)
   - 0: DMA check and wait at drawing function start
   - 1: DMA check and wait at drawing function start + bitmap drawing function end wait on (default mode)
   - 2: DMA wait at drawing function end */
#define LCD_DMA_ENDWAIT   1

/* Because there are DMA capable and DMA unable memory regions
   here we can set what is the DMA unable region condition
   note: where the condition is true, it is considered a DMA-unable region
   Default (all memory used for bitmap drawing is DMA capable):
     #define LCD_DMA_UNABLE(addr)  0
   Example stm32f429 with CCMRAM (the CCMRAM is not DMA capable):
     #define LCD_DMA_UNABLE(addr)  ((addr < 0x20000000) && (addr >= 0x10000000))
   Example stm32h743 (the DTCMRAM and ITCMRAM are not DMA capable):
     #define LCD_DMA_UNABLE(addr)  (((addr < 0x24000000) && (addr >= 0x20000000)) || (addr < 0x08000000))
   Note: if we ensure that we do not draw a bitmap from a DMA-capable memory area, it is not necessary to set it (leave it that way) */
#define LCD_DMA_UNABLE(addr)     0

/* RGB565 to RGB888 and RGB888 to RGB565 convert byte order 
   - 0: forward direction
   - 1: back direction
   (warning: the SPI DMA order is from low address to hight address step byte)
   note: DMA2D can only convert bitmaps in the forward direction on the f4 and f7 families.
         If the red and blue colors are reversed, change the LCD setting: e.g. ILI9341_COLORMODE. */
#define  LCD_IO_RGB24_ORDER      0

/* Buffer pixel number for DMA2D bitdepth conversion (byte = 3 * pixel number) */
#define LCD_DMA2D_BUFFERSIZE   512

#endif
