/*
 * Touch calibrate application
 *
 * Created on: 2022.12
 *     Author: Benjami
 * Lcd touchscreen calibration program
 *     output: printf...
 */

#define CALIBBOXSIZE          6
#define CALIBBOXPOS           15

#define CALIBDELAY            500
#define TOUCHDELAY            50

/* overflow limit (cindex 1, 2, 4, 5 and cindex 3, 6) */
#define MAXCINT1245           262144
#define MAXCINT36             1073741824

#include <stdio.h>
#include "main.h"
#include "ts.h"
#include "stm32_adafruit_lcd.h"
#include "stm32_adafruit_ts.h"

extern  TS_DrvTypeDef         *ts_drv;

#ifdef  osCMSIS
#define Delay(t)              osDelay(t)
#define GetTime()             osKernelSysTick()
#else
#define Delay(t)              HAL_Delay(t)
#define GetTime()             HAL_GetTick()
#endif

//-----------------------------------------------------------------------------
void touchcalib_drawBox(int32_t x, int32_t y, uint16_t cl)
{
  BSP_LCD_SetTextColor(cl);
  BSP_LCD_DrawRect(x - CALIBBOXSIZE / 2, y - CALIBBOXSIZE / 2, CALIBBOXSIZE, CALIBBOXSIZE);
}

//-----------------------------------------------------------------------------
void mainApp(void)
{
  uint16_t tx, ty;
  ts_three_points tc, dc; /* touchscreen and display corrdinates */
  ts_cindex ci;

  printf("\r\nPlease: set the LCD ORIENTATION to the value on which the program will run.\r\n");
  Delay(100);
  printf("Press the screen at the yellow squares that appear,\r\n");
  Delay(100);
  printf("then paste the following line into stm32_adafruit_ts.h :\r\n");

  BSP_LCD_Init();
  BSP_TS_Init(BSP_LCD_GetXSize(), BSP_LCD_GetYSize());
  BSP_LCD_Clear(LCD_COLOR_BLACK);

  dc.x0 = 20;
  dc.y0 = 20;
  dc.x1 = BSP_LCD_GetXSize() >> 1;
  dc.y1 = BSP_LCD_GetYSize() - 1 - 20;
  dc.x2 = BSP_LCD_GetXSize() - 1 - 20;
  dc.y2 = BSP_LCD_GetYSize() >> 1;

  touchcalib_drawBox(dc.x0, dc.y0, LCD_COLOR_YELLOW);
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

  BSP_TS_CalibCalc(&tc, &dc, &ci);

  printf("#define  TS_CINDEX            {%d, %d, %d, %d, %d, %d, %d}\r\n", (int)ci[0], (int)ci[1], (int)ci[2], (int)ci[3], (int)ci[4], (int)ci[5], (int)ci[6]);

  while(1);
}
