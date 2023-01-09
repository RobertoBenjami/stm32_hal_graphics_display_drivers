/*
 * STMPE811 HAL I2C touch driver
 * author: Roberto Benjami
 * v.2023.01
 *
 */
#include <stdlib.h>
#include <stdio.h>

//-----------------------------------------------------------------------------
#include "main.h"
#include "lcd.h"
#include "ts.h"
#include "ts_stmpe811qtr.h"

//=============================================================================
#ifndef osCMSIS
#define Delay(t)              HAL_Delay(t)
#else
#define Delay(t)              osDelay(t)
#endif

//-----------------------------------------------------------------------------
/* Chip IDs */
#define STMPE811_ID                     0x0811

/* Identification registers & System Control */
#define STMPE811_REG_CHP_ID_LSB         0x00
#define STMPE811_REG_CHP_ID_MSB         0x01
#define STMPE811_REG_ID_VER             0x02

/* Global interrupt Enable bit */
#define STMPE811_GIT_EN                 0x01

/* IO expander functionalities */
#define STMPE811_ADC_FCT                0x01
#define STMPE811_TS_FCT                 0x02
#define STMPE811_IO_FCT                 0x04
#define STMPE811_TEMPSENS_FCT           0x08

/* Global Interrupts definitions */
#define STMPE811_GIT_IO                 0x80  /* IO interrupt                   */
#define STMPE811_GIT_ADC                0x40  /* ADC interrupt                  */
#define STMPE811_GIT_TEMP               0x20  /* Not implemented                */
#define STMPE811_GIT_FE                 0x10  /* FIFO empty interrupt           */
#define STMPE811_GIT_FF                 0x08  /* FIFO full interrupt            */
#define STMPE811_GIT_FOV                0x04  /* FIFO overflowed interrupt      */
#define STMPE811_GIT_FTH                0x02  /* FIFO above threshold interrupt */
#define STMPE811_GIT_TOUCH              0x01  /* Touch is detected interrupt    */
#define STMPE811_ALL_GIT                0x1F  /* All global interrupts          */
#define STMPE811_TS_IT                  (STMPE811_GIT_TOUCH | STMPE811_GIT_FTH |  STMPE811_GIT_FOV | STMPE811_GIT_FF | STMPE811_GIT_FE) /* Touch screen interrupts */

/* General Control Registers */
#define STMPE811_REG_SYS_CTRL1          0x03
#define STMPE811_REG_SYS_CTRL2          0x04
#define STMPE811_REG_SPI_CFG            0x08

/* Interrupt system Registers */
#define STMPE811_REG_INT_CTRL           0x09
#define STMPE811_REG_INT_EN             0x0A
#define STMPE811_REG_INT_STA            0x0B
#define STMPE811_REG_IO_INT_EN          0x0C
#define STMPE811_REG_IO_INT_STA         0x0D

/* IO Registers */
#define STMPE811_REG_IO_SET_PIN         0x10
#define STMPE811_REG_IO_CLR_PIN         0x11
#define STMPE811_REG_IO_MP_STA          0x12
#define STMPE811_REG_IO_DIR             0x13
#define STMPE811_REG_IO_ED              0x14
#define STMPE811_REG_IO_RE              0x15
#define STMPE811_REG_IO_FE              0x16
#define STMPE811_REG_IO_AF              0x17

/* ADC Registers */
#define STMPE811_REG_ADC_INT_EN         0x0E
#define STMPE811_REG_ADC_INT_STA        0x0F
#define STMPE811_REG_ADC_CTRL1          0x20
#define STMPE811_REG_ADC_CTRL2          0x21
#define STMPE811_REG_ADC_CAPT           0x22
#define STMPE811_REG_ADC_DATA_CH0       0x30
#define STMPE811_REG_ADC_DATA_CH1       0x32
#define STMPE811_REG_ADC_DATA_CH2       0x34
#define STMPE811_REG_ADC_DATA_CH3       0x36
#define STMPE811_REG_ADC_DATA_CH4       0x38
#define STMPE811_REG_ADC_DATA_CH5       0x3A
#define STMPE811_REG_ADC_DATA_CH6       0x3B
#define STMPE811_REG_ADC_DATA_CH7       0x3C

/* Touch Screen Registers */
#define STMPE811_REG_TSC_CTRL           0x40
#define STMPE811_REG_TSC_CFG            0x41
#define STMPE811_REG_WDM_TR_X           0x42
#define STMPE811_REG_WDM_TR_Y           0x44
#define STMPE811_REG_WDM_BL_X           0x46
#define STMPE811_REG_WDM_BL_Y           0x48
#define STMPE811_REG_FIFO_TH            0x4A
#define STMPE811_REG_FIFO_STA           0x4B
#define STMPE811_REG_FIFO_SIZE          0x4C
#define STMPE811_REG_TSC_DATA_X         0x4D
#define STMPE811_REG_TSC_DATA_Y         0x4F
#define STMPE811_REG_TSC_DATA_Z         0x51
#define STMPE811_REG_TSC_DATA_XYZ       0x52
#define STMPE811_REG_TSC_FRACT_XYZ      0x56
#define STMPE811_REG_TSC_DATA_INC       0x57
#define STMPE811_REG_TSC_DATA_NON_INC   0xD7
#define STMPE811_REG_TSC_I_DRIVE        0x58
#define STMPE811_REG_TSC_SHIELD         0x59

/* Touch Screen Pins definition */
#define STMPE811_TOUCH_YD               STMPE811_PIN_7
#define STMPE811_TOUCH_XD               STMPE811_PIN_6
#define STMPE811_TOUCH_YU               STMPE811_PIN_5
#define STMPE811_TOUCH_XU               STMPE811_PIN_4
#define STMPE811_TOUCH_IO_ALL           (uint32_t)(STMPE811_TOUCH_YD | STMPE811_TOUCH_XD | STMPE811_TOUCH_YU | STMPE811_TOUCH_XU)

/* IO Pins definition */
#define STMPE811_PIN_0                  0x01
#define STMPE811_PIN_1                  0x02
#define STMPE811_PIN_2                  0x04
#define STMPE811_PIN_3                  0x08
#define STMPE811_PIN_4                  0x10
#define STMPE811_PIN_5                  0x20
#define STMPE811_PIN_6                  0x40
#define STMPE811_PIN_7                  0x80
#define STMPE811_PIN_ALL                0xFF

/* IO Pins directions */
#define STMPE811_DIRECTION_IN           0x00
#define STMPE811_DIRECTION_OUT          0x01

/* IO IT types */
#define STMPE811_TYPE_LEVEL             0x00
#define STMPE811_TYPE_EDGE              0x02

/* IO IT polarity */
#define STMPE811_POLARITY_LOW           0x00
#define STMPE811_POLARITY_HIGH          0x04

/* IO Pin IT edge modes */
#define STMPE811_EDGE_FALLING           0x01
#define STMPE811_EDGE_RISING            0x02

/* TS registers masks */
#define STMPE811_TS_CTRL_ENABLE         0x01
#define STMPE811_TS_CTRL_STATUS         0x80

//=============================================================================
/* TS chip select pin set */
void    stmpe811_ts_Init(uint16_t DeviceAddr);
uint8_t stmpe811_ts_DetectTouch(uint16_t DeviceAddr);
void    stmpe811_ts_GetXY(uint16_t DeviceAddr, uint16_t *X, uint16_t *Y);

//=============================================================================
extern  I2C_HandleTypeDef               TS_I2C_HANDLE;

TS_DrvTypeDef   stmpe811_ts_drv =
{
  stmpe811_ts_Init,
  0,
  0,
  0,
  stmpe811_ts_DetectTouch,
  stmpe811_ts_GetXY,
  0,
  0,
  0,
  0
};

TS_DrvTypeDef  *ts_drv = &stmpe811_ts_drv;
uint8_t stmpe811_inited = 0;

//=============================================================================
void stmpe811_ts_Init(uint16_t DeviceAddr)
{
  uint8_t dt8, mode, idl, idh;

  HAL_I2C_Mem_Write(&TS_I2C_HANDLE, TS_I2C_ADDRESS, (uint16_t)STMPE811_REG_SYS_CTRL1, I2C_MEMADD_SIZE_8BIT, (uint8_t *)"\x02", 1, TS_I2C_TIMEOUT);
  Delay(10);
  HAL_I2C_Mem_Write(&TS_I2C_HANDLE, TS_I2C_ADDRESS, (uint16_t)STMPE811_REG_SYS_CTRL1, I2C_MEMADD_SIZE_8BIT, (uint8_t *)"\x00", 1, TS_I2C_TIMEOUT);
  Delay(2);

  HAL_I2C_Mem_Read(&TS_I2C_HANDLE, TS_I2C_ADDRESS, STMPE811_REG_SYS_CTRL2, I2C_MEMADD_SIZE_8BIT, &mode, 1, TS_I2C_TIMEOUT);
  mode &= ~(STMPE811_IO_FCT);
  HAL_I2C_Mem_Write(&TS_I2C_HANDLE, TS_I2C_ADDRESS, (uint16_t)STMPE811_REG_SYS_CTRL2, I2C_MEMADD_SIZE_8BIT, &mode, 1, TS_I2C_TIMEOUT);

  HAL_I2C_Mem_Read(&TS_I2C_HANDLE, TS_I2C_ADDRESS, STMPE811_REG_IO_AF, I2C_MEMADD_SIZE_8BIT, &dt8, 1, TS_I2C_TIMEOUT);
  dt8 &= ~(uint8_t)STMPE811_TOUCH_IO_ALL;
  HAL_I2C_Mem_Write(&TS_I2C_HANDLE, TS_I2C_ADDRESS, (uint16_t)STMPE811_REG_IO_AF, I2C_MEMADD_SIZE_8BIT, &dt8, 1, TS_I2C_TIMEOUT);

  mode &= ~(STMPE811_TS_FCT | STMPE811_ADC_FCT);
  HAL_I2C_Mem_Write(&TS_I2C_HANDLE, TS_I2C_ADDRESS, (uint16_t)STMPE811_REG_SYS_CTRL2, I2C_MEMADD_SIZE_8BIT, &mode, 1, TS_I2C_TIMEOUT);
  HAL_I2C_Mem_Write(&TS_I2C_HANDLE, TS_I2C_ADDRESS, (uint16_t)STMPE811_REG_ADC_CTRL1, I2C_MEMADD_SIZE_8BIT, (uint8_t *)"\x49", 1, TS_I2C_TIMEOUT);
  Delay(2);
  HAL_I2C_Mem_Write(&TS_I2C_HANDLE, TS_I2C_ADDRESS, (uint16_t)STMPE811_REG_ADC_CTRL2, I2C_MEMADD_SIZE_8BIT, (uint8_t *)"\x01", 1, TS_I2C_TIMEOUT);
  HAL_I2C_Mem_Write(&TS_I2C_HANDLE, TS_I2C_ADDRESS, (uint16_t)STMPE811_REG_TSC_CFG, I2C_MEMADD_SIZE_8BIT, (uint8_t *)"\x9A", 1, TS_I2C_TIMEOUT);
  HAL_I2C_Mem_Write(&TS_I2C_HANDLE, TS_I2C_ADDRESS, (uint16_t)STMPE811_REG_FIFO_TH, I2C_MEMADD_SIZE_8BIT, (uint8_t *)"\x01", 1, TS_I2C_TIMEOUT);
  HAL_I2C_Mem_Write(&TS_I2C_HANDLE, TS_I2C_ADDRESS, (uint16_t)STMPE811_REG_FIFO_STA, I2C_MEMADD_SIZE_8BIT, (uint8_t *)"\x01", 1, TS_I2C_TIMEOUT);
  HAL_I2C_Mem_Write(&TS_I2C_HANDLE, TS_I2C_ADDRESS, (uint16_t)STMPE811_REG_FIFO_STA, I2C_MEMADD_SIZE_8BIT, (uint8_t *)"\x00", 1, TS_I2C_TIMEOUT);
  HAL_I2C_Mem_Write(&TS_I2C_HANDLE, TS_I2C_ADDRESS, (uint16_t)STMPE811_REG_TSC_FRACT_XYZ, I2C_MEMADD_SIZE_8BIT, (uint8_t *)"\x01", 1, TS_I2C_TIMEOUT);
  HAL_I2C_Mem_Write(&TS_I2C_HANDLE, TS_I2C_ADDRESS, (uint16_t)STMPE811_REG_TSC_I_DRIVE, I2C_MEMADD_SIZE_8BIT, (uint8_t *)"\x01", 1, TS_I2C_TIMEOUT);
  HAL_I2C_Mem_Write(&TS_I2C_HANDLE, TS_I2C_ADDRESS, (uint16_t)STMPE811_REG_TSC_CTRL, I2C_MEMADD_SIZE_8BIT, (uint8_t *)"\x01", 1, TS_I2C_TIMEOUT);
  HAL_I2C_Mem_Write(&TS_I2C_HANDLE, TS_I2C_ADDRESS, (uint16_t)STMPE811_REG_INT_STA, I2C_MEMADD_SIZE_8BIT, (uint8_t *)"\xFF", 1, TS_I2C_TIMEOUT);
  Delay(2);

  HAL_I2C_Mem_Read(&TS_I2C_HANDLE, TS_I2C_ADDRESS, STMPE811_REG_CHP_ID_LSB, I2C_MEMADD_SIZE_8BIT, &idl, 1, TS_I2C_TIMEOUT);
  HAL_I2C_Mem_Read(&TS_I2C_HANDLE, TS_I2C_ADDRESS, STMPE811_REG_CHP_ID_MSB, I2C_MEMADD_SIZE_8BIT, &idh, 1, TS_I2C_TIMEOUT);
  if(((idl << 8) | idh) == STMPE811_ID)
    stmpe811_inited = 1;
}

//-----------------------------------------------------------------------------
uint8_t stmpe811_ts_DetectTouch(uint16_t DeviceAddr)
{
  uint8_t state, dt8, ret = 0;

  if(!stmpe811_inited)
    return(0);

  HAL_I2C_Mem_Read(&TS_I2C_HANDLE, TS_I2C_ADDRESS, STMPE811_REG_TSC_CTRL, I2C_MEMADD_SIZE_8BIT, &dt8, 1, TS_I2C_TIMEOUT);
  state = ((dt8 & (uint8_t)STMPE811_TS_CTRL_STATUS) == (uint8_t)0x80);
  if(state > 0)
  {
    HAL_I2C_Mem_Read(&TS_I2C_HANDLE, TS_I2C_ADDRESS, STMPE811_REG_FIFO_SIZE, I2C_MEMADD_SIZE_8BIT, &dt8, 1, TS_I2C_TIMEOUT);
    if(dt8 > 0)
    {
      ret = 1;
    }
  }
  else
  {
    HAL_I2C_Mem_Write(&TS_I2C_HANDLE, TS_I2C_ADDRESS, (uint16_t)STMPE811_REG_FIFO_STA, I2C_MEMADD_SIZE_8BIT, (uint8_t *)"\x01", 1, TS_I2C_TIMEOUT);
    HAL_I2C_Mem_Write(&TS_I2C_HANDLE, TS_I2C_ADDRESS, (uint16_t)STMPE811_REG_FIFO_STA, I2C_MEMADD_SIZE_8BIT, (uint8_t *)"\x00", 1, TS_I2C_TIMEOUT);
  }
  return ret;
}

//-----------------------------------------------------------------------------
void stmpe811_ts_GetXY(uint16_t DeviceAddr, uint16_t *X, uint16_t *Y)
{
  uint8_t  dataXYZ[4];
  uint32_t uldataXYZ;

  HAL_I2C_Mem_Read(&TS_I2C_HANDLE, TS_I2C_ADDRESS, STMPE811_REG_TSC_DATA_NON_INC, I2C_MEMADD_SIZE_8BIT, dataXYZ, sizeof(dataXYZ), TS_I2C_TIMEOUT);
  uldataXYZ = (dataXYZ[0] << 24)|(dataXYZ[1] << 16)|(dataXYZ[2] << 8)|(dataXYZ[3] << 0);
  *X = (uldataXYZ >> 20) & 0x00000FFF;
  *Y = (uldataXYZ >>  8) & 0x00000FFF;

  HAL_I2C_Mem_Write(&TS_I2C_HANDLE, TS_I2C_ADDRESS, (uint16_t)STMPE811_REG_FIFO_STA, I2C_MEMADD_SIZE_8BIT, (uint8_t *)"\x01", 1, TS_I2C_TIMEOUT);
  HAL_I2C_Mem_Write(&TS_I2C_HANDLE, TS_I2C_ADDRESS, (uint16_t)STMPE811_REG_FIFO_STA, I2C_MEMADD_SIZE_8BIT, (uint8_t *)"\x00", 1, TS_I2C_TIMEOUT);
}
