#include QMK_KEYBOARD_H

#include "quantum.h"

#define SCROLL_DIVISOR_H 32.0
#define SCROLL_DIVISOR_V 32.0

static bool scroll_enabled = false;

static int32_t scroll_accumulated_h = 0;
static int32_t scroll_accumulated_v = 0;

typedef union {
    uint32_t raw;
    struct {
        uint8_t dpi_mode : 3;
    };
} vial_config_t;

vial_config_t vial_config;

int get_dpi(uint8_t dpi_mode) {
    switch (dpi_mode) {
        case 0:
            return 400;
        case 1:
            return 500;
        case 2:
            return 630;
        default:
        case 3:
            return 800;
        case 4:
            return 1000;
        case 5:
            return 1250;
        case 6:
            return 1600;
        case 7:
            return 2000;
    }
}

void via_set_layout_options_kb(uint32_t value) {
    vial_config.raw = value;
    pointing_device_set_cpi(get_dpi(vial_config.dpi_mode));
}

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
    vial_config.raw = via_get_layout_options();
    via_set_layout_options_kb(vial_config.raw);
}
