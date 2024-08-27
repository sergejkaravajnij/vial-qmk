#include "qp_lvgl.h"


bool is_display_enabled(void);
bool display_init_kb(void);
void display_housekeeping_task(void);
void display_process_layer_state(uint8_t layer);
void display_turn_on(void);
void display_turn_off(void);
