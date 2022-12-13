/* 3D Filled Vector Graphics
   (c) 2019 Pawel A. Hernik
   YouTube videos:
   https://youtu.be/YLf2WXjunyg
   https://youtu.be/5y28ipwQs-E

   Modify: Roberto Benjami
   Add CPU usage if run under freertos */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "main.h"

#include "3d_filled_vector.h"

/* BSP_LCD_... */
#include "stm32_adafruit_lcd.h"

/* check the defaultTask and Task2 stack owerflow (only freertos) */
#define  STACKOWERFLOW_CHECK  1

#if LCD_REVERSE16 == 0
#define  RC(a)   a
#endif
#if LCD_REVERSE16 == 1
#define  RC(a)   ((((a) & 0xFF) << 8) | (((a) & 0xFF00) >> 8))
#endif

//=============================================================================
#ifndef osCMSIS
#define Delay(t)              HAL_Delay(t)
#define GetTime()             HAL_GetTick()
#else
volatile uint32_t task02_count = 0, task02_run = 0;
uint32_t refcpuusage = 1;
#if osCMSIS < 0x20000
void StartTask02(void const * argument);
#define Delay(t)              osDelay(t)
#define GetTime()             osKernelSysTick()
osThreadId Task2Handle;
extern osThreadId defaultTaskHandle;
osThreadDef(Task2, StartTask02, osPriorityLow, 0, 144);
#else
void StartTask02(void * argument);
#define Delay(t)              osDelay(t)
#define GetTime()             osKernelGetTickCount()
osThreadId_t Task2Handle;
extern osThreadId_t defaultTaskHandle;
const osThreadAttr_t t2_attributes = {.name = "Task2", .stack_size = 144, .priority = (osPriority_t) osPriorityLow,};
#endif
#endif

/*
 Implemented features:
 - optimized rendering without local framebuffer, in STM32 case 1 to 32 lines buffer can be used
 - pattern based background
 - 3D starfield
 - no floating point arithmetic
 - no slow trigonometric functions
 - rotations around X and Y axes
 - simple outside screen culling
 - rasterizer working for all convex polygons
 - backface culling
 - visible faces sorting by Z axis
*/

int  buttonState;
int  prevState = 1;
long btDebounce    = 30;
long btMultiClick  = 600;
long btLongClick   = 500;
long btLongerClick = 2000;
long btTime = 0, btTime2 = 0, millis;
int  clickCnt = 1;

#define GPIO_Port_(a)      a ## _GPIO_Port
#define GPIO_Port(a)       GPIO_Port_(a)
#define Pin_(a)            a ## _Pin
#define Pin(a)             Pin_(a)

// 0=idle, 1,2,3=click, -1,-2=longclick
int checkButton()
{
  int state;
  millis = GetTime();

  if(HAL_GPIO_ReadPin(GPIO_Port(BUTTON_NAME), Pin(BUTTON_NAME)))
    state = 1 - BUTTON_ON;
  else
    state = BUTTON_ON;

  if(state == 0 && prevState == 1)
  {
    btTime = millis; prevState = state;
    return 0;
  } // button just pressed

  if(state == 1 && prevState == 0)
  { // button just released
    prevState = state;
    if(millis - btTime >= btDebounce && millis - btTime < btLongClick)
    {
      if(millis - btTime2 < btMultiClick)
        clickCnt++;
      else
        clickCnt = 1;
      btTime2 = millis;
      return clickCnt; 
    } 
  }

  if(state == 0 && millis - btTime >= btLongerClick)
  {
    prevState = state;
    return -2;
  }

  if(state == 0 && millis - btTime >= btLongClick)
  {
    prevState = state;
    return -1;
  }

  return 0;
}

int prevButtonState = 0;

int handleButton()
{
  prevButtonState = buttonState;
  buttonState = checkButton();
  return buttonState;
}

// --------------------------------------------------------------------------
char txt[30];
#define MAX_OBJ 12
int bgMode = 3;
int object = 6;
int bfCull = 1;
int orient = 0;
int polyMode = 0;

#include "pat2.h"
#include "pat7.h"
#include "pat8.h"
#include "gfx3d.h"

// --------------------------------------------------------------------------
void setup() 
{
  uint8_t  e;

  #ifndef osCMSIS
  Delay(300);
  #else
  #if osCMSIS < 0x20000
  /* Freertos 1 */
  Task2Handle = osThreadCreate(osThread(Task2), "T2");
  #else
  /* Freertos 2 */
  Task2Handle = osThreadNew(StartTask02, (void*)"T2", &t2_attributes);
  #endif
  task02_run = 1; task02_count = 0;
  Delay(300);
  refcpuusage = task02_count / 300;
  #endif

  e = BSP_LCD_Init();
  if(e == LCD_ERROR)
  {
    // printf("\r\nLcd Init Error\r\n");
    while(1);
  }

  BSP_LCD_Clear(LCD_COLOR_BLACK);
  BSP_LCD_SetFont(&FONTNAME);
  initStars();
}

unsigned int ms, msMin = 1000, msMax = 0, stats = 1, optim = 0; // optim=1 for ST7735, 0 for ST7789

// --------------------------------------------------------------------------
#ifdef osCMSIS
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

// --------------------------------------------------------------------------
void showStats()
{
  #ifdef osCMSIS
  uint32_t cu;
  cu = cpuusage_calc(ms);
  snprintf(txt, 30, "cpu usage %d %%   ", (int)cu);
  BSP_LCD_SetTextColor(LCD_COLOR_CYAN);
  BSP_LCD_DisplayStringAt(0, SCR_HT - 4 * CHARSIZEY, (uint8_t *)txt, LEFT_MODE);
  #endif

  if(ms < msMin) msMin = ms;
  if(ms > msMax) msMax = ms;
  BSP_LCD_SetBackColor(LCD_COLOR_BLACK);
  if(optim == 0)
  {
    snprintf(txt, 30, "%d ms     %d fps ", (int)ms, (int)(1000 / ms));
    BSP_LCD_SetTextColor(LCD_COLOR_YELLOW);
    BSP_LCD_DisplayStringAt(0, SCR_HT - 3 * CHARSIZEY, (uint8_t *)txt, LEFT_MODE);
    snprintf(txt, 30, "%d-%d ms  %d-%d fps   ", msMin, msMax, 1000 / msMax, 1000 / msMin);
    BSP_LCD_SetTextColor(LCD_COLOR_GREEN);
    BSP_LCD_DisplayStringAt(0, SCR_HT - 2 * CHARSIZEY, (uint8_t *)txt, LEFT_MODE);
    snprintf(txt, 30, "total/vis %d / %d   ", numPolys, numVisible);
    BSP_LCD_SetTextColor(LCD_COLOR_MAGENTA);
    BSP_LCD_DisplayStringAt(0, SCR_HT - CHARSIZEY, (uint8_t *)txt, LEFT_MODE);
  }
  else if(optim == 1)
  {
    optim = 2;
    snprintf(txt, 30, "00 ms     00 fps");
    BSP_LCD_SetTextColor(LCD_COLOR_YELLOW);
    BSP_LCD_DisplayStringAt(0, SCR_HT - 3 * CHARSIZEY, (uint8_t *)txt, LEFT_MODE);
    snprintf(txt, 30, "00-00 ms  00-00 fps");
    BSP_LCD_SetTextColor(LCD_COLOR_GREEN);
    BSP_LCD_DisplayStringAt(0, SCR_HT - 2 * CHARSIZEY, (uint8_t *)txt, LEFT_MODE);
    snprintf(txt, 30, "total/vis 000 / 000");
    BSP_LCD_SetTextColor(LCD_COLOR_MAGENTA);
    BSP_LCD_DisplayStringAt(0, SCR_HT - CHARSIZEY, (uint8_t *)txt, LEFT_MODE);
  }
  else
  {
    snprintf(txt, 30, "%2d", (int)ms);
    BSP_LCD_SetTextColor(LCD_COLOR_YELLOW);
    BSP_LCD_DisplayStringAt(0, SCR_HT - 3 * CHARSIZEY, (uint8_t *)txt, LEFT_MODE);
    snprintf(txt, 30,"%2d", (int)(1000 / ms));
    BSP_LCD_DisplayStringAt(10 * CHARSIZEX, SCR_HT - 3 * CHARSIZEY, (uint8_t *)txt, LEFT_MODE);
    snprintf(txt, 30,"%2d-%2d", msMin, msMax);
    BSP_LCD_SetTextColor(LCD_COLOR_GREEN);
    BSP_LCD_DisplayStringAt(0, SCR_HT - 2 * CHARSIZEY, (uint8_t *)txt, LEFT_MODE);
    snprintf(txt, 30, "%2d-%2d", 1000 / msMax, 1000 / msMin);
    BSP_LCD_DisplayStringAt(10 * CHARSIZEX, SCR_HT - 2 * CHARSIZEY, (uint8_t *)txt, LEFT_MODE);
    snprintf(txt, 30, "%3d", numPolys);
    BSP_LCD_SetTextColor(LCD_COLOR_MAGENTA);
    BSP_LCD_DisplayStringAt(10 * CHARSIZEX, SCR_HT - CHARSIZEY, (uint8_t *)txt, LEFT_MODE);
    snprintf(txt, 30, "%3d", numVisible);
    BSP_LCD_DisplayStringAt(16 * CHARSIZEX, SCR_HT - CHARSIZEY, (uint8_t *)txt, LEFT_MODE);
  }
}

// --------------------------------------------------------------------------
void loop()
{
  handleButton();
  if(buttonState < 0)
  {
    if(buttonState == -1 && prevButtonState >= 0 && ++bgMode > 4)
      bgMode = 0;
    if(buttonState == -2 && prevButtonState == -1)
    {
      stats = !stats;
      BSP_LCD_SetTextColor(LCD_COLOR_BLACK);
      BSP_LCD_FillRect(0, HT_3D, SCR_WD, SCR_HT - HT_3D);
      if(optim)
        optim = 1;
    }
  }
  else if(buttonState > 0)
  {
    if(++object > MAX_OBJ)
      object = 0;
    msMin = 1000;
    msMax = 0;
  }
  polyMode = 0;
  orient = 0;
  bfCull = 1;
  lightShade = 0;
  switch(object)
  {
    case 0:
      numVerts  = numVertsCubeQ;
      verts     = (int16_t*)vertsCubeQ;
      numPolys  = numQuadsCubeQ;
      polys     = (uint8_t*)quadsCubeQ;
      polyColors = (uint16_t*)colsCubeQ;
      break;
    case 1:
      numVerts  = numVertsCubeQ;
      verts     = (int16_t*)vertsCubeQ;
      numPolys  = numQuadsCubeQ;
      polys     = (uint8_t*)quadsCubeQ;
      lightShade = 44000;
      break;
   case 2:
      numVerts  = numVertsCross;
      verts     = (int16_t*)vertsCross;
      numPolys  = numQuadsCross;
      polys     = (uint8_t*)quadsCross;
      polyColors = (uint16_t*)colsCross;
      break;
   case 3:
      numVerts  = numVertsCross;
      verts     = (int16_t*)vertsCross;
      numPolys  = numQuadsCross;
      polys     = (uint8_t*)quadsCross;
      lightShade = 14000;
      break;
   case 4:
      numVerts  = numVerts3;
      verts     = (int16_t*)verts3;
      numPolys  = numQuads3;
      polys     = (uint8_t*)quads3;
      polyColors = (uint16_t*)cols3;
      break;
   case 5:
      numVerts  = numVerts3;
      verts     = (int16_t*)verts3;
      numPolys  = numQuads3;
      polys     = (uint8_t*)quads3;
      lightShade = 20000;
      break;
   case 6:
      numVerts  = numVertsCubes;
      verts     = (int16_t*)vertsCubes;
      numPolys  = numQuadsCubes;
      polys     = (uint8_t*)quadsCubes;
      polyColors = (uint16_t*)colsCubes;
      bfCull    = 0;
      break;
   case 7:
      numVerts  = numVertsCubes;
      verts     = (int16_t*)vertsCubes;
      numPolys  = numQuadsCubes;
      polys     = (uint8_t*)quadsCubes;
      bfCull    = 1;
      lightShade = 14000;
      break;
   case 8:
      numVerts  = numVertsCone;
      verts     = (int16_t*)vertsCone;
      numPolys  = numTrisCone;
      polys     = (uint8_t*)trisCone;
      polyColors = (uint16_t*)colsCone;
      bfCull    = 1;
      orient    = 1;
      polyMode  = 1;
      break;
   case 9:
      numVerts  = numVertsSphere;
      verts     = (int16_t*)vertsSphere;
      numPolys  = numTrisSphere;
      polys     = (uint8_t*)trisSphere;
      //polyColors = (uint16_t*)colsSphere;
      lightShade = 58000;
      bfCull    = 1;
      orient    = 1;
      polyMode  = 1;
      break;
   case 10:
      numVerts  = numVertsTorus;
      verts     = (int16_t*)vertsTorus;
      numPolys  = numTrisTorus;
      polys     = (uint8_t*)trisTorus;
      polyColors = (uint16_t*)colsTorus;
      bfCull    = 1;
      orient    = 1;
      polyMode  = 1;
      break;
   case 11:
      numVerts  = numVertsTorus;
      verts     = (int16_t*)vertsTorus;
      numPolys  = numTrisTorus;
      polys     = (uint8_t*)trisTorus;
      lightShade = 20000;
      bfCull    = 1;
      orient    = 1;
      polyMode  = 1;
      break;
   case 12:
      numVerts  = numVertsMonkey;
      verts     = (int16_t*)vertsMonkey;
      numPolys  = numTrisMonkey;
      polys     = (uint8_t*)trisMonkey;
      //polyColors = (uint16_t*)colsMonkey;
      lightShade = 20000;
      bfCull    = 1;
      orient    = 1;
      polyMode  = 1;
      break;
  }
  ms = GetTime();
  render3D(polyMode);
  ms = GetTime() - ms;
  if(stats)
    showStats();
}

//-----------------------------------------------------------------------------
void mainApp(void)
{
  setup();
  while(1)
    loop();
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
  for(;;)
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
