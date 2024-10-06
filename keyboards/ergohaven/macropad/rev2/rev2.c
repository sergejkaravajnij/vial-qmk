#include "display.h"
#include "ergohaven.h"

void housekeeping_task_user(void) {
    display_housekeeping_task();
}

void keyboard_post_init_user(void) {
    display_init_kb();
}


layer_state_t default_layer_state_set_user(layer_state_t state) {
    display_process_layer_state(get_highest_layer(layer_state | state));
    return state;
}

layer_state_t layer_state_set_user(layer_state_t state) {
    display_process_layer_state(get_highest_layer(state | default_layer_state));
    return state;
}
