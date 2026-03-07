/*******************************************************************************
 * Size: 30 px
 * Bpp: 1
 * Opts: --bpp 1 --size 30 --no-compress --stride 1 --align 1 --font Micro5-Regular.ttf --range 48-63 --format lvgl -o font_micro_30num.c
 ******************************************************************************/

#ifdef __has_include
#if __has_include("lvgl.h")
#ifndef LV_LVGL_H_INCLUDE_SIMPLE
#define LV_LVGL_H_INCLUDE_SIMPLE
#endif
#endif
#endif

#ifdef LV_LVGL_H_INCLUDE_SIMPLE
#include "lvgl.h"
#else
#include "lvgl/lvgl.h"
#endif

#ifndef FONT_MICRO_30NUM
#define FONT_MICRO_30NUM 1
#endif

#if FONT_MICRO_30NUM

/*-----------------
 *    BITMAPS
 *----------------*/

/*Store the image of the glyphs*/
static LV_ATTRIBUTE_LARGE_CONST const uint8_t glyph_bitmap[] = {
    /* U+0030 "0" */
    0xff, 0xff, 0xff, 0xe7, 0xe7, 0xe7, 0xe7, 0xe7,
    0xe7, 0xe7, 0xe7, 0xff, 0xff, 0xff,

    /* U+0031 "1" */
    0xff, 0xff, 0xc7, 0x1c, 0x71, 0xc7, 0x1c, 0x71,
    0xc7, 0x1c, 0x70,

    /* U+0032 "2" */
    0xff, 0xff, 0xff, 0x7, 0x7, 0x7, 0xff, 0xff,
    0xff, 0xe0, 0xe0, 0xff, 0xff, 0xff,

    /* U+0033 "3" */
    0xff, 0xff, 0xff, 0xe0, 0x70, 0x38, 0x1c, 0x7e,
    0x3f, 0x1f, 0x81, 0xc0, 0xff, 0xff, 0xff, 0xfc,

    /* U+0034 "4" */
    0xe7, 0xe7, 0xe7, 0xe7, 0xe7, 0xe7, 0xff, 0xff,
    0xff, 0x7, 0x7, 0x7, 0x7, 0x7,

    /* U+0035 "5" */
    0xff, 0xff, 0xff, 0xe0, 0xe0, 0xe0, 0xff, 0xff,
    0xff, 0x7, 0x7, 0xff, 0xff, 0xff,

    /* U+0036 "6" */
    0xff, 0xff, 0xff, 0xe0, 0xe0, 0xe0, 0xff, 0xff,
    0xff, 0xe7, 0xe7, 0xff, 0xff, 0xff,

    /* U+0037 "7" */
    0xff, 0xff, 0xff, 0xe0, 0x70, 0x38, 0x1c, 0xe,
    0x7, 0x3, 0x8e, 0x7, 0x3, 0x81, 0xc0, 0xe0,

    /* U+0038 "8" */
    0xff, 0xff, 0xff, 0xe7, 0xe7, 0xe7, 0xff, 0xff,
    0xff, 0xe7, 0xe7, 0xff, 0xff, 0xff,

    /* U+0039 "9" */
    0xff, 0xff, 0xff, 0xe7, 0xe7, 0xe7, 0xff, 0xff,
    0xff, 0x7, 0x7, 0xff, 0xff, 0xff,

    /* U+003A ":" */
    0xff, 0x80, 0x0, 0xff, 0x80,

    /* U+003B ";" */
    0xff, 0x80, 0x0, 0x1f, 0xff, 0xc0,

    /* U+003C "<" */
    0x1f, 0x1f, 0x1f, 0xe0, 0xe0, 0xe0, 0x1f, 0x1f,
    0x1f,

    /* U+003D "=" */
    0xff, 0xff, 0xff, 0x0, 0x0, 0x0, 0xff, 0xff,
    0xff,

    /* U+003E ">" */
    0xf8, 0xf8, 0xf8, 0x7, 0x7, 0x7, 0xf8, 0xf8,
    0xf8,

    /* U+003F "?" */
    0xff, 0xff, 0xff, 0xe0, 0x70, 0x38, 0x1c, 0x70,
    0x38, 0x1c, 0x0, 0x0, 0x3, 0x81, 0xc0, 0xe0};

/*---------------------
 *  GLYPH DESCRIPTION
 *--------------------*/

static const lv_font_fmt_txt_glyph_dsc_t glyph_dsc[] = {
    {.bitmap_index = 0, .adv_w = 0, .box_w = 0, .box_h = 0, .ofs_x = 0, .ofs_y = 0} /* id = 0 reserved */,
    {.bitmap_index = 0, .adv_w = 175, .box_w = 8, .box_h = 14, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 14, .adv_w = 131, .box_w = 6, .box_h = 14, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 25, .adv_w = 175, .box_w = 8, .box_h = 14, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 39, .adv_w = 175, .box_w = 9, .box_h = 14, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 55, .adv_w = 175, .box_w = 8, .box_h = 14, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 69, .adv_w = 175, .box_w = 8, .box_h = 14, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 83, .adv_w = 175, .box_w = 8, .box_h = 14, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 97, .adv_w = 175, .box_w = 9, .box_h = 14, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 113, .adv_w = 175, .box_w = 8, .box_h = 14, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 127, .adv_w = 175, .box_w = 8, .box_h = 14, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 141, .adv_w = 87, .box_w = 3, .box_h = 11, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 146, .adv_w = 87, .box_w = 3, .box_h = 14, .ofs_x = 0, .ofs_y = -3},
    {.bitmap_index = 152, .adv_w = 175, .box_w = 8, .box_h = 9, .ofs_x = 0, .ofs_y = 2},
    {.bitmap_index = 161, .adv_w = 175, .box_w = 8, .box_h = 9, .ofs_x = 0, .ofs_y = 2},
    {.bitmap_index = 170, .adv_w = 175, .box_w = 8, .box_h = 9, .ofs_x = 0, .ofs_y = 2},
    {.bitmap_index = 179, .adv_w = 175, .box_w = 9, .box_h = 14, .ofs_x = 0, .ofs_y = 0}};

/*---------------------
 *  CHARACTER MAPPING
 *--------------------*/

/*Collect the unicode lists and glyph_id offsets*/
static const lv_font_fmt_txt_cmap_t cmaps[] =
    {
        {.range_start = 48, .range_length = 16, .glyph_id_start = 1, .unicode_list = NULL, .glyph_id_ofs_list = NULL, .list_length = 0, .type = LV_FONT_FMT_TXT_CMAP_FORMAT0_TINY}};

/*--------------------
 *  ALL CUSTOM DATA
 *--------------------*/

#if LVGL_VERSION_MAJOR == 8
/*Store all the custom data of the font*/
static lv_font_fmt_txt_glyph_cache_t cache;
#endif

#if LVGL_VERSION_MAJOR >= 8
static const lv_font_fmt_txt_dsc_t font_dsc = {
#else
static lv_font_fmt_txt_dsc_t font_dsc = {
#endif
    .glyph_bitmap = glyph_bitmap,
    .glyph_dsc = glyph_dsc,
    .cmaps = cmaps,
    .kern_dsc = NULL,
    .kern_scale = 0,
    .cmap_num = 1,
    .bpp = 1,
    .kern_classes = 0,
    .bitmap_format = 0,
#if LVGL_VERSION_MAJOR == 8
    .cache = &cache
#endif

};

/*-----------------
 *  PUBLIC FONT
 *----------------*/

/*Initialize a public general font descriptor*/
#if LVGL_VERSION_MAJOR >= 8
const lv_font_t font_micro_30num = {
#else
lv_font_t font_micro_30num = {
#endif
    .get_glyph_dsc = lv_font_get_glyph_dsc_fmt_txt, /*Function pointer to get glyph's data*/
    .get_glyph_bitmap = lv_font_get_bitmap_fmt_txt, /*Function pointer to get glyph's bitmap*/
    .line_height = 17,                              /*The maximum line height required by the font*/
    .base_line = 3,                                 /*Baseline measured from the bottom of the line*/
#if !(LVGL_VERSION_MAJOR == 6 && LVGL_VERSION_MINOR == 0)
    .subpx = LV_FONT_SUBPX_NONE,
#endif
#if LV_VERSION_CHECK(7, 4, 0) || LVGL_VERSION_MAJOR >= 8
    .underline_position = -2,
    .underline_thickness = 1,
#endif
    // .static_bitmap = 0,
    .dsc = &font_dsc, /*The custom font data. Will be accessed by `get_glyph_bitmap/dsc` */
#if LV_VERSION_CHECK(8, 2, 0) || LVGL_VERSION_MAJOR >= 9
    .fallback = NULL,
#endif
    .user_data = NULL,
};

#endif /*#if FONT_MICRO_30NUM*/
