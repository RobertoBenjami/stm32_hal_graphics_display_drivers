//-----------------------------------------------------------------------------
/* Setting section */

/* Bitmap and character test select
   - 0: do not use the bitmap and character test
   - 1: apply the bitmap and character test */
#define BITMAP_TEST           1

/* Pixel and Image read test and verify
   - 0: test off
   - 1: test on */
#define READ_TEST             1

/* Freertos also measures cpu usage
   - 0: measure off
   - 1: measure on */
#define POWERMETER            1

/* check the defaultTask and Task2 stack owerflow (only freertos)
   - 0: check off
   - 1: check on */
#define STACKOWERFLOW_CHECK   1

/* Chapter delays */
#define DELAY_CHAPTER         1000

/* Leds pin assign: leds pin user label in Cube
   note: If no have a led -> delete or commented this defines */
#define LED1_NAME             LED0
#define LED2_NAME             LED1

/* Led activ state
   - 0 Led on if pin state is reset
   - 1 Led on if pin state is set */
#define LED_ACTIVE            0
