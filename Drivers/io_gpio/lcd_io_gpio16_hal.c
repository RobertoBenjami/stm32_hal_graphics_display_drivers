/*
 * 16 bit paralell LCD GPIO driver for all stm32 family
 * 5 controll pins (CS, RS, WR, RD, RST) + 16 data pins + 1 backlight pin
 */

/*
 * Author: Roberto Benjami
 * version:  2023.01
 */

#include "main.h"
#include "lcd.h"
#include "lcd_io.h"
#include "lcd_io_gpio16_hal.h"

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
  LL_GPIO_SetPinMode(LCD_D8_GPIO_Port, LCD_D8_Pin, LL_GPIO_MODE_INPUT);
  LL_GPIO_SetPinMode(LCD_D9_GPIO_Port, LCD_D9_Pin, LL_GPIO_MODE_INPUT);
  LL_GPIO_SetPinMode(LCD_D10_GPIO_Port, LCD_D10_Pin, LL_GPIO_MODE_INPUT);
  LL_GPIO_SetPinMode(LCD_D11_GPIO_Port, LCD_D11_Pin, LL_GPIO_MODE_INPUT);
  LL_GPIO_SetPinMode(LCD_D12_GPIO_Port, LCD_D12_Pin, LL_GPIO_MODE_INPUT);
  LL_GPIO_SetPinMode(LCD_D13_GPIO_Port, LCD_D13_Pin, LL_GPIO_MODE_INPUT);
  LL_GPIO_SetPinMode(LCD_D14_GPIO_Port, LCD_D14_Pin, LL_GPIO_MODE_INPUT);
  LL_GPIO_SetPinMode(LCD_D15_GPIO_Port, LCD_D15_Pin, LL_GPIO_MODE_INPUT);
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
  LL_GPIO_SetPinMode(LCD_D8_GPIO_Port, LCD_D8_Pin, LL_GPIO_MODE_OUTPUT);
  LL_GPIO_SetPinMode(LCD_D9_GPIO_Port, LCD_D9_Pin, LL_GPIO_MODE_OUTPUT);
  LL_GPIO_SetPinMode(LCD_D10_GPIO_Port, LCD_D10_Pin, LL_GPIO_MODE_OUTPUT);
  LL_GPIO_SetPinMode(LCD_D11_GPIO_Port, LCD_D11_Pin, LL_GPIO_MODE_OUTPUT);
  LL_GPIO_SetPinMode(LCD_D12_GPIO_Port, LCD_D12_Pin, LL_GPIO_MODE_OUTPUT);
  LL_GPIO_SetPinMode(LCD_D13_GPIO_Port, LCD_D13_Pin, LL_GPIO_MODE_OUTPUT);
  LL_GPIO_SetPinMode(LCD_D14_GPIO_Port, LCD_D14_Pin, LL_GPIO_MODE_OUTPUT);
  LL_GPIO_SetPinMode(LCD_D15_GPIO_Port, LCD_D15_Pin, LL_GPIO_MODE_OUTPUT);
  #endif
}

//-----------------------------------------------------------------------------
/* Read the data pins */
uint8_t LCDRead8(void)
{
  uint8_t ret = 0;
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
  return ret;
}

//-----------------------------------------------------------------------------
/* Read the data pins */
uint16_t LCDRead16(void)
{
  uint16_t ret = 0;
  #ifdef LCD_READ
  LCD_READ(ret);
  #else
  if(HAL_GPIO_ReadPin(LCD_D0_GPIO_Port, LCD_D0_Pin))
    ret = 0x0001;
  if(HAL_GPIO_ReadPin(LCD_D1_GPIO_Port, LCD_D1_Pin))
    ret |= 0x0002;
  if(HAL_GPIO_ReadPin(LCD_D2_GPIO_Port, LCD_D2_Pin))
    ret |= 0x0004;
  if(HAL_GPIO_ReadPin(LCD_D3_GPIO_Port, LCD_D3_Pin))
    ret |= 0x0008;
  if(HAL_GPIO_ReadPin(LCD_D4_GPIO_Port, LCD_D4_Pin))
    ret |= 0x0010;
  if(HAL_GPIO_ReadPin(LCD_D5_GPIO_Port, LCD_D5_Pin))
    ret |= 0x0020;
  if(HAL_GPIO_ReadPin(LCD_D6_GPIO_Port, LCD_D6_Pin))
    ret |= 0x0040;
  if(HAL_GPIO_ReadPin(LCD_D7_GPIO_Port, LCD_D7_Pin))
    ret |= 0x0080;
  if(HAL_GPIO_ReadPin(LCD_D8_GPIO_Port, LCD_D8_Pin))
    ret = 0x0100;
  if(HAL_GPIO_ReadPin(LCD_D9_GPIO_Port, LCD_D9_Pin))
    ret |= 0x0200;
  if(HAL_GPIO_ReadPin(LCD_D10_GPIO_Port, LCD_D10_Pin))
    ret |= 0x0400;
  if(HAL_GPIO_ReadPin(LCD_D11_GPIO_Port, LCD_D11_Pin))
    ret |= 0x0800;
  if(HAL_GPIO_ReadPin(LCD_D12_GPIO_Port, LCD_D12_Pin))
    ret |= 0x1000;
  if(HAL_GPIO_ReadPin(LCD_D13_GPIO_Port, LCD_D13_Pin))
    ret |= 0x2000;
  if(HAL_GPIO_ReadPin(LCD_D14_GPIO_Port, LCD_D14_Pin))
    ret |= 0x4000;
  if(HAL_GPIO_ReadPin(LCD_D15_GPIO_Port, LCD_D15_Pin))
    ret |= 0x8000;
  #endif
  return ret;
}
#endif

union
{
  uint16_t dt16;
  uint8_t  dt8;
}
pre_dt;

//-----------------------------------------------------------------------------
/* Write the data pins */
void LCDWrite8(uint8_t dt)
{
  #ifdef LCD_WRITE
  LCD_WRITE((uint16_t)dt);
  #else
  if(dt != pre_dt.dt8)
  {
    pre_dt.dt8 = dt;
    HAL_GPIO_WritePin(LCD_D0_GPIO_Port, LCD_D0_Pin, dt & 0x01);
    HAL_GPIO_WritePin(LCD_D1_GPIO_Port, LCD_D1_Pin, dt & 0x02);
    HAL_GPIO_WritePin(LCD_D2_GPIO_Port, LCD_D2_Pin, dt & 0x04);
    HAL_GPIO_WritePin(LCD_D3_GPIO_Port, LCD_D3_Pin, dt & 0x08);
    HAL_GPIO_WritePin(LCD_D4_GPIO_Port, LCD_D4_Pin, dt & 0x10);
    HAL_GPIO_WritePin(LCD_D5_GPIO_Port, LCD_D5_Pin, dt & 0x20);
    HAL_GPIO_WritePin(LCD_D6_GPIO_Port, LCD_D6_Pin, dt & 0x40);
    HAL_GPIO_WritePin(LCD_D7_GPIO_Port, LCD_D7_Pin, dt & 0x80);
  }
  #endif
}

/* Write the data pins */
void LCDWrite16(uint16_t dt)
{
  #ifdef LCD_WRITE
  LCD_WRITE(dt);
  #else
  if(dt != pre_dt.dt16)
  {
    pre_dt.dt16 = dt;
    HAL_GPIO_WritePin(LCD_D0_GPIO_Port, LCD_D0_Pin, dt & 0x0001);
    HAL_GPIO_WritePin(LCD_D1_GPIO_Port, LCD_D1_Pin, dt & 0x0002);
    HAL_GPIO_WritePin(LCD_D2_GPIO_Port, LCD_D2_Pin, dt & 0x0004);
    HAL_GPIO_WritePin(LCD_D3_GPIO_Port, LCD_D3_Pin, dt & 0x0008);
    HAL_GPIO_WritePin(LCD_D4_GPIO_Port, LCD_D4_Pin, dt & 0x0010);
    HAL_GPIO_WritePin(LCD_D5_GPIO_Port, LCD_D5_Pin, dt & 0x0020);
    HAL_GPIO_WritePin(LCD_D6_GPIO_Port, LCD_D6_Pin, dt & 0x0040);
    HAL_GPIO_WritePin(LCD_D7_GPIO_Port, LCD_D7_Pin, dt & 0x0080);
    HAL_GPIO_WritePin(LCD_D8_GPIO_Port, LCD_D8_Pin, ((dt & 0x0100) == 0x0100));
    HAL_GPIO_WritePin(LCD_D9_GPIO_Port, LCD_D9_Pin, ((dt & 0x0200) == 0x0200));
    HAL_GPIO_WritePin(LCD_D10_GPIO_Port, LCD_D10_Pin, ((dt & 0x0400) == 0x0400));
    HAL_GPIO_WritePin(LCD_D11_GPIO_Port, LCD_D11_Pin, ((dt & 0x0800) == 0x0800));
    HAL_GPIO_WritePin(LCD_D12_GPIO_Port, LCD_D12_Pin, ((dt & 0x1000) == 0x1000));
    HAL_GPIO_WritePin(LCD_D13_GPIO_Port, LCD_D13_Pin, ((dt & 0x2000) == 0x2000));
    HAL_GPIO_WritePin(LCD_D14_GPIO_Port, LCD_D14_Pin, ((dt & 0x4000) == 0x4000));
    HAL_GPIO_WritePin(LCD_D15_GPIO_Port, LCD_D15_Pin, ((dt & 0x8000) == 0x8000));
  }
  #endif
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
      LCDWrite8(*pData);
      while(Size--) /* fill 8bit */
      {
        HAL_GPIO_WritePin(LCD_WR_GPIO_Port, LCD_WR_Pin, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(LCD_WR_GPIO_Port, LCD_WR_Pin, GPIO_PIN_SET);
      }
    }
    else if(Mode & LCD_IO_DATA16)
    { /* fill 16bit */
      LCDWrite16(*(uint16_t *)pData);
      while(Size--)
      {
        HAL_GPIO_WritePin(LCD_WR_GPIO_Port, LCD_WR_Pin, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(LCD_WR_GPIO_Port, LCD_WR_Pin, GPIO_PIN_SET);
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
        HAL_GPIO_WritePin(LCD_WR_GPIO_Port, LCD_WR_Pin, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(LCD_WR_GPIO_Port, LCD_WR_Pin, GPIO_PIN_SET);
        pData++;
      }
    }
    else
    { /* multidata 16bit */
      {
        while(Size--)
        {
          LCDWrite16(*(uint16_t *)pData);
          HAL_GPIO_WritePin(LCD_WR_GPIO_Port, LCD_WR_Pin, GPIO_PIN_RESET);
          HAL_GPIO_WritePin(LCD_WR_GPIO_Port, LCD_WR_Pin, GPIO_PIN_SET);
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
    uint16_t c16[2];
    uint32_t c24;
  }rgb888;

  #if LCD_RGB24_MODE == 0
  uint8_t ccnt = 0, ctmp = 0; /* color counter  (even and odd pixels), color temp */

  if(Mode & LCD_IO_FILL)
  { /* fill 16bit to 24bit */
    rgb888.c24 = RGB565TO888(*pData);
    while(Size--)
    {
      if(!ccnt)
      {
        ccnt = 1;
        LCDWrite16((rgb888.c8[2] << 8) | rgb888.c8[1]);
        HAL_GPIO_WritePin(LCD_WR_GPIO_Port, LCD_WR_Pin, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(LCD_WR_GPIO_Port, LCD_WR_Pin, GPIO_PIN_SET);
        LCDWrite16((rgb888.c8[0] << 8) | rgb888.c8[2]);
        HAL_GPIO_WritePin(LCD_WR_GPIO_Port, LCD_WR_Pin, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(LCD_WR_GPIO_Port, LCD_WR_Pin, GPIO_PIN_SET);
      }
      else
      {
        ccnt = 0;
        LCDWrite16((rgb888.c8[1] << 8) | rgb888.c8[0]);
        HAL_GPIO_WritePin(LCD_WR_GPIO_Port, LCD_WR_Pin, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(LCD_WR_GPIO_Port, LCD_WR_Pin, GPIO_PIN_SET);
      }
    }
  }
  else
  { /* multidata 16bit to 24bit */
    while(Size--)
    {
      rgb888.c24 = RGB565TO888(*pData);
      pData++;
      if(!ccnt)
      {
        ccnt = 1;
        LCDWrite16((rgb888.c8[2] << 8) | rgb888.c8[1]);
        ctmp = rgb888.c8[0];
        HAL_GPIO_WritePin(LCD_WR_GPIO_Port, LCD_WR_Pin, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(LCD_WR_GPIO_Port, LCD_WR_Pin, GPIO_PIN_SET);
      }
      else
      {
        ccnt = 0;
        LCDWrite16((ctmp << 8) | rgb888.c8[2]);
        HAL_GPIO_WritePin(LCD_WR_GPIO_Port, LCD_WR_Pin, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(LCD_WR_GPIO_Port, LCD_WR_Pin, GPIO_PIN_SET);
        LCDWrite16(rgb888.c16[0]);
        HAL_GPIO_WritePin(LCD_WR_GPIO_Port, LCD_WR_Pin, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(LCD_WR_GPIO_Port, LCD_WR_Pin, GPIO_PIN_SET);
      }
    }
    if(!ccnt)
    {
      LCDWrite16(ctmp << 8);
      HAL_GPIO_WritePin(LCD_WR_GPIO_Port, LCD_WR_Pin, GPIO_PIN_RESET);
      HAL_GPIO_WritePin(LCD_WR_GPIO_Port, LCD_WR_Pin, GPIO_PIN_SET);
    }
  }

  #elif LCD_RGB24_MODE == 1
  if(Mode & LCD_IO_FILL)
  { /* fill 16bit to 24bit */
    rgb888.c24 = RGB565TO888(*pData);
    while(Size--)
    {
      LCDWrite16(rgb888.c16[0]);
      HAL_GPIO_WritePin(LCD_WR_GPIO_Port, LCD_WR_Pin, GPIO_PIN_RESET);
      HAL_GPIO_WritePin(LCD_WR_GPIO_Port, LCD_WR_Pin, GPIO_PIN_SET);
      LCDWrite16(rgb888.c16[1]);
      HAL_GPIO_WritePin(LCD_WR_GPIO_Port, LCD_WR_Pin, GPIO_PIN_RESET);
      HAL_GPIO_WritePin(LCD_WR_GPIO_Port, LCD_WR_Pin, GPIO_PIN_SET);
    }
  }
  else
  { /* multidata 16bit to 24bit */
    while(Size--)
    {
      rgb888.c24 = RGB565TO888(*pData++);
      LCDWrite16(rgb888.c16[0]);
      HAL_GPIO_WritePin(LCD_WR_GPIO_Port, LCD_WR_Pin, GPIO_PIN_RESET);
      HAL_GPIO_WritePin(LCD_WR_GPIO_Port, LCD_WR_Pin, GPIO_PIN_SET);
      LCDWrite16(rgb888.c16[1]);
      HAL_GPIO_WritePin(LCD_WR_GPIO_Port, LCD_WR_Pin, GPIO_PIN_RESET);
      HAL_GPIO_WritePin(LCD_WR_GPIO_Port, LCD_WR_Pin, GPIO_PIN_SET);
    }
  }
  #endif
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
        HAL_GPIO_WritePin(LCD_RD_GPIO_Port, LCD_RD_Pin, GPIO_PIN_RESET);
        *pData = LCDRead8();
        HAL_GPIO_WritePin(LCD_RD_GPIO_Port, LCD_RD_Pin, GPIO_PIN_SET);
        pData++;
      }
    }
    else
    { /* 16bit */
      while(Size--)
      {
        HAL_GPIO_WritePin(LCD_RD_GPIO_Port, LCD_RD_Pin, GPIO_PIN_RESET);
        *(uint16_t *)pData = LCDRead16();
        HAL_GPIO_WritePin(LCD_RD_GPIO_Port, LCD_RD_Pin, GPIO_PIN_SET);
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
    uint16_t c16[2];
    uint32_t c24;
  }rgb888;

  #if LCD_RGB24_MODE == 0
  uint8_t ccnt = 0, ctmp = 0; /* color counter (even and odd pixels), color temp */
  while(Size--)
  {
    if(!ccnt)
    {
      ccnt = 1;
      HAL_GPIO_WritePin(LCD_RD_GPIO_Port, LCD_RD_Pin, GPIO_PIN_RESET);
      rgb888.c16[1] = LCDRead16();
      HAL_GPIO_WritePin(LCD_RD_GPIO_Port, LCD_RD_Pin, GPIO_PIN_SET);
      HAL_GPIO_WritePin(LCD_RD_GPIO_Port, LCD_RD_Pin, GPIO_PIN_RESET);
      rgb888.c16[0] = LCDRead16();
      HAL_GPIO_WritePin(LCD_RD_GPIO_Port, LCD_RD_Pin, GPIO_PIN_SET);
      ctmp = rgb888.c8[0];
      rgb888.c24 >>= 8;
    }
    else
    {
      ccnt = 0;
      HAL_GPIO_WritePin(LCD_RD_GPIO_Port, LCD_RD_Pin, GPIO_PIN_RESET);
      rgb888.c16[0] = LCDRead16();
      HAL_GPIO_WritePin(LCD_RD_GPIO_Port, LCD_RD_Pin, GPIO_PIN_SET);
      rgb888.c8[2] = ctmp;
    }
    *pData = RGB888TO565(rgb888.c24);
    pData++;
  }

  #elif LCD_RGB24_MODE == 1
  while(Size--)
  {
    HAL_GPIO_WritePin(LCD_RD_GPIO_Port, LCD_RD_Pin, GPIO_PIN_RESET);
    rgb888.c16[0] = LCDRead16();
    HAL_GPIO_WritePin(LCD_RD_GPIO_Port, LCD_RD_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(LCD_RD_GPIO_Port, LCD_RD_Pin, GPIO_PIN_RESET);
    rgb888.c16[1] = LCDRead16();
    HAL_GPIO_WritePin(LCD_RD_GPIO_Port, LCD_RD_Pin, GPIO_PIN_SET);
    *pData = RGB888TO565(rgb888.c24);
    pData++;
  }
  #endif
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
  {
    LCDWrite8(Cmd);
    HAL_GPIO_WritePin(LCD_WR_GPIO_Port, LCD_WR_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(LCD_WR_GPIO_Port, LCD_WR_Pin, GPIO_PIN_SET);
  }
  else if(Mode & LCD_IO_CMD16)
  {
    LCDWrite8(Cmd >> 8);
    HAL_GPIO_WritePin(LCD_WR_GPIO_Port, LCD_WR_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(LCD_WR_GPIO_Port, LCD_WR_Pin, GPIO_PIN_SET);
    LCDWrite8(Cmd);
    HAL_GPIO_WritePin(LCD_WR_GPIO_Port, LCD_WR_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(LCD_WR_GPIO_Port, LCD_WR_Pin, GPIO_PIN_SET);
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
    while(DummySize--)
    {
      HAL_GPIO_WritePin(LCD_RD_GPIO_Port, LCD_RD_Pin, GPIO_PIN_RESET);
      HAL_GPIO_WritePin(LCD_RD_GPIO_Port, LCD_RD_Pin, GPIO_PIN_SET);
    }
    if(Mode & LCD_IO_DATA24TO16)
      LCDReadMultiData24to16((uint16_t *)pData, Size, Mode);
    else
      LCDReadMultiData8and16(pData, Size, Mode);
    LCDDirWrite();
  }
  #endif /* #if LCD_DATADIR == 1 */
  HAL_GPIO_WritePin(LCD_CS_GPIO_Port, LCD_CS_Pin, GPIO_PIN_SET);
}
