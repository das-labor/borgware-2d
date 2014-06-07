Borgware-2D
===========

Firmware for AVR based two-dimensional LED matrices, especially the
[Blinken Borgs](http://www.das-labor.org/wiki/Blinken_Borgs) from
[Das LABOR](http://das-labor.org/index.en.php).
Main platform is the [Borg16](http://www.das-labor.org/wiki/Borg16) construction
kit. Other supported platforms are the
[LED Brett](http://www.hackerspace-ffm.de/wiki/index.php?title=LedBrett)
projector from [Hackerspace FFM](http://www.hackerspace-ffm.de), the 
[ELO Ping-Pong Board](http://www.elo-web.de/elo/mikrocontroller-und-programmierung/ping-pong/das-franzis-pingpong) 
or the [LoL Shield](http://jimmieprodgers.com/kits/lolshield/) from Jimmie P.
Rodgers.

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

Supported build platforms are Linux, FreeBSD, NetBSD and Windows (via Cygwin).
Due to customized linker scripts, simulator support is currently limited to x86
and x86_64 archs. Following dependencies have to be met:

Linux / FreeBSD / NetBSD
------------------------

Package names are based on Debian/Ubuntu repositories. Please adapt the names
according to your Linux distribution (or BSD for that matter).

* build-essential (pulls in an ordinary gcc build tool chain for the host)
* bc
* make (gmake on BSD)
* libncurses5-dev
* gcc-avr
* avr-libc
* binutils-avr
* avrdude
* freeglut3-dev
* bash (note to the BSD folks: bash is required for the config tool)

Windows
-------
* AVR GCC toolchain for Windows, choose your poison:
  * [WinAVR](http://winavr.sourceforge.net)
    * already includes [avrdude](http://www.nongnu.org/avrdude/)
    * installer offers to add the toolchain to the system path
    * straight forward download from SourceForge
    * project abandoned in 2010, therefore heavily outdated (avr-gcc 4.3.3)
  * [Atmel AVR Toolchain for Windows](http://www.atmel.com/tools/atmelavrtoolchainforwindows.aspx)
    * actively maintained, therefore fairly up to date
    * homepage nags you with rather awkward registration process before download
    * you have to add the toolchain to the system path manually
    * avrdude is not included (but it is possible to install WinAVR in parallel)
* [Cygwin(64)](http://www.cygwin.com/)
  * bc
  * make
  * gcc-core
  * gdb (in case you want to debug your code in the simulator)
  * libncurses-devel (Cygwin)
  * libncursesw-devel (Cygwin64)
* [libusb-win32](http://sourceforge.net/apps/trac/libusb-win32/wiki) in case you
  want to use an USBasp programmer device with avrdude

Configure
---------

Open a (Cygwin) terminal, change to your checkout directory and type:
 > make menuconfig

In case you build on BSD, just use 'gmake' instead of 'make'. This starts an
Ncurses based text interface for configuring certain aspects of your target
platform. After a fresh checkout, the first thing you do is to load a profile
with sane defaults. In the menuconfig interface, select 'Load a Default
Configuration' and choose a preset. After hitting enter, the main menu returns
immediately. You can either tune your configuration or just exit (choose 'Yes'
at the confirmation dialog to save your stuff).

Be careful if you use an IDE like Eclipse to manage the build, as
integrated terminal emulators tend to choke on Ncurses generated output.
Make sure that 'make menuconfig' has been run at least once in an ordinary
terminal emulator after a fresh checkout or after issuing 'make mrproper'.

Note: Always use 'make clean' after changing something in the menu, because
subsequent builds may be broken if you don't.

Compile
-------

To build for the actual target platform, just type:
 > make 

This yields an 'image.hex' file which you can flash to your AVR device.

If you want to test and debug your code within a GUI application, you can use
the simulator:
 > make simulator

Again, use 'gmake' instead of 'make' on BSD.

You can start the simulator by typing ./borgsim(.exe)

Simulator Handling
------------------

Please keep in mind that the simulator is NOT an emulator. All it does is
compile the source code to a native host application so you can step through
your C-Code with an ordinary host debugger. The GUI thread reads the simulated
frame buffer every 20ms (40ms on  Windows) and draws its contents.

Joystick movements are simulated by the WASD keys and SPACE acts as the fire
button. The OpenGL based simulator (Linux/BSD) enables you to adjust the
viewing angle of the LED matrix via the arrow keys (not available on Windows).

LoL Shield on Arduino (and clones)
----------------------------------

Although this project supports the LoL Shield on various Arduino boards, it does
not use the Arduino software stack at all. Instead, it follows the classical
Unix approach involving make files and mere command line tools. If you haven't
used anything besides the Arduino IDE, getting the Borgware to run (let alone
extending it) might be difficult at first. This README won't even try to cover
all workflows involved.

However, here are some hints:

* Find out what 'avrdude' command line parameters are used by the Arduino IDE to
  flash your device (look into the upload log). Use that parameters (with the -U
  option pointing to the Borgware 'image.hex' file) to flash your board.
* Pay attention to the size of your image. Borgware 2D can easily be configured
  to exceed the usable flash memory of a 32 KiB device. After a successful
  build, both flash and SRAM usage are displayed. Keep in mind that stock
  Arduino MCUs already reserve 0.5 to 4KiB of flash memory for bootloaders.
* Same with SRAM. The "data size" should not exceed 2KiB (make that 1.5 KiB as
  we still need some additional SRAM at runtime).
* In case avrdude gives you a verification error, chances are that your image
  already reaches the bootloader area in the flash. Try to disable some
  animations or games in Menuconfig to trim your image.

Digital joysticks are supported on the 'analog' pins, which are used in digital
input mode with internal pullups turned on. Just connect them to GND to signal
joystick movements.

| Arduino Pin | Meaning       |
| ----------- | ------------- |
| A0          | Up            |
| A1          | Down          |
| A2          | Left          |
| A3          | Right         |
| A4          | Not Connected |
| A5          | Fire          |