# STM32 graphics display drivers with HAL

## What should be set first?
Set the peripherals and GPIO pins in cubemx according to the comments in the io driver header.

## Which files should we add to the project?

Upper layer:
- stm32_adafruit_lcd.h, stm32_adafruit_lcd.c, lcd.h, bmp.h
- Fonts folder
- if used touchscreen: stm32_adafruit_ts.h, stm32_adafruit_ts.c, ts.h

Middle layer (must be added to the project according to the type of LCD)
- lcd / hx8347g.h, hx8347g.c (hx8347 lcd driver)
- lcd / ili9325.h, ili9325.c (ili9325 lcd driver)
- lcd / ili9328.h, ili9328.c (ili9328 lcd driver)
- lcd / ili9341.h, ili9341.c (ili9341 lcd driver)
- lcd / ili9488.h, ili9488.c (ili9488 lcd driver)
- lcd / st7735.h, st7735.c (st7735 lcd driver)
- lcd / st7781.h, st7781.c (st7781 lcd driver)

Lower layer (only the necessary files are added)
- lcd_io.h (this is always necessary)
- io_spi / lcd_io_spi_hal.h, lcd_io_spi_hal.c (SPI lcd io driver)
- io_spi / lcd_io_spi_dma2d_hal.h, lcd_io_spi_dma2d_hal.c (SPI lcd io driver with DMA2D bitdepth convert)
- io_spi / lcdts_io_xpt2046_spi_hal.h, lcdts_io_xpt2046_spi_hal.c (SPI lcd io and touchscreen driver in shared SPI pins)
- io_spi / ts_xpt2046.h, ts_xpt2046.c (hardware and software SPI touchscreen driver)
- io_gpio / lcd_io_gpio8_hal.h, lcd_io_gpio8_hal.c (8bit paralell lcd io driver in GPIO)
- io_gpio / lcd_io_gpio16_hal.h, lcd_io_gpio16_hal.c (16bit paralell lcd io driver in GPIO)
- io_gpio / lcdts_io_gpio8_hal.h, lcdts_io_gpio8_hal.c (8bit paralell lcd io driver in GPIO with analog resistive touchscreen)
- io_fscm / lcd_io_fsmc8_hal.h, lcd_io_fsmc8_hal.c (8bit paralell lcd io driver in FSMC hardware)
- io_fscm / lcd_io_fsmc16_hal.h, lcd_io_fsmc16_hal.c (16bit paralell lcd io driver in FSMC hardware)
- io_i2c / ts_stmpe811qtr.h, ts_stmpe811qtr.c (i2c stmpe811 touchscreen driver)

## Note

I rewrote the graphics driver. I changed the old baremetal style to HAL. Why? Many new processor families have appeared recently, and managing the differences between each processor family in a bare-metal way has become too complicated. Using the HAL, the operation of a given i/o peripheral only needs to be done once, the deviations are done by the processor's own libraries for me. Deficiencies will also be filled in this way, because in the old days there are processor families where the selection is quite incomplete.

## How It Works? 

Take the example program “appLcdSpeedTest.c” as an example.

## Upper layer

The “appLcdSpeedTest.c” uses the functions of the upper layer of the driver (stm32_adafruit_lcd.h / c). This layer contains many drawing functions (initialization, point, line, rectangle, circle, oval, some filled shapes, text, bitmap, image, point and image readback, etc.), if we need more, we can supplement it. This part of the driver is the same for all display types.
- bmp.h is required for the bitmap function, it contains the description of the bitmap header
- lcd.h is a port for upper and middle layer communication.

The following things can be set in this layer
stm32_adafruit_lcd.h:
- default font size: LCD_DEFAULT_FONT
- default background color: LCD_DEFAULT_BACKCOLOR
- default drawing color: LCD_DEFAULT_TEXTCOLOR

We can change these in the program at any time with the functions BSP_LCD_SetFont, BSP_LCD_SetBackColor, BSP_LCD_SetTextColor.

lcd.h:
- 16-bit color code byte sequence reversal: LCD_REVERSE16

We only turn this on if we want to draw in DMA mode with the fsmc8 io interface (the DMA controller can only work in this way). If we turn it on, we must also store all color codes and bitmap data in reverse byte order in our program.

The difference between bitmap and image drawing:
- The bitmap is drawn from the bottom up, the bitmap data must contain the bitmap header.
- The image draws from top to bottom and contains only the pointer containing the raw bit pattern. Therefore, the size of the image must also be specified.

## Middle layer

This layer contains only a few drawing functions (initialization, cursor position setting, drawing window setting, point drawing, horizontal and vertical line drawing, bitmap drawing, image drawing and readback). The upper layer must map all the drawing functions to these few drawing functions. This layer depends on the type of display, because the drawing functions on each display can be solved with a different method, so we have to add the files of the display we use to the project (e.g. ili9341.h / c).

The following things can be set in this layer:

ili9341.h (or other display.h):
- interface type (only for some types): INTERFACE
- Rotation every 90 degrees: ORIENTATION
- clear screen during initialization: INITCLEAR
- color order (if the red and blue colors are swapped, you can change it here): COLORMODE
- color depth for drawing: WRITEBITDEPTH
- color depth for reading: READBITDEPTH
- do not change the screen size: LCD_PIXEL_WIDTH, LCD_PIXEL_HEIGHT

## Lower layer

Carries out the delivery of the data required for initialization and drawing over a physical channel. The physical channel can be an SPI interface or a parallel interface. The parallel interface can use the GPIO pins “lcd_io_gpiox_hal.h / c”, or if the controller contains FSC/FSMC peripherals, we use the “lcd_io_fsmcx_hal.h / c” interface, because it is much faster.

## Touchscreen

The touchscreen driver has only 2 layers.

## Upper layer

- stm32_adafruit_ts.h, stm32_adafruit_ts.c, ts.h

Setting in stm32_adafruit_ts.h:

- TS_CINDEX values that are necessary to calculate the screen coordinate from the AD value of the touchscreen. It is possible to produce with the App TouchCalib or the App / Paint application.

## Lower layer

There are 4 types of touchscreen drivers
- Analog resistive touchscreen with GPIO 8 bits (io_gpio / lcdts_io_gpio8_hal.h, lcdts_io_gpio8_hal.c)
- Xpt2046 touchscreen driver on independent SPI channel (io_spi / ts_xpt2046.h, ts_xpt2046.c). This driver can also work with software SPI, so you can connect it to any pin and you don't need an SPI peripheral.
- Xpt2046 touchscreen driver on shared SPI channel (io_spi / lcdts_io_xpt2046_spi_hal.h, lcdts_io_xpt2046_spi_hal.c)
- Stmpe811 touchscreen driver on I2C channel (io_i2c / ts_stmpe811qtr.h, ts_stmpe811qtr.c)
