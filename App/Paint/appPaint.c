#include <stdlib.h>
#include <stdio.h>
#include "main.h"

/* BSP LCD driver */
#include "stm32_adafruit_lcd.h"

/* BSP TS driver */
#include "stm32_adafruit_ts.h"

//=============================================================================
/* Setting section (please set the necessary things in this section) */

/* Touchscreen calibrate at starting
   - 0: off (for the touchscreen, use the TS_CINDEX values in stm32_adafruit_ts.h)
   - 1: on  (the touchscreen must be calibrated at startup)
   - 2: on and printf (the touchscreen must be calibrated at startup and printf the cindex values) */
#define TS_CALBIBRATE         1

//=============================================================================
#ifdef  osCMSIS
#define Delay(t)              osDelay(t)
#define GetTime()             osKernelSysTick()
#else
#define Delay(t)              HAL_Delay(t)
#define GetTime()             HAL_GetTick()
#endif

#if TS_CALBIBRATE == 0
#define ts_calib()
#elif TS_CALBIBRATE > 0

#include "ts.h"

#define CALIBDELAY            500
#define CALIBBOXSIZE          6
#define CALIBBOXPOS           15
#define TOUCHDELAY            50

extern  TS_DrvTypeDef         *ts_drv;

//-----------------------------------------------------------------------------
void touchcalib_drawBox(int32_t x, int32_t y, uint16_t cl)
{
  BSP_LCD_SetTextColor(cl);
  BSP_LCD_DrawRect(x - CALIBBOXSIZE / 2, y - CALIBBOXSIZE / 2, CALIBBOXSIZE, CALIBBOXSIZE);
}

//-----------------------------------------------------------------------------
/* Touchscreen calibration function */
void ts_calib(void)
{
  uint16_t tx, ty;
  ts_three_points tc, dc; /* touchscreen and display corrdinates */
  #if TS_CALBIBRATE == 2
  ts_cindex ci;
  #endif

  dc.x0 = 20;
  dc.y0 = 20;
  dc.x1 = BSP_LCD_GetXSize() >> 1;
  dc.x2 = BSP_LCD_GetXSize() - 1 - 20;
  dc.y1 = BSP_LCD_GetYSize() - 1 - 20;
  dc.y2 = BSP_LCD_GetYSize() >> 1;

  touchcalib_drawBox(dc.x0, dc.y0, LCD_COLOR_YELLOW);
  Delay(CALIBDELAY);
  while(!ts_drv->DetectTouch(0))
    Delay(TOUCHDELAY);
  ts_drv->GetXY(0, &tx, &ty);
  tc.x0 = tx; tc.y0 = ty;

  while(ts_drv->DetectTouch(0))
    Delay(TOUCHDELAY);

  touchcalib_drawBox(dc.x0, dc.y0, LCD_COLOR_GRAY);
  touchcalib_drawBox(dc.x1, dc.y1, LCD_COLOR_YELLOW);
  Delay(CALIBDELAY);
  while(!ts_drv->DetectTouch(0))
    Delay(TOUCHDELAY);
  ts_drv->GetXY(0, &tx, &ty);
  tc.x1 = tx; tc.y1 = ty;
  while(ts_drv->DetectTouch(0))
    Delay(TOUCHDELAY);

  touchcalib_drawBox(dc.x1, dc.y1, LCD_COLOR_GRAY);
  touchcalib_drawBox(dc.x2, dc.y2, LCD_COLOR_YELLOW);
  Delay(CALIBDELAY);
  while(!ts_drv->DetectTouch(0))
    Delay(TOUCHDELAY);
  ts_drv->GetXY(0, &tx, &ty);
  tc.x2 = tx; tc.y2 = ty;
  while(ts_drv->DetectTouch(0))
    Delay(TOUCHDELAY);

  #if TS_CALBIBRATE == 1
  BSP_TS_CalibCalc(&tc, &dc, NULL);
  #elif TS_CALBIBRATE == 2
  BSP_TS_CalibCalc(&tc, &dc, &ci);
  BSP_TS_SetCindex(&ci);
  printf("\r\n#define  TS_CINDEX            {%d, %d, %d, %d, %d, %d, %d}\r\n", (int)ci[0], (int)ci[1], (int)ci[2], (int)ci[3], (int)ci[4], (int)ci[5], (int)ci[6]);
  #endif

  Delay(CALIBDELAY);
  BSP_LCD_Clear(LCD_COLOR_BLACK);
}

#endif

void mainApp(void)
{
  TS_StateTypeDef ts;
  uint16_t boxsize;
  uint16_t oldcolor, currentcolor;

  BSP_LCD_Init();
  BSP_TS_Init(BSP_LCD_GetXSize(), BSP_LCD_GetYSize());
  BSP_LCD_Clear(LCD_COLOR_BLACK);
  ts_calib();
  boxsize = BSP_LCD_GetXSize() / 6;

  BSP_LCD_SetTextColor(LCD_COLOR_RED);
  BSP_LCD_FillRect(0, 0, boxsize, boxsize);
  BSP_LCD_SetTextColor(LCD_COLOR_YELLOW);
  BSP_LCD_FillRect(boxsize, 0, boxsize, boxsize);
  BSP_LCD_SetTextColor(LCD_COLOR_GREEN);
  BSP_LCD_FillRect(boxsize * 2, 0, boxsize, boxsize);
  BSP_LCD_SetTextColor(LCD_COLOR_CYAN);
  BSP_LCD_FillRect(boxsize * 3, 0, boxsize, boxsize);
  BSP_LCD_SetTextColor(LCD_COLOR_BLUE);
  BSP_LCD_FillRect(boxsize * 4, 0, boxsize, boxsize);
  BSP_LCD_SetTextColor(LCD_COLOR_MAGENTA);
  BSP_LCD_FillRect(boxsize * 5, 0, boxsize, boxsize);
  BSP_LCD_SetTextColor(LCD_COLOR_WHITE);

  BSP_LCD_DrawRect(0, 0, boxsize, boxsize);
  currentcolor = LCD_COLOR_RED;

  while(1)
  {
    BSP_TS_GetState(&ts);
    if(ts.TouchDetected)
    {
      if(ts.Y < boxsize)
      {
        oldcolor = currentcolor;

        BSP_LCD_SetTextColor(LCD_COLOR_WHITE);
        if (ts.X < boxsize)
        {
          currentcolor = LCD_COLOR_RED;
          BSP_LCD_DrawRect(0, 0, boxsize, boxsize);
        }
        else if (ts.X < boxsize * 2)
        {
          currentcolor = LCD_COLOR_YELLOW;
          BSP_LCD_DrawRect(boxsize, 0, boxsize, boxsize);
        }
        else if (ts.X < boxsize * 3)
        {
          currentcolor = LCD_COLOR_GREEN;
          BSP_LCD_DrawRect(boxsize*2, 0, boxsize, boxsize);
        }
        else if (ts.X < boxsize * 4)
        {
          currentcolor = LCD_COLOR_CYAN;
          BSP_LCD_DrawRect(boxsize*3, 0, boxsize, boxsize);
        }
        else if (ts.X < boxsize * 5)
        {
          currentcolor = LCD_COLOR_BLUE;
          BSP_LCD_DrawRect(boxsize*4, 0, boxsize, boxsize);
        }
        else if (ts.X < boxsize * 6)
        {
          currentcolor = LCD_COLOR_MAGENTA;
          BSP_LCD_DrawRect(boxsize*5, 0, boxsize, boxsize);
        }

        if (oldcolor != currentcolor)
        {
          BSP_LCD_SetTextColor(oldcolor);
          if (oldcolor == LCD_COLOR_RED)
            BSP_LCD_FillRect(0, 0, boxsize, boxsize);
          if (oldcolor == LCD_COLOR_YELLOW)
            BSP_LCD_FillRect(boxsize, 0, boxsize, boxsize);
          if (oldcolor == LCD_COLOR_GREEN)
            BSP_LCD_FillRect(boxsize * 2, 0, boxsize, boxsize);
          if (oldcolor == LCD_COLOR_CYAN)
            BSP_LCD_FillRect(boxsize * 3, 0, boxsize, boxsize);
          if (oldcolor == LCD_COLOR_BLUE)
            BSP_LCD_FillRect(boxsize * 4, 0, boxsize, boxsize);
          if (oldcolor == LCD_COLOR_MAGENTA)
            BSP_LCD_FillRect(boxsize * 5, 0, boxsize, boxsize);
        }
      }
      else
      {
        BSP_LCD_DrawPixel(ts.X, ts.Y, currentcolor);
      }
    }
    Delay(1);
  }
}
