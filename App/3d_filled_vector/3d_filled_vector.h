// 3D Filled Vector Graphics
// (c) 2019 Pawel A. Hernik
// YouTube videos:
// https://youtu.be/YLf2WXjunyg
// https://youtu.be/5y28ipwQs-E

/* Screen dimension (width / height) */
#define SCR_WD  240
#define SCR_HT  320

/* 3d field dimension (width / height) */
#define WD_3D   240
#define HT_3D   320

/* Statistic character size */
#define CHARSIZEX 10
#define CHARSIZEY 12

/* Frame Buffer lines */
#define NLINES   40

/* Double buffer (if DMA transfer used -> speed inc)
   - 0 Double buffer disabled
   - 1 Double buffer enabled */
#define DOUBLEBUF    1

/* Font size (Font8 or Font12 or Font16 or Font20 or Font24) */
#define FONTNAME     Font12

/* Button pin assign: button pin user label in Cube */
#define BUTTON_NAME  BT_K0

/* Button pin active level (0 or 1) */
#define BUTTON_ON    0

/* Leds pin assign: leds pin user label in Cube
   note: If no have a led -> delete or commented this defines */
#define LED1_NAME    LED0
#define LED2_NAME    LED1

/* Led activ state
   - 0 Led on if pin state is reset
   - 1 Led on if pin state is set */
#define LED_ACTIVE   0
