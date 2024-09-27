#include "display.h"
#include "qp.h"
#include "lvgl_helpers.h"
#include "ergohaven_ruen.h"
#include "ergohaven_keycode_str.h"
#include "hid.h"
#include "ergohaven.h"
#include "ergohaven_symbols.h"

LV_FONT_DECLARE(ergohaven_symbols_20);
LV_FONT_DECLARE(ergohaven_symbols_28);
LV_IMG_DECLARE(ergohaven_logo);

static uint32_t screen_timer = 0;

static bool display_enabled = false;
static bool is_display_on   = false;

typedef enum {
    SCREEN_OFF = -1,
    SCREEN_SPLASH,
    SCREEN_LAYOUT,
    SCREEN_VOLUME,
    SCREEN_HID,
} screen_t;

static screen_t screen_state        = SCREEN_OFF;
static screen_t change_screen_state = SCREEN_OFF;

static painter_device_t display;

/* shared styles */
lv_style_t style_screen;
lv_style_t style_container;
lv_style_t style_button;
lv_style_t style_button_active;

/* screens */
static lv_obj_t *screen_splash;
static lv_obj_t *screen_hid;
static lv_obj_t *screen_layout;
static lv_obj_t *screen_volume;
static lv_obj_t *label_version;

/* hid screen content */
static lv_obj_t *label_time;
static lv_obj_t *label_layer;
static lv_obj_t *label_media_artist;
static lv_obj_t *label_media_title;
static lv_obj_t *label_layer;
static lv_obj_t *label_layout;

/* layout screen content */
static lv_obj_t *key_labels[15];
static lv_obj_t *label_layer_small;

/* volume screen content */
static lv_obj_t *arc_volume;
static lv_obj_t *label_volume_arc;

static lv_color_t accent_color_red;
static lv_color_t accent_color_blue;

/* public function to be used in keymaps */
bool is_display_enabled(void) {
    return display_enabled;
}

void init_styles(void) {
    lv_style_init(&style_screen);
    lv_style_set_bg_color(&style_screen, lv_color_black());

    lv_style_init(&style_container);
    lv_style_set_pad_top(&style_container, 0);
    lv_style_set_pad_bottom(&style_container, 0);
    lv_style_set_pad_left(&style_container, 0);
    lv_style_set_pad_right(&style_container, 0);
    lv_style_set_bg_opa(&style_container, 0);
    lv_style_set_border_width(&style_container, 0);
    lv_style_set_width(&style_container, lv_pct(100));
    lv_style_set_height(&style_container, LV_SIZE_CONTENT);

    lv_style_init(&style_button);
    lv_style_set_pad_top(&style_button, 4);
    lv_style_set_pad_bottom(&style_button, 4);
    lv_style_set_pad_left(&style_button, 4);
    lv_style_set_pad_right(&style_button, 4);
    lv_style_set_radius(&style_button, 6);
    lv_style_set_text_color(&style_button, accent_color_blue);

    lv_style_init(&style_button_active);
    lv_style_set_bg_color(&style_button_active, accent_color_blue);
    lv_style_set_bg_opa(&style_button_active, LV_OPA_100);
    lv_style_set_text_color(&style_button_active, lv_color_black());
}

void init_screen_layout(void) {
    screen_layout = lv_obj_create(NULL);
    lv_obj_add_style(screen_layout, &style_screen, 0);
    use_flex_column(screen_layout);
    lv_obj_set_scrollbar_mode(screen_layout, LV_SCROLLBAR_MODE_OFF);

    label_layer_small = lv_label_create(screen_layout);
    lv_label_set_text(label_layer_small, "layer");
    lv_obj_set_style_pad_top(label_layer_small, 25, 0);
    lv_obj_set_style_pad_bottom(label_layer_small, 25, 0);
    lv_obj_set_style_text_color(label_layer_small, accent_color_blue, 0);
    lv_obj_set_style_text_font(label_layer_small, &ergohaven_symbols_28, LV_PART_MAIN);

    lv_obj_t *cont = lv_obj_create(screen_layout);
    lv_obj_set_size(cont, 232, 250);
    // lv_obj_center(cont);
    lv_obj_set_flex_flow(cont, LV_FLEX_FLOW_ROW_WRAP);
    int32_t v = 0;
    lv_obj_set_style_pad_row(cont, v, 0);
    lv_obj_set_style_pad_column(cont, v, 0);
    lv_obj_add_style(cont, &style_container, 0);
    lv_obj_set_scrollbar_mode(cont, LV_SCROLLBAR_MODE_OFF);

    uint32_t i;
    for (i = 0; i < 15; i++) {
        if (i == 12) {
            lv_obj_t *obj = lv_obj_create(cont);
            lv_obj_set_size(obj, 231, 10);
            lv_obj_set_scrollbar_mode(obj, LV_SCROLLBAR_MODE_OFF);
            lv_obj_add_style(obj, &style_screen, 0);
            lv_obj_set_style_border_opa(obj, 0, 0);
        }
        lv_obj_t *obj = lv_obj_create(cont);
        lv_obj_set_size(obj, 77, 45);
        lv_obj_set_scrollbar_mode(obj, LV_SCROLLBAR_MODE_OFF);
        lv_obj_add_style(obj, &style_screen, 0);
        lv_obj_set_style_border_width(obj, 1, 0);

        key_labels[i] = lv_label_create(obj);
        lv_obj_center(key_labels[i]);
        lv_obj_set_style_text_font(key_labels[i], &ergohaven_symbols_20, LV_PART_MAIN);
        lv_obj_set_style_text_align(key_labels[i], LV_TEXT_ALIGN_CENTER, 0);

        if (i >= 12) {
            lv_obj_set_style_border_opa(obj, 0, 0);
            lv_obj_set_style_text_color(key_labels[i], accent_color_blue, 0);
        }
    }
}

void init_screen_splash(void) {
    screen_splash = lv_obj_create(NULL);
    lv_obj_add_style(screen_splash, &style_screen, 0);
    use_flex_column(screen_splash);

    lv_obj_t *img = lv_img_create(screen_splash);
    lv_img_set_src(img, &ergohaven_logo);
    lv_obj_align(img, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_pad_top(img, 60, 0);
    lv_obj_set_style_pad_bottom(img, 60, 0);

    label_version = lv_label_create(screen_splash);
    lv_label_set_text(label_version, "v" EH_VERSION_STR);
}

void init_screen_hid(void) {
    screen_hid = lv_obj_create(NULL);
    lv_obj_add_style(screen_hid, &style_screen, 0);
    lv_obj_set_layout(screen_hid, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(screen_hid, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(screen_hid, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_scrollbar_mode(screen_hid, LV_SCROLLBAR_MODE_OFF);

    label_time = lv_label_create(screen_hid);
    lv_label_set_text(label_time, "HH:MM");
    lv_obj_set_style_text_font(label_time, &lv_font_montserrat_48, LV_PART_MAIN);
    lv_obj_set_style_pad_top(label_time, 40, 0);
    lv_obj_set_style_pad_bottom(label_time, 20, 0);

    lv_obj_t *bottom_row = lv_obj_create(screen_hid);
    lv_obj_add_style(bottom_row, &style_container, 0);
    use_flex_row(bottom_row);

    label_layer = lv_label_create(bottom_row);
    lv_label_set_text(label_layer, "");
    lv_obj_set_size(label_layer, 110, 20);
    lv_obj_set_style_text_align(label_layer, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_style_text_font(label_layer, &ergohaven_symbols_20, LV_PART_MAIN);

    label_layout = lv_label_create(bottom_row);
    lv_label_set_text(label_layout, "");
    lv_obj_set_size(label_layout, 110, 20);
    lv_obj_set_style_text_align(label_layout, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_style_text_font(label_layout, &ergohaven_symbols_20, LV_PART_MAIN);

    label_media_title = lv_label_create(screen_hid);
    lv_label_set_text(label_media_title, "");
    lv_label_set_long_mode(label_media_title, LV_LABEL_LONG_WRAP);
    lv_obj_set_width(label_media_title, lv_pct(95));
    lv_obj_set_style_text_align(label_media_title, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_style_text_font(label_media_title, &ergohaven_symbols_28, LV_PART_MAIN);
    lv_obj_set_style_pad_top(label_media_title, 10, 0);
    lv_obj_set_style_pad_bottom(label_media_title, 0, 0);

    label_media_artist = lv_label_create(screen_hid);
    lv_label_set_text(label_media_artist, "");
    lv_label_set_long_mode(label_media_artist, LV_LABEL_LONG_WRAP);
    lv_obj_set_width(label_media_artist, lv_pct(95));
    lv_obj_set_style_text_align(label_media_artist, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_style_text_font(label_media_artist, &ergohaven_symbols_20, LV_PART_MAIN);

    lv_obj_set_style_pad_top(label_media_artist, 0, 0);
    lv_obj_set_style_pad_bottom(label_media_artist, 20, 0);
    lv_obj_set_style_text_color(label_media_artist, accent_color_blue, 0);
}

void init_screen_volume(void) {
    screen_volume = lv_obj_create(NULL);
    lv_obj_add_style(screen_volume, &style_screen, 0);

    arc_volume = lv_arc_create(screen_volume);
    lv_obj_set_size(arc_volume, 200, 200);
    lv_obj_center(arc_volume);

    label_volume_arc = lv_label_create(screen_volume);
    lv_label_set_text(label_volume_arc, "00");
    lv_obj_set_style_text_font(label_volume_arc, &lv_font_montserrat_48, LV_PART_MAIN);
    lv_obj_center(label_volume_arc);

    lv_obj_t *volume_text_label = lv_label_create(screen_volume);
    lv_label_set_text(volume_text_label, "Volume");
    lv_obj_align(volume_text_label, LV_ALIGN_BOTTOM_MID, 0, -50);
}

bool display_init_kb(void) {
    display_enabled = false;
    dprint("display_init_kb - start\n");

    gpio_set_pin_output(GP17);

    display = qp_st7789_make_spi_device(240, 280, LCD_CS_PIN, LCD_DC_PIN, LCD_RST_PIN, 16, 3);
    qp_set_viewport_offsets(display, 0, 20);

    accent_color_red  = lv_color_make(248, 83, 107);
    accent_color_blue = lv_color_make(84, 189, 191);

    if (!qp_init(display, QP_ROTATION_180) || !qp_lvgl_attach(display)) return display_enabled;

    dprint("display_init_kb - initialised\n");

    lv_disp_t  *lv_display = lv_disp_get_default();
    lv_theme_t *lv_theme   = lv_theme_default_init(lv_display, accent_color_blue, accent_color_red, true, LV_FONT_DEFAULT);
    lv_disp_set_theme(lv_display, lv_theme);
    init_styles();

    init_screen_splash();
    init_screen_layout();
    init_screen_hid();
    init_screen_volume();
    display_enabled = true;

    display_process_layer_state(layer_state);
    change_screen_state = SCREEN_SPLASH;

    return display_enabled;
}

bool display_process_hid_data(struct hid_data_t *hid_data) {
    static uint32_t hid_sync_time = 0;
    bool            new_hid_data  = false;

    dprintf("display_process_hid_data");
    if (hid_data->time_changed) {
        lv_label_set_text_fmt(label_time, "%02d:%02d", hid_data->hours, hid_data->minutes);
        hid_data->time_changed = false;
        new_hid_data           = true;
    }
    if (hid_data->volume_changed) {
        lv_label_set_text_fmt(label_volume_arc, "%02d", hid_data->volume);
        lv_arc_set_value(arc_volume, hid_data->volume);
        change_screen_state      = SCREEN_VOLUME;
        hid_data->volume_changed = false;
        new_hid_data             = true;
        screen_timer             = timer_read32();
    }
    if (hid_data->media_artist_changed) {
        lv_label_set_text(label_media_artist, hid_data->media_artist);
        change_screen_state            = SCREEN_HID;
        hid_data->media_artist_changed = false;
        new_hid_data                   = true;
    }
    if (hid_data->media_title_changed) {
        lv_label_set_text(label_media_title, hid_data->media_title);
        change_screen_state           = SCREEN_HID;
        hid_data->media_title_changed = false;
        new_hid_data                  = true;
    }
    if (new_hid_data) hid_sync_time = timer_read32();
    return (hid_sync_time != 0) && timer_elapsed32(hid_sync_time) < 61 * 1000;
}

void set_layout_label(uint8_t layout) {
    switch (layout) {
        case LANG_EN:
            lv_label_set_text(label_layout, EH_SYMBOL_GLOBE " EN");
            break;

        case LANG_RU:
            lv_label_set_text(label_layout, EH_SYMBOL_GLOBE " RU");
            break;
    }
}

static const char *PROGMEM LAYER_NAME[] = {
    "ZERO", "ONE", "TWO", "THREE", "FOUR", "FIVE", "SIX", "SEVEN", "EIGHT", "NINE", "TEN", "ELEVEN", "TWELVE", "THIRTEEN", "FOURTEEN", "FIFTEEN",
};

const char *get_layer_name(uint8_t layer) {
    if (layer < 16) {
        return LAYER_NAME[layer];
    } else {
        return "UNDEFINED";
    }
}

uint16_t get_keycode(int layer, int row, int col) {
    uint16_t keycode = dynamic_keymap_get_keycode(layer, row, col);
    if (keycode == KC_TRANSPARENT) keycode = dynamic_keymap_get_keycode(0, row, col);
    return keycode;
}

uint16_t get_encoder_keycode(int layer, int encoder, bool clockwise) {
    uint16_t keycode = dynamic_keymap_get_encoder(layer, encoder, clockwise);
    if (keycode == KC_TRANSPARENT) keycode = dynamic_keymap_get_encoder(0, encoder, clockwise);
    return keycode;
}

static int update_layer_index = 0;

void display_process_layer_state(uint8_t layer) {
    if (!display_enabled) return;

    change_screen_state = SCREEN_LAYOUT;

    const char *layer_name = get_layer_name(layer);
    char        buf[32];
    sprintf(buf, EH_SYMBOL_LAYER " %s", layer_name);
    lv_label_set_text(label_layer_small, buf);
    lv_label_set_text(label_layer, buf);

    update_layer_index = 0;
}

void update_layer_task(void) {
    if (update_layer_index >= 15) return;
    uint8_t  layer   = get_highest_layer(layer_state | default_layer_state);
    uint16_t keycode = KC_TRANSPARENT;
    if (update_layer_index < 12)
        keycode = get_keycode(layer, update_layer_index / 3 + 1, update_layer_index % 3);
    else if (update_layer_index == 13)
        keycode = get_keycode(layer, 0, 2);
    else if (update_layer_index == 12)
        keycode = get_encoder_keycode(layer, 0, false);
    else if (update_layer_index == 14)
        keycode = get_encoder_keycode(layer, 0, true);
    lv_label_set_text(key_labels[update_layer_index], keycode_to_str(keycode));
    update_layer_index += 1;
}

void update_screen_state(void) {
    screen_timer = timer_read32();
    screen_state = change_screen_state;
    switch (screen_state) {
        case SCREEN_SPLASH:
            lv_scr_load(screen_splash);
            display_turn_on();
            break;
        case SCREEN_HID:
            lv_scr_load(screen_hid);
            display_turn_on();
            break;
        case SCREEN_LAYOUT:
            lv_scr_load(screen_layout);
            display_turn_on();
            break;
        case SCREEN_VOLUME:
            lv_scr_load(screen_volume);
            display_turn_on();
            break;
        case SCREEN_OFF:
            display_turn_off();
            break;
    }
}

void display_housekeeping_task(void) {
    if (!display_enabled) return;

    update_layer_task();

    struct hid_data_t *hid_data   = get_hid_data();
    bool               hid_active = display_process_hid_data(hid_data);
    static uint8_t     prev_lang  = 0;
    uint8_t            cur_lang   = get_cur_lang();
    set_layout_label(cur_lang);
    if (prev_lang != cur_lang) {
        change_screen_state = SCREEN_HID;
        prev_lang           = cur_lang;
    }

    if (screen_state == change_screen_state) {
        uint32_t screen_elapsed   = timer_elapsed32(screen_timer);
        uint32_t activity_elapsed = last_input_activity_elapsed();

        switch (screen_state) {
            case SCREEN_SPLASH:
                if (screen_elapsed > 2 * 1000) {
                    change_screen_state = SCREEN_LAYOUT;
                }
                break;

            case SCREEN_LAYOUT:
                if (hid_active && activity_elapsed > 10 * 1000) {
                    change_screen_state = SCREEN_HID;
                } else if (activity_elapsed > EH_TIMEOUT) {
                    change_screen_state = SCREEN_OFF;
                }
                break;

            case SCREEN_HID:
                if (!hid_active) {
                    change_screen_state = SCREEN_LAYOUT;
                } else if (activity_elapsed > EH_TIMEOUT && screen_elapsed > 10 * 1000) {
                    change_screen_state = SCREEN_OFF;
                }
                break;

            case SCREEN_VOLUME:
                if (screen_elapsed > 2 * 1000) {
                    change_screen_state = SCREEN_HID;
                }
                break;

            case SCREEN_OFF:
                if (activity_elapsed < EH_TIMEOUT) {
                    change_screen_state = SCREEN_LAYOUT;
                }
                break;
        }
    }
    if (change_screen_state != screen_state) {
        update_screen_state();
        return;
    }
}

void display_turn_on(void) {
    if (!is_display_on) {
        gpio_write_pin_high(GP17);
        qp_power(display, true);
        is_display_on = true;
    }
}

void display_turn_off(void) {
    if (is_display_on) {
        is_display_on = false;
        qp_power(display, false);
        gpio_write_pin_low(GP17);
    }
}

void suspend_power_down_kb(void) {
    display_turn_off();
    suspend_power_down_user();
}

void suspend_wakeup_init_kb(void) {
    display_turn_on();
    suspend_wakeup_init_user();
}
