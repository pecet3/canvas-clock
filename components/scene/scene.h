#pragma once

typedef enum
{
    SCENE_SET_MAIN,
    SCENE_SET_CANVAS,

    SCENE_CANVAS_SAVE_SLOT,
    SCENE_CANVAS_LOAD_SLOT,
    SCENE_CANVAS_DELETE_SLOT,
    SCENE_CANVAS_DRAW_BUF,

} scene_event_t;
void scene_event(scene_event_t event, void *data);
void scene_init(void);