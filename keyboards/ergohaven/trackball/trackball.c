#include QMK_KEYBOARD_H

#include "quantum.h"

static int32_t scroll_divisor_x = 32;
static int32_t scroll_divisor_y = 32;

static bool scroll_enabled = false;
static bool sniper_enabled = false;

static int32_t scroll_accumulated_h = 0;
static int32_t scroll_accumulated_v = 0;

typedef union {
    uint32_t raw;
    struct {
        uint8_t scroll_mode : 3;
        uint8_t sniper_mode : 1;
        uint8_t dpi_mode : 4;
    };
} vial_config_t;

vial_config_t vial_config;

static const int DPI_TABLE[] = {400, 500, 630, 800, 1000, 1250, 1600, 2000, 2500, 3200, 4000, 5000, 6300, 8000, 8200};

int get_dpi(uint8_t dpi_mode) {
    if (dpi_mode < ARRAY_SIZE(DPI_TABLE))
        return DPI_TABLE[dpi_mode];
    else
        return 800;
}

int get_scroll_div(uint8_t div_mode) {
    switch (div_mode) {
        case 0:
            return 6;
        case 1:
            return 8;
        case 2:
            return 11;
        case 3:
            return 16;
        default:
        case 4:
            return 23;
        case 5:
            return 32;
        case 6:
            return 45;
        case 7:
            return 64;
    }
}

int get_sniper_div(uint8_t mode) {
    switch (mode) {
        case 0:
        default:
            return 2;
        case 1:
            return 4;
    }
}

void via_set_layout_options_kb(uint32_t value) {
    vial_config.raw = value;
    pointing_device_set_cpi(get_dpi(vial_config.dpi_mode));
    scroll_divisor_x = scroll_divisor_y = get_scroll_div(vial_config.scroll_mode);
}

report_mouse_t pointing_device_task_user(report_mouse_t mouse_report) {
    if (scroll_enabled) {
        scroll_accumulated_h += mouse_report.x;
        scroll_accumulated_v -= mouse_report.y;

        mouse_report.h = scroll_accumulated_h / scroll_divisor_x;
        mouse_report.v = scroll_accumulated_v / scroll_divisor_y;

        scroll_accumulated_h -= mouse_report.h * scroll_divisor_x;
        scroll_accumulated_v -= mouse_report.v * scroll_divisor_y;

        mouse_report.x = 0;
        mouse_report.y = 0;
    }
    return mouse_report;
}

bool led_update_user(led_t led_state) {
    scroll_enabled = led_state.num_lock;
    if (sniper_enabled != led_state.scroll_lock) {
        sniper_enabled = led_state.scroll_lock;
        int base_dpi   = get_dpi(vial_config.dpi_mode);
        int div        = 1;
        if (sniper_enabled) div = get_sniper_div(vial_config.sniper_mode);
        pointing_device_set_cpi(base_dpi / div);
    }
    return true;
}

void pointing_device_init_kb(void) {
    vial_config.raw = via_get_layout_options();
    via_set_layout_options_kb(vial_config.raw);
}
