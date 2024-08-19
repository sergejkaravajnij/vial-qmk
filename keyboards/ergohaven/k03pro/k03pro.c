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
