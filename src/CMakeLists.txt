SET(SOURCES screens/ui_Screen1.c
    screens/ui_Screen2.c
    ui.c
    components/ui_comp_hook.c
    ui_helpers.c
    ui_events.c
    images/ui_img_map.c
    images/ui_img_battery_indicator_20_png.c
    images/ui_img_battery_indicator_40_png.c
    images/ui_img_battery_indicator_60_png.c
    images/ui_img_battery_indicator_80_png.c
    images/ui_img_battery_indicator_100_png.c
    images/ui_img_navitation_arrow_destination_png.c
    images/ui_img_start_png.c
    images/ui_img_arrival_png.c
    images/ui_img_straight_png.c
    images/ui_img_continue_left_png.c
    images/ui_img_continue_right_png.c
    images/ui_img_left_end_png.c
    images/ui_img_left_uturn_png.c
    images/ui_img_off_ramp_slight_left_png.c
    images/ui_img_off_ramp_slight_right_png.c
    images/ui_img_right_end_png.c
    images/ui_img_round_left_270_png.c
    images/ui_img_round_right_90_png.c
    images/ui_img_round_straight_180_png.c
    images/ui_img_slight_left_png.c
    images/ui_img_slight_right_png.c
    )
add_library(ui ${SOURCES})
