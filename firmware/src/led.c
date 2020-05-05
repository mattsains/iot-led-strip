#include "led.h"

static ledc_channel_config_t ledc_channel[8];

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

    // LED 0
    ledc_channel[0] = (ledc_channel_config_t){
        .channel = LEDC_CHANNEL_0,
        .duty = 0,
        .gpio_num = LED_0_W,
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .hpoint = 0,
        .timer_sel = LEDC_TIMER_1};
    ledc_channel[1] = (ledc_channel_config_t){
        .channel = LEDC_CHANNEL_1,
        .duty = 0,
        .gpio_num = LED_0_R,
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .hpoint = 0,
        .timer_sel = LEDC_TIMER_1};

    ledc_channel[2] = (ledc_channel_config_t){
        .channel = LEDC_CHANNEL_2,
        .duty = 0,
        .gpio_num = LED_0_G,
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .hpoint = 0,
        .timer_sel = LEDC_TIMER_1};

    ledc_channel[3] = (ledc_channel_config_t){
        .channel = LEDC_CHANNEL_3,
        .duty = 0,
        .gpio_num = LED_0_B,
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .hpoint = 0,
        .timer_sel = LEDC_TIMER_1};

    // LED 1
    ledc_channel[4] = (ledc_channel_config_t){
        .channel = LEDC_CHANNEL_0,
        .duty = 0,
        .gpio_num = LED_1_W,
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .hpoint = 0,
        .timer_sel = LEDC_TIMER_1};
    ledc_channel[5] = (ledc_channel_config_t){
        .channel = LEDC_CHANNEL_1,
        .duty = 0,
        .gpio_num = LED_1_R,
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .hpoint = 0,
        .timer_sel = LEDC_TIMER_1};

    ledc_channel[6] = (ledc_channel_config_t){
        .channel = LEDC_CHANNEL_2,
        .duty = 0,
        .gpio_num = LED_1_G,
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .hpoint = 0,
        .timer_sel = LEDC_TIMER_1};

    ledc_channel[7] = (ledc_channel_config_t){
        .channel = LEDC_CHANNEL_3,
        .duty = 0,
        .gpio_num = LED_1_B,
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .hpoint = 0,
        .timer_sel = LEDC_TIMER_1};

    for (uint8_t i = 0; i < 8; i++)
    {
        ledc_channel_config(&ledc_channel[i]);
    }

    ledc_fade_func_install(0);

    // set up ws2381 strip
    rmt_config_t config = {
        .rmt_mode = RMT_MODE_TX,
        .channel = RMT_CHANNEL_0,
        .clk_div = 1,
        .gpio_num = LED_2_DAT,
        .mem_block_num = 1,
        .tx_config = {
            .carrier_freq_hz = 0,
            .carrier_level = RMT_CARRIER_LEVEL_HIGH,
            .idle_level = RMT_IDLE_LEVEL_LOW,
            .carrier_duty_percent = 33,
            .carrier_en = false,
            .loop_en = false,
            .idle_output_en = true,
        }};

    rmt_config(&config);
    rmt_driver_install(RMT_CHANNEL_0, 0, 0);
}

void set_led(uint8_t led, uint32_t level)
{
    int actual = (level * 2047) / 1000;
    ledc_set_fade_with_time(ledc_channel[led].speed_mode, ledc_channel[led].channel, actual, 500);
    ledc_fade_start(ledc_channel[led].speed_mode, ledc_channel[led].channel, LEDC_FADE_NO_WAIT);
}

void deallocate_tx_buffer(rmt_channel_t channel, void *buffer) {
    free(buffer);
}

void set_strip(uint8_t *data, size_t num_bytes)
{
    rmt_item32_t *outBuffer = malloc((8 * num_bytes + 1) * sizeof(rmt_item32_t));

    for (size_t i = 0; i < num_bytes; i++)
    {
        uint8_t b = data[i];
        for (uint8_t j = 0; j < 8; j++)
        {
            if (b && (1 << j) != 0)
            {
                outBuffer[i * 8 + j] = (rmt_item32_t){{{65, 1, 36, 0}}};
            }
            else
            {
                outBuffer[i * 8 + j] = (rmt_item32_t){{{32, 1, 69, 0}}};
            }
        }
    }
    outBuffer[num_bytes * 8] = (rmt_item32_t){{{2419, 0, 2419, 0}}};

    rmt_register_tx_end_callback(&deallocate_tx_buffer, outBuffer);
    rmt_write_items(RMT_CHANNEL_0, outBuffer, 1, false);
}