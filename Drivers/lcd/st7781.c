#include "main.h"
#include "lcd.h"
#include "lcd_io.h"
#include "st7781.h"

// Lcd
void      st7781_Init(void);
uint32_t  st7781_ReadID(void);
void      st7781_DisplayOn(void);
void      st7781_DisplayOff(void);
void      st7781_SetCursor(uint16_t Xpos, uint16_t Ypos);
void      st7781_WritePixel(uint16_t Xpos, uint16_t Ypos, uint16_t RGB_Code);
uint16_t  st7781_ReadPixel(uint16_t Xpos, uint16_t Ypos);
void      st7781_SetDisplayWindow(uint16_t Xpos, uint16_t Ypos, uint16_t Width, uint16_t Height);
void      st7781_DrawHLine(uint16_t RGBCode, uint16_t Xpos, uint16_t Ypos, uint16_t Length);
void      st7781_DrawVLine(uint16_t RGBCode, uint16_t Xpos, uint16_t Ypos, uint16_t Length);
uint16_t  st7781_GetLcdPixelWidth(void);
uint16_t  st7781_GetLcdPixelHeight(void);
void      st7781_DrawBitmap(uint16_t Xpos, uint16_t Ypos, uint8_t *pbmp);
void      st7781_DrawRGBImage(uint16_t Xpos, uint16_t Ypos, uint16_t Xsize, uint16_t Ysize, uint16_t *pdata);
void      st7781_ReadRGBImage(uint16_t Xpos, uint16_t Ypos, uint16_t Xsize, uint16_t Ysize, uint16_t *pdata);
void      st7781_FillRect(uint16_t Xpos, uint16_t Ypos, uint16_t Xsize, uint16_t Ysize, uint16_t RGBCode);
void      st7781_Scroll(int16_t Scroll, uint16_t TopFix, uint16_t BottonFix);
void      st7781_UserCommand(uint16_t Command, uint8_t * pData, uint32_t Size, uint8_t Mode);

// Touchscreen
void      st7781_ts_Init(uint16_t DeviceAddr);
uint8_t   st7781_ts_DetectTouch(uint16_t DeviceAddr);
void      st7781_ts_GetXY(uint16_t DeviceAddr, uint16_t *X, uint16_t *Y);

LCD_DrvTypeDef   st7781_drv =
{
  st7781_Init,
  st7781_ReadID,
  st7781_DisplayOn,
  st7781_DisplayOff,
  st7781_SetCursor,
  st7781_WritePixel,
  st7781_ReadPixel,
  st7781_SetDisplayWindow,
  st7781_DrawHLine,
  st7781_DrawVLine,
  st7781_GetLcdPixelWidth,
  st7781_GetLcdPixelHeight,
  st7781_DrawBitmap,
  st7781_DrawRGBImage,
  st7781_FillRect,
  st7781_ReadRGBImage,
  st7781_Scroll,
  st7781_UserCommand
};

LCD_DrvTypeDef  *lcd_drv = &st7781_drv;
/* transaction data */
#define TRANSDATAMAXSIZE  4
union
{
  char       c[TRANSDATAMAXSIZE];
  uint8_t   d8[TRANSDATAMAXSIZE];
  uint16_t d16[TRANSDATAMAXSIZE / 2];
}transdata;

#define ST7781_START_OSC          0x00
#define ST7781_DRIV_OUT_CTRL      0x01
#define ST7781_DRIV_WAV_CTRL      0x02
#define ST7781_ENTRY_MOD          0x03
#define ST7781_RESIZE_CTRL        0x04
#define ST7781_DISP_CTRL1         0x07
#define ST7781_DISP_CTRL2         0x08
#define ST7781_DISP_CTRL3         0x09
#define ST7781_DISP_CTRL4         0x0A
#define ST7781_RGB_DISP_IF_CTRL1  0x0C
#define ST7781_FRM_MARKER_POS     0x0D
#define ST7781_RGB_DISP_IF_CTRL2  0x0F
#define ST7781_POW_CTRL1          0x10
#define ST7781_POW_CTRL2          0x11
#define ST7781_POW_CTRL3          0x12
#define ST7781_POW_CTRL4          0x13
#define ST7781_GRAM_HOR_AD        0x20
#define ST7781_GRAM_VER_AD        0x21
#define ST7781_RW_GRAM            0x22
#define ST7781_POW_CTRL7          0x29
#define ST7781_FRM_RATE_COL_CTRL  0x2B
#define ST7781_GAMMA_CTRL1        0x30
#define ST7781_GAMMA_CTRL2        0x31
#define ST7781_GAMMA_CTRL3        0x32
#define ST7781_GAMMA_CTRL4        0x35
#define ST7781_GAMMA_CTRL5        0x36
#define ST7781_GAMMA_CTRL6        0x37
#define ST7781_GAMMA_CTRL7        0x38
#define ST7781_GAMMA_CTRL8        0x39
#define ST7781_GAMMA_CTRL9        0x3C
#define ST7781_GAMMA_CTRL10       0x3D
#define ST7781_HOR_START_AD       0x50
#define ST7781_HOR_END_AD         0x51
#define ST7781_VER_START_AD       0x52
#define ST7781_VER_END_AD         0x53
#define ST7781_GATE_SCAN_CTRL1    0x60
#define ST7781_GATE_SCAN_CTRL2    0x61
#define ST7781_GATE_SCAN_CTRL3    0x6A
#define ST7781_PART_IMG1_DISP_POS 0x80
#define ST7781_PART_IMG1_START_AD 0x81
#define ST7781_PART_IMG1_END_AD   0x82
#define ST7781_PART_IMG2_DISP_POS 0x83
#define ST7781_PART_IMG2_START_AD 0x84
#define ST7781_PART_IMG2_END_AD   0x85
#define ST7781_PANEL_IF_CTRL1     0x90
#define ST7781_PANEL_IF_CTRL2     0x92
#define ST7781_PANEL_IF_CTRL3     0x93
#define ST7781_PANEL_IF_CTRL4     0x95
#define ST7781_PANEL_IF_CTRL5     0x97
#define ST7781_PANEL_IF_CTRL6     0x98

// entry mode bitjei (16 vs 18 bites szinkód, szinsorrend, rajzolási irány)
#define ST7781_ENTRY_18BITCOLOR   0x8000
#define ST7781_ENTRY_18BITBCD     0x4000

#define ST7781_ENTRY_RGB          0x1000
#define ST7781_ENTRY_BGR          0x0000

#define ST7781_ENTRY_VERTICAL     0x0008
#define ST7781_ENTRY_X_RIGHT      0x0010
#define ST7781_ENTRY_X_LEFT       0x0000
#define ST7781_ENTRY_Y_DOWN       0x0020
#define ST7781_ENTRY_Y_UP         0x0000

#if ST7781_COLORMODE == 0
#define ST7781_ENTRY_COLORMODE    ST7781_ENTRY_RGB
#else
#define ST7781_ENTRY_COLORMODE    ST7781_ENTRY_BGR
#endif

#if ST7781_ORIENTATION == 0
#define ST7781_XSIZE                      ST7781_LCD_PIXEL_WIDTH
#define ST7781_YSIZE                      ST7781_LCD_PIXEL_HEIGHT
#define ST7781_ENTRY_DATA_RIGHT_THEN_UP   (ST7781_ENTRY_COLORMODE | ST7781_ENTRY_X_RIGHT | ST7781_ENTRY_Y_UP)
#define ST7781_ENTRY_DATA_RIGHT_THEN_DOWN (ST7781_ENTRY_COLORMODE | ST7781_ENTRY_X_RIGHT | ST7781_ENTRY_Y_DOWN)
#define ST7781_ENTRY_DATA_DOWN_THEN_RIGHT (ST7781_ENTRY_COLORMODE | ST7781_ENTRY_X_RIGHT | ST7781_ENTRY_Y_DOWN | ST7781_ENTRY_VERTICAL)
#define ST7781_DRIV_OUT_CTRL_DATA         0x0100
#define ST7781_GATE_SCAN_CTRL1_DATA       0xA700
#define ST7781_SETCURSOR(x, y)            {transdata.d16[0] = x;\
                                           LCD_IO_WriteCmd16MultipleData16(ST7781_GRAM_HOR_AD, &transdata, 1);\
                                           transdata.d16[0] = y;\
                                           LCD_IO_WriteCmd16MultipleData16(ST7781_GRAM_VER_AD, &transdata, 1);}
#elif ST7781_ORIENTATION == 1
#define ST7781_XSIZE                      ST7781_LCD_PIXEL_HEIGHT
#define ST7781_YSIZE                      ST7781_LCD_PIXEL_WIDTH
#define ST7781_ENTRY_DATA_RIGHT_THEN_UP   (ST7781_ENTRY_COLORMODE | ST7781_ENTRY_X_LEFT  | ST7781_ENTRY_Y_DOWN | ST7781_ENTRY_VERTICAL)
#define ST7781_ENTRY_DATA_RIGHT_THEN_DOWN (ST7781_ENTRY_COLORMODE | ST7781_ENTRY_X_RIGHT | ST7781_ENTRY_Y_DOWN | ST7781_ENTRY_VERTICAL)
#define ST7781_ENTRY_DATA_DOWN_THEN_RIGHT (ST7781_ENTRY_COLORMODE | ST7781_ENTRY_X_RIGHT | ST7781_ENTRY_Y_DOWN)
#define ST7781_DRIV_OUT_CTRL_DATA         0x0000
#define ST7781_GATE_SCAN_CTRL1_DATA       0xA700
#define ST7781_SETCURSOR(x, y)            {transdata.d16[0] = y;\
                                           LCD_IO_WriteCmd16MultipleData16(ST7781_GRAM_HOR_AD, &transdata, 1);\
                                           transdata.d16[0] = x;\
                                           LCD_IO_WriteCmd16MultipleData16(ST7781_GRAM_VER_AD, &transdata, 1);}
#elif ST7781_ORIENTATION == 2
#define ST7781_XSIZE                      ST7781_LCD_PIXEL_WIDTH
#define ST7781_YSIZE                      ST7781_LCD_PIXEL_HEIGHT
#define ST7781_ENTRY_DATA_RIGHT_THEN_UP   (ST7781_ENTRY_COLORMODE | ST7781_ENTRY_X_RIGHT | ST7781_ENTRY_Y_UP)
#define ST7781_ENTRY_DATA_RIGHT_THEN_DOWN (ST7781_ENTRY_COLORMODE | ST7781_ENTRY_X_RIGHT | ST7781_ENTRY_Y_DOWN)
#define ST7781_ENTRY_DATA_DOWN_THEN_RIGHT (ST7781_ENTRY_COLORMODE | ST7781_ENTRY_X_RIGHT | ST7781_ENTRY_Y_DOWN | ST7781_ENTRY_VERTICAL)
#define ST7781_DRIV_OUT_CTRL_DATA         0x0000
#define ST7781_GATE_SCAN_CTRL1_DATA       0x2700
#define ST7781_SETCURSOR(x, y)            {transdata.d16[0] = x;\
                                           LCD_IO_WriteCmd16MultipleData16(ST7781_GRAM_HOR_AD, &transdata, 1);\
                                           transdata.d16[0] = y;\
                                           LCD_IO_WriteCmd16MultipleData16(ST7781_GRAM_VER_AD, &transdata, 1);}
#elif ST7781_ORIENTATION == 3
#define ST7781_XSIZE                      ST7781_LCD_PIXEL_HEIGHT
#define ST7781_YSIZE                      ST7781_LCD_PIXEL_WIDTH
#define ST7781_ENTRY_DATA_RIGHT_THEN_UP   (ST7781_ENTRY_COLORMODE | ST7781_ENTRY_X_LEFT  | ST7781_ENTRY_Y_DOWN | ST7781_ENTRY_VERTICAL)
#define ST7781_ENTRY_DATA_RIGHT_THEN_DOWN (ST7781_ENTRY_COLORMODE | ST7781_ENTRY_X_RIGHT | ST7781_ENTRY_Y_DOWN | ST7781_ENTRY_VERTICAL)
#define ST7781_ENTRY_DATA_DOWN_THEN_RIGHT (ST7781_ENTRY_COLORMODE | ST7781_ENTRY_X_RIGHT | ST7781_ENTRY_Y_DOWN)
#define ST7781_DRIV_OUT_CTRL_DATA         0x0100
#define ST7781_GATE_SCAN_CTRL1_DATA       0x2700
#define ST7781_SETCURSOR(x, y)            {transdata.d16[0] = y;\
                                           LCD_IO_WriteCmd16MultipleData16(ST7781_GRAM_HOR_AD, &transdata, 1);\
                                           transdata.d16[0] = x;\
                                           LCD_IO_WriteCmd16MultipleData16(ST7781_GRAM_VER_AD, &transdata, 1);}
#endif

#ifndef LCD_REVERSE16
#define LCD_REVERSE16    0
#endif

const uint16_t DrivOutCtrlData = ST7781_DRIV_OUT_CTRL_DATA;
const uint16_t EntryRightThenUp = ST7781_ENTRY_DATA_RIGHT_THEN_UP;
const uint16_t EntryRightThenDown = ST7781_ENTRY_DATA_RIGHT_THEN_DOWN;
const uint16_t EntryDownThenRight = ST7781_ENTRY_DATA_DOWN_THEN_RIGHT;
const uint16_t LcdPixelWidth = ST7781_LCD_PIXEL_WIDTH - 1;
const uint16_t LcdPixelHeight = ST7781_LCD_PIXEL_HEIGHT - 1;
const uint16_t GateScanCtrl1Data = ST7781_GATE_SCAN_CTRL1_DATA;

/* the last set drawing direction is stored here */
uint16_t LastEntry = ST7781_ENTRY_DATA_RIGHT_THEN_DOWN;

//-----------------------------------------------------------------------------
/* Pixel draw and read functions */
#if LCD_REVERSE16 == 0
#define  LCD_IO_DrawFill(Color, Size) \
  LCD_IO_WriteCmd16DataFill16(ST7781_RW_GRAM, Color, Size)
#define  LCD_IO_DrawBitmap(pData, Size) \
  LCD_IO_WriteCmd16MultipleData16(ST7781_RW_GRAM, pData, Size)
#define  LCD_IO_ReadBitmap(pData, Size) \
  LCD_IO_ReadCmd16MultipleData16(ST7781_RW_GRAM, pData, Size, 2)
#elif LCD_REVERSE16 == 1
/* Reverse byte order */
#define  LCD_IO_DrawFill(Color, Size) \
  LCD_IO_WriteCmd16DataFill16r(ST7781_RW_GRAM, Color, Size)
#define  LCD_IO_DrawBitmap(pData, Size) \
  LCD_IO_WriteCmd16MultipleData16r(ST7781_RW_GRAM, pData, Size)
#define  LCD_IO_ReadBitmap(pData, Size) \
  LCD_IO_ReadCmd16MultipleData16r(ST7781_RW_GRAM, pData, Size, 2)
#endif

//-----------------------------------------------------------------------------
void st7781_Init(void)
{
  LCD_IO_Init();

  LCD_IO_WriteCmd16MultipleData8(0xF3, "\x00\x08", 2);

  LCD_Delay(5);

  LCD_IO_WriteCmd16MultipleData16(ST7781_DRIV_OUT_CTRL, &DrivOutCtrlData, 1);
  LCD_IO_WriteCmd16MultipleData8(ST7781_DRIV_WAV_CTRL, "\x07\x00", 2);
  LCD_IO_WriteCmd16MultipleData16(ST7781_ENTRY_MOD, &EntryRightThenDown, 1);

  LCD_IO_WriteCmd16MultipleData8(ST7781_DISP_CTRL2, "\x03\x02", 2);
  LCD_IO_WriteCmd16MultipleData8(ST7781_DISP_CTRL3, "\x00\x00", 2);
  /*POWER CONTROL REGISTER INITIAL*/
  LCD_IO_WriteCmd16MultipleData8(ST7781_POW_CTRL1, "\x00\x00", 2);
  LCD_IO_WriteCmd16MultipleData8(ST7781_POW_CTRL2, "\x00\x07", 2);
  LCD_IO_WriteCmd16MultipleData8(ST7781_POW_CTRL3, "\x00\x00", 2);
  LCD_IO_WriteCmd16MultipleData8(ST7781_POW_CTRL4, "\x00\x00", 2);

  LCD_Delay(5);
  /*POWER SUPPPLY STARTUP 1 SETTING*/
  LCD_IO_WriteCmd16MultipleData8(ST7781_POW_CTRL1, "\x14\xB0", 2);

  LCD_Delay(5);
  LCD_IO_WriteCmd16MultipleData8(ST7781_POW_CTRL2, "\x00\x07", 2);
  LCD_Delay(5);

  /*POWER SUPPLY STARTUP 2 SETTING*/
  LCD_IO_WriteCmd16MultipleData8(ST7781_POW_CTRL3, "\x00\x8E", 2);
  LCD_IO_WriteCmd16MultipleData8(ST7781_POW_CTRL4, "\x0C\x00", 2);
  LCD_IO_WriteCmd16MultipleData8(ST7781_POW_CTRL7, "\x00\x15", 2);

  LCD_Delay(5);

  /****GAMMA CLUSTER SETTING****/
  LCD_IO_WriteCmd16MultipleData8(ST7781_GAMMA_CTRL1, "\x00\x00", 2);
  LCD_IO_WriteCmd16MultipleData8(ST7781_GAMMA_CTRL2, "\x01\x07", 2);
  LCD_IO_WriteCmd16MultipleData8(ST7781_GAMMA_CTRL3, "\x00\x00", 2);
  LCD_IO_WriteCmd16MultipleData8(ST7781_GAMMA_CTRL4, "\x02\x03", 2);
  LCD_IO_WriteCmd16MultipleData8(ST7781_GAMMA_CTRL5, "\x04\x02", 2);
  LCD_IO_WriteCmd16MultipleData8(ST7781_GAMMA_CTRL6, "\x00\x00", 2);
  LCD_IO_WriteCmd16MultipleData8(ST7781_GAMMA_CTRL7, "\x02\x07", 2);
  LCD_IO_WriteCmd16MultipleData8(ST7781_GAMMA_CTRL8, "\x00\x00", 2);
  LCD_IO_WriteCmd16MultipleData8(ST7781_GAMMA_CTRL9, "\x02\x03", 2);
  LCD_IO_WriteCmd16MultipleData8(ST7781_GAMMA_CTRL10, "\x04\x03", 2);

  //-DISPLAY WINDOWS 240*320-
  LCD_IO_WriteCmd16MultipleData8(ST7781_HOR_START_AD, "\x00\x00", 2);
  LCD_IO_WriteCmd16MultipleData16(ST7781_HOR_END_AD, &LcdPixelWidth, 1);
  LCD_IO_WriteCmd16MultipleData8(ST7781_VER_START_AD, "\x00\x00", 2);
  LCD_IO_WriteCmd16MultipleData16(ST7781_VER_END_AD, &LcdPixelHeight, 1);

  //----FRAME RATE SETTING-----
  LCD_IO_WriteCmd16MultipleData16(ST7781_GATE_SCAN_CTRL1, &GateScanCtrl1Data, 1);
  LCD_IO_WriteCmd16MultipleData8(ST7781_GATE_SCAN_CTRL2, "\x00\x03", 2);
  LCD_IO_WriteCmd16MultipleData8(ST7781_PANEL_IF_CTRL1, "\x00\x29", 2);
  LCD_Delay(5);

  //------DISPLAY ON------
  LCD_IO_WriteCmd16MultipleData8(ST7781_FRM_RATE_COL_CTRL, "\x00\x0E", 2);
  LCD_IO_WriteCmd16MultipleData8(ST7781_DISP_CTRL1, "\x01\x33", 2);
}

//-----------------------------------------------------------------------------
/**
  * @brief  Enables the Display.
  * @param  None
  * @retval None
  */
void st7781_DisplayOn(void)
{
  /* Power On sequence -------------------------------------------------------*/
  LCD_IO_WriteCmd16MultipleData8(ST7781_POW_CTRL1, "\x14\xB0", 2);
  LCD_IO_Bl_OnOff(1);
}

//-----------------------------------------------------------------------------
/**
  * @brief  Disables the Display.
  * @param  None
  * @retval None
  */
void st7781_DisplayOff(void)
{
  /* Power Off sequence ------------------------------------------------------*/
  LCD_IO_WriteCmd16MultipleData8(ST7781_POW_CTRL1, "\x00\x02", 2);
  LCD_IO_Bl_OnOff(0);
}

//-----------------------------------------------------------------------------
/**
  * @brief  Get the LCD pixel Width.
  * @param  None
  * @retval The Lcd Pixel Width
  */
uint16_t st7781_GetLcdPixelWidth(void)
{
  #if ((ST7781_ORIENTATION == 0) || (ST7781_ORIENTATION == 2))
  return (uint16_t)ST7781_LCD_PIXEL_WIDTH;
  #elif ((ST7781_ORIENTATION == 1) || (ST7781_ORIENTATION == 3))
  return (uint16_t)ST7781_LCD_PIXEL_HEIGHT;
  #endif
}

//-----------------------------------------------------------------------------
/**
  * @brief  Get the LCD pixel Height.
  * @param  None
  * @retval The Lcd Pixel Height
  */
uint16_t st7781_GetLcdPixelHeight(void)
{
  #if ((ST7781_ORIENTATION == 0) || (ST7781_ORIENTATION == 2))
  return (uint16_t)ST7781_LCD_PIXEL_HEIGHT;
  #elif ((ST7781_ORIENTATION == 1) || (ST7781_ORIENTATION == 3))
  return (uint16_t)ST7781_LCD_PIXEL_WIDTH;
  #endif
}

//-----------------------------------------------------------------------------
/**
  * @brief  Get the ST7781 ID.
  * @param  None
  * @retval The ST7781 ID
  */
uint32_t st7781_ReadID(void)
{
  uint32_t ret = 0;
  LCD_IO_ReadCmd16MultipleData16(0x00, &ret, 1, 2);
  return ret;
}

//-----------------------------------------------------------------------------
/**
  * @brief  Set Cursor position.
  * @param  Xpos: specifies the X position.
  * @param  Ypos: specifies the Y position.
  * @retval None
  */
void st7781_SetCursor(uint16_t Xpos, uint16_t Ypos)
{
  ST7781_SETCURSOR(Xpos, Ypos);
}

//-----------------------------------------------------------------------------
/**
  * @brief  Write pixel.
  * @param  Xpos: specifies the X position.
  * @param  Ypos: specifies the Y position.
  * @param  RGBCode: the RGB pixel color
  * @retval None
  */
void st7781_WritePixel(uint16_t Xpos, uint16_t Ypos, uint16_t RGBCode)
{
  if(LastEntry != ST7781_ENTRY_DATA_RIGHT_THEN_DOWN)
  {
    LastEntry = ST7781_ENTRY_DATA_RIGHT_THEN_DOWN;
    LCD_IO_WriteCmd16MultipleData16(ST7781_ENTRY_MOD, &EntryRightThenDown, 1);
  }
  ST7781_SETCURSOR(Xpos, Ypos);
  LCD_IO_DrawFill(RGBCode, 1);
}

//-----------------------------------------------------------------------------
/**
  * @brief  Read pixel.
  * @param  None
  * @retval the RGB pixel color
  */
uint16_t st7781_ReadPixel(uint16_t Xpos, uint16_t Ypos)
{
  uint16_t ret;
  if(LastEntry != ST7781_ENTRY_DATA_RIGHT_THEN_DOWN)
  {
    LastEntry = ST7781_ENTRY_DATA_RIGHT_THEN_DOWN;
    LCD_IO_WriteCmd16MultipleData16(ST7781_ENTRY_MOD, &EntryRightThenDown, 1);
  }
  ST7781_SETCURSOR(Xpos, Ypos);
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
void st7781_SetDisplayWindow(uint16_t Xpos, uint16_t Ypos, uint16_t Width, uint16_t Height)
{
  #if (ST7781_ORIENTATION == 0) || (ST7781_ORIENTATION == 2)
  transdata.d16[0] = Xpos;
  LCD_IO_WriteCmd16MultipleData16(ST7781_HOR_START_AD, &transdata, 1);
  transdata.d16[0] = Xpos + Width - 1;
  LCD_IO_WriteCmd16MultipleData16(ST7781_HOR_END_AD, &transdata, 1);

  transdata.d16[0] = Ypos;
  LCD_IO_WriteCmd16MultipleData16(ST7781_VER_START_AD, &transdata, 1);
  transdata.d16[0] = Ypos + Height - 1;
  LCD_IO_WriteCmd16MultipleData16(ST7781_VER_END_AD, &transdata, 1);

  #elif (ST7781_ORIENTATION == 1) || (ST7781_ORIENTATION == 3)
  transdata.d16[0] = Ypos;
  LCD_IO_WriteCmd16MultipleData16(ST7781_HOR_START_AD, &transdata, 1);
  transdata.d16[0] = Ypos + Height - 1;
  LCD_IO_WriteCmd16MultipleData16(ST7781_HOR_END_AD, &transdata, 1);

  transdata.d16[0] = Xpos;
  LCD_IO_WriteCmd16MultipleData16(ST7781_VER_START_AD, &transdata, 1);
  transdata.d16[0] = Xpos + Width - 1;
  LCD_IO_WriteCmd16MultipleData16(ST7781_VER_END_AD, &transdata, 1);
  #endif
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
void st7781_DrawHLine(uint16_t RGBCode, uint16_t Xpos, uint16_t Ypos, uint16_t Length)
{
  if(LastEntry != ST7781_ENTRY_DATA_RIGHT_THEN_DOWN)
  {
    LastEntry = ST7781_ENTRY_DATA_RIGHT_THEN_DOWN;
    LCD_IO_WriteCmd16MultipleData16(ST7781_ENTRY_MOD, &EntryRightThenDown, 1);
  }
  ST7781_SETCURSOR(Xpos, Ypos);
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
void st7781_DrawVLine(uint16_t RGBCode, uint16_t Xpos, uint16_t Ypos, uint16_t Length)
{
  if(LastEntry != ST7781_ENTRY_DATA_DOWN_THEN_RIGHT)
  {
    LastEntry = ST7781_ENTRY_DATA_DOWN_THEN_RIGHT;
    LCD_IO_WriteCmd16MultipleData16(ST7781_ENTRY_MOD, &EntryDownThenRight, 1);
  }
  ST7781_SETCURSOR(Xpos, Ypos);
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
void st7781_FillRect(uint16_t Xpos, uint16_t Ypos, uint16_t Xsize, uint16_t Ysize, uint16_t RGBCode)
{
  st7781_SetDisplayWindow(Xpos, Ypos, Xsize, Ysize);
  if(LastEntry != ST7781_ENTRY_DATA_RIGHT_THEN_DOWN)
  {
    LastEntry = ST7781_ENTRY_DATA_RIGHT_THEN_DOWN;
    LCD_IO_WriteCmd16MultipleData16(ST7781_ENTRY_MOD, &EntryRightThenDown, 1);
  }
  ST7781_SETCURSOR(Xpos, Ypos);
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
void st7781_DrawBitmap(uint16_t Xpos, uint16_t Ypos, uint8_t *pbmp)
{
  uint32_t index = 0, size = 0;
  /* Read bitmap size */
  size = *(uint16_t *) (pbmp + 2);
  size |= (*(uint16_t *) (pbmp + 4)) << 16;
  /* Get bitmap data address offset */
  index = *(uint16_t *) (pbmp + 10);
  index |= (*(uint16_t *) (pbmp + 12)) << 16;
  size = (size - index)/2;
  pbmp += index;

  if(LastEntry != ST7781_ENTRY_DATA_RIGHT_THEN_UP)
  {
    LastEntry = ST7781_ENTRY_DATA_RIGHT_THEN_UP;
    LCD_IO_WriteCmd16MultipleData16(ST7781_ENTRY_MOD, &EntryRightThenUp, 1);
  }
  ST7781_SETCURSOR(Xpos, Ypos);
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
void st7781_DrawRGBImage(uint16_t Xpos, uint16_t Ypos, uint16_t Xsize, uint16_t Ysize, uint16_t *pdata)
{
  if(LastEntry != ST7781_ENTRY_DATA_RIGHT_THEN_DOWN)
  {
    LastEntry = ST7781_ENTRY_DATA_RIGHT_THEN_DOWN;
    LCD_IO_WriteCmd16MultipleData16(ST7781_ENTRY_MOD, &EntryRightThenDown, 1);
  }
  st7781_SetDisplayWindow(Xpos, Ypos, Xsize, Ysize);
  ST7781_SETCURSOR(Xpos, Ypos);
  LCD_IO_DrawBitmap(pdata, Xsize * Ysize);
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
void st7781_ReadRGBImage(uint16_t Xpos, uint16_t Ypos, uint16_t Xsize, uint16_t Ysize, uint16_t *pdata)
{
  if(LastEntry != ST7781_ENTRY_DATA_RIGHT_THEN_DOWN)
  {
    LastEntry = ST7781_ENTRY_DATA_RIGHT_THEN_DOWN;
    LCD_IO_WriteCmd16MultipleData16(ST7781_ENTRY_MOD, &EntryRightThenDown, 1);
  }
  st7781_SetDisplayWindow(Xpos, Ypos, Xsize, Ysize);
  ST7781_SETCURSOR(Xpos, Ypos);
  LCD_IO_ReadBitmap(pdata, Xsize * Ysize);
}

//-----------------------------------------------------------------------------
/**
  * @brief  Set display scroll parameters
  * @param  Scroll    : Scroll size [pixel]
  * @param  TopFix    : Top fix size [pixel]
  * @param  BottonFix : Botton fix size [pixel]
  * @retval None
  */
void st7781_Scroll(int16_t Scroll, uint16_t TopFix, uint16_t BottonFix)
{ /* This display is not capable scroll function */
  static int16_t scrparam;
  Scroll = (0 - Scroll) % ST7781_LCD_PIXEL_HEIGHT;
  if(Scroll < 0)
    Scroll += ST7781_LCD_PIXEL_HEIGHT;
  if(Scroll != scrparam)
  {
    scrparam = Scroll;
    LCD_IO_WriteCmd16DataFill16(ST7781_GATE_SCAN_CTRL3, scrparam, 1);
  }
}

//-----------------------------------------------------------------------------
void st7781_UserCommand(uint16_t Command, uint8_t* pData, uint32_t Size, uint8_t Mode)
{
  if(Mode == 0)
    LCD_IO_WriteCmd16MultipleData8((uint8_t)Command, pData, Size);
  else if(Mode == 1)
    LCD_IO_WriteCmd16MultipleData16((uint8_t)Command, pData, Size);
  else if(Mode == 2)
    LCD_IO_ReadCmd16MultipleData8((uint8_t)Command, pData, Size, 1);
  else if(Mode == 3)
    LCD_IO_ReadCmd16MultipleData16((uint8_t)Command, pData, Size, 1);
}
