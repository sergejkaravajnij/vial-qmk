# MCU name
MCU = RP2040
BOARD = GENERIC_RP_RP2040
BOOTLOADER = rp2040
ALLOW_WARNINGS = yes
PICO_INTRINSICS_ENABLED = no

QUANTUM_PAINTER_ENABLE = yes
QUANTUM_PAINTER_DRIVERS += st7735_spi
QUANTUM_PAINTER_LVGL_INTEGRATION = yes

LTO_ENABLE = no
VIA_ENABLE = yes
TAP_DANCE_ENABLE = yes
COMBO_ENABLE = yes
KEY_OVERRIDE_ENABLE = yes
DYNAMIC_MACRO_ENABLE = yes
OLED_ENABLE = yes
CAPS_WORD_ENABLE = yes
