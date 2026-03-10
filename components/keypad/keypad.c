#include "driver/gpio.h"
#include "keypad.h"
int row_pins[4] = {19, 18, 5, 17};
int col_pins[4] = {16, 4, 0, 2};

keypad_key_t keymap[4][4] = {
    {KEY_1, KEY_2, KEY_3, KEY_A},
    {KEY_4, KEY_5, KEY_6, KEY_B},
    {KEY_7, KEY_8, KEY_9, KEY_C},
    {KEY_STAR, KEY_0, KEY_HASH, KEY_D}};

#define DEBOUNCE_COUNT 3

uint8_t key_counter[4][4] = {0};
uint8_t key_state[4][4] = {0};

keypad_key_t keypad_scan()
{
    for (int c = 0; c < 4; c++)
    {
        gpio_set_level(col_pins[c], 0);

        for (int r = 0; r < 4; r++)
        {
            int pressed = (gpio_get_level(row_pins[r]) == 0);

            if (pressed)
            {
                if (key_counter[r][c] < DEBOUNCE_COUNT)
                    key_counter[r][c]++;

                if (key_counter[r][c] == DEBOUNCE_COUNT && key_state[r][c] == 0)
                {
                    key_state[r][c] = 1;
                    gpio_set_level(col_pins[c], 1);
                    return keymap[r][c];
                }
            }
            else
            {
                key_counter[r][c] = 0;
                key_state[r][c] = 0;
            }
        }

        gpio_set_level(col_pins[c], 1);
    }

    return KEY_NONE;
}

void keypad_init()
{
    gpio_config_t io_conf = {0};

    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.pull_up_en = GPIO_PULLUP_ENABLE;
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    io_conf.intr_type = GPIO_INTR_DISABLE;

    for (int i = 0; i < 4; i++)
    {
        io_conf.pin_bit_mask = (1ULL << row_pins[i]);
        gpio_config(&io_conf);
    }

    io_conf.mode = GPIO_MODE_OUTPUT;
    io_conf.pull_up_en = GPIO_PULLUP_DISABLE;

    for (int i = 0; i < 4; i++)
    {
        io_conf.pin_bit_mask = (1ULL << col_pins[i]);
        gpio_config(&io_conf);
        gpio_set_level(col_pins[i], 1);
    }
}