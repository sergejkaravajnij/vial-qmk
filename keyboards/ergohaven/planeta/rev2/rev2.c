#include "display.h"


void housekeeping_task_user(void) {
    if (is_display_enabled()) {
        display_housekeeping_task();
    }
}

void keyboard_post_init_user(void) {
    display_init_kb();
}

layer_state_t layer_state_set_user(layer_state_t state) {
    if (is_display_enabled()) {
        display_process_layer_state(get_current_layer());
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
