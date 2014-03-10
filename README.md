Borgware-2D
===========

Firmware for AVR based two-dimensional LED matrices, especially the
[Blinken Borgs](http://www.das-labor.org/wiki/Blinken_Borgs) from
[Das LABOR](http://das-labor.org/index.en.php).
Main platform is the [Borg16](http://www.das-labor.org/wiki/Borg16) construction
kit. Other supported platforms are the
[LED Brett](http://www.hackerspace-ffm.de/wiki/index.php?title=LedBrett)
projector from [Hackerspace FFM](http://www.hackerspace-ffm.de) or the 
[ELO Ping-Pong Board](http://www.elo-web.de/elo/mikrocontroller-und-programmierung/ping-pong/das-franzis-pingpong).

![Small Borg16](/doc/img/Borg16-small.jpg)
![Glow Lamp Borg](/doc/img/Glow_Lamp_Borg.jpg)
 
Animations
----------

![Matrix](/doc/img/anim-matrix.png)
![Fire](/doc/img/anim-feuer.png)
![Scrolling Text](/doc/img/anim-scroll.png)

[Animated GIF (3.5 MB)](/doc/img/borg_anim.gif)

Games
-----

![Snake](/doc/img/game-snake.png)
![Tetris](/doc/img/game-tetris.png)

* Tetris
 * Classic: Standard Tetris Clone
 * First Person Tetris: Rotate the bucket instead of the Tetromino.
 * Bastet: Dices the worst the possible Tetromino the whole time.
* Snake
* Breakout
* Space Invaders

Build
=====

Supported build platforms are Linux, FreeBSD and Windows (via Cygwin). Due to
customized linker scripts, simulator support is currently limited to x86 and
x86_64 archs. Following dependencies have to be met:

Linux / FreeBSD
---------------

Package names are based on Debian/Ubuntu repositories. Please adapt the names
according to your Linux distribution (or FreeBSD for that matter).

* build-essential (pulls in an ordinary gcc build tool chain for the host)
* make (gmake on FreeBSD)
* libncurses5-dev
* gcc-avr
* avr-libc
* binutils-avr
* avrdude
* freeglut3-dev

Windows
-------

* [WinAVR](http://winavr.sourceforge.net) (includes avr-gcc and avrdude)
* [Cygwin(64)](http://www.cygwin.com/)
  * make
  * gcc-core
  * libncurses-devel (Cygwin)
  * libncursesw-devel (Cygwin64)
* [libusb-win32](http://sourceforge.net/apps/trac/libusb-win32/wiki) if you want
  to use your USBasp programmer device with avrdude on Windows

Configure
---------

Open a (Cygwin) terminal, change to your checkout directory and type:
 > make menuconfig

This starts a curses based text interface for configuring certain aspects of
your target platform. Be careful if you use an IDE like Eclipse to manage the
build, as integrated terminal emulators tend to choke on curses generated shell
output. Make sure that 'make menuconfig' has been run at least once in an
ordinary terminal emulator after a fresh checkout or after issuing 'make
mrproper'.

Compile
-------

To build for the actual target platform, just type:
 > make 

If you want to test and debug your code within a GUI application, you can use
the simulator:
 > make simulator

In case you build on FreeBSD, just use 'gmake' instead of 'make'.

You can start the simulator by typing ./borgsim(.exe)

Simulator Handling
------------------

Please keep in mind that the simulator is NOT an emulator. All it does is
compile the source to a native host application so you can step through your
C-Code. The GUI thread reads the simulated frame buffer every 40ms and draws its
contents.

Joystick directions are simulated by the WASD keys and SPACE acts as the fire
button. The OpenGL based simulator (Linux/FreeBSD) enables you to adjust the
viewing angle of the LED matrix via the arrow keys (not available on Windows).

