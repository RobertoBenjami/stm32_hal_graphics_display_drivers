#include "main.h"
#include "lcd.h"
#include "lcd_io.h"
#include "ili9488.h"

void     ili9488_Init(void);
uint32_t ili9488_ReadID(void);
void     ili9488_DisplayOn(void);
void     ili9488_DisplayOff(void);
void     ili9488_SetCursor(uint16_t Xpos, uint16_t Ypos);
void     ili9488_WritePixel(uint16_t Xpos, uint16_t Ypos, uint16_t RGB_Code);
uint16_t ili9488_ReadPixel(uint16_t Xpos, uint16_t Ypos);
void     ili9488_SetDisplayWindow(uint16_t Xpos, uint16_t Ypos, uint16_t Width, uint16_t Height);
void     ili9488_DrawHLine(uint16_t RGBCode, uint16_t Xpos, uint16_t Ypos, uint16_t Length);
void     ili9488_DrawVLine(uint16_t RGBCode, uint16_t Xpos, uint16_t Ypos, uint16_t Length);
uint16_t ili9488_GetLcdPixelWidth(void);
uint16_t ili9488_GetLcdPixelHeight(void);
void     ili9488_DrawBitmap(uint16_t Xpos, uint16_t Ypos, uint8_t *pbmp);
void     ili9488_DrawRGBImage(uint16_t Xpos, uint16_t Ypos, uint16_t Xsize, uint16_t Ysize, uint16_t *pData);
void     ili9488_ReadRGBImage(uint16_t Xpos, uint16_t Ypos, uint16_t Xsize, uint16_t Ysize, uint16_t *pData);
void     ili9488_FillRect(uint16_t Xpos, uint16_t Ypos, uint16_t Xsize, uint16_t Ysize, uint16_t RGBCode);
void     ili9488_Scroll(int16_t Scroll, uint16_t TopFix, uint16_t BottonFix); 
void     ili9488_UserCommand(uint16_t Command, uint8_t * pData, uint32_t Size, uint8_t Mode);

LCD_DrvTypeDef   ili9488_drv =
{
  ili9488_Init,
  ili9488_ReadID,
  ili9488_DisplayOn,
  ili9488_DisplayOff,
  ili9488_SetCursor,
  ili9488_WritePixel,
  ili9488_ReadPixel,
  ili9488_SetDisplayWindow,
  ili9488_DrawHLine,
  ili9488_DrawVLine,
  ili9488_GetLcdPixelWidth,
  ili9488_GetLcdPixelHeight,
  ili9488_DrawBitmap,
  ili9488_DrawRGBImage,
  ili9488_FillRect,
  ili9488_ReadRGBImage,
  ili9488_Scroll,
  ili9488_UserCommand
};

LCD_DrvTypeDef  *lcd_drv = &ili9488_drv;

/* transaction data */
#define TRANSDATAMAXSIZE  4
union
{
  char       c[TRANSDATAMAXSIZE];
  uint8_t   d8[TRANSDATAMAXSIZE];
  uint16_t d16[TRANSDATAMAXSIZE / 2];
}transdata;

#define ILI9488_NOP           0x00
#define ILI9488_SWRESET       0x01
#define ILI9488_RDDID         0x04
#define ILI9488_RDDST         0x09

#define ILI9488_SLPIN         0x10
#define ILI9488_SLPOUT        0x11
#define ILI9488_PTLON         0x12
#define ILI9488_NORON         0x13

#define ILI9488_RDMODE        0x0A
#define ILI9488_RDMADCTL      0x0B
#define ILI9488_RDPIXFMT      0x0C
#define ILI9488_RDIMGFMT      0x0D
#define ILI9488_RDSELFDIAG    0x0F

#define ILI9488_INVOFF        0x20
#define ILI9488_INVON         0x21
#define ILI9488_GAMMASET      0x26
#define ILI9488_DISPOFF       0x28
#define ILI9488_DISPON        0x29

#define ILI9488_CASET         0x2A
#define ILI9488_PASET         0x2B
#define ILI9488_RAMWR         0x2C
#define ILI9488_RAMRD         0x2E

#define ILI9488_PTLAR         0x30
#define ILI9488_VSCRDEF       0x33
#define ILI9488_MADCTL        0x36
#define ILI9488_VSCRSADD      0x37
#define ILI9488_PIXFMT        0x3A
#define ILI9488_RAMWRCONT     0x3C
#define ILI9488_RAMRDCONT     0x3E

#define ILI9488_IMCTR         0xB0
#define ILI9488_FRMCTR1       0xB1
#define ILI9488_FRMCTR2       0xB2
#define ILI9488_FRMCTR3       0xB3
#define ILI9488_INVCTR        0xB4
#define ILI9488_DFUNCTR       0xB6

#define ILI9488_PWCTR1        0xC0
#define ILI9488_PWCTR2        0xC1
#define ILI9488_PWCTR3        0xC2
#define ILI9488_PWCTR4        0xC3
#define ILI9488_PWCTR5        0xC4
#define ILI9488_VMCTR1        0xC5
#define ILI9488_VMCTR2        0xC7

#define ILI9488_RDID1         0xDA
#define ILI9488_RDID2         0xDB
#define ILI9488_RDID3         0xDC
#define ILI9488_RDID4         0xDD

#define ILI9488_GMCTRP1       0xE0
#define ILI9488_GMCTRN1       0xE1
#define ILI9488_IMGFUNCT      0xE9

#define ILI9488_ADJCTR3       0xF7

#define ILI9488_MAD_RGB       0x00
#define ILI9488_MAD_BGR       0x08
#define ILI9488_MAD_VREFRORD  0x10
#define ILI9488_MAD_HREFRORD  0x04

#define ILI9488_MAD_VERTICAL  0x20
#define ILI9488_MAD_X_LEFT    0x00
#define ILI9488_MAD_X_RIGHT   0x40
#define ILI9488_MAD_Y_UP      0x80
#define ILI9488_MAD_Y_DOWN    0x00

#if ILI9488_COLORMODE == 0
#define ILI9488_MAD_COLORMODE ILI9488_MAD_RGB
#elif ILI9488_COLORMODE == 1
#define ILI9488_MAD_COLORMODE ILI9488_MAD_BGR
#endif

#define ILI9488_SETWINDOW(x1, x2, y1, y2) \
  { transdata.d16[0] = x1; transdata.d16[1] = x2; LCD_IO_WriteCmd8MultipleData16(ILI9488_CASET, (uint16_t *)&transdata, 2); \
    transdata.d16[0] = y1; transdata.d16[1] = y2; LCD_IO_WriteCmd8MultipleData16(ILI9488_PASET, (uint16_t *)&transdata, 2); }

#define ILI9488_SETCURSOR(x, y) \
  { transdata.d16[0] = x; transdata.d16[1] = transdata.d16[0]; LCD_IO_WriteCmd8MultipleData16(ILI9488_CASET, (uint16_t *)&transdata, 2); \
    transdata.d16[0] = y; transdata.d16[1] = transdata.d16[0]; LCD_IO_WriteCmd8MultipleData16(ILI9488_PASET, (uint16_t *)&transdata, 2); }

/* the drawing directions of the 4 orientations */
#if ILI9488_ORIENTATION == 0
#define ILI9488_MAX_X                      (ILI9488_LCD_PIXEL_WIDTH - 1)
#define ILI9488_MAX_Y                      (ILI9488_LCD_PIXEL_HEIGHT - 1)
#define ILI9488_MAD_DATA_RIGHT_THEN_UP     (ILI9488_MAD_COLORMODE | ILI9488_MAD_X_RIGHT | ILI9488_MAD_Y_UP)
#define ILI9488_MAD_DATA_RIGHT_THEN_DOWN   (ILI9488_MAD_COLORMODE | ILI9488_MAD_X_RIGHT | ILI9488_MAD_Y_DOWN)
#elif ILI9488_ORIENTATION == 1
#define ILI9488_MAX_X                      (ILI9488_LCD_PIXEL_HEIGHT - 1)
#define ILI9488_MAX_Y                      (ILI9488_LCD_PIXEL_WIDTH - 1)
#define ILI9488_MAD_DATA_RIGHT_THEN_UP     (ILI9488_MAD_COLORMODE | ILI9488_MAD_X_RIGHT | ILI9488_MAD_Y_DOWN | ILI9488_MAD_VERTICAL)
#define ILI9488_MAD_DATA_RIGHT_THEN_DOWN   (ILI9488_MAD_COLORMODE | ILI9488_MAD_X_LEFT  | ILI9488_MAD_Y_DOWN | ILI9488_MAD_VERTICAL)
#elif ILI9488_ORIENTATION == 2
#define ILI9488_MAX_X                      (ILI9488_LCD_PIXEL_WIDTH - 1)
#define ILI9488_MAX_Y                      (ILI9488_LCD_PIXEL_HEIGHT - 1)
#define ILI9488_MAD_DATA_RIGHT_THEN_UP     (ILI9488_MAD_COLORMODE | ILI9488_MAD_X_LEFT  | ILI9488_MAD_Y_DOWN)
#define ILI9488_MAD_DATA_RIGHT_THEN_DOWN   (ILI9488_MAD_COLORMODE | ILI9488_MAD_X_LEFT  | ILI9488_MAD_Y_UP)
#elif ILI9488_ORIENTATION == 3
#define ILI9488_MAX_X                      (ILI9488_LCD_PIXEL_HEIGHT - 1)
#define ILI9488_MAX_Y                      (ILI9488_LCD_PIXEL_WIDTH - 1)
#define ILI9488_MAD_DATA_RIGHT_THEN_UP     (ILI9488_MAD_COLORMODE | ILI9488_MAD_X_LEFT  | ILI9488_MAD_Y_UP   | ILI9488_MAD_VERTICAL)
#define ILI9488_MAD_DATA_RIGHT_THEN_DOWN   (ILI9488_MAD_COLORMODE | ILI9488_MAD_X_RIGHT | ILI9488_MAD_Y_UP   | ILI9488_MAD_VERTICAL)
#endif

#if ILI9488_INTERFACE == 0 || ILI9488_INTERFACE == 1
static  uint16_t  yStart, yEnd;

#define SETWINDOW(x1, x2, y1, y2)          ILI9488_SETWINDOW(x1, x2, y1, y2)
#define SETCURSOR(x, y)                    ILI9488_SETCURSOR(x, y)

#elif ILI9488_INTERFACE == 2
#if ILI9488_ORIENTATION == 0
#define SETWINDOW(x1, x2, y1, y2)          ILI9488_SETWINDOW(ILI9488_MAX_X - (x2), ILI9488_MAX_X - (x1), y1, y2)
#define SETCURSOR(x, y)                    ILI9488_SETCURSOR(ILI9488_MAX_X - (x), y)
#elif ILI9488_ORIENTATION == 1
#define SETWINDOW(x1, x2, y1, y2)          ILI9488_SETWINDOW(x1, x2, y1, y2)
#define SETCURSOR(x, y)                    ILI9488_SETCURSOR(x, y)
#elif ILI9488_ORIENTATION == 2
#define SETWINDOW(x1, x2, y1, y2)          ILI9488_SETWINDOW(x1, x2, ILI9488_MAX_Y - (y2), ILI9488_MAX_Y - (y1))
#define SETCURSOR(x, y)                    ILI9488_SETCURSOR(x, ILI9488_MAX_Y - (y))
#elif ILI9488_ORIENTATION == 3
#define SETWINDOW(x1, x2, y1, y2)          ILI9488_SETWINDOW(ILI9488_MAX_X - (x2), ILI9488_MAX_X - (x1), ILI9488_MAX_Y - (y2), ILI9488_MAX_Y - (y1))
#define SETCURSOR(x, y)                    ILI9488_SETCURSOR(ILI9488_MAX_X - (x), ILI9488_MAX_Y - (y))
#endif
#endif

#ifndef LCD_REVERSE16
#define LCD_REVERSE16    0
#endif

#define ILI9488_LCD_INITIALIZED    0x01
#define ILI9488_IO_INITIALIZED     0x02
static  uint8_t   Is_ili9488_Initialized = 0;

const uint8_t EntryRightThenUp = ILI9488_MAD_DATA_RIGHT_THEN_UP;
const uint8_t EntryRightThenDown = ILI9488_MAD_DATA_RIGHT_THEN_DOWN;

/* the last set drawing direction is stored here */
uint8_t LastEntry = ILI9488_MAD_DATA_RIGHT_THEN_DOWN;

//-----------------------------------------------------------------------------
/* Pixel draw and read functions */

#if ILI9488_WRITEBITDEPTH == ILI9488_READBITDEPTH
/* 16/16 and 24/24 bit, no need to change bitdepth data */
#define SetWriteDir()
#define SetReadDir()
#else /* #if ILI9488_WRITEBITDEPTH == ILI9488_READBITDEPTH */
uint8_t lastdir = 0;
#if ILI9488_WRITEBITDEPTH == 16
/* 16/24 bit */
#define SetWriteDir() {                                      \
  if(lastdir != 0)                                           \
  {                                                          \
    LCD_IO_WriteCmd8MultipleData8(ILI9488_PIXFMT, "\55", 1); \
    lastdir = 0;                                             \
  }                                                          }
#define SetReadDir() {                                       \
  if(lastdir == 0)                                           \
  {                                                          \
    LCD_IO_WriteCmd8MultipleData8(ILI9488_PIXFMT, "\66", 1); \
    lastdir = 1;                                             \
  }                                                          }
#elif ILI9488_WRITEBITDEPTH == 24
/* 24/16 bit */
#define SetWriteDir() {                                      \
  if(lastdir != 0)                                           \
  {                                                          \
    LCD_IO_WriteCmd8MultipleData8(ILI9488_PIXFMT, "\66", 1); \
    lastdir = 0;                                             \
  }                                                          }
#define SetReadDir() {                                       \
  if(lastdir == 0)                                           \
  {                                                          \
    LCD_IO_WriteCmd8MultipleData8(ILI9488_PIXFMT, "\55", 1); \
    lastdir = 1;                                             \
  }                                                          }
#endif /* #elif ILI9488_WRITEBITDEPTH == 24 */
#endif /* #else ILI9488_WRITEBITDEPTH == ILI9488_READBITDEPTH */

#if ILI9488_WRITEBITDEPTH == 16
#if LCD_REVERSE16 == 0
#define  LCD_IO_DrawFill(Color, Size) { \
  SetWriteDir(); \
  LCD_IO_WriteCmd8DataFill16(ILI9488_RAMWR, Color, Size); }           /* Fill 16 bit pixel(s) */
#define  LCD_IO_DrawBitmap(pData, Size) { \
  SetWriteDir(); \
  LCD_IO_WriteCmd8MultipleData16(ILI9488_RAMWR, pData, Size); }       /* Draw 16 bit bitmap */
#elif LCD_REVERSE16 == 1
#define  LCD_IO_DrawFill(Color, Size) { \
  SetWriteDir(); \
  LCD_IO_WriteCmd8DataFill16r(ILI9488_RAMWR, Color, Size); }          /* Fill 16 bit pixel(s) */
#define  LCD_IO_DrawBitmap(pData, Size) { \
  SetWriteDir(); \
  LCD_IO_WriteCmd8MultipleData16r(ILI9488_RAMWR, pData, Size); }      /* Draw 16 bit bitmap */
#endif /* #else LCD_REVERSE16 == 0 */
#elif ILI9488_WRITEBITDEPTH == 24
#define  LCD_IO_DrawFill(Color, Size) { \
  SetWriteDir(); \
  LCD_IO_WriteCmd8DataFill16to24(ILI9488_RAMWR, Color, Size); }       /* Fill 24 bit pixel(s) from 16 bit color code */
#define  LCD_IO_DrawBitmap(pData, Size) { \
  SetWriteDir(); \
  LCD_IO_WriteCmd8MultipleData16to24(ILI9488_RAMWR, pData, Size); }   /* Draw 24 bit Lcd bitmap from 16 bit bitmap data */
#endif /* #elif ILI9488_WRITEBITDEPTH == 24 */

#if ILI9488_READBITDEPTH == 16
#if LCD_REVERSE16 == 0
#define  LCD_IO_ReadBitmap(pData, Size) { \
  SetReadDir(); \
  LCD_IO_ReadCmd8MultipleData16(ILI9488_RAMRD, pData, Size, 1); }     /* Read 16 bit LCD */
#elif LCD_REVERSE16 == 1
#define  LCD_IO_ReadBitmap(pData, Size) { \
  SetReadDir(); \
  LCD_IO_ReadCmd8MultipleData16r(ILI9488_RAMRD, pData, Size, 1); }    /* Read 16 bit LCD */
#endif /* #else LCD_REVERSE16 == 0 */
#elif ILI9488_READBITDEPTH == 24
#define  LCD_IO_ReadBitmap(pData, Size) { \
  SetReadDir(); \
  LCD_IO_ReadCmd8MultipleData24to16(ILI9488_RAMRD, pData, Size, 1); } /* Read 24 bit Lcd and convert to 16 bit bitmap */
#endif /* #elif ILI9488_READBITDEPTH == 24 */

//-----------------------------------------------------------------------------
void ili9488_Init(void)
{
  if((Is_ili9488_Initialized & ILI9488_LCD_INITIALIZED) == 0)
  {
    Is_ili9488_Initialized |= ILI9488_LCD_INITIALIZED;
    if((Is_ili9488_Initialized & ILI9488_IO_INITIALIZED) == 0)
      LCD_IO_Init();
    Is_ili9488_Initialized |= ILI9488_IO_INITIALIZED;
  }

  LCD_Delay(105);
  LCD_IO_WriteCmd8MultipleData8(ILI9488_SWRESET, NULL, 0);
  LCD_Delay(5);
  // positive gamma control
  LCD_IO_WriteCmd8MultipleData8(ILI9488_GMCTRP1, (uint8_t *)"\x00\x01\x02\x04\x14\x09\x3F\x57\x4D\x05\x0B\x09\x1A\x1D\x0F", 15);
  // negative gamma control
  LCD_IO_WriteCmd8MultipleData8(ILI9488_GMCTRN1, (uint8_t *)"\x00\x1D\x20\x02\x0E\x03\x35\x12\x47\x02\x0D\x0C\x38\x39\x0F", 15);
  // Power Control 1 (Vreg1out, Verg2out)
  LCD_IO_WriteCmd8MultipleData8(ILI9488_PWCTR1, (uint8_t *)"\x17\x15", 2);
  LCD_Delay(5);
  // Power Control 2 (VGH,VGL)
  LCD_IO_WriteCmd8MultipleData8(ILI9488_PWCTR2, (uint8_t *)"\x41", 1);
  LCD_Delay(5);
  // Power Control 3 (Vcom)
  LCD_IO_WriteCmd8MultipleData8(ILI9488_VMCTR1, (uint8_t *)"\x00\x12\x80", 3);
  LCD_Delay(5);
  #if ILI9488_WRITEBITDEPTH == 16
  LCD_IO_WriteCmd8MultipleData8(ILI9488_PIXFMT, (uint8_t *)"\x55", 1);  // Interface Pixel Format (16 bit)
  #elif ILI9488_WRITEBITDEPTH == 24
  LCD_IO_WriteCmd8MultipleData8(ILI9488_PIXFMT, (uint8_t *)"\x66", 1);  // Interface Pixel Format (24 bit)
  #endif
  #if ILI9488_INTERFACE == 0
  LCD_IO_WriteCmd8MultipleData8(ILI9488_IMCTR, (uint8_t *)"\x80", 1);   // Interface Mode Control (SDO NOT USE)
  #elif ILI9488_INTERFACE == 1
  LCD_IO_WriteCmd8MultipleData8(ILI9488_IMCTR, (uint8_t *)"\x00", 1);   // Interface Mode Control (SDO USE)
  #endif
  LCD_IO_WriteCmd8MultipleData8(ILI9488_FRMCTR1, (uint8_t *)"\xA0", 1); // Frame rate (60Hz)
  LCD_IO_WriteCmd8MultipleData8(ILI9488_INVCTR, (uint8_t *)"\x02", 1);  // Display Inversion Control (2-dot)
  LCD_IO_WriteCmd8MultipleData8(ILI9488_DFUNCTR, (uint8_t *)"\x02\x02", 2); // Display Function Control RGB/MCU Interface Control
  LCD_IO_WriteCmd8MultipleData8(ILI9488_IMGFUNCT, (uint8_t *)"\x01", 1); // Set Image Functio (Disable 24 bit data)
  LCD_IO_WriteCmd8MultipleData8(ILI9488_ADJCTR3, (uint8_t *)"\xA9\x51\x2C\x82", 4); // Adjust Control (D7 stream, loose)
  LCD_Delay(5);
  LCD_IO_WriteCmd8MultipleData8(ILI9488_SLPOUT, NULL, 0); // Exit Sleep
  LCD_Delay(120);
  #if ILI9488_INITCLEAR == 1
  ili9488_FillRect(0, 0, ILI9488_MAX_X + 1, ILI9488_MAX_Y + 1, 0x0000);
  LCD_Delay(1);
  #endif
  LCD_IO_WriteCmd8MultipleData8(ILI9488_DISPON, NULL, 0); // Display on
  LCD_Delay(5);
  LCD_IO_WriteCmd8MultipleData8(ILI9488_MADCTL, &EntryRightThenDown, 1);
}

//-----------------------------------------------------------------------------
/**
  * @brief  Enables the Display.
  * @param  None
  * @retval None
  */
void ili9488_DisplayOn(void)
{
  LCD_IO_WriteCmd8MultipleData8(ILI9488_SLPOUT, NULL, 0); // Exit Sleep
  LCD_IO_Bl_OnOff(1);
}

//-----------------------------------------------------------------------------
/**
  * @brief  Disables the Display.
  * @param  None
  * @retval None
  */
void ili9488_DisplayOff(void)
{
  LCD_IO_WriteCmd8MultipleData8(ILI9488_SLPIN, NULL, 0); // Sleep
  LCD_IO_Bl_OnOff(0);
}

//-----------------------------------------------------------------------------
/**
  * @brief  Get the LCD pixel Width.
  * @param  None
  * @retval The Lcd Pixel Width
  */
uint16_t ili9488_GetLcdPixelWidth(void)
{
  return ILI9488_MAX_X + 1;
}

//-----------------------------------------------------------------------------
/**
  * @brief  Get the LCD pixel Height.
  * @param  None
  * @retval The Lcd Pixel Height
  */
uint16_t ili9488_GetLcdPixelHeight(void)
{
  return ILI9488_MAX_Y + 1;
}

//-----------------------------------------------------------------------------
/**
  * @brief  Get the ILI9488 ID.
  * @param  None
  * @retval The ILI9488 ID
  */
uint32_t ili9488_ReadID(void)
{
  uint32_t id = 0;

  if(Is_ili9488_Initialized == 0)
  {
    ili9488_Init();
  }
  LCD_IO_ReadCmd8MultipleData8(0x04, (uint8_t *)&id, 3, 1);
  return id;
}

//-----------------------------------------------------------------------------
/**
  * @brief  Set Cursor position.
  * @param  Xpos: specifies the X position.
  * @param  Ypos: specifies the Y position.
  * @retval None
  */
void ili9488_SetCursor(uint16_t Xpos, uint16_t Ypos)
{
  SETCURSOR(Xpos, Ypos);
}

//-----------------------------------------------------------------------------
/**
  * @brief  Write pixel.
  * @param  Xpos: specifies the X position.
  * @param  Ypos: specifies the Y position.
  * @param  RGBCode: the RGB pixel color
  * @retval None
  */
void ili9488_WritePixel(uint16_t Xpos, uint16_t Ypos, uint16_t RGBCode)
{
  if(LastEntry != ILI9488_MAD_DATA_RIGHT_THEN_DOWN)
  {
    LastEntry = ILI9488_MAD_DATA_RIGHT_THEN_DOWN;
    LCD_IO_WriteCmd8MultipleData8(ILI9488_MADCTL, &EntryRightThenDown, 1);
  }
  SETCURSOR(Xpos, Ypos);
  LCD_IO_DrawFill(RGBCode, 1);
}

//-----------------------------------------------------------------------------
/**
  * @brief  Read pixel.
  * @param  None
  * @retval the RGB pixel color
  */
uint16_t ili9488_ReadPixel(uint16_t Xpos, uint16_t Ypos)
{
  uint16_t ret;
  if(LastEntry != ILI9488_MAD_DATA_RIGHT_THEN_DOWN)
  {
    LastEntry = ILI9488_MAD_DATA_RIGHT_THEN_DOWN;
    LCD_IO_WriteCmd8MultipleData8(ILI9488_MADCTL, &EntryRightThenDown, 1);
  }
  SETCURSOR(Xpos, Ypos);
  LCD_IO_ReadBitmap(&ret, 1);
  return ret;
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
void ili9488_SetDisplayWindow(uint16_t Xpos, uint16_t Ypos, uint16_t Width, uint16_t Height)
{
  #if ILI9488_INTERFACE == 0 || ILI9488_INTERFACE == 1
  yStart = Ypos; yEnd = Ypos + Height - 1;
  #endif
  SETWINDOW(Xpos, Xpos + Width - 1, Ypos, Ypos + Height - 1);
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
void ili9488_DrawHLine(uint16_t RGBCode, uint16_t Xpos, uint16_t Ypos, uint16_t Length)
{
  if(LastEntry != ILI9488_MAD_DATA_RIGHT_THEN_DOWN)
  {
    LastEntry = ILI9488_MAD_DATA_RIGHT_THEN_DOWN;
    LCD_IO_WriteCmd8MultipleData8(ILI9488_MADCTL, &EntryRightThenDown, 1);
  }
  ili9488_FillRect(Xpos, Ypos, Length, 1, RGBCode);
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
void ili9488_DrawVLine(uint16_t RGBCode, uint16_t Xpos, uint16_t Ypos, uint16_t Length)
{
  if(LastEntry != ILI9488_MAD_DATA_RIGHT_THEN_DOWN)
  {
    LastEntry = ILI9488_MAD_DATA_RIGHT_THEN_DOWN;
    LCD_IO_WriteCmd8MultipleData8(ILI9488_MADCTL, &EntryRightThenDown, 1);
  }
  ili9488_FillRect(Xpos, Ypos, 1, Length, RGBCode);
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
void ili9488_FillRect(uint16_t Xpos, uint16_t Ypos, uint16_t Xsize, uint16_t Ysize, uint16_t RGBCode)
{
  if(LastEntry != ILI9488_MAD_DATA_RIGHT_THEN_DOWN)
  {
    LastEntry = ILI9488_MAD_DATA_RIGHT_THEN_DOWN;
    LCD_IO_WriteCmd8MultipleData8(ILI9488_MADCTL, &EntryRightThenDown, 1);
  }
  ili9488_SetDisplayWindow(Xpos, Ypos, Xsize, Ysize);
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
void ili9488_DrawBitmap(uint16_t Xpos, uint16_t Ypos, uint8_t *pbmp)
{
  uint32_t index = 0, size = 0;
  /* Read bitmap size */
  Ypos += pbmp[22] + (pbmp[23] << 8) - 1;
  size = *(volatile uint16_t *) (pbmp + 2);
  size |= (*(volatile uint16_t *) (pbmp + 4)) << 16;
  /* Get bitmap data address offset */
  index = *(volatile uint16_t *) (pbmp + 10);
  index |= (*(volatile uint16_t *) (pbmp + 12)) << 16;
  size = (size - index)/2;
  pbmp += index;

  if(LastEntry != ILI9488_MAD_DATA_RIGHT_THEN_UP)
  {
    LastEntry = ILI9488_MAD_DATA_RIGHT_THEN_UP;
    LCD_IO_WriteCmd8MultipleData8(ILI9488_MADCTL, &EntryRightThenUp, 1);
  }
  #if ILI9488_INTERFACE == 0 || ILI9488_INTERFACE == 1
  transdata.d16[0] = ILI9488_MAX_Y - yEnd;
  transdata.d16[1] = ILI9488_MAX_Y - yStart;
  LCD_IO_WriteCmd8MultipleData16(ILI9488_PASET, &transdata, 2);
  LCD_IO_DrawBitmap(pbmp, size);
  #elif ILI9488_INTERFACE == 2
  LCD_IO_DrawBitmap(pbmp, size);
  #endif
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
void ili9488_DrawRGBImage(uint16_t Xpos, uint16_t Ypos, uint16_t Xsize, uint16_t Ysize, uint16_t *pData)
{
  if(LastEntry != ILI9488_MAD_DATA_RIGHT_THEN_DOWN)
  {
    LastEntry = ILI9488_MAD_DATA_RIGHT_THEN_DOWN;
    LCD_IO_WriteCmd8MultipleData8(ILI9488_MADCTL, &EntryRightThenDown, 1);
  }
  ili9488_SetDisplayWindow(Xpos, Ypos, Xsize, Ysize);
  LCD_IO_DrawBitmap(pData, Xsize * Ysize);
}

//-----------------------------------------------------------------------------
/**
  * @brief  Read 16bit/pixel vitmap from Lcd..
  * @param  pdata: picture address.
  * @param  Xpos:  Image X position in the LCD
  * @param  Ypos:  Image Y position in the LCD
  * @param  Xsize: Image X size in the LCD
  * @param  Ysize: Image Y size in the LCD
  * @retval None
  * @brief  Draw direction: right then down
  */
void ili9488_ReadRGBImage(uint16_t Xpos, uint16_t Ypos, uint16_t Xsize, uint16_t Ysize, uint16_t *pData)
{
  if(LastEntry != ILI9488_MAD_DATA_RIGHT_THEN_DOWN)
  {
    LastEntry = ILI9488_MAD_DATA_RIGHT_THEN_DOWN;
    LCD_IO_WriteCmd8MultipleData8(ILI9488_MADCTL, &EntryRightThenDown, 1);
  }
  ili9488_SetDisplayWindow(Xpos, Ypos, Xsize, Ysize);
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
void ili9488_Scroll(int16_t Scroll, uint16_t TopFix, uint16_t BottonFix)
{
  static uint16_t scrparam[4] = {0, 0, 0, 0};
  #if (ILI9488_ORIENTATION == 0)
  if((TopFix != scrparam[1]) || (BottonFix != scrparam[3]))
  {
    scrparam[1] = TopFix;
    scrparam[3] = BottonFix;
    scrparam[2] = ILI9488_LCD_PIXEL_HEIGHT - TopFix - BottonFix;
    LCD_IO_WriteCmd8MultipleData16(ILI9488_VSCRDEF, &scrparam[1], 3);
  }
  Scroll = (0 - Scroll) % scrparam[2];
  if(Scroll < 0)
    Scroll = scrparam[2] + Scroll + scrparam[1];
  else
    Scroll = Scroll + scrparam[1];
  #elif (ILI9488_ORIENTATION == 1)
  if((TopFix != scrparam[1]) || (BottonFix != scrparam[3]))
  {
    scrparam[1] = TopFix;
    scrparam[3] = BottonFix;
    scrparam[2] = ILI9488_LCD_PIXEL_HEIGHT - TopFix - BottonFix;
    LCD_IO_WriteCmd8MultipleData16(ILI9488_VSCRDEF, &scrparam[1], 3);
  }
  Scroll = (0 - Scroll) % scrparam[2];
  if(Scroll < 0)
    Scroll = scrparam[2] + Scroll + scrparam[1];
  else
    Scroll = Scroll + scrparam[1];
  #elif (ILI9488_ORIENTATION == 2)
  if((TopFix != scrparam[3]) || (BottonFix != scrparam[1]))
  {
    scrparam[3] = TopFix;
    scrparam[1] = BottonFix;
    scrparam[2] = ILI9488_LCD_PIXEL_HEIGHT - TopFix - BottonFix;
    LCD_IO_WriteCmd8MultipleData16(ILI9488_VSCRDEF, &scrparam[1], 3);
  }
  Scroll %= scrparam[2];
  if(Scroll < 0)
    Scroll = scrparam[2] + Scroll + scrparam[1];
  else
    Scroll = Scroll + scrparam[1];
  #elif (ILI9488_ORIENTATION == 3)
  if((TopFix != scrparam[3]) || (BottonFix != scrparam[1]))
  {
    scrparam[3] = TopFix;
    scrparam[1] = BottonFix;
    scrparam[2] = ILI9488_LCD_PIXEL_HEIGHT - TopFix - BottonFix;
    LCD_IO_WriteCmd8MultipleData16(ILI9488_VSCRDEF, &scrparam[1], 3);
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
    LCD_IO_WriteCmd8DataFill16(ILI9488_VSCRSADD, scrparam[0], 1);
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
void ili9488_UserCommand(uint16_t Command, uint8_t* pData, uint32_t Size, uint8_t Mode)
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
