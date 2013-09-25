
Borgware 2d
===========

Firmware für 2d Blinken Borks.
[Blinken Borgs](http://www.das-labor.org/wiki/Blinken_Borgs) sind LED 
Matrixanzeigen zum and die Wand hängen oder auf den Tisch stellen.

Dies ist insbesondere die Software für den [Borg16](http://www.das-labor.org/wiki/Borg16) Bausatz, 
aber auch für eine Reihe ähnlicher Priojekte.


![Kleine Borg16](/doc/img/Borg16-small.jpg)
![Glühbirnen Borg](/doc/img/Borg16-bulb.jpg)
 
Animationen
-----------

<img src="http://www.das-labor.org/w/images/thumb/f/f4/Borgware2d_snake.png/120px-Borgware2d_snake.png"> &nbsp;
<img src="http://www.das-labor.org/w/images/thumb/4/44/Borgware2d_matrix.png/120px-Borgware2d_matrix.png"> &nbsp;
<img src="http://www.das-labor.org/w/images/thumb/4/4c/Borgware2d_feuer.png/120px-Borgware2d_feuer.png"> &nbsp;
<img src="http://www.das-labor.org/w/images/thumb/1/1e/Borgware2d_scroll.png/120px-Borgware2d_scroll.png"> 

Spiele
------

### Tetris

<img src="http://www.das-labor.org/w/images/thumb/0/0f/Borgware2d_tetris.png/120px-Borgware2d_tetris.png">

 * Classic: Das ganz normale Tetris
 * First Person Tetris: Statt die Steine zu drehen, dreht man hierbei das Spielfeld um den Stein
 * Bastet: Diese Tetris Variante gibt einem stets den Stein, den man gerade am wenigsten gebrauchen kann

### Snake

### Pong

### Space Invaders


Kompilieren
===========

Zunächst müssen alle Abhänigkeiten installiert sein. Unter Debian/Ubuntu Systemen 
ist ein 

 > sudo aptitude install -y build-essential make libncurses5-dev gcc-avr avr-libc binutils-avr avrdude freeglut3-dev

ausreichend.

Anschliessend kann die zu bauende Firmware mit 

 > make menuconfig

konfiguriert, und mit 

 > make 

gebaut werden.

