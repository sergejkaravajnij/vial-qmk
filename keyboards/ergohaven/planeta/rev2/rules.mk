SERIAL_DRIVER = vendor

QUANTUM_PAINTER_ENABLE = yes
QUANTUM_PAINTER_DRIVERS += st7789_spi
QUANTUM_PAINTER_LVGL_INTEGRATION = yes
RGBLIGHT_ENABLE = yes

RAW_ENABLE = yes

SRC += keyboards/ergohaven/lvgl_helpers.c
SRC += display.c
SRC += keyboards/ergohaven/ergohaven_rgb.c
SRC += keyboards/ergohaven/ergohaven_display.c
SRC += keyboards/ergohaven/ergohaven_logo.c
SRC += keyboards/ergohaven/ergohaven_symbols_20.c
SRC += keyboards/ergohaven/ergohaven_symbols_28.c
