#include "display.h"
#include "qp.h"
#include "lvgl_helpers.h"
#include "ergohaven_ruen.h"
#include "hid.h"
#include "ergohaven.h"

static uint16_t home_screen_timer = 0;

static bool display_enabled;

static painter_device_t display;

/* shared styles */
lv_style_t style_screen;
lv_style_t style_container;
lv_style_t style_button;
lv_style_t style_button_active;

/* screens */
static lv_obj_t *screen_home;
static lv_obj_t *screen_volume;
static lv_obj_t *screen_media;

/* home screen content */
static lv_obj_t *label_time;
static lv_obj_t *label_volume_home;
static lv_obj_t *label_shift;
static lv_obj_t *label_ctrl;
static lv_obj_t *label_alt;
static lv_obj_t *label_gui;
static lv_obj_t *label_layer;
static lv_obj_t *label_caps;
static lv_obj_t *label_num;
static lv_obj_t *label_layout;
static lv_obj_t *label_version;

/* volume screen content */
static lv_obj_t *arc_volume;
static lv_obj_t *label_volume_arc;

/* media screen content */
static lv_obj_t *label_media_artist;
static lv_obj_t *label_media_title;

/* public function to be used in keymaps */
// bool is_display_enabled(void) {
//     return display_enabled;
// }

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
    lv_style_set_text_color(&style_button, lv_palette_main(LV_PALETTE_TEAL));

    lv_style_init(&style_button_active);
    lv_style_set_bg_color(&style_button_active, lv_palette_main(LV_PALETTE_TEAL));
    lv_style_set_bg_opa(&style_button_active, LV_OPA_100);
    lv_style_set_text_color(&style_button_active, lv_color_black());
}

void init_screen_home(void) {
    screen_home = lv_scr_act();
    lv_obj_add_style(screen_home, &style_screen, 0);
    use_flex_column(screen_home);

    label_volume_home = lv_label_create(screen_home);
    lv_label_set_text(label_volume_home, "Ergohaven");

    label_time = lv_label_create(screen_home);
    lv_label_set_text(label_time, "K:03 Pro");
    lv_obj_set_style_text_font(label_time, &lv_font_montserrat_48, LV_PART_MAIN);

    lv_obj_t *mods = lv_obj_create(screen_home);
    lv_obj_add_style(mods, &style_container, 0);
    use_flex_row(mods);

    label_gui   = create_button(mods, "GUI", &style_button, &style_button_active);
    label_alt   = create_button(mods, "ALT", &style_button, &style_button_active);
    label_ctrl  = create_button(mods, "CTL", &style_button, &style_button_active);
    label_shift = create_button(mods, "SFT", &style_button, &style_button_active);

    label_caps = create_button(mods, "CAPS", &style_button, &style_button_active);
    label_num = create_button(mods, "NUM", &style_button, &style_button_active);

    lv_obj_t *bottom_row = lv_obj_create(screen_home);
    lv_obj_add_style(bottom_row, &style_container, 0);
    use_flex_row(bottom_row);

    label_layer = lv_label_create(bottom_row);
    lv_label_set_text(label_layer, "");
    lv_obj_align(label_layer, LV_ALIGN_LEFT_MID, 20, 0);
    display_process_layer_state(0);

    label_layout = lv_label_create(bottom_row);
    lv_label_set_text(label_layout, "");
    lv_obj_align(label_layout, LV_ALIGN_RIGHT_MID, -20, 0);

    label_version = lv_label_create(screen_home);
    lv_label_set_text(label_version, EH_VERSION_STR);
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

void init_screen_media(void) {
    screen_media = lv_obj_create(NULL);
    lv_obj_add_style(screen_media, &style_screen, 0);
    use_flex_column(screen_media);
    lv_obj_set_flex_align(screen_media, LV_FLEX_ALIGN_SPACE_EVENLY, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    label_media_artist = lv_label_create(screen_media);
    lv_label_set_text(label_media_artist, "N/A");
    lv_label_set_long_mode(label_media_artist, LV_LABEL_LONG_WRAP);
    lv_obj_set_width(label_media_artist, lv_pct(90));
    lv_obj_set_style_text_align(label_media_artist, LV_TEXT_ALIGN_CENTER, 0);

    label_media_title = lv_label_create(screen_media);
    lv_label_set_text(label_media_title, "N/A");
    lv_label_set_long_mode(label_media_title, LV_LABEL_LONG_WRAP);
    lv_obj_set_width(label_media_title, lv_pct(90));
    lv_obj_set_style_text_align(label_media_title, LV_TEXT_ALIGN_CENTER, 0);
}

bool display_init_kb(void) {
    display_enabled = false;
    dprint("display_init_kb - start\n");

    gpio_set_pin_output(GP17);
    gpio_write_pin_high(GP17);

    display = qp_st7789_make_spi_device(240, 280, LCD_CS_PIN, LCD_DC_PIN, LCD_RST_PIN, 16, 3);
    qp_set_viewport_offsets(display, 0, 20);

    if (!qp_init(display, QP_ROTATION_180) || !qp_power(display, true) || !qp_lvgl_attach(display)) return display_enabled;

    dprint("display_init_kb - initialised\n");

    lv_disp_t  *lv_display = lv_disp_get_default();
    lv_theme_t *lv_theme   = lv_theme_default_init(lv_display, lv_palette_main(LV_PALETTE_TEAL), lv_palette_main(LV_PALETTE_BLUE), true, LV_FONT_DEFAULT);
    lv_disp_set_theme(lv_display, lv_theme);
    init_styles();

    init_screen_home();
    init_screen_volume();
    init_screen_media();
    display_enabled = true;

    return display_enabled;
}

void set_layout_label(uint8_t layout) {
    switch (layout) {
        case LANG_EN:
            lv_label_set_text(label_layout, "EN");
            break;

        case LANG_RU:
            lv_label_set_text(label_layout, "RU");
            break;
    }
}

void start_home_screen_timer(void) {
    dprint("start_home_screen_timer\n");
    home_screen_timer = timer_read();
}

void display_process_hid_data(struct hid_data_t *hid_data) {
    dprintf("display_process_hid_data");
    if (hid_data->time_changed) {
        lv_label_set_text_fmt(label_time, "%02d:%02d", hid_data->hours, hid_data->minutes);
        hid_data->time_changed = false;
    }
    if (hid_data->volume_changed) {
        lv_label_set_text_fmt(label_volume_home, "Vol: %02d%%", hid_data->volume);
        lv_label_set_text_fmt(label_volume_arc, "%02d", hid_data->volume);
        lv_arc_set_value(arc_volume, hid_data->volume);
        lv_scr_load(screen_volume);
        start_home_screen_timer();
        hid_data->volume_changed = false;
    }
    if (hid_data->media_artist_changed) {
        lv_label_set_text(label_media_artist, hid_data->media_artist);
        lv_scr_load(screen_media);
        start_home_screen_timer();
        hid_data->media_artist_changed = false;
    }
    if (hid_data->media_title_changed) {
        lv_label_set_text(label_media_title, hid_data->media_title);
        lv_scr_load(screen_media);
        start_home_screen_timer();
        hid_data->media_title_changed = false;
    }
}

void display_process_layer_state(uint8_t layer) {
    lv_label_set_text(label_layer, layer_upper_name(layer));
}

void display_housekeeping_task(void) {
    if (home_screen_timer && timer_elapsed(home_screen_timer) > 5000) {
        home_screen_timer = 0;
        lv_scr_load(screen_home);
    }

    if (last_input_activity_elapsed() > EH_TIMEOUT ) {
        rgblight_suspend();
        gpio_write_pin_low(GP17);
        qp_power(display, false);
        return;
    } else {
        rgblight_wakeup();
        gpio_write_pin_high(GP17);
        qp_power(display, true);
    }

    toggle_state(label_shift, LV_STATE_PRESSED, (get_mods() | get_oneshot_mods()) & MOD_MASK_SHIFT);
    toggle_state(label_ctrl, LV_STATE_PRESSED, (get_mods() | get_oneshot_mods()) & MOD_MASK_CTRL);
    toggle_state(label_alt, LV_STATE_PRESSED, (get_mods() | get_oneshot_mods()) & MOD_MASK_ALT);
    toggle_state(label_gui, LV_STATE_PRESSED, (get_mods() | get_oneshot_mods()) & MOD_MASK_GUI);
    struct hid_data_t *hid_data = get_hid_data();
    display_process_hid_data(hid_data);
    set_layout_label(get_cur_lang());
}

bool led_update_kb(led_t led_state) {
    bool res = led_update_user(led_state);
    if (res && is_display_enabled()) {
        toggle_state(label_caps, LV_STATE_PRESSED, led_state.caps_lock);
        toggle_state(label_num, LV_STATE_PRESSED, led_state.num_lock);
    }

    return res;
}
