/*
 * lcd_io.h
 *
 *  Created on: 2022.11
 *      Author: Benjami
 */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __LCD_IO_H
#define __LCD_IO_H

#ifdef __cplusplus
extern "C" {
#endif

#include "lcd.h"

//=============================================================================
/* Interface section */

  /* Mode bits */
#define  LCD_IO_CMD8       (1 << 0)
#define  LCD_IO_CMD16      (1 << 1)

#define  LCD_IO_WRITE      (1 << 2)
#define  LCD_IO_READ       (1 << 3)

#define  LCD_IO_DATA8      (1 << 4)
#define  LCD_IO_DATA16     (1 << 5)
#define  LCD_IO_DATA24TO16 (1 << 6)  /* at read */
#define  LCD_IO_DATA16TO24 (1 << 6)  /* at write */

#define  LCD_IO_MULTIDATA  (1 << 7)
#define  LCD_IO_FILL       (1 << 8)

#define  LCD_IO_REVERSE16  (1 << 9)
/* LCD_IO_REVERSE16 only FMC/FSMC 8 bit IO interface
   note: DMA on the 8-bit FMC interface is only possible in low byte->high byte order.
         The displays, on the other hand, request it in order high byte->low byte.
         If we still want to use DMA, the color codes must be stored in reverse byte order in the bitmap memory.
         With this switch, we can signal LCD_IO to do a color code transaction in reverse order,
         so it can also use DMA for bitmap drawing. */

/* Link function for LCD peripheral */
void     LCD_Delay (uint32_t delay);
void     LCD_IO_Init(void);
void     LCD_IO_Bl_OnOff(uint8_t Bl);
void     LCD_IO_Transaction(uint16_t Cmd, uint8_t *pData, uint32_t Size, uint32_t DummySize, uint32_t Mode);

/* 8 bit write commands */
#define  LCD_IO_WriteCmd8DataFill16(Cmd, Data, Size) \
  LCD_IO_Transaction((uint16_t)Cmd, (uint8_t *)&Data, Size, 0, LCD_IO_CMD8 | LCD_IO_WRITE | LCD_IO_DATA16 | LCD_IO_FILL)
#define  LCD_IO_WriteCmd8DataFill16to24(Cmd, Data, Size) \
  LCD_IO_Transaction((uint16_t)Cmd, (uint8_t *)&Data, Size, 0, LCD_IO_CMD8 | LCD_IO_WRITE | LCD_IO_DATA16TO24 | LCD_IO_FILL)
#define  LCD_IO_WriteCmd8MultipleData8(Cmd, pData, Size) \
  LCD_IO_Transaction((uint16_t)Cmd, (uint8_t *)pData, Size, 0, LCD_IO_CMD8 | LCD_IO_WRITE | LCD_IO_DATA8 | LCD_IO_MULTIDATA)
#define  LCD_IO_WriteCmd8MultipleData16(Cmd, pData, Size) \
  LCD_IO_Transaction((uint16_t)Cmd, (uint8_t *)pData, Size, 0, LCD_IO_CMD8 | LCD_IO_WRITE | LCD_IO_DATA16 | LCD_IO_MULTIDATA)
#define  LCD_IO_WriteCmd8MultipleData16to24(Cmd, pData, Size) \
  LCD_IO_Transaction((uint16_t)Cmd, (uint8_t *)pData, Size, 0, LCD_IO_CMD8 | LCD_IO_WRITE | LCD_IO_DATA16TO24 | LCD_IO_MULTIDATA)

/* 16 bit write commands */
#define  LCD_IO_WriteCmd16DataFill16(Cmd, Data, Size) \
  LCD_IO_Transaction((uint16_t)Cmd, (uint8_t *)&Data, Size, 0, LCD_IO_CMD16 | LCD_IO_WRITE | LCD_IO_DATA16 | LCD_IO_FILL)
#define  LCD_IO_WriteCmd16DataFill16to24(Cmd, Data, Size) \
  LCD_IO_Transaction((uint16_t)Cmd, (uint8_t *)&Data, Size, 0, LCD_IO_CMD16 | LCD_IO_WRITE | LCD_IO_DATA16TO24 | LCD_IO_FILL)
#define  LCD_IO_WriteCmd16MultipleData8(Cmd, pData, Size) \
  LCD_IO_Transaction((uint16_t)Cmd, (uint8_t *)pData, Size, 0, LCD_IO_CMD16 | LCD_IO_WRITE | LCD_IO_DATA8 | LCD_IO_MULTIDATA)
#define  LCD_IO_WriteCmd16MultipleData16(Cmd, pData, Size) \
  LCD_IO_Transaction((uint16_t)Cmd, (uint8_t *)pData, Size, 0, LCD_IO_CMD16 | LCD_IO_WRITE | LCD_IO_DATA16 | LCD_IO_MULTIDATA)
#define  LCD_IO_WriteCmd16MultipleData16to24(Cmd, pData, Size) \
  LCD_IO_Transaction((uint16_t)Cmd, (uint8_t *)pData, Size, 0, LCD_IO_CMD16 | LCD_IO_WRITE | LCD_IO_DATA16TO24 | LCD_IO_MULTIDATA)

/* 8 bit read commands */
#define  LCD_IO_ReadCmd8MultipleData8(Cmd, pData, Size, DummySize) \
  LCD_IO_Transaction((uint16_t)Cmd, (uint8_t *)pData, Size, DummySize, LCD_IO_CMD8 | LCD_IO_READ | LCD_IO_DATA8 | LCD_IO_MULTIDATA)
#define  LCD_IO_ReadCmd8MultipleData16(Cmd, pData, Size, DummySize) \
  LCD_IO_Transaction((uint16_t)Cmd, (uint8_t *)pData, Size, DummySize, LCD_IO_CMD8 | LCD_IO_READ | LCD_IO_DATA16 | LCD_IO_MULTIDATA)
#define  LCD_IO_ReadCmd8MultipleData24to16(Cmd, pData, Size, DummySize) \
  LCD_IO_Transaction((uint16_t)Cmd, (uint8_t *)pData, Size, DummySize, LCD_IO_CMD8 | LCD_IO_READ | LCD_IO_DATA24TO16 | LCD_IO_MULTIDATA)

/* 16 bit read commands */
#define  LCD_IO_ReadCmd16MultipleData8(Cmd, pData, Size, DummySize) \
  LCD_IO_Transaction((uint16_t)Cmd, (uint8_t *)pData, Size, DummySize, LCD_IO_CMD16 | LCD_IO_READ | LCD_IO_DATA8 | LCD_IO_MULTIDATA)
#define  LCD_IO_ReadCmd16MultipleData16(Cmd, pData, Size, DummySize) \
  LCD_IO_Transaction((uint16_t)Cmd, (uint8_t *)pData, Size, DummySize, LCD_IO_CMD16 | LCD_IO_READ | LCD_IO_DATA16 | LCD_IO_MULTIDATA)
#define  LCD_IO_ReadCmd16MultipleData24to16(Cmd, pData, Size, DummySize) \
  LCD_IO_Transaction((uint16_t)Cmd, (uint8_t *)pData, Size, DummySize, LCD_IO_CMD16 | LCD_IO_READ | LCD_IO_DATA24TO16 | LCD_IO_MULTIDATA)

/* 8 and 16 bit write and read commands with reverse byte order 16 bit data (only 16bitdepth pixel data in FSMC 8 bit io interface) */
/* 8 bit write commands */
#define  LCD_IO_WriteCmd8DataFill16r(Cmd, Data, Size) \
  LCD_IO_Transaction((uint16_t)Cmd, (uint8_t *)&Data, Size, 0, LCD_IO_CMD8 | LCD_IO_WRITE | LCD_IO_DATA16 | LCD_IO_FILL | LCD_IO_REVERSE16)
#define  LCD_IO_WriteCmd8DataFill16to24r(Cmd, Data, Size) \
  LCD_IO_Transaction((uint16_t)Cmd, (uint8_t *)&Data, Size, 0, LCD_IO_CMD8 | LCD_IO_WRITE | LCD_IO_DATA16TO24 | LCD_IO_FILL | LCD_IO_REVERSE16)
#define  LCD_IO_WriteCmd8MultipleData16r(Cmd, pData, Size) \
  LCD_IO_Transaction((uint16_t)Cmd, (uint8_t *)pData, Size, 0, LCD_IO_CMD8 | LCD_IO_WRITE | LCD_IO_DATA16 | LCD_IO_MULTIDATA | LCD_IO_REVERSE16)
#define  LCD_IO_WriteCmd8MultipleData16to24r(Cmd, pData, Size) \
  LCD_IO_Transaction((uint16_t)Cmd, (uint8_t *)pData, Size, 0, LCD_IO_CMD8 | LCD_IO_WRITE | LCD_IO_DATA16TO24 | LCD_IO_MULTIDATA | LCD_IO_REVERSE16)

/* 16 bit write commands */
#define  LCD_IO_WriteCmd16DataFill16r(Cmd, Data, Size) \
  LCD_IO_Transaction((uint16_t)Cmd, (uint8_t *)&Data, Size, 0, LCD_IO_CMD16 | LCD_IO_WRITE | LCD_IO_DATA16 | LCD_IO_FILL | LCD_IO_REVERSE16)
#define  LCD_IO_WriteCmd16MultipleData16r(Cmd, pData, Size) \
  LCD_IO_Transaction((uint16_t)Cmd, (uint8_t *)pData, Size, 0, LCD_IO_CMD16 | LCD_IO_WRITE | LCD_IO_DATA16 | LCD_IO_MULTIDATA | LCD_IO_REVERSE16)

/* 8 bit read commands */
#define  LCD_IO_ReadCmd8MultipleData16r(Cmd, pData, Size, DummySize) \
  LCD_IO_Transaction((uint16_t)Cmd, (uint8_t *)pData, Size, DummySize, LCD_IO_CMD8 | LCD_IO_READ | LCD_IO_DATA16 | LCD_IO_MULTIDATA | LCD_IO_REVERSE16)

/* 16 bit read commands */
#define  LCD_IO_ReadCmd16MultipleData16r(Cmd, pData, Size, DummySize) \
  LCD_IO_Transaction((uint16_t)Cmd, (uint8_t *)pData, Size, DummySize, LCD_IO_CMD16 | LCD_IO_READ | LCD_IO_DATA16 | LCD_IO_MULTIDATA | LCD_IO_REVERSE16)

#ifdef __cplusplus
}
#endif

#endif /* SRC_LCD_LCD_IO_H_ */
