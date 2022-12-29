/*
 * XPT2046 touch driver
 * author: Roberto Benjami
 * v.2022.12
 */

//=============================================================================

/* SPI handle select (see in main.c file, default: hspi1, hspi2 ... hspi6) */
#define TS_SPI_HANDLE         hspi1

/* these constants can be defined with the application appTouchCalib.c */
#define TS_CINDEX             {1762408, -6696, -157114, 622430562, -116657, -5119, 456792831}
