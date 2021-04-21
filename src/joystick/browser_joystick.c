#include "joystick.h"

#include <emscripten.h>
#include <stdio.h>

void joy_init() {}

int joy_button_pressed(enum JoystickButton button) {
    //printf("joy_button_pressed(%d)\n", button);
    return EM_ASM_INT({
        let btn_name = null;
        switch ($0) {
            case 0:
                btn_name = "up";
                break;
            case 1:
                btn_name = "down";
                break;
            case 2:
                btn_name = "left";
                break;
            case 3:
                btn_name = "right";
                break;
            case 4:
                btn_name = "fire";
                break;
        }

        if (btn_name === null) {
            return 0;
        }

        return btn[btn_name] ? 1 : 0;
    }, button);
}