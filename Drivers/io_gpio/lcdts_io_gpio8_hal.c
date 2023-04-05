/*
 * 8 bit paralell LCD abd TS GPIO driver for all stm32 family
 * 5 controll pins (CS, RS, WR, RD, RST) + 8 data pins + 1 backlight pin
 */

/*
 * Author: Roberto Benjami
 * version:  2023.04
 */

#include "main.h"
#include "lcd.h"
#include "lcd_io.h"
#include "lcdts_io_gpio8_hal.h"
#include "ts.h"

/* processor family dependent things */
#if defined(STM32C0)
#include "stm32c0xx_ll_gpio.h"
#elif defined(STM32F0)
#include "stm32f0xx_ll_gpio.h"
#elif defined(STM32F1)
#include "stm32f1xx_ll_gpio.h"
#elif defined(STM32F2)
#include "stm32f2xx_ll_gpio.h"
#elif defined(STM32F3)
#include "stm32f3xx_ll_gpio.h"
#elif defined(STM32F4)
#include "stm32f4xx_ll_gpio.h"
#elif defined(STM32F7)
#include "stm32f7xx_ll_gpio.h"
#elif defined(STM32H5)
#include "stm32h5xx_ll_gpio.h"
#elif defined(STM32H7)
#include "stm32h7xx_ll_gpio.h"
#elif defined(STM32G0)
#include "stm32g0xx_ll_gpio.h"
#elif defined(STM32G4)
#include "stm32g4xx_ll_gpio.h"
#elif defined(STM32L0)
#include "stm32l0xx_ll_gpio.h"
#elif defined(STM32L1)
#include "stm32l1xx_ll_gpio.h"
#elif defined(STM32L4)
#include "stm32l4xx_ll_gpio.h"
#elif defined(STM32L5)
#include "stm32l5xx_ll_gpio.h"
#elif defined(STM32WB)
#include "stm32wbxx_ll_gpio.h"
#elif defined(STM32WBA)
#include "stm32wbaxx_ll_gpio.h"
#elif defined(STM32WL)
#include "stm32wlxx_ll_gpio.h"
#else
#error unknown processor family
#endif

//-----------------------------------------------------------------------------
/* Bitdepth convert macros */
#if LCD_RGB24_ORDER == 0
#define  RGB565TO888(c16)     ((c16 & 0xF800) << 8) | ((c16 & 0x07E0) << 5) | ((c16 & 0x001F) << 3)
#define  RGB888TO565(c24)     ((c24 & 0XF80000) >> 8 | (c24 & 0xFC00) >> 5 | (c24 & 0xF8 ) >> 3)
#elif LCD_RGB24_ORDER == 1
#define  RGB565TO888(c16)     ((c16 & 0xF800) >> 8) | ((c16 & 0x07E0) << 5) | ((c16 & 0x001F) << 19)
#define  RGB888TO565(c24)     ((c24 & 0XF80000) >> 19 | (c24 & 0xFC00) >> 5 | (c24 & 0xF8 ) << 8)
#endif /* #elif LCD_RGB24_ORDER == 1 */

//-----------------------------------------------------------------------------
#if LCD_DATADIR == 1

/* Sets the data pins to input */
void LCDDirRead(void)
{
  #ifdef LCD_DIRREAD
  LCD_DIRREAD;
  #else
  LL_GPIO_SetPinMode(LCD_D0_GPIO_Port, LCD_D0_Pin, LL_GPIO_MODE_INPUT);
  LL_GPIO_SetPinMode(LCD_D1_GPIO_Port, LCD_D1_Pin, LL_GPIO_MODE_INPUT);
  LL_GPIO_SetPinMode(LCD_D2_GPIO_Port, LCD_D2_Pin, LL_GPIO_MODE_INPUT);
  LL_GPIO_SetPinMode(LCD_D3_GPIO_Port, LCD_D3_Pin, LL_GPIO_MODE_INPUT);
  LL_GPIO_SetPinMode(LCD_D4_GPIO_Port, LCD_D4_Pin, LL_GPIO_MODE_INPUT);
  LL_GPIO_SetPinMode(LCD_D5_GPIO_Port, LCD_D5_Pin, LL_GPIO_MODE_INPUT);
  LL_GPIO_SetPinMode(LCD_D6_GPIO_Port, LCD_D6_Pin, LL_GPIO_MODE_INPUT);
  LL_GPIO_SetPinMode(LCD_D7_GPIO_Port, LCD_D7_Pin, LL_GPIO_MODE_INPUT);
  #endif
}

//-----------------------------------------------------------------------------
/* Sets the data pins to output */
void LCDDirWrite(void)
{
  #ifdef LCD_DIRWRITE
  LCD_DIRWRITE;
  #else
  LL_GPIO_SetPinMode(LCD_D0_GPIO_Port, LCD_D0_Pin, LL_GPIO_MODE_OUTPUT);
  LL_GPIO_SetPinMode(LCD_D1_GPIO_Port, LCD_D1_Pin, LL_GPIO_MODE_OUTPUT);
  LL_GPIO_SetPinMode(LCD_D2_GPIO_Port, LCD_D2_Pin, LL_GPIO_MODE_OUTPUT);
  LL_GPIO_SetPinMode(LCD_D3_GPIO_Port, LCD_D3_Pin, LL_GPIO_MODE_OUTPUT);
  LL_GPIO_SetPinMode(LCD_D4_GPIO_Port, LCD_D4_Pin, LL_GPIO_MODE_OUTPUT);
  LL_GPIO_SetPinMode(LCD_D5_GPIO_Port, LCD_D5_Pin, LL_GPIO_MODE_OUTPUT);
  LL_GPIO_SetPinMode(LCD_D6_GPIO_Port, LCD_D6_Pin, LL_GPIO_MODE_OUTPUT);
  LL_GPIO_SetPinMode(LCD_D7_GPIO_Port, LCD_D7_Pin, LL_GPIO_MODE_OUTPUT);
  #endif
}

//-----------------------------------------------------------------------------
/* Read the data pins */
uint8_t LCDRead8(void)
{
  uint8_t ret = 0;
  HAL_GPIO_WritePin(LCD_RD_GPIO_Port, LCD_RD_Pin, GPIO_PIN_RESET);
  #ifdef LCD_READ
  LCD_READ(ret);
  #else
  if(HAL_GPIO_ReadPin(LCD_D0_GPIO_Port, LCD_D0_Pin))
    ret = 0x01;
  if(HAL_GPIO_ReadPin(LCD_D1_GPIO_Port, LCD_D1_Pin))
    ret |= 0x02;
  if(HAL_GPIO_ReadPin(LCD_D2_GPIO_Port, LCD_D2_Pin))
    ret |= 0x04;
  if(HAL_GPIO_ReadPin(LCD_D3_GPIO_Port, LCD_D3_Pin))
    ret |= 0x08;
  if(HAL_GPIO_ReadPin(LCD_D4_GPIO_Port, LCD_D4_Pin))
    ret |= 0x10;
  if(HAL_GPIO_ReadPin(LCD_D5_GPIO_Port, LCD_D5_Pin))
    ret |= 0x20;
  if(HAL_GPIO_ReadPin(LCD_D6_GPIO_Port, LCD_D6_Pin))
    ret |= 0x40;
  if(HAL_GPIO_ReadPin(LCD_D7_GPIO_Port, LCD_D7_Pin))
    ret |= 0x80;
  #endif
  HAL_GPIO_WritePin(LCD_RD_GPIO_Port, LCD_RD_Pin, GPIO_PIN_SET);
  return ret;
}
#endif

//-----------------------------------------------------------------------------
/* Write the data pins */
void LCDWrite8(uint8_t dt8)
{
  #ifdef LCD_WRITE
  LCD_WRITE(dt8);
  #else
  static uint8_t pre_dt8 = 0;
  if(dt8 != pre_dt8)
  {
    pre_dt8 = dt8;
    HAL_GPIO_WritePin(LCD_D0_GPIO_Port, LCD_D0_Pin, dt8 & 0x01);
    HAL_GPIO_WritePin(LCD_D1_GPIO_Port, LCD_D1_Pin, dt8 & 0x02);
    HAL_GPIO_WritePin(LCD_D2_GPIO_Port, LCD_D2_Pin, dt8 & 0x04);
    HAL_GPIO_WritePin(LCD_D3_GPIO_Port, LCD_D3_Pin, dt8 & 0x08);
    HAL_GPIO_WritePin(LCD_D4_GPIO_Port, LCD_D4_Pin, dt8 & 0x10);
    HAL_GPIO_WritePin(LCD_D5_GPIO_Port, LCD_D5_Pin, dt8 & 0x20);
    HAL_GPIO_WritePin(LCD_D6_GPIO_Port, LCD_D6_Pin, dt8 & 0x40);
    HAL_GPIO_WritePin(LCD_D7_GPIO_Port, LCD_D7_Pin, dt8 & 0x80);
  }
  #endif
  HAL_GPIO_WritePin(LCD_WR_GPIO_Port, LCD_WR_Pin, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(LCD_WR_GPIO_Port, LCD_WR_Pin, GPIO_PIN_SET);
}

//=============================================================================
/* Wrtite fill and multi data to Lcd (8 and 16 bit mode)
   - pData: 8 or 16 bits data pointer
   - Size: data number
   - dinc: 0=fill mode, 1=multidata mode
   - bitdepth: 0 = 8bit data, 1 = 16bit data */
void LCDWriteFillMultiData8and16(uint8_t * pData, uint32_t Size, uint32_t Mode)
{
  if(Mode & LCD_IO_FILL)
  { /* fill */
    if(Mode & LCD_IO_DATA8)
    { /* fill 8bit */
      while(Size--) /* fill 8bit */
        LCDWrite8(*pData);
    }
    else if(Mode & LCD_IO_DATA16)
    { /* fill 16bit */
      while(Size--)
      {
        LCDWrite8((*(uint16_t *)pData) >> 8);
        LCDWrite8(*(uint16_t *)pData);
      }
    }
  }
  else
  { /* multidata */
    if(Mode & LCD_IO_DATA8)
    { /* multidata 8bit */
      while(Size--)
      {
        LCDWrite8(*pData);
        pData++;
      }
    }
    else
    { /* multidata 16bit */
      {
        while(Size--)
        {
          LCDWrite8((*(uint16_t *)pData) >> 8);
          LCDWrite8(*(uint16_t *)pData);
          pData += 2;
        }
      }
    }
  }
}

//-----------------------------------------------------------------------------
/* Wrtite fill and multi data to Lcd (convert RGB16 bit (5-6-5) to RGB24 bit (8-8-8) mode, no dma capability)
   - pData: RGB 16 bits data pointer
   - Size: data number
   - dinc: 0=fill mode, 1=multidata mode */
void LCDWriteFillMultiData16to24(uint16_t * pData, uint32_t Size, uint32_t Mode)
{
  union
  {
    uint8_t  c8[4];
    uint32_t c24;
  }rgb888;

  if(Mode & LCD_IO_FILL)
  { /* fill 16bit to 24bit */
    rgb888.c24 = RGB565TO888(*pData);
    while(Size--)
    {
      LCDWrite8(rgb888.c8[0]);
      LCDWrite8(rgb888.c8[1]);
      LCDWrite8(rgb888.c8[2]);
    }
  }
  else
  { /* multidata 16bit to 24bit */
    while(Size--)
    {
      rgb888.c24 = RGB565TO888(*pData);
      LCDWrite8(rgb888.c8[0]);
      LCDWrite8(rgb888.c8[1]);
      LCDWrite8(rgb888.c8[2]);
      pData++;
    }
  }
}

//=============================================================================
/* Read */

#if LCD_DATADIR == 1

//-----------------------------------------------------------------------------
/* Read data from Lcd
   - pData: 8 or 16 bits data pointer
   - Size: data number
   - bitdepth: 0 = 8bit data, 1 = 16bit data */
void LCDReadMultiData8and16(uint8_t * pData, uint32_t Size, uint32_t Mode)
{
  { /* not DMA mode */
    if(Mode & LCD_IO_DATA8)
    { /* 8bit */
      while(Size--)
      {
        *pData = LCDRead8();
        pData++;
      }
    }
    else
    { /* 16bit */
      while(Size--)
      {
        *(uint16_t *)pData = (LCDRead8() << 8) | LCDRead8();
        pData += 2;
      }
    }
  }
}

//-----------------------------------------------------------------------------
/* Read 24bit (8-8-8) RGB data from LCD, and convert to 16bit (5-6-5) RGB data
   - pData: 16 bits RGB data pointer
   - Size: pixel number */
void LCDReadMultiData24to16(uint16_t * pData, uint32_t Size, uint32_t Mode)
{
  union
  {
    uint8_t  c8[4];
    uint32_t c24;
  }rgb888;

  { /* not DMA mode */
    while(Size--)
    {
      rgb888.c8[0] = LCDRead8();
      rgb888.c8[1] = LCDRead8();
      rgb888.c8[2] = LCDRead8();
      *pData = RGB888TO565(rgb888.c24);
      pData++;
    }
  }
}

#endif /* #if LCD_DATADIR == 1 */

//=============================================================================
/* Public functions */

/* n millisec delay */
void LCD_Delay(uint32_t Delay)
{
  #ifndef  osCMSIS
  HAL_Delay(Delay);
  #else
  osDelay(Delay);
  #endif
}

/* Backlight on-off (Bl=0 -> off, Bl=1 -> on) */
//-----------------------------------------------------------------------------
void LCD_IO_Bl_OnOff(uint8_t Bl)
{
  #if defined(LCD_BL_GPIO_Port) && defined (LCD_BL_Pin)
  if(Bl)
    #if LCD_BLON == 0
    HAL_GPIO_WritePin(LCD_BL_GPIO_Port, LCD_BL_Pin, GPIO_PIN_RESET);
    #elif LCD_BLON == 1
    HAL_GPIO_WritePin(LCD_BL_GPIO_Port, LCD_BL_Pin, GPIO_PIN_SET);
    #endif
  else
    #if LCD_BLON == 0
    HAL_GPIO_WritePin(LCD_BL_GPIO_Port, LCD_BL_Pin, GPIO_PIN_SET);
    #elif LCD_BLON == 1
    HAL_GPIO_WritePin(LCD_BL_GPIO_Port, LCD_BL_Pin, GPIO_PIN_RESET);
    #endif
  #endif
}

//-----------------------------------------------------------------------------
/* Lcd IO init, reset, spi speed init, get the freertos task id */
void LCD_IO_Init(void)
{
  #if defined(LCD_RST_GPIO_Port) && defined (LCD_RST_Pin)
  HAL_Delay(10);
  HAL_GPIO_WritePin(LCD_RST_GPIO_Port, LCD_RST_Pin, GPIO_PIN_RESET);
  HAL_Delay(10);
  HAL_GPIO_WritePin(LCD_RST_GPIO_Port, LCD_RST_Pin, GPIO_PIN_SET);
  #endif
  LCD_Delay(10);
}

//-----------------------------------------------------------------------------
/* Lcd IO transaction
   - Cmd: 8 or 16 bits command
   - pData: 8 or 16 bits data pointer
   - Size: data number
   - DummySize: dummy byte number at read
   - Mode: 8 or 16 or 24 bit mode, write or read, fill or multidata (see the LCD_IO_... defines) */
void LCD_IO_Transaction(uint16_t Cmd, uint8_t *pData, uint32_t Size, uint32_t DummySize, uint32_t Mode)
{
  #if LCD_DATADIR == 0  /* only TX mode */
  if(Mode & LCD_IO_READ)
    return;
  #endif

  HAL_GPIO_WritePin(LCD_CS_GPIO_Port, LCD_CS_Pin, GPIO_PIN_RESET);

  /* Command write */
  HAL_GPIO_WritePin(LCD_RS_GPIO_Port, LCD_RS_Pin, GPIO_PIN_RESET);
  if(Mode & LCD_IO_CMD8)
    LCDWrite8(Cmd);
  else if(Mode & LCD_IO_CMD16)
  {
    LCDWrite8(Cmd >> 8);
    LCDWrite8(Cmd);
  }
  HAL_GPIO_WritePin(LCD_RS_GPIO_Port, LCD_RS_Pin, GPIO_PIN_SET);

  if(Size == 0)
  { /* only command byte or word */
    HAL_GPIO_WritePin(LCD_CS_GPIO_Port, LCD_CS_Pin, GPIO_PIN_SET);
    return;
  }

  /* Datas write or read */
  if(Mode & LCD_IO_WRITE)
  { /* Write Lcd */
    if(Mode & LCD_IO_DATA16TO24)
      LCDWriteFillMultiData16to24((uint16_t *)pData, Size, Mode);
    else
      LCDWriteFillMultiData8and16(pData, Size, Mode);
  }
  #if LCD_DATADIR == 1
  else if(Mode & LCD_IO_READ)
  { /* Read LCD */
    LCDDirRead();
    uint8_t RxDummy __attribute__((unused));
    while(DummySize--)
      RxDummy = LCDRead8();
    if(Mode & LCD_IO_DATA24TO16)
      LCDReadMultiData24to16((uint16_t *)pData, Size, Mode);
    else
      LCDReadMultiData8and16(pData, Size, Mode);
    LCDDirWrite();
  }
  #endif /* #if LCD_DATADIR == 1 */
  HAL_GPIO_WritePin(LCD_CS_GPIO_Port, LCD_CS_Pin, GPIO_PIN_SET);
}

//=============================================================================

#define ABS(N)                (((N)<0) ? (-(N)) : (N))

//=============================================================================

static  uint16_t  tx, ty;

extern  ADC_HandleTypeDef     TS_AD_HANDLE;

//=============================================================================
/* TS chip select pin set */
void    ts_Init(uint16_t DeviceAddr);
uint8_t ts_DetectTouch(uint16_t DeviceAddr);
void    ts_GetXY(uint16_t DeviceAddr, uint16_t *X, uint16_t *Y);

//=============================================================================
#ifdef  __GNUC__
#pragma GCC push_options
#pragma GCC optimize("O0")
#elif   defined(__CC_ARM)
#pragma push
#pragma O0
#endif
void TS_IO_Delay(uint32_t c)
{
  while(c--);
}
#ifdef  __GNUC__
#pragma GCC pop_options
#elif   defined(__CC_ARM)
#pragma pop
#endif
//-----------------------------------------------------------------------------
void TS_IO_Init(void)
{
}

uint16_t TS_IO_GetAd(uint32_t chn)
{
  uint16_t ret = 0;
  ADC_ChannelConfTypeDef sConfig = {0};

  sConfig.Channel = chn;
  sConfig.Rank = ADC_REGULAR_RANK_1;
  sConfig.SamplingTime = ADC_SAMPLETIME_15CYCLES;
  if (HAL_ADC_ConfigChannel(&TS_AD_HANDLE, &sConfig) == HAL_OK)
  {
    TS_IO_Delay(TS_AD_DELAY);
    for(uint32_t i = 0; i < 4; i++)
    {
      HAL_ADC_Start(&TS_AD_HANDLE);
      HAL_ADC_PollForConversion(&TS_AD_HANDLE, 100);
      ret += HAL_ADC_GetValue(&TS_AD_HANDLE);
    }
  }
  return (ret >> 2);
}

//-----------------------------------------------------------------------------
/* return:
   - 0 : touchscreen is not pressed
   - 1 : touchscreen is pressed */
uint8_t TS_IO_DetectToch(void)
{
  uint8_t ret;
  LL_GPIO_SetPinMode(LCD_D7_GPIO_Port, LCD_D7_Pin, LL_GPIO_MODE_INPUT); /* YM = D_INPUT */
  LL_GPIO_SetPinMode(LCD_WR_GPIO_Port, LCD_WR_Pin, LL_GPIO_MODE_INPUT); /* YP = D_INPUT */
  HAL_GPIO_WritePin(LCD_RS_GPIO_Port, LCD_RS_Pin, GPIO_PIN_RESET);      /* XM = 0 */
  HAL_GPIO_WritePin(LCD_D6_GPIO_Port, LCD_D6_Pin, GPIO_PIN_RESET);      /* XP = 0 */
  LL_GPIO_SetPinPull(LCD_WR_GPIO_Port, LCD_WR_Pin, LL_GPIO_PULL_UP);    /* YP pullup resistor on */
  TS_IO_Delay(TS_AD_DELAY);
  if(HAL_GPIO_ReadPin(LCD_WR_GPIO_Port, LCD_WR_Pin))                    /* YP ? */
    ret = 0;                                                            /* Touchscreen is not touch */
  else
    ret = 1;                                                            /* Touchscreen is touch */
  LL_GPIO_SetPinPull(LCD_WR_GPIO_Port, LCD_WR_Pin, LL_GPIO_PULL_NO);    /* YP pullup resistor off */
  HAL_GPIO_WritePin(LCD_RS_GPIO_Port, LCD_RS_Pin, GPIO_PIN_SET);        /* XM = 1 */
  HAL_GPIO_WritePin(LCD_D6_GPIO_Port, LCD_D6_Pin, GPIO_PIN_SET);        /* XP = 1 */
  LL_GPIO_SetPinMode(LCD_D7_GPIO_Port, LCD_D7_Pin, LL_GPIO_MODE_OUTPUT);/* YM = OUT */
  LL_GPIO_SetPinMode(LCD_WR_GPIO_Port, LCD_WR_Pin, LL_GPIO_MODE_OUTPUT);/* YP = OUT */
  return ret;
}

//-----------------------------------------------------------------------------
/* read the X position */
uint16_t TS_IO_GetX(void)
{
  uint16_t ret;
  LL_GPIO_SetPinMode(LCD_D7_GPIO_Port, LCD_D7_Pin, LL_GPIO_MODE_INPUT); /* YM = D_INPUT */
  LL_GPIO_SetPinMode(LCD_WR_GPIO_Port, LCD_WR_Pin, LL_GPIO_MODE_ANALOG);/* YP = AN_INPUT */
  HAL_GPIO_WritePin(LCD_RS_GPIO_Port, LCD_RS_Pin, GPIO_PIN_SET);        /* XM = 1 */
  HAL_GPIO_WritePin(LCD_D6_GPIO_Port, LCD_D6_Pin, GPIO_PIN_RESET);      /* XP = 0 */
  ret = TS_IO_GetAd(TS_WR_ADCCH);                                       /* Ad Converter TS_YP_ADCCH */
  HAL_GPIO_WritePin(LCD_D6_GPIO_Port, LCD_D6_Pin, GPIO_PIN_SET);        /* XP = 1 */
  LL_GPIO_SetPinMode(LCD_D7_GPIO_Port, LCD_D7_Pin, LL_GPIO_MODE_OUTPUT);/* YM = OUT */
  LL_GPIO_SetPinMode(LCD_WR_GPIO_Port, LCD_WR_Pin, LL_GPIO_MODE_OUTPUT);/* YP = OUT */
  return ret;
}

//-----------------------------------------------------------------------------
/* read the Y position */
uint16_t TS_IO_GetY(void)
{
  uint16_t ret;
  LL_GPIO_SetPinMode(LCD_RS_GPIO_Port, LCD_RS_Pin, LL_GPIO_MODE_ANALOG);/* XM = AN_INPUT */
  LL_GPIO_SetPinMode(LCD_D6_GPIO_Port, LCD_D6_Pin, LL_GPIO_MODE_INPUT); /* XP = D_INPUT */
  HAL_GPIO_WritePin(LCD_D7_GPIO_Port, LCD_D7_Pin, GPIO_PIN_RESET);      /* YM = 0 */
  HAL_GPIO_WritePin(LCD_WR_GPIO_Port, LCD_WR_Pin, GPIO_PIN_SET);        /* YP = 1 */
  ret = TS_IO_GetAd(TS_RS_ADCCH);                                       /* Ad Converter TS_XM_ADCCH */
  HAL_GPIO_WritePin(LCD_D7_GPIO_Port, LCD_D7_Pin, GPIO_PIN_SET);        /* YM = 1 */
  LL_GPIO_SetPinMode(LCD_RS_GPIO_Port, LCD_RS_Pin, LL_GPIO_MODE_OUTPUT);/* XM = OUT */
  LL_GPIO_SetPinMode(LCD_D6_GPIO_Port, LCD_D6_Pin, LL_GPIO_MODE_OUTPUT);/* XP = OUT */
  return ret;
}

TS_DrvTypeDef   gpio_ts_drv =
{
  ts_Init,
  0,
  0,
  0,
  ts_DetectTouch,
  ts_GetXY,
  0,
  0,
  0,
  0
};

TS_DrvTypeDef  *ts_drv = &gpio_ts_drv;


//-----------------------------------------------------------------------------
void ts_Init(uint16_t DeviceAddr)
{
}

//-----------------------------------------------------------------------------
uint8_t ts_DetectTouch(uint16_t DeviceAddr)
{
  static uint8_t ret = 0;
  int32_t x1, x2, y1, y2, i;

  ret = 0;
  if(TS_IO_DetectToch())
  {
    x1 = TS_IO_GetX(); /* Get X */
    y1 = TS_IO_GetY(); /* Get Y */
    i = TOUCH_MAXREPEAT;
    while(i--)
    {
      x2 = TS_IO_GetX(); /* Get X */
      y2 = TS_IO_GetY(); /* Get Y */
      if((ABS(x1 - x2) < TOUCH_FILTER) && (ABS(y1 - y2) < TOUCH_FILTER))
      {
        x1 = (x1 + x2) >> 1;
        y1 = (y1 + y2) >> 1;
        i = 0;
        if(TS_IO_DetectToch())
        {
          tx = x1;
          ty = y1;
          ret = 1;
        }
      }
      else
      {
        x1 = x2;
        y1 = y2;
      }
    }
  }
  return ret;
}

//-----------------------------------------------------------------------------
void ts_GetXY(uint16_t DeviceAddr, uint16_t *X, uint16_t *Y)
{
  *X = tx,
  *Y = ty;
}
