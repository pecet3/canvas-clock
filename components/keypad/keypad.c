#include "driver/gpio.h"

int row_pins[4] = {19, 18, 5, 17};
int col_pins[4] = {16, 4, 0, 2};

char keymap[4][4] = {
    {'1', '2', '3', 'A'},
    {'4', '5', '6', 'B'},
    {'7', '8', '9', 'C'},
    {'*', '0', '#', 'D'}};

char keypad_scan()
{
    for (int c = 0; c < 4; c++)
    {
        gpio_set_level(col_pins[c], 0);

        for (int r = 0; r < 4; r++)
        {
            if (gpio_get_level(row_pins[r]) == 0)
            {
                gpio_set_level(col_pins[c], 1);
                return keymap[r][c];
            }
        }

        gpio_set_level(col_pins[c], 1);
    }

    return 0;
}

void keypad_init()
{
    gpio_config_t io_conf;

    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.pull_up_en = 1;
    io_conf.pull_down_en = 0;
    io_conf.intr_type = GPIO_INTR_DISABLE;

    for (int i = 0; i < 4; i++)
    {
        io_conf.pin_bit_mask = (1ULL << row_pins[i]);
        gpio_config(&io_conf);
    }

    io_conf.mode = GPIO_MODE_OUTPUT;
    io_conf.pull_up_en = 0;

    for (int i = 0; i < 4; i++)
    {
        io_conf.pin_bit_mask = (1ULL << col_pins[i]);
        gpio_config(&io_conf);
        gpio_set_level(col_pins[i], 1);
    }
}
