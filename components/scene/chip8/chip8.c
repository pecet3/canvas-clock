#include "display.h"
#include "esp_check.h"
#include "esp_log.h"
#include <stdint.h>
#include "chip8.h"
#include <string.h>
#include "esp_random.h"

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

void delete(chip8_t *self)
{
    free(self);
}

void fetch_op(chip8_t *self)
{
    self->curr_op = self->ram[self->pc] << 8 | self->ram[self->pc + 1];
    self->pc += 2;
}

void exec_op(chip8_t *self)
{
    ESP_LOGI(TAG, "opcode: 0x%04X", self->curr_op);
    switch (self->curr_op & 0xF000)
    {
    case 0x0:
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
    case 0x1:
        self->pc = NNN(self->curr_op);
        break;
    case 0x2:
        self->stack[self->sp] = self->pc;
        self->sp += 1;
        self->pc = NNN(self->curr_op);
        break;
    case 0x3:
    {
        if (self->v_reg[X(self->curr_op)] == KK(self->curr_op))
        {
            self->pc += 2;
        }
        break;
    }
    case 0x4:
    {
        if (self->v_reg[X(self->curr_op)] != KK(self->curr_op))
        {
            self->pc += 2;
        }
        break;
    }
    case 0x5:
    {
        if (self->v_reg[X(self->curr_op)] == self->v_reg[Y(self->curr_op)])
        {
            self->pc += 2;
        }
        break;
    }
    case 0x6:
    {
        self->v_reg[X(self->curr_op)] = KK(self->curr_op);
        break;
    }
    case 0x7:
    {
        self->v_reg[X(self->curr_op)] += KK(self->curr_op);
        break;
    }
    case 0x8:
        switch (self->curr_op & 0x000F)
        {
        case 0x0:
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
    case 0x9:
    {
        if (self->v_reg[X(self->curr_op)] != self->v_reg[Y(self->curr_op)])
        {
            self->pc += 2;
        }
        break;
    }
    case 0xA:
    {
        self->i_reg = NNN(self->curr_op);
        break;
    }
    case 0xB:
    {
        self->pc = NNN(self->curr_op) + self->v_reg[0];
        break;
    }
    case 0xC:
    {
        uint8_t random_byte = (uint8_t)(esp_random() % 256);
        self->v_reg[X(self->curr_op)] = random_byte & KK(self->curr_op);
        break;
    }
    case 0xD:
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
        break;
    }
    case 0xE:
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

    case 0xF:
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
