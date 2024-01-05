/*
 * ILI9486 LCD driver
 * 2023.03
*/

#include "main.h"
#include "lcd.h"
#include "lcd_io.h"
#include "ili9486.h"

/* Lcd */
void     ili9486_Init(void);
uint32_t ili9486_ReadID(void);
void     ili9486_DisplayOn(void);
void     ili9486_DisplayOff(void);
void     ili9486_SetCursor(uint16_t Xpos, uint16_t Ypos);
void     ili9486_WritePixel(uint16_t Xpos, uint16_t Ypos, uint16_t RGB_Code);
uint16_t ili9486_ReadPixel(uint16_t Xpos, uint16_t Ypos);
void     ili9486_SetDisplayWindow(uint16_t Xpos, uint16_t Ypos, uint16_t Width, uint16_t Height);
void     ili9486_DrawHLine(uint16_t RGBCode, uint16_t Xpos, uint16_t Ypos, uint16_t Length);
void     ili9486_DrawVLine(uint16_t RGBCode, uint16_t Xpos, uint16_t Ypos, uint16_t Length);
void     ili9486_FillRect(uint16_t Xpos, uint16_t Ypos, uint16_t Xsize, uint16_t Ysize, uint16_t RGBCode);
uint16_t ili9486_GetLcdPixelWidth(void);
uint16_t ili9486_GetLcdPixelHeight(void);
void     ili9486_DrawBitmap(uint16_t Xpos, uint16_t Ypos, uint8_t *pbmp);
void     ili9486_DrawRGBImage(uint16_t Xpos, uint16_t Ypos, uint16_t Xsize, uint16_t Ysize, uint16_t *pData);
void     ili9486_ReadRGBImage(uint16_t Xpos, uint16_t Ypos, uint16_t Xsize, uint16_t Ysize, uint16_t *pData);
void     ili9486_Scroll(int16_t Scroll, uint16_t TopFix, uint16_t BottonFix);
void     ili9486_UserCommand(uint16_t Command, uint8_t * pData, uint32_t Size, uint8_t Mode);

LCD_DrvTypeDef   ili9486_drv =
{
  ili9486_Init,
  ili9486_ReadID,
  ili9486_DisplayOn,
  ili9486_DisplayOff,
  ili9486_SetCursor,
  ili9486_WritePixel,
  ili9486_ReadPixel,
  ili9486_SetDisplayWindow,
  ili9486_DrawHLine,
  ili9486_DrawVLine,
  ili9486_GetLcdPixelWidth,
  ili9486_GetLcdPixelHeight,
  ili9486_DrawBitmap,
  ili9486_DrawRGBImage,
  ili9486_FillRect,
  ili9486_ReadRGBImage,
  ili9486_Scroll,
  ili9486_UserCommand
};

LCD_DrvTypeDef  *lcd_drv = &ili9486_drv;

/* transaction data */
#define TRANSDATAMAXSIZE  4
union
{
  char       c[TRANSDATAMAXSIZE];
  uint8_t   d8[TRANSDATAMAXSIZE];
  uint16_t d16[TRANSDATAMAXSIZE / 2];
}transdata;

#define ILI9486_NOP            0x00
#define ILI9486_SWRESET        0x01

#define ILI9486_RDDID          0x04
#define ILI9486_RDDST          0x09
#define ILI9486_RDMODE         0x0A
#define ILI9486_RDMADCTL       0x0B
#define ILI9486_RDPIXFMT       0x0C
#define ILI9486_RDIMGFMT       0x0D
#define ILI9486_RDSELFDIAG     0x0F

#define ILI9486_SLPIN          0x10
#define ILI9486_SLPOUT         0x11
#define ILI9486_PTLON          0x12
#define ILI9486_NORON          0x13

#define ILI9486_INVOFF         0x20
#define ILI9486_INVON          0x21
#define ILI9486_GAMMASET       0x26
#define ILI9486_DISPOFF        0x28
#define ILI9486_DISPON         0x29

#define ILI9486_CASET          0x2A
#define ILI9486_PASET          0x2B
#define ILI9486_RAMWR          0x2C
#define ILI9486_RAMRD          0x2E

#define ILI9486_PTLAR          0x30
#define ILI9486_VSCRDEF        0x33
#define ILI9486_MADCTL         0x36
#define ILI9486_VSCRSADD       0x37     /* Vertical Scrolling Start Address */
#define ILI9486_PIXFMT         0x3A     /* COLMOD: Pixel Format Set */

#define ILI9486_RGB_INTERFACE  0xB0     /* RGB Interface Signal Control */
#define ILI9486_FRMCTR1        0xB1
#define ILI9486_FRMCTR2        0xB2
#define ILI9486_FRMCTR3        0xB3
#define ILI9486_INVCTR         0xB4
#define ILI9486_DFUNCTR        0xB6     /* Display Function Control */

#define ILI9486_PWCTR1         0xC0
#define ILI9486_PWCTR2         0xC1
#define ILI9486_PWCTR3         0xC2
#define ILI9486_PWCTR4         0xC3
#define ILI9486_PWCTR5         0xC4
#define ILI9486_VMCTR1         0xC5
#define ILI9486_VMCTR2         0xC7

#define ILI9486_RDID1          0xDA
#define ILI9486_RDID2          0xDB
#define ILI9486_RDID3          0xDC
#define ILI9486_RDID4          0xDD

#define ILI9486_GMCTRP1        0xE0
#define ILI9486_GMCTRN1        0xE1
#define ILI9486_DGCTR1         0xE2
#define ILI9486_DGCTR2         0xE3

//-----------------------------------------------------------------------------
#define ILI9486_MAD_RGB        0x08
#define ILI9486_MAD_BGR        0x00

#define ILI9486_MAD_VERTICAL   0x20
#define ILI9486_MAD_X_LEFT     0x00
#define ILI9486_MAD_X_RIGHT    0x40
#define ILI9486_MAD_Y_UP       0x80
#define ILI9486_MAD_Y_DOWN     0x00

#if ILI9486_COLORMODE == 0
#define ILI9486_MAD_COLORMODE  ILI9486_MAD_RGB
#else
#define ILI9486_MAD_COLORMODE  ILI9486_MAD_BGR
#endif

#define SETWINDOW(x1, x2, y1, y2) 	\
{ \
  transdata.d16[0] = (((x1) >> 8) | ((x1) << 8)); 	\
  transdata.d16[1] = (((x2) >> 8) | ((x2) << 8)); 	\
  LCD_IO_WriteCmd8MultipleData8(ILI9486_CASET, &transdata, 4); \
  transdata.d16[0] = (((y1) >> 8) | ((y1) << 8)); 	\
  transdata.d16[1] = (((y2) >> 8) | ((y2) << 8)); 	\
  LCD_IO_WriteCmd8MultipleData8(ILI9486_PASET, &transdata, 4); \
}

#define SETCURSOR(x, y) 					        \
{ \
  transdata.d16[0] = (((x) >> 8) | ((x) << 8)); 	\
  transdata.d16[1] = transdata.d16[0]; 			\
  LCD_IO_WriteCmd8MultipleData8(ILI9486_CASET, (uint8_t *)&transdata, 4); \
  transdata.d16[0] = (((y) >> 8) | ((y) << 8)); 	\
  transdata.d16[1] = transdata.d16[0]; 			\
  LCD_IO_WriteCmd8MultipleData8(ILI9486_PASET, (uint8_t *)&transdata, 4); \
}

#if (ILI9486_ORIENTATION == 0)
#define ILI9486_SIZE_X                     ILI9486_LCD_PIXEL_WIDTH
#define ILI9486_SIZE_Y                     ILI9486_LCD_PIXEL_HEIGHT
#define ILI9486_MAD_DATA_RIGHT_THEN_UP     (ILI9486_MAD_COLORMODE | ILI9486_MAD_X_RIGHT | ILI9486_MAD_Y_UP)
#define ILI9486_MAD_DATA_RIGHT_THEN_DOWN   (ILI9486_MAD_COLORMODE | ILI9486_MAD_X_RIGHT | ILI9486_MAD_Y_DOWN)
#define ILI9486_MAD_DATA_RGBMODE           (ILI9486_MAD_COLORMODE | ILI9486_MAD_X_LEFT  | ILI9486_MAD_Y_DOWN)
#elif (ILI9486_ORIENTATION == 1)
#define ILI9486_SIZE_X                     ILI9486_LCD_PIXEL_HEIGHT
#define ILI9486_SIZE_Y                     ILI9486_LCD_PIXEL_WIDTH
#define ILI9486_MAD_DATA_RIGHT_THEN_UP     (ILI9486_MAD_COLORMODE | ILI9486_MAD_X_RIGHT | ILI9486_MAD_Y_DOWN | ILI9486_MAD_VERTICAL)
#define ILI9486_MAD_DATA_RIGHT_THEN_DOWN   (ILI9486_MAD_COLORMODE | ILI9486_MAD_X_LEFT  | ILI9486_MAD_Y_DOWN | ILI9486_MAD_VERTICAL)
#define ILI9486_MAD_DATA_RGBMODE           (ILI9486_MAD_COLORMODE | ILI9486_MAD_X_RIGHT | ILI9486_MAD_Y_DOWN)
#elif (ILI9486_ORIENTATION == 2)
#define ILI9486_SIZE_X                     ILI9486_LCD_PIXEL_WIDTH
#define ILI9486_SIZE_Y                     ILI9486_LCD_PIXEL_HEIGHT
#define ILI9486_MAD_DATA_RIGHT_THEN_UP     (ILI9486_MAD_COLORMODE | ILI9486_MAD_X_LEFT  | ILI9486_MAD_Y_DOWN)
#define ILI9486_MAD_DATA_RIGHT_THEN_DOWN   (ILI9486_MAD_COLORMODE | ILI9486_MAD_X_LEFT  | ILI9486_MAD_Y_UP)
#define ILI9486_MAD_DATA_RGBMODE           (ILI9486_MAD_COLORMODE | ILI9486_MAD_X_RIGHT | ILI9486_MAD_Y_UP)
#elif (ILI9486_ORIENTATION == 3)
#define ILI9486_SIZE_X                     ILI9486_LCD_PIXEL_HEIGHT
#define ILI9486_SIZE_Y                     ILI9486_LCD_PIXEL_WIDTH
#define ILI9486_MAD_DATA_RIGHT_THEN_UP     (ILI9486_MAD_COLORMODE | ILI9486_MAD_X_LEFT  | ILI9486_MAD_Y_UP   | ILI9486_MAD_VERTICAL)
#define ILI9486_MAD_DATA_RIGHT_THEN_DOWN   (ILI9486_MAD_COLORMODE | ILI9486_MAD_X_RIGHT | ILI9486_MAD_Y_UP   | ILI9486_MAD_VERTICAL)
#define ILI9486_MAD_DATA_RGBMODE           (ILI9486_MAD_COLORMODE | ILI9486_MAD_X_LEFT  | ILI9486_MAD_Y_UP)
#endif

//-----------------------------------------------------------------------------
#define ILI9486_LCD_INITIALIZED    0x01
#define ILI9486_IO_INITIALIZED     0x02
static  uint8_t   Is_ili9486_Initialized = 0;

const uint8_t EntryRightThenUp = ILI9486_MAD_DATA_RIGHT_THEN_UP;
const uint8_t EntryRightThenDown = ILI9486_MAD_DATA_RIGHT_THEN_DOWN;

/* the last set drawing direction is stored here */
uint8_t LastEntry = ILI9486_MAD_DATA_RIGHT_THEN_DOWN;

uint16_t  yStart, yEnd;

//-----------------------------------------------------------------------------
/* Pixel draw and read functions */

#if ILI9486_WRITEBITDEPTH == ILI9486_READBITDEPTH
/* 16/16 and 24/24 bit, no need to change bitdepth data */
#define SetWriteDir()
#define SetReadDir()
#else /* #if ILI9486_WRITEBITDEPTH == ILI9486_READBITDEPTH */
uint8_t lastdir = 0;
#if ILI9486_WRITEBITDEPTH == 16
/* 16/24 bit */
#define SetWriteDir() {                                      \
  if(lastdir != 0)                                           \
  {                                                          \
    LCD_IO_WriteCmd8MultipleData8(ILI9486_PIXFMT, "\55", 1); \
    lastdir = 0;                                             \
  }                                                          }
#define SetReadDir() {                                       \
  if(lastdir == 0)                                           \
  {                                                          \
    LCD_IO_WriteCmd8MultipleData8(ILI9486_PIXFMT, "\66", 1); \
    lastdir = 1;                                             \
  }                                                          }
#elif ILI9486_WRITEBITDEPTH == 24
/* 24/16 bit */
#define SetWriteDir() {                                      \
  if(lastdir != 0)                                           \
  {                                                          \
    LCD_IO_WriteCmd8MultipleData8(ILI9486_PIXFMT, "\66", 1); \
    lastdir = 0;                                             \
  }                                                          }
#define SetReadDir() {                                       \
  if(lastdir == 0)                                           \
  {                                                          \
    LCD_IO_WriteCmd8MultipleData8(ILI9486_PIXFMT, "\55", 1); \
    lastdir = 1;                                             \
  }                                                          }
#endif /* #elif ILI9486_WRITEBITDEPTH == 24 */
#endif /* #else ILI9486_WRITEBITDEPTH == ILI9486_READBITDEPTH */

#if ILI9486_WRITEBITDEPTH == 16
#if LCD_REVERSE16 == 0
#define  LCD_IO_DrawFill(Color, Size) { \
  SetWriteDir(); \
  LCD_IO_WriteCmd8DataFill16(ILI9486_RAMWR, Color, Size); }           /* Fill 16 bit pixel(s) */
#define  LCD_IO_DrawBitmap(pData, Size) { \
  SetWriteDir(); \
  LCD_IO_WriteCmd8MultipleData16(ILI9486_RAMWR, pData, Size); }       /* Draw 16 bit bitmap */
#elif LCD_REVERSE16 == 1
#define  LCD_IO_DrawFill(Color, Size) { \
  SetWriteDir(); \
  LCD_IO_WriteCmd8DataFill16r(ILI9486_RAMWR, Color, Size); }          /* Fill 16 bit pixel(s) */
#define  LCD_IO_DrawBitmap(pData, Size) { \
  SetWriteDir(); \
  LCD_IO_WriteCmd8MultipleData16r(ILI9486_RAMWR, pData, Size); }      /* Draw 16 bit bitmap */
#endif /* #else LCD_REVERSE16 == 0 */
#elif ILI9486_WRITEBITDEPTH == 24
#define  LCD_IO_DrawFill(Color, Size) { \
  SetWriteDir(); \
  LCD_IO_WriteCmd8DataFill16to24(ILI9486_RAMWR, Color, Size); }       /* Fill 24 bit pixel(s) from 16 bit color code */
#define  LCD_IO_DrawBitmap(pData, Size) { \
  SetWriteDir(); \
  LCD_IO_WriteCmd8MultipleData16to24(ILI9486_RAMWR, pData, Size); }   /* Draw 24 bit Lcd bitmap from 16 bit bitmap data */
#endif /* #elif ILI9486_WRITEBITDEPTH == 24 */

#if ILI9486_READBITDEPTH == 16
#if LCD_REVERSE16 == 0
#define  LCD_IO_ReadBitmap(pData, Size) { \
  SetReadDir(); \
  LCD_IO_ReadCmd8MultipleData16(ILI9486_RAMRD, pData, Size, 1); }     /* Read 16 bit LCD */
#elif LCD_REVERSE16 == 1
#define  LCD_IO_ReadBitmap(pData, Size) { \
  SetReadDir(); \
  LCD_IO_ReadCmd8MultipleData16r(ILI9486_RAMRD, pData, Size, 1); }    /* Read 16 bit LCD */
#endif /* #else LCD_REVERSE16 == 0 */
#elif ILI9486_READBITDEPTH == 24
#define  LCD_IO_ReadBitmap(pData, Size) { \
  SetReadDir(); \
  LCD_IO_ReadCmd8MultipleData24to16(ILI9486_RAMRD, pData, Size, 1); } /* Read 24 bit Lcd and convert to 16 bit bitmap */
#endif /* #elif ILI9486_READBITDEPTH == 24 */

//-----------------------------------------------------------------------------
/**
  * @brief  ILI9486 initialization
  * @param  None
  * @retval None
  */
void ili9486_Init(void)
{
  if((Is_ili9486_Initialized & ILI9486_LCD_INITIALIZED) == 0)
  {
    Is_ili9486_Initialized |= ILI9486_LCD_INITIALIZED;
    if((Is_ili9486_Initialized & ILI9486_IO_INITIALIZED) == 0)
      LCD_IO_Init();
    Is_ili9486_Initialized |= ILI9486_IO_INITIALIZED;
  }
  LCD_Delay(10);
  LCD_IO_WriteCmd8MultipleData8(ILI9486_SWRESET, NULL, 0);
  LCD_Delay(100);

  LCD_IO_WriteCmd8MultipleData8(ILI9486_RGB_INTERFACE, (uint8_t *)"\x00", 1);   /* RGB mode off (0xB0) */
  LCD_IO_WriteCmd8MultipleData8(ILI9486_SLPOUT, NULL, 0);                       /* Exit Sleep (0x11) */
  LCD_Delay(10);

  #if ILI9486_WRITEBITDEPTH == 16
  LCD_IO_WriteCmd8MultipleData8(ILI9486_PIXFMT, (uint8_t *)"\x55", 1); /* interface format (16 bit) */
  #elif ILI9486_WRITEBITDEPTH == 24
  LCD_IO_WriteCmd8MultipleData8(ILI9486_PIXFMT, (uint8_t *)"\x66", 1); /* interface format (24 bit) */
  #endif

  LCD_IO_WriteCmd8MultipleData8(ILI9486_MADCTL, &EntryRightThenDown, 1);

  LCD_IO_WriteCmd8MultipleData8(ILI9486_PWCTR3, (uint8_t *)"\x44", 1); /* 0xC2 */
  LCD_IO_WriteCmd8MultipleData8(ILI9486_VMCTR1, (uint8_t *)"\x00\x00\x00\x00", 4); /* 0xC5 */

  /* positive gamma control (0xE0) */
  LCD_IO_WriteCmd8MultipleData8(ILI9486_GMCTRP1, (uint8_t *)"\x0F\x1F\x1C\x0C\x0F\x08\x48\x98\x37\x0A\x13\x04\x11\x0D\x00", 15);

  /* negative gamma control (0xE1) */
  LCD_IO_WriteCmd8MultipleData8(ILI9486_GMCTRN1, (uint8_t *)"\x0F\x32\x2E\x0B\x0D\x05\x47\x75\x37\x06\x10\x03\x24\x20\x00", 15);

  /* Digital gamma control1 (0xE2) */
  LCD_IO_WriteCmd8MultipleData8(ILI9486_DGCTR1, (uint8_t *)"\x0F\x32\x2E\x0B\x0D\x05\x47\x75\x37\x06\x10\x03\x24\x20\x00", 15);

  LCD_IO_WriteCmd8MultipleData8(ILI9486_NORON, NULL, 0);    /* Normal display on (0x13) */
  LCD_IO_WriteCmd8MultipleData8(ILI9486_INVOFF, NULL, 0);   /* Display inversion off (0x20) */

  #if ILI9486_INITCLEAR == 1
  ili9486_FillRect(0, 0, ILI9486_SIZE_X, ILI9486_SIZE_Y, 0x0000);
  LCD_Delay(1);
  #endif

  LCD_IO_WriteCmd8MultipleData8(ILI9486_SLPOUT, NULL, 0);   /* Exit Sleep (0x11) */
  LCD_Delay(200);
  LCD_IO_WriteCmd8MultipleData8(ILI9486_DISPON, NULL, 0);   /* Display on (0x29) */
  LCD_Delay(10);
}

//-----------------------------------------------------------------------------
/**
  * @brief  Enables the Display.
  * @param  None
  * @retval None
  */
void ili9486_DisplayOn(void)
{
  LCD_IO_WriteCmd8MultipleData8(ILI9486_SLPOUT, NULL, 0); // Exit Sleep
  LCD_IO_Bl_OnOff(1);
}

//-----------------------------------------------------------------------------
/**
  * @brief  Disables the Display.
  * @param  None
  * @retval None
  */
void ili9486_DisplayOff(void)
{
  LCD_IO_WriteCmd8MultipleData8(ILI9486_SLPIN, NULL, 0); // Sleep
  LCD_IO_Bl_OnOff(0);
}

//-----------------------------------------------------------------------------
/**
  * @brief  Get the LCD pixel Width.
  * @param  None
  * @retval The Lcd Pixel Width
  */
uint16_t ili9486_GetLcdPixelWidth(void)
{
  return ILI9486_SIZE_X;
}

//-----------------------------------------------------------------------------
/**
  * @brief  Get the LCD pixel Height.
  * @param  None
  * @retval The Lcd Pixel Height
  */
uint16_t ili9486_GetLcdPixelHeight(void)
{
  return ILI9486_SIZE_Y;
}

//-----------------------------------------------------------------------------
/**
  * @brief  Get the ILI9486 ID.
  * @param  None
  * @retval The ILI9486 ID
  */
uint32_t ili9486_ReadID(void)
{
  uint32_t id = 0;

  if(Is_ili9486_Initialized == 0)
  {
    ili9486_Init();
  }
  LCD_IO_ReadCmd8MultipleData8(0xD3, (uint8_t *)&id, 3, 1);
  return id;
}


//-----------------------------------------------------------------------------
/**
  * @brief  Set Cursor position.
  * @param  Xpos: specifies the X position.
  * @param  Ypos: specifies the Y position.
  * @retval None
  */
void ili9486_SetCursor(uint16_t Xpos, uint16_t Ypos)
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
void ili9486_WritePixel(uint16_t Xpos, uint16_t Ypos, uint16_t RGBCode)
{
  if(LastEntry != ILI9486_MAD_DATA_RIGHT_THEN_DOWN)
  {
    LastEntry = ILI9486_MAD_DATA_RIGHT_THEN_DOWN;
    LCD_IO_WriteCmd8MultipleData8(ILI9486_MADCTL, &EntryRightThenDown, 1);
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
uint16_t ili9486_ReadPixel(uint16_t Xpos, uint16_t Ypos)
{
  uint16_t ret;
  if(LastEntry != ILI9486_MAD_DATA_RIGHT_THEN_DOWN)
  {
    LastEntry = ILI9486_MAD_DATA_RIGHT_THEN_DOWN;
    LCD_IO_WriteCmd8MultipleData8(ILI9486_MADCTL, &EntryRightThenDown, 1);
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
void ili9486_SetDisplayWindow(uint16_t Xpos, uint16_t Ypos, uint16_t Width, uint16_t Height)
{
  yStart = Ypos; yEnd = Ypos + Height - 1;
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
void ili9486_DrawHLine(uint16_t RGBCode, uint16_t Xpos, uint16_t Ypos, uint16_t Length)
{
  if(LastEntry != ILI9486_MAD_DATA_RIGHT_THEN_DOWN)
  {
    LastEntry = ILI9486_MAD_DATA_RIGHT_THEN_DOWN;
    LCD_IO_WriteCmd8MultipleData8(ILI9486_MADCTL, &EntryRightThenDown, 1);
  }
  ili9486_FillRect(Xpos, Ypos, Length, 1, RGBCode);
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
void ili9486_DrawVLine(uint16_t RGBCode, uint16_t Xpos, uint16_t Ypos, uint16_t Length)
{
  if(LastEntry != ILI9486_MAD_DATA_RIGHT_THEN_DOWN)
  {
    LastEntry = ILI9486_MAD_DATA_RIGHT_THEN_DOWN;
    LCD_IO_WriteCmd8MultipleData8(ILI9486_MADCTL, &EntryRightThenDown, 1);
  }
  ili9486_FillRect(Xpos, Ypos, 1, Length, RGBCode);
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
void ili9486_FillRect(uint16_t Xpos, uint16_t Ypos, uint16_t Xsize, uint16_t Ysize, uint16_t RGBCode)
{
  if(LastEntry != ILI9486_MAD_DATA_RIGHT_THEN_DOWN)
  {
    LastEntry = ILI9486_MAD_DATA_RIGHT_THEN_DOWN;
    LCD_IO_WriteCmd8MultipleData8(ILI9486_MADCTL, &EntryRightThenDown, 1);
  }
  ili9486_SetDisplayWindow(Xpos, Ypos, Xsize, Ysize);
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
void ili9486_DrawBitmap(uint16_t Xpos, uint16_t Ypos, uint8_t *pbmp)
{
  uint32_t index = 0, size = 0;
  uint16_t tmp_d16 = 0;
  /* Read bitmap size */
  Ypos += pbmp[22] + (pbmp[23] << 8) - 1;
  size = *(volatile uint16_t *) (pbmp + 2);
  size |= (*(volatile uint16_t *) (pbmp + 4)) << 16;
  /* Get bitmap data address offset */
  index = *(volatile uint16_t *) (pbmp + 10);
  index |= (*(volatile uint16_t *) (pbmp + 12)) << 16;
  size = (size - index)/2;
  pbmp += index;

  if(LastEntry != ILI9486_MAD_DATA_RIGHT_THEN_UP)
  {
    LastEntry = ILI9486_MAD_DATA_RIGHT_THEN_UP;
    LCD_IO_WriteCmd8MultipleData8(ILI9486_MADCTL, &EntryRightThenUp, 1);
  }
  tmp_d16 = ILI9486_SIZE_X - 1 - yEnd;
  transdata.d16[0] = (tmp_d16 << 8) | (tmp_d16 >> 8);
  tmp_d16 = ILI9486_SIZE_Y - 1 - yStart;
  transdata.d16[1] = (tmp_d16 << 8) | (tmp_d16 >> 8);
  LCD_IO_WriteCmd8MultipleData8(ILI9486_PASET, &transdata, 4);
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
void ili9486_DrawRGBImage(uint16_t Xpos, uint16_t Ypos, uint16_t Xsize, uint16_t Ysize, uint16_t *pData)
{
  if(LastEntry != ILI9486_MAD_DATA_RIGHT_THEN_DOWN)
  {
    LastEntry = ILI9486_MAD_DATA_RIGHT_THEN_DOWN;
    LCD_IO_WriteCmd8MultipleData8(ILI9486_MADCTL, &EntryRightThenDown, 1);
  }
  ili9486_SetDisplayWindow(Xpos, Ypos, Xsize, Ysize);
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
void ili9486_ReadRGBImage(uint16_t Xpos, uint16_t Ypos, uint16_t Xsize, uint16_t Ysize, uint16_t *pData)
{
  if(LastEntry != ILI9486_MAD_DATA_RIGHT_THEN_DOWN)
  {
    LastEntry = ILI9486_MAD_DATA_RIGHT_THEN_DOWN;
    LCD_IO_WriteCmd8MultipleData8(ILI9486_MADCTL, &EntryRightThenDown, 1);
  }
  ili9486_SetDisplayWindow(Xpos, Ypos, Xsize, Ysize);
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
void ili9486_Scroll(int16_t Scroll, uint16_t TopFix, uint16_t BottonFix)
{
  static uint16_t scrparam[4] = {0, 0, 0, 0};
  #if (ILI9486_ORIENTATION == 0)
  if((TopFix != scrparam[1]) || (BottonFix != scrparam[3]))
  {
    scrparam[1] = TopFix;
    scrparam[3] = BottonFix;
    scrparam[2] = ILI9486_LCD_PIXEL_HEIGHT - TopFix - BottonFix;
    LCD_IO_WriteCmd8MultipleData16(ILI9486_VSCRDEF, &scrparam[1], 3);
  }
  Scroll = (0 - Scroll) % scrparam[2];
  if(Scroll < 0)
    Scroll = scrparam[2] + Scroll + scrparam[1];
  else
    Scroll = Scroll + scrparam[1];
  #elif (ILI9486_ORIENTATION == 1)
  if((TopFix != scrparam[1]) || (BottonFix != scrparam[3]))
  {
    scrparam[1] = TopFix;
    scrparam[3] = BottonFix;
    scrparam[2] = ILI9486_LCD_PIXEL_HEIGHT - TopFix - BottonFix;
    LCD_IO_WriteCmd8MultipleData16(ILI9486_VSCRDEF, &scrparam[1], 3);
  }
  Scroll = (0 - Scroll) % scrparam[2];
  if(Scroll < 0)
    Scroll = scrparam[2] + Scroll + scrparam[1];
  else
    Scroll = Scroll + scrparam[1];
  #elif (ILI9486_ORIENTATION == 2)
  if((TopFix != scrparam[3]) || (BottonFix != scrparam[1]))
  {
    scrparam[3] = TopFix;
    scrparam[1] = BottonFix;
    scrparam[2] = ILI9486_LCD_PIXEL_HEIGHT - TopFix - BottonFix;
    LCD_IO_WriteCmd8MultipleData16(ILI9486_VSCRDEF, &scrparam[1], 3);
  }
  Scroll %= scrparam[2];
  if(Scroll < 0)
    Scroll = scrparam[2] + Scroll + scrparam[1];
  else
    Scroll = Scroll + scrparam[1];
  #elif (ILI9486_ORIENTATION == 3)
  if((TopFix != scrparam[3]) || (BottonFix != scrparam[1]))
  {
    scrparam[3] = TopFix;
    scrparam[1] = BottonFix;
    scrparam[2] = ILI9486_LCD_PIXEL_HEIGHT - TopFix - BottonFix;
    LCD_IO_WriteCmd8MultipleData16(ILI9486_VSCRDEF, &scrparam[1], 3);
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
    LCD_IO_WriteCmd8DataFill16(ILI9486_VSCRSADD, scrparam[0], 1);
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
void ili9486_UserCommand(uint16_t Command, uint8_t* pData, uint32_t Size, uint8_t Mode)
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
