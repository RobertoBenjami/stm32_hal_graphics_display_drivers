//=============================================================================
/* Information section */

/*
 * XPT2046 touch driver
 * author: Roberto Benjami
 * v.2023.03
 */
 
 /* Settings in CUBEIDE or CUBEMX

 If software SPI
   GPIO
   - Touchscreen SCK
     - output level: Low 
     - mode: Output Push Pull
     - Pull-up/Pull-down: No pull-up and no pull-down
     - Max output speed: Very High
     - User Label: TS_SCK
   - Touchscreen MOSI
     - output level: Low
     - mode: Output Push Pull
     - Pull-up/Pull-down: No pull-up and no pull-down
     - Max output speed: Very High
     - User Label: TS_MOSI
   - Touchscreen MISO
     - output level: n/a 
     - mode: Input mode
     - Pull-up/Pull-down: No pull-up and no pull-down
     - Max output speed: n/a
     - User Label: TS_MISO
   - Touchscreen chip select:
     - output level: High
     - mode: Output Push Pull
     - Pull-up/Pull-down: No pull-up and no pull-down
     - Max output speed: Very High
     - User Label: TS_CS
 
 If hardvare SPI
   SPI Parameter Settings
   - Mode: "Full duplex master"
   - Hardware NSS signal: disabled or enabled (see the Touchscreen chip select GPIO setting)
   - Frame format: Motorola
   - Data Size: 8 bits
   - First bit: MSB first
   - Prescaler: 64 (you can experiment with higher speeds)
   - Clock Polarity: Low
   - Clock Phase: 1 Edge
     note: if the selected SPI pin is not good, change it in the pinout view
   GPIO
   - Touchscreen SCK
     - output level: n/a 
     - mode: Alternate Function Push Pull
     - Pull-up/Pull-down: No pull-up and no pull-down
     - Max output speed: Very High
     - User Label: TS_SCK
   - Touchscreen MOSI
     - output level: n/a 
     - mode: Alternate Function Push Pull
     - Pull-up/Pull-down: No pull-up and no pull-down
     - Max output speed: Very High
     - User Label: TS_MOSI
   - Touchscreen MISO
     - output level: n/a 
     - mode: Alternate Function Push Pull
     - Pull-up/Pull-down: No pull-up and no pull-down
     - Max output speed: Very High
     - User Label: TS_MISO
   - Touchscreen chip select (if hardware NSS signal == disabled)
     - output level: High 
     - mode: Output Push Pull
     - Pull-up/Pull-down: No pull-up and no pull-down
     - Max output speed: Very High
     - User Label: TS_CS
   - Touchscreen chip select (if hardware NSS signal == enabled)
     - output level: n/a
     - mode: Alternate Function Push Pull
     - Pull-up/Pull-down: No pull-up and no pull-down
     - Max output speed: Very High
     - User Label: TS_CS

 Software and hardware SPI
   GPIO
   - Touchscreen IRQ (only when connected)
     - output level: n/a 
     - mode: Input mode
     - Pull-up/Pull-down: No pull-up and no pull-down
     - Max output speed: n/a
     - User Label: TS_IRQ
*/

//=============================================================================
/* Setting section (please set the necessary things in this section) */

/* SPI handle select
   - software SPI (set this macro value: -1, set in CUBEMX the TS_CS, TS_SCK, TS_MISO, TS_MOSI, TS_IRQ pin)
   - hardware SPI handle: see in main.c file (default: hspi1, hspi2 ... hspi6) */
#define TS_SPI_HANDLE         hspi1

/* SPI CS mode (only hardware SPI)
   - 0: software CS operation (hardware NSS signal: disabled)
   - 1: hardware CS operation (hardware NSS signal: enabled) */
#define TS_CS_MODE            1

/* SPI write and read speed (if deleted and hardware SPI -> setting in CUBEMX)
   - software SPI: TS_SCK clock delay (see the TS_IO_Delay function)
   - hardware SPI: clock div fPCLK: 0=/2, 1=/4, 2=/8, 3=/16, 4=/32, 5=/64, 6=/128, 7=/256 */
#define TS_SPI_SPD            4

/* Wait time before reading xpt2046 (see: TS_IO_Transaction and TS_IO_Delay dunctions) */
#define XPT2046_READDELAY     0

/* The touch value that it still accepts as the same value */
#define TOUCH_FILTER          40

/* This is how many times it tries to read the same value */
#define TOUCH_MAXREPEAT       8
