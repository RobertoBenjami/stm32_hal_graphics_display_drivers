/*
 * 8 bit paralell LCD FSMC driver for all stm32 family
 * 5 controll pins (CS, RS, WR, RD, RST) + 8 data pins + 1 backlight pin
 */

/*
 * Author: Roberto Benjami
 * version:  2022.12
 */

#include "main.h"
#include "lcd.h"
#include "lcd_io.h"
#include "lcd_io_fsmc8_hal.h"

//-----------------------------------------------------------------------------
#define  DMA_MINSIZE       0x0040
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

#define LCD_ADDR_DATA         (LCD_ADDR_BASE + (1 << LCD_REGSELECT_BIT))

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

#if LCD_RGB24_BUFFSIZE < DMA_MINSIZE
#undef  LCD_RGB24_BUFFSIZE
#define LCD_RGB24_BUFFSIZE 0
#else
uint8_t lcd_rgb24_dma_buffer[LCD_RGB24_BUFFSIZE * 3 + 1];
#endif  /* #else LCD_RGB24_BUFFSIZE < DMA_MINSIZE */

//-----------------------------------------------------------------------------
#ifndef  osCMSIS
/* DMA mode on, Freertos off mode */

#define LcdTransInit()

#if LCD_DMA_ENDWAIT == 0
#define LcdTransStart()       {while(dmastatus.status);}
#define LcdDmaWaitEnd(d)
#elif LCD_DMA_ENDWAIT == 1
#define LcdTransStart()       {while(dmastatus.status);}
#define LcdDmaWaitEnd(d)      {if(d) while(dmastatus.status);}
#elif LCD_DMA_ENDWAIT == 2
#define LcdTransStart()
#define LcdDmaWaitEnd(d)      {while(dmastatus.status);}
#endif  /* #elif LCD_DMA_ENDWAIT == 2 */

#define LcdTransEnd()

#define LcdDmaTransEnd()      {dmastatus.status = 0;}

#else    /* #ifndef osCMSIS */
/* Freertos mode */

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

//=============================================================================
/* TX DMA */
#if LCD_DMA_TX == 1

//-----------------------------------------------------------------------------
/* DMA operation end callback function prototype */
__weak void LCD_IO_DmaTxCpltCallback(DMA_HandleTypeDef *hdma)
{
  UNUSED(hdma);
}

//-----------------------------------------------------------------------------
/* Fill 24bit bitmap from 16bit color */
void FillConvert16to24(uint16_t color, uint8_t * tg, uint32_t Size)
{
  uint32_t c24;
  #if LCD_REVERSE16 == 0
  c24 = RGB565TO888(color);
  #elif LCD_REVERSE16 == 1
  uint16_t c16 = __REVSH(color);
  c24 = RGB565TO888(c16);
  #endif
  while(Size--)
  {
    *(uint32_t *)tg = c24;
    tg += 3;
  }
}

//-----------------------------------------------------------------------------
/* Convert from 16bit bitmnap to 24bit bitmap */
void BitmapConvert16to24(uint16_t * src, uint8_t * tg, uint32_t Size)
{
  while(Size--)
  {
    #if LCD_REVERSE16 == 0
    *(uint32_t *)tg = RGB565TO888(*src);
    #elif LCD_REVERSE16 == 1
    uint16_t c16 = __REVSH(*src);
    *(uint32_t *)tg = RGB565TO888(c16);
    #endif
    src++;
    tg += 3;
  }
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
      else if(dmastatus.status == (DMA_STATUS_MULTIDATA | DMA_STATUS_16BIT))
        dmastatus.ptr += dmastatus.trsize << 1;   /* 16bit multidata */
      else if(dmastatus.status == (DMA_STATUS_MULTIDATA | DMA_STATUS_24BIT))
        dmastatus.ptr += dmastatus.trsize << 1;   /* 16 to 24bit multidata */

      dmastatus.size -= dmastatus.trsize;
      if(dmastatus.size <= dmastatus.maxtrsize)
        dmastatus.trsize = dmastatus.size;

      #if LCD_RGB24_BUFFSIZE == 0
      HAL_DMA_Start_IT(&LCD_DMA_HANDLE, dmastatus.ptr, LCD_ADDR_DATA, dmastatus.trsize);
      #else /* #if LCD_RGB24_BUFFSIZE == 0 */
      if(dmastatus.status == (DMA_STATUS_MULTIDATA | DMA_STATUS_24BIT))
      {
        BitmapConvert16to24((uint16_t *)dmastatus.ptr, lcd_rgb24_dma_buffer, dmastatus.trsize);
        HAL_DMA_Start_IT(&LCD_DMA_HANDLE, (uint32_t)&lcd_rgb24_dma_buffer, LCD_ADDR_DATA, dmastatus.trsize * 3);
      }
      else if(dmastatus.status == (DMA_STATUS_FILL | DMA_STATUS_24BIT))
        HAL_DMA_Start_IT(&LCD_DMA_HANDLE, (uint32_t)&lcd_rgb24_dma_buffer, LCD_ADDR_DATA, dmastatus.trsize * 3);
      else
        HAL_DMA_Start_IT(&LCD_DMA_HANDLE, dmastatus.ptr, LCD_ADDR_DATA, dmastatus.trsize);
      #endif /* #if LCD_RGB24_BUFFSIZE == 0 */
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

  #if LCD_REVERSE16 == 0
  if((Size > DMA_MINSIZE) && (!LCD_DMA_UNABLE((uint32_t)pData)) && (Mode & (LCD_IO_FILL | LCD_IO_DATA8 | LCD_IO_DATA16TO24)))
  #elif LCD_REVERSE16 == 1
  if((Size > DMA_MINSIZE) && (!LCD_DMA_UNABLE((uint32_t)pData)))
  #endif
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
        #if LCD_REVERSE16 == 0
        dmastatus.data = __REVSH(*(uint16_t *)pData);
        #elif LCD_REVERSE16 == 1
        if(Mode & LCD_IO_REVERSE16)
          dmastatus.data = *(uint16_t *)pData;
        else
          dmastatus.data = __REVSH(*(uint16_t *)pData);
        #endif
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
        #if LCD_REVERSE16 == 0
        c16 = __REVSH(*pD16);
        #elif LCD_REVERSE16 == 1
        if(Mode & LCD_IO_REVERSE16)
          c16 = *pD16;
        else
          c16 = __REVSH(*pD16);
        #endif
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
        #if LCD_REVERSE16 == 1
        if(Mode & LCD_IO_REVERSE16)
        {
          while(Size--)
          {
            *(volatile uint16_t *)LCD_ADDR_DATA = *pD16;
            pD16++;
          }
        }
        else
        #endif
        {
          while(Size--)
          {
            *(volatile uint16_t *)LCD_ADDR_DATA = __REVSH(*pD16);
            pD16++;
          }
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
    uint32_t c24;
  }rgb888;
  #if LCD_REVERSE16 == 1
  uint16_t c16;
  #endif

  #if LCD_DMA_TX == 1 && LCD_RGB24_BUFFSIZE > 0
  if(Size > DMA_MINSIZE)
  { /* DMA mode */
    dmastatus.maxtrsize = LCD_RGB24_BUFFSIZE;
    dmastatus.size = Size;

    if(Size > LCD_RGB24_BUFFSIZE)
      dmastatus.trsize = LCD_RGB24_BUFFSIZE;
    else
      dmastatus.trsize = Size;

    if(Mode & LCD_IO_FILL)
    {
      dmastatus.status = DMA_STATUS_FILL | DMA_STATUS_24BIT;
      FillConvert16to24(*(uint16_t *)pData, lcd_rgb24_dma_buffer, dmastatus.trsize);
    }
    else
    {
      dmastatus.status = DMA_STATUS_MULTIDATA | DMA_STATUS_24BIT;
      dmastatus.ptr = (uint32_t)pData;
      BitmapConvert16to24((uint16_t *)pData, lcd_rgb24_dma_buffer, dmastatus.trsize);
    }

    LCD_DMA_HANDLE.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    LCD_DMA_HANDLE.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
    LCD_DMA_HANDLE.Init.PeriphInc = DMA_PINC_ENABLE;
    LCD_DMA_HANDLE.Init.MemInc = DMA_MINC_DISABLE;
    LCD_DMA_HANDLE.XferCpltCallback = &HAL_DMA_TxCpltCallback;
    HAL_DMA_Init(&LCD_DMA_HANDLE);
    HAL_DMA_Start_IT(&LCD_DMA_HANDLE,  (uint32_t)&lcd_rgb24_dma_buffer, LCD_ADDR_DATA, dmastatus.trsize * 3);

    LcdDmaWaitEnd(m & LCD_IO_FILL);
  }
  else
  #endif
  { /* not DMA mode */
    if(Mode & LCD_IO_FILL)
    { /* fill 16bit to 24bit */
      #if LCD_REVERSE16 == 0
      rgb888.c24 = RGB565TO888(*pData);
      #elif LCD_REVERSE16 == 1
      c16 = __REVSH(*pData);
      rgb888.c24 = RGB565TO888(c16);
      #endif
      while(Size--)
      {
        *(volatile uint8_t *)LCD_ADDR_DATA = rgb888.c8[0];
        *(volatile uint8_t *)LCD_ADDR_DATA = rgb888.c8[1];
        *(volatile uint8_t *)LCD_ADDR_DATA = rgb888.c8[2];
      }
    }
    else
    { /* multidata 16bit to 24bit */
      while(Size--)
      {
        #if LCD_REVERSE16 == 0
        rgb888.c24 = RGB565TO888(*pData);
        #elif LCD_REVERSE16 == 1
        c16 = __REVSH(*pData);
        rgb888.c24 = RGB565TO888(c16);
        #endif
        *(volatile uint8_t *)LCD_ADDR_DATA = rgb888.c8[0];
        *(volatile uint8_t *)LCD_ADDR_DATA = rgb888.c8[1];
        *(volatile uint8_t *)LCD_ADDR_DATA = rgb888.c8[2];
        pData++;
      }
    }
    LcdTransEnd();
  }
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
/* Convert from 16bit bitmnap to 24bit bitmap */
void BitmapConvert24to16(uint8_t * src, uint16_t * tg, uint32_t Size)
{
  uint32_t c24;
  while(Size--)
  {
    c24 = *(uint32_t *)src;
    #if LCD_REVERSE16 == 0
    *tg = RGB888TO565(c24);
    #elif LCD_REVERSE16 == 1
    *tg = __REVSH(RGB888TO565(c24));
    #endif
    src += 3;
    tg++;
  }
}

//-----------------------------------------------------------------------------
/* SPI DMA operation interrupt */
void HAL_DMA_RxCpltCallback(DMA_HandleTypeDef *hdma)
{
  if(hdma == &LCD_DMA_HANDLE)
  {
    #if LCD_RGB24_BUFFSIZE > 0
    if(dmastatus.status == (DMA_STATUS_MULTIDATA | DMA_STATUS_24BIT))
    {
      BitmapConvert24to16(lcd_rgb24_dma_buffer, (uint16_t *)dmastatus.ptr, dmastatus.trsize);
      dmastatus.ptr += dmastatus.trsize << 1; /* 24bit multidata */
    }
    else
    #endif /* LCD_RGB24_BUFFSIZE > 0 */
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

      #if LCD_RGB24_BUFFSIZE > 0
      if(dmastatus.status == (DMA_STATUS_MULTIDATA | DMA_STATUS_24BIT))
        HAL_DMA_Start_IT(&LCD_DMA_HANDLE, LCD_ADDR_DATA, (uint32_t)&lcd_rgb24_dma_buffer, dmastatus.trsize * 3);
      else
      #endif /* #if LCD_RGB24_BUFFSIZE > 0 */
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
  #if LCD_DMA_RX == 1
  #if LCD_REVERSE16 == 0
  if((Size > DMA_MINSIZE) && (!LCD_DMA_UNABLE((uint32_t)pData)) && (Mode & LCD_IO_DATA8))
  #elif LCD_REVERSE16 == 1
  if((Size > DMA_MINSIZE) && (!LCD_DMA_UNABLE((uint32_t)pData)) && (Mode & (LCD_IO_DATA8 | LCD_IO_REVERSE16)))
  #endif
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
    LcdDmaWaitEnd(m & LCD_IO_MULTIDATA);
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
      #if LCD_REVERSE16 == 1
      if(Mode & LCD_IO_REVERSE16)
      {
        while(Size--)
        {
          *pD16 = *(volatile uint16_t *)LCD_ADDR_DATA;
          pD16++;
        }
      }
      else
      #endif
      {
        while(Size--)
        {
          *pD16 = __REVSH(*(volatile uint16_t *)LCD_ADDR_DATA);
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
    uint32_t c24;
  }rgb888;

  #if LCD_DMA_RX == 1 && LCD_RGB24_BUFFSIZE > 0
  if(Size > DMA_MINSIZE)
  { /* DMA mode */
    /* SPI RX DMA setting (8bit, multidata) */
    LCD_DMA_HANDLE.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    LCD_DMA_HANDLE.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
    LCD_DMA_HANDLE.Init.MemInc = DMA_MINC_ENABLE;

    dmastatus.maxtrsize = LCD_RGB24_BUFFSIZE;
    dmastatus.size = Size;

    if(Size > LCD_RGB24_BUFFSIZE)
      dmastatus.trsize = LCD_RGB24_BUFFSIZE;
    else
      dmastatus.trsize = Size;

    dmastatus.status = DMA_STATUS_MULTIDATA | DMA_STATUS_24BIT;
    dmastatus.ptr = (uint32_t)pData;
    LCD_DMA_HANDLE.XferCpltCallback = &HAL_DMA_RxCpltCallback;
    HAL_DMA_Init(&LCD_DMA_HANDLE);
    HAL_DMA_Start_IT(&LCD_DMA_HANDLE, LCD_ADDR_DATA, (uint32_t)&lcd_rgb24_dma_buffer, dmastatus.trsize * 3);
    LcdDmaWaitEnd(1);
  }
  else
  #endif /* #if LCD_DMA_RX == 1 && LCD_RGB24_BUFFSIZE > 0 */
  { /* not DMA mode */
    while(Size--)
    {
      rgb888.c8[0] = *(volatile uint8_t *)LCD_ADDR_DATA;
      rgb888.c8[1] = *(volatile uint8_t *)LCD_ADDR_DATA;
      rgb888.c8[2] = *(volatile uint8_t *)LCD_ADDR_DATA;
      #if LCD_REVERSE16 == 0
      *pData = RGB888TO565(rgb888.c24);
      #elif LCD_REVERSE16 == 1
      *pData = __REVSH(RGB888TO565(rgb888.c24));
      #endif
      pData++;
    }
    LcdTransEnd();
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
    *(volatile uint16_t *)LCD_ADDR_BASE = __REVSH(Cmd);

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
