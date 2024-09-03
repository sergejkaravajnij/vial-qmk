#include "display.h"
#include "transactions.h"
#include "ergohaven_ruen.h"
#include "ergohaven_rgb.h"

typedef union {
    uint32_t raw;
    struct {
        // layout options
        uint8_t scroll_divisor : 3;
        bool    invert_scroll : 1;
        uint8_t dpi_mode : 3;
        // other options
        uint8_t lang : 1;
        bool    mac : 1;
        bool    caps_word : 1;
    };
} vial_config_t;

vial_config_t vial_config;

uint8_t get_lang(void) {
    return is_keyboard_master() ? get_cur_lang() : vial_config.lang;
}

uint8_t get_mac(void) {
    return is_keyboard_master() ? keymap_config.swap_lctl_lgui : vial_config.mac;
}

uint8_t get_caps_word(void) {
    return is_keyboard_master() ? is_caps_word_on() : vial_config.caps_word;
}

static const uint16_t DPI_TABLE[] = {320, 400, 500, 630, 800, 1000};

uint16_t get_dpi(uint8_t dpi_mode) {
    if (dpi_mode < ARRAY_SIZE(DPI_TABLE))
        return DPI_TABLE[dpi_mode];
    else
        return 400;
}

static int32_t scroll_divisor_x = 8;
static int32_t scroll_divisor_y = 8;

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

void via_set_layout_options_kb(uint32_t value) {
    vial_config.raw = value;
    uint16_t dpi    = get_dpi(vial_config.dpi_mode);
    if (pointing_device_get_cpi() != dpi) {
        pointing_device_set_cpi(dpi);
    }
    scroll_divisor_x = scroll_divisor_y = get_scroll_div(vial_config.scroll_divisor);
}

void sync_config(uint8_t in_buflen, const void* in_data, uint8_t out_buflen, void* out_data) {
    uint32_t value;
    memcpy(&value, in_data, sizeof(uint32_t));
    via_set_layout_options_kb(value);
}

bool is_display_side(void) {
#ifdef K03_DISPLAY_RIGHT
    return !is_keyboard_left();
#endif
#ifdef K03_DISPLAY_LEFT
    return is_keyboard_left();
#endif
    return false;
}

static bool lights_off = false;

void lights_wakeup(void) {
    if (lights_off) {
        lights_off = false;
        rgblight_wakeup();
    }
}

void lights_suspend(void) {
    if (!lights_off) {
        lights_off = true;
        rgblight_suspend();
    }
}

void housekeeping_task_user(void) {
    if (is_display_enabled() && is_display_side()) {
        display_housekeeping_task();
    }

    uint32_t elapsed = last_input_activity_elapsed();
    if (elapsed > __UINT32_MAX__ - 1000) // possible overflow
        elapsed = 0;

    if (elapsed > EH_TIMEOUT) {
        lights_suspend();
        if (is_display_side()) display_turn_off();
        return;
    } else {
        lights_wakeup();
        if (is_display_side()) display_turn_on();
    }

    if (is_keyboard_master()) {
        // Interact with slave every 500ms
        static uint32_t last_sync          = 0;
        static uint32_t last_synced_config = 0;
        if (timer_elapsed32(last_sync) > 500) {
            vial_config.lang      = get_lang();
            vial_config.mac       = get_mac();
            vial_config.caps_word = get_caps_word();
            if (last_synced_config != vial_config.raw) {
                if (transaction_rpc_send(RPC_SYNC_CONFIG, sizeof(vial_config_t), &vial_config)) {
                    last_sync          = timer_read32();
                    last_synced_config = vial_config.raw;
                }
            }
        }
    }
    layer_state_set_rgb(layer_state);
}

report_mouse_t pointing_device_task_user(report_mouse_t mouse_report) {
    static int32_t scroll_accumulated_h = 0;
    static int32_t scroll_accumulated_v = 0;

    if (mouse_report.h != 0 || mouse_report.v != 0) {
        if (vial_config.invert_scroll) {
            scroll_accumulated_h -= mouse_report.h;
            scroll_accumulated_v += mouse_report.v;
        } else {
            scroll_accumulated_h += mouse_report.h;
            scroll_accumulated_v -= mouse_report.v;
        }

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
    if (is_display_side()) {
        display_init_kb();
    }

    vial_config.raw = via_get_layout_options();
    via_set_layout_options_kb(vial_config.raw);

    transaction_register_rpc(RPC_SYNC_CONFIG, sync_config);
}
