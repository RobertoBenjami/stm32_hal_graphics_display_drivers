#include <stdlib.h>
#include <stdio.h>
#include <string.h>
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
   - 2: on and printf (the touchscreen must be calibrated at startup and printf the cindex values)
   - 3: on and displays the TS_CINDEX values on the screen */
#define TS_CALBIBRATE         0

/* If TS_CALBIBRATE == 3 -> Text line size */
#define TS_CALIBTEXTSIZE      20

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
  #elif TS_CALBIBRATE == 3
  ts_cindex ci;
  static char s[16];
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
  #elif TS_CALBIBRATE == 3
  BSP_TS_CalibCalc(&tc, &dc, &ci);
  BSP_TS_SetCindex(&ci);
  BSP_LCD_DisplayStringAt(10, 10, (uint8_t *)"#define TS_CINDEX", LEFT_MODE);
  for(uint32_t i=0; i<7; i++)
  {
    sprintf(s, "%d", (int)ci[i]);
    BSP_LCD_DisplayStringAt(10, i*10+20, (uint8_t *)s, LEFT_MODE);
  }
  Delay(CALIBDELAY);
  while(!ts_drv->DetectTouch(0))
    Delay(TOUCHDELAY);
  while(ts_drv->DetectTouch(0))
    Delay(TOUCHDELAY);
  #endif
  Delay(CALIBDELAY);
  BSP_LCD_Clear(LCD_COLOR_BLACK);
}

#endif

extern sFONT font_128x64_8_ledonoff;    /* button pictures */
extern sFONT led_48x48;                 /* screen led pictures */

void offButtonTouchDown(void);
void onButtonTouchDown(void);

/* event type codes */
enum EVENTCODE{EVENT_NONE, EVENT_PAINT, EVENT_TOUCH_DOWN, EVENT_TOUCH_UP, EVENT_TOUCH_MOVE, EVENT_TOUCH_ENTER, EVENT_TOUCH_LEAVE};

/* event type definition */
typedef struct
{
  uint32_t          event_type;         /* event type (see the EVENTCODE) */
  union
  {
    struct
    { /* 2 * 32 bit parameters */
      uint32_t      param32_1;
      uint32_t      param32_2;
    };
    struct
    { /* 4 * 16 bit parameters */
      uint16_t      param16_1;
      uint16_t      param16_2;
      uint16_t      param16_3;
      uint16_t      param16_4;
    };
  };
}tEvent;

/* common object type definition */
typedef struct tObject tObject;
struct tObject
{
  uint16_t          x;                  /* x position */
  uint16_t          y;                  /* y position */
  uint16_t          width;              /* width */
  uint16_t          height;             /* height */
  void              (*eventProc)(tObject * self, tEvent * event); /* event function */
};

void tButton_event(tObject * self, tEvent * event);

/* button object type definition (the first 5 variables are the same as in tObject -> inheritance without C++) */
typedef struct
{
  uint16_t          x;                  /* x position */
  uint16_t          y;                  /* y position */
  uint16_t          width;              /* width */
  uint16_t          height;             /* height */
  void              (*eventProc)(tObject * self, tEvent * event); /* event function */
  void              (*onTouch)(void);   /* touch function */
  sFONT *           font;               /* font address */
  uint8_t           chr_u[9];           /* button in passive state */
  uint8_t           chr_d[9];           /* button in active state */
  uint16_t          color_u[9];         /* colors in passive state */
  uint16_t          color_d[9];         /* colors in active state */
}tButton;

/* off button properties */
const tButton btn_off =
{
  56,                                   /* x */
  32,                                   /* y */
  128,                                  /* width */
  64,                                   /* height */
  &tButton_event,                       /* event function */
  &offButtonTouchDown,                  /* touch function */
  &font_128x64_8_ledonoff,              /* font address */
  "\x21\x20\x22\x24\x25",               /* background, border, text, led pin, led */
  "\x21\x20\x22\x24\x25",               /* background, border, text, led pin, led */
  {LCD_COLOR(10, 10, 100), LCD_COLOR(255, 255, 0),   LCD_COLOR(123, 123, 10), LCD_COLOR(180, 180, 180), LCD_COLOR(180, 0, 0)},
  {LCD_COLOR(85, 85, 200), LCD_COLOR(255, 255, 255), LCD_COLOR(200, 200, 30), LCD_COLOR(180, 180, 180), LCD_COLOR(180, 0, 0)},
};

/* on button properties */
const tButton btn_on =
{
  56,
  144,
  128,
  64,
  &tButton_event,
  &onButtonTouchDown,
  &font_128x64_8_ledonoff,
  "\x21\x20\x23\x24\x25\x26",           /* background, border, text, led pin, led, led light */
  "\x21\x20\x23\x24\x25\x26",           /* background, border, text, led pin, led, led light */
  {LCD_COLOR(10, 10, 100), LCD_COLOR(255, 255, 0),   LCD_COLOR(123, 123, 10), LCD_COLOR(180, 180, 180), LCD_COLOR(255, 0, 0), LCD_COLOR(255, 0, 0)},
  {LCD_COLOR(85, 85, 200), LCD_COLOR(255, 255, 255), LCD_COLOR(200, 200, 30), LCD_COLOR(180, 180, 180), LCD_COLOR(255, 0, 0), LCD_COLOR(255, 0, 0)},
};

/* 2 button array */
const tObject * objects[] = {(tObject *)&btn_off, (tObject *)&btn_on, NULL};

/* paint button on screen */
void tButton_paint(tButton * self, uint8_t off_on)
{
  uint8_t * pchr;
  uint16_t * pcolors;
  if(!off_on)
  { /* passive state */
    pchr = self->chr_u;
    pcolors = self->color_u;
  }
  else
  { /* active state */
    pchr = self->chr_d;
    pcolors = self->color_d;
  }
  BSP_LCD_DisplayMultilayerChar(self->x, self->y, pchr, pcolors, self->font);
}

/* tbutton event processor */
void tButton_event(tObject * self, tEvent * event)
{
  tButton * s = (tButton *)self;
  if(event->event_type == EVENT_PAINT || event->event_type == EVENT_TOUCH_UP || event->event_type == EVENT_TOUCH_LEAVE)
    tButton_paint(s, 0);                /* paint the button passive state */
  else if(event->event_type == EVENT_TOUCH_ENTER || event->event_type == EVENT_TOUCH_DOWN)
  {
    if(s->onTouch)
      s->onTouch();                     /* this function should work when the touchscreen button is pressed */
    tButton_paint(s, 1);                /* paint the button active state */
  }
}


/* this function should work when the touchscreen off button is pressed */
void offButtonTouchDown(void)
{
  static const uint16_t ledoffcolor[] = {LCD_COLOR(100, 10, 10), LCD_COLOR(110, 15, 15)};
  HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_SET); /* led off */
  BSP_LCD_DisplayMultilayerChar(230, 100, (uint8_t *)"\x20\x21", (uint16_t *)&ledoffcolor, &led_48x48); /* lcd screen led off */
}

/* this function should work when the touchscreen on button is pressed */
void onButtonTouchDown(void)
{
  static const uint16_t ledoncolor[]  = {LCD_COLOR(240, 15, 15), LCD_COLOR(255, 120, 30)};
  HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_RESET); /* led on */
  BSP_LCD_DisplayMultilayerChar(230, 100, (uint8_t *)"\x20\x21", (uint16_t *)&ledoncolor, &led_48x48); /* lcd screen led on */
}

/* read touchscreen and create event */
void GetTouch(tEvent * event)
{
  static TS_StateTypeDef pre_ts;
  TS_StateTypeDef ts;

  BSP_TS_GetState(&ts);

  if(ts.TouchDetected && !pre_ts.TouchDetected)
  { /* touch pressure */
    event->event_type = EVENT_TOUCH_DOWN;
    event->param16_1 = ts.X;
    event->param16_2 = ts.Y;
  }
  else if(!ts.TouchDetected && pre_ts.TouchDetected)
  { /* touch release  */
    event->event_type = EVENT_TOUCH_UP;
    event->param16_1 = pre_ts.X;        /* previous position so that we know where we released it*/
    event->param16_2 = pre_ts.Y;
  }
  else if(ts.TouchDetected && pre_ts.TouchDetected)
  { /* touch position is move */
    if(ts.X != pre_ts.X || ts.Y != pre_ts.Y)
    {
      event->event_type = EVENT_TOUCH_MOVE;
      event->param16_1 = ts.X;          /* actual position */
      event->param16_2 = ts.Y;
      event->param16_3 = pre_ts.X;      /* previous position */
      event->param16_4 = pre_ts.Y;
    }
  }

  memcpy(&pre_ts, &ts, sizeof(TS_StateTypeDef));
}

/* event processor */
void eventProcess(tObject ** os, tEvent * event)
{
  uint32_t i = 0, i1, i2;
  tObject * o = os[0];
  while(o)
  {
    if(o->eventProc != NULL)
    {
      if(event->event_type == EVENT_PAINT && o->eventProc != NULL)
        o->eventProc((void *)o, event);
      else if(event->event_type == EVENT_TOUCH_DOWN || event->event_type == EVENT_TOUCH_UP)
      {
        if((event->param16_1 >= o->x) && (event->param16_1 < o->x + o->width) &&
           (event->param16_2 >= o->y) && (event->param16_2 < o->y + o->height)) /* in object ? */
          o->eventProc((void *)o, event);
      }
      else if(event->event_type == EVENT_TOUCH_MOVE)
      {                                           /* move */
        i1 =(event->param16_1 >= o->x) && (event->param16_1 < o->x + o->width) &&
            (event->param16_2 >= o->y) && (event->param16_2 < o->y + o->height); /* in object ? */
        i2 =(event->param16_3 >= o->x) && (event->param16_3 < o->x + o->width) &&
            (event->param16_4 >= o->y) && (event->param16_4 < o->y + o->height); /* prev in object ? */
        if(i1 && !i2)
          event->event_type = EVENT_TOUCH_ENTER;  /* now entered */
        else if(!i1 && i2)
          event->event_type = EVENT_TOUCH_LEAVE;  /* now leaved */
        o->eventProc((void *)o, event);
      }
    }
    i++;
    o = os[i];
  }
}

#define  BACKCOLOR   LCD_COLOR(16, 32, 32)

void mainApp(void)
{
  uint32_t t_touch;
  tEvent event;

  BSP_LCD_Init();
  BSP_TS_Init(BSP_LCD_GetXSize(), BSP_LCD_GetYSize());
  BSP_LCD_SetBackColor(BACKCOLOR);
  BSP_LCD_Clear(BACKCOLOR);
  ts_calib();

  offButtonTouchDown();                           /* set the default led state */

  event.event_type = EVENT_PAINT;
  eventProcess((tObject **)objects, &event);      /* paint event */

  t_touch = GetTime();
  while(1)
  {
    if(GetTime() - t_touch > 50)
    { /* 50msec frequently */
      t_touch = GetTime();

      event.event_type = EVENT_NONE;
      GetTouch(&event);                           /* get touch creen */
      if(event.event_type != EVENT_NONE)
        eventProcess((tObject **)objects, &event);/* event processor (if there was an event) */
    }
  }
}
