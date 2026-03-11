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
#include "storage.h"
#define X(op) ((op & 0x0F00) >> 8)
#define Y(op) ((op & 0x00F0) >> 4)
#define N(op) (op & 0x000F)
#define KK(op) (op & 0x00FF)
#define NNN(op) (op & 0x0FFF)

const static char *TAG = "Chip8";

chip8_t *new(void)
{
    chip8_t *self = (chip8_t *)calloc(1, sizeof(chip8_t));
    memcpy(&self->ram[CHIP8_RAM_START_ADDR], FONTSET, sizeof(FONTSET));
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
        self->keypad[idx] = is_pressed;
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
        case 0x9E:
            if (self->keypad[self->v_reg[X(self->curr_op)]])
            {
                self->pc += 2;
            }
            break;
        case 0xA1:
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
        case 0x07:
            self->v_reg[X(self->curr_op)] = self->delay_tim;
            break;
        case 0x0A:
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
        case 0x15:
            self->delay_tim = self->v_reg[X(self->curr_op)];
            break;
        case 0x18:
            self->sound_tim = self->v_reg[X(self->curr_op)];
            break;
        case 0x1E:
            self->i_reg += self->v_reg[X(self->curr_op)];
            break;
        case 0x29:
            self->i_reg = CHIP8_RAM_START_ADDR + (self->v_reg[X(self->curr_op)] * 5);
            break;
        case 0x33:
        {
            uint8_t val = self->v_reg[X(self->curr_op)];
            self->ram[self->i_reg] = val / 100;
            self->ram[self->i_reg + 1] = (val / 10) % 10;
            self->ram[self->i_reg + 2] = val % 10;
            break;
        }
        case 0x55:
            for (int i = 0; i <= X(self->curr_op); i++)
            {
                self->ram[self->i_reg + i] = self->v_reg[i];
            }
            break;
        case 0x65:
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

int keypad_to_chip8(keypad_key_t key)
{
    switch (key)
    {
    case KEY_1:
        return 0x1;
    case KEY_2:
        return 0x2;
    case KEY_3:
        return 0x3;
    case KEY_4:
        return 0x4;

    case KEY_5:
        return 0x5;
    case KEY_6:
        return 0x6;
    case KEY_7:
        return 0x7;
    case KEY_8:
        return 0x8;
    case KEY_9:
        return 0x9;
    case KEY_A:
        return 0xC;
    case KEY_B:
        return 0xD;
    case KEY_C:
        return 0xE;
    case KEY_STAR:
        return 0xA;
    case KEY_0:
        return 0x0;
    case KEY_HASH:
        return 0xB;
    case KEY_D:
        return 0xF;

    default:
        return -1;
    }
}

static uint16_t expand_table[256];

static void chip8_expand_init()
{
    for (int i = 0; i < 256; i++)
    {
        uint16_t out = 0;

        for (int b = 0; b < 8; b++)
        {
            if (i & (1 << (7 - b)))
            {
                out |= 3 << (14 - b * 2);
            }
        }

        expand_table[i] = out;
    }
}

static chip8_t *chip8;
static volatile bool is_stop;
static void chip8_task(void *arg)
{
    chip8_expand_init();

    while (!is_stop)
    {
        for (int i = 0; i < 16; i++)
        {
            keypress(chip8, i, false);
        }
        keypad_key_t key = keypad_scan();

        if (key != KEY_NONE)
        {
            keypress(chip8, keypad_to_chip8(key), true);
        }

        for (int e = 0; e < 8; e++)
        {
            fetch_op(chip8);
            exec_op(chip8);
        }
        update_timers(chip8);

        if (chip8->is_drawing)
        {

            chip8->is_drawing = false;

            uint8_t buf[CANVAS_BUF_SIZE];
            canvas_get_drawing_buf((char *)(buf), CANVAS_BUF_SIZE);

            int stride = CANVAS_WIDTH / 8;
            bool *p = chip8->screen;

            for (int y = 0; y < 32; y++)
            {
                int dy = y * 2;

                uint8_t *row1 = &buf[8 + dy * stride];
                uint8_t *row2 = &buf[8 + (dy + 1) * stride];

                for (int x = 0; x < 64; x += 8)
                {
                    uint8_t sprite = 0;

                    for (int i = 0; i < 8; i++)
                        sprite |= (*p++ << (7 - i));

                    uint16_t expanded = expand_table[sprite];

                    int bx = (x * 2) >> 3;

                    row1[bx] = expanded >> 8;
                    row1[bx + 1] = expanded & 0xFF;

                    row2[bx] = expanded >> 8;
                    row2[bx + 1] = expanded & 0xFF;
                }
            }

            canvas_set_drawing_buf((char *)buf, sizeof(buf));
        }

        vTaskDelay(pdMS_TO_TICKS(16));
    }
    vTaskDelete(NULL);
}

void chip8_run()
{
    ESP_LOGI(TAG, "RUN");
    chip8 = new();
    if (chip8 == NULL)
    {
        ESP_LOGE(TAG, "Failed to init chip8");
        return;
    }
    storage_chip8_rom_t *rom = storage_get_chip8_rom("Tetris");
    if (rom == NULL)
    {
        ESP_LOGE(TAG, "Failed to find ROM with name: %s", rom->name);
        return;
    }
    load_rom(chip8, rom->rom, rom->len);
    ESP_LOGI(TAG, "LOADED ROM: %s %d", rom->name, rom->len);
    is_stop = false;
    xTaskCreate(chip8_task, "Chip8Task", 10240, NULL, 5, NULL);
}

void chip8_stop()
{
    ESP_LOGI(TAG, "STOP");
    is_stop = true;
    vTaskDelay(pdMS_TO_TICKS(10));
    delete(chip8);
}