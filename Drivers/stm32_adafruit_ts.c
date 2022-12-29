#include "main.h"
#include "ts.h"
#include "stm32_adafruit_ts.h"

extern TS_DrvTypeDef          *ts_drv;

uint16_t TsXBoundary, TsYBoundary;

#define MAXCINT1245           262144
#define MAXCINT36             1073741824

ts_cindex cindex = TS_CINDEX;

//-----------------------------------------------------------------------------
void BSP_TS_CalibCalc(ts_three_points * tp, ts_three_points * dp, ts_cindex * ci)
{
  long long int i0, i1, i2, i3, i4, i5, i6, i1t, i2t, i3t, i4t, i5t, i6t;
  int idv = 1, d;

  if(ci == NULL)
    ci = &cindex;

  i0 = (tp->x0 - tp->x2) * (tp->y1 - tp->y2) - (tp->x1 - tp->x2) * (tp->y0 - tp->y2);
  i1 = (dp->x0 - dp->x2) * (tp->y1 - tp->y2) - (dp->x1 - dp->x2) * (tp->y0 - tp->y2);
  i2 = (tp->x0 - tp->x2) * (dp->x1 - dp->x2) - (dp->x0 - dp->x2) * (tp->x1 - tp->x2);
  i3 = (long long int)tp->y0 * (tp->x2 * dp->x1 - tp->x1 * dp->x2) +
       (long long int)tp->y1 * (tp->x0 * dp->x2 - tp->x2 * dp->x0) +
       (long long int)tp->y2 * (tp->x1 * dp->x0 - tp->x0 * dp->x1);
  i4 = (dp->y0 - dp->y2) * (tp->y1 - tp->y2) - (dp->y1 - dp->y2) * (tp->y0 - tp->y2);
  i5 = (tp->x0 - tp->x2) * (dp->y1 - dp->y2) - (dp->y0 - dp->y2) * (tp->x1 - tp->x2);
  i6 = (long long int)tp->y0 * (tp->x2 * dp->y1 - tp->x1 * dp->y2) +
       (long long int)tp->y1 * (tp->x0 * dp->y2 - tp->x2 * dp->y0) +
       (long long int)tp->y2 * (tp->x1 * dp->y0 - tp->x0 * dp->y1);
  i1t = i1; i2t = i2; i3t = i3; i4t = i4; i5t = i5; i6t = i6;
  do
  {
    d = 0;
    if((i1t >= MAXCINT1245) || (i1t < -MAXCINT1245))
    {
      d = 1;
      i1t /= 2;
    }
    if((i2t >= MAXCINT1245) || (i2t < -MAXCINT1245))
    {
      d = 1;
      i2t /= 2;
    }
    if((i4t >= MAXCINT1245) || (i4t < -MAXCINT1245))
    {
      d = 1;
      i4t /= 2;
    }
    if((i5t >= MAXCINT1245) || (i5t < -MAXCINT1245))
    {
      d = 1;
      i5t /= 2;
    }
    if((i3t >= MAXCINT36) || (i3t < -MAXCINT36))
    {
      d = 1;
      i3t /= 2;
    }
    if((i6t >= MAXCINT36) || (i6t < -MAXCINT36))
    {
      d = 1;
      i6t /= 2;
    }
    if(d)
      idv *= 2;
  }while(d);

  if(idv > 1)
  {
    i0 /= idv; i1 /= idv; i2 /= idv; i3 /= idv; i4 /= idv; i5 /= idv; i6 /= idv;
  }

  (*ci)[0] = (int32_t)i0;
  (*ci)[1] = (int32_t)i1;
  (*ci)[2] = (int32_t)i2;
  (*ci)[3] = (int32_t)i3;
  (*ci)[4] = (int32_t)i4;
  (*ci)[5] = (int32_t)i5;
  (*ci)[6] = (int32_t)i6;
}

//-----------------------------------------------------------------------------
/* calculate display coordinate from touchscreen coordinate and cindex
   param:
   - tx  : touchscreen X coordinate
   - ty  : touchscreen Y coordinate
   - dx* : pointer to display X coordinate
   - dy* : pointer to display Y coordinate
   - mx  : max display X coordinate
   - my  : max display Y coordinate
   return: dx, dy */
void BSP_TS_DisplaycoordCalc(uint16_t tx, uint16_t ty, uint16_t * dx, uint16_t * dy, uint16_t mx, uint16_t my)
{
  int32_t x, y;
  x = (cindex[1] * tx + cindex[2] * ty + cindex[3]) / cindex[0];
  y = (cindex[4] * tx + cindex[5] * ty + cindex[6]) / cindex[0];

  if(x < 0)
    x = 0;
  else if(x > mx)
    x = mx;

  if(y < 0)
    y = 0;
  else if(y > my)
    y = my;

  *dx = x;
  *dy = y;
}

//-----------------------------------------------------------------------------
/* Set the cindex values
   param:
   - ci: pointer to cindex array */
void BSP_TS_SetCindex(ts_cindex * ci)
{
  for(uint32_t i = 0; i < 7; i++)
    cindex[i] = (*ci)[i];
}

//-----------------------------------------------------------------------------
/* Get the cindex values
   param:
   - ci: pointer to cindex array */
void BSP_TS_GetCindex(ts_cindex * ci)
{
  for(uint32_t i = 0; i < 7; i++)
    (*ci)[i] = cindex[i];
}

//-----------------------------------------------------------------------------
/**
  * @brief  Initializes and configures the touch screen functionalities and 
  *         configures all necessary hardware resources (GPIOs, clocks..).
  * @param  XSize: The maximum X size of the TS area on LCD
  * @param  YSize: The maximum Y size of the TS area on LCD  
  * @retval TS_OK: if all initializations are OK. Other value if error.
  */
uint8_t BSP_TS_Init(uint16_t XSize, uint16_t YSize)
{
  uint8_t ret = TS_ERROR;

  /* Initialize x and y positions boundaries */
  TsXBoundary = XSize;
  TsYBoundary = YSize;

  if(ts_drv)
    ret = TS_OK;

  if(ret == TS_OK)
  {
    /* Initialize the LL TS Driver */
    ts_drv->Init(0);
  }

  return ret;
}

//-----------------------------------------------------------------------------
/**
  * @brief  Returns status and positions of the touch screen.
  * @param  TsState: Pointer to touch screen current state structure
  */
void BSP_TS_GetState(TS_StateTypeDef* TsState)
{
  uint16_t tx, ty, dx, dy;

  TsState->TouchDetected = ts_drv->DetectTouch(0);
  if(TsState->TouchDetected)
  {
    ts_drv->GetXY(0, &tx, &ty);
    BSP_TS_DisplaycoordCalc(tx, ty, &dx, &dy, TsXBoundary-1, TsYBoundary-1);
    TsState->X = dx;
    TsState->Y = dy;
  }
}
