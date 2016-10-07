#include <linux/types.h>

/*  PIN  VALUE
* --------------------------------------------------------------------
*  P9_11 30 | P9_12 60 | P9_14 50 | P9_16 51 | P9_41A 20 | P9_42A 7
*  P8_11 45 | P8_13 23 | P8_15 47 | P8_16 46
* --------------------------------------------------------------------
*/
// LED related GPIO settings
#define GPIO_LED_ANODE 60
#define GPIO_LED_CATHODE 50
//#define GPIO_BUFFER_CONTROL 30
#define GPIO_BUFFER_CONTROL 51

// Qing - May 2, 2015
#define GPIO_LED_OR_PD 2 // P9_22  Choose between PD and LED
#define GPIO_H_POWER_LED 49 // P9_23  Output of high power LED

// SPI related GPIO settings
#define SPI_CLC 45 // 32+13 P8_11
#define SPI_MISO 23 // 0+23 P8_13
#define SPI_MOSI 47 // 32+15 P8_15
//#define SPI_CS 46 // 32+14 P8_16
#define SPI_CS 27 // 0+27 P8_17

#define ADDR_BASE_0 0x44e07000
#define ADDR_BASE_1 0x4804c000
#define READ_OFFSET 0x138
#define SET_OFFSET 0x194
#define CLEAR_OFFSET 0x190

#define BIT_CLC (45-32) // 32+13 P8_11
#define BIT_MISO (23) // 0+23 P8_13
#define BIT_MOSI (47-32) // 32+15 P8_15
//#define BIT_CS (46-32) // 32+14 P8_16
#define BIT_CS (27) // 0+27 P8_17

#define BIT_LED_ANODE (60-32)
#define BIT_LED_CATHODE (50-32)
//#define BIT_BUFFER_CONTROL (30)
#define BIT_BUFFER_CONTROL (51-32)
// Qing - May 2, 2015
#define BIT_LED_OR_PD 2 // Choose between PD or LED 
#define BIT_H_POWER_LED (49-32) // For high power LED
