//import * as Asyncify from './asyncify.js';

//window.Asyncify = Asyncify;

window.btn = {
    up: false,
    down: false,
    left: false,
    right: false,
    fire: false
}

window.addEventListener('keydown', function (event) {
    switch (event.code) {
        case "ArrowLeft":
            btn.left = true;
            break;
        case "ArrowRight":
            btn.right = true;
            break;
        case "ArrowUp":
            btn.up = true;
            break;
        case "ArrowDown":
            btn.down = true;
            break;
        case "Space":
            btn.fire = true;
            break;
    }
});

window.addEventListener('keyup', function (event) {
    switch (event.code) {
        case "ArrowLeft":
            btn.left = false;
            break;
        case "ArrowRight":
            btn.right = false;
            break;
        case "ArrowUp":
            btn.up = false;
            break;
        case "ArrowDown":
            btn.down = false;
            break;
        case "Space":
            btn.fire = false;
            break;
    }
});