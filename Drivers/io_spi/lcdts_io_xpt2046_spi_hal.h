//=============================================================================
/* Information section */

/*
 * SPI HAL LCD and TS driver for all stm32 family
 * author: Roberto Benjami
 * v.2022.12
 */

/* Features:
   - transaction-centric operation
   - only hardware SPI (SPI, MOSI and MISO pins are shared for lcd and touchscreen)
   - write function 8 and 16bit without DMA or with DMA (in both fill and bitmap mode)
   - write function with bitdepth convert (16bit RGB565 to 24bit RGB888) without DMA or with DMA
   - all writing functions are possible in both fill and bitmap mode
   - read function 8 and 16bit without DMA or with DMA
   - read function with bitdepth convert (24bit RGB888 to 16bit RGB565) without DMA or with DMA */

/* Settings in CUBEIDE or CUBEMX
   SPI Parameter Settings
   - Mode: "Full duplex master"
   - Hardware NSS signal: disabled
   - Frame format: Motorola
   - Data Size: 8 bits
   - First bit: MSB first
   - Prescaler: it doesn't matter, the speed needs to be specified elsewhere
   - Clock Polarity: Low
   - Clock Phase: 1 Edge
     note: if the selected SPI pin is not good, change it in the pinout view
   SPI DMA Settings (it's not neccessary)
   - Add DMA Request SPIn_TX, left at the default value (it's not neccessary)
   - Add DMA Request SPIn_RX, left at the default value (it's not neccessary)
   GPIO
   - Lcd chip select: 
     - output level: High 
     - mode: Output Push Pull
     - Pull-up/Pull-down: No pull-up and no pull-down
     - Max output speed: Very High
     - User Label: LCD_CS
   - Touchscreen chip select:
     - output level: High 
     - mode: Output Push Pull
     - Pull-up/Pull-down: No pull-up and no pull-down
     - Max output speed: Very High
     - User Label: TS_CS
   - Lcd RS
     - output level: Low 
     - mode: Output Push Pull
     - Pull-up/Pull-down: No pull-up and no pull-down
     - Max output speed: Very High
     - User Label: LCD_RS
   - Lcd and Touchscreen SCK
     - output level: n/a 
     - mode: Alternate Function Push Pull
     - Pull-up/Pull-down: No pull-up and no pull-down
     - Max output speed: Very High
     - User Label: LCDTS_SCK
   - Lcd and Touchscreen MOSI
     - output level: n/a 
     - mode: Alternate Function Push Pull
     - Pull-up/Pull-down: No pull-up and no pull-down
     - Max output speed: Very High
     - User Label: LCDTS_MOSI
   - Lcd and Touchscreen MISO
     - output level: n/a 
     - mode: Alternate Function Push Pull
     - Pull-up/Pull-down: No pull-up and no pull-down
     - Max output speed: Very High
     - User Label: LCDTS_MISO
   - Lcd reset pin (only when connected)
     - output level: High
     - mode: Output Push Pull
     - Pull-up/Pull-down: No pull-up and no pull-down
     - Max output speed: Low
     - User Label: LCD_RST
   - Lcd back light pin (only when connected)
     - output level: Low or High
     - mode: Output Push Pull
     - Pull-up/Pull-down: No pull-up and no pull-down
     - Max output speed: Low
     - User Label: LCD_BL
   - Touchscreen IRQ (only when connected)
     - output level: n/a 
     - mode: Input mode
     - Pull-up/Pull-down: No pull-up and no pull-down
     - Max output speed: n/a
     - User Label: TS_IRQ

   Settings in main.h:
   - If you use freertos, add this line the main.h file
     #include "cmsis_os.h"
     (note: then the driver will also use the rtos signal to wait for the end of the dma transaction)
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
void LCD_IO_DmaRxCpltCallback(SPI_HandleTypeDef *hspi);

//=============================================================================
/* Setting section (please set the necessary things in this section) */

#ifndef __LCDTS_IO_XPT2046_SPI_HAL_H__
#define __LCDTS_IO_XPT2046_SPI_HAL_H__

/* SPI handle select (see in main.c file, default: hspi1, hspi2 ... hspi6) */
#define LCDTS_SPI_HANDLE      hspi1

/* SPI mode
   - 0: only TX (write on MOSI pin, no MISO pin)
   - 1: half duplex (MOSI pin is bidirectional)
   - 2: full duplex (write on MOSI pin, read on MISO pin) */
#define LCD_SPI_MODE          2

/* SPI write and read speed (if deleted -> setting in CUBE)
   - hardware SPI clock div fPCLK: 0=/2, 1=/4, 2=/8, 3=/16, 4=/32, 5=/64, 6=/128, 7=/256 
     note: 
	 - on my LCDs the read speed is lower than the write speed
	 - on my LCDs the touchscreen speed is lower than the lcd speed */
#define LCD_SPI_SPD_WRITE     1
#define LCD_SPI_SPD_READ      4
#define TS_SPI_SPD            4

/* Wait time before reading xpt2046 (see: TS_IO_Transaction and TS_IO_Delay dunctions) */
#define XPT2046_READDELAY     0

/* Backlight control (the logical level of the active state) */
#define LCD_BLON              0

/* When data direction change (OUT->IN) there is a display that requires extra clock
   example ST7735: 1, ILI9341: 0, ILI9488: 0 */
#define LCD_SCK_EXTRACLK      0

/* DMA TX/RX enable/disable
   - 0: DMA disable
   - 1: DMA enable */
#define LCD_DMA_TX            0
#define LCD_DMA_RX            0

/* In dma mode the bitmap drawing function is completed before the actual drawing.
   When should we wait for the previous DMA operation to complete?
   - 0: The transaction function returns immediately after starting the fill or bitmap draw using DMA. 
        At the beginning of the function, it checks whether the DMA is still working on a previous drawing, 
        if so it waits until it is finished. This is the fastest drawing method, 
        but care must be taken not to change the content of the bitmap after starting the bitmap drawing, 
        because it may result in an incorrect drawing.
   - 1: When drawing a bitmap, the transaction function waits for the end of the drawing, 
        if it fills, it returns immediately after starting the DMA. 
        At the beginning of the function, it checks whether the DMA is still working on a previous drawing, 
        if so it waits until it is finished. 
        With this setting, there will be no errors in drawing under any circumstances. This is the default mode.
   - 2: At the end of each transaction, it waits until the drawing is finished. 
        It is recommended to choose this when using Freertos, other tasks may run while waiting */
#define LCD_DMA_ENDWAIT       1

/* Because there are DMA capable and DMA unable memory regions
   here we can set what is the DMA unable region condition
   note: where the condition is true, it is considered a DMA-unable region
   Default (all memory used for bitmap drawing is DMA capable):
     #define LCD_DMA_UNABLE(addr)  0
   Example stm32f407 and stm32f429 with CCMRAM (the CCMRAM is not DMA capable):
     #define LCD_DMA_UNABLE(addr)  ((addr < 0x20000000) && (addr >= 0x10000000))
   Example stm32h743 (the DTCMRAM and ITCMRAM are not DMA capable):
     #define LCD_DMA_UNABLE(addr)  (((addr < 0x24000000) && (addr >= 0x20000000)) || (addr < 0x08000000))
   Note: if we ensure that we do not draw a bitmap from a DMA-capable memory area, it is not necessary to set it (leave it that way) */
#define LCD_DMA_UNABLE(addr)  0

/* RGB565 to RGB888 and RGB888 to RGB565 convert byte order
   - 0: forward direction
   - 1: back direction
   (warning: the SPI DMA order is from low address to hight address step byte)
   note: If the red and blue colors are reversed and used 24bit mode, change this value */
#define LCD_RGB24_ORDER       0

/* Pixel buffer size for DMA bitdepth conversion (buffer size [byte] = 3 * pixel buffer size)
   note: if 0 -> does not use DMA for 24-bit drawing and reading */
#define LCD_RGB24_BUFFSIZE    0

#endif /* #ifndef __LCDTS_IO_XPT2046_SPI_HAL_H__ */
