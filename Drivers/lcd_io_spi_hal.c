/*
 * SPI HAL LCD driver STM32H7
 * author: Roberto Benjami
 * v.2022.11
*/

//-----------------------------------------------------------------------------
#include <stdio.h>

#include "main.h"
#include "lcd.h"
#include "lcd_io_spi_hal.h"

#if  LCD_DMA_WAITMODE == 1
#include "cmsis_os.h"
#endif

#if defined(STM32C0)
#include "stm32c0xx_ll_gpio.h"
#define  LCD_SPI_SETDATASIZE_8BIT(hlcdspi)        MODIFY_REG(hlcdspi.Instance->CR2, SPI_CR2_DS, SPI_DATASIZE_8BIT)
#define  LCD_SPI_SETDATASIZE_16BIT(hlcdspi)       MODIFY_REG(hlcdspi.Instance->CR2, SPI_CR2_DS, SPI_DATASIZE_16BIT)
#define  LCD_SPI_SETBAUDRATE(hlcdspi, br)         MODIFY_REG(hlcdspi.Instance->CR1, SPI_CR1_BR, br << SPI_CR1_BR_Pos)
#define  LCD_SPI_RXFIFOCLEAR(hlcdspi, dummy)      while(hlcdspi.Instance->SR & SPI_SR_RXNE) dummy = hlcdspi.Instance->DR
#elif defined(STM32F0)
#include "stm32f0xx_ll_gpio.h"
#define  LCD_SPI_SETDATASIZE_8BIT(hlcdspi)        MODIFY_REG(hlcdspi.Instance->CR2, SPI_CR2_DS, SPI_DATASIZE_8BIT)
#define  LCD_SPI_SETDATASIZE_16BIT(hlcdspi)       MODIFY_REG(hlcdspi.Instance->CR2, SPI_CR2_DS, SPI_DATASIZE_16BIT)
#define  LCD_SPI_SETBAUDRATE(hlcdspi, br)         MODIFY_REG(hlcdspi.Instance->CR1, SPI_CR1_BR, br << SPI_CR1_BR_Pos)
#define  LCD_SPI_RXFIFOCLEAR(hlcdspi, dummy)      while(hlcdspi.Instance->SR & SPI_SR_RXNE) dummy = hlcdspi.Instance->DR
#elif defined(STM32F1)
#include "stm32f1xx_ll_gpio.h"
#define  LCD_SPI_SETDATASIZE_8BIT(hlcdspi)        hlcdspi.Instance->CR1 &= ~SPI_CR1_DFF
#define  LCD_SPI_SETDATASIZE_16BIT(hlcdspi)       hlcdspi.Instance->CR1 |= SPI_CR1_DFF
#define  LCD_SPI_SETBAUDRATE(hlcdspi, br)         MODIFY_REG(hlcdspi.Instance->CR1, SPI_CR1_BR, br << SPI_CR1_BR_Pos)
#define  LCD_SPI_RXFIFOCLEAR(hlcdspi, dummy)      while(hlcdspi.Instance->SR & SPI_SR_RXNE) dummy = hlcdspi.Instance->DR
#elif defined(STM32F2)
#include "stm32f2xx_ll_gpio.h"
#define  LCD_SPI_SETDATASIZE_8BIT(hlcdspi)        hlcdspi.Instance->CR1 &= ~SPI_CR1_DFF
#define  LCD_SPI_SETDATASIZE_16BIT(hlcdspi)       hlcdspi.Instance->CR1 |= SPI_CR1_DFF
#define  LCD_SPI_SETBAUDRATE(hlcdspi, br)         MODIFY_REG(hlcdspi.Instance->CR1, SPI_CR1_BR, br << SPI_CR1_BR_Pos)
#define  LCD_SPI_RXFIFOCLEAR(hlcdspi, dummy)      while(hlcdspi.Instance->SR & SPI_SR_RXNE) dummy = hlcdspi.Instance->DR
#elif defined(STM32F3)
#include "stm32f3xx_ll_gpio.h"
#define  LCD_SPI_SETDATASIZE_8BIT(hlcdspi)        MODIFY_REG(hlcdspi.Instance->CR2, SPI_CR2_DS, SPI_DATASIZE_8BIT)
#define  LCD_SPI_SETDATASIZE_16BIT(hlcdspi)       MODIFY_REG(hlcdspi.Instance->CR2, SPI_CR2_DS, SPI_DATASIZE_16BIT)
#define  LCD_SPI_SETBAUDRATE(hlcdspi, br)         MODIFY_REG(hlcdspi.Instance->CR1, SPI_CR1_BR, br << SPI_CR1_BR_Pos)
#define  LCD_SPI_RXFIFOCLEAR(hlcdspi, dummy)      while(hlcdspi.Instance->SR & SPI_SR_RXNE) dummy = hlcdspi.Instance->DR
#elif defined(STM32F4)
#include "stm32f4xx_ll_gpio.h"
#define  LCD_SPI_SETDATASIZE_8BIT(hlcdspi)        hlcdspi.Instance->CR1 &= ~SPI_CR1_DFF
#define  LCD_SPI_SETDATASIZE_16BIT(hlcdspi)       hlcdspi.Instance->CR1 |= SPI_CR1_DFF
#define  LCD_SPI_SETBAUDRATE(hlcdspi, br)         MODIFY_REG(hlcdspi.Instance->CR1, SPI_CR1_BR, br << SPI_CR1_BR_Pos)
#define  LCD_SPI_RXFIFOCLEAR(hlcdspi, dummy)      while(hlcdspi.Instance->SR & SPI_SR_RXNE) dummy = hlcdspi.Instance->DR
#elif defined(STM32F7)
#include "stm32f7xx_ll_gpio.h"
#define  LCD_SPI_SETDATASIZE_8BIT(hlcdspi)        MODIFY_REG(hlcdspi.Instance->CR2, SPI_CR2_DS, SPI_DATASIZE_8BIT)
#define  LCD_SPI_SETDATASIZE_16BIT(hlcdspi)       MODIFY_REG(hlcdspi.Instance->CR2, SPI_CR2_DS, SPI_DATASIZE_16BIT)
#define  LCD_SPI_SETBAUDRATE(hlcdspi, br)         MODIFY_REG(hlcdspi.Instance->CR1, SPI_CR1_BR, br << SPI_CR1_BR_Pos)
#define  LCD_SPI_RXFIFOCLEAR(hlcdspi, dummy)      while(hlcdspi.Instance->SR & SPI_SR_RXNE) dummy = hlcdspi.Instance->DR
#elif defined(STM32H7)
#include "stm32h7xx_ll_gpio.h"
#define  LCD_SPI_SETDATASIZE_8BIT(hlcdspi)        MODIFY_REG(hlcdspi.Instance->CFG1, SPI_CFG1_DSIZE, SPI_DATASIZE_8BIT)
#define  LCD_SPI_SETDATASIZE_16BIT(hlcdspi)       MODIFY_REG(hlcdspi.Instance->CFG1, SPI_CFG1_DSIZE, SPI_DATASIZE_16BIT)
#define  LCD_SPI_SETBAUDRATE(hlcdspi, br)         MODIFY_REG(hlcdspi.Instance->CFG1, SPI_CFG1_MBR, br << SPI_CFG1_MBR_Pos)
#define  LCD_SPI_RXFIFOCLEAR(hlcdspi, dummy)      while(hlcdspi.Instance->SR & SPI_SR_RXP) dummy = hlcdspi.Instance->RXDR
#elif defined(STM32G0)
#include "stm32g0xx_ll_gpio.h"
#define  LCD_SPI_SETDATASIZE_8BIT(hlcdspi)        MODIFY_REG(hlcdspi.Instance->CR2, SPI_CR2_DS, SPI_DATASIZE_8BIT)
#define  LCD_SPI_SETDATASIZE_16BIT(hlcdspi)       MODIFY_REG(hlcdspi.Instance->CR2, SPI_CR2_DS, SPI_DATASIZE_16BIT)
#define  LCD_SPI_SETBAUDRATE(hlcdspi, br)         MODIFY_REG(hlcdspi.Instance->CR1, SPI_CR1_BR, br << SPI_CR1_BR_Pos)
#define  LCD_SPI_RXFIFOCLEAR(hlcdspi, dummy)      while(hlcdspi.Instance->SR & SPI_SR_RXNE) dummy = hlcdspi.Instance->DR
#elif defined(STM32G4)
#include "stm32g4xx_ll_gpio.h"
#define  LCD_SPI_SETDATASIZE_8BIT(hlcdspi)        MODIFY_REG(hlcdspi.Instance->CR2, SPI_CR2_DS, SPI_DATASIZE_8BIT)
#define  LCD_SPI_SETDATASIZE_16BIT(hlcdspi)       MODIFY_REG(hlcdspi.Instance->CR2, SPI_CR2_DS, SPI_DATASIZE_16BIT)
#define  LCD_SPI_SETBAUDRATE(hlcdspi, br)         MODIFY_REG(hlcdspi.Instance->CR1, SPI_CR1_BR, br << SPI_CR1_BR_Pos)
#define  LCD_SPI_RXFIFOCLEAR(hlcdspi, dummy)      while(hlcdspi.Instance->SR & SPI_SR_RXNE) dummy = hlcdspi.Instance->DR
#elif defined(STM32L0)
#include "stm32l0xx_ll_gpio.h"
#define  LCD_SPI_SETDATASIZE_8BIT(hlcdspi)        hlcdspi.Instance->CR1 &= ~SPI_CR1_DFF
#define  LCD_SPI_SETDATASIZE_16BIT(hlcdspi)       hlcdspi.Instance->CR1 |= SPI_CR1_DFF
#define  LCD_SPI_SETBAUDRATE(hlcdspi, br)         MODIFY_REG(hlcdspi.Instance->CR1, SPI_CR1_BR, br << SPI_CR1_BR_Pos)
#define  LCD_SPI_RXFIFOCLEAR(hlcdspi, dummy)      while(hlcdspi.Instance->SR & SPI_SR_RXNE) dummy = hlcdspi.Instance->DR
#elif defined(STM32L1)
#include "stm32l1xx_ll_gpio.h"
#define  LCD_SPI_SETDATASIZE_8BIT(hlcdspi)        hlcdspi.Instance->CR1 &= ~SPI_CR1_DFF
#define  LCD_SPI_SETDATASIZE_16BIT(hlcdspi)       hlcdspi.Instance->CR1 |= SPI_CR1_DFF
#define  LCD_SPI_SETBAUDRATE(hlcdspi, br)         MODIFY_REG(hlcdspi.Instance->CR1, SPI_CR1_BR, br << SPI_CR1_BR_Pos)
#define  LCD_SPI_RXFIFOCLEAR(hlcdspi, dummy)      while(hlcdspi.Instance->SR & SPI_SR_RXNE) dummy = hlcdspi.Instance->DR
#elif defined(STM32L4)
#include "stm32l4xx_ll_gpio.h"
#define  LCD_SPI_SETDATASIZE_8BIT(hlcdspi)        MODIFY_REG(hlcdspi.Instance->CR2, SPI_CR2_DS, SPI_DATASIZE_8BIT)
#define  LCD_SPI_SETDATASIZE_16BIT(hlcdspi)       MODIFY_REG(hlcdspi.Instance->CR2, SPI_CR2_DS, SPI_DATASIZE_16BIT)
#define  LCD_SPI_SETBAUDRATE(hlcdspi, br)         MODIFY_REG(hlcdspi.Instance->CR1, SPI_CR1_BR, br << SPI_CR1_BR_Pos)
#define  LCD_SPI_RXFIFOCLEAR(hlcdspi, dummy)      while(hlcdspi.Instance->SR & SPI_SR_RXNE) dummy = hlcdspi.Instance->DR
#elif defined(STM32L5)
#include "stm32l5xx_ll_gpio.h"
#define  LCD_SPI_SETDATASIZE_8BIT(hlcdspi)        MODIFY_REG(hlcdspi.Instance->CR2, SPI_CR2_DS, SPI_DATASIZE_8BIT)
#define  LCD_SPI_SETDATASIZE_16BIT(hlcdspi)       MODIFY_REG(hlcdspi.Instance->CR2, SPI_CR2_DS, SPI_DATASIZE_16BIT)
#define  LCD_SPI_SETBAUDRATE(hlcdspi, br)         MODIFY_REG(hlcdspi.Instance->CR1, SPI_CR1_BR, br << SPI_CR1_BR_Pos)
#define  LCD_SPI_RXFIFOCLEAR(hlcdspi, dummy)      while(hlcdspi.Instance->SR & SPI_SR_RXNE) dummy = hlcdspi.Instance->DR
#elif defined(STM32WB)
#include "stm32wbxx_ll_gpio.h"
#define  LCD_SPI_SETDATASIZE_8BIT(hlcdspi)        MODIFY_REG(hlcdspi.Instance->CR2, SPI_CR2_DS, SPI_DATASIZE_8BIT)
#define  LCD_SPI_SETDATASIZE_16BIT(hlcdspi)       MODIFY_REG(hlcdspi.Instance->CR2, SPI_CR2_DS, SPI_DATASIZE_16BIT)
#define  LCD_SPI_SETBAUDRATE(hlcdspi, br)         MODIFY_REG(hlcdspi.Instance->CR1, SPI_CR1_BR, br << SPI_CR1_BR_Pos)
#define  LCD_SPI_RXFIFOCLEAR(hlcdspi, dummy)      while(hlcdspi.Instance->SR & SPI_SR_RXNE) dummy = hlcdspi.Instance->DR
#elif defined(STM32WL)
#include "stm32wlxx_ll_gpio.h"
#define  LCD_SPI_SETDATASIZE_8BIT(hlcdspi)        MODIFY_REG(hlcdspi.Instance->CR2, SPI_CR2_DS, SPI_DATASIZE_8BIT)
#define  LCD_SPI_SETDATASIZE_16BIT(hlcdspi)       MODIFY_REG(hlcdspi.Instance->CR2, SPI_CR2_DS, SPI_DATASIZE_16BIT)
#define  LCD_SPI_SETBAUDRATE(hlcdspi, br)         MODIFY_REG(hlcdspi.Instance->CR1, SPI_CR1_BR, br << SPI_CR1_BR_Pos)
#define  LCD_SPI_RXFIFOCLEAR(hlcdspi, dummy)      while(hlcdspi.Instance->SR & SPI_SR_RXNE) dummy = hlcdspi.Instance->DR
#else
#error unknown processor family
#endif

#define  DMA_MINSIZE       0x0010
#define  DMA_MAXSIZE       0xFFFE

//-----------------------------------------------------------------------------
/* Link function for LCD peripheral */
void  LCD_Delay (uint32_t delay);
void  LCD_IO_Init(void);
void  LCD_IO_Bl_OnOff(uint8_t Bl);
void  LCD_IO_Transaction(uint16_t Cmd, uint16_t *pData, uint32_t Size, uint32_t DummySize, uint32_t Mode);

//=============================================================================
extern  SPI_HandleTypeDef   LCD_SPI_HANDLE;

//-----------------------------------------------------------------------------
#if LCD_SPI_MODE == 0
/* Transmit only mode */

#define  LcdDirRead()
#define  LcdDirWrite()

#elif (LCD_SPI_MODE == 1) || (LCD_SPI_MODE == 2)
/* Half duplex and full duplex mode */

//-----------------------------------------------------------------------------
/* Switch from SPI write mode to SPI read mode, read the dummy bits, modify the SPI speed */
void LcdDirRead(uint32_t DummySize)
{
  uint32_t RxDummy __attribute__((unused));
  __HAL_SPI_DISABLE(&LCD_SPI_HANDLE);   /* stop SPI */
  #if LCD_SPI_MODE == 1
  SPI_1LINE_RX(&LCD_SPI_HANDLE);        /* if half duplex -> change MOSI data direction */
  #endif
  LL_GPIO_SetPinMode(LCD_SCK_GPIO_Port, LCD_SCK_Pin, LL_GPIO_MODE_OUTPUT); /* GPIO mode = output */
  while(DummySize--)
  { /* Dummy pulses */
    HAL_GPIO_WritePin(LCD_SCK_GPIO_Port, LCD_SCK_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(LCD_SCK_GPIO_Port, LCD_SCK_Pin, GPIO_PIN_SET);
  }
  LL_GPIO_SetPinMode(LCD_SCK_GPIO_Port, LCD_SCK_Pin, LL_GPIO_MODE_ALTERNATE); /* GPIO mode = alternative */
  #if defined(LCD_SPI_SPD_WRITE) && defined(LCD_SPI_SPD_READ) && (LCD_SPI_SPD_WRITE != LCD_SPI_SPD_READ)
  LCD_SPI_SETBAUDRATE(LCD_SPI_HANDLE, LCD_SPI_SPD_READ);       /* speed change */
  #endif
  LCD_SPI_RXFIFOCLEAR(LCD_SPI_HANDLE, RxDummy);                /* RX fifo clear */
}

//-----------------------------------------------------------------------------
/* Switch from SPI read mode to SPI write mode, modify the SPI speed */
void LcdDirWrite(void)
{
  __HAL_SPI_DISABLE(&LCD_SPI_HANDLE);   /* stop SPI */
  #if defined(LCD_SPI_SPD_WRITE) && defined(LCD_SPI_SPD_READ) && (LCD_SPI_SPD_WRITE != LCD_SPI_SPD_READ)
  LCD_SPI_SETBAUDRATE(LCD_SPI_HANDLE, LCD_SPI_SPD_WRITE);       /* speed change */
  #endif
}

#endif /* LCD_SPI_MODE */

//-----------------------------------------------------------------------------
static inline void LcdSpiMode8(void)
{
  LCD_SPI_SETDATASIZE_8BIT(LCD_SPI_HANDLE);
  LCD_SPI_HANDLE.Init.DataSize = SPI_DATASIZE_8BIT;
}

static inline void LcdSpiMode16(void)
{
  LCD_SPI_SETDATASIZE_16BIT(LCD_SPI_HANDLE);
  LCD_SPI_HANDLE.Init.DataSize = SPI_DATASIZE_16BIT;
}

//-----------------------------------------------------------------------------
#if LCD_DMA_TX == 0
/* DMA off */

#define LcdSpiTransInit()
#define LcdSpiTransStart()
#define LcdSpiTransEnd()

#elif LCD_DMA_TX == 1
/* DMA on */

struct
{
  volatile uint32_t txsize;    /* DMA transaction data counter */
  volatile uint32_t txptr;     /* data pointer for DMA */
  volatile uint32_t txstatus;  /* 0 = DMA is free, 1=DMA is busy */
  volatile uint16_t txdata;    /* fill operation data for DMA */
}dmastatus;

//-----------------------------------------------------------------------------
#if LCD_DMA_WAITMODE == 0
/* DMA mode on, Freertos off */

#define LcdSpiTransInit()

#if LCD_DMA_ENDWAIT == 0
#define LcdSpiTransStart()  while(dmastatus.txstatus);
#define LcdSpiDmaWaitEnd()
#elif LCD_DMA_ENDWAIT == 1
#define LcdSpiTransStart()  while(dmastatus.txstatus);
#define LcdSpiDmaWaitEnd()  if(dinc) while(dmastatus.txstatus);
#elif LCD_DMA_ENDWAIT == 2
#define LcdSpiTransStart()
#define LcdSpiDmaWaitEnd()  while(dmastatus.txstatus);
#endif

#define LcdSpiTransEnd()

#define LcdSpiDmaTransStart(void)  dmastatus.txstatus = 1

#define LcdSpiDmaTransEnd(void)    dmastatus.txstatus = 0


#elif LCD_DMA_WAITMODE == 1
/* Freertos mode */

#define LCDDMASIGNAL  1

//-----------------------------------------------------------------------------
#if osCMSIS < 0x20000
/* DMA mode on, Freertos 1 */

osThreadId LcdTaskId;
#define LcdSignalWait         osSignalWait(LCDDMASIGNAL, osWaitForever)
#define LcdSignalSet          osSignalSet(LcdTaskId, LCDDMASIGNAL)
//-----------------------------------------------------------------------------
#else
/* DMA mode on, Freertos 2 */

osThreadId_t LcdTaskId;
#define LcdSignalWait         osThreadFlagsWait(LCDDMASIGNAL, osFlagsWaitAny, osWaitForever)
#define LcdSignalSet          osThreadFlagsSet(LcdTaskId, LCDDMASIGNAL)

#endif

//-----------------------------------------------------------------------------
/* DMA mode on, Freertos 1 and 2 */

#if LCD_DMA_ENDWAIT == 0
#define LcdSpiTransInit()     {LcdTaskId = osThreadGetId(); LcdSignalSet;}
#define LcdSpiTransStart()    {LcdSignalWait;}
#define LcdSpiTransEnd()      {LcdSignalSet;}
#define LcdSpiDmaWaitEnd()
#elif LCD_DMA_ENDWAIT == 1
#define LcdSpiTransInit()     {LcdTaskId = osThreadGetId(); LcdSignalSet;}
#define LcdSpiTransStart()    {LcdSignalWait;}
#define LcdSpiTransEnd()      {LcdSignalSet;}
#define LcdSpiDmaWaitEnd()    {if(dinc) {LcdSignalWait; LcdSignalSet;}}
#elif LCD_DMA_ENDWAIT == 2
#define LcdSpiTransInit()     {LcdTaskId = osThreadGetId();}
#define LcdSpiTransStart()
#define LcdSpiTransEnd()
#define LcdSpiDmaWaitEnd()    {LcdSignalWait;}
#endif

#define LcdSpiDmaTransStart() {dmastatus.txstatus = 1;}
#define LcdSpiDmaTransEnd()   {dmastatus.txstatus = 0; LcdSignalSet;}

#endif

//-----------------------------------------------------------------------------
/* DMA operation end callback function prototype */
__weak void LCD_IO_DmaTxCpltCallback(SPI_HandleTypeDef *hspi)
{
  UNUSED(hspi);
}

//-----------------------------------------------------------------------------
/* Get the DMA operation status (0=DMA is free, 1=DMA is busy) */
uint32_t LCD_IO_DmaBusy(void)
{
  return dmastatus.txstatus;
}

//-----------------------------------------------------------------------------
/* SPI DMA operation interrupt */
void HAL_SPI_TxCpltCallback(SPI_HandleTypeDef *hspi)
{
  if(hspi == &LCD_SPI_HANDLE)
  {
    uint32_t Size;
    if(dmastatus.txsize)
    { /* dma operation is still required */
      if(dmastatus.txsize > DMA_MAXSIZE)
      { /* the size is too large, it cannot be done with one dma operation */
        Size = DMA_MAXSIZE;
        dmastatus.txsize -= DMA_MAXSIZE;
      }
      else
      { /* can be done with one dma operation */
        Size = dmastatus.txsize;
        dmastatus.txsize = 0;
      }

      HAL_SPI_Transmit_DMA(&LCD_SPI_HANDLE, (uint8_t *)dmastatus.txptr, Size);

      if(hspi->hdmatx->Init.MemInc == DMA_MINC_ENABLE)
      { /* multidata -> pointer increase */
        if(hspi->hdmatx->Init.MemDataAlignment == DMA_MDATAALIGN_BYTE)
          dmastatus.txptr += Size;      /* 8bit */
        else
          dmastatus.txptr += Size << 1; /* 16bit */
      }

    }
    else
    { /* dma operations have ended */
      HAL_GPIO_WritePin(LCD_CS_GPIO_Port, LCD_CS_Pin, GPIO_PIN_SET);
      LcdSpiDmaTransEnd();
      LCD_IO_DmaTxCpltCallback(hspi);
    }
  }
}
#endif /* #if LCD_DMA_TX == 1 */

//-----------------------------------------------------------------------------
/* Wrtite fill and multi data to Lcd (8 and 16 bit mode)
   - pData: 8 or 16 bits data pointer
   - Size: data number
   - dinc: 0=fill mode, 1=multidata mode
   - bitdepth: 0 = 8bit data, 1 = 16bit data */
void LCDWriteFillMultiData8and16(uint16_t * pData, uint32_t Size, uint32_t dinc, uint32_t bitdepth)
{
  if(bitdepth == 1)
    LcdSpiMode16();
  else
    LcdSpiMode8();

  #if LCD_DMA_TX == 1

  #ifdef LCD_DMA_UNABLE /* definied DMA unable memory area ? */
  if((Size > DMA_MINSIZE) && LCD_SPI_HANDLE.hdmatx && !LCD_DMA_UNABLE((uint32_t)pData))
  #else
  if((Size > DMA_MINSIZE) && LCD_SPI_HANDLE.hdmatx)
  #endif
  { /* DMA mode */
    LcdSpiDmaTransStart();

    if(bitdepth == 0)
    { /* 8bit DMA */
      LCD_SPI_HANDLE.hdmatx->Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
      LCD_SPI_HANDLE.hdmatx->Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    }
    else
    { /* 16bit DMA */
      LCD_SPI_HANDLE.hdmatx->Init.MemDataAlignment = DMA_MDATAALIGN_HALFWORD;
      LCD_SPI_HANDLE.hdmatx->Init.PeriphDataAlignment = DMA_PDATAALIGN_HALFWORD;
    }

    if(Size > DMA_MAXSIZE)
    { /* the transaction cannot be performed with one DMA operation */
      dmastatus.txsize = Size - DMA_MAXSIZE;
      Size = DMA_MAXSIZE;
    }
    else /* the transaction can be performed with one DMA operation */
      dmastatus.txsize = 0;

    if(dinc) /* multidata */
    {
      LCD_SPI_HANDLE.hdmatx->Init.MemInc = DMA_MINC_ENABLE;
      dmastatus.txptr = (uint32_t)pData + (Size << bitdepth);
    }
    else     /* fill */
    {
      LCD_SPI_HANDLE.hdmatx->Init.MemInc = DMA_MINC_DISABLE;
      dmastatus.txdata = *pData;
      dmastatus.txptr = (uint32_t)&dmastatus.txdata;
      pData = (uint16_t *)&dmastatus.txdata;
    }

    HAL_DMA_Init(LCD_SPI_HANDLE.hdmatx);
    HAL_SPI_Transmit_DMA(&LCD_SPI_HANDLE, (uint8_t *)pData, Size);
    LcdSpiDmaWaitEnd();
  }
  else
  #endif
  { /* not DMA mode */
    if(dinc)
    { /* data out */
      while(Size)
      { /* multidata 8bit and 16bit */
        uint16_t Size16;
        if(Size > DMA_MAXSIZE)
        {
          Size16 = DMA_MAXSIZE;
          Size -= DMA_MAXSIZE;
        }
        else
        {
          Size16 = Size;
          Size = 0;
        }
        HAL_SPI_Transmit(&LCD_SPI_HANDLE, (uint8_t *)pData, Size16, 500);
        pData += Size16;
      }
    }
    else /* fill out */
      while(Size--) /* fill 8 and 16bit */
        HAL_SPI_Transmit(&LCD_SPI_HANDLE, (uint8_t *)pData, 1, 5);

    HAL_GPIO_WritePin(LCD_CS_GPIO_Port, LCD_CS_Pin, GPIO_PIN_SET);
    LcdSpiTransEnd();
  }
}

//-----------------------------------------------------------------------------
/* Wrtite fill and multi data to Lcd (convert RGB16 bit (5-6-5) to RGB24 bit (8-8-8) mode, no dma capability)
   - pData: RGB 16 bits data pointer
   - Size: data number
   - dinc: 0=fill mode, 1=multidata mode */
void LCDWriteFillMultiData16to24(uint16_t * pData, uint32_t Size, uint32_t dinc)
{
  uint8_t rgb24[3];
  LcdSpiMode8();

  if(dinc == 0)
  { /* fill 16bit to 24bit */
    rgb24[0] = (*pData & 0xF800) >> 8;
    rgb24[1] = (*pData & 0x07E0) >> 3;
    rgb24[2] = (*pData & 0x001F) << 3;
    #if LCD_DMA_TX == 1
    if(rgb24[0] == rgb24[1] && rgb24[1] == rgb24[2]) /* if R=G=B -> option for DMA use */
    {
      LCDWriteFillMultiData8and16((uint16_t *)rgb24, Size * 3, 0, 0);
      return;
    }
    else
    #endif
    {
      while(Size--)
        HAL_SPI_Transmit(&LCD_SPI_HANDLE, (uint8_t *)rgb24, 3, 5);
    }
  }
  else
    while(Size--)
    { /* multidata 16bit to 24bit */
      rgb24[0] = (*pData & 0xF800) >> 8;
      rgb24[1] = (*pData & 0x07E0) >> 3;
      rgb24[2] = (*pData & 0x001F) << 3;
      HAL_SPI_Transmit(&LCD_SPI_HANDLE, (uint8_t *)rgb24, 3, 5);
      pData++;
    }

  HAL_GPIO_WritePin(LCD_CS_GPIO_Port, LCD_CS_Pin, GPIO_PIN_SET);
  LcdSpiTransEnd();
}

//-----------------------------------------------------------------------------
#if LCD_SPI_MODE != 0

//-----------------------------------------------------------------------------
/* Read data from Lcd
   - pData: 8 or 16 bits data pointer
   - Size: data number
   - bitdepth: 0 = 8bit data, 1 = 16bit data */
void LCDReadMultiData(uint16_t * pData, uint32_t Size, uint32_t bitdepth)
{
  if(!bitdepth)
    LcdSpiMode8();
  else
    LcdSpiMode16();

  while(Size)
  {
    uint32_t Size16;
    if(Size > DMA_MAXSIZE)
    {
      Size16 = DMA_MAXSIZE;
      Size -= DMA_MAXSIZE;
    }
    else
    {
      Size16 = Size;
      Size = 0;
    }
    HAL_SPI_Receive(&LCD_SPI_HANDLE, (uint8_t *)pData, Size16, 500);
    *(uint32_t *)&pData += Size16 << bitdepth;
  }
  LcdDirWrite();
  HAL_GPIO_WritePin(LCD_CS_GPIO_Port, LCD_CS_Pin, GPIO_PIN_SET);
  LcdSpiTransEnd();
}

//-----------------------------------------------------------------------------
/* Read 24bit (8-8-8) RGB data from LCD, and convert to 16bit (5-6-5) RGB data
   - pData: 16 bits RGB data pointer
   - Size: pixel number */
void LCDReadMultiData24to16(uint16_t * pData, uint32_t Size)
{
  uint8_t  rgb888[3];

  LcdSpiMode8();
  while(Size--)
  {
    HAL_SPI_Receive(&LCD_SPI_HANDLE, rgb888, 3, 5);
    *pData = (rgb888[0] & 0XF8) << 8 | (rgb888[1] & 0xFC) << 3 | rgb888[2] >> 3;
    pData++;
  }
  LcdDirWrite();
  HAL_GPIO_WritePin(LCD_CS_GPIO_Port, LCD_CS_Pin, GPIO_PIN_SET);
  LcdSpiTransEnd();
}

#endif /* #if LCD_SPI_MODE != 0 */

//=============================================================================
/* Public functions */

/* n millisec delay */
void LCD_Delay(uint32_t Delay)
{
  #if LCD_DMA_WAITMODE == 0
  HAL_Delay(Delay);
  #elif LCD_DMA_WAITMODE == 1
  osDelay(Delay);
  #endif
}

/* Backlight on-off (Bl=0 -> off, Bl=1 -> on) */
//-----------------------------------------------------------------------------
void LCD_IO_Bl_OnOff(uint8_t Bl)
{
  #if defined(LCD_BL_GPIO_Port) && defined (LCD_BL_Pin)
  if(Bl)
    #if LCD_BLON == 1
    HAL_GPIO_WritePin(LCD_BL_GPIO_Port, LCD_BL_Pin, GPIO_PIN_SET);
    #else
    HAL_GPIO_WritePin(LCD_BL_GPIO_Port, LCD_BL_Pin, GPIO_PIN_RESET);
    #endif
  else
    #if LCD_BLON == 1
    HAL_GPIO_WritePin(LCD_BL_GPIO_Port, LCD_BL_Pin, GPIO_PIN_RESET);
    #else
    HAL_GPIO_WritePin(LCD_BL_GPIO_Port, LCD_BL_Pin, GPIO_PIN_SET);
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
  #if LCD_SPI_MODE != 0
  HAL_GPIO_WritePin(LCD_SCK_GPIO_Port, LCD_SCK_Pin, GPIO_PIN_SET);
  #endif
  HAL_Delay(10);
  #if defined(LCD_SPI_SPD_WRITE)
  LCD_SPI_SETBAUDRATE(LCD_SPI_HANDLE, LCD_SPI_SPD_WRITE);
  #endif
  LcdSpiTransInit();
}

//-----------------------------------------------------------------------------
/* Lcd IO transaction
   - Cmd: 8 or 16 bits command
   - pData: 8 or 16 bits data pointer
   - Size: data number
   - DummySize: dummy byte number at read
   - Mode: 8 or 16 or 24 bit mode, write or read, fill or multidata (see the LCD_IO_... defines) */
void LCD_IO_Transaction(uint16_t Cmd, uint16_t *pData, uint32_t Size, uint32_t DummySize, uint32_t Mode)
{
  #if LCD_SPI_MODE == 0  /* only TX mode */
  if(Mode & LCD_IO_READ)
    return;
  #endif

  LcdSpiTransStart();
  HAL_GPIO_WritePin(LCD_CS_GPIO_Port, LCD_CS_Pin, GPIO_PIN_RESET);

  /* Command write */
  if(Mode & LCD_IO_CMD8)
    LcdSpiMode8();
  else if(Mode & LCD_IO_CMD16)
    LcdSpiMode16();
  HAL_GPIO_WritePin(LCD_RS_GPIO_Port, LCD_RS_Pin, GPIO_PIN_RESET);
  HAL_SPI_Transmit(&LCD_SPI_HANDLE, (uint8_t *)&Cmd, 1, 10); /* CMD write */
  HAL_GPIO_WritePin(LCD_RS_GPIO_Port, LCD_RS_Pin, GPIO_PIN_SET);

  if(!Size)
  { /* only command byte or word */
    HAL_GPIO_WritePin(LCD_CS_GPIO_Port, LCD_CS_Pin, GPIO_PIN_SET);
    LcdSpiTransEnd();
    return;
  }

  /* Datas write or read */
  #if LCD_SPI_MODE != 0
  if(Mode & LCD_IO_READ)
  { /* Read LCD */
    LcdDirRead((DummySize << 3) + LCD_SCK_EXTRACLK);
    if(Mode & LCD_IO_DATA8)
      LCDReadMultiData(pData, Size, 0);
    else if(Mode & LCD_IO_DATA16)
      LCDReadMultiData(pData, Size, 1);
    else if(Mode & LCD_IO_DATA24TO16)
      LCDReadMultiData24to16(pData, Size);
  }
  else
  #endif /* #if LCD_SPI_MODE != 0 */
  { /* Write Lcd */
    uint32_t f = 0;   /* fill */
    if(Mode & LCD_IO_MULTIDATA)
      f = 1;          /* multidata */

    if(Mode & LCD_IO_DATA16TO24)
      LCDWriteFillMultiData16to24(pData, Size, f);
    else
    {
      uint32_t b = 0; /* 8 bit */
      if(Mode & LCD_IO_DATA16)
        b = 1;        /* 16 bit */
      LCDWriteFillMultiData8and16(pData, Size, f, b);
    }
  }
}
