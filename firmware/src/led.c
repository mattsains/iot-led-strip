#include "driver/ledc.h"

static ledc_channel_config_t ledc_channel[4];

void setup_leds(void)
{
    ledc_timer_config_t ledc_timer = {
        .duty_resolution = LEDC_TIMER_11_BIT,
        .freq_hz = 20000,
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .timer_num = LEDC_TIMER_1,
        .clk_cfg = LEDC_AUTO_CLK,
    };

    ledc_timer_config(&ledc_timer);

    ledc_channel[0] = (ledc_channel_config_t){
        .channel = LEDC_CHANNEL_0,
        .duty = 0,
        .gpio_num = 4,
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .hpoint = 0,
        .timer_sel = LEDC_TIMER_1};
    ledc_channel[1] = (ledc_channel_config_t){
        .channel = LEDC_CHANNEL_1,
        .duty = 0,
        .gpio_num = 18,
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .hpoint = 0,
        .timer_sel = LEDC_TIMER_1};

    ledc_channel[2] = (ledc_channel_config_t){
        .channel = LEDC_CHANNEL_2,
        .duty = 0,
        .gpio_num = 16,
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .hpoint = 0,
        .timer_sel = LEDC_TIMER_1};

    ledc_channel[3] = (ledc_channel_config_t){
        .channel = LEDC_CHANNEL_3,
        .duty = 0,
        .gpio_num = 17,
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .hpoint = 0,
        .timer_sel = LEDC_TIMER_1};

    ledc_channel_config(&ledc_channel[0]);
    ledc_channel_config(&ledc_channel[1]);
    ledc_channel_config(&ledc_channel[2]);
    ledc_channel_config(&ledc_channel[3]);

    ledc_fade_func_install(0);
}

void set_led(uint8_t led, uint32_t level) {
    int actual = (level * 2047)/1000;
    ledc_set_fade_with_time(ledc_channel[led].speed_mode, ledc_channel[led].channel, actual, 500);
    ledc_fade_start(ledc_channel[led].speed_mode, ledc_channel[led].channel, LEDC_FADE_NO_WAIT);
}