/*
 * XPT2046 HAL touch driver
 * author: Roberto Benjami
 * v.2023.01
 *
 * - software and hardware SPI
 */
#include <stdlib.h>
#include <stdio.h>

//-----------------------------------------------------------------------------
#include "main.h"
#include "lcd.h"
#include "ts.h"
#include "ts_xpt2046.h"

#if defined(STM32H7)
#define  LCD_SPI_SETBAUDRATE(hlcdspi, br)         MODIFY_REG(hlcdspi.Instance->CFG1, SPI_CFG1_MBR, br << SPI_CFG1_MBR_Pos)
#else
#define  LCD_SPI_SETBAUDRATE(hlcdspi, br)         MODIFY_REG(hlcdspi.Instance->CR1, SPI_CR1_BR, br << SPI_CR1_BR_Pos)
#endif

//=============================================================================

/* if not used the TS_IRQ pin -> Z1-Z2 touch sensitivy */
#define TS_ZSENS              128

#define TS_SPI_TIMEOUT        HAL_MAX_DELAY

#define TOUCH_FILTER          16
#define TOUCH_MAXREPEAT       8

#define XPT2046_MODE          0
#define XPT2046_SER           0
#define XPT2046_PD            0
#define XPT2046_CMD_GETTEMP   ((1 << 7) | (0 << 4) | (XPT2046_MODE << 3) | (XPT2046_SER << 2) | XPT2046_PD)
#define XPT2046_CMD_GETTVBAT  ((1 << 7) | (2 << 4) | (XPT2046_MODE << 3) | (XPT2046_SER << 2) | XPT2046_PD)
#define XPT2046_CMD_GETX      ((1 << 7) | (5 << 4) | (XPT2046_MODE << 3) | (XPT2046_SER << 2) | XPT2046_PD)
#define XPT2046_CMD_GETY      ((1 << 7) | (1 << 4) | (XPT2046_MODE << 3) | (XPT2046_SER << 2) | XPT2046_PD)
#define XPT2046_CMD_GETZ1     ((1 << 7) | (3 << 4) | (XPT2046_MODE << 3) | (XPT2046_SER << 2) | XPT2046_PD)
#define XPT2046_CMD_GETZ2     ((1 << 7) | (4 << 4) | (XPT2046_MODE << 3) | (XPT2046_SER << 2) | XPT2046_PD)

#define ABS(N)                (((N)<0) ? (-(N)) : (N))

//=============================================================================

static  uint16_t  tx, ty;

//=============================================================================
/* TS chip select pin set */
void    xpt2046_ts_Init(uint16_t DeviceAddr);
uint8_t xpt2046_ts_DetectTouch(uint16_t DeviceAddr);
void    xpt2046_ts_GetXY(uint16_t DeviceAddr, uint16_t *X, uint16_t *Y);

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

#if !defined(TS_SPI_HANDLE) || TS_SPI_HANDLE == -1

#if TS_SPI_SPD > 0
#define TS_CLK_DELAY TS_IO_Delay(TS_SPI_SPD);
#else
#define TS_CLK_DELAY
#endif

//-----------------------------------------------------------------------------
uint16_t TS_IO_Transaction(uint8_t cmd)
{
  uint16_t ret = 0;
  uint32_t i;
  HAL_GPIO_WritePin(TS_CS_GPIO_Port, TS_CS_Pin, GPIO_PIN_RESET);
  for(i=0; i<8; i++)
  {
    HAL_GPIO_WritePin(TS_MOSI_GPIO_Port, TS_MOSI_Pin, cmd & 0x80);
    TS_CLK_DELAY;
    HAL_GPIO_WritePin(TS_SCK_GPIO_Port, TS_SCK_Pin, GPIO_PIN_SET);
    TS_CLK_DELAY;
    HAL_GPIO_WritePin(TS_SCK_GPIO_Port, TS_SCK_Pin, GPIO_PIN_RESET);
    cmd <<= 1;
  }

  #if XPT2046_READDELAY > 0
  TS_IO_Delay(XPT2046_READDELAY);
  #endif

  HAL_GPIO_WritePin(TS_MOSI_GPIO_Port, TS_MOSI_Pin, GPIO_PIN_RESET);
  for(i=0; i<16; i++)
  {
    ret <<= 1;
    TS_CLK_DELAY;
    HAL_GPIO_WritePin(TS_SCK_GPIO_Port, TS_SCK_Pin, GPIO_PIN_SET);
    if(HAL_GPIO_ReadPin(TS_MISO_GPIO_Port, TS_MISO_Pin))
      ret |= 1;
    TS_CLK_DELAY;
    HAL_GPIO_WritePin(TS_SCK_GPIO_Port, TS_SCK_Pin, GPIO_PIN_RESET);
  }
  HAL_GPIO_WritePin(TS_CS_GPIO_Port, TS_CS_Pin, GPIO_PIN_SET);
  return ((ret & 0x7FFF) >> 3);
}
#else

extern  SPI_HandleTypeDef     TS_SPI_HANDLE;

//-----------------------------------------------------------------------------
uint16_t TS_IO_Transaction(uint8_t cmd)
{
  const uint16_t d = 0;
  uint16_t ret;
  HAL_GPIO_WritePin(TS_CS_GPIO_Port, TS_CS_Pin, GPIO_PIN_RESET);
  HAL_SPI_Transmit(&TS_SPI_HANDLE, (uint8_t *)&cmd, 1, TS_SPI_TIMEOUT);
  #if XPT2046_READDELAY > 0
  TS_IO_Delay(XPT2046_READDELAY);
  #endif
  HAL_SPI_TransmitReceive(&TS_SPI_HANDLE, (uint8_t *)&d, (uint8_t *)&ret, 2, TS_SPI_TIMEOUT);
  HAL_GPIO_WritePin(TS_CS_GPIO_Port, TS_CS_Pin, GPIO_PIN_SET);
  ret = __REVSH(ret);
  return ((ret & 0x7FFF) >> 3);
}
#endif

//-----------------------------------------------------------------------------
/* return:
   - 0 : touchscreen is not pressed
   - 1 : touchscreen is pressed */
uint8_t TS_IO_DetectToch(void)
{
  uint8_t  ret;
  #if defined(TS_IRQ_GPIO_Port) && defined (TS_IRQ_Pin)
  if(HAL_GPIO_ReadPin(TS_IRQ_GPIO_Port, TS_IRQ_Pin))
    ret = 0;
  else
    ret = 1;
  #else
  if((TS_IO_Transaction(XPT2046_CMD_GETZ1) > TS_ZSENS) || (TS_IO_Transaction(XPT2046_CMD_GETZ2) < (4095 - TS_ZSENS)))
    ret = 1;
  else
    ret = 0;
  #endif
  return ret;
}

TS_DrvTypeDef   xpt2046_ts_drv =
{
  xpt2046_ts_Init,
  0,
  0,
  0,
  xpt2046_ts_DetectTouch,
  xpt2046_ts_GetXY,
  0,
  0,
  0,
  0
};

TS_DrvTypeDef  *ts_drv = &xpt2046_ts_drv;


//-----------------------------------------------------------------------------
void xpt2046_ts_Init(uint16_t DeviceAddr)
{
  #if TS_SPI_HANDLE != -1 && defined(TS_SPI_SPD) && TS_SPI_SPD >= 0 && TS_SPI_SPD <= 7
  LCD_SPI_SETBAUDRATE(TS_SPI_HANDLE, TS_SPI_SPD);
  #endif
}

//-----------------------------------------------------------------------------
uint8_t xpt2046_ts_DetectTouch(uint16_t DeviceAddr)
{
  uint8_t ret = 0;
  int32_t x1, x2, y1, y2, i;

  if(TS_IO_DetectToch())
  {
    x1 = TS_IO_Transaction(XPT2046_CMD_GETX); /* Get X */
    y1 = TS_IO_Transaction(XPT2046_CMD_GETY); /* Get Y */
    i = TOUCH_MAXREPEAT;
    while(i--)
    {
      x2 = TS_IO_Transaction(XPT2046_CMD_GETX); /* Get X */
      y2 = TS_IO_Transaction(XPT2046_CMD_GETY); /* Get Y */
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
void xpt2046_ts_GetXY(uint16_t DeviceAddr, uint16_t *X, uint16_t *Y)
{
  *X = tx,
  *Y = ty;
}
