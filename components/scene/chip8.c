#include "display.h"
#include "esp_check.h"
#include "esp_log.h"
#include <stdint.h>
#include "chip8.h"
#include <string.h>
#include "esp_random.h"
#include "canvas.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "keypad.h"

#define X(op) ((op & 0x0F00) >> 8)
#define Y(op) ((op & 0x00F0) >> 4)
#define N(op) (op & 0x000F)
#define KK(op) (op & 0x00FF)
#define NNN(op) (op & 0x0FFF)

const static char *TAG = "Chip8";

chip8_t *new(void)
{
    chip8_t *self = (chip8_t *)calloc(1, sizeof(chip8_t));
    memcpy(&self->ram[CHIP8_RAM_START_ADDR], FONTSET, FONTSET_SIZE);
    self->pc = PC_START;
    return self;
}

void load_rom(chip8_t *self, const char *buf, int size)
{
    memcpy(&self->ram[PROGRAM_START_ADDR], buf, size);
}

void delete(chip8_t *self)
{
    free(self);
}

void update_timers(chip8_t *self)
{
    if (self->delay_tim > 0)
    {
        self->delay_tim--;
    }
    if (self->sound_tim > 0)
    {
        self->sound_tim--;
    }
}

void fetch_op(chip8_t *self)
{
    self->curr_op = self->ram[self->pc] << 8 | self->ram[self->pc + 1];
    self->pc += 2;
}

void keypress(chip8_t *self, int idx, bool is_pressed)
{
    if (idx >= 0 && idx < 16)
    {
        self->keypad[idx] = is_pressed ? 1 : 0;
    }
}
void exec_op(chip8_t *self)
{
    // ESP_LOGI(TAG, "opcode: 0x%04X", self->curr_op);
    switch (self->curr_op & 0xF000)
    {
    case 0x0000:
        switch (self->curr_op & 0x00FF)
        {
        case 0x00E0:
            memset(self->screen, false, SCREEN_WIDTH * SCREEN_HEIGHT);
            break;
        case 0x00EE:
            self->sp -= 1;
            self->pc = self->stack[self->sp];
            break;
        default:
            ESP_LOGE(TAG, "Wrong Opcode!");
            break;
        }
        break;
    case 0x1000:
        self->pc = NNN(self->curr_op);
        break;
    case 0x2000:
        self->stack[self->sp] = self->pc;
        self->sp += 1;
        self->pc = NNN(self->curr_op);
        break;
    case 0x3000:
    {
        if (self->v_reg[X(self->curr_op)] == KK(self->curr_op))
        {
            self->pc += 2;
        }
        break;
    }
    case 0x4000:
    {
        if (self->v_reg[X(self->curr_op)] != KK(self->curr_op))
        {
            self->pc += 2;
        }
        break;
    }
    case 0x5000:
    {
        if (self->v_reg[X(self->curr_op)] == self->v_reg[Y(self->curr_op)])
        {
            self->pc += 2;
        }
        break;
    }
    case 0x6000:
    {
        self->v_reg[X(self->curr_op)] = KK(self->curr_op);
        break;
    }
    case 0x7000:
    {
        self->v_reg[X(self->curr_op)] += KK(self->curr_op);
        break;
    }
    case 0x8000:
        switch (self->curr_op & 0x000F)
        {
        case 0x0000:
        {
            self->v_reg[X(self->curr_op)] = self->v_reg[Y(self->curr_op)];
            break;
        }
        case 0x0001:
        {
            self->v_reg[X(self->curr_op)] |= self->v_reg[Y(self->curr_op)];
            break;
        }
        case 0x0002:
        {
            self->v_reg[X(self->curr_op)] &= self->v_reg[Y(self->curr_op)];
            break;
        }
        case 0x0003:
        {
            self->v_reg[X(self->curr_op)] ^= self->v_reg[Y(self->curr_op)];
            break;
        }
        case 0x0004:
        {
            self->v_reg[0xF] = self->v_reg[X(self->curr_op)] + self->v_reg[Y(self->curr_op)] > 0xFF;
            self->v_reg[X(self->curr_op)] += self->v_reg[Y(self->curr_op)];
            break;
        }
        case 0x0005:
        {
            self->v_reg[0xF] = self->v_reg[X(self->curr_op)] > self->v_reg[Y(self->curr_op)];
            self->v_reg[X(self->curr_op)] -= self->v_reg[Y(self->curr_op)];
            break;
        }
        case 0x0006:
        {
            self->v_reg[0xF] = self->v_reg[X(self->curr_op)] & 0x1;
            self->v_reg[X(self->curr_op)] >>= 1;
            break;
        }
        case 0x0007:
        {
            self->v_reg[0xF] = self->v_reg[Y(self->curr_op)] > self->v_reg[X(self->curr_op)];
            self->v_reg[X(self->curr_op)] = self->v_reg[Y(self->curr_op)] - self->v_reg[X(self->curr_op)];
            break;
        }
        case 0x000E:
        {
            self->v_reg[0xF] = (self->v_reg[X(self->curr_op)] & 0x80) >> 7;
            self->v_reg[X(self->curr_op)] <<= 1;
            break;
        }
        break;
        }
        break;
    case 0x9000:
    {
        if (self->v_reg[X(self->curr_op)] != self->v_reg[Y(self->curr_op)])
        {
            self->pc += 2;
        }
        break;
    }
    case 0xA000:
    {
        self->i_reg = NNN(self->curr_op);
        break;
    }
    case 0xB000:
    {
        self->pc = NNN(self->curr_op) + self->v_reg[0];
        break;
    }
    case 0xC000:
    {
        uint8_t random_byte = (uint8_t)(esp_random() % 256);
        self->v_reg[X(self->curr_op)] = random_byte & KK(self->curr_op);
        break;
    }
    case 0xD000:
    {
        uint8_t x_coord = self->v_reg[X(self->curr_op)];
        uint8_t y_coord = self->v_reg[Y(self->curr_op)];
        uint8_t num_rows = N(self->curr_op);

        bool flipped = false;

        for (int y_line = 0; y_line < num_rows; y_line++)
        {
            uint16_t addr = self->i_reg + y_line;
            uint8_t pixels = self->ram[addr];

            for (int x_line = 0; x_line < 8; x_line++)
            {
                if (pixels & (0x80 >> x_line))
                {
                    int x = (x_coord + x_line) % SCREEN_WIDTH;
                    int y = (y_coord + y_line) % SCREEN_HEIGHT;

                    int idx = x + (SCREEN_WIDTH * y);

                    if (self->screen[idx])
                        flipped = true;

                    self->screen[idx] ^= true;
                }
            }
        }

        self->v_reg[0xF] = flipped ? 1 : 0;
        self->is_drawing = true;
        break;
    }
    case 0xE000:
        switch (KK(self->curr_op))
        {
        case 0x9E00:
            if (self->keypad[self->v_reg[X(self->curr_op)]])
            {
                self->pc += 2;
            }
            break;
        case 0xA100:
            if (!self->keypad[self->v_reg[X(self->curr_op)]])
            {
                self->pc += 2;
            }
            break;
        }
        break;

    case 0xF000:
        switch (KK(self->curr_op))
        {
        case 0x0700:
            self->v_reg[X(self->curr_op)] = self->delay_tim;
            break;
        case 0x0A00:
        {
            bool key_pressed = false;
            for (int i = 0; i < 16; i++)
            {
                if (self->keypad[i])
                {
                    self->v_reg[X(self->curr_op)] = i;
                    key_pressed = true;
                    break;
                }
            }
            if (!key_pressed)
            {
                self->pc -= 2;
            }
            break;
        }
        case 0x1500:
            self->delay_tim = self->v_reg[X(self->curr_op)];
            break;
        case 0x1800:
            self->sound_tim = self->v_reg[X(self->curr_op)];
            break;
        case 0x1E00:
            self->i_reg += self->v_reg[X(self->curr_op)];
            break;
        case 0x2900:
            self->i_reg = CHIP8_RAM_START_ADDR + (self->v_reg[X(self->curr_op)] * 5);
            break;
        case 0x3300:
        {
            uint8_t val = self->v_reg[X(self->curr_op)];
            self->ram[self->i_reg] = val / 100;
            self->ram[self->i_reg + 1] = (val / 10) % 10;
            self->ram[self->i_reg + 2] = val % 10;
            break;
        }
        case 0x5500:
            for (int i = 0; i <= X(self->curr_op); i++)
            {
                self->ram[self->i_reg + i] = self->v_reg[i];
            }
            break;
        case 0x6500:
            for (int i = 0; i <= X(self->curr_op); i++)
            {
                self->v_reg[i] = self->ram[self->i_reg + i];
            }
            break;
        }
        break;
    default:
        ESP_LOGE(TAG, "Unimplemented opcode: 0x%04X", self->curr_op);
        break;
    }
}

char Tetris__Fran_Dachille__1991__ch8[] = {
    0xa2, 0xb4, 0x23, 0xe6, 0x22, 0xb6, 0x70, 0x01, 0xd0, 0x11, 0x30, 0x25,
    0x12, 0x06, 0x71, 0xff, 0xd0, 0x11, 0x60, 0x1a, 0xd0, 0x11, 0x60, 0x25,
    0x31, 0x00, 0x12, 0x0e, 0xc4, 0x70, 0x44, 0x70, 0x12, 0x1c, 0xc3, 0x03,
    0x60, 0x1e, 0x61, 0x03, 0x22, 0x5c, 0xf5, 0x15, 0xd0, 0x14, 0x3f, 0x01,
    0x12, 0x3c, 0xd0, 0x14, 0x71, 0xff, 0xd0, 0x14, 0x23, 0x40, 0x12, 0x1c,
    0xe7, 0xa1, 0x22, 0x72, 0xe8, 0xa1, 0x22, 0x84, 0xe9, 0xa1, 0x22, 0x96,
    0xe2, 0x9e, 0x12, 0x50, 0x66, 0x00, 0xf6, 0x15, 0xf6, 0x07, 0x36, 0x00,
    0x12, 0x3c, 0xd0, 0x14, 0x71, 0x01, 0x12, 0x2a, 0xa2, 0xc4, 0xf4, 0x1e,
    0x66, 0x00, 0x43, 0x01, 0x66, 0x04, 0x43, 0x02, 0x66, 0x08, 0x43, 0x03,
    0x66, 0x0c, 0xf6, 0x1e, 0x00, 0xee, 0xd0, 0x14, 0x70, 0xff, 0x23, 0x34,
    0x3f, 0x01, 0x00, 0xee, 0xd0, 0x14, 0x70, 0x01, 0x23, 0x34, 0x00, 0xee,
    0xd0, 0x14, 0x70, 0x01, 0x23, 0x34, 0x3f, 0x01, 0x00, 0xee, 0xd0, 0x14,
    0x70, 0xff, 0x23, 0x34, 0x00, 0xee, 0xd0, 0x14, 0x73, 0x01, 0x43, 0x04,
    0x63, 0x00, 0x22, 0x5c, 0x23, 0x34, 0x3f, 0x01, 0x00, 0xee, 0xd0, 0x14,
    0x73, 0xff, 0x43, 0xff, 0x63, 0x03, 0x22, 0x5c, 0x23, 0x34, 0x00, 0xee,
    0x80, 0x00, 0x67, 0x05, 0x68, 0x06, 0x69, 0x04, 0x61, 0x1f, 0x65, 0x10,
    0x62, 0x07, 0x00, 0xee, 0x40, 0xe0, 0x00, 0x00, 0x40, 0xc0, 0x40, 0x00,
    0x00, 0xe0, 0x40, 0x00, 0x40, 0x60, 0x40, 0x00, 0x40, 0x40, 0x60, 0x00,
    0x20, 0xe0, 0x00, 0x00, 0xc0, 0x40, 0x40, 0x00, 0x00, 0xe0, 0x80, 0x00,
    0x40, 0x40, 0xc0, 0x00, 0x00, 0xe0, 0x20, 0x00, 0x60, 0x40, 0x40, 0x00,
    0x80, 0xe0, 0x00, 0x00, 0x40, 0xc0, 0x80, 0x00, 0xc0, 0x60, 0x00, 0x00,
    0x40, 0xc0, 0x80, 0x00, 0xc0, 0x60, 0x00, 0x00, 0x80, 0xc0, 0x40, 0x00,
    0x00, 0x60, 0xc0, 0x00, 0x80, 0xc0, 0x40, 0x00, 0x00, 0x60, 0xc0, 0x00,
    0xc0, 0xc0, 0x00, 0x00, 0xc0, 0xc0, 0x00, 0x00, 0xc0, 0xc0, 0x00, 0x00,
    0xc0, 0xc0, 0x00, 0x00, 0x40, 0x40, 0x40, 0x40, 0x00, 0xf0, 0x00, 0x00,
    0x40, 0x40, 0x40, 0x40, 0x00, 0xf0, 0x00, 0x00, 0xd0, 0x14, 0x66, 0x35,
    0x76, 0xff, 0x36, 0x00, 0x13, 0x38, 0x00, 0xee, 0xa2, 0xb4, 0x8c, 0x10,
    0x3c, 0x1e, 0x7c, 0x01, 0x3c, 0x1e, 0x7c, 0x01, 0x3c, 0x1e, 0x7c, 0x01,
    0x23, 0x5e, 0x4b, 0x0a, 0x23, 0x72, 0x91, 0xc0, 0x00, 0xee, 0x71, 0x01,
    0x13, 0x50, 0x60, 0x1b, 0x6b, 0x00, 0xd0, 0x11, 0x3f, 0x00, 0x7b, 0x01,
    0xd0, 0x11, 0x70, 0x01, 0x30, 0x25, 0x13, 0x62, 0x00, 0xee, 0x60, 0x1b,
    0xd0, 0x11, 0x70, 0x01, 0x30, 0x25, 0x13, 0x74, 0x8e, 0x10, 0x8d, 0xe0,
    0x7e, 0xff, 0x60, 0x1b, 0x6b, 0x00, 0xd0, 0xe1, 0x3f, 0x00, 0x13, 0x90,
    0xd0, 0xe1, 0x13, 0x94, 0xd0, 0xd1, 0x7b, 0x01, 0x70, 0x01, 0x30, 0x25,
    0x13, 0x86, 0x4b, 0x00, 0x13, 0xa6, 0x7d, 0xff, 0x7e, 0xff, 0x3d, 0x01,
    0x13, 0x82, 0x23, 0xc0, 0x3f, 0x01, 0x23, 0xc0, 0x7a, 0x01, 0x23, 0xc0,
    0x80, 0xa0, 0x6d, 0x07, 0x80, 0xd2, 0x40, 0x04, 0x75, 0xfe, 0x45, 0x02,
    0x65, 0x04, 0x00, 0xee, 0xa7, 0x00, 0xf2, 0x55, 0xa8, 0x04, 0xfa, 0x33,
    0xf2, 0x65, 0xf0, 0x29, 0x6d, 0x32, 0x6e, 0x00, 0xdd, 0xe5, 0x7d, 0x05,
    0xf1, 0x29, 0xdd, 0xe5, 0x7d, 0x05, 0xf2, 0x29, 0xdd, 0xe5, 0xa7, 0x00,
    0xf2, 0x65, 0xa2, 0xb4, 0x00, 0xee, 0x6a, 0x00, 0x60, 0x19, 0x00, 0xee,
    0x37, 0x23};
unsigned int Tetris__Fran_Dachille__1991__ch8_len = 494;

static volatile bool is_stop;
static void chip8_task(void *arg)
{

    chip8_t *chip8 = new();
    load_rom(chip8, Tetris__Fran_Dachille__1991__ch8, Tetris__Fran_Dachille__1991__ch8_len);
    ESP_LOGI(TAG, "LOADED ROM TO RAM");

    while (!is_stop)
    {
        for (int i = 0; i < 16; i++)
        {
            keypress(chip8, i, false);
        }
        keypad_key_t key = keypad_scan();

        keypress(chip8, (int)(key - 1), (bool)(key));

        for (int e = 0; e < 10; e++)
        {
            fetch_op(chip8);
            exec_op(chip8);
        }
        update_timers(chip8);

        if (chip8->is_drawing)
        {

            chip8->is_drawing = false;

            display_mux_lock();
            bool *p = chip8->screen;

            for (int y = 0; y < SCREEN_HEIGHT; y++)
            {
                int sy = y * 2;

                for (int x = 0; x < SCREEN_WIDTH; x++)
                {
                    int sx = x * 2;
                    int color = *p++;

                    canvas_draw_pixel_locked(sx, sy, !color);
                    canvas_draw_pixel_locked(sx + 1, sy, !color);
                    canvas_draw_pixel_locked(sx, sy + 1, !color);
                    canvas_draw_pixel_locked(sx + 1, sy + 1, !color);
                }
            }
            display_mux_unlock();
        }

        vTaskDelay(pdMS_TO_TICKS(2));
    }
    delete(chip8);
    vTaskDelete(NULL);
}

void chip8_run()
{
    ESP_LOGI(TAG, "RUN");

    is_stop = false;
    xTaskCreate(chip8_task, "Chip8Task", 2 * 10240, NULL, 5, NULL);
}

void chip8_stop()
{
    ESP_LOGI(TAG, "STOP");
    is_stop = true;
}