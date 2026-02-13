/* Console example — declarations of command registration functions.

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include "lvgl.h"
#pragma once

#ifdef __cplusplus
extern "C"
{
#endif
    void display_init(void);
    _lock_t *display_get_lvgl_mux(void);
    lv_display_t *display_get_lvgl_displ(void);

    void display_mux_lock(void);
    void display_mux_unlock(void);
#ifdef __cplusplus
}
#endif
