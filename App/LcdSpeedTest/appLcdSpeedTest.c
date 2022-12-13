/* LCD speed test program
 * The results displayed in printf
 *
 * author: Roberto Benjami
 * version:  2022.12
 */

//-----------------------------------------------------------------------------
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "main.h"

#include "lcd.h"
#include "bmp.h"

#include "appLcdSpeedTest.h"

/* BSP_LCD_... */
#include "stm32_adafruit_lcd.h"

/* Test photo */
#if BITMAP_TEST == 1
#define rombitmap             beer_60x100_16
#define ROMBITMAP_WIDTH       60
#define ROMBITMAP_HEIGHT      100
#endif

#define GPIO_Port_(a)         a ## _GPIO_Port
#define GPIO_Port(a)          GPIO_Port_(a)
#define Pin_(a)               a ## _Pin
#define Pin(a)                Pin_(a)

#ifdef  __CC_ARM
#define random()              rand()
#endif

//-----------------------------------------------------------------------------
#ifndef  osCMSIS
/* no freertos */
#define Delay(t)              HAL_Delay(t)
#define GetTime()             HAL_GetTick()

#define POWERMETER_START
#define POWERMETER_STOP
#define POWERMETER_REF
#define POWERMETER_PRINT      Delay(10); printf("\r\n")

#else
/* freertos */
#define Delay(t)              osDelay(t)
volatile uint32_t task02_count = 0, task02_run = 0;
uint32_t refcpuusage = 1;

#if osCMSIS < 0x20000
/* freertos V1 */

#define GetTime()             osKernelSysTick()
extern osThreadId defaultTaskHandle;
void StartTask02(void const * argument);
osThreadId Task2Handle;
osThreadDef(Task2, StartTask02, osPriorityLow, 0, 144);

#else
/* freertos V2 */

#define GetTime()             osKernelGetTickCount()
extern osThreadId_t defaultTaskHandle;
void StartTask02(void * argument);
osThreadId_t Task2Handle;
const osThreadAttr_t t2_attributes = {.name = "Task2", .stack_size = 256, .priority = (osPriority_t) osPriorityLow,};

#endif
/* common freertos V1 and freertos V2 */

#if     POWERMETER == 1
#define POWERMETER_START      {task02_count = 0; task02_run = 1;}
#define POWERMETER_STOP       task02_run = 0

#define POWERMETER_REF        {task02_run = 1; refcpuusage = task02_count / t;}
#define POWERMETER_PRINT      {Delay(10); if(t) printf(", cpu usage:%d%%\r\n", (int)cpuusage_calc(t)); else printf("\r\n");}
#endif

uint32_t cpuusage_calc(uint32_t t)
{
  uint32_t cpuusage;
  if(t)
  {
    cpuusage = ((100 * task02_count) / t) / refcpuusage;
    if(cpuusage > 100)
      cpuusage = 100;
    cpuusage = 100 - cpuusage;
  }
  else
    cpuusage = 0;

  #if STACKOWERFLOW_CHECK == 1
  uint32_t wm;
  wm = uxTaskGetStackHighWaterMark(Task2Handle);
  if(!wm)
    while(1);
  wm = uxTaskGetStackHighWaterMark(defaultTaskHandle);
  if(!wm)
    while(1);
  #endif

  task02_count = 0;
  return cpuusage;
}

#endif

//-----------------------------------------------------------------------------
#if BITMAP_TEST == 1
extern const BITMAPSTRUCT rombitmap;
#if READ_TEST == 1
uint16_t bitmap[ROMBITMAP_WIDTH * ROMBITMAP_HEIGHT];
#endif
#endif

//-----------------------------------------------------------------------------
uint32_t ClearTest(uint32_t n)
{
  uint32_t ctStartT = GetTime();
  for(uint32_t i = 0; i < n; i++)
    BSP_LCD_Clear(LCD_COLOR_BLACK);
  return(GetTime() - ctStartT);
}

//-----------------------------------------------------------------------------
uint32_t PixelTest(uint32_t n)
{
  uint16_t x, y;

  uint32_t ctStartT = GetTime();
  for(uint32_t i = 0; i < n; i++)
  {
    x = random() % BSP_LCD_GetXSize();
    y = random() % BSP_LCD_GetYSize();
    BSP_LCD_DrawPixel(x, y, LCD_COLOR16(random() & 0xFFFF));
  }
  return(GetTime() - ctStartT);
}

//-----------------------------------------------------------------------------
uint32_t LineTest(uint32_t n)
{
  uint16_t x1, y1, x2, y2;

  uint32_t ctStartT = GetTime();
  for(uint32_t i = 0; i < n; i++)
  {
    x1 = random() % BSP_LCD_GetXSize();
    y1 = random() % BSP_LCD_GetYSize();
    x2 = random() % BSP_LCD_GetXSize();
    y2 = random() % BSP_LCD_GetYSize();
    BSP_LCD_SetTextColor(LCD_COLOR16(random() & 0xFFFF));
    BSP_LCD_DrawLine(x1, y1, x2, y2);
  }
  return(GetTime() - ctStartT);
}

//-----------------------------------------------------------------------------
uint32_t FillRectTest(uint32_t n)
{
  uint16_t x, y, w, h;

  uint32_t ctStartT = GetTime();
  for(uint32_t i = 0; i < n; i++)
  {
    w = random() % (BSP_LCD_GetXSize() >> 1);
    h = random() % (BSP_LCD_GetYSize() >> 1);
    x = random() % (BSP_LCD_GetXSize() - w);
    y = random() % (BSP_LCD_GetYSize() - h);
    BSP_LCD_SetTextColor(LCD_COLOR16(random() & 0xFFFF));
    BSP_LCD_FillRect(x, y, w, h);
  }
  return(GetTime() - ctStartT);
}

//-----------------------------------------------------------------------------
uint32_t CharTest(uint32_t n)
{
  uint16_t x, y;
  uint8_t  c;
  sFONT * fp;

  uint32_t ctStartT = GetTime();
  for(uint32_t i = 0; i < n; i++)
  {
    c = random() % 5;
    if(c == 0)
      BSP_LCD_SetFont(&Font8);
    else if(c == 1)
      BSP_LCD_SetFont(&Font12);
    else if(c == 2)
      BSP_LCD_SetFont(&Font16);
    else if(c == 3)
      BSP_LCD_SetFont(&Font20);
    else if(c == 4)
      BSP_LCD_SetFont(&Font24);
    fp = BSP_LCD_GetFont();

    x = random() % (BSP_LCD_GetXSize() - fp->Width);
    y = random() % (BSP_LCD_GetYSize() - fp->Height);
    BSP_LCD_SetTextColor(LCD_COLOR16(random() & 0xFFFF));
    BSP_LCD_SetBackColor(LCD_COLOR16(random() & 0xFFFF));

    c = random() % 96 + ' ';
    BSP_LCD_DisplayChar(x, y, c);
  }
  return(GetTime() - ctStartT);
}

//-----------------------------------------------------------------------------
uint32_t CircleTest(uint32_t n)
{
  uint16_t x, y, r, rmax;

  rmax = BSP_LCD_GetXSize();
  if(rmax > BSP_LCD_GetYSize())
    rmax = BSP_LCD_GetYSize();
  rmax >>= 2;

  uint32_t ctStartT = GetTime();
  for(uint32_t i = 0; i < n; i++)
  {
    do
      r = random() % rmax;
    while(r == 0);

    x = random() % (BSP_LCD_GetXSize() - (r << 1)) + r;
    y = random() % (BSP_LCD_GetYSize() - (r << 1)) + r;
    BSP_LCD_SetTextColor(LCD_COLOR16(random() & 0xFFFF));
    BSP_LCD_DrawCircle(x, y, r);
  }
  return(GetTime() - ctStartT);
}

//-----------------------------------------------------------------------------
uint32_t FillCircleTest(uint32_t n)
{
  uint16_t x, y, r, rmax;

  rmax = BSP_LCD_GetXSize();
  if(rmax > BSP_LCD_GetYSize())
    rmax = BSP_LCD_GetYSize();
  rmax >>= 2;

  uint32_t ctStartT = GetTime();
  for(uint32_t i = 0; i < n; i++)
  {
    do
      r = random() % rmax;
    while(r == 0);

    x = random() % (BSP_LCD_GetXSize() - (r << 1)) + r;
    y = random() % (BSP_LCD_GetYSize() - (r << 1)) + r;
    BSP_LCD_SetTextColor(LCD_COLOR16(random() & 0xFFFF));
    BSP_LCD_FillCircle(x, y, r);
  }
  return(GetTime() - ctStartT);
}

//-----------------------------------------------------------------------------
uint32_t ColorTest(void)
{
  uint16_t xs, ys;
  uint8_t  cy;

  uint32_t ctStartT = GetTime();
  xs = BSP_LCD_GetXSize();
  ys = BSP_LCD_GetYSize();
  for(uint16_t x = 0; x < xs; x++)
  {
    cy = (uint32_t)(x << 8) / xs;
    BSP_LCD_SetTextColor(LCD_COLOR(cy, cy, cy));
    BSP_LCD_DrawVLine(x, 0, ys >> 2);

    BSP_LCD_SetTextColor(LCD_COLOR(cy, 0, 0));
    BSP_LCD_DrawVLine(x, ys >> 2, ys >> 2);

    BSP_LCD_SetTextColor(LCD_COLOR(0, cy, 0));
    BSP_LCD_DrawVLine(x, ys >> 1, ys >> 2);

    BSP_LCD_SetTextColor(LCD_COLOR(0, 0, cy));
    BSP_LCD_DrawVLine(x, (ys >> 1) + (ys >> 2), ys >> 2);
  }
  return(GetTime() - ctStartT);
}

//-----------------------------------------------------------------------------
#if BITMAP_TEST == 1
uint32_t BitmapTest(uint32_t n)
{
  extern const BITMAPSTRUCT beer_60x100_16;
  uint16_t x, y;

  uint32_t ctStartT = GetTime();
  for(uint32_t i = 0; i < n; i++)
  {
    x = random() % (BSP_LCD_GetXSize() - rombitmap.infoHeader.biWidth);
    y = random() % (BSP_LCD_GetYSize() - rombitmap.infoHeader.biHeight);
    BSP_LCD_DrawBitmap(x, y, (uint8_t *)&rombitmap);
  }
  return(GetTime() - ctStartT);
}

//-----------------------------------------------------------------------------
uint32_t ScrollTest(uint32_t n)
{
  uint16_t ss, o, tf, bf;
  int16_t  i;
  uint32_t ctStartT = GetTime();
  ss = BSP_LCD_GetXSize();
  o = 0;
  if(BSP_LCD_GetYSize() > ss)
  {
    ss = BSP_LCD_GetYSize();
    o = 1;                              /* vertical display */
  }
  BSP_LCD_SetTextColor(LCD_COLOR_YELLOW);
  BSP_LCD_SetBackColor(LCD_COLOR_BLUE);
  BSP_LCD_SetFont(&Font12);
  BSP_LCD_DisplayChar(0, 0, '1');
  BSP_LCD_DisplayChar(BSP_LCD_GetXSize() - 8, 0, '2');

  BSP_LCD_SetFont(&Font16);
  BSP_LCD_DisplayChar(0, BSP_LCD_GetYSize() - 16, '3');
  BSP_LCD_DisplayChar(BSP_LCD_GetXSize() - 12, BSP_LCD_GetYSize() - 16, '4');
  if(o == 0)
  { /* horizontal display */
    tf = 12; bf = 16;
    BSP_LCD_DrawBitmap(tf, (BSP_LCD_GetYSize() - rombitmap.infoHeader.biHeight) / 2, (uint8_t *)&rombitmap);
    ss -= (tf + bf + rombitmap.infoHeader.biWidth);
  }
  else
  { /* vertical display */
    tf = 12; bf = 16;
    BSP_LCD_DrawBitmap((BSP_LCD_GetXSize() - rombitmap.infoHeader.biWidth) / 2, tf, (uint8_t *)&rombitmap);
    ss -= (tf + bf + rombitmap.infoHeader.biHeight);
  }
  i = 0;
  while(i < ss)
  {
    Delay(20);
    BSP_LCD_Scroll(i, tf, bf);
    i++;
  }
  do
  {
    i--;
    Delay(20);
    BSP_LCD_Scroll(i, tf, bf);
  } while(i > 0);

  Delay(1000);

  BSP_LCD_SetTextColor(LCD_COLOR_BLACK);
  if(o == 0)
  { /* horizontal display */
    BSP_LCD_FillRect(tf, (BSP_LCD_GetYSize() - rombitmap.infoHeader.biHeight) / 2, rombitmap.infoHeader.biWidth, rombitmap.infoHeader.biHeight);
    BSP_LCD_DrawBitmap(BSP_LCD_GetXSize() - rombitmap.infoHeader.biWidth - bf, (BSP_LCD_GetYSize() - rombitmap.infoHeader.biHeight) / 2, (uint8_t *)&rombitmap);
  }
  else
  { /* vertical display */
    BSP_LCD_FillRect((BSP_LCD_GetXSize() - rombitmap.infoHeader.biWidth) / 2, tf, rombitmap.infoHeader.biWidth, rombitmap.infoHeader.biHeight);
    BSP_LCD_DrawBitmap((BSP_LCD_GetXSize() - rombitmap.infoHeader.biWidth) / 2, BSP_LCD_GetYSize() - rombitmap.infoHeader.biHeight - bf, (uint8_t *)&rombitmap);
  }
  i = 0;
  while(i > 0 - ss)
  {
    Delay(20);
    BSP_LCD_Scroll(i, tf, bf);
    i--;
  }
  do
  {
    i++;
    Delay(20);
    BSP_LCD_Scroll(i, tf, bf);
  } while(i < 0);

  Delay(1000);

  i = -500;
  while(i < 500)
  {
    Delay(10);
    BSP_LCD_Scroll(i, tf, bf);
    i++;
  }

  Delay(1000);
  BSP_LCD_Scroll(0, 0, 0);
  ctStartT = GetTime() - ctStartT;
  return ctStartT;
}

//-----------------------------------------------------------------------------
#if READ_TEST == 1
uint32_t ReadPixelTest(uint32_t n)
{
  uint16_t x, y, x0, y0, xsize, ysize;
  uint32_t error = 0;

  x0 = 20;
  y0 =  5;
  xsize = rombitmap.infoHeader.biWidth;
  ysize = rombitmap.infoHeader.biHeight;

  /* Draw bitmap */
  BSP_LCD_DrawBitmap(x0, y0, (uint8_t *)&rombitmap);

  /* Read bitmap (BSP_LCD_ReadPixel) */
  uint32_t ctStartT = GetTime();
  while(n--)
    for(y = 0; y < ysize; y++)
      for(x = 0; x < xsize; x++)
        bitmap[y * xsize + x] = BSP_LCD_ReadPixel(x0 + x, y0 + y);
  ctStartT = GetTime() - ctStartT;

  /* Check the read error */
  for(y = 0; y < ysize; y++)
    for(x = 0; x < xsize; x++)
    {
      if(bitmap[y * xsize + x] != rombitmap.data[(ysize - 1 - y) * xsize + x])
        error++;
    }

  if(error)
    printf("ReadPixelTest error: %d\r\n", (int)error);

  BSP_LCD_DrawRGB16Image(x0 + 45, y0 + 10, xsize, ysize, &bitmap[0]);
  return(ctStartT);
}

//-----------------------------------------------------------------------------
uint32_t ReadImageTest(uint32_t n)
{
  uint16_t x, y, x0, y0, xsize, ysize;
  uint32_t error = 0;

  x0 = 20;
  y0 =  5;
  xsize = rombitmap.infoHeader.biWidth;
  ysize = rombitmap.infoHeader.biHeight;

  /* Draw bitmap */
  BSP_LCD_DrawBitmap(x0, y0, (uint8_t *)&rombitmap);

  /* Read bitmap (BSP_LCD_ReadRGB16Image) */
  uint32_t ctStartT = GetTime();
  while(n--)
    BSP_LCD_ReadRGB16Image(x0, y0, xsize, ysize, &bitmap[0]);
  ctStartT = GetTime() - ctStartT;

  /* Check the read error */
  for(y = 0; y < ysize; y++)
    for(x = 0; x < xsize; x++)
    {
      if(bitmap[y * xsize + x] != rombitmap.data[(ysize - 1 - y) * xsize + x])
        error++;
    }

  if(error)
    printf("ReadImageTest error: %d\r\n", (int)error);

  BSP_LCD_DrawRGB16Image(x0 - 15, y0 + 20, xsize, ysize, &bitmap[0]);
  return(ctStartT);
}
#endif /* #if READ_TEST == 1 */
#endif /* #if BITMAP_TEST == 1 */

//-----------------------------------------------------------------------------
void mainApp(void)
{
  #ifdef osCMSIS
  #if osCMSIS < 0x20000
  /* Freertos 1 */
  Task2Handle = osThreadCreate(osThread(Task2), "T2");
  #else
  /* Freertos 2 */
  Task2Handle = osThreadNew(StartTask02, (void*)"T2", &t2_attributes);
  #endif
  #endif

  uint32_t t;

  Delay(300);

  BSP_LCD_Init();

  t = random();

  Delay(100);
  printf("\r\nDisplay ID = %X\r\n", (unsigned int)BSP_LCD_ReadID());
  Delay(100);

  while(1)
  {
    #ifdef  __GNUC__
    _impure_ptr->_r48->_rand_next = 0;
    #endif

    Delay(100);
    t = 300;
    POWERMETER_START;
    Delay(t);
    POWERMETER_STOP;
    POWERMETER_REF;
    printf("Delay 300\r\n");
    Delay(DELAY_CHAPTER);

    POWERMETER_START;
    t = ClearTest(10);
    POWERMETER_STOP;
    printf("Clear Test (10x): %d ms", (int)t);
    POWERMETER_PRINT;
    Delay(DELAY_CHAPTER);

    POWERMETER_START;
    t = PixelTest(100000);
    POWERMETER_STOP;
    printf("Pixel Test (100000 pixel): %d ms", (int)t);
    POWERMETER_PRINT;
    Delay(DELAY_CHAPTER);

    BSP_LCD_Clear(LCD_COLOR_BLACK);
    POWERMETER_START;
    t = LineTest(1000);
    POWERMETER_STOP;
    printf("Line Test (1000 lines): %d ms", (int)t);
    POWERMETER_PRINT;
    Delay(DELAY_CHAPTER);

    BSP_LCD_Clear(LCD_COLOR_BLACK);
    POWERMETER_START;
    t = FillRectTest(250);
    POWERMETER_STOP;
    printf("Fill Rect Test (250 rect): %d ms", (int)t);
    POWERMETER_PRINT;
    Delay(DELAY_CHAPTER);

    BSP_LCD_Clear(LCD_COLOR_BLACK);
    POWERMETER_START;
    t = CircleTest(1000);
    POWERMETER_STOP;
    printf("Circle Test (1000 circles): %d ms", (int)t);
    POWERMETER_PRINT;
    Delay(DELAY_CHAPTER);

    BSP_LCD_Clear(LCD_COLOR_BLACK);
    POWERMETER_START;
    t = FillCircleTest(250);
    POWERMETER_STOP;
    printf("Fill Circle Test (250 circles): %d ms", (int)t);
    POWERMETER_PRINT;
    Delay(DELAY_CHAPTER);

    #if BITMAP_TEST == 1
    BSP_LCD_Clear(LCD_COLOR_BLACK);
    POWERMETER_START;
    t = CharTest(5000);
    POWERMETER_STOP;
    printf("Char Test (5000 char): %d ms", (int)t);
    POWERMETER_PRINT;
    Delay(DELAY_CHAPTER);

    BSP_LCD_Clear(LCD_COLOR_BLACK);
    POWERMETER_START;
    t = BitmapTest(100);
    POWERMETER_STOP;
    printf("Bitmap Test (100 bitmap): %d ms", (int)t);
    POWERMETER_PRINT;
    Delay(DELAY_CHAPTER);

    #if READ_TEST == 1
    BSP_LCD_Clear(LCD_COLOR_BLACK);
    POWERMETER_START;
    t = ReadPixelTest(20);
    POWERMETER_STOP;
    printf("ReadPixel Test (20x bitmap read): %d ms", (int)t);
    POWERMETER_PRINT;
    Delay(DELAY_CHAPTER);

    BSP_LCD_Clear(LCD_COLOR_BLACK);
    POWERMETER_START;
    t = ReadImageTest(20);
    POWERMETER_STOP;
    printf("ReadImage Test (20x bitmap read): %d ms", (int)t);
    POWERMETER_PRINT;
    Delay(DELAY_CHAPTER);
    #endif

    BSP_LCD_Clear(LCD_COLOR_BLACK);
    POWERMETER_START;
    t = ScrollTest(0);
    POWERMETER_STOP;
    printf("Scroll Test: %d ms", (int)t);
    POWERMETER_PRINT;
    Delay(DELAY_CHAPTER);

    #endif /* #if BITMAP_TEST == 1 */

    BSP_LCD_Clear(LCD_COLOR_BLACK);
    POWERMETER_START;
    t = ColorTest();
    POWERMETER_STOP;
    printf("Color Test: %d ms", (int)t);
    POWERMETER_PRINT;
    Delay(3 * DELAY_CHAPTER);

    BSP_LCD_DisplayOff();
    Delay(DELAY_CHAPTER);
    BSP_LCD_DisplayOn();
    Delay(DELAY_CHAPTER);

    printf("\r\n");
  }
}

#ifdef osCMSIS

//-----------------------------------------------------------------------------
/* The other task constantly increases one counter */
#if osCMSIS < 0x20000
void StartTask02(void const * argument)
#else
void StartTask02(void * argument)
#endif
{
  while(1)
  {
    #ifdef LED1_NAME
    taskENTER_CRITICAL();
    #if LED_ACTIVE == 0
    HAL_GPIO_WritePin(GPIO_Port(LED1_NAME), Pin(LED1_NAME), GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GPIO_Port(LED1_NAME), Pin(LED1_NAME), GPIO_PIN_SET);
    #elif LED_ACTIVE == 1
    HAL_GPIO_WritePin(GPIO_Port(LED1_NAME), Pin(LED1_NAME), GPIO_PIN_SET);
    HAL_GPIO_WritePin(GPIO_Port(LED1_NAME), Pin(LED1_NAME), GPIO_PIN_RESET);
    #endif
    taskEXIT_CRITICAL();
    #endif
    #ifdef LED2_NAME
    taskENTER_CRITICAL();
    #if LED_ACTIVE == 0
    HAL_GPIO_WritePin(GPIO_Port(LED2_NAME), Pin(LED2_NAME), GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GPIO_Port(LED2_NAME), Pin(LED2_NAME), GPIO_PIN_SET);
    #elif LED_ACTIVE == 1
    HAL_GPIO_WritePin(GPIO_Port(LED2_NAME), Pin(LED2_NAME), GPIO_PIN_SET);
    HAL_GPIO_WritePin(GPIO_Port(LED2_NAME), Pin(LED2_NAME), GPIO_PIN_RESET);
    #endif
    taskEXIT_CRITICAL();
    #endif
    if(task02_run)
      task02_count++;
  }
}

#endif
