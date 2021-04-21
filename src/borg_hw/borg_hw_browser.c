#include "../config.h"
#include "borg_hw.h"

#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <emscripten.h>

#include "../pixel.h"

char buf[512];
char js_pixmap[NUM_ROWS][NUM_COLS][4];

unsigned char pixmap[NUMPLANE][NUM_ROWS][LINEBYTES];

//unsigned char pixvalue[4][3] = { {53, 36, 30}, {100, 29, 13}, {100, 55, 42}, {100, 82, 69} };
unsigned char pixvalue[4][3] = { {35, 40, 36}, {100, 29, 13}, {100, 55, 42}, {98, 26, 10} };

void watchdog_enable() {}
void borg_hw_init() {
    EM_ASM({
        window.canvas_ctx = Module.canvas.getContext('2d', {
            alpha: true,
            antialias: false,
            depth: false
        });

        window.data_buf = HEAPU8.subarray($0, $0 + $1);
    }, js_pixmap, sizeof(js_pixmap));

    for (int x = 0; x < NUM_ROWS; x++) {
        for (int y = 0; y < NUM_COLS; y++) {
            js_pixmap[x][y][0] = 0xff;
        }
    }
}
void timer0_off() {}
uint8_t eeprom_read_byte(const uint8_t *p) {
    return 0xff;
};
void eeprom_write_byte(uint8_t *p, uint8_t value) {};

uint16_t eeprom_read_word(uint16_t *p) { return 0xffff; };

void eeprom_write_word(uint16_t *p, uint16_t value) {};

void update_screen() {
    int factor = (256/NUMPLANE) - 1;

    for (int x = 0; x < NUM_COLS; x++) {
        for (int y = 0; y < NUM_ROWS; y++) {
            pixel px = { x, y };
            unsigned char value = get_pixel_value(px) * factor;
            js_pixmap[y][NUM_COLS - ( x + 1)][3] = value;
            /*
            char *value = pixvalue[get_pixel_value(px)];
            js_pixmap[y][NUM_COLS - (x + 1)][0] = value[0]; 
            js_pixmap[y][NUM_COLS - (x + 1)][1] = value[1]; 
            js_pixmap[y][NUM_COLS - (x + 1)][2] = value[2]; 
            */
        }
    }

    EM_ASM({
        const img = new ImageData(new Uint8ClampedArray(window.data_buf), $0, $1);
        window.requestAnimationFrame((_timestamp) => {
            window.canvas_ctx.putImageData(img, 0, 0);
        });
    }, NUM_ROWS, NUM_COLS);
}
