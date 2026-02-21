#include "font_changa_24i_num.c"
#include "font_terminus_12.c"
#include "font_betania_28num.c"
#include "lvgl.h"
lv_font_t *get_font_changa24i_num()
{
    return &font_changa_24i_num;
}
lv_font_t *get_font_terminus12()
{
    return &font_terminus_12;
}
lv_font_t *get_font_betania28_num()
{
    return &font_betania_28num;
}
