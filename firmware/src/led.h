#include "driver/ledc.h"
#include "driver/rmt.h"

#define LED_0_W 4
#define LED_0_R 18
#define LED_0_G 16
#define LED_0_B 17

#define LED_1_W 14
#define LED_1_R 26
#define LED_1_G 25
#define LED_1_B 27

#define LED_2_DAT 12

void setup_leds(void);
void set_led(uint8_t led, uint32_t level);
void set_strip(uint8_t *data, size_t num_bytes);