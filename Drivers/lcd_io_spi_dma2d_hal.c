/*
 * SPI HAL LCD driver with DMA2D (stm32f4xx, stm32f7xx, stm32h7xx)
 * author: Roberto Benjami
 * v.2022.11
*/

//-----------------------------------------------------------------------------
#include <stdio.h>

#include "main.h"
#include "lcd.h"
#include "lcd_io_spi_dma2d_hal.h"

#if  LCD_DMA_WAITMODE == 1
#include "cmsis_os.h"
#endif

#if LCD_IO_RGB24_ORDER == 0
#define  RGB565TO888(c16)      ((c16 & 0xF800) << 8) | ((c16 & 0x07E0) << 5) | ((c16 & 0x001F) << 3)
#define  RGB888TO565(c24)      ((c24 & 0XF80000) >> 8 | (c24 & 0xFC00) >> 5 | (c24 & 0xF8 ) >> 3)
#elif LCD_IO_RGB24_ORDER == 1
#define  RGB565TO888(c16)      ((c16 & 0xF800) >> 8) | ((c16 & 0x07E0) << 5) | ((c16 & 0x001F) << 19)
#define  RGB888TO565(c24)      ((c24 & 0XF80000) >> 19 | (c24 & 0xFC00) >> 5 | (c24 & 0xF8 ) << 8)
#endif

#if defined(STM32F4)
#include "stm32f4xx_ll_gpio.h"
#define  LCD_SPI_SETDATASIZE_8BIT(hlcdspi)        hlcdspi.Instance->CR1 &= ~SPI_CR1_DFF
#define  LCD_SPI_SETDATASIZE_16BIT(hlcdspi)       hlcdspi.Instance->CR1 |= SPI_CR1_DFF
#define  LCD_SPI_SETBAUDRATE(hlcdspi, br)         MODIFY_REG(hlcdspi.Instance->CR1, SPI_CR1_BR, br << SPI_CR1_BR_Pos)
#define  LCD_SPI_RXFIFOCLEAR(hlcdspi, dummy)      while(hlcdspi.Instance->SR & SPI_SR_RXNE) dummy = hlcdspi.Instance->DR
#if LCD_IO_RGB24_ORDER == 0
#define  DMA2D_CHECK                              1
#elif LCD_IO_RGB24_ORDER == 1
#define  DMA2D_CHECK                              (!dinc)
#endif
#define  RedBlueOrder(hlcddma2d)

#elif defined(STM32F7)
#include "stm32f7xx_ll_gpio.h"
#define  LCD_SPI_SETDATASIZE_8BIT(hlcdspi)        MODIFY_REG(hlcdspi.Instance->CR2, SPI_CR2_DS, SPI_DATASIZE_8BIT)
#define  LCD_SPI_SETDATASIZE_16BIT(hlcdspi)       MODIFY_REG(hlcdspi.Instance->CR2, SPI_CR2_DS, SPI_DATASIZE_16BIT)
#define  LCD_SPI_SETBAUDRATE(hlcdspi, br)         MODIFY_REG(hlcdspi.Instance->CR1, SPI_CR1_BR, br << SPI_CR1_BR_Pos)
#define  LCD_SPI_RXFIFOCLEAR(hlcdspi, dummy)      while(hlcdspi.Instance->SR & SPI_SR_RXNE) dummy = hlcdspi.Instance->DR
#if LCD_IO_RGB24_ORDER == 0
#define  DMA2D_CHECK                              1
#elif LCD_IO_RGB24_ORDER == 1
#define  DMA2D_CHECK                              (!dinc)
#endif
#define  RedBlueOrder(hlcddma2d)

#elif defined(STM32H7)
#include "stm32h7xx_ll_gpio.h"
#define  LCD_SPI_SETDATASIZE_8BIT(hlcdspi)        MODIFY_REG(hlcdspi.Instance->CFG1, SPI_CFG1_DSIZE, SPI_DATASIZE_8BIT)
#define  LCD_SPI_SETDATASIZE_16BIT(hlcdspi)       MODIFY_REG(hlcdspi.Instance->CFG1, SPI_CFG1_DSIZE, SPI_DATASIZE_16BIT)
#define  LCD_SPI_SETBAUDRATE(hlcdspi, br)         MODIFY_REG(hlcdspi.Instance->CFG1, SPI_CFG1_MBR, br << SPI_CFG1_MBR_Pos)
#define  LCD_SPI_RXFIFOCLEAR(hlcdspi, dummy)      while(hlcdspi.Instance->SR & SPI_SR_RXP) dummy = hlcdspi.Instance->RXDR
#define  DMA2D_CHECK                              1
#if LCD_IO_RGB24_ORDER == 0
#define  RedBlueOrder(hlcddma2d)                  hlcddma2d.Init.RedBlueSwap = DMA2D_RB_REGULAR
#elif LCD_IO_RGB24_ORDER == 1
#define  RedBlueOrder(hlcddma2d)                  hlcddma2d.Init.RedBlueSwap = DMA2D_RB_SWAP
#endif

#else
#error unknown processor family
#endif

#define  DMA_MINSIZE       0x0010
#define  DMA_MAXSIZE       0xFFFE

#define  LCD_SPI_TIMEOUT   10

//-----------------------------------------------------------------------------
/* Link function for LCD peripheral */
void  LCD_Delay (uint32_t delay);
void  LCD_IO_Init(void);
void  LCD_IO_Bl_OnOff(uint8_t Bl);
void  LCD_IO_Transaction(uint16_t Cmd, uint16_t *pData, uint32_t Size, uint32_t DummySize, uint32_t Mode);

//=============================================================================
extern  SPI_HandleTypeDef   LCD_SPI_HANDLE;
extern  DMA2D_HandleTypeDef LCD_DMA2D_HANDLE;

#define DMA_STATUS_FREE       0
#define DMA_STATUS_FILL       (1 << 0)
#define DMA_STATUS_MULTIDATA  (1 << 1)
#define DMA_STATUS_8BIT       (1 << 2)
#define DMA_STATUS_16BIT      (1 << 3)
#define DMA_STATUS_24BIT      (1 << 4)

//-----------------------------------------------------------------------------
#ifndef LCD_DMA_UNABLE /* definied DMA unable memory area ? */
#define LCD_DMA_UNABLE(addr)  0
#endif

struct
{
  uint32_t status;             /* DMA status (0=free, other: see the DMA_STATUS... macros)  */
  uint32_t size;               /* all transactions data size */
  uint32_t trsize;             /* actual DMA transaction data size */
  uint32_t maxtrsize;          /* max size / one DMA transaction */
  uint32_t ptr;                /* data pointer for DMA */
  uint16_t data;               /* fill operation data for DMA */
}volatile dmastatus;

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
uint8_t lcd_dma2d_buffer[LCD_DMA2D_BUFFERSIZE * 3];

//-----------------------------------------------------------------------------
#if LCD_DMA_WAITMODE == 0
/* Freertos off */

#define LcdSpiTransInit()   {LCD_DMA2D_HANDLE.XferCpltCallback  = Dma2dCpltCallback; RedBlueOrder(LCD_DMA2D_HANDLE);}

#if LCD_DMA_ENDWAIT == 0
#define LcdSpiTransStart()  {while(dmastatus.status != DMA_STATUS_FREE);}
#define LcdSpiDmaWaitEnd()
#elif LCD_DMA_ENDWAIT == 1
#define LcdSpiTransStart()  {while(dmastatus.status != DMA_STATUS_FREE);}
#define LcdSpiDmaWaitEnd()  {if(dinc) while(dmastatus.status != DMA_STATUS_FREE);}
#elif LCD_DMA_ENDWAIT == 2
#define LcdSpiTransStart()
#define LcdSpiDmaWaitEnd()  {while(dmastatus.status != DMA_STATUS_FREE);}
#endif

#define LcdSpiTransEnd()

#define LcdSpiDmaTransEnd()       {dmastatus.status = DMA_STATUS_FREE;}


#elif LCD_DMA_WAITMODE == 1
/* Freertos mode */

#define LCDDMASIGNAL  1

//-----------------------------------------------------------------------------
#if osCMSIS < 0x20000
/* Freertos 1 */

osThreadId LcdTaskId;
#define LcdSignalWait         osSignalWait(LCDDMASIGNAL, osWaitForever)
#define LcdSignalSet          osSignalSet(LcdTaskId, LCDDMASIGNAL)
//-----------------------------------------------------------------------------
#else
/* Freertos 2 */

osThreadId_t LcdTaskId;
#define LcdSignalWait         osThreadFlagsWait(LCDDMASIGNAL, osFlagsWaitAny, osWaitForever)
#define LcdSignalSet          osThreadFlagsSet(LcdTaskId, LCDDMASIGNAL)

#endif  /* #else osCMSIS < 0x20000 */

//-----------------------------------------------------------------------------
/* Freertos 1 and 2 */

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

#define LcdSpiDmaTransEnd()   {dmastatus.status = DMA_STATUS_FREE; LcdSignalSet;}

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
  uint32_t ret = 0;
  if(dmastatus.status != DMA_STATUS_FREE)
    ret = 1;
  return ret;
}

//-----------------------------------------------------------------------------
/* SPI DMA operation completed interrupt */
void HAL_SPI_TxCpltCallback(SPI_HandleTypeDef *hspi)
{
  if(hspi == &LCD_SPI_HANDLE)
  {
    if(dmastatus.size > dmastatus.trsize)
    { /* dma operation is still required */

      if(dmastatus.status == (DMA_STATUS_MULTIDATA | DMA_STATUS_8BIT))
        dmastatus.ptr += dmastatus.trsize;        /* 8bit multidata */
      if(dmastatus.status == (DMA_STATUS_MULTIDATA | DMA_STATUS_16BIT))
          dmastatus.ptr += dmastatus.trsize << 1; /* 16bit multidata */
      if(dmastatus.status == (DMA_STATUS_MULTIDATA | DMA_STATUS_24BIT))
          dmastatus.ptr += dmastatus.trsize << 1; /* 24bit multidata */

      dmastatus.size -= dmastatus.trsize;
      if(dmastatus.size <= dmastatus.maxtrsize)
        dmastatus.trsize = dmastatus.size;

      if(dmastatus.status == (DMA_STATUS_MULTIDATA | DMA_STATUS_24BIT))
        HAL_DMA2D_Start_IT(&LCD_DMA2D_HANDLE, dmastatus.ptr, (uint32_t)lcd_dma2d_buffer, dmastatus.trsize, 1);
      else if(dmastatus.status == (DMA_STATUS_FILL | DMA_STATUS_24BIT))
        HAL_SPI_Transmit_DMA(&LCD_SPI_HANDLE, (uint8_t *)dmastatus.ptr, dmastatus.trsize * 3);
      else
        HAL_SPI_Transmit_DMA(&LCD_SPI_HANDLE, (uint8_t *)dmastatus.ptr, dmastatus.trsize);
    }
    else
    { /* dma operations have ended */
      HAL_GPIO_WritePin(LCD_CS_GPIO_Port, LCD_CS_Pin, GPIO_PIN_SET);
      LcdSpiDmaTransEnd();
      LCD_IO_DmaTxCpltCallback(hspi);
    }
  }
}

//-----------------------------------------------------------------------------
/* DMA2D operation completed interrupt */
void Dma2dCpltCallback(DMA2D_HandleTypeDef *hdma2d)
{

  HAL_SPI_Transmit_DMA(&LCD_SPI_HANDLE, (uint8_t *)lcd_dma2d_buffer, dmastatus.trsize * 3);
}

//-----------------------------------------------------------------------------
/* Wrtite fill and multi data to Lcd (8 and 16 bit mode)
   - pData: 8 or 16 bits data pointer
   - Size: data number
   - dinc: 0=fill mode, 1=multidata mode
   - bitdepth: 0 = 8bit data, 1 = 16bit data */
void LCDWriteFillMultiData8and16(uint16_t * pData, uint32_t Size, uint32_t dinc, uint32_t bitdepth)
{
  if(bitdepth == 0)
    LcdSpiMode8();
  else
    LcdSpiMode16();

  if((Size > DMA_MINSIZE) && (!LCD_DMA_UNABLE((uint32_t)pData)))
  { /* DMA mode */
    if(bitdepth == 0)
    { /* 8bit DMA */
      LCD_SPI_HANDLE.hdmatx->Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
      LCD_SPI_HANDLE.hdmatx->Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
      dmastatus.status = DMA_STATUS_8BIT;
    }
    else
    { /* 16bit DMA */
      LCD_SPI_HANDLE.hdmatx->Init.PeriphDataAlignment = DMA_PDATAALIGN_HALFWORD;
      LCD_SPI_HANDLE.hdmatx->Init.MemDataAlignment = DMA_MDATAALIGN_HALFWORD;
      dmastatus.status = DMA_STATUS_16BIT;
    }

    if(!dinc)
    { /* fill */
      LCD_SPI_HANDLE.hdmatx->Init.MemInc = DMA_MINC_DISABLE;
      dmastatus.status |= DMA_STATUS_FILL;
      dmastatus.data = *pData;
      dmastatus.ptr = (uint32_t)&dmastatus.data;
    }
    else
    { /* multidata */
      LCD_SPI_HANDLE.hdmatx->Init.MemInc = DMA_MINC_ENABLE;
      dmastatus.status |= DMA_STATUS_MULTIDATA;
      dmastatus.ptr = (uint32_t)pData;
    }

    dmastatus.size = Size;
    dmastatus.maxtrsize = DMA_MAXSIZE;

    if(Size > DMA_MAXSIZE)
      dmastatus.trsize = DMA_MAXSIZE;
    else /* the transaction can be performed with one DMA operation */
      dmastatus.trsize = Size;

    HAL_DMA_Init(LCD_SPI_HANDLE.hdmatx);
    HAL_SPI_Transmit_DMA(&LCD_SPI_HANDLE, (uint8_t *)dmastatus.ptr, dmastatus.trsize);
    LcdSpiDmaWaitEnd();
  }
  else
  { /* not DMA mode */
    if(dinc == 0)
    { /* fill */
      while(Size--) /* fill 8 and 16bit */
        HAL_SPI_Transmit(&LCD_SPI_HANDLE, (uint8_t *)pData, 1, LCD_SPI_TIMEOUT);
    }
    else
    { /* multidata */
      while(Size)
      {
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
        HAL_SPI_Transmit(&LCD_SPI_HANDLE, (uint8_t *)pData, Size16, LCD_SPI_TIMEOUT);
        pData += Size16;
      }
    }

    /* transaction end */
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
  static uint32_t rgb888;
  LcdSpiMode8();

  if((Size > DMA_MINSIZE) && (!LCD_DMA_UNABLE((uint32_t)pData)) && DMA2D_CHECK)
  {
    uint32_t dma2d_ptr;

    /* SPI TX DMA setting (8bit, multidata) */
    LCD_SPI_HANDLE.hdmatx->Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    LCD_SPI_HANDLE.hdmatx->Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
    LCD_SPI_HANDLE.hdmatx->Init.MemInc = DMA_MINC_ENABLE;
    HAL_DMA_Init(LCD_SPI_HANDLE.hdmatx);

    if(dinc == 0)
    { /* fill 16bit to 24bit */
      dmastatus.status = DMA_STATUS_FILL | DMA_STATUS_24BIT;
      dmastatus.ptr = (uint32_t)lcd_dma2d_buffer; /* 24bit color bitmap address */
      dma2d_ptr = RGB565TO888(*pData);            /* 24bit color code */
      LCD_DMA2D_HANDLE.Init.Mode = DMA2D_R2M;     /* register mode */
    }
    else
    { /* multidata 16bit to 24bit */
      dmastatus.status = DMA_STATUS_MULTIDATA | DMA_STATUS_24BIT;
      dmastatus.ptr = (uint32_t)pData;            /* 16bit color bitmap address */
      dma2d_ptr = (uint32_t)pData;                /* 16bit color bitmap address */
      LCD_DMA2D_HANDLE.Init.Mode = DMA2D_M2M_PFC; /* mem to mem with pixel format conversion mode */
    }
    dmastatus.maxtrsize = LCD_DMA2D_BUFFERSIZE;
    dmastatus.size = Size;

    if(Size > LCD_DMA2D_BUFFERSIZE)
      dmastatus.trsize = LCD_DMA2D_BUFFERSIZE;
    else
      dmastatus.trsize = Size;

    /* DMA2D setting */
    HAL_DMA2D_Init(&LCD_DMA2D_HANDLE);
    HAL_DMA2D_Start_IT(&LCD_DMA2D_HANDLE, dma2d_ptr, (uint32_t)lcd_dma2d_buffer, dmastatus.trsize, 1);
    LcdSpiDmaWaitEnd();
    return;
  }
  else
  {
    if(dinc == 0)
    { /* fill 16bit to 24bit */
      rgb888 = RGB565TO888(*pData);
      while(Size--)
        HAL_SPI_Transmit(&LCD_SPI_HANDLE, (uint8_t *)&rgb888, 3, LCD_SPI_TIMEOUT);
    }
    else
    { /* multidata 16bit to 24bit */
      while(Size--)
      {
        rgb888 = RGB565TO888(*pData);
        HAL_SPI_Transmit(&LCD_SPI_HANDLE, (uint8_t *)&rgb888, 3, LCD_SPI_TIMEOUT);
        pData++;
      }
    }
    HAL_GPIO_WritePin(LCD_CS_GPIO_Port, LCD_CS_Pin, GPIO_PIN_SET);
    LcdSpiTransEnd();
  }
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
    HAL_SPI_Receive(&LCD_SPI_HANDLE, (uint8_t *)pData, Size16, LCD_SPI_TIMEOUT);
    *(uint8_t *)&pData += Size16 << bitdepth;
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
  uint32_t  rgb888;

  LcdSpiMode8();
  while(Size--)
  {
    HAL_SPI_Receive(&LCD_SPI_HANDLE, (uint8_t *)&rgb888, 3, LCD_SPI_TIMEOUT);
    *pData = RGB888TO565(rgb888);
    pData++;
  }
  HAL_GPIO_WritePin(LCD_CS_GPIO_Port, LCD_CS_Pin, GPIO_PIN_SET);
  LcdDirWrite();
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
  LCD_DMA2D_HANDLE.XferCpltCallback  = Dma2dCpltCallback;
  LCD_DMA2D_HANDLE.LayerCfg[1].InputColorMode = DMA2D_INPUT_RGB565;
  LCD_DMA2D_HANDLE.Init.ColorMode = DMA2D_OUTPUT_RGB888;
  RedBlueOrder(LCD_DMA2D_HANDLE);
  LcdSpiTransInit();
}

//-----------------------------------------------------------------------------
/* Lcd IO transaction
   - Cmd: 8 or 16 bits command
   - pData: 8 or 16 bits data pointer
   - Size: data number
   - DummySize: dummy byte number at read
   - Mode: 8 or 16 or 24 bit mode, write or read, fill or multidata (see the LCD_IO_... defines in "lcd.h") */
void LCD_IO_Transaction(uint16_t Cmd, uint16_t *pData, uint32_t Size, uint32_t DummySize, uint32_t Mode)
{
  #if LCD_SPI_MODE == 0  /* only TX mode */
  if(Mode & LCD_IO_READ)
    return;
  #endif

  LcdSpiTransStart();

  if(dmastatus.status)
    while(1);
  HAL_GPIO_WritePin(LCD_CS_GPIO_Port, LCD_CS_Pin, GPIO_PIN_RESET);

  /* Command write */
  if(Mode & LCD_IO_CMD8)
    LcdSpiMode8();
  else if(Mode & LCD_IO_CMD16)
    LcdSpiMode16();
  HAL_GPIO_WritePin(LCD_RS_GPIO_Port, LCD_RS_Pin, GPIO_PIN_RESET);
  HAL_SPI_Transmit(&LCD_SPI_HANDLE, (uint8_t *)&Cmd, 1, LCD_SPI_TIMEOUT); /* CMD write */
  HAL_GPIO_WritePin(LCD_RS_GPIO_Port, LCD_RS_Pin, GPIO_PIN_SET);

  if(!Size)
  { /* only command byte or word */
    HAL_GPIO_WritePin(LCD_CS_GPIO_Port, LCD_CS_Pin, GPIO_PIN_SET);
    LcdSpiTransEnd();
    return;
  }

  /* Data write or read */
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
