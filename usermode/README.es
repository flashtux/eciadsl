INTRODUCCIÓN
============

EciAdsl es un driver de GNU/Linux para los modems USB basados en los chip-sets ADSL Globespan.
Se sabe que el driver funciona en muchos países, para los diferentes tipos de modems USB ADSL,
que utilizan distintos tipos de capas y modos de encapsulamiento PPP (vea la lista de los
módems soportados en http://eciadsl.flashtux.org/modems.php).


PRE-REQUISITOS
==============

- un sistema ejecutando GNU/Linux (arquitectura x86) con las herramientas normales
  (MDK9 / RH8 / mas recientes / exoticos pueden tener problemas solucionables,
  en el caso de BSD y otros sistemas, estos aún no estan soportados oficialmente)
- soporte de hardware USB 1.1
- Linux kernel 2.4.x (>=2.4.18-pre3 o <2.4.18-pre3+el parche N_HDLC para obtener
  la caracteristica de funcionamiento correspondiente a la reconexion automatica
  del pppd, la cual es recomendada)
	- soporte USB  (como módulos):
		- soporte general de USB
		- fs preliminar USB
		- su controlador de HUB USB COMO MÓDULO
		- ningún módulo DABUSB habilitado
	- soporte PPP  (incluso el paquete pppd usermode >=2.4.0)
- instalación desde los programas fuentes: paquetes de desarrollo estándar
  (fuentes de Linux en /usr/src/linux, el software GNU tal como gcc>=2.91.66,
   el GNU make, etc.)
- El módem USB (*sólo* los que poseen dentro el chipset de GlobeSpan)
- bash (>=2.x.x)
- opcionalmente: el tcl/tk >= 8.x.x, el perl,
- los privilegios de superusuario root (por lo menos para instalar el driver)


INSTALACIÓN DE LOS PROGRAMAS FUENTES
====================================

Lea el archivo INSTALAR (INSTALL).


INSTALACIÓN DE UN PAQUETE PRE-COMPILADO
=======================================

Usando un .rpm, como root,:
	rpm -i <el archivo>.rpm

Usted puede tener que quitar versiones instaladas previamente de este
paquete (o use un flag para ignorar los instalados previamente si Ud. ya lo
advirtió).
Entonces: lea el archivo INSTALL, que se encuentra instalado en el directorio
/usr/local/doc/eciadsl por omisión, comenzando en la sección de la CONFIGURACIÓN.


SOLUCIONANDO PROBLEMAS
======================

Si usted obtiene algunos problemas, vea el archivo TROUBLESHOOTING (SOLUCIONANDO PROBLEMAS).


Dónde conseguir soporte?:
	Soporte, paquetes (driver y los archivos de sincronización synchxx.bin) y la
    documentación:
		- IRC: irc.freenode.net, #eci channel
		- http://eciadsl.flashtux.org?lang=es
		  (documentación, foro, lista de correo de usuarios, etc.)

	Nosotros proporcionamos soporte en los idiomas francés e inglés.


Proporciónenos la siguiente información:

	- la versión del kernel (vea: `uname -a` o `cat /proc/version`). nos indica si
	  el kernel es original de su distro o uno que usted compiló a partir de los
	  programas fuentes
	- La distribución de Linux
	- la versión del pppd (vea: `pppd --version`)
	- la versión del modutils (vea: `lsmod --version`)
	- El controlador USB (HUB), modelo (vea: `lspci | grep USB`), y la lista de todos
	  los HUBs USB usted tiene (si ellos son tarjetas PCI o HUBs externos)
	- sus otros periféricos USB
	- la versión del driver eciadsl que está utilizando (vea el archivo VERSIÓN en el
	  tarball de los fuentes si existe para versión >0.5, o pruebe pppoeci --version,
	  o el nombre del paquete/o fecha del CVS) y:
		- la información de su driver MS Windows si es que usted lo usa
		  (y también el tipo de modulación que su proveedor soporta)
		- el archivo synchxx.bin que usted ha probado
	- datos exactos de su módem como marca y modelo
	- su localización (pais/estado/region/ciudad)
	- su proveedor y el protocolo PPP de encapsulación/autenticacion utilizado
	- su nivel de experiencia en Linux (por ejemplo: novicio, promedio, avanzado,
	  experimentado, gurú)
	- la salida del comando route -n antes de y después de ejecutar startmodem
	  si ocurre un problema en la red despues de ejecutarlo
	- su archivo de configuración /etc/eciadsl/eciadsl.conf, y si es posible, su
	  problema que ocurre después de ejecutar startmodem
	- su archivo /etc/eciadsl/eciadsl.conf, y si es posible, su archivo /var/log/messages
	  (verifique que contiene el rastro de la actividad ya sea de su último reinicio de
	  Linux, la utilización del driver o su problema).

Puede ayudar mucho si usted nos envía estas informaciones. También consiga la salida de
las herramientas del paquete que usted ha ejecutado (los scripts de configuración o el script
startmodem), también proporciónenos su archivo /var/log/messages, /var/log/log si está disponible,
la salida del comando `dmesg`, `lsmod` y `cat /proc/bus/usb/devices`.

Si usted nos avisa desde MS Windows, y si sus particiones Linux (sólo ext2 o ext3)
están en la misma máquina, ¡utilice explore2fs para obtener los archivos directamente
de su partición Linux en vez de reiniciar Linux cada vez para obtener un archivo!
explore2fs: http://uranus.it.swin.edu.au/~jn/linux/explore2fs.htm



