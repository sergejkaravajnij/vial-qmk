#include QMK_KEYBOARD_H

#include "quantum.h"

#define SCROLL_DIVISOR_H 32.0
#define SCROLL_DIVISOR_V 32.0

static bool scroll_enabled = false;

static int32_t scroll_accumulated_h = 0;
static int32_t scroll_accumulated_v = 0;

report_mouse_t pointing_device_task_user(report_mouse_t mouse_report) {
    if (scroll_enabled) {
        scroll_accumulated_h += mouse_report.x;
        scroll_accumulated_v -= mouse_report.y;

        mouse_report.h = scroll_accumulated_h / SCROLL_DIVISOR_H;
        mouse_report.v = scroll_accumulated_v / SCROLL_DIVISOR_V;

        scroll_accumulated_h -= mouse_report.h * SCROLL_DIVISOR_H;
        scroll_accumulated_v -= mouse_report.v * SCROLL_DIVISOR_V;

        mouse_report.x = 0;
        mouse_report.y = 0;
    }
    return mouse_report;
}

bool led_update_user(led_t led_state) {
    scroll_enabled = led_state.num_lock;
    return true;
}

void pointing_device_init_kb(void) {
    // set the DPI.
    pointing_device_set_cpi(800);
}
