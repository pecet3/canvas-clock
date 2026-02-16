#pragma once
typedef enum
{
    SCENE_CLOCK,
    SCENE_CANVAS,
    SCENE_CURRENCY,
    SCENE_MAIN,
} scene_t;

void scene_set(scene_t scene);
void scene_init(void);