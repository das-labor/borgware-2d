
Borgware 2d
===========

Firmware f�r 2d Blinken Borks.
[Blinken Borgs](http://www.das-labor.org/wiki/Blinken_Borgs) sind LED 
Matrixanzeigen zum and die Wand h�ngen oder auf den Tisch stellen.

Dies ist insbesondere die Software f�r den [Borg16](http://www.das-labor.org/wiki/Borg16) Bausatz, 
aber auch f�r eine Reihe �hnlicher Priojekte.


![Kleine Borg16](https://gitlab.das-labor.org/borgware/borgware-2d/raw/master/doc/img/Borg16-small.jpg) &nbsp;
![Gl�hbirnen Borg](https://gitlab.das-labor.org/borgware/borgware-2d/raw/master/doc/img/Borg16-bulb.jpg)
 
Animationen
-----------

![Matrix](https://gitlab.das-labor.org/borgware/borgware-2d/raw/master/doc/img/anim-matrix.png) &nbsp;
![Feuer](https://gitlab.das-labor.org/borgware/borgware-2d/raw/master/doc/img/anim-feuer.jpg)
![Scroll](https://gitlab.das-labor.org/borgware/borgware-2d/raw/master/doc/img/anim-scroll.jpg)


Spiele
------

![Snake](https://gitlab.das-labor.org/borgware/borgware-2d/raw/master/doc/img/game-snake.png) &nbsp;
![Tetris](https://gitlab.das-labor.org/borgware/borgware-2d/raw/master/doc/img/game-tetris.jpg)

### Tetris

 * Classic: Das ganz normale Tetris
 * First Person Tetris: Statt die Steine zu drehen, dreht man hierbei das Spielfeld um den Stein
 * Bastet: Diese Tetris Variante gibt einem stets den Stein, den man gerade am wenigsten gebrauchen kann

### Snake

### Pong

### Space Invaders


Kompilieren
===========

Zun�chst m�ssen alle Abh�nigkeiten installiert sein. Unter Debian/Ubuntu Systemen 
ist ein 

 > sudo aptitude install -y build-essential make libncurses5-dev gcc-avr avr-libc binutils-avr avrdude freeglut3-dev

ausreichend.

Anschliessend kann die zu bauende Firmware mit 

 > make menuconfig

konfiguriert, und mit 

 > make 

gebaut werden.

