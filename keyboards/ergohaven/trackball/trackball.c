#include QMK_KEYBOARD_H

#include "quantum.h"

typedef enum { NL_SCROLL_SL_SNIPER, NL_SNIPER_SL_SCROLL, NL_TEXT_SL_SCROLL } trackball_mode_t;
typedef enum { ROT_0, ROT_90, ROT_180, ROT_270 } orientation_t;
static const int DPI_TABLE[]        = {400, 500, 630, 800, 1000, 1250, 1600, 2000, 2500, 3200, 4000, 5000, 6300, 8000, 8200};
static const int DIV_TABLE[]        = {6, 8, 11, 16, 23, 32, 45, 64};
static const int SNIPER_DIV_TABLE[] = {2, 4};

typedef union {
    uint32_t raw;
    struct {
        uint8_t scroll_mode : 3;
        uint8_t sniper_mode : 1;
        uint8_t dpi_mode : 4;
        uint8_t orientation : 2;
        uint8_t mode : 3;
    };
} vial_config_t;

static vial_config_t vial_config;

static led_t leds;

static int32_t scroll_divisor = 32;

static bool scroll_enabled = false;
static bool sniper_enabled = false;
static bool text_enabled   = false;

static int32_t accumulated_h = 0;
static int32_t accumulated_v = 0;

static orientation_t orientation;

int get_dpi(uint8_t dpi_mode) {
    if (dpi_mode < ARRAY_SIZE(DPI_TABLE))
        return DPI_TABLE[dpi_mode];
    else
        return DPI_TABLE[0];
}

int get_scroll_div(uint8_t div_mode) {
    if (div_mode < ARRAY_SIZE(DIV_TABLE))
        return DIV_TABLE[div_mode];
    else
        return DIV_TABLE[0];
}

int get_sniper_div(uint8_t mode) {
    if (mode < ARRAY_SIZE(SNIPER_DIV_TABLE))
        return SNIPER_DIV_TABLE[mode];
    else
        return SNIPER_DIV_TABLE[0];
}

void update_settings(void) {
    switch (vial_config.mode) {
        default:
        case NL_SCROLL_SL_SNIPER:
            scroll_enabled = leds.num_lock;
            sniper_enabled = leds.scroll_lock;
            text_enabled   = false;
            break;

        case NL_SNIPER_SL_SCROLL:
            scroll_enabled = leds.scroll_lock;
            sniper_enabled = leds.num_lock;
            text_enabled   = false;
            break;

        case NL_TEXT_SL_SCROLL:
            scroll_enabled = leds.scroll_lock;
            sniper_enabled = false;
            if (!scroll_enabled) text_enabled = leds.num_lock;
            break;
    }
    scroll_divisor = get_scroll_div(vial_config.scroll_mode);
    orientation    = vial_config.orientation;
    int base_dpi   = get_dpi(vial_config.dpi_mode);
    int div        = sniper_enabled ? get_sniper_div(vial_config.sniper_mode) : 1;
    pointing_device_set_cpi(base_dpi / div);
}

void via_set_layout_options_kb(uint32_t value) {
    vial_config.raw = value;
    update_settings();
}

bool led_update_user(led_t led_state) {
    leds = led_state;
    update_settings();
    return true;
}

report_mouse_t pointing_device_task_user(report_mouse_t mrpt) {
    switch (orientation) {
        int8_t tmp;
        case ROT_0:
            break;
        case ROT_270:
            tmp    = mrpt.x;
            mrpt.x = mrpt.y;
            mrpt.y = -tmp;
            break;
        case ROT_180:
            mrpt.x = -mrpt.x;
            mrpt.y = -mrpt.y;
            break;
        case ROT_90:
            tmp    = mrpt.x;
            mrpt.x = -mrpt.y;
            mrpt.y = tmp;
            break;
    }

    if (scroll_enabled || text_enabled) {
        accumulated_h += mrpt.x;
        accumulated_v -= mrpt.y;

        int shift_x = accumulated_h / scroll_divisor;
        int shift_y = accumulated_v / scroll_divisor;

        accumulated_h -= shift_x * scroll_divisor;
        accumulated_v -= shift_y * scroll_divisor;

        mrpt.x = 0;
        mrpt.y = 0;

        if (scroll_enabled) {
            mrpt.h = shift_x;
            mrpt.v = shift_y;
        } else if (text_enabled) {
            while (shift_x > 0) {
                tap_code(KC_RIGHT);
                shift_x--;
            }
            while (shift_x < 0) {
                tap_code(KC_LEFT);
                shift_x++;
            }
            while (shift_y > 0) {
                tap_code(KC_UP);
                shift_y--;
            }
            while (shift_y < 0) {
                tap_code(KC_DOWN);
                shift_y++;
            }
        }
    }

    return mrpt;
}

void pointing_device_init_kb(void) {
    vial_config.raw = via_get_layout_options();
    update_settings();
}
