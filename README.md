
Borgware 2d
===========

Firmware f�r 2d Blinken Borks.
[Blinken Borgs](http://www.das-labor.org/wiki/Blinken_Borgs) sind LED 
Matrixanzeigen zum and die Wand h�ngen oder auf den Tisch stellen.

Dies ist insbesondere die Software f�r den [Borg16](http://www.das-labor.org/wiki/Borg16) Bausatz, 
aber auch f�r eine Reihe �hnlicher Priojekte.

<img src="http://www.das-labor.org/w/images/thumb/5/5a/Borg16Screen.jpg/800px-Borg16Screen.jpg" height=300 />
&nbsp;
<img src="http://www.das-labor.org/w/images/9/96/BlinkenBorg.jpg" height=300 />
 
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

