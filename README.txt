
Borgware 2d
===========

Firmware f�r 2d Blinken Borgs.


Kompilieren
===========

Zun�chst m�ssen alle Abh�nigkeiten installiert sein. Unter Debian/Ubuntu Systemen 
ist ein 

  sudo aptitude install -y build-essential make libncurses5-dev gcc-avr avr-libc binutils-avr avrdude freeglut3-dev

ausreichend.

Anschliessend kann die zu bauende Firmware mit 

  make menuconfig

konfigurier, und mit 

  make 

