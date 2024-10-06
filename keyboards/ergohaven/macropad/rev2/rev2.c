#include "display.h"
#include "ergohaven.h"

void housekeeping_task_user(void) {
    display_housekeeping_task();
}

void keyboard_post_init_user(void) {
    display_init_kb();
}

layer_state_t layer_state_set_user(layer_state_t state) {
    display_process_layer_state(get_current_layer());
    return state;
}
