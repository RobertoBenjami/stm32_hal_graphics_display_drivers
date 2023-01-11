/*
 * 8 bit paralell LCD GPIO driver
 * 5 controll pins (CS, RS, WR, RD, RST) + 8 data pins + backlight pin (BL)
 */

#ifndef __LCD_IO_GPIO8_H
#define __LCD_IO_GPIO8_H

//=============================================================================
/* Data direction
   - 0: only draw mode
   - 1: bidirectional mode */
#define LCD_DATADIR           0

/* RGB565 to RGB888 and RGB888 to RGB565 convert byte order
   - 0: forward direction
   - 1: back direction
   note: If the red and blue colors are reversed and used 24bit mode, change this value */
#define LCD_RGB24_ORDER       0

/*=============================================================================
I/O group optimization so that GPIO operations are not performed bit by bit:
Note: If the pins are in order, they will automatically optimize.

The example belongs to the following pins:
  LCD_D0<-B8,  LCD_D1<-B9,  LCD_D2<-B10, LCD_D3<-B11
  LCD_D4<-B12, LCD_D5<-B13, LCD_D6<-B14, LCD_D7<-B15 */
#if 0
/* datapins setting to output (data direction: STM32 -> LCD) */
#define LCD_DIRWRITE { /* B8..B15 <- 0b01 */ \
GPIOB->MODER = (GPIOB->MODER & ~0xFFFF0000) | 0x55550000; }
/* datapins setting to input (data direction: STM32 <- LCD) */
#define LCD_DIRREAD { /* B8..B15 <- 0b00 */ \
GPIOB->MODER = (GPIOB->MODER & ~0xFFFF0000); }
/* datapins write, STM32 -> LCD (write I/O pins from dt data) */
#define LCD_WRITE(dt) { /* B8..15 <- dt0..7 */ \
GPIOB->BSRR = 0xFF000000 | (dt << 8); }
/* datapins read, STM32 <- LCD (read from I/O pins and store to dt data) */
#define LCD_READ(dt) { /* dt0..7 <- B8..15 */ \
dt = ((GPIOB->IDR & 0xFF00) >> 8); }
#endif

/*
The example belongs to the following pins:
  LCD_D0<-D14, LCD_D1<-D15, LCD_D2<-D0, LCD_D3<-D1
  LCD_D4<-E7,  LCD_D5<-E8,  LCD_D6<-E9, LCD_D7<-E10 */
#if 0
/* datapins setting to output (data direction: STM32 -> LCD) */
#define LCD_DIRWRITE { /* D0..D1, D14..D15, E7..E10 <- 0b01 */ \
GPIOD->MODER = (GPIOD->MODER & ~0b11110000000000000000000000001111) | 0b01010000000000000000000000000101; \
GPIOE->MODER = (GPIOE->MODER & ~0b00000000001111111100000000000000) | 0b00000000000101010100000000000000; }
/* datapins setting to input (data direction: STM32 <- LCD) */
#define LCD_DIRREAD { /* D0..D1, D14..D15, E7..E10 <- 0b00 */ \
GPIOD->MODER = (GPIOD->MODER & ~0b11110000000000000000000000001111); \
GPIOE->MODER = (GPIOE->MODER & ~0b00000000001111111100000000000000); }
/* datapins write, STM32 -> LCD (write I/O pins from dt data) */
#define LCD_WRITE(dt) { /* D14..15 <- dt0..1, D0..1 <- dt2..3, E7..10 <- dt4..7 */ \
GPIOD->ODR = (GPIOD->ODR & ~0b1100000000000011) | (((dt & 0b00000011) << 14) | ((dt & 0b00001100) >> 2)); \
GPIOE->ODR = (GPIOE->ODR & ~0b0000011110000000) | ((dt & 0b11110000) << 3); }
/* datapins read, STM32 <- LCD (read from I/O pins and store to dt data) */
#define LCD_READ(dt) { /* dt0..1 <- D14..15, dt2..3 <- D0..1, dt4..7 <- E7..10 */ \
dt = ((GPIOD->IDR & 0b1100000000000000) >> 14) | ((GPIOD->IDR & 0b0000000000000011) << 2) | \
     ((GPIOE->IDR & 0b0000011110000000) >> 3); }
/* Note: the keil compiler cannot use binary numbers, convert it to hexadecimal */
#endif

#endif // __LCD_IO_GPIO8_H