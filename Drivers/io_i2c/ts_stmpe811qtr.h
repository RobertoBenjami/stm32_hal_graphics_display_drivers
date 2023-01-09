/*
 * STMPE811 HAL I2C touch driver
 * author: Roberto Benjami
 * v.2023.01
 *
 */

//=============================================================================
/* Setting section (please set the necessary things in this section) */

/* I2C handle select (see in main.c file, default: hi2c1, hi2c1 ... hi2c3) */
#define TS_I2C_HANDLE         hi2c1

/* I2C device address (if ADDR0 pin == 0 -> 0x82, if ADDR0 pin == 1 -> 0x88) */
#define TS_I2C_ADDRESS        0x82

/* I2C device timeout (ms) */
#define TS_I2C_TIMEOUT        30
