# STM32 graphics display drivers with HAL

It would have been very complicated to write the IO_SPI driver in baremetal style for the stm h7 family, 
so I asked for the help of the factory HAL driver (the baremetal driver has become too complicated). 
The simpler code results in a slower run, but so many sacrifices had to be made. 
I also standardized the LCD driver level, a command can be solved with a single IO function call. 
I also supplemented the BSP driver, making it possible to create individual LCD commands from 
the user's code (BSP_LCD_DataWrite, BSP_LCD_DataRead). I removed some of the settings from the 
configuration include file, so it must be set in the Cube.

## Setting in CubeIde or CubeMX:
###SPI
####Parameter Settings:
- Mode: Full-Duplex Master or Half-Duplex Master or Transmit Only Master
- Hardware NSS Signal: Disable
- Frame Format: Motorola
- Data Size: 8 bits
- First Bit: MSB First
- Prescaler (for Baud Rate): first a higher value, later it can be reduced until it works well
- Clock Polarity: High
- Clock Phase: 2 Edge
(What is not listed is left at the default value)
DMA Settings (not required):
- Add DMA Request SPIn_TX, left at the default value
  
## GPIO:
### Lcd Reset (not required):
- GPIO output level: low
- GPIO mode: Output Push Pull
- GPIO Pull-up/Pull-down: No pull-up and no pull-down
- Maximum output speed: Low
- User Label: LCD_RST
### Lcd CS:
- GPIO output level: High
- GPIO mode: Output Push Pull
- GPIO Pull-up/Pull-down: No pull-up and no pull-down
- Maximum output speed: Very High
- User Label: LCD_CS
### Lcd RS:
- GPIO output level: High
- GPIO mode: Output Push Pull
- GPIO Pull-up/Pull-down: No pull-up and no pull-down
- Maximum output speed: Very High
- User Label: LCD_RS
### Lcd SCK:
- User Label: LCD_SCK
### Lcd backlight (not required):
- GPIO output level: low
- GPIO mode: Output Push Pull
- GPIO Pull-up/Pull-down: No pull-up and no pull-down
- Maximum output speed: Low
- User Label: LCD_BL

## Settings in code:
### lcd_io_spi_hal.h:
- #define LCD_SPI_HANDLE   hspi1 or hspi2 or... (depending on which spi we chose)
- #define LCD_SPI_MODE     0 or 1 or 2 (depending on the selected spi mode)
  0: if SPI mode is Transmit Only Master
  1: if SPI mode is Half-Duplex Master
  2: if SPI mode is Full-Duplex Master
- #define LCD_SPI_SPD_WRITE (a smaller value results in a higher speed. reduce it until it works well)
- #define LCD_SPI_SPD_READ  (a smaller value results in a higher speed. reduce it until it works well)
- #define LCD_BLON 0 or 1 (what logic level is required for the backlight to turn on?)
- #define LCD_SCK_EXTRACLK  0 or 1 (it depends on the type of lcd)
  0: ili9341, ili9488
  1: st7735
- #define LCD_DMA_TX 0 or 1 (SPI TX DMA disabled/enabled)
- uint32_t LCD_IO_DmaBusy(void); (while it returns 1, the DMA operation is not complete)
- void LCD_IO_DmaTxCpltCallback(SPI_HandleTypeDef *hspi): (if we create a function with this name in our program, 
  the driver will call it when the DMA operation ends)
- #define LCD_DMA_WAITMODE  0 or 1 (how to wait for the end of the DMA operation (while of freertos signal))
  0: using a while loop, we wait until the previous DMA operation is completed
  1: using freertos, we let another task run until the DMA operation is finished
- #define LCD_DMA_ENDWAIT   0 or 1 or 2 (when should we wait for the previous DMA operation to complete?)
  0: also at the beginning for filling and bitmap drawing
  1: at the beginning of LCD operation, but at the end of bitmap drawing
  2: also at the end for filling and bitmap drawing
- #define LCD_DMA_ENDWAIT   0 or 1 or 2 (when should we wait for the previous DMA operation to complete?)
  Using dma, the draw function can return immediately after starting the fill and bitmap draw operation.
  If you want to draw again at this point, you have to wait for the previous operation to finish. 
  We can choose when to wait from among 3 strategies.
  0: After each drawing function, the function returns as soon as possible. 
  At the start of the draw function, it checks if there is an unfinished draw operation in progress. 
  If there is, it waits for it to complete. 
  Choosing this strategy, we must be careful not to write to the bitmap memory area after the drawing 
  function returns, because it is still in use by the DMA. 
  If we enter it, an incorrect drawing may be created.
  1: At the beginning of drawing, it checks whether there is an unfinished drawing operation in progress. 
  If there is, it waits for it to complete. In the case of drawing a bitmap, the function returns 
  only after the entire content has been drawn, so it is not possible to spoil the content of the 
  bitmap afterwards.
  2: When drawing fill and bitmap, it waits until the drawing operation is completed at the end, 
  so you no longer need to check if there is an unfinished drawing operation at the beginning 
  of the drawing function.
  Which one should we choose? Without Freertos, by selecting method 0, 
  we can have more work done by the processor, but we must be careful not to write into the 
  bitmap memory of the ongoing drawing. Let's use the LCD_IO_DmaBusy() function and if 
  the value is 1, don't draw, but give the processor another job. 
  Using Freertos, waiting for the completion of the DMA operation is solved by blocking the given task, 
  in which case the other tasks are allowed to run.	
- #define LCD_DMA_UNABLE(addr): here we can prevent DMA-unable memory address from being used by DMA

### ili9341.h, ili9488.h, st7735.h ...
- #define  ..._SPIMODE 0 or 1 (not available for all types)
> 0: for Half-Duplex mode
> 1: for Full-Duplex mode
- #define  ..._ORIENTATION  0...3 (here you can rotate the screen in the right direction)
