/*
 * SPI HAL LCD driver STM32H7
 * author: Roberto Benjami
 * v.2022.11
 */

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
*/
//=============================================================================
/* SPI handle select (see in main.c file, default: hspi1, hspi2 ... hspi6) */
#define LCD_SPI_HANDLE    hspi1

/* SPI mode
   - 0: only TX (write on MOSI pin, no MISO pin)
   - 1: half duplex (MOSI pin is bidirectional)
   - 2: full duplex (write on MOSI pin, read on MISO pin) */
#define LCD_SPI_MODE      0

/* SPI write and read speed (if deleted -> setting in CUBE)
   - hardware SPI clock div fPCLK: 0=/2, 1=/4, 2=/8, 3=/16, 4=/32, 5=/64, 6=/128, 7=/256 */
#define LCD_SPI_SPD_WRITE 3
#define LCD_SPI_SPD_READ  4

/* Backlight control
   - the logical level of the active state */
#define LCD_BLON          0

/* When data direction change (OUT->IN) there is a display that requires extra clock
   example ST7735: 1, ILI9341: 0, ILI9488: 0 */
#define LCD_SCK_EXTRACLK  0

/* DMA TX enable/disable
   - 0 = DMA disable, 1 = DMA enable */
#define LCD_DMA_TX        0

/* Queries whether a DMA operation is in progress
   - 0: not in progress
   - 1: in progress */
uint32_t LCD_IO_DmaBusy(void);

/* If we want to know when the LCD DMA operation is finished, let's create a function in our program:
   void LCD_IO_DmaTxCpltCallback(SPI_HandleTypeDef *hspi)
   This function is called by the driver at the end of DMA operations. */

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
 * here we can set what is the DMA unable region condition
 * note: where the condition is true, it is considered a DMA-unable region
 * If you delete this definition: all memory are DMA capable */
// #define LCD_DMA_UNABLE(addr)  (((addr < 0x24000000) && (addr >= 0x20000000)) || (addr < 0x08000000))
