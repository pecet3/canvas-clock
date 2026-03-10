#pragma once

typedef enum
{
    SCENE_SET_MAIN,
    SCENE_SET_CANVAS,
    SCENE_SET_CHIP8,

    SCENE_CHIP8_STOP,

    SCENE_CANVAS_SAVE_SLOT,
    SCENE_CANVAS_LOAD_SLOT,
    SCENE_CANVAS_DELETE_SLOT,
    SCENE_CANVAS_DRAW_BUF,
    SCENE_CANVAS_FILL_COLOR,

} scene_event_t;
void scene_event(scene_event_t event, void *data);
void scene_init(void);