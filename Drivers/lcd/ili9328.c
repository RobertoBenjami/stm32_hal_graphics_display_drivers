#include "main.h"
#include "lcd.h"
#include "lcd_io.h"
#include "ili9328.h"

void     ili9328_Init(void);
uint32_t ili9328_ReadID(void);
void     ili9328_DisplayOn(void);
void     ili9328_DisplayOff(void);
void     ili9328_SetCursor(uint16_t Xpos, uint16_t Ypos);
void     ili9328_WritePixel(uint16_t Xpos, uint16_t Ypos, uint16_t RGB_Code);
uint16_t ili9328_ReadPixel(uint16_t Xpos, uint16_t Ypos);
void     ili9328_SetDisplayWindow(uint16_t Xpos, uint16_t Ypos, uint16_t Width, uint16_t Height);
void     ili9328_DrawHLine(uint16_t RGBCode, uint16_t Xpos, uint16_t Ypos, uint16_t Length);
void     ili9328_DrawVLine(uint16_t RGBCode, uint16_t Xpos, uint16_t Ypos, uint16_t Length);
uint16_t ili9328_GetLcdPixelWidth(void);
uint16_t ili9328_GetLcdPixelHeight(void);
void     ili9328_DrawBitmap(uint16_t Xpos, uint16_t Ypos, uint8_t *pbmp);
void     ili9328_DrawRGBImage(uint16_t Xpos, uint16_t Ypos, uint16_t Xsize, uint16_t Ysize, uint16_t *pdata);
void     ili9328_ReadRGBImage(uint16_t Xpos, uint16_t Ypos, uint16_t Xsize, uint16_t Ysize, uint16_t *pdata);
void     ili9328_FillRect(uint16_t Xpos, uint16_t Ypos, uint16_t Xsize, uint16_t Ysize, uint16_t RGBCode);
void     ili9328_Scroll(int16_t Scroll, uint16_t TopFix, uint16_t BottonFix);
void     ili9328_UserCommand(uint16_t Command, uint8_t * pData, uint32_t Size, uint8_t Mode);

LCD_DrvTypeDef   ili9328_drv =
{
  ili9328_Init,
  ili9328_ReadID,
  ili9328_DisplayOn,
  ili9328_DisplayOff,
  ili9328_SetCursor,
  ili9328_WritePixel,
  ili9328_ReadPixel,
  ili9328_SetDisplayWindow,
  ili9328_DrawHLine,
  ili9328_DrawVLine,
  ili9328_GetLcdPixelWidth,
  ili9328_GetLcdPixelHeight,
  ili9328_DrawBitmap,
  ili9328_DrawRGBImage,
  ili9328_FillRect,
  ili9328_ReadRGBImage,
  ili9328_Scroll,
  ili9328_UserCommand
};

LCD_DrvTypeDef  *lcd_drv = &ili9328_drv;
/* transaction data */
#define TRANSDATAMAXSIZE  4
union
{
  char       c[TRANSDATAMAXSIZE];
  uint8_t   d8[TRANSDATAMAXSIZE];
  uint16_t d16[TRANSDATAMAXSIZE / 2];
}transdata;

#define ILI9328_START_OSC          0x00
#define ILI9328_DRIV_OUT_CTRL      0x01
#define ILI9328_DRIV_WAV_CTRL      0x02
#define ILI9328_ENTRY_MOD          0x03
#define ILI9328_RESIZE_CTRL        0x04
#define ILI9328_DISP_CTRL1         0x07
#define ILI9328_DISP_CTRL2         0x08
#define ILI9328_DISP_CTRL3         0x09
#define ILI9328_DISP_CTRL4         0x0A
#define ILI9328_RGB_DISP_IF_CTRL1  0x0C
#define ILI9328_FRM_MARKER_POS     0x0D
#define ILI9328_RGB_DISP_IF_CTRL2  0x0F
#define ILI9328_POW_CTRL1          0x10
#define ILI9328_POW_CTRL2          0x11
#define ILI9328_POW_CTRL3          0x12
#define ILI9328_POW_CTRL4          0x13
#define ILI9328_GRAM_HOR_AD        0x20
#define ILI9328_GRAM_VER_AD        0x21
#define ILI9328_RW_GRAM            0x22
#define ILI9328_POW_CTRL7          0x29
#define ILI9328_FRM_RATE_COL_CTRL  0x2B
#define ILI9328_GAMMA_CTRL1        0x30
#define ILI9328_GAMMA_CTRL2        0x31
#define ILI9328_GAMMA_CTRL3        0x32
#define ILI9328_GAMMA_CTRL4        0x35
#define ILI9328_GAMMA_CTRL5        0x36
#define ILI9328_GAMMA_CTRL6        0x37
#define ILI9328_GAMMA_CTRL7        0x38
#define ILI9328_GAMMA_CTRL8        0x39
#define ILI9328_GAMMA_CTRL9        0x3C
#define ILI9328_GAMMA_CTRL10       0x3D
#define ILI9328_HOR_START_AD       0x50
#define ILI9328_HOR_END_AD         0x51
#define ILI9328_VER_START_AD       0x52
#define ILI9328_VER_END_AD         0x53
#define ILI9328_GATE_SCAN_CTRL1    0x60
#define ILI9328_GATE_SCAN_CTRL2    0x61
#define ILI9328_GATE_SCAN_CTRL3    0x6A
#define ILI9328_PART_IMG1_DISP_POS 0x80
#define ILI9328_PART_IMG1_START_AD 0x81
#define ILI9328_PART_IMG1_END_AD   0x82
#define ILI9328_PART_IMG2_DISP_POS 0x83
#define ILI9328_PART_IMG2_START_AD 0x84
#define ILI9328_PART_IMG2_END_AD   0x85
#define ILI9328_PANEL_IF_CTRL1     0x90
#define ILI9328_PANEL_IF_CTRL2     0x92
#define ILI9328_PANEL_IF_CTRL3     0x93
#define ILI9328_PANEL_IF_CTRL4     0x95
#define ILI9328_PANEL_IF_CTRL5     0x97
#define ILI9328_PANEL_IF_CTRL6     0x98

#define ILI9328_ENTRY_18BITCOLOR   0x8000
#define ILI9328_ENTRY_18BITBCD     0x4000

#define ILI9328_ENTRY_RGB          0x1000
#define ILI9328_ENTRY_BGR          0x0000

#define ILI9328_ENTRY_VERTICAL     0x0008
#define ILI9328_ENTRY_X_LEFT       0x0000
#define ILI9328_ENTRY_X_RIGHT      0x0010
#define ILI9328_ENTRY_Y_UP         0x0000
#define ILI9328_ENTRY_Y_DOWN       0x0020

#if ILI9328_COLORMODE == 0
#define ILI9328_ENTRY_COLORMODE    ILI9328_ENTRY_RGB
#elif ILI9328_COLORMODE == 1
#define ILI9328_ENTRY_COLORMODE    ILI9328_ENTRY_BGR
#endif

#if ILI9328_ORIENTATION == 0
#define ILI9328_ENTRY_DATA_RIGHT_THEN_UP     (ILI9328_ENTRY_COLORMODE | ILI9328_ENTRY_X_RIGHT | ILI9328_ENTRY_Y_UP)
#define ILI9328_ENTRY_DATA_RIGHT_THEN_DOWN   (ILI9328_ENTRY_COLORMODE | ILI9328_ENTRY_X_RIGHT | ILI9328_ENTRY_Y_DOWN)
#define ILI9328_ENTRY_DATA_DOWN_THEN_RIGHT   (ILI9328_ENTRY_COLORMODE | ILI9328_ENTRY_X_RIGHT | ILI9328_ENTRY_Y_DOWN | ILI9328_ENTRY_VERTICAL)
#define ILI9328_DRIV_OUT_CTRL_DATA           0x0100
#define ILI9328_GATE_SCAN_CTRL1_DATA         0xA700
#define ILI9328_SETCURSOR(x, y)              {transdata.d16[0] = x;\
                                              LCD_IO_WriteCmd16MultipleData16(ILI9328_GRAM_HOR_AD, &transdata, 1);\
                                              transdata.d16[0] = y;\
                                              LCD_IO_WriteCmd16MultipleData16(ILI9328_GRAM_VER_AD, &transdata, 1);}
#elif ILI9328_ORIENTATION == 1
#define ILI9328_ENTRY_DATA_RIGHT_THEN_UP     (ILI9328_ENTRY_COLORMODE | ILI9328_ENTRY_X_LEFT  | ILI9328_ENTRY_Y_DOWN | ILI9328_ENTRY_VERTICAL)
#define ILI9328_ENTRY_DATA_RIGHT_THEN_DOWN   (ILI9328_ENTRY_COLORMODE | ILI9328_ENTRY_X_RIGHT | ILI9328_ENTRY_Y_DOWN | ILI9328_ENTRY_VERTICAL)
#define ILI9328_ENTRY_DATA_DOWN_THEN_RIGHT   (ILI9328_ENTRY_COLORMODE | ILI9328_ENTRY_X_RIGHT | ILI9328_ENTRY_Y_DOWN)
#define ILI9328_DRIV_OUT_CTRL_DATA           0x0000
#define ILI9328_GATE_SCAN_CTRL1_DATA         0xA700
#define ILI9328_SETCURSOR(x, y)              {transdata.d16[0] = y;\
                                              LCD_IO_WriteCmd16MultipleData16(ILI9328_GRAM_HOR_AD, &transdata, 1);\
                                              transdata.d16[0] = x;\
                                              LCD_IO_WriteCmd16MultipleData16(ILI9328_GRAM_VER_AD, &transdata, 1);}
#elif ILI9328_ORIENTATION == 2
#define ILI9328_ENTRY_DATA_RIGHT_THEN_UP     (ILI9328_ENTRY_COLORMODE | ILI9328_ENTRY_X_RIGHT | ILI9328_ENTRY_Y_UP)
#define ILI9328_ENTRY_DATA_RIGHT_THEN_DOWN   (ILI9328_ENTRY_COLORMODE | ILI9328_ENTRY_X_RIGHT | ILI9328_ENTRY_Y_DOWN)
#define ILI9328_ENTRY_DATA_DOWN_THEN_RIGHT   (ILI9328_ENTRY_COLORMODE | ILI9328_ENTRY_X_RIGHT | ILI9328_ENTRY_Y_DOWN | ILI9328_ENTRY_VERTICAL)
#define ILI9328_DRIV_OUT_CTRL_DATA           0x0000
#define ILI9328_GATE_SCAN_CTRL1_DATA         0x2700
#define ILI9328_SETCURSOR(x, y)              {transdata.d16[0] = x;\
                                              LCD_IO_WriteCmd16MultipleData16(ILI9328_GRAM_HOR_AD, &transdata, 1);\
                                              transdata.d16[0] = y;\
                                              LCD_IO_WriteCmd16MultipleData16(ILI9328_GRAM_VER_AD, &transdata, 1);}
#elif ILI9328_ORIENTATION == 3
#define ILI9328_ENTRY_DATA_RIGHT_THEN_UP     (ILI9328_ENTRY_COLORMODE | ILI9328_ENTRY_X_LEFT  | ILI9328_ENTRY_Y_DOWN | ILI9328_ENTRY_VERTICAL)
#define ILI9328_ENTRY_DATA_RIGHT_THEN_DOWN   (ILI9328_ENTRY_COLORMODE | ILI9328_ENTRY_X_RIGHT | ILI9328_ENTRY_Y_DOWN | ILI9328_ENTRY_VERTICAL)
#define ILI9328_ENTRY_DATA_DOWN_THEN_RIGHT   (ILI9328_ENTRY_COLORMODE | ILI9328_ENTRY_X_RIGHT | ILI9328_ENTRY_Y_DOWN)
#define ILI9328_DRIV_OUT_CTRL_DATA           0x0100
#define ILI9328_GATE_SCAN_CTRL1_DATA         0x2700
#define ILI9328_SETCURSOR(x, y)              {transdata.d16[0] = y;\
                                              LCD_IO_WriteCmd16MultipleData16(ILI9328_GRAM_HOR_AD, &transdata, 1);\
                                              transdata.d16[0] = x;\
                                              LCD_IO_WriteCmd16MultipleData16(ILI9328_GRAM_VER_AD, &transdata, 1);}
#endif

#ifndef LCD_REVERSE16
#define LCD_REVERSE16    0
#endif

#if LCD_REVERSE16 == 0
#if ILI9328_COLORMODE == 0
#define  RGB565TOBGR565(rgb16)  rgb16 = __REVSH((rgb16 & 0xE007) | ((rgb16 & 0x00F8) << 5) | ((rgb16 & 0x1F00) >> 5))
#elif ILI9328_COLORMODE == 1
#define  RGB565TOBGR565(rgb16)  rgb16 = __REVSH(rgb16)
#endif
#elif LCD_REVERSE16 == 1
#if ILI9328_COLORMODE == 0
#define  RGB565TOBGR565(rgb16)  rgb16 = ((rgb16 & 0xE007) | ((rgb16 & 0x00F8) << 5) | ((rgb16 & 0x1F00) >> 5))
#elif ILI9328_COLORMODE == 1
#define  RGB565TOBGR565(rgb16)
#endif
#endif

#define ILI9328_LCD_INITIALIZED    0x01
#define ILI9328_IO_INITIALIZED     0x02
static  uint8_t   Is_ili9328_Initialized = 0;

const uint16_t DrivOutCtrlData = ILI9328_DRIV_OUT_CTRL_DATA;
const uint16_t EntryRightThenUp = ILI9328_ENTRY_DATA_RIGHT_THEN_UP;
const uint16_t EntryRightThenDown = ILI9328_ENTRY_DATA_RIGHT_THEN_DOWN;
const uint16_t EntryDownThenRight = ILI9328_ENTRY_DATA_DOWN_THEN_RIGHT;
const uint16_t LcdPixelWidth = ILI9328_LCD_PIXEL_WIDTH - 1;
const uint16_t LcdPixelHeight = ILI9328_LCD_PIXEL_HEIGHT - 1;
const uint16_t GateScanCtrl1Data = ILI9328_GATE_SCAN_CTRL1_DATA;

/* the last set drawing direction is stored here */
uint16_t LastEntry = ILI9328_ENTRY_DATA_RIGHT_THEN_DOWN;

//-----------------------------------------------------------------------------
/* Pixel draw and read functions */
#if LCD_REVERSE16 == 0
#define  LCD_IO_DrawFill(Color, Size) \
  LCD_IO_WriteCmd16DataFill16(ILI9328_RW_GRAM, Color, Size)
#define  LCD_IO_DrawBitmap(pData, Size) \
  LCD_IO_WriteCmd16MultipleData16(ILI9328_RW_GRAM, pData, Size)
#elif LCD_REVERSE16 == 1
#define  LCD_IO_DrawFill(Color, Size) \
  LCD_IO_WriteCmd16DataFill16r(ILI9328_RW_GRAM, Color, Size)
#define  LCD_IO_DrawBitmap(pData, Size) \
  LCD_IO_WriteCmd16MultipleData16r(ILI9328_RW_GRAM, pData, Size)
#endif
#define  LCD_IO_ReadBitmap(pData, Size) \
  LCD_IO_ReadCmd16MultipleData16(ILI9328_RW_GRAM, pData, Size, 2)

//-----------------------------------------------------------------------------
/* Init the Display and init the IO */
void ili9328_Init(void)
{
  if((Is_ili9328_Initialized & ILI9328_LCD_INITIALIZED) == 0)
  {
    Is_ili9328_Initialized |= ILI9328_LCD_INITIALIZED;
    if((Is_ili9328_Initialized & ILI9328_IO_INITIALIZED) == 0)
      LCD_IO_Init();
    Is_ili9328_Initialized |= ILI9328_IO_INITIALIZED;

    LCD_Delay(5);

    LCD_IO_WriteCmd16MultipleData16(ILI9328_DRIV_OUT_CTRL, &DrivOutCtrlData, 1);
    LCD_IO_WriteCmd16MultipleData8(ILI9328_DRIV_WAV_CTRL, "\x07\x00", 2);
    LCD_IO_WriteCmd16MultipleData16(ILI9328_ENTRY_MOD, &EntryRightThenDown, 1);
    LCD_IO_WriteCmd16MultipleData8(ILI9328_DISP_CTRL2, "\x03\x02", 2);
    LCD_IO_WriteCmd16MultipleData8(ILI9328_DISP_CTRL3, "\x00\x00", 2);

    /*POWER CONTROL REGISTER INITIAL*/
    LCD_IO_WriteCmd16MultipleData8(ILI9328_POW_CTRL1, "\x00\x00", 2);
    LCD_IO_WriteCmd16MultipleData8(ILI9328_POW_CTRL2, "\x00\x07", 2);
    LCD_IO_WriteCmd16MultipleData8(ILI9328_POW_CTRL3, "\x00\x00", 2);
    LCD_IO_WriteCmd16MultipleData8(ILI9328_POW_CTRL4, "\x00\x00", 2);

    LCD_Delay(5);
    /*POWER SUPPPLY STARTUP 1 SETTING*/
    LCD_IO_WriteCmd16MultipleData8(ILI9328_POW_CTRL1, "\x14\xB0", 2);

    LCD_Delay(5);
    LCD_IO_WriteCmd16MultipleData8(ILI9328_POW_CTRL2, "\x00\x07", 2);
    LCD_Delay(5);

    /*POWER SUPPLY STARTUP 2 SETTING*/
    LCD_IO_WriteCmd16MultipleData8(ILI9328_POW_CTRL3, "\x00\x8E", 2);
    LCD_IO_WriteCmd16MultipleData8(ILI9328_POW_CTRL4, "\x0C\x00", 2);
    LCD_IO_WriteCmd16MultipleData8(ILI9328_POW_CTRL7, "\x00\x15", 2);

    LCD_Delay(5);
    /****GAMMA CLUSTER SETTING****/
    LCD_IO_WriteCmd16MultipleData8(ILI9328_GAMMA_CTRL1, "\x00\x00", 2);
    LCD_IO_WriteCmd16MultipleData8(ILI9328_GAMMA_CTRL2, "\x01\x07", 2);
    LCD_IO_WriteCmd16MultipleData8(ILI9328_GAMMA_CTRL3, "\x00\x00", 2);
    LCD_IO_WriteCmd16MultipleData8(ILI9328_GAMMA_CTRL4, "\x02\x03", 2);
    LCD_IO_WriteCmd16MultipleData8(ILI9328_GAMMA_CTRL5, "\x04\x02", 2);
    LCD_IO_WriteCmd16MultipleData8(ILI9328_GAMMA_CTRL6, "\x00\x00", 2);
    LCD_IO_WriteCmd16MultipleData8(ILI9328_GAMMA_CTRL7, "\x02\x07", 2);
    LCD_IO_WriteCmd16MultipleData8(ILI9328_GAMMA_CTRL8, "\x00\x00", 2);
    LCD_IO_WriteCmd16MultipleData8(ILI9328_GAMMA_CTRL9, "\x02\x03", 2);
    LCD_IO_WriteCmd16MultipleData8(ILI9328_GAMMA_CTRL10, "\x04\x03", 2);

    //-DISPLAY WINDOWS 240*320-
    LCD_IO_WriteCmd16MultipleData8(ILI9328_HOR_START_AD, "\x00\x00", 2);
    LCD_IO_WriteCmd16MultipleData16(ILI9328_HOR_END_AD, &LcdPixelWidth, 1);
    LCD_IO_WriteCmd16MultipleData8(ILI9328_VER_START_AD, "\x00\x00", 2);
    LCD_IO_WriteCmd16MultipleData16(ILI9328_VER_END_AD, &LcdPixelHeight, 1);

    //----FRAME RATE SETTING-----
    LCD_IO_WriteCmd16MultipleData16(ILI9328_GATE_SCAN_CTRL1, &GateScanCtrl1Data, 1);
    LCD_IO_WriteCmd16MultipleData8(ILI9328_GATE_SCAN_CTRL2, "\x00\x03", 2);
    LCD_IO_WriteCmd16MultipleData8(ILI9328_PANEL_IF_CTRL1, "\x00\x29", 2);
    LCD_Delay(5);

    //------DISPLAY ON------
    LCD_IO_WriteCmd16MultipleData8(ILI9328_FRM_RATE_COL_CTRL, "\x00\x0E", 2);

    LCD_IO_WriteCmd16MultipleData8(ILI9328_DISP_CTRL1, "\x01\x33", 2);
  }
}

//-----------------------------------------------------------------------------
/* Enables the Display */
void ili9328_DisplayOn(void)
{
  /* Power On sequence -------------------------------------------------------*/
  LCD_IO_WriteCmd16MultipleData8(ILI9328_POW_CTRL1, "\x00\x00", 2);           /* SAP, BT[3:0], AP, DSTB, SLP, STB */
  LCD_IO_WriteCmd16MultipleData8(ILI9328_POW_CTRL2, "\x00\x00", 2);           /* DC1[2:0], DC0[2:0], VC[2:0] */
  LCD_IO_WriteCmd16MultipleData8(ILI9328_POW_CTRL3, "\x00\x00", 2);           /* VREG1OUT voltage */
  LCD_IO_WriteCmd16MultipleData8(ILI9328_POW_CTRL4, "\x00\x00", 2);           /* VDV[4:0] for VCOM amplitude*/
  LCD_Delay(1);
  LCD_IO_WriteCmd16MultipleData8(ILI9328_POW_CTRL1, "\x14\xB0", 2);
  LCD_Delay(1);
  LCD_IO_WriteCmd16MultipleData8(ILI9328_POW_CTRL2, "\x00\x07", 2);
  LCD_Delay(1);
  LCD_IO_WriteCmd16MultipleData8(ILI9328_POW_CTRL3, "\x00\x8E", 2);          /* VREG1OUT voltage */
  LCD_IO_WriteCmd16MultipleData8(ILI9328_POW_CTRL4, "\x0C\x00", 2);
  LCD_IO_WriteCmd16MultipleData8(ILI9328_POW_CTRL7, "\x00\x15", 2);
  LCD_Delay(1);

  /* Display On */
  LCD_IO_WriteCmd16MultipleData8(ILI9328_FRM_RATE_COL_CTRL, "\x00\x0E", 2);  /* 110Hz */
  LCD_IO_WriteCmd16MultipleData8(ILI9328_DISP_CTRL1, "\x01\x33", 2);         /* display ON */
  LCD_IO_Bl_OnOff(1);
}

//-----------------------------------------------------------------------------
/* Disables the Display */
void ili9328_DisplayOff(void)
{
  /* Power Off sequence ------------------------------------------------------*/
  LCD_IO_WriteCmd16MultipleData8(ILI9328_POW_CTRL1, "\x00\x00", 2);          /* SAP, BT[3:0], AP, DSTB, SLP, STB */
  LCD_IO_WriteCmd16MultipleData8(ILI9328_POW_CTRL2, "\x00\x00", 2);          /* DC1[2:0], DC0[2:0], VC[2:0] */
  LCD_IO_WriteCmd16MultipleData8(ILI9328_POW_CTRL3, "\x00\x00", 2);          /* VREG1OUT voltage */
  LCD_IO_WriteCmd16MultipleData8(ILI9328_POW_CTRL4, "\x00\x00", 2);          /* VDV[4:0] for VCOM amplitude*/
  LCD_IO_WriteCmd16MultipleData8(ILI9328_POW_CTRL7, "\x00\x00", 2);          /* VCM[4:0] for VCOMH */

  /* Display Off */
  LCD_IO_WriteCmd16MultipleData8(ILI9328_DISP_CTRL1, "\x00\x00", 2);
  LCD_IO_Bl_OnOff(0);
}

//-----------------------------------------------------------------------------
/* Get the LCD pixel Width
   - retval The Lcd Pixel Width */
uint16_t ili9328_GetLcdPixelWidth(void)
{
  #if ((ILI9328_ORIENTATION == 0) || (ILI9328_ORIENTATION == 2))
  return (uint16_t)ILI9328_LCD_PIXEL_WIDTH;
  #elif ((ILI9328_ORIENTATION == 1) || (ILI9328_ORIENTATION == 3))
  return (uint16_t)ILI9328_LCD_PIXEL_HEIGHT;
  #endif
}

//-----------------------------------------------------------------------------
/* Get the LCD pixel Height
   - retval The Lcd Pixel Height */
uint16_t ili9328_GetLcdPixelHeight(void)
{
  #if ((ILI9328_ORIENTATION == 0) || (ILI9328_ORIENTATION == 2))
  return (uint16_t)ILI9328_LCD_PIXEL_HEIGHT;
  #elif ((ILI9328_ORIENTATION == 1) || (ILI9328_ORIENTATION == 3))
  return (uint16_t)ILI9328_LCD_PIXEL_WIDTH;
  #endif
}

//-----------------------------------------------------------------------------
/* Get the ILI9328 ID
   - retval The ILI9328 ID */
uint32_t ili9328_ReadID(void)
{
  uint32_t ret;
  if(Is_ili9328_Initialized == 0)
  {
    ili9328_Init();
  }
  LCD_IO_ReadCmd16MultipleData16(0x00, &ret, 1, 2);
  return ret;
}

//-----------------------------------------------------------------------------
/* Set Cursor position
   - Xpos: specifies the X position
   - Ypos: specifies the Y position */
void ili9328_SetCursor(uint16_t Xpos, uint16_t Ypos)
{
  ILI9328_SETCURSOR(Xpos, Ypos);
}

//-----------------------------------------------------------------------------
/* Write pixel
   - Xpos: specifies the X position
   - Ypos: specifies the Y position
   - RGBCode: the RGB pixel color */
void ili9328_WritePixel(uint16_t Xpos, uint16_t Ypos, uint16_t RGBCode)
{
  if(LastEntry != ILI9328_ENTRY_DATA_RIGHT_THEN_DOWN)
  {
    LastEntry = ILI9328_ENTRY_DATA_RIGHT_THEN_DOWN;
    LCD_IO_WriteCmd16MultipleData16(ILI9328_ENTRY_MOD, &EntryRightThenDown, 1);
  }
  ILI9328_SETCURSOR(Xpos, Ypos);
  LCD_IO_DrawFill(RGBCode, 1);
}

//-----------------------------------------------------------------------------
/* Read pixel.
   - Xpos: specifies the X position
   - Ypos: specifies the Y position
   - retval: the RGB pixel color */
uint16_t ili9328_ReadPixel(uint16_t Xpos, uint16_t Ypos)
{
  uint16_t ret;
  if(LastEntry != ILI9328_ENTRY_DATA_RIGHT_THEN_DOWN)
  {
    LastEntry = ILI9328_ENTRY_DATA_RIGHT_THEN_DOWN;
    LCD_IO_WriteCmd16MultipleData16(ILI9328_ENTRY_MOD, &EntryRightThenDown, 1);
  }
  ILI9328_SETCURSOR(Xpos, Ypos);
  LCD_IO_ReadBitmap(&ret, 1);
  RGB565TOBGR565(ret);
  return ret;
}


//-----------------------------------------------------------------------------
/* Sets a display window
   - Xpos:   specifies the X bottom left position
   - Ypos:   specifies the Y bottom left position
   - Height: display window height
   - Width:  display window width */
void ili9328_SetDisplayWindow(uint16_t Xpos, uint16_t Ypos, uint16_t Width, uint16_t Height)
{
  #if (ILI9328_ORIENTATION == 0) || (ILI9328_ORIENTATION == 2)
  transdata.d16[0] = Xpos;
  LCD_IO_WriteCmd16MultipleData16(ILI9328_HOR_START_AD, &transdata, 1);
  transdata.d16[0] = Xpos + Width - 1;
  LCD_IO_WriteCmd16MultipleData16(ILI9328_HOR_END_AD, &transdata, 1);

  transdata.d16[0] = Ypos;
  LCD_IO_WriteCmd16MultipleData16(ILI9328_VER_START_AD, &transdata, 1);
  transdata.d16[0] = Ypos + Height - 1;
  LCD_IO_WriteCmd16MultipleData16(ILI9328_VER_END_AD, &transdata, 1);

  #elif (ILI9328_ORIENTATION == 1) || (ILI9328_ORIENTATION == 3)
  transdata.d16[0] = Ypos;
  LCD_IO_WriteCmd16MultipleData16(ILI9328_HOR_START_AD, &transdata, 1);
  transdata.d16[0] = Ypos + Height - 1;
  LCD_IO_WriteCmd16MultipleData16(ILI9328_HOR_END_AD, &transdata, 1);

  transdata.d16[0] = Xpos;
  LCD_IO_WriteCmd16MultipleData16(ILI9328_VER_START_AD, &transdata, 1);
  transdata.d16[0] = Xpos + Width - 1;
  LCD_IO_WriteCmd16MultipleData16(ILI9328_VER_END_AD, &transdata, 1);
  #endif
}

//-----------------------------------------------------------------------------
/* Draw vertical line
   - RGBCode: Specifies the RGB color
   - Xpos:     specifies the X position
   - Ypos:     specifies the Y position
   - Length:   specifies the Line length */
void ili9328_DrawHLine(uint16_t RGBCode, uint16_t Xpos, uint16_t Ypos, uint16_t Length)
{
  if(LastEntry != ILI9328_ENTRY_DATA_RIGHT_THEN_DOWN)
  {
    LastEntry = ILI9328_ENTRY_DATA_RIGHT_THEN_DOWN;
    LCD_IO_WriteCmd16MultipleData16(ILI9328_ENTRY_MOD, &EntryRightThenDown, 1);
  }
  ILI9328_SETCURSOR(Xpos, Ypos);
  LCD_IO_DrawFill(RGBCode, Length);
}

//-----------------------------------------------------------------------------
/* Draw vertical line
   - RGBCode:  specifies the RGB color
   - Xpos:     specifies the X position
   - Ypos:     specifies the Y position
   - Length:   specifies the Line length  */
void ili9328_DrawVLine(uint16_t RGBCode, uint16_t Xpos, uint16_t Ypos, uint16_t Length)
{
  if(LastEntry != ILI9328_ENTRY_DATA_DOWN_THEN_RIGHT)
  {
    LastEntry = ILI9328_ENTRY_DATA_DOWN_THEN_RIGHT;
    LCD_IO_WriteCmd16MultipleData16(ILI9328_ENTRY_MOD, &EntryDownThenRight, 1);
  }
  ILI9328_SETCURSOR(Xpos, Ypos);
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
void ili9328_FillRect(uint16_t Xpos, uint16_t Ypos, uint16_t Xsize, uint16_t Ysize, uint16_t RGBCode)
{
  if(LastEntry != ILI9328_ENTRY_DATA_RIGHT_THEN_DOWN)
  {
    LastEntry = ILI9328_ENTRY_DATA_RIGHT_THEN_DOWN;
    LCD_IO_WriteCmd16MultipleData16(ILI9328_ENTRY_MOD, &EntryRightThenDown, 1);
  }
  ili9328_SetDisplayWindow(Xpos, Ypos, Xsize, Ysize);
  ILI9328_SETCURSOR(Xpos, Ypos);
  LCD_IO_DrawFill(RGBCode, Xsize * Ysize);
}

//-----------------------------------------------------------------------------
/* Displays a 16bit bitmap picture, draw direction: right then up
   - Xpos:  Bmp X position in the LCD
   - Ypos:  Bmp Y position in the LCD
   - pbmp: Bmp picture address (The bmp data also contains the bmp header!) */
void ili9328_DrawBitmap(uint16_t Xpos, uint16_t Ypos, uint8_t *pbmp)
{
  uint32_t index = 0, size = 0;
  /* Read bitmap size */
  Ypos += pbmp[22] + (pbmp[23] << 8) - 1;
  size = *(volatile uint16_t *) (pbmp + 2);
  size |= (*(volatile uint16_t *) (pbmp + 4)) << 16;
  /* Get bitmap data address offset */
  index = *(volatile uint16_t *) (pbmp + 10);
  index |= (*(volatile uint16_t *) (pbmp + 12)) << 16;
  size = (size - index) / 2;
  pbmp += index;

  if(LastEntry != ILI9328_ENTRY_DATA_RIGHT_THEN_UP)
  {
    LastEntry = ILI9328_ENTRY_DATA_RIGHT_THEN_UP;
    LCD_IO_WriteCmd16MultipleData16(ILI9328_ENTRY_MOD, &EntryRightThenUp, 1);
  }
  ILI9328_SETCURSOR(Xpos, Ypos);
  LCD_IO_DrawBitmap(pbmp, size);
}

//-----------------------------------------------------------------------------
/* Displays 16bit/pixel picture, draw direction: right then down
   - pdata: picture address.
   - Xpos:  Image X position in the LCD
   - Ypos:  Image Y position in the LCD
   - Xsize: Image X size in the LCD
   - Ysize: Image Y size in the LCD */
void ili9328_DrawRGBImage(uint16_t Xpos, uint16_t Ypos, uint16_t Xsize, uint16_t Ysize, uint16_t *pdata)
{
  if(LastEntry != ILI9328_ENTRY_DATA_RIGHT_THEN_DOWN)
  {
    LastEntry = ILI9328_ENTRY_DATA_RIGHT_THEN_DOWN;
    LCD_IO_WriteCmd16MultipleData16(ILI9328_ENTRY_MOD, &EntryRightThenDown, 1);
  }
  ili9328_SetDisplayWindow(Xpos, Ypos, Xsize, Ysize);
  ILI9328_SETCURSOR(Xpos, Ypos);
  LCD_IO_DrawBitmap(pdata, Xsize * Ysize);
}

//-----------------------------------------------------------------------------
/* Read 16bit/pixel vitmap from Lcd, draw direction: right then down
   - pdata: picture address.
   - Xpos:  Image X position in the LCD
   - Ypos:  Image Y position in the LCD
   - Xsize: Image X size in the LCD
   - Ysize: Image Y size in the LCD
   note: My lcd only works with pixel-by-pixel reading */
void ili9328_ReadRGBImage(uint16_t Xpos, uint16_t Ypos, uint16_t Xsize, uint16_t Ysize, uint16_t *pdata)
{
  for(uint16_t yp = Ypos; yp < Ypos + Ysize; yp++)
    for(uint16_t xp = Xpos; xp < Xpos + Xsize; xp++)
    {
      *pdata = ili9328_ReadPixel(xp, yp);
      pdata++;
    }
}

//-----------------------------------------------------------------------------
/* Set display scroll parameters
   - Scroll    : Scroll size [pixel]
   - TopFix    : Top fix size [pixel]
   - BottonFix : Botton fix size [pixel] */
void ili9328_Scroll(int16_t Scroll, uint16_t TopFix, uint16_t BottonFix)
{ /* Only full screen scrolling is possible */
  static uint16_t scrparam;
  Scroll = (0 - Scroll) % ILI9328_LCD_PIXEL_HEIGHT;
  if(Scroll < 0)
    Scroll += ILI9328_LCD_PIXEL_HEIGHT;
  if(Scroll != scrparam)
  {
    scrparam = Scroll;
    LCD_IO_WriteCmd16DataFill16(ILI9328_GATE_SCAN_CTRL3, scrparam, 1);
  }
}

//-----------------------------------------------------------------------------
/* User command (it is possible to send direct commands)
   - Command   : Lcd command
   - pData     : data pointer
   - Size      : data number
   - Mode      : 0=write 8bits datas, 1=0=write 16bits datas, 2=read 8bits datas, 3=read 16bits datas */
void ili9328_UserCommand(uint16_t Command, uint8_t* pData, uint32_t Size, uint8_t Mode)
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
