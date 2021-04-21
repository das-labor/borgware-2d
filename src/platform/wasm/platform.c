#include <emscripten.h>
#include "../../borg_hw/borg_hw_browser.h"

void sleep_before(int ms) {
    emscripten_sleep(ms);
    update_screen();
}

void sleep_step() {}