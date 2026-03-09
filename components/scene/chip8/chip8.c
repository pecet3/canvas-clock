#include "display.h"
#include "esp_check.h"
#include "esp_log.h"
#include <stdint.h>
#include "chip8.h"
#include <string.h>

const static char *TAG = "Chip8";

chip8_t *new(void)
{
    chip8_t *self = (chip8_t *)calloc(1, sizeof(chip8_t));
    memcpy(&self->ram[CHIP8_RAM_START_ADDR], FONTSET, FONTSET_SIZE);
    return self;
}

void delete(chip8_t *self)
{
    free(self);
}

uint16_t nnn(chip8_t *self)
{
    return self->curr_op & 0x0FFF;
}

uint8_t kk(chip8_t *self)
{
    return self->curr_op & 0x00FF;
}

uint8_t x(chip8_t *self)
{
    return (self->curr_op & 0x0F00) >> 8;
}

uint8_t y(chip8_t *self)
{
    return (self->curr_op & 0x00F0) >> 4;
}

void fetch_op(chip8_t *self)
{
    self->curr_op = self->ram[self->pc] << 8 | self->ram[self->pc + 1];
}

void exec_op(chip8_t *self)
{
    self->pc += 2;

    switch (self->curr_op & 0xF000)
    {
    case 0x0000:
        switch (self->curr_op & 0x00FF)
        {
        case 0x00E0:
            memset(self->screen, 0, SCREEN_WIDTH * SCREEN_HEIGHT);
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
        self->pc = nnn(self);
        break;
    case 0x2000:
        self->stack[self->sp] = self->pc;
        self->sp += 1;
        self->pc = nnn(self);
        break;
    case 0x3000:
    {
        if (self->v_reg[x(self)] == kk(self))
        {
            self->pc += 2;
        }
        break;
    }
    case 0x4000:
    {
        if (self->v_reg[x(self)] != kk(self))
        {
            self->pc += 2;
        }
        break;
    }
    case 0x5000:
    {
        if (self->v_reg[x(self)] == self->v_reg[y(self)])
        {
            self->pc += 2;
        }
        break;
    }
    case 0x6000:
    {
        self->v_reg[x(self)] = kk(self);
        break;
    }
    case 0x7000:
    {
        self->v_reg[x(self)] += kk(self);
        break;
    }
    case 0x8000:
        switch (self->curr_op & 0x000F)
        {
        case 0x0000:
        {
            self->v_reg[x(self)] = self->v_reg[y(self)];
            break;
        }
        case 0x0001:
        {
            self->v_reg[x(self)] |= self->v_reg[y(self)];
            break;
        }
        case 0x0002:
        {
            self->v_reg[x(self)] &= self->v_reg[y(self)];
            break;
        }
        case 0x0003:
        {
            self->v_reg[x(self)] ^= self->v_reg[y(self)];
            break;
        }
        case 0x0004:
        {
            self->v_reg[0xF] = self->v_reg[x(self)] + self->v_reg[y(self)] > 0xFF;
            self->v_reg[x(self)] += self->v_reg[y(self)];
            break;
        }
        case 0x0005:
        {
            self->v_reg[0xF] = self->v_reg[x(self)] > self->v_reg[y(self)];
            self->v_reg[x(self)] -= self->v_reg[y(self)];
            break;
        }
        case 0x0006:
        {
            self->v_reg[0xF] = self->v_reg[x(self)] & 0x1;
            self->v_reg[x(self)] >>= 1;
            break;
        }
        case 0x0007:
        {
            self->v_reg[0xF] = self->v_reg[y(self)] > self->v_reg[x(self)];
            self->v_reg[x(self)] = self->v_reg[y(self)] - self->v_reg[x(self)];
            break;
        }
        case 0x000E:
        {
            self->v_reg[0xF] = (self->v_reg[x(self)] & 0x80) >> 7;
            self->v_reg[x(self)] <<= 1;
            break;
        }
        break;
        }
        break;
    default:
        ESP_LOGE(TAG, "Unimplemented opcode: 0x%04X", self->curr_op);
        break;
    }
}
