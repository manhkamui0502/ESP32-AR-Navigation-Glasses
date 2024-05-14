// This file was generated by SquareLine Studio
// SquareLine Studio version: SquareLine Studio 1.3.4
// LVGL version: 8.3.6
// Project name: SquareLine_Project

#include "../ui.h"

void ui_Screen2_screen_init(void)
{
    ui_Screen2 = lv_obj_create(NULL);
    lv_obj_clear_flag(ui_Screen2, LV_OBJ_FLAG_SCROLLABLE); /// Flags
    lv_obj_set_style_bg_color(ui_Screen2, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_Screen2, 255, LV_PART_MAIN | LV_STATE_DEFAULT);

    ui_mapPanel = lv_obj_create(ui_Screen2);
    lv_obj_set_width(ui_mapPanel, lv_pct(100));
    lv_obj_set_height(ui_mapPanel, lv_pct(100));
    lv_obj_set_align(ui_mapPanel, LV_ALIGN_CENTER);
    lv_obj_clear_flag(ui_mapPanel, LV_OBJ_FLAG_SCROLLABLE); /// Flags
    lv_obj_set_style_radius(ui_mapPanel, 2, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui_mapPanel, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_mapPanel, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui_mapPanel, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    ui_mapImage = lv_img_create(ui_mapPanel);
    lv_obj_set_width(ui_mapImage, LV_SIZE_CONTENT);  /// 1
    lv_obj_set_height(ui_mapImage, LV_SIZE_CONTENT); /// 1
    lv_obj_set_x(ui_mapImage, -14);
    lv_obj_set_y(ui_mapImage, -1);
    lv_obj_set_align(ui_mapImage, LV_ALIGN_CENTER);
    lv_obj_add_flag(ui_mapImage, LV_OBJ_FLAG_ADV_HITTEST);  /// Flags
    lv_obj_clear_flag(ui_mapImage, LV_OBJ_FLAG_SCROLLABLE); /// Flags

    ui_batteryPanel2 = lv_obj_create(ui_Screen2);
    lv_obj_set_width(ui_batteryPanel2, 21);
    lv_obj_set_height(ui_batteryPanel2, 16);
    lv_obj_set_x(ui_batteryPanel2, 68);
    lv_obj_set_y(ui_batteryPanel2, -27);
    lv_obj_set_align(ui_batteryPanel2, LV_ALIGN_CENTER);
    lv_obj_clear_flag(ui_batteryPanel2, LV_OBJ_FLAG_SCROLLABLE); /// Flags
    lv_obj_set_style_radius(ui_batteryPanel2, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui_batteryPanel2, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui_batteryPanel2, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui_batteryPanel2, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    ui_batteryImage2 = lv_img_create(ui_batteryPanel2);
    lv_img_set_src(ui_batteryImage2, &ui_img_battery_indicator_100_png);
    lv_obj_set_width(ui_batteryImage2, LV_SIZE_CONTENT);  /// 1
    lv_obj_set_height(ui_batteryImage2, LV_SIZE_CONTENT); /// 1
    lv_obj_set_x(ui_batteryImage2, -1);
    lv_obj_set_y(ui_batteryImage2, -2);
    lv_obj_set_align(ui_batteryImage2, LV_ALIGN_CENTER);
    lv_obj_add_flag(ui_batteryImage2, LV_OBJ_FLAG_ADV_HITTEST);  /// Flags
    lv_obj_clear_flag(ui_batteryImage2, LV_OBJ_FLAG_SCROLLABLE); /// Flags
}
