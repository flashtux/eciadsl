INTRODUCCI�N
============

EciAdsl es un driver de GNU/Linux para los modems USB basados en los chip-sets ADSL Globespan.
Se sabe que el driver funciona en muchos pa�ses, para los diferentes tipos de modems USB ADSL,
que utilizan distintos tipos de capas y modos de encapsulamiento PPP (vea la lista de los
m�dems soportados en http://eciadsl.flashtux.org/modems.php).


PRE-REQUISITOS
==============

- un sistema ejecutando GNU/Linux (arquitectura x86) con las herramientas normales
  (MDK9 / RH8 / mas recientes / exoticos pueden tener problemas solucionables,
  en el caso de BSD y otros sistemas, estos a�n no estan soportados oficialmente)
- soporte de hardware USB 1.1
- Linux kernel 2.4.x (>=2.4.18-pre3 o <2.4.18-pre3+el parche N_HDLC para obtener
  la caracteristica de funcionamiento correspondiente a la reconexion automatica
  del pppd, la cual es recomendada)
	- soporte USB  (como m�dulos):
		- soporte general de USB
		- fs preliminar USB
		- su controlador de HUB USB COMO M�DULO
		- ning�n m�dulo DABUSB habilitado
	- soporte PPP  (incluso el paquete pppd usermode >=2.4.0)
- instalaci�n desde los programas fuentes: paquetes de desarrollo est�ndar
  (fuentes de Linux en /usr/src/linux, el software GNU tal como gcc>=2.91.66,
   el GNU make, etc.)
- El m�dem USB (*s�lo* los que poseen dentro el chipset de GlobeSpan)
- bash (>=2.x.x)
- opcionalmente: el tcl/tk >= 8.x.x, el perl,
- los privilegios de superusuario root (por lo menos para instalar el driver)


INSTALACI�N DE LOS PROGRAMAS FUENTES
====================================

Lea el archivo INSTALAR (INSTALL).


INSTALACI�N DE UN PAQUETE PRE-COMPILADO
=======================================

Usando un .rpm, como root,:
	rpm -i <el archivo>.rpm

Usted puede tener que quitar versiones instaladas previamente de este
paquete (o use un flag para ignorar los instalados previamente si Ud. ya lo
advirti�).
Entonces: lea el archivo INSTALL, que se encuentra instalado en el directorio
/usr/local/doc/eciadsl por omisi�n, comenzando en la secci�n de la CONFIGURACI�N.


SOLUCIONANDO PROBLEMAS
======================

Si usted obtiene algunos problemas, vea el archivo TROUBLESHOOTING (SOLUCIONANDO PROBLEMAS).


D�nde conseguir soporte?:
	Soporte, paquetes (driver y los archivos de sincronizaci�n synchxx.bin) y la
    documentaci�n:
		- IRC: irc.freenode.net, #eci channel
		- http://eciadsl.flashtux.org?lang=es
		  (documentaci�n, foro, lista de correo de usuarios, etc.)

	Nosotros proporcionamos soporte en los idiomas franc�s e ingl�s.


Proporci�nenos la siguiente informaci�n:

	- la versi�n del kernel (vea: `uname -a` o `cat /proc/version`). nos indica si
	  el kernel es original de su distro o uno que usted compil� a partir de los
	  programas fuentes
	- La distribuci�n de Linux
	- la versi�n del pppd (vea: `pppd --version`)
	- la versi�n del modutils (vea: `lsmod --version`)
	- El controlador USB (HUB), modelo (vea: `lspci | grep USB`), y la lista de todos
	  los HUBs USB usted tiene (si ellos son tarjetas PCI o HUBs externos)
	- sus otros perif�ricos USB
	- la versi�n del driver eciadsl que est� utilizando (vea el archivo VERSI�N en el
	  tarball de los fuentes si existe para versi�n >0.5, o pruebe pppoeci --version,
	  o el nombre del paquete/o fecha del CVS) y:
		- la informaci�n de su driver MS Windows si es que usted lo usa
		  (y tambi�n el tipo de modulaci�n que su proveedor soporta)
		- el archivo synchxx.bin que usted ha probado
	- datos exactos de su m�dem como marca y modelo
	- su localizaci�n (pais/estado/region/ciudad)
	- su proveedor y el protocolo PPP de encapsulaci�n/autenticacion utilizado
	- su nivel de experiencia en Linux (por ejemplo: novicio, promedio, avanzado,
	  experimentado, gur�)
	- la salida del comando route -n antes de y despu�s de ejecutar startmodem
	  si ocurre un problema en la red despues de ejecutarlo
	- su archivo de configuraci�n /etc/eciadsl/eciadsl.conf, y si es posible, su
	  problema que ocurre despu�s de ejecutar startmodem
	- su archivo /etc/eciadsl/eciadsl.conf, y si es posible, su archivo /var/log/messages
	  (verifique que contiene el rastro de la actividad ya sea de su �ltimo reinicio de
	  Linux, la utilizaci�n del driver o su problema).

Puede ayudar mucho si usted nos env�a estas informaciones. Tambi�n consiga la salida de
las herramientas del paquete que usted ha ejecutado (los scripts de configuraci�n o el script
startmodem), tambi�n proporci�nenos su archivo /var/log/messages, /var/log/log si est� disponible,
la salida del comando `dmesg`, `lsmod` y `cat /proc/bus/usb/devices`.

Si usted nos avisa desde MS Windows, y si sus particiones Linux (s�lo ext2 o ext3)
est�n en la misma m�quina, �utilice explore2fs para obtener los archivos directamente
de su partici�n Linux en vez de reiniciar Linux cada vez para obtener un archivo!
explore2fs: http://uranus.it.swin.edu.au/~jn/linux/explore2fs.htm



