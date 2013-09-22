
Borgware 2d
===========

Firmware für 2d Blinken Borgs.


Kompilieren
===========

Zunächst müssen alle Abhänigkeiten installiert sein. Unter Debian/Ubuntu Systemen 
ist ein 

  sudo aptitude install -y build-essential make libncurses5-dev gcc-avr avr-libc binutils-avr avrdude freeglut3-dev

ausreichend.

Anschliessend kann die zu bauende Firmware mit 

  make menuconfig

konfigurier, und mit 

  make 

