/*
 * XPT2046 touch driver
 * author: Roberto Benjami
 * v.2022.12
 */

//=============================================================================
/* Setting section (please set the necessary things in this section) */

/* SPI handle select (see in main.c file, default: hspi1, hspi2 ... hspi6) */
#define TS_SPI_HANDLE         hspi1

/* Wait time before reading xpt2046 (see: TS_IO_Transaction and TS_IO_Delay dunctions) */
#define XPT2046_READDELAY     0
