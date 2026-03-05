#pragma once
typedef enum
{
    SCENE_CLOCK,
    SCENE_CANVAS_DRAW,
    SCENE_CANVAS_SHOW,
    SCENE_CANVAS_ALARM,
    SCENE_CURRENCY,
    SCENE_MAIN,
} scene_t;

void scene_set(scene_t scene, void *data);
void scene_init(void);