#include "main.h"
#include "lcd.h"
#include "lcd_io.h"
#include "hx8347g.h"

void     hx8347g_Init(void);
uint32_t hx8347g_ReadID(void);
void     hx8347g_DisplayOn(void);
void     hx8347g_DisplayOff(void);
void     hx8347g_SetCursor(uint16_t Xpos, uint16_t Ypos);
void     hx8347g_WritePixel(uint16_t Xpos, uint16_t Ypos, uint16_t RGB_Code);
uint16_t hx8347g_ReadPixel(uint16_t Xpos, uint16_t Ypos);
void     hx8347g_SetDisplayWindow(uint16_t Xpos, uint16_t Ypos, uint16_t Width, uint16_t Height);
void     hx8347g_DrawHLine(uint16_t RGBCode, uint16_t Xpos, uint16_t Ypos, uint16_t Length);
void     hx8347g_DrawVLine(uint16_t RGBCode, uint16_t Xpos, uint16_t Ypos, uint16_t Length);
uint16_t hx8347g_GetLcdPixelWidth(void);
uint16_t hx8347g_GetLcdPixelHeight(void);
void     hx8347g_DrawBitmap(uint16_t Xpos, uint16_t Ypos, uint8_t *pbmp);
void     hx8347g_DrawRGBImage(uint16_t Xpos, uint16_t Ypos, uint16_t Xsize, uint16_t Ysize, uint16_t *pdata);
void     hx8347g_ReadRGBImage(uint16_t Xpos, uint16_t Ypos, uint16_t Xsize, uint16_t Ysize, uint16_t *pdata);
void     hx8347g_FillRect(uint16_t Xpos, uint16_t Ypos, uint16_t Xsize, uint16_t Ysize, uint16_t RGBCode);
void     hx8347g_Scroll(int16_t Scroll, uint16_t TopFix, uint16_t BottonFix);
void     hx8347g_UserCommand(uint16_t Command, uint8_t * pData, uint32_t Size, uint8_t Mode);


LCD_DrvTypeDef   hx8347g_drv =
{
  hx8347g_Init,
  hx8347g_ReadID,
  hx8347g_DisplayOn,
  hx8347g_DisplayOff,
  hx8347g_SetCursor,
  hx8347g_WritePixel,
  hx8347g_ReadPixel,
  hx8347g_SetDisplayWindow,
  hx8347g_DrawHLine,
  hx8347g_DrawVLine,
  hx8347g_GetLcdPixelWidth,
  hx8347g_GetLcdPixelHeight,
  hx8347g_DrawBitmap,
  hx8347g_DrawRGBImage,
  hx8347g_FillRect,
  hx8347g_ReadRGBImage,
  hx8347g_Scroll,
  hx8347g_UserCommand

};

LCD_DrvTypeDef  *lcd_drv = &hx8347g_drv;

#define HX8347G_ID_AD              0x00
#define HX8347G_MODE_CTRL          0x01
#define HX8347G_HOR_START_AD       0x02
#define HX8347G_HOR_START_AD_HI    0x02
#define HX8347G_HOR_START_AD_LO    0x03
#define HX8347G_HOR_END_AD         0x04
#define HX8347G_HOR_END_AD_HI      0x04
#define HX8347G_HOR_END_AD_LO      0x05
#define HX8347G_VER_START_AD       0x06
#define HX8347G_VER_START_AD_HI    0x06
#define HX8347G_VER_START_AD_LO    0x07
#define HX8347G_VER_END_AD         0x08
#define HX8347G_VER_END_AD_HI      0x08
#define HX8347G_VER_END_AD_LO      0x09
#define HX8347G_PARTIAL_START      0x0A
#define HX8347G_PARTIAL_START_HI   0x0A
#define HX8347G_PARTIAL_START_LO   0x0B
#define HX8347G_PARTIAL_END        0x0C
#define HX8347G_PARTIAL_END_HI     0x0C
#define HX8347G_PARTIAL_END_LO     0x0D
#define HX8347G_VER_SCR_TOP        0x0E
#define HX8347G_VER_SCR_TOP_HI     0x0E
#define HX8347G_VER_SCR_TOP_LO     0x0F
#define HX8347G_VER_SCR_HEIGHT     0x10
#define HX8347G_VER_SCR_HEIGHT_HI  0x10
#define HX8347G_VER_SCR_HEIGHT_LO  0x11
#define HX8347G_VER_SCR_BTN        0x12
#define HX8347G_VER_SCR_BTN_HI     0x12
#define HX8347G_VER_SCR_BTN_LO     0x13
#define HX8347G_VER_SCR_START      0x14
#define HX8347G_VER_SCR_START_HI   0x14
#define HX8347G_VER_SCR_START_LO   0x15
#define HX8347G_ENTRY_MOD          0x16
#define HX8347G_COLMOD             0x17
#define HX8347G_OSC_CTRL1          0x18
#define HX8347G_OSC_CTRL2          0x19
#define HX8347G_PWR_CTRL1          0x1A
#define HX8347G_PWR_CTRL2          0x1B
#define HX8347G_PWR_CTRL3          0x1C
#define HX8347G_PWR_CTRL4          0x1D
#define HX8347G_PWR_CTRL5          0x1E
#define HX8347G_PWR_CTRL6          0x1F
#define HX8347G_RW_GRAM            0x22
#define HX8347G_VCOM_CTRL1         0x23
#define HX8347G_VCOM_CTRL2         0x24
#define HX8347G_VCOM_CTRL3         0x25
#define HX8347G_DISP_CTRL1         0x26
#define HX8347G_DISP_CTRL2         0x27
#define HX8347G_DISP_CTRL3         0x28
#define HX8347G_FRAMERATE_CTRL1    0x29
#define HX8347G_FRAMERATE_CTRL2    0x2A
#define HX8347G_FRAMERATE_CTRL3    0x2B
#define HX8347G_FRAMERATE_CTRL4    0x2C
#define HX8347G_CYCLE_CTRL1        0x2D
#define HX8347G_CYCLE_CTRL2        0x2E
#define HX8347G_DISP_INVERSION     0x2F
#define HX8347G_RGB_INTERFACE1     0x31
#define HX8347G_RGB_INTERFACE2     0x32
#define HX8347G_RGB_INTERFACE3     0x33
#define HX8347G_RGB_INTERFACE4     0x34

#define HX8347G_PANEL_CHAR         0x36
#define HX8347G_OTP_CTRL1          0x38
#define HX8347G_OTP_CTRL2          0x39
#define HX8347G_OTP_CTRL3          0x3A
#define HX8347G_OTP_CTRL4          0x3B
#define HX8347G_CABC_CTRL1         0x3C
#define HX8347G_CABC_CTRL2         0x3D
#define HX8347G_CABC_CTRL3         0x3E
#define HX8347G_CABC_CTRL4         0x3F
#define HX8347G_R1_CTRL_1          0x40
#define HX8347G_R1_CTRL_2          0x41
#define HX8347G_R1_CTRL_3          0x42
#define HX8347G_R1_CTRL_4          0x43
#define HX8347G_R1_CTRL_5          0x44
#define HX8347G_R1_CTRL_6          0x45
#define HX8347G_R1_CTRL_7          0x46
#define HX8347G_R1_CTRL_8          0x47
#define HX8347G_R1_CTRL_9          0x48
#define HX8347G_R1_CTRL_10         0x49
#define HX8347G_R1_CTRL_11         0x4A
#define HX8347G_R1_CTRL_12         0x4B
#define HX8347G_R1_CTRL_13         0x4C
#define HX8347G_R1_CTRL_14         0x50
#define HX8347G_R1_CTRL_15         0x51
#define HX8347G_R1_CTRL_16         0x52
#define HX8347G_R1_CTRL_17         0x53
#define HX8347G_R1_CTRL_18         0x54
#define HX8347G_R1_CTRL_19         0x55
#define HX8347G_R1_CTRL_20         0x56
#define HX8347G_R1_CTRL_21         0x57
#define HX8347G_R1_CTRL_22         0x58
#define HX8347G_R1_CTRL_23         0x59
#define HX8347G_R1_CTRL_24         0x5A
#define HX8347G_R1_CTRL_25         0x5B
#define HX8347G_R1_CTRL_26         0x5C
#define HX8347G_R1_CTRL_27         0x5D

#define HX8347G_TE_CTRL            0x60
#define HX8347G_ID1                0x61
#define HX8347G_ID2                0x62
#define HX8347G_ID3                0x63

#define HX8347G_TE_OUT_LINE        0x84
#define HX8347G_TE_OUT_LINE_HI     0x84
#define HX8347G_TE_OUT_LINE_LO     0x85

#define HX8347G_PWR_SAVING1        0xE4
#define HX8347G_PWR_SAVING2        0xE5
#define HX8347G_PWR_SAVING3        0xE6
#define HX8347G_PWR_SAVING4        0xE7
#define HX8347G_SRC_OP_CTRL_NORM   0xE8
#define HX8347G_SRC_OP_CTRL_IDLE   0xE9
#define HX8347G_PWR_CTRL_INT       0xEA
#define HX8347G_PWR_CTRL_INT_HI    0xEA
#define HX8347G_PWR_CTRL_INT_LO    0xEB
#define HX8347G_SRC_CTRL_INT       0xEC
#define HX8347G_SRC_CTRL_INT_HI    0xEC
#define HX8347G_SRC_CTRL_INT_LO    0xED

#define HX8347G_PAGE_SELECT        0xFF
#define HX8347G_SETGAMMA           0xE0
#define HX8347G_SETCE              0xE5
#define HX8347G_SETVMF             0xFD

#define HX8347G_ENTRY_RGB          0x00
#define HX8347G_ENTRY_BGR          0x08

#define HX8347G_ENTRY_VERTICAL     0x20
#define HX8347G_ENTRY_X_LEFT       0x40
#define HX8347G_ENTRY_X_RIGHT      0x00
#define HX8347G_ENTRY_Y_UP         0x80
#define HX8347G_ENTRY_Y_DOWN       0x00

#if HX8347G_COLORMODE == 0
#define HX8347G_ENTRY_COLORMODE    HX8347G_ENTRY_RGB
#else
#define HX8347G_ENTRY_COLORMODE    HX8347G_ENTRY_BGR
#endif

#if (HX8347G_ORIENTATION == 0)
#define HX8347G_MAX_X                        (HX8347G_LCD_PIXEL_WIDTH - 1)
#define HX8347G_MAX_Y                        (HX8347G_LCD_PIXEL_HEIGHT - 1)
#define HX8347G_ENTRY_DATA_RIGHT_THEN_DOWN   (HX8347G_ENTRY_COLORMODE | HX8347G_ENTRY_X_RIGHT | HX8347G_ENTRY_Y_DOWN)
#define HX8347G_ENTRY_DATA_RIGHT_THEN_UP     (HX8347G_ENTRY_COLORMODE | HX8347G_ENTRY_X_RIGHT | HX8347G_ENTRY_Y_UP)
#elif (HX8347G_ORIENTATION == 1)
#define HX8347G_MAX_X                        (HX8347G_LCD_PIXEL_HEIGHT - 1)
#define HX8347G_MAX_Y                        (HX8347G_LCD_PIXEL_WIDTH - 1)
#define HX8347G_ENTRY_DATA_RIGHT_THEN_DOWN   (HX8347G_ENTRY_COLORMODE | HX8347G_ENTRY_X_LEFT  | HX8347G_ENTRY_Y_DOWN | HX8347G_ENTRY_VERTICAL)
#define HX8347G_ENTRY_DATA_RIGHT_THEN_UP     (HX8347G_ENTRY_COLORMODE | HX8347G_ENTRY_X_RIGHT | HX8347G_ENTRY_Y_DOWN | HX8347G_ENTRY_VERTICAL)
#elif (HX8347G_ORIENTATION == 2)
#define HX8347G_MAX_X                        (HX8347G_LCD_PIXEL_WIDTH - 1)
#define HX8347G_MAX_Y                        (HX8347G_LCD_PIXEL_HEIGHT - 1)
#define HX8347G_ENTRY_DATA_RIGHT_THEN_DOWN   (HX8347G_ENTRY_COLORMODE | HX8347G_ENTRY_X_LEFT  | HX8347G_ENTRY_Y_UP)
#define HX8347G_ENTRY_DATA_RIGHT_THEN_UP     (HX8347G_ENTRY_COLORMODE | HX8347G_ENTRY_X_LEFT  | HX8347G_ENTRY_Y_DOWN)
#elif (HX8347G_ORIENTATION == 3)
#define HX8347G_MAX_X                        (HX8347G_LCD_PIXEL_HEIGHT - 1)
#define HX8347G_MAX_Y                        (HX8347G_LCD_PIXEL_WIDTH - 1)
#define HX8347G_ENTRY_DATA_RIGHT_THEN_DOWN   (HX8347G_ENTRY_COLORMODE | HX8347G_ENTRY_X_RIGHT | HX8347G_ENTRY_Y_UP   | HX8347G_ENTRY_VERTICAL)
#define HX8347G_ENTRY_DATA_RIGHT_THEN_UP     (HX8347G_ENTRY_COLORMODE | HX8347G_ENTRY_X_LEFT  | HX8347G_ENTRY_Y_UP   | HX8347G_ENTRY_VERTICAL)
#endif

#define HX8347G_SETCURSOR(x, y)              {hx8347g_WriteRegPair(HX8347G_HOR_START_AD, x); \
                                              hx8347g_WriteRegPair(HX8347G_HOR_END_AD, x);   \
                                              hx8347g_WriteRegPair(HX8347G_VER_START_AD, y); \
                                              hx8347g_WriteRegPair(HX8347G_VER_END_AD, y);   }

#ifndef LCD_REVERSE16
#define LCD_REVERSE16    0
#endif

#define HX8347G_LCD_INITIALIZED    0x01
#define HX8347G_IO_INITIALIZED     0x02
static  uint8_t   Is_hx8347g_Initialized = 0;

const uint8_t EntryRightThenUp = HX8347G_ENTRY_DATA_RIGHT_THEN_UP;
const uint8_t EntryRightThenDown = HX8347G_ENTRY_DATA_RIGHT_THEN_DOWN;

/* the last set drawing direction is stored here */
uint16_t LastEntry = HX8347G_ENTRY_DATA_RIGHT_THEN_DOWN;

static  uint16_t  yStart, yEnd;

//-----------------------------------------------------------------------------


#if HX8347G_WRITEBITDEPTH == 16
#if LCD_REVERSE16 == 0
#define  LCD_IO_DrawFill(Color, Size) { \
  LCD_IO_WriteCmd8DataFill16(HX8347G_RW_GRAM, Color, Size); }           /* Fill 16 bit pixel(s) */
#define  LCD_IO_DrawBitmap(pData, Size) { \
  LCD_IO_WriteCmd8MultipleData16(HX8347G_RW_GRAM, pData, Size); }       /* Draw 16 bit bitmap */
#elif LCD_REVERSE16 == 1
#define  LCD_IO_DrawFill(Color, Size) { \
  LCD_IO_WriteCmd8DataFill16r(HX8347G_RW_GRAM, Color, Size); }          /* Fill 16 bit pixel(s) */
#define  LCD_IO_DrawBitmap(pData, Size) { \
  LCD_IO_WriteCmd8MultipleData16r(HX8347G_RW_GRAM, pData, Size); }      /* Draw 16 bit bitmap */
#endif
#elif HX8347G_WRITEBITDEPTH == 24
#define  LCD_IO_DrawFill(Color, Size) { \
  LCD_IO_WriteCmd8DataFill16to24(HX8347G_RW_GRAM, Color, Size); }       /* Fill 24 bit pixel(s) from 16 bit color code */
#define  LCD_IO_DrawBitmap(pData, Size) { \
  LCD_IO_WriteCmd8MultipleData16to24(HX8347G_RW_GRAM, pData, Size); }   /* Draw 24 bit Lcd bitmap from 16 bit bitmap data */
#endif

/* Reading works in a fixed 24-bit bit depth, it is not necessary to change the COLMOD register */
#define  LCD_IO_ReadBitmap(pData, Size) { \
  LCD_IO_ReadCmd8MultipleData24to16(HX8347G_RW_GRAM, pData, Size, 1); } /* Read 24 bit Lcd and convert to 16 bit bitmap */


//-----------------------------------------------------------------------------
void hx8347g_WriteRegPair(uint8_t CmdPair, uint16_t Data)
{
  LCD_IO_WriteCmd8MultipleData8(CmdPair, ((uint8_t *)&Data + 1), 1);
  LCD_IO_WriteCmd8MultipleData8(CmdPair + 1, ((uint8_t *)&Data), 1);
}

//-----------------------------------------------------------------------------
void hx8347g_Init(void)
{
  if((Is_hx8347g_Initialized & HX8347G_LCD_INITIALIZED) == 0)
  {
    Is_hx8347g_Initialized |= HX8347G_LCD_INITIALIZED;
    if((Is_hx8347g_Initialized & HX8347G_IO_INITIALIZED) == 0)
      LCD_IO_Init();
    Is_hx8347g_Initialized |= HX8347G_IO_INITIALIZED;

    LCD_IO_WriteCmd8MultipleData8(0xF3, "\x08", 1);

    LCD_Delay(5);

    LCD_IO_WriteCmd8MultipleData8(HX8347G_CYCLE_CTRL2, "\x89", 1);
    LCD_IO_WriteCmd8MultipleData8(HX8347G_FRAMERATE_CTRL1, "\x8F", 1);
    LCD_IO_WriteCmd8MultipleData8(HX8347G_FRAMERATE_CTRL3, "\x02", 1);
    LCD_IO_WriteCmd8MultipleData8(0xE2, "\x00", 1);
    LCD_IO_WriteCmd8MultipleData8(HX8347G_PWR_SAVING1, "\x01", 1);
    LCD_IO_WriteCmd8MultipleData8(HX8347G_PWR_SAVING2, "\x10", 1);
    LCD_IO_WriteCmd8MultipleData8(HX8347G_PWR_SAVING3, "\x01", 1);
    LCD_IO_WriteCmd8MultipleData8(HX8347G_PWR_SAVING4, "\x10", 1);
    LCD_IO_WriteCmd8MultipleData8(HX8347G_SRC_OP_CTRL_NORM, "\x70", 1);
    LCD_IO_WriteCmd8MultipleData8(0xF2, "\x00", 1);

    hx8347g_WriteRegPair(HX8347G_PWR_CTRL_INT, 0x0020);
    hx8347g_WriteRegPair(HX8347G_SRC_CTRL_INT, 0x3CC8);
    LCD_IO_WriteCmd8MultipleData8(HX8347G_SRC_OP_CTRL_IDLE, "\x38", 1);
    LCD_IO_WriteCmd8MultipleData8(0xF1, "\x01", 1);

    // skip gamma, do later
    LCD_IO_WriteCmd8MultipleData8(HX8347G_PWR_CTRL2, "\x1A", 1);
    LCD_IO_WriteCmd8MultipleData8(HX8347G_PWR_CTRL1, "\x02", 1);
    LCD_IO_WriteCmd8MultipleData8(HX8347G_VCOM_CTRL2, "\x61", 1);
    LCD_IO_WriteCmd8MultipleData8(HX8347G_VCOM_CTRL3, "\x5C", 1);

    LCD_IO_WriteCmd8MultipleData8(HX8347G_OSC_CTRL2, "\x36", 1);
    LCD_IO_WriteCmd8MultipleData8(HX8347G_OSC_CTRL2, "\x01", 1);
    LCD_IO_WriteCmd8MultipleData8(HX8347G_PWR_CTRL6, "\x88", 1);
    LCD_Delay(5);
    LCD_IO_WriteCmd8MultipleData8(HX8347G_PWR_CTRL6, "\x80", 1);
    LCD_Delay(5);
    LCD_IO_WriteCmd8MultipleData8(HX8347G_PWR_CTRL6, "\x90", 1);
    LCD_Delay(5);
    LCD_IO_WriteCmd8MultipleData8(HX8347G_PWR_CTRL6, "\xD4", 1);
    LCD_Delay(5);
    #if HX8347G_WRITEBITDEPTH == 16
    LCD_IO_WriteCmd8MultipleData8(HX8347G_COLMOD, "\x55", 1); // 16bit/pixel
    #elif HX8347G_WRITEBITDEPTH == 24
    LCD_IO_WriteCmd8MultipleData8(HX8347G_COLMOD, "\x66", 1); // 16bit/pixel
    #endif

    LCD_IO_WriteCmd8MultipleData8(HX8347G_PANEL_CHAR, "\x09", 1);
    LCD_IO_WriteCmd8MultipleData8(HX8347G_DISP_CTRL3, "\x38", 1);
    LCD_Delay(40);
    LCD_IO_WriteCmd8MultipleData8(HX8347G_DISP_CTRL3, "\x3C", 1);

    LCD_IO_WriteCmd8MultipleData8(HX8347G_ENTRY_MOD, &EntryRightThenDown, 1);
    LCD_IO_WriteCmd8MultipleData8(HX8347G_MODE_CTRL, "\x08", 1);
  }
}

//-----------------------------------------------------------------------------
/**
  * @brief  Enables the Display.
  * @param  None
  * @retval None
  */
void hx8347g_DisplayOn(void)
{
}

//-----------------------------------------------------------------------------
/**
  * @brief  Disables the Display.
  * @param  None
  * @retval None
  */
void hx8347g_DisplayOff(void)
{
}

//-----------------------------------------------------------------------------
/**
  * @brief  Get the LCD pixel Width.
  * @param  None
  * @retval The Lcd Pixel Width
  */
uint16_t hx8347g_GetLcdPixelWidth(void)
{
  return HX8347G_MAX_X + 1;
}

//-----------------------------------------------------------------------------
/**
  * @brief  Get the LCD pixel Height.
  * @param  None
  * @retval The Lcd Pixel Height
  */
uint16_t hx8347g_GetLcdPixelHeight(void)
{
  return HX8347G_MAX_Y + 1;
}

//-----------------------------------------------------------------------------
/**
  * @brief  Get the HX8347G ID.
  * @param  None
  * @retval The HX8347G ID
  */
uint32_t hx8347g_ReadID(void)
{
  uint32_t ret = 0;
  if((Is_hx8347g_Initialized & HX8347G_IO_INITIALIZED) == 0)
  {
    LCD_IO_Init();
  }
  LCD_IO_ReadCmd8MultipleData8(HX8347G_ID_AD, (uint8_t *)&ret, 2, 1);
  return ret;
}

//-----------------------------------------------------------------------------
/**
  * @brief  Set Cursor position.
  * @param  Xpos: specifies the X position.
  * @param  Ypos: specifies the Y position.
  * @retval None
  */
void hx8347g_SetCursor(uint16_t Xpos, uint16_t Ypos)
{
  HX8347G_SETCURSOR(Xpos, Ypos);
}

//-----------------------------------------------------------------------------
/**
  * @brief  Write pixel.
  * @param  Xpos: specifies the X position.
  * @param  Ypos: specifies the Y position.
  * @param  RGBCode: the RGB pixel color
  * @retval None
  */
void hx8347g_WritePixel(uint16_t Xpos, uint16_t Ypos, uint16_t RGBCode)
{
  if(LastEntry != HX8347G_ENTRY_DATA_RIGHT_THEN_DOWN)
  {
    LastEntry = HX8347G_ENTRY_DATA_RIGHT_THEN_DOWN;
    LCD_IO_WriteCmd8MultipleData8(HX8347G_ENTRY_MOD, &EntryRightThenDown, 1);
  }
  HX8347G_SETCURSOR(Xpos, Ypos);
  LCD_IO_DrawFill(RGBCode, 1);
}

//-----------------------------------------------------------------------------
/**
  * @brief  Read pixel.
  * @param  None
  * @retval the RGB pixel color
  */
uint16_t hx8347g_ReadPixel(uint16_t Xpos, uint16_t Ypos)
{
  uint16_t ret;
  if(LastEntry != HX8347G_ENTRY_DATA_RIGHT_THEN_DOWN)
  {
    LastEntry = HX8347G_ENTRY_DATA_RIGHT_THEN_DOWN;
    LCD_IO_WriteCmd8MultipleData8(HX8347G_ENTRY_MOD, &EntryRightThenDown, 1);
  }
  HX8347G_SETCURSOR(Xpos, Ypos);
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
void hx8347g_SetDisplayWindow(uint16_t Xpos, uint16_t Ypos, uint16_t Width, uint16_t Height)
{
  yStart = Ypos; yEnd = Ypos + Height - 1;
  hx8347g_WriteRegPair(HX8347G_HOR_START_AD, Xpos);
  hx8347g_WriteRegPair(HX8347G_HOR_END_AD, Xpos + Width - 1);
  hx8347g_WriteRegPair(HX8347G_VER_START_AD, Ypos);
  hx8347g_WriteRegPair(HX8347G_VER_END_AD, Ypos + Height - 1);
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
void hx8347g_DrawHLine(uint16_t RGBCode, uint16_t Xpos, uint16_t Ypos, uint16_t Length)
{
  if(LastEntry != HX8347G_ENTRY_DATA_RIGHT_THEN_DOWN)
  {
    LastEntry = HX8347G_ENTRY_DATA_RIGHT_THEN_DOWN;
    LCD_IO_WriteCmd8MultipleData8(HX8347G_ENTRY_MOD, &EntryRightThenDown, 1);
  }
  hx8347g_WriteRegPair(HX8347G_HOR_START_AD, Xpos);
  hx8347g_WriteRegPair(HX8347G_HOR_END_AD, Xpos + Length - 1);
  hx8347g_WriteRegPair(HX8347G_VER_START_AD, Ypos);
  hx8347g_WriteRegPair(HX8347G_VER_END_AD, Ypos);
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
void hx8347g_DrawVLine(uint16_t RGBCode, uint16_t Xpos, uint16_t Ypos, uint16_t Length)
{
  if(LastEntry != HX8347G_ENTRY_DATA_RIGHT_THEN_DOWN)
  {
    LastEntry = HX8347G_ENTRY_DATA_RIGHT_THEN_DOWN;
    LCD_IO_WriteCmd8MultipleData8(HX8347G_ENTRY_MOD, &EntryRightThenDown, 1);
  }
  hx8347g_WriteRegPair(HX8347G_HOR_START_AD, Xpos);
  hx8347g_WriteRegPair(HX8347G_HOR_END_AD, Xpos);
  hx8347g_WriteRegPair(HX8347G_VER_START_AD, Ypos);
  hx8347g_WriteRegPair(HX8347G_VER_END_AD, Ypos + Length - 1);
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
void hx8347g_FillRect(uint16_t Xpos, uint16_t Ypos, uint16_t Xsize, uint16_t Ysize, uint16_t RGBCode)
{
  if(LastEntry != HX8347G_ENTRY_DATA_RIGHT_THEN_DOWN)
  {
    LastEntry = HX8347G_ENTRY_DATA_RIGHT_THEN_DOWN;
    LCD_IO_WriteCmd8MultipleData8(HX8347G_ENTRY_MOD, &EntryRightThenDown, 1);
  }
  hx8347g_SetDisplayWindow(Xpos, Ypos, Xsize, Ysize);
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
void hx8347g_DrawBitmap(uint16_t Xpos, uint16_t Ypos, uint8_t *pbmp)
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

  if(LastEntry != HX8347G_ENTRY_DATA_RIGHT_THEN_UP)
  {
    LastEntry = HX8347G_ENTRY_DATA_RIGHT_THEN_UP;
    LCD_IO_WriteCmd8MultipleData8(HX8347G_ENTRY_MOD, &EntryRightThenUp, 1);
  }
  hx8347g_WriteRegPair(HX8347G_VER_START_AD, HX8347G_MAX_Y - yEnd);
  hx8347g_WriteRegPair(HX8347G_VER_END_AD, HX8347G_MAX_Y - yStart);
  LCD_IO_DrawBitmap((uint16_t *)pbmp, size);
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
void hx8347g_DrawRGBImage(uint16_t Xpos, uint16_t Ypos, uint16_t Xsize, uint16_t Ysize, uint16_t *pdata)
{
  if(LastEntry != HX8347G_ENTRY_DATA_RIGHT_THEN_DOWN)
  {
    LastEntry = HX8347G_ENTRY_DATA_RIGHT_THEN_DOWN;
    LCD_IO_WriteCmd8MultipleData8(HX8347G_ENTRY_MOD, &EntryRightThenDown, 1);
  }
  hx8347g_SetDisplayWindow(Xpos, Ypos, Xsize, Ysize);
  LCD_IO_DrawBitmap(pdata, Xsize * Ysize);
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
void hx8347g_ReadRGBImage(uint16_t Xpos, uint16_t Ypos, uint16_t Xsize, uint16_t Ysize, uint16_t *pdata)
{
  if(LastEntry != HX8347G_ENTRY_DATA_RIGHT_THEN_DOWN)
  {
    LastEntry = HX8347G_ENTRY_DATA_RIGHT_THEN_DOWN;
    LCD_IO_WriteCmd8MultipleData8(HX8347G_ENTRY_MOD, &EntryRightThenDown, 1);
  }
  hx8347g_SetDisplayWindow(Xpos, Ypos, Xsize, Ysize);
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
void hx8347g_Scroll(int16_t Scroll, uint16_t TopFix, uint16_t BottonFix)
{
  static uint16_t scrparam[4] = {0, 0, 0, 0};
  #if (HX8347G_ORIENTATION == 0)
  if((TopFix != scrparam[1]) || (BottonFix != scrparam[3]))
  {
    scrparam[1] = TopFix;
    scrparam[3] = BottonFix;
    scrparam[2] = HX8347G_LCD_PIXEL_HEIGHT - TopFix - BottonFix;
    hx8347g_WriteRegPair(HX8347G_VER_SCR_TOP, scrparam[1]);
    hx8347g_WriteRegPair(HX8347G_VER_SCR_HEIGHT, scrparam[2]);
    hx8347g_WriteRegPair(HX8347G_VER_SCR_BTN, scrparam[3]);
  }
  Scroll = (0 - Scroll) % scrparam[2];
  if(Scroll < 0)
    Scroll = scrparam[2] + Scroll + scrparam[1];
  else
    Scroll = Scroll + scrparam[1];
  #elif (HX8347G_ORIENTATION == 1)
  if((TopFix != scrparam[1]) || (BottonFix != scrparam[3]))
  {
    scrparam[1] = TopFix;
    scrparam[3] = BottonFix;
    scrparam[2] = HX8347G_LCD_PIXEL_HEIGHT - TopFix - BottonFix;
    hx8347g_WriteRegPair(HX8347G_VER_SCR_TOP, scrparam[1]);
    hx8347g_WriteRegPair(HX8347G_VER_SCR_HEIGHT, scrparam[2]);
    hx8347g_WriteRegPair(HX8347G_VER_SCR_BTN, scrparam[3]);
  }
  Scroll = (0 - Scroll) % scrparam[2];
  if(Scroll < 0)
    Scroll = scrparam[2] + Scroll + scrparam[1];
  else
    Scroll = Scroll + scrparam[1];
  #elif (HX8347G_ORIENTATION == 2)
  if((TopFix != scrparam[3]) || (BottonFix != scrparam[1]))
  {
    scrparam[3] = TopFix;
    scrparam[1] = BottonFix;
    scrparam[2] = HX8347G_LCD_PIXEL_HEIGHT - TopFix - BottonFix;
    hx8347g_WriteRegPair(HX8347G_VER_SCR_TOP, scrparam[1]);
    hx8347g_WriteRegPair(HX8347G_VER_SCR_HEIGHT, scrparam[2]);
    hx8347g_WriteRegPair(HX8347G_VER_SCR_BTN, scrparam[3]);
  }
  Scroll %= scrparam[2];
  if(Scroll < 0)
    Scroll = scrparam[2] + Scroll + scrparam[1];
  else
    Scroll = Scroll + scrparam[1];
  #elif (HX8347G_ORIENTATION == 3)
  if((TopFix != scrparam[3]) || (BottonFix != scrparam[1]))
  {
    scrparam[3] = TopFix;
    scrparam[1] = BottonFix;
    scrparam[2] = HX8347G_LCD_PIXEL_HEIGHT - TopFix - BottonFix;
    hx8347g_WriteRegPair(HX8347G_VER_SCR_TOP, scrparam[1]);
    hx8347g_WriteRegPair(HX8347G_VER_SCR_HEIGHT, scrparam[2]);
    hx8347g_WriteRegPair(HX8347G_VER_SCR_BTN, scrparam[3]);
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
    hx8347g_WriteRegPair(HX8347G_VER_SCR_START, scrparam[0]);
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
void hx8347g_UserCommand(uint16_t Command, uint8_t* pData, uint32_t Size, uint8_t Mode)
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
