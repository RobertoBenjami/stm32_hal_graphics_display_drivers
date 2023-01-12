/*
 * 16 bit paralell LCD FSMC driver
 * 5 controll pins (CS, RS, WR, RD, RST) + 16 data pins + 1 backlight pin
 */

/*
 * Author: Roberto Benjami
 * version:  2023.01
 */

#include "main.h"
#include "lcd.h"
#include "lcd_io.h"
#include "lcd_io_fsmc16_hal.h"

//-----------------------------------------------------------------------------
#define  DMA_MINSIZE       0x0010
#define  DMA_MAXSIZE       0xFFFE
/* note:
   - DMA_MINSIZE: if the transacion Size < DMA_MINSIZE -> not use the DMA for transaction
   - DMA_MAXSIZE: if the transacion Size > DMA_MAXSIZE -> multiple DMA transactions (because DMA transaction size register is 16bit) */

//-----------------------------------------------------------------------------
/* Bitdepth convert macros */
#if LCD_RGB24_ORDER == 0
#define  RGB565TO888(c16)     ((c16 & 0xF800) << 8) | ((c16 & 0x07E0) << 5) | ((c16 & 0x001F) << 3)
#define  RGB888TO565(c24)     ((c24 & 0XF80000) >> 8 | (c24 & 0xFC00) >> 5 | (c24 & 0xF8 ) >> 3)
#elif LCD_RGB24_ORDER == 1
#define  RGB565TO888(c16)     ((c16 & 0xF800) >> 8) | ((c16 & 0x07E0) << 5) | ((c16 & 0x001F) << 19)
#define  RGB888TO565(c24)     ((c24 & 0XF80000) >> 19 | (c24 & 0xFC00) >> 5 | (c24 & 0xF8 ) << 8)
#endif /* #elif LCD_RGB24_ORDER == 1 */

#ifndef LCD_REVERSE16
#define LCD_REVERSE16         0
#endif

#define LCD_ADDR_DATA         (LCD_ADDR_BASE + (3 << LCD_REGSELECT_BIT))

//=============================================================================

extern DMA_HandleTypeDef      LCD_DMA_HANDLE;

//-----------------------------------------------------------------------------
#if LCD_DMA_TX == 0 && LCD_DMA_RX == 0
/* DMA off mode */

#define LcdTransInit()
#define LcdTransStart()
#define LcdTransEnd()

uint32_t LCD_IO_DmaBusy(void)
{
  return 0;
}

#else /* #if LCD_DMA_TX == 0 && LCD_DMA_RX == 0 */
/* DMA on mode */

#define DMA_STATUS_FREE       0
#define DMA_STATUS_FILL       (1 << 0)
#define DMA_STATUS_MULTIDATA  (1 << 1)
#define DMA_STATUS_8BIT       (1 << 2)
#define DMA_STATUS_16BIT      (1 << 3)
#define DMA_STATUS_24BIT      (1 << 4)

struct
{
  volatile uint32_t status;   /* DMA status (0=free, other: see the DMA_STATUS... macros)  */
  uint32_t size;              /* all transactions data size */
  uint32_t trsize;            /* actual DMA transaction data size */
  uint32_t maxtrsize;         /* max size / one DMA transaction */
  uint32_t ptr;               /* data pointer for DMA */
  uint16_t data;              /* fill operation data for DMA */
}dmastatus;

//-----------------------------------------------------------------------------
#ifndef  osCMSIS
/* DMA mode on, Freertos off mode */

#define LcdTransInit()

#if LCD_DMA_ENDWAIT == 0
#define LcdTransStart()       {while((volatile uint32_t)(dmastatus.status));}
#define LcdDmaWaitEnd(d)
#elif LCD_DMA_ENDWAIT == 1
#define LcdTransStart()       {while((volatile uint32_t)dmastatus.status);}
#define LcdDmaWaitEnd(d)      {if(d) while((volatile uint32_t)dmastatus.status);}
#elif LCD_DMA_ENDWAIT == 2
#define LcdTransStart()
#define LcdDmaWaitEnd(d)      {while((volatile uint32_t)dmastatus.status);}
#endif  /* #elif LCD_DMA_ENDWAIT == 2 */

#define LcdTransEnd()

#define LcdDmaTransEnd()      {dmastatus.status = 0;}


#else    /* #ifndef osCMSIS */
/* Freertos mode */

#define LCDDMASIGNAL  1

//-----------------------------------------------------------------------------
#if osCMSIS < 0x20000
/* DMA on, Freertos 1 mode */

osSemaphoreId LcdSemIdHandle;
osSemaphoreDef(LcdSemId);
#define LcdSemNew0            LcdSemIdHandle = osSemaphoreCreate(osSemaphore(LcdSemId), 1); osSemaphoreWait(LcdSemIdHandle, 0)
#define LcdSemNew1            LcdSemIdHandle = osSemaphoreCreate(osSemaphore(LcdSemId), 1)
#define LcdSemWait            osSemaphoreWait(LcdSemIdHandle, osWaitForever)
#define LcdSemSet             osSemaphoreRelease(LcdSemIdHandle)

//-----------------------------------------------------------------------------
#else /* #if osCMSIS < 0x20000 */
/* DMA on, Freertos 2 mode */

osSemaphoreId_t LcdSemId;
#define LcdSemNew0            LcdSemId = osSemaphoreNew(1, 0, 0)
#define LcdSemNew1            LcdSemId = osSemaphoreNew(1, 1, 0)
#define LcdSemWait            osSemaphoreAcquire(LcdSemId, osWaitForever)
#define LcdSemSet             osSemaphoreRelease(LcdSemId)

#endif /* #else osCMSIS < 0x20000 */

//-----------------------------------------------------------------------------
/* DMA on, Freertos 1 and 2 mode */

#if LCD_DMA_ENDWAIT == 0
#define LcdTransInit()        {LcdSemNew1;}
#define LcdTransStart()       {LcdSemWait;}
#define LcdTransEnd()         {LcdSemSet;}
#define LcdDmaWaitEnd(d)
#elif LCD_DMA_ENDWAIT == 1
#define LcdTransInit()        {LcdSemNew1;}
#define LcdTransStart()       {LcdSemWait;}
#define LcdTransEnd()         {LcdSemSet;}
#define LcdDmaWaitEnd(d)      {if(d) {LcdSemWait; LcdSemSet;}}
#elif LCD_DMA_ENDWAIT == 2
#define LcdTransInit()        {LcdSemNew0;}
#define LcdTransStart()
#define LcdTransEnd()
#define LcdDmaWaitEnd(d)      {LcdSemWait;}
#endif  /* elif LCD_DMA_ENDWAIT == 2 */

#define LcdDmaTransEnd()      {dmastatus.status = 0; LcdSemSet;}

#endif /* #else osCMSIS */

//-----------------------------------------------------------------------------
/* Get the DMA operation status (0=DMA is free, 1=DMA is busy) */
uint32_t LCD_IO_DmaBusy(void)
{
  uint32_t ret = 0;
  if(dmastatus.status != DMA_STATUS_FREE)
    ret = 1;
  return ret;
}

#endif /* #else LCD_DMA_TX == 0 && LCD_DMA_RX == 0 */

//-----------------------------------------------------------------------------
/* TX DMA */
#if LCD_DMA_TX == 1

//-----------------------------------------------------------------------------
/* DMA operation end callback function prototype */
__weak void LCD_IO_DmaTxCpltCallback(DMA_HandleTypeDef *hdma)
{
  UNUSED(hdma);
}

//-----------------------------------------------------------------------------
/* SPI DMA operation interrupt */
void HAL_DMA_TxCpltCallback(DMA_HandleTypeDef *hdma)
{
  if(hdma == &LCD_DMA_HANDLE)
  {
    if(dmastatus.size > dmastatus.trsize)
    { /* dma operation is still required */

      if(dmastatus.status == (DMA_STATUS_MULTIDATA | DMA_STATUS_8BIT))
        dmastatus.ptr += dmastatus.trsize;        /* 8bit multidata */
      if(dmastatus.status == (DMA_STATUS_MULTIDATA | DMA_STATUS_16BIT))
        dmastatus.ptr += dmastatus.trsize << 1;   /* 16bit multidata */

      dmastatus.size -= dmastatus.trsize;
      if(dmastatus.size <= dmastatus.maxtrsize)
        dmastatus.trsize = dmastatus.size;

      HAL_DMA_Start_IT(&LCD_DMA_HANDLE, dmastatus.ptr, LCD_ADDR_DATA, dmastatus.trsize);
    }
    else
    { /* dma operations have ended */
      LcdDmaTransEnd();
      LCD_IO_DmaTxCpltCallback(hdma);
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
void LCDWriteFillMultiData8and16(uint8_t * pData, uint32_t Size, uint32_t Mode)
{
  #if LCD_DMA_TX == 1

  if((Size > DMA_MINSIZE) && (!LCD_DMA_UNABLE((uint32_t)pData)))
  { /* DMA mode */

    if(Mode & LCD_IO_DATA8)
    { /* 8bit DMA */
      LCD_DMA_HANDLE.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
      LCD_DMA_HANDLE.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
      dmastatus.status = DMA_STATUS_8BIT;
    }
    else
    { /* 16bit DMA */
      LCD_DMA_HANDLE.Init.PeriphDataAlignment = DMA_PDATAALIGN_HALFWORD;
      LCD_DMA_HANDLE.Init.MemDataAlignment = DMA_MDATAALIGN_HALFWORD;
      dmastatus.status = DMA_STATUS_16BIT;
    }

    LCD_DMA_HANDLE.Init.MemInc = DMA_MINC_DISABLE;

    if(Mode & LCD_IO_FILL)
    { /* fill */
      LCD_DMA_HANDLE.Init.PeriphInc = DMA_PINC_DISABLE;
      dmastatus.status |= DMA_STATUS_FILL;
      dmastatus.ptr = (uint32_t)&dmastatus.data;

      if(Mode & LCD_IO_DATA16)
      {
        dmastatus.data = *(uint16_t *)pData;
      }
      else
        dmastatus.data = *pData;

    }
    else
    { /* multidata */
      LCD_DMA_HANDLE.Init.PeriphInc = DMA_PINC_ENABLE;
      dmastatus.status |= DMA_STATUS_MULTIDATA;
      dmastatus.ptr = (uint32_t)pData;
    }

    dmastatus.size = Size;
    dmastatus.maxtrsize = DMA_MAXSIZE;

    if(Size > DMA_MAXSIZE)
      dmastatus.trsize = DMA_MAXSIZE;
    else /* the transaction can be performed with one DMA operation */
      dmastatus.trsize = Size;

    LCD_DMA_HANDLE.XferCpltCallback = &HAL_DMA_TxCpltCallback;
    HAL_DMA_Init(&LCD_DMA_HANDLE);
    HAL_DMA_Start_IT(&LCD_DMA_HANDLE, dmastatus.ptr, LCD_ADDR_DATA, dmastatus.trsize);
    LcdDmaWaitEnd(Mode & LCD_IO_MULTIDATA);
  }
  else
  #endif /* #if LCD_DMA_TX == 1 */
  { /* not DMA mode */
    uint16_t * pD16 = (uint16_t *)pData;
    uint16_t c16;

    if(Mode & LCD_IO_FILL)
    { /* fill */
      if(Mode & LCD_IO_DATA8)
      { /* fill 8bit */
        while(Size--) /* fill 8bit */
          *(volatile uint8_t *)LCD_ADDR_DATA = *pData;
      }
      else if(Mode & LCD_IO_DATA16)
      { /* fill 16bit */
        c16 = *pD16;
        {
          while(Size--)
            *(volatile uint16_t *)LCD_ADDR_DATA = c16;
        }
      }
    }
    else
    { /* multidata */
      if(Mode & LCD_IO_DATA8)
      { /* multidata 8bit */
        while(Size--)
        {
          *(volatile uint8_t *)LCD_ADDR_DATA = *pData;
          pData++;
        }
      }
      else
      { /* multidata 16bit */
        while(Size--)
        {
          *(volatile uint16_t *)LCD_ADDR_DATA = *pD16;
          pD16++;
        }
      }
    }
    LcdTransEnd();
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
        *(volatile uint16_t *)LCD_ADDR_DATA = (rgb888.c8[2] << 8) | rgb888.c8[1];
        *(volatile uint16_t *)LCD_ADDR_DATA = (rgb888.c8[0] << 8) | rgb888.c8[2];
      }
      else
      {
        ccnt = 0;
        *(volatile uint16_t *)LCD_ADDR_DATA = (rgb888.c8[1] << 8) | rgb888.c8[0];
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
        *(volatile uint16_t *)LCD_ADDR_DATA = (rgb888.c8[2] << 8) | rgb888.c8[1];
        ctmp = rgb888.c8[0];
      }
      else
      {
        ccnt = 0;
        *(volatile uint16_t *)LCD_ADDR_DATA = (ctmp << 8) | rgb888.c8[2];
        *(volatile uint16_t *)LCD_ADDR_DATA = rgb888.c16[0];
      }
    }
    if(!ccnt)
      *(volatile uint16_t *)LCD_ADDR_DATA = ctmp << 8;
  }

  #elif LCD_RGB24_MODE == 1
  if(Mode & LCD_IO_FILL)
  { /* fill 16bit to 24bit */
    rgb888.c24 = RGB565TO888(*pData);
    while(Size--)
    {
      *(volatile uint16_t *)LCD_ADDR_DATA = rgb888.c16[0];
      *(volatile uint16_t *)LCD_ADDR_DATA = rgb888.c16[1];
    }
  }
  else
  { /* multidata 16bit to 24bit */
    while(Size--)
    {
      rgb888.c24 = RGB565TO888(*pData++);
      *(volatile uint16_t *)LCD_ADDR_DATA = rgb888.c16[0];
      *(volatile uint16_t *)LCD_ADDR_DATA = rgb888.c16[1];
    }
  }
  #endif
}

//=============================================================================
/* Read */

#if LCD_DATADIR == 1

/* RX DMA */
#if LCD_DMA_RX == 1

//-----------------------------------------------------------------------------
/* DMA operation end callback function prototype */
__weak void LCD_IO_DmaRxCpltCallback(DMA_HandleTypeDef *hdma)
{
  UNUSED(hdma);
}

//-----------------------------------------------------------------------------
/* SPI DMA operation interrupt */
void HAL_DMA_RxCpltCallback(DMA_HandleTypeDef *hdma)
{
  if(hdma == &LCD_DMA_HANDLE)
  {
    {
      if(dmastatus.status == (DMA_STATUS_MULTIDATA | DMA_STATUS_8BIT))
        dmastatus.ptr += dmastatus.trsize;        /* 8bit multidata */
      else if(dmastatus.status == (DMA_STATUS_MULTIDATA | DMA_STATUS_16BIT))
        dmastatus.ptr += dmastatus.trsize << 1;   /* 16bit multidata */
    }

    if(dmastatus.size > dmastatus.trsize)
    { /* dma operation is still required */
      dmastatus.size -= dmastatus.trsize;
      if(dmastatus.size <= dmastatus.maxtrsize)
        dmastatus.trsize = dmastatus.size;

      HAL_DMA_Start_IT(&LCD_DMA_HANDLE, LCD_ADDR_DATA, dmastatus.ptr, dmastatus.trsize);
    }
    else
    { /* dma operations have ended */
      LcdDmaTransEnd();
      LCD_IO_DmaRxCpltCallback(hdma);
    }
  }
}
#endif  /* #if LCD_DMA_RX == 1 */

//-----------------------------------------------------------------------------
/* Read data from Lcd
   - pData: 8 or 16 bits data pointer
   - Size: data number
   - bitdepth: 0 = 8bit data, 1 = 16bit data */
void LCDReadMultiData8and16(uint8_t * pData, uint32_t Size, uint32_t Mode)
{
  //uint32_t m = Mode;
  #if LCD_DMA_RX == 1
  if((Size > DMA_MINSIZE) && (!LCD_DMA_UNABLE((uint32_t)pData)) && (Mode & LCD_IO_DATA16))
  { /* DMA mode */
    /* RX DMA setting (8bit, 16bit, multidata) */
    if(Mode & LCD_IO_DATA8)
    {
      LCD_DMA_HANDLE.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
      LCD_DMA_HANDLE.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
      dmastatus.status = DMA_STATUS_MULTIDATA | DMA_STATUS_8BIT;
    }
    else
    {
      LCD_DMA_HANDLE.Init.PeriphDataAlignment = DMA_PDATAALIGN_HALFWORD;
      LCD_DMA_HANDLE.Init.MemDataAlignment = DMA_MDATAALIGN_HALFWORD;
      dmastatus.status = DMA_STATUS_MULTIDATA | DMA_STATUS_16BIT;
    }
    LCD_DMA_HANDLE.Init.PeriphInc = DMA_PINC_DISABLE;
    LCD_DMA_HANDLE.Init.MemInc = DMA_MINC_ENABLE;

    dmastatus.maxtrsize = DMA_MAXSIZE;
    dmastatus.size = Size;

    if(Size > DMA_MAXSIZE)
      dmastatus.trsize = DMA_MAXSIZE;
    else
      dmastatus.trsize = Size;

    dmastatus.ptr = (uint32_t)pData;

    LCD_DMA_HANDLE.XferCpltCallback = &HAL_DMA_RxCpltCallback;
    HAL_DMA_Init(&LCD_DMA_HANDLE);
    HAL_DMA_Start_IT(&LCD_DMA_HANDLE, LCD_ADDR_DATA, dmastatus.ptr, dmastatus.trsize);
    LcdDmaWaitEnd(Mode & LCD_IO_MULTIDATA);
  }
  else
  #endif /* #if LCD_DMA_RX == 1 */
  { /* not DMA mode */
    if(Mode & LCD_IO_DATA8)
    { /* 8bit */
      while(Size--)
      {
        *pData = *(volatile uint8_t *)LCD_ADDR_DATA;
        pData++;
      }
    }
    else
    { /* 16bit */
      uint16_t * pD16 = (uint16_t *)pData;
      {
        while(Size--)
        {
          *pD16 = *(volatile uint16_t *)LCD_ADDR_DATA;
          pD16++;
        }
      }
    }
    LcdTransEnd();
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
      rgb888.c16[1] = *(volatile uint16_t *)LCD_ADDR_DATA;
      rgb888.c16[0] = *(volatile uint16_t *)LCD_ADDR_DATA;
      ctmp = rgb888.c8[0];
      rgb888.c24 >>= 8;
    }
    else
    {
      ccnt = 0;
      rgb888.c16[0] = *(volatile uint16_t *)LCD_ADDR_DATA;
      rgb888.c8[2] = ctmp;
    }
    *pData = RGB888TO565(rgb888.c24);
    pData++;
  }

  #elif LCD_RGB24_MODE == 1
  while(Size--)
  {
    rgb888.c16[0] = *(volatile uint16_t *)LCD_ADDR_DATA;
    rgb888.c16[1] = *(volatile uint16_t *)LCD_ADDR_DATA;
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
  LcdTransInit();
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

  LcdTransStart();

  /* Command write */
  if(Mode & LCD_IO_CMD8)
    *(volatile uint8_t *)LCD_ADDR_BASE = Cmd;
  else if(Mode & LCD_IO_CMD16)
    *(volatile uint16_t *)LCD_ADDR_BASE = Cmd;

  if(Size == 0)
  { /* only command byte or word */
    LcdTransEnd();
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
    uint8_t RxDummy __attribute__((unused));
    while(DummySize--)
      RxDummy = *(volatile uint8_t *)LCD_ADDR_DATA;
    if(Mode & LCD_IO_DATA24TO16)
      LCDReadMultiData24to16((uint16_t *)pData, Size, Mode);
    else
      LCDReadMultiData8and16(pData, Size, Mode);
  }
  #endif /* #if LCD_DATADIR == 1 */
}
