/*
 ST7735 LCD driver v2022.11

 Add function:
 - st7735_FillRect
 - st7735_ReadRGBImage
 - st7735_Scroll
 - st7735_UserCommand
*/

#include <string.h>
#include "main.h"
#include "lcd.h"
#include "lcd_io.h"
#include "st7735.h"

void     st7735_Init(void);
uint32_t st7735_ReadID(void);
void     st7735_DisplayOn(void);
void     st7735_DisplayOff(void);
void     st7735_SetCursor(uint16_t Xpos, uint16_t Ypos);
void     st7735_WritePixel(uint16_t Xpos, uint16_t Ypos, uint16_t RGB_Code);
uint16_t st7735_ReadPixel(uint16_t Xpos, uint16_t Ypos);
void     st7735_SetDisplayWindow(uint16_t Xpos, uint16_t Ypos, uint16_t Width, uint16_t Height);
void     st7735_DrawHLine(uint16_t RGBCode, uint16_t Xpos, uint16_t Ypos, uint16_t Length);
void     st7735_DrawVLine(uint16_t RGBCode, uint16_t Xpos, uint16_t Ypos, uint16_t Length);
void     st7735_FillRect(uint16_t Xpos, uint16_t Ypos, uint16_t Xsize, uint16_t Ysize, uint16_t RGBCode);
uint16_t st7735_GetLcdPixelWidth(void);
uint16_t st7735_GetLcdPixelHeight(void);
void     st7735_DrawBitmap(uint16_t Xpos, uint16_t Ypos, uint8_t *pbmp);
void     st7735_DrawRGBImage(uint16_t Xpos, uint16_t Ypos, uint16_t Xsize, uint16_t Ysize, uint16_t *pData);
void     st7735_ReadRGBImage(uint16_t Xpos, uint16_t Ypos, uint16_t Xsize, uint16_t Ysize, uint16_t *pData);
void     st7735_Scroll(int16_t Scroll, uint16_t TopFix, uint16_t BottonFix);
void     st7735_UserCommand(uint16_t Command, uint8_t * pData, uint32_t Size, uint8_t Mode);

LCD_DrvTypeDef   st7735_drv =
{
  st7735_Init,
  st7735_ReadID,
  st7735_DisplayOn,
  st7735_DisplayOff,
  st7735_SetCursor,
  st7735_WritePixel,
  st7735_ReadPixel,
  st7735_SetDisplayWindow,
  st7735_DrawHLine,
  st7735_DrawVLine,
  st7735_GetLcdPixelWidth,
  st7735_GetLcdPixelHeight,
  st7735_DrawBitmap,
  st7735_DrawRGBImage,
  st7735_FillRect,
  st7735_ReadRGBImage,
  st7735_Scroll,
  st7735_UserCommand
};

LCD_DrvTypeDef  *lcd_drv = &st7735_drv;

/* transaction data */
#define TRANSDATAMAXSIZE  4
union
{
  char       c[TRANSDATAMAXSIZE];
  uint8_t   d8[TRANSDATAMAXSIZE];
  uint16_t d16[TRANSDATAMAXSIZE / 2];
}transdata;

#define ST7735_NOP            0x00
#define ST7735_SWRESET        0x01

#define ST7735_RDDID          0x04
#define ST7735_RDDST          0x09
#define ST7735_RDMODE         0x0A
#define ST7735_RDMADCTL       0x0B
#define ST7735_RDPIXFMT       0x0C
#define ST7735_RDIMGFMT       0x0D
#define ST7735_RDSELFDIAG     0x0F

#define ST7735_SLPIN          0x10
#define ST7735_SLPOUT         0x11
#define ST7735_PTLON          0x12
#define ST7735_NORON          0x13

#define ST7735_INVOFF         0x20
#define ST7735_INVON          0x21
#define ST7735_GAMMASET       0x26
#define ST7735_DISPOFF        0x28
#define ST7735_DISPON         0x29

#define ST7735_CASET          0x2A
#define ST7735_PASET          0x2B
#define ST7735_RAMWR          0x2C
#define ST7735_RAMRD          0x2E

#define ST7735_PTLAR          0x30
#define ST7735_VSCRDEF        0x33
#define ST7735_MADCTL         0x36
#define ST7735_VSCRSADD       0x37
#define ST7735_COLMOD         0x3A

#define ST7735_FRMCTR1        0xB1
#define ST7735_FRMCTR2        0xB2
#define ST7735_FRMCTR3        0xB3
#define ST7735_INVCTR         0xB4
#define ST7735_DISSET5        0xB6

#define ST7735_PWCTR1         0xC0
#define ST7735_PWCTR2         0xC1
#define ST7735_PWCTR3         0xC2
#define ST7735_PWCTR4         0xC3
#define ST7735_PWCTR5         0xC4
#define ST7735_VMCTR1         0xC5
#define ST7735_VMCTR2         0xC7

#define ST7735_RDID1          0xDA
#define ST7735_RDID2          0xDB
#define ST7735_RDID3          0xDC
#define ST7735_RDID4          0xDD

#define ST7735_GMCTRP1        0xE0
#define ST7735_GMCTRN1        0xE1

#define ST7735_PWCTR6         0xFC

#define ST7735_MAD_RGB        0x00
#define ST7735_MAD_BGR        0x08

//-----------------------------------------------------------------------------
#define ST7735_MAD_VERTICAL   0x20
#define ST7735_MAD_X_LEFT     0x00
#define ST7735_MAD_X_RIGHT    0x40
#define ST7735_MAD_Y_UP       0x80
#define ST7735_MAD_Y_DOWN     0x00

#if ST7735_COLORMODE == 0
#define ST7735_MAD_COLORMODE  ST7735_MAD_RGB
#elif ST7735_COLORMODE == 1
#define ST7735_MAD_COLORMODE  ST7735_MAD_BGR
#endif

#if (ST7735_ORIENTATION == 0)
#define ST7735_SIZE_X                     ST7735_LCD_PIXEL_WIDTH
#define ST7735_SIZE_Y                     ST7735_LCD_PIXEL_HEIGHT
#define ST7735_MAD_DATA_RIGHT_THEN_UP     (ST7735_MAD_COLORMODE | ST7735_MAD_X_LEFT  | ST7735_MAD_Y_UP)
#define ST7735_MAD_DATA_RIGHT_THEN_DOWN   (ST7735_MAD_COLORMODE | ST7735_MAD_X_LEFT  | ST7735_MAD_Y_DOWN)
#elif (ST7735_ORIENTATION == 1)
#define ST7735_SIZE_X                     ST7735_LCD_PIXEL_HEIGHT
#define ST7735_SIZE_Y                     ST7735_LCD_PIXEL_WIDTH
#define ST7735_MAD_DATA_RIGHT_THEN_UP     (ST7735_MAD_COLORMODE | ST7735_MAD_X_LEFT  | ST7735_MAD_Y_DOWN | ST7735_MAD_VERTICAL)
#define ST7735_MAD_DATA_RIGHT_THEN_DOWN   (ST7735_MAD_COLORMODE | ST7735_MAD_X_RIGHT | ST7735_MAD_Y_DOWN | ST7735_MAD_VERTICAL)
#elif (ST7735_ORIENTATION == 2)
#define ST7735_SIZE_X                     ST7735_LCD_PIXEL_WIDTH
#define ST7735_SIZE_Y                     ST7735_LCD_PIXEL_HEIGHT
#define ST7735_MAD_DATA_RIGHT_THEN_UP     (ST7735_MAD_COLORMODE | ST7735_MAD_X_RIGHT | ST7735_MAD_Y_DOWN)
#define ST7735_MAD_DATA_RIGHT_THEN_DOWN   (ST7735_MAD_COLORMODE | ST7735_MAD_X_RIGHT | ST7735_MAD_Y_UP)
#elif (ST7735_ORIENTATION == 3)
#define ST7735_SIZE_X                     ST7735_LCD_PIXEL_HEIGHT
#define ST7735_SIZE_Y                     ST7735_LCD_PIXEL_WIDTH
#define ST7735_MAD_DATA_RIGHT_THEN_UP     (ST7735_MAD_COLORMODE | ST7735_MAD_X_RIGHT | ST7735_MAD_Y_UP   | ST7735_MAD_VERTICAL)
#define ST7735_MAD_DATA_RIGHT_THEN_DOWN   (ST7735_MAD_COLORMODE | ST7735_MAD_X_LEFT  | ST7735_MAD_Y_UP   | ST7735_MAD_VERTICAL)
#endif

#define ST7735_SETWINDOW(x1, x2, y1, y2) \
  { transdata.d16[0] = x1; transdata.d16[1] = x2; LCD_IO_WriteCmd8MultipleData16(ST7735_CASET, (uint16_t *)&transdata, 2); \
    transdata.d16[0] = y1; transdata.d16[1] = y2; LCD_IO_WriteCmd8MultipleData16(ST7735_PASET, (uint16_t *)&transdata, 2); }

#define ST7735_SETCURSOR(x, y)            ST7735_SETWINDOW(x, x, y, y)

//-----------------------------------------------------------------------------
#define ST7735_LCD_INITIALIZED    0x01
#define ST7735_IO_INITIALIZED     0x02
static  uint8_t   Is_st7735_Initialized = 0;

const uint8_t EntryRightThenUp = ST7735_MAD_DATA_RIGHT_THEN_UP;
const uint8_t EntryRightThenDown = ST7735_MAD_DATA_RIGHT_THEN_DOWN;

/* the last set drawing direction is stored here */
uint16_t LastEntry = ST7735_MAD_DATA_RIGHT_THEN_DOWN;

static  uint16_t  yStart, yEnd;

//-----------------------------------------------------------------------------
#if ST7735_WRITEBITDEPTH == ST7735_READBITDEPTH
/* 16/16 and 24/24 bit, no need to change bitdepth data */
#define SetWriteDir()
#define SetReadDir()
#else /* #if ST7735_WRITEBITDEPTH == ST7735_READBITDEPTH */
uint8_t lastdir = 0;
#if ST7735_WRITEBITDEPTH == 16
/* 16/24 bit */
#define SetWriteDir() {                                     \
  if(lastdir != 0)                                          \
  {                                                         \
    LCD_IO_WriteCmd8MultipleData8(ST7735_COLMOD, "\55", 1); \
    lastdir = 0;                                            \
  }                                                         }
#define SetReadDir() {                                      \
  if(lastdir == 0)                                          \
  {                                                         \
    LCD_IO_WriteCmd8MultipleData8(ST7735_COLMOD, "\66", 1); \
    lastdir = 1;                                            \
  }                                                         }
#elif ST7735_WRITEBITDEPTH == 24
/* 24/16 bit */
#define SetWriteDir() {                                     \
  if(lastdir != 0)                                          \
  {                                                         \
    LCD_IO_WriteCmd8MultipleData8(ST7735_COLMOD, "\66", 1); \
    lastdir = 0;                                            \
  }                                                         }
#define SetReadDir() {                                      \
  if(lastdir == 0)                                          \
  {                                                         \
    LCD_IO_WriteCmd8MultipleData8(ST7735_COLMOD, "\55", 1); \
    lastdir = 1;                                            \
  }                                                         }
#endif /* #elif ST7735_WRITEBITDEPTH == 24 */
#endif /* #else ST7735_WRITEBITDEPTH == ST7735_READBITDEPTH */

#if ST7735_WRITEBITDEPTH == 16
#define  LCD_IO_DrawFill(Color, Size) { \
  SetWriteDir(); \
  LCD_IO_WriteCmd8DataFill16(ST7735_RAMWR, Color, Size); }           /* Fill 16 bit pixel(s) */
#define  LCD_IO_DrawBitmap(pData, Size) { \
  SetWriteDir(); \
  LCD_IO_WriteCmd8MultipleData16(ST7735_RAMWR, pData, Size); }       /* Draw 16 bit bitmap */
#elif ST7735_WRITEBITDEPTH == 24
#define  LCD_IO_DrawFill(Color, Size) { \
  SetWriteDir(); \
  LCD_IO_WriteCmd8DataFill16to24(ST7735_RAMWR, Color, Size); }       /* Fill 24 bit pixel(s) from 16 bit color code */
#define  LCD_IO_DrawBitmap(pData, Size) { \
  SetWriteDir(); \
  LCD_IO_WriteCmd8MultipleData16to24(ST7735_RAMWR, pData, Size); }   /* Draw 24 bit Lcd bitmap from 16 bit bitmap data */
#endif /* #elif ST7735_WRITEBITDEPTH == 24 */

#if ST7735_READBITDEPTH == 16
#define  LCD_IO_ReadBitmap(pData, Size) { \
  SetReadDir(); \
  LCD_IO_ReadCmd8MultipleData16(ST7735_RAMRD, pData, Size, 1); }     /* Read 16 bit LCD */
#elif ST7735_READBITDEPTH == 24
#define  LCD_IO_ReadBitmap(pData, Size) { \
  SetReadDir(); \
  LCD_IO_ReadCmd8MultipleData24to16(ST7735_RAMRD, pData, Size, 1); } /* Read 24 bit Lcd and convert to 16 bit bitmap */
#endif /* #elif ST7735_READBITDEPTH == 24 */

//-----------------------------------------------------------------------------
void st7735_Init(void)
{
  if((Is_st7735_Initialized & ST7735_LCD_INITIALIZED) == 0)
  {
    Is_st7735_Initialized |= ST7735_LCD_INITIALIZED;
    if((Is_st7735_Initialized & ST7735_IO_INITIALIZED) == 0)
      LCD_IO_Init();
    Is_st7735_Initialized |= ST7735_IO_INITIALIZED;
  }

  LCD_Delay(1);
  LCD_IO_WriteCmd8MultipleData8(ST7735_SWRESET, NULL, 0);
  LCD_Delay(1);

  // positive gamma control
  LCD_IO_WriteCmd8MultipleData8(ST7735_GMCTRP1, (uint8_t *)"\x09\x16\x09\x20\x21\x1B\x13\x19\x17\x15\x1E\x2B\x04\x05\x02\x0E", 16);

  // negative gamma control
  LCD_IO_WriteCmd8MultipleData8(ST7735_GMCTRN1, (uint8_t *)"\x0B\x14\x08\x1E\x22\x1D\x18\x1E\x1B\x1A\x24\x2B\x06\x06\x02\x0F", 16);

  // Power Control 1 (Vreg1out, Verg2out)
  LCD_IO_WriteCmd8MultipleData8(ST7735_PWCTR1, (uint8_t *)"\x17\x15", 2);

  // Power Control 2 (VGH,VGL)
  LCD_IO_WriteCmd8MultipleData8(ST7735_PWCTR2, (uint8_t *)"\x41", 1);

  // Power Control 3 (Vcom)
  LCD_IO_WriteCmd8MultipleData8(ST7735_VMCTR1, (uint8_t *)"\x00\x12\x80", 3);

  #if ST7735_WRITEBITDEPTH == 16
  LCD_IO_WriteCmd8MultipleData8(ST7735_COLMOD, (uint8_t *)"\x55", 1); // Interface Pixel Format (16 bit)
  #elif ST7735_WRITEBITDEPTH == 24
  LCD_IO_WriteCmd8MultipleData8(ST7735_COLMOD, (uint8_t *)"\x66", 1); // Interface Pixel Format (16 bit)
  #endif

  #if ST7735_SPIMODE == 0
  LCD_IO_WriteCmd8MultipleData8(0xB0, (uint8_t *)"\x80", 1); // Interface Mode Control (SDO NOT USE)
  #elif ST7735_SPIMODE == 1
  LCD_IO_WriteCmd8MultipleData8(0xB0, (uint8_t *)"\x00", 1); // Interface Mode Control (SDO USE)
  #endif
  LCD_IO_WriteCmd8MultipleData8(0xB1, (uint8_t *)"\xA0", 1);// Frame rate (60Hz)
  LCD_IO_WriteCmd8MultipleData8(0xB4, (uint8_t *)"\x02", 1);// Display Inversion Control (2-dot)
  LCD_IO_WriteCmd8MultipleData8(0xB6, (uint8_t *)"\x02\x02", 2); // Display Function Control RGB/MCU Interface Control
  LCD_IO_WriteCmd8MultipleData8(0xE9, (uint8_t *)"\x00", 1);// Set Image Functio (Disable 24 bit data)
  LCD_IO_WriteCmd8MultipleData8(0xF7, (uint8_t *)"\xA9\x51\x2C\x82", 4);// Adjust Control (D7 stream, loose)

  LCD_IO_WriteCmd8MultipleData8(ST7735_MADCTL, &EntryRightThenDown, 1);
  LCD_IO_WriteCmd8MultipleData8(ST7735_SLPOUT, NULL, 0);    // Exit Sleep
  #if ST7735_INITCLEAR == 1
  st7735_FillRect(0, 0, ST7735_SIZE_X, ST7735_SIZE_Y, 0x0000);
  LCD_Delay(10);
  #endif
  LCD_IO_WriteCmd8MultipleData8(ST7735_DISPON, NULL, 0);    // Display on
}

//-----------------------------------------------------------------------------
/**
  * @brief  Enables the Display.
  * @param  None
  * @retval None
  */
void st7735_DisplayOn(void)
{
  LCD_IO_Bl_OnOff(1);
  LCD_IO_WriteCmd8MultipleData8(ST7735_SLPOUT, NULL, 0);    // Exit Sleep
}

//-----------------------------------------------------------------------------
/**
  * @brief  Disables the Display.
  * @param  None
  * @retval None
  */
void st7735_DisplayOff(void)
{
  LCD_IO_WriteCmd8MultipleData8(ST7735_SLPIN, NULL, 0);    // Sleep
  LCD_IO_Bl_OnOff(0);
}

//-----------------------------------------------------------------------------
/**
  * @brief  Get the LCD pixel Width.
  * @param  None
  * @retval The Lcd Pixel Width
  */
uint16_t st7735_GetLcdPixelWidth(void)
{
  return ST7735_SIZE_X;
}

//-----------------------------------------------------------------------------
/**
  * @brief  Get the LCD pixel Height.
  * @param  None
  * @retval The Lcd Pixel Height
  */
uint16_t st7735_GetLcdPixelHeight(void)
{
  return ST7735_SIZE_Y;
}

//-----------------------------------------------------------------------------
/**
  * @brief  Get the ST7735 ID.
  * @param  None
  * @retval The ST7735 ID
  */
uint32_t st7735_ReadID(void)
{
  uint32_t dt = 0;
  LCD_IO_ReadCmd8MultipleData8(ST7735_RDDID, (uint8_t *)&dt, 3, 1);
  return dt;
}

//-----------------------------------------------------------------------------
/**
  * @brief  Set Cursor position.
  * @param  Xpos: specifies the X position.
  * @param  Ypos: specifies the Y position.
  * @retval None
  */
void st7735_SetCursor(uint16_t Xpos, uint16_t Ypos)
{
  ST7735_SETCURSOR(Xpos, Ypos);
}

//-----------------------------------------------------------------------------
/**
  * @brief  Write pixel.
  * @param  Xpos: specifies the X position.
  * @param  Ypos: specifies the Y position.
  * @param  RGBCode: the RGB pixel color
  * @retval None
  */
void st7735_WritePixel(uint16_t Xpos, uint16_t Ypos, uint16_t RGBCode)
{
  if(LastEntry != ST7735_MAD_DATA_RIGHT_THEN_DOWN)
  {
    LastEntry = ST7735_MAD_DATA_RIGHT_THEN_DOWN;
    LCD_IO_WriteCmd8MultipleData8(ST7735_MADCTL, &EntryRightThenDown, 1);
  }
  ST7735_SETCURSOR(Xpos, Ypos);
  LCD_IO_DrawFill(RGBCode, 1);
}


//-----------------------------------------------------------------------------
/**
  * @brief  Read pixel.
  * @param  None
  * @retval the RGB pixel color
  */
uint16_t st7735_ReadPixel(uint16_t Xpos, uint16_t Ypos)
{
  uint16_t ret;
  if(LastEntry != ST7735_MAD_DATA_RIGHT_THEN_DOWN)
  {
    LastEntry = ST7735_MAD_DATA_RIGHT_THEN_DOWN;
    LCD_IO_WriteCmd8MultipleData8(ST7735_MADCTL, &EntryRightThenDown, 1);
  }
  ST7735_SETCURSOR(Xpos, Ypos);
  LCD_IO_ReadBitmap(&ret, 1);
  return(ret);
}

//-----------------------------------------------------------------------------
/**
  * @brief  Sets a display window
  * @param  Xpos:   specifies the X bottom left position.
  * @param  Ypos:   specifies the Y bottom left position.
  * @param  Height: display window height.
  * @param  Width:  display window width.
  * @retval None
  */
void st7735_SetDisplayWindow(uint16_t Xpos, uint16_t Ypos, uint16_t Width, uint16_t Height)
{
  yStart = Ypos; yEnd = Ypos + Height - 1;
  ST7735_SETWINDOW(Xpos, Xpos + Width - 1, Ypos, Ypos + Height - 1);
}

//-----------------------------------------------------------------------------
/**
  * @brief  Draw vertical line.
  * @param  RGBCode: Specifies the RGB color
  * @param  Xpos:     specifies the X position.
  * @param  Ypos:     specifies the Y position.
  * @param  Length:   specifies the Line length.
  * @retval None
  */
void st7735_DrawHLine(uint16_t RGBCode, uint16_t Xpos, uint16_t Ypos, uint16_t Length)
{
  if(LastEntry != ST7735_MAD_DATA_RIGHT_THEN_DOWN)
  {
    LastEntry = ST7735_MAD_DATA_RIGHT_THEN_DOWN;
    LCD_IO_WriteCmd8MultipleData8(ST7735_MADCTL, &EntryRightThenDown, 1);
  }
  ST7735_SETWINDOW(Xpos, Xpos + Length - 1, Ypos, Ypos);
  LCD_IO_DrawFill(RGBCode, Length);
}

//-----------------------------------------------------------------------------
/**
  * @brief  Draw vertical line.
  * @param  RGBCode: Specifies the RGB color
  * @param  Xpos:     specifies the X position.
  * @param  Ypos:     specifies the Y position.
  * @param  Length:   specifies the Line length.
  * @retval None
  */
void st7735_DrawVLine(uint16_t RGBCode, uint16_t Xpos, uint16_t Ypos, uint16_t Length)
{
  if(LastEntry != ST7735_MAD_DATA_RIGHT_THEN_DOWN)
  {
    LastEntry = ST7735_MAD_DATA_RIGHT_THEN_DOWN;
    LCD_IO_WriteCmd8MultipleData8(ST7735_MADCTL, &EntryRightThenDown, 1);
  }
  ST7735_SETWINDOW(Xpos, Xpos, Ypos, Ypos + Length - 1);
  LCD_IO_DrawFill(RGBCode, Length);
}

//-----------------------------------------------------------------------------
/**
  * @brief  Draw Filled rectangle
  * @param  Xpos:     specifies the X position.
  * @param  Ypos:     specifies the Y position.
  * @param  Xsize:    specifies the X size
  * @param  Ysize:    specifies the Y size
  * @param  RGBCode:  specifies the RGB color
  * @retval None
  */
void st7735_FillRect(uint16_t Xpos, uint16_t Ypos, uint16_t Xsize, uint16_t Ysize, uint16_t RGBCode)
{
  if(LastEntry != ST7735_MAD_DATA_RIGHT_THEN_DOWN)
  {
    LastEntry = ST7735_MAD_DATA_RIGHT_THEN_DOWN;
    LCD_IO_WriteCmd8MultipleData8(ST7735_MADCTL, &EntryRightThenDown, 1);
  }
  ST7735_SETWINDOW(Xpos, Xpos + Xsize - 1, Ypos, Ypos + Ysize - 1);
  LCD_IO_DrawFill(RGBCode, Xsize * Ysize);
}

//-----------------------------------------------------------------------------
/**
  * @brief  Displays a 16bit bitmap picture..
  * @param  BmpAddress: Bmp picture address.
  * @param  Xpos:  Bmp X position in the LCD
  * @param  Ypos:  Bmp Y position in the LCD
  * @retval None
  * @brief  Draw direction: right then up
  */
void st7735_DrawBitmap(uint16_t Xpos, uint16_t Ypos, uint8_t *pbmp)
{
  uint32_t index, size;
  /* Read bitmap size */
  size = *(volatile uint16_t *) (pbmp + 2);
  size |= (*(volatile uint16_t *) (pbmp + 4)) << 16;
  /* Get bitmap data address offset */
  index = *(volatile uint16_t *) (pbmp + 10);
  index |= (*(volatile uint16_t *) (pbmp + 12)) << 16;
  size = (size - index)/2;
  pbmp += index;

  if(LastEntry != ST7735_MAD_DATA_RIGHT_THEN_UP)
  {
    LastEntry = ST7735_MAD_DATA_RIGHT_THEN_UP;
    LCD_IO_WriteCmd8MultipleData8(ST7735_MADCTL, &EntryRightThenUp, 1);
  }
  transdata.d16[0] = ST7735_SIZE_Y - 1 - yEnd;
  transdata.d16[1] = ST7735_SIZE_Y - 1 - yStart;
  LCD_IO_WriteCmd8MultipleData16(ST7735_PASET, &transdata, 2);
  LCD_IO_DrawBitmap(pbmp, size);
}

//-----------------------------------------------------------------------------
/**
  * @brief  Displays 16bit/pixel picture..
  * @param  pdata: picture address.
  * @param  Xpos:  Image X position in the LCD
  * @param  Ypos:  Image Y position in the LCD
  * @param  Xsize: Image X size in the LCD
  * @param  Ysize: Image Y size in the LCD
  * @retval None
  * @brief  Draw direction: right then down
  */
void st7735_DrawRGBImage(uint16_t Xpos, uint16_t Ypos, uint16_t Xsize, uint16_t Ysize, uint16_t *pData)
{
  if(LastEntry != ST7735_MAD_DATA_RIGHT_THEN_DOWN)
  {
    LastEntry = ST7735_MAD_DATA_RIGHT_THEN_DOWN;
    LCD_IO_WriteCmd8MultipleData8(ST7735_MADCTL, &EntryRightThenDown, 1);
  }
  st7735_SetDisplayWindow(Xpos, Ypos, Xsize, Ysize);
  LCD_IO_DrawBitmap(pData, Xsize * Ysize);
}

//-----------------------------------------------------------------------------
/**
  * @brief  Read 16bit/pixel picture from Lcd and store to RAM
  * @param  pdata: picture address.
  * @param  Xpos:  Image X position in the LCD
  * @param  Ypos:  Image Y position in the LCD
  * @param  Xsize: Image X size in the LCD
  * @param  Ysize: Image Y size in the LCD
  * @retval None
  * @brief  Draw direction: right then down
  */
void st7735_ReadRGBImage(uint16_t Xpos, uint16_t Ypos, uint16_t Xsize, uint16_t Ysize, uint16_t *pData)
{
  if(LastEntry != ST7735_MAD_DATA_RIGHT_THEN_DOWN)
  {
    LastEntry = ST7735_MAD_DATA_RIGHT_THEN_DOWN;
    LCD_IO_WriteCmd8MultipleData8(ST7735_MADCTL, &EntryRightThenDown, 1);
  }
  st7735_SetDisplayWindow(Xpos, Ypos, Xsize, Ysize);
  LCD_IO_ReadBitmap(pData, Xsize * Ysize);
}

//-----------------------------------------------------------------------------
/**
  * @brief  Set display scroll parameters
  * @param  Scroll    : Scroll size [pixel]
  * @param  TopFix    : Top fix size [pixel]
  * @param  BottonFix : Botton fix size [pixel]
  * @retval None
  */
void st7735_Scroll(int16_t Scroll, uint16_t TopFix, uint16_t BottonFix)
{
  static uint16_t scrparam[4] = {0, 0, 0, 0};
  // Scroll = Scroll % ST7735_LCD_PIXEL_HEIGHT;
  #if (ST7735_ORIENTATION == 0)
  if((TopFix != scrparam[1]) || (BottonFix != scrparam[3]))
  {
    scrparam[1] = TopFix;
    scrparam[3] = BottonFix;
    scrparam[2] = ST7735_LCD_PIXEL_HEIGHT - TopFix - BottonFix;
    LCD_IO_WriteCmd8MultipleData16(ST7735_VSCRDEF, &scrparam[1], 3);
  }
  Scroll = (0 - Scroll) % scrparam[2];
  if(Scroll < 0)
    Scroll = scrparam[2] + Scroll + scrparam[1];
  else
    Scroll = Scroll + scrparam[1];
  #elif (ST7735_ORIENTATION == 1)
  if((TopFix != scrparam[1]) || (BottonFix != scrparam[3]))
  {
    scrparam[1] = TopFix;
    scrparam[3] = BottonFix;
    scrparam[2] = ST7735_LCD_PIXEL_HEIGHT - TopFix - BottonFix;
    LCD_IO_WriteCmd8MultipleData16(ST7735_VSCRDEF, &scrparam[1], 3);
  }
  Scroll = (0 - Scroll) % scrparam[2];
  if(Scroll < 0)
    Scroll = scrparam[2] + Scroll + scrparam[1];
  else
    Scroll = Scroll + scrparam[1];
  #elif (ST7735_ORIENTATION == 2)
  if((TopFix != scrparam[3]) || (BottonFix != scrparam[1]))
  {
    scrparam[3] = TopFix;
    scrparam[1] = BottonFix;
    scrparam[2] = ST7735_LCD_PIXEL_HEIGHT - TopFix - BottonFix;
    LCD_IO_WriteCmd8MultipleData16(ST7735_VSCRDEF, &scrparam[1], 3);
  }
  Scroll %= scrparam[2];
  if(Scroll < 0)
    Scroll = scrparam[2] + Scroll + scrparam[1];
  else
    Scroll = Scroll + scrparam[1];
  #elif (ST7735_ORIENTATION == 3)
  if((TopFix != scrparam[3]) || (BottonFix != scrparam[1]))
  {
    scrparam[3] = TopFix;
    scrparam[1] = BottonFix;
    scrparam[2] = ST7735_LCD_PIXEL_HEIGHT - TopFix - BottonFix;
    LCD_IO_WriteCmd8MultipleData16(ST7735_VSCRDEF, &scrparam[1], 3);
  }
  Scroll %= scrparam[2];
  if(Scroll < 0)
    Scroll = scrparam[2] + Scroll + scrparam[1];
  else
    Scroll = Scroll + scrparam[1];
  #endif
  if(Scroll != scrparam[0])
  {
    scrparam[0] = Scroll;
    LCD_IO_WriteCmd8DataFill16(ST7735_VSCRSADD, scrparam[0], 1);
  }
}

//-----------------------------------------------------------------------------
/**
  * @brief  User command
  * @param  Command   : Lcd command
  * @param  pData     : data pointer
  * @param  Size      : data number
  * @param  Mode      : 0=write 8bits datas, 1=0=write 16bits datas, 2=read 8bits datas, 3=read 16bits datas
  * @retval None
  */
void st7735_UserCommand(uint16_t Command, uint8_t* pData, uint32_t Size, uint8_t Mode)
{
  if(Mode == 0)
    LCD_IO_WriteCmd8MultipleData8((uint8_t)Command, pData, Size);
  else if(Mode == 1)
    LCD_IO_WriteCmd8MultipleData16((uint8_t)Command, pData, Size);
  else if(Mode == 2)
    LCD_IO_ReadCmd8MultipleData8((uint8_t)Command, pData, Size, 1);
  else if(Mode == 3)
    LCD_IO_ReadCmd8MultipleData16((uint8_t)Command, pData, Size, 1);
}
