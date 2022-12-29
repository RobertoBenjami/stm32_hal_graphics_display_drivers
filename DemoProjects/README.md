graph_hal_f407vet_spi_(appLcdSpeedTest_st7735).zip:
  - Board: stm32f407vet black board + st7735 spi LCD
  - LCD_CS: PB12
  - LCD_SCK: PB13
  - LCD_MOSI: PB15 (bidirectional mode)
  - LCD_RS: PD13
  - LED0: PA6 (not used)
  - LED1: PA7 (not used)
  - BTN0: PE3 (not used)
  - BTN1: PE4 (not used)
  This demo presents the LCD driver speed in DMA mode. Freertos not used.
  
graph_hal_f407vet_spi_(3d_filled_vector_ili9488).zip:
  - Board: stm32f407vet black board + ili9488 spi LCD
  - LCD_CS: PB12
  - LCD_SCK: PB13
  - LCD_MISO: PB14
  - LCD_MOSI: PB15
  - LCD_RS: PD13
  - LED0: PA6
  - LED1: PA7
  - BTN0: PE3 (not used)
  - BTN1: PE4
  This demo presents the LCD driver. Freertos not used. SPI2 max speed is 21MBit/s -> The demo run about 5fps. Transfer it to SPI1, it is capable of 42MBit/s.
	
	