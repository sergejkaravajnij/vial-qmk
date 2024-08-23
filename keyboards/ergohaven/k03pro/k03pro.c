#include "display.h"

static bool display_enabled;

bool is_display_side(void) {
#ifdef K03_DISPLAY_RIGHT
    return !is_keyboard_left();
#endif
#ifdef K03_DISPLAY_LEFT
    return is_keyboard_left();
#endif
    return false;
}

bool is_display_enabled(void) {
    return display_enabled;
}

void housekeeping_task_user(void) {
    if (is_display_enabled()) {
        display_housekeeping_task();
    }
}

// void keyboard_post_init_user(void) {
//     display_init_kb();
// }

report_mouse_t pointing_device_task_user(report_mouse_t mouse_report) {
    static int32_t scroll_accumulated_h = 0;
    static int32_t scroll_accumulated_v = 0;
    static int32_t scroll_divisor_x     = 8;
    static int32_t scroll_divisor_y     = 8;

    if (mouse_report.h != 0 || mouse_report.v != 0) {
        scroll_accumulated_h += mouse_report.h;
        scroll_accumulated_v += mouse_report.v;

        mouse_report.h = scroll_accumulated_h / scroll_divisor_x;
        mouse_report.v = scroll_accumulated_v / scroll_divisor_y;

        scroll_accumulated_h -= mouse_report.h * scroll_divisor_x;
        scroll_accumulated_v -= mouse_report.v * scroll_divisor_y;

        mouse_report.x = 0;
        mouse_report.y = 0;
    }
    return mouse_report;
}

void keyboard_post_init_user(void) {
    display_enabled = false;

    if (is_display_side()) {
        display_enabled = display_init_kb();
    }

    pointing_device_set_cpi(800);
}

layer_state_t layer_state_set_user(layer_state_t state) {
    if (is_display_enabled()) {
        display_process_layer_state(get_highest_layer(state));
    }
    //   #if defined(AUDIO_ENABLE)
    //     static bool is_base_on = false;
    // if (layer_state_cmp(state, _BASE) != is_base_on) {
    //         is_base_on = layer_state_cmp(state, _BASE);
    //         if (is_base_on) {
    //             stop_all_notes();
    //         } else {
    //             PLAY_SONG(base_sound);
    //         }
    //     }
    // #endif
    return state;
}
