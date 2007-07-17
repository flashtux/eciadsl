PREGUNTAS DE USO FRECUENTE Y ALGUNOS PROBLEMAS COMUNES
======================================================

Al público interesado:
Driver para los usuarios del modem USB ECI ADSL (hasta la versión 0.11).

Antes de leer por favor tome en cuenta lo siguiente:
la lista de Preguntas/Respuestas (Q/A) no es exhaustiva, Es posible que usted no
encuentre la pregunta y respuesta mas adecuada a su problema.
Sin embargo, por favor léalas, ellas pueden proporcionarle la ayuda suficiente u
orientarlo en la solución de su problema.
Y aunque su problema se encuentre en la lista, es posible que las respuestas no
lo ayuden a resolverlo. Entonces, por favor avísenos, nosotros intentaremos
ayudarlo para encontrar una solución.

1 - Compilación del driver, instalación, y configuración
2 - Inicialización del módem
3 - Sincronización del módem
4 - Problemas de conexión
5 - Temas varios

1.0
================================================================================
P:	No sé si mi módem es soportado.
	No encuentro mi módem en la lista (usando eciadsl-config-tk o eciadsl-config-text).

--------------------------------------------------------------------------------
R:	Verifique los pre-requisitos.

	Verifique en http://eciadsl.flashtux.org/modems.php?lang=en

	Instale la última versión del driver, entonces ejecute eciadsl-start o
	eciadsl-doctor, si ellos reportan errores, intente ejecutar el script
	eciadsl-probe-device para verificar la compatibilidad de su módem. ¡Si
	todavía no funciona, avísenos!


1.1
================================================================================
P:	`./configure` falla y devuelve algunos errores :-\

--------------------------------------------------------------------------------
R:	Por favor repórtenos la salida de `./configure`.
	Verifique que opciones podrían ayudarlo (vea `./configure --help`).
	Por ejemplo, usted puede usar la opción "--disable-gcc-check" si
	`./configure` falla en encontrar gcc o verificando su número de versión
	considerando que gcc está instalado y esta	en el $PATH. Este error se puede
	producir en algunos sistemas que incluyen un compilador gcc modificado (como
	es el caso de Debian).


1.2
================================================================================
P:	Obtengo el siguiente error cuando ejecuto `make` o `make install`:
	Makefile:1: Makefile.config: No such file or directory
	(No existe el archivo o directorio)
	make: *** No rule to make target `Makefile.config'. Stop.

--------------------------------------------------------------------------------
R:	¡Usted ejecuto `./configure`? Usted debe poseer una versión >=0.6-pre4!


1.3
================================================================================
P:	`eciadsl-config-tk` falla inmediatamente, diciendo,:
	wish: command not found (el comando no se encontró)

--------------------------------------------------------------------------------
R:	Instale tcl/tk, o ejecute `eciadsl-config-text`. Aun cuando tcl/tk no es
	obligatorio, verifique los pre-requisitos.


1.4
================================================================================
P:	eciadsl-probe-device me muestra un VID1/PID1 que es el mismo que VID2/VID2,
	¿es esto normal?

--------------------------------------------------------------------------------
R:	Usted debe reiniciar su módem (desconectar/esperar/reconectar) antes de
	ejecutar el script eciadsl-probe-device.
	Por favor lea la ADVERTENCIA desplegada por este script cuando es ejecutado.

	Esto puede ser debido a problemas más generales en la configuración del USB
	de su sistema. Ya sea que el "hotplug" interfiere con el driver o los
	módulos de USB tienen problemas con su hardware (también podría ser un bug
	en el kernel).

	También compruebe si el módulo dabusb es mostrado en /var/log/messages
	(vea la pregunta anterior relacionada con el modulo dabusb).


1.5
================================================================================
P:	eciadsl-start, eciadsl-config-tk o eciadsl-doctor me informan un ¡módem no
	soportado!

--------------------------------------------------------------------------------
R:	Vea 1.4.

	Si su módem conectado no es mostrado en /proc/bus/usb/devices, verifique en
	/var/log/messages. Si usted revisa los mensajes que aparecieron durante la
	última inicialización:
		kernel: usb_control/bulk_msg: timeout
		kernel: usb.c: USB device not accepting new address=2 (error=-110)
	Esto indica que hay un problema entre la configuración del
	hardware/BIOS/kernel, este es un tema relacionado con las las
	interrupciones. Pruebe desactivando el soporte APIC en el kernel o en la
	inicializacion del sistema (use noapic cuando inicie el kernel, pero esto
	puede provenir de otro conflicto de IRQ entre dispositivos del hardware, o
	alguna otra configuración del kernel, o también puede deberse a la
	habilitación/deshabilitación del soporte USB en el BIOS).

	Tales problemas son comunes con algunos chipsets de USB tales como los VIA y
	SiS 700x.
	Si usted no sabe qué chipset tiene o si usted quiere obtener la confirmación
	que su problema es un problema de driver relacionado con el hardware/kernel,
	por favor avísenos.
	Si esto es confirmado, una solución posible sería comprar un HUB USB para
	instalar en un slot PCI libre (no intente conectar un HUB USB externo en un
	puerto USB de su HUB USB incorporado en la tarjeta madre, esto no
	funcionará, debido a que el problema proviene del chipset incluido en la
	tarjeta madre).
	¡No compre esas tarjetas PCI si usted no puede probarlas antes!

	También verifique la pregunta que trata del módulo dabusb, si el módulo
	dabusb está cargado, usted no podrá usar su módem con este driver hasta que
	usted desactive la carga del módulo dabusb.


1.6
================================================================================
P:	No conozco las direcciones IP del(o los) DNS de mi proveedor.

--------------------------------------------------------------------------------
R:	La mayoría de las personas los necesita (excepto aquéllos que usan DHCP por
	ejemplo, para los cuales la configuración es realizada en forma automática),
	si usted no puede encontrarlos en el website de su proveedor o en el CD de
	la configuración o en los documentos que ellos le envían a usted, Existe una
	manera de obtener esta información incluso bajo MS Windows. Aquí se muestra
	la manera para obtener esta información desde Linux, ejecute el siguiente
	comando:
		>host -t ns <nombre de dominio del proveedor>

	Este comando le devuelve un cantidad de nombres de servidores DNSs. Usted
	puede ejecutar el comando ping sobre algunos de ellos para obtener las
	direcciones IP.

	Por ejemplo:
		> host -t ns t clix.com
		clix.com. name server ns4.dnsmanaged.com.
		clix.com. name server ns1.dnsmanaged.com.
		clix.com. name server ns7.dnsmanaged.com.
		clix.com. name server ns6.dnsmanaged.com.

		>ping -c 1 -q ns4.dnsmanaged.com
		PING ns4.dnsmanaged.com (192.31.80.34) from 10.0.0.99: 56(84) bytes of
		data.
		--- ns4.dnsmanaged.com ping statistics---
		1 packets transmitted, 1 received, 0% loss, time 0ms,
		rtt min/avg/max/mdev = 169.466/169.466/169.466/0.000 ms

	La direccion IP del servidor ns4.dnsmanaged.com es 192.31.80.34.
	Repita el comando ping con otro servidor de nombre DNS, entonces usted
	obtendrá al menos 2 direcciones IP de los servidores DNS para usar: -).
	Si la herramienta config del driver ECIADSL no le permite seleccionar el
	DNS de su proveedor, seleccione 'Otro' y tipee por lo menos una direccion
	IP correspondiente al DNS a utilizar.


2.0
================================================================================
P:	los mensajes emitidos por eciadsl-start, eciadsl-config-tk o eciadsl-doctor
	indican: /proc/bus/usb: No such file or directory (No existe el archivo o
	Directorio).

--------------------------------------------------------------------------------
R:	Verifique la configuración USB de su sistema. Esto puede ser debido a una
	falta de soporte para el usbdevfs en su configuración del kernel. Si el
	kernel tiene el soporte para él, pruebe montarlo usando:
		>mount -t usbdevfs none /proc/bus/usb
	Esto puede ponerse en su archivo /etc/fstab.
	Para conseguir que sea montado automaticamente en momento de la
	inicialización del sistema, agregue esta línea:
		usbdevfs /proc/bus/usb usbdevfs defaults 0 0

	Nota: No habrá nada en /proc/bus/usb hasta que un módulo controlador
	anfitrión también sea cargado.


2.1
================================================================================
P:	ecidoctor me informa que el módulo DABUSB está cargado, o el comando
	eciadsl-start dice que se encontró el módulo dabusb, ¡pero no fué posible
	desinstalarlo!

--------------------------------------------------------------------------------
R:	Hotplug probablemente esté habilitado, y este detecta sus módems como un
	dispositivo de audio y carga el módulo dabusb para agregar el soporte para
	este dispositivo de audio.

	Si el archivo /etc/hotplug/blacklist existe, edítelo y agregue una línea que
	contega la palabra 'dabusb' (sin las comillas simples). Reinicie Linux.

	Si usted no puede encontrar el archivo considerando que el hotplug está
	instalado y habilitado, debe haber otra manera de configurarlo, sin embargo
	usted también puede aplicar el método siguiente (un poco rudo):

	Reinicie su maquina Linux con su módem *DESCONECTADO*, entonces:

	Usted puede quitar el módulo dabusb de su sistema usando para esto el script
	eciadsl-config-tk o eciadsl-config-text.

	Usted también puede ejecutar directamente el comando call eciadsl-remove-dabusb
	(en /usr/local/bin por omisión).

	O manualmente digite el siguiente comando:
		>modprobe -r dabusb && rm -f $(modprobe -l | grep dabusb) && depmod -a

	Si este kernel ha sido compilado a mano, también no se olvide de quitar el
	soporte dabusb de la configuración del kernel.


2.2
================================================================================
P:	¡No hay manera de quitar el módulo dabusb! ¡En cada reinicio del sistema, es
	cargado nuevamente!

--------------------------------------------------------------------------------
R:	Vea 2.1, pero use el método manual.


2.3
================================================================================
P:	Intento quitar el módulo dabusb, pero siempre obtengo:
	dabusb: Device or resource busy (Dispositivo o recurso ocupado)

--------------------------------------------------------------------------------
R:	Vea 2.2.


2.4
================================================================================
P:	Mi módem parece ser inicializado al reiniciar el sistema, los LEDs están
	encendidos fijos o intermitentes. ¿Significa ésto que mi sistema reconoció
	mi módem y que no tengo que instalar el driver?

--------------------------------------------------------------------------------
R:	¡No! Esto no es en absoluto amistoso.
	Ése es un problema del módulo dabusb. Vea 2.3.


2.5
================================================================================
P:	`eciadsl-doctor` me informa:
	El soporte de HDLC no es el adecuado (buggy), usted debe aplicar el parche
	HDLC a los fuentes del Nucleo (Kernel).

--------------------------------------------------------------------------------
R:	Siga los pre-requisitos, lea la documentación sobre el problema del N_HDLC.

	Si usted quiere usar la opcion 'persistente' del pppd para reconectar
	automáticamente cuando es desconectado del proveedor de servicios, usted
	tendrá que parchar el kernel o use un kernel >=2.4.18-pre3.
	Usted puede encontrar el parche n_hdlc.c.diff en el archivo de los drivers
	del módem Speedtouch en: http://speedtouch.sourceforge.net/
	Éstas son las instrucciones sobre cómo usarlo:
		>cd /usr/src/linux
		>patch -p1--dry-run < /path/to/n_hdlc.c.diff

	Si ningún mensaje del error ha sido devuelto por el comando anterior, digite
	la siguiente línea para realizar la corrección del fuente:
		>patch -p1 < /path/to/n_hdlc.c.diff
	una vez realizado esto valla a la pregunta de configuración del kernel
	(5.3).


2.6
================================================================================
P:	eciadsl-start dice: eciadsl-firmware: Timeout.

--------------------------------------------------------------------------------
R:	Su módem probablemente no es soportado, comience a leer este FAQ desde el
	comienzo.


2.7
================================================================================
P:	eciadsl-start/eciadsl-probe-device/eciadsl-doctor no pueden encontrar mi módem.

--------------------------------------------------------------------------------
R:	Su módem probablemente no es soportado o hay un problema con el hardware de
	su HUB USB, también podría ser un bug en los módulos del kernel.


3.0
================================================================================
P:	eciadsl-start dice: eciadsl-synch: Timeout

--------------------------------------------------------------------------------
R:	Vea 3.1.


3.1
================================================================================
P:	eciadsl-start dice: eciadsl-synch: failed

--------------------------------------------------------------------------------
R:	Vea 3.2.

	Problema conocido: bajo GNOME o KDE, eciadsl-synch no puede obtener la
	sincronización. Intente con la última versión, o contáctenos.
	Esto también puede pasar si usted ejecuta el eciadsl-start mientras su CPU esta
	bajo fuerte carga.


3.2
================================================================================
P:	eciadsl-start se detiene en el bloque xxx.

--------------------------------------------------------------------------------
R:	Siga los pre-requisitos.

	Asegúrese que la línea telefónica esté conectada correctamente a ambos
	extremos (al módem y al conector de la pared), y que el microfiltro del DSL
	esté colocado entre los adaptadores de la pared y cualquier otro dispositivo
	(teléfono, fax, contestador automático), verifique si su microfiltro está
	OK.

	Actualice su versión del driver eciadsl (usuarios de la versión 0.5:
	consigan versión 0.7 o el último CVS).

	Pruebe otro archivo de sincronización synchxx.bin (disponibles en línea).

	Nota: entre cada prueba realizada con archivos de sincronización
	diferentes, se requiere desconectar y conectar el modem USB.


4.0
================================================================================
P:	/var/log/messages muestra fallos en CHAP o en PAP.

--------------------------------------------------------------------------------
R:	Verifique en sus archivos /etc/ppp/pap-secrets o /etc/ppp/chap-secrets. Las
	siguientes entradas deben ser como estas:
		"username" * "userpassword" *
	Usted puede configurar esto utilizando eciadsl-config-tk.

	Dependiendo de la versión de pppd, la sintaxis de pap-secrets y chap-secrets
	puede diferir. En ese caso, intente modificarlos a mano o contáctenos.


4.1
================================================================================
P:	eciadsl-start termino OK, pero aún no puedo usar el acceso a Internet

--------------------------------------------------------------------------------
R:	Vea 4.0.

	Si usted puede hacer ping a una IP pero no puede hacer ping un nombre de
	host, verifique su archivo /etc/resolv.conf, debe incluir la dirección IP
	del DNS de su proveedor.
	Por ejemplo (los del Wanadoo francés):
		nameserver 193.252.19.3
		nameserver 193.252.19.4
	Usted puede colocarlos usando eciadsl-config-tk.

	Si usted todavía no puede acceder un host por su nombre usando el comando
	nslookup, pruebe los siguientes comandos y nos informa los resultados:

	>route -n (o netstat -rn)

	  Kernel IP routing table
	  Destination     Gateway         Genmask         Flags   MSS Window  irtt Iface
	* 80.14.50.1      0.0.0.0         255.255.255.255 UH      40 0           0 ppp0
	* 10.0.0.0        0.0.0.0         255.255.255.0   U       40 0           0 eth0
	* 127.0.0.0       0.0.0.0         255.0.0.0       U       40 0           0 lo
	  0.0.0.0         80.14.50.1      0.0.0.0         UG      40 0           0 ppp0

	  Las líneas que llevan un * inicial son opcionales, su eth de la red local
	  debe, diferir de 10.0.0.0 (si es que usted tiene uno).
	  La línea de UG debe aparecer, esto indica la ruta predefinida al gateway.

	>ifconfig ppp0

	  ppp0    Link encap:Point-to-Point Protocol
        	  inet addr:80.14.50.227 P-t-P:80.14.50.1 Mask:255.255.255.255
        	  UP POINTOPOINT RUNNING NOARP MULTICAST MTU:1500 Metric:1
        	  RX packets:80951 errors:0 dropped:0 overruns:0 frame:0
        	  TX packets:85395 errors:0 dropped:0 overruns:0 carrier:0
        	  collisions:0 txqueuelen:3

	  Las direcciones IP y los otros valores serán distintos de los anteriores.

	Si la interfaz ppp0 no esta activa, verifique las líneas de error en
	/var/log/messages o /var/log/ppp después de haber ejecutado el comando
	eciadsl-start.

	Si la ruta predefinida (UG) esta configurada para la interfaz eth0, quítela
	utilizando el siguiente comando:
		>route del default dev eth0
	o quite la línea de "gateway" de su archivo /etc/sysconfig/network, o
	desactive su red LAN antes de ejecutar el comando eciadsl-start.

	Si la ruta predefinida no es configurada a la interfaz ppp0 aun cuando el
	ppp0 esté activo, realice el siguiente comando:
		>route add default dev ppp0

	Esto puede ser un problema del firewall. Intente desactivarlo, si esto
	ahora funciona, usted tiene que ajustar la configuración de su firewall.
	Usuarios de Mandrake 9: si usted ve líneas como las siguientes en su
	/var/log/messages:
		Shorewall:INPUT:REJECT:IN=ppp0....
	entonces usted puede estar seguro que es un problema del firewall.


4.2
================================================================================
P:	Después de un rato se desconecta, las opciones de persistencia de pppd
	parecen funcionar pero no puedo accesar ningún sitio en Internet.

--------------------------------------------------------------------------------
R:	¿Usted sabe que pppd llama a /etc/ppp/ip-down cuando se desconecta y que
	llama a /etc/ppp/ip-up una vez reconectado? ¿Quizá el valor predeterminado
	de su distribución para ip-down restaura la tabla de enrutamiento basica de
	red o quita el DNS en /etc/resolv.conf?


4.3
================================================================================
P:	Después de unos minutos de acceso de Internet, nada parece funcionar.
	No funciona el ping, no hay acceso vía http ni nslookup.

--------------------------------------------------------------------------------
R:	A menudo se ha visto en algunas distribuciones recientes (MDK9, Slackware
	8.1 y RH8).

	Compruebe el conductor del USB que usted está utilizando con comando del lsmod.
	Si usted está utilizando uhci, descargúelo (modprobe -r uhci) y cargue usb-uhci
	(modprobe usb-uhci).

	Usted ha visto una desconexion (y quizas una reconexion)?  En
	/var/log/messages o en /var/log/ppp? en ese caso, salte a la pregunta
	anterior.

	El resto podría ser configuración de la red/firewall relacionado, pero no
	seguramente. Pruebe `network down` si tiene red pero no la necesita.


4.4
================================================================================
P:	En /var/log/messages encuentro varios mensajes que indican "LCP Timeout" y
	finalmente un mensaje que dice modem hangup (y eventualmente otros problemas
	entre ambos mensajes) tal como estos:
		sent [LCP ConfReq id=0x1 <asyncmap 0x0> <magic 0xf990655f>]
		...
		LCP: timeout sending Config-Requests
		Connection terminated.

--------------------------------------------------------------------------------
R:	¿Está su kernel realmente OK? ¿El soporte N_HDLC es OK? Verifique los
	pre-requisitos primero (Lea el archivo README).

	Éste puede ser un problema del sincronismo. Pruebe otro archivo synchxx.bin
	disponible en la página oficial.

	También vea 4.5, porque los problemas de autenticación de CHAP/PAP no
	siempre son reportados en detalle por el pppd.
	Las preguntas 4.6 a 4.9 también pueden ayudar.

	También vea mas adelante las preguntas relacionadas con los usuarios
	internacionales (5.4).


4.5
================================================================================
P:	A veces veo algunos mensajes de timeouts (temporización) de LCP en
	/var/log/messages, pero mi conexión aún está activa o no se interrumpe.

--------------------------------------------------------------------------------
R:	Intente quitar los comentarios de las dos líneas que tratan con LCP en el
	archivo /etc/ppp/peers/adsl, y quizá para aumentar los valores ligeramente.

	Esto también puede deberse a su proveedor, espere por pocos minutos/horas si
	usted todavía no se puede conectar.

	Si estas temporizaciones de LCP no afectan su conexión de PPP, simplemente
	olvidese de ellas o pruebe otro archivo synchxx.bin.


4.6
================================================================================
P:	Obtengo una desconexion del módem (modem hangup) o no puedo conectar en
	absoluto, y veo éstos mensajes en /var/log/messages:
	ioctl(PPP..): Inappropiate ioctl for device (ioctl inadecuado para el
	dispositivo).
	Connexion failed

--------------------------------------------------------------------------------
R:	Verifique la configuración de su kernel (mas adelante).

	Actualice su versión del driver de eciadsl.

	Pruebe otro archivo synchxx.bin.

	Esto puede suceder cuando la invocación de eciadsl-pppoeci en el archivo
	/etc/ppp/peers/adsl está corrupto o malo (probablemente porque ¿ha sido
	editado a mano?).
	Por ejemplo, el uso de "-vendor 0915" es incorrecto. La sintaxis exacta es
	"-vendor 0x0915".
	¡Utilice eciadsl-config-tk para configurar el driver en forma adecuada!

	Generalmente, esto también puede provenir de una mala configuración, por
	ejemplo, si el modo del PPP no es el que usted necesita.

	Permita el registro detallado al eciadsl-pppoeci (- v 2), y si usted ve en el
	archivo log de eciadsl-pppoeci estas líneas:
		hi! I'm the parent process, I handle the endpoint 0x07
		file descriptors: fdin=3, fdout=4
		error loading N_HDLC
	Usted debe asegurarse que su kernel tiene el módulo n_hdlc (pre-requisito).
	¡Vea la pregunta pertinente (5.3)!


4.7
================================================================================
P:	Obtengo algunos mensajes de "USB timeout" (temporización de USB) en
	/var/log/messages.

--------------------------------------------------------------------------------
R:	Quite los comentarios a la linea MTU en el archivo /etc/ppp/peers/adsl.
	Eventualmente modifique el valor del MTU (vea `man pppd`).

	Esto también puede estar relacionado con un problema del kernel/hardware
	(vea las preguntas siguientes).


4.8
================================================================================
P:	El enlace PPP a veces se desconecta (modem hangup).

--------------------------------------------------------------------------------
R:	Esto puede ser debido a un alto tráfico en la línea o a cualquier problema
	diario de desconexion del proveedor (lista no-exhaustiva).

	Habilite el modo 'persistente' para el pppd en /etc/ppp/peers/adsl (esto\
	requiere el soporte N_HDLC en el kernel, vea la pregunta relacionada con la
	configuración del kernel un poco mas adelante). Verifique que la opción
	'persist' está configurada (por omision es así).

	Disminuya el valor del MTU en /etc/ppp/peers/adsl a 1000 o incluso a 512
	puede ayudar.

	Usted también puede probar usando un script de autoconexion de terceros, y
	por supuesto verifique que su configuración ha sido realizada adecuadamente.


4.9
================================================================================
P:	Obtengo desconexiones del módem pero mi problema no se parece a las
	preguntas anteriores o ¡mi módem se apaga!

--------------------------------------------------------------------------------
R:	¿Su chipset de USB es un antiguo VIA o un SiS 700x? Algunos son conocidos
	por contener defectos y por tener problemas con el suministro de energia al
	USB o por un módulo defectuoso del kernel. Si su módem se apaga (sin
	energía), éste podría ser su caso (VÍA).

	En algunos sistemas (chipset USB defectuosos o procesadores viejos), esto
	puede pasar cuando usted usa multiples dispositivos USB que extraen mucha
	energía (dispositivos de video, hdd).

	Esto también puede pasar si su CPU está muy sobrecargada (quemando un CD o
	algo así) o cuando el bus USB también está muy sobrecargado (uso de webcam,
	etc.).


4.10
================================================================================
P:	Veo caracteres extraños en el terminal/consola después de que el comando
	eciadsl-start finaliza, y no obtengo ninguna conexión PPP:
	Connect Modem ...
	~ÿ}#À!}!}!} }4}"}&} } } } }%}&øïpÆ}

--------------------------------------------------------------------------------
R:	Usted no está bebido, pppd no puede comunicarse con el eciadsl-pppoeci (parte
	del driver) y debe provenir de una mala versión del pppd, de una configuración
	mala del ppp, dentro del kernel, o porque usted está usando el driver en un
	sistema no soportado.

	También verifique el archivo /etc/ppp/peers/adsl. La invocación del eciadsl-pppoeci
	podría estar corrupta, o el archivo no existe en lo absoluto (éste es un
	problema conocido).


4.11
================================================================================
P:	Obtengo mensajes "Kernel panic/oops".

--------------------------------------------------------------------------------
R:	Espere por la versión 0.7 de eciadsl (hay una falla en la limpieza del URB
	cuando el pppd se desconecta).

	Esto puede provenir de un problema en el módulo del kernel (OHCI?), el
	hardware mal soportado, CPU o bus USB muy sobrecargado, o cualquier tipo de
	problema del sistema a nivel global (incluso un error en el driver).


4.12
================================================================================
P:	Obtengo este mensaje en /var/log/messages:
	kernel: usb-uhci.c: ENXIO 80000xxx, flags 0, urb c5205d60, burb c2ad1120

--------------------------------------------------------------------------------
R:	<POR HACER: describir los problemas relacionados con el URB>

	Intente desactivar todos los otros dispositivos USB para asegurarse que este
	problema está relacionado con el uso del módem.


4.13
================================================================================
P:	El comando (script) eciadsl-doctor dice:
	Modem hangup
	Connection terminated.
	... usb_control/bulk_msg: timeout

--------------------------------------------------------------------------------
R:	¡Demasiadas razones posibles! Por favor contáctenos.
	Vea también 1.5.


4.14
================================================================================
P:	Mi relación de transferencia es muy lenta (p.ej.: 2-5KB/sec en lugar de los
	40KB/sec esperados)

--------------------------------------------------------------------------------
R:	Intente con otro archivo synchxx.bin.

	Recientemente, algunos usuarios en Bélgica han tenido tales problemas,
	desde que su proveedor comenzó a entregar un ancho de banda de 3.3Mbit/sec.

	Usted también puede jugar con el valor de MTU.


4.15
================================================================================
P:	Utilizo DHCP con mi ISP, `eciadsl-start` finaliza OK pero no puedo acceder a
	internet.

--------------------------------------------------------------------------------
R:	Ninguna línea "UG" (ninguna ruta predefinida), podría ser un problema del
	cliente DHCP en su sistema.
	La mayoría de los clientes comunes de DHCP estan incorrectos, dhclient y
	dhcpcd.
	Verifique si su cliente DHCP está instalado, de lo contrario instale una
	versión más reciente.

	Nota: este problema ha sido informado por usuarios en Finlandia, Suecia y
	otros países en Asia.


4.16
================================================================================
P:	eciadsl-start dice:
	eciadsl-synch: failed to create shared semaphore: No space left on device
	(el programa eciadsl-synch: fallo en la creación de un semáforo compartido: No
	queda espacio en el dispositivo)

--------------------------------------------------------------------------------
R:	Este es un problema introducido en la versión 0.6. debera ser corregido en
	la versión superior a la 0.7. Por favor actualice con cualquier versión
	> 0.7. o use el cvs.


5.0
================================================================================
P:	¿Es posible ejecutar eciadsl-start en el momento del inicio del sistema?

--------------------------------------------------------------------------------
R:	Sí, usando init.d por ejemplo, o /etc/ppp/ppp_on_boot para los usuarios de
	Debiam.

	Haciéndolo posible usando init.d:

	Los Pre-requisitos:
	 - Driver de ECIADSL instalado y configurado
	 - los fuentes empaquetados de ECIADSL, disponibles en:
	   http://eciadsl.flashtux.org/download.php?lang=en


	Copie el archivo rc.adsl a /etc/init.d:
		> cp rc.adsl /etc/init.d
		> chmod +x /etc/init.d/rc.adsl
	(el archivo rc.adsl se proporciona con la versión >= 0.7 (o CVS))

	Para Debian, simplemente digite como root:
	>update-rc.d rc.adsl defaults 15

	Para otras distribuciones, vea mas adelante:

	En /etc/rc.d, cada directorio rc *.d corresponde a los runlevel de
	inicialización. Para el caso, runlevel 5 es el que lleva a un login gráfico
	(esto es predefinido en la mayoría de los sistemas Linux, pero a veces es el
	3), de cualquier modo /etc/rc.d/rc5.d pertenece a ese nivel.

	Usted encontrará más información sobre los niveles de inicialización:
		>man inittab
	o mire su archivo /etc/inittab.

	Para saber qué nivel de inicialización predefinido se alcanza en el
	reinicio, busque una línea como la siguiente en el archivo /etc/inittab:
		id:5:initdefault:
	Aquí, el nivel predefinido es 5.
	Para el funcionamiento inferior, digamos que usamos el nivel de
	inicialización 5 al inicio del sistema.

	Usted tiene que saber que cuando su sistema entra en un nivel de
	inicialización, llama todos los archivos K* desde el último nivel, entonces
	llama todos los archivos S* desde el nuevo nivel. Todos estos archivos K* y
	S* estan en /etc/rc.d/rc<init level>.d a ellos pertenecen.
	De hecho, estos archivos S* y K* son los enlaces simbolicos (symlinks), y
	apuntan a scripts localizados (la mayoría de las veces) en /etc/init.d.

	Cada archivo S* es así: Sxxyyyyy dónde xx es un número de 2-digitos, e yyyyy
	un nombre de servicio (el nombre es comprensible, pero usted puede ponerlo
	como quiera).
	Todos los archivos Sxxyyyyy son llamados siguiendo la numeración xx
	(ascendente), despues por el nombre de servicio (el yyyyy, el orden
	alfabético ascendente).

	Por ejemplo:
		>ls /etc/rc.d/rc5.d
	shows
		S01foo
		S15bar
		S99dummy
		K10makemyday
		K80whatthefuck
	(tenga cuidado, es sólo un ejemplo).
	Así cuando se ingresa al nivel de inicialización 5, primero llamará a
	S01foo, después a S15bar y luego a S99dummy.

	Cuando se deja el nivel de inicialización 5, es lo mismo para los archivos
	K*. Pero siguiendo el sistema, ellos son llamados en el mismo orden (por xx
	en forma ascendente), o en el orden inverso (por xx en modo descendente).

	Ahora usted sabe cómo todo este material trabaja, usted puede hacer los
	enlaces pertinentes a /etc/init.d en /etc/rc.d/rc5.d:
		>ln -s /etc/init.d/rc.adsl /etc/rc.d/rc5.d/S90adsl
		>ln -s /etc/init.d/rc.adsl /etc/rc.d/rc5.d/K90adsl

	Antes que usted cree estos enlaces, asegurese que el archivo S90 es
	relevante, esto significa que S90* será llamado después de todos los otros
	servicios de la red.
	Por supuesto, verifique que el archivo K90 es relevante. Según algunos
	sistemas (orden de muerte inverso) sería bueno usar K00, pero simplemente
	piense el mejor orden para matar la conexión.

	Una vez que todos los enlaces simbólicos (symlinks) se hayan realizado,
	usted puede probarlos mediante la reinicialización de su sistema.
	Usted también puede probarlos sin reiniciar: cierre su sesión de X11, abra
	una sesión root en modo consola, entonces:
		>init 3
	luego
		>init 5
	¿Entendido lo que hace? Significa dejando el nivel de inicialización 5 yendo
	al nivel de inicialización 3, luego, ¡regresando al nivel de inicialización
	5. ¡Los enlaces relevantes S* y K* serán llamados!


5.1
================================================================================
P:	No puedo conectar en modo consola, considerando que funciona bien bajo X11.

--------------------------------------------------------------------------------
R:	Éste puede ser un problema del framebuffer. Intente iniciar su Linux sin el
	soporte framebuffer para la consola.


5.2
================================================================================
P:	No puedo conectar bajo X11 considerando que funciona bien en el modo
	consola.

--------------------------------------------------------------------------------
R:	Vea 5.1.


5.3
================================================================================
P:	Cómo configuro adecuadamente el kernel a partir de los fuentes para obtener
	el soporte USB/PPP/N_HDLC

--------------------------------------------------------------------------------
R:	Digite los siguientes comandos:
		>cd /usr/src/linux
		>make -s menuconfig

		  --- General setup
		  [*] System V IPC
		  ..
		  USB support --->
		  <M> Support for USB
		  [ ]   USB verbose debug messages
		  --- Miscellaneous USB options
		  [*]   Preliminary USB device filesystem
		  [ ]   Enforce USB bandwidth allocation (EXPERIMENTAL)
		  [ ]   Long timeout for slow-responding devices (some MGE Ellipse UPSes)
		  --- USB Host Controller Drivers
		  < >   EHCI HCD (USB 2.0) support (EXPERIMENTAL)
		  <M>   UHCI (Intel PIIX4, VIA, ...) support
		  <M>   UHCI Alternate Driver (JE) support
		  <M>   OHCI (Compaq, iMacs, OPTi, SiS, ALi, ...) support
		  ..
		  --- USB Multimedia devices
		  ..
		  < >   DABUSB driver
		  ..

		  Character devices --->
		  ..
		  [*] Non-standard serial port support
		  <M>   HDLC line discipline support
		  ..

		  Network device support --->
		  ..
		  <M> PPP (point-to-point protocol) support
		  [ ] PPP multilink support (EXPERIMENTAL)
		  [ ] PPP filtering
		  <M> PPP support for async serial ports
		  <M> PPP support for sync tty ports
		  <M> PPP Deflate compression
		  <M> PPP BSD-Compress compression
		  < > PPP over Ethernet (EXPERIMENTAL)
		  < > PPP over ATM (EXPERIMENTAL)
		  ..

	Siguiendo el tipo de encapsulado de PPP que su proveedor utiliza, usted
	puede necesitar parámetros adicionales en su configuración del kernel.
	Usuarios PPPoE o IPoATM, por favor, refiéranse a 5.3.1.

	Una vez que el archivo de configuración del kernel ha sido guardado, digite:
		>make -s dep modules modules_install && depmod -a
	o el comando que usted use para compilar e instalar los módulos del kernel.
	Por supuesto su configuración completa del kernel debe ponerse
	adecuadamente, y el soporte de módulos cargables activado.
	También ejecute: `make -s bzImage` ¡si el soporte PPP o USB estuvieran
	previamente en el kernel en vez de módulos!
	Usted puede habilitar todos los drivers USB de controlador anfitrión pero
	debe utilizar sólo el mas relevante.
	El módulo DABUSB debe desactivarse.

	Actualice su paquete del modutils.

	Entonces usted debe ver estos módulos disponibles cuando usted digita el
	comando `modprobe -l`:
		usbcore
	y
		usb-uhci o ush-ohci o uhci
	y
		ppp_generic ppp_async ppp_synctty bsd_comp ppp_deflate
	y también
		n_hdlc

	Agregue las líneas siguientes en su /etc/modules.conf si es que ellas no existen:
		alias char-major-108 ppp_generic
		alias /dev/ppp ppp_generic
		alias tty-ldisc-3 ppp_async
		alias tty-ldisc-13 n_hdlc
		alias tty-ldisc-14 ppp_synctty
		alias ppp-compress-21 bsd_comp
		alias ppp-compress-24 ppp_deflate
		alias ppp-compress-26 ppp_deflate
		alias char-major-180 usbcore
	y
		alias usb-hostadapter usb-uhci
	o	alias usb-hostadapter usb-ohci
	o	alias usb-hostadapter uhci
	entonces
		>touch /etc/modules.conf /lib/modules/<kernel version>/modules.dep


5.3.1
================================================================================
P:	¿Cómo configuro adecuadamente el kernel a partir de los fuentes si yo
	utilizo PPPoE, Bridged Ethernet (RFC1483B) o IPoATM (RFC1483R)?

--------------------------------------------------------------------------------
R:	La configuración del kernel y principios de la compilación todavía son 
	los mismos (vea 5.3), pero usted necesita esto:

	  Network device support --->
	  ..
	  [*] Network device support
    	 ..
    	 <M> Universal TUN/TAP device driver support
    	 ..
	  <M> PPP (point-to-point protocol) support
	  ..
	  <M> PPP over ATM (EXPERIMENTAL)

	entonces digite:
		>mkdir /dev/net
		>mknod /dev/net/tun0 c 10 200
		>ln -s /dev/net/tun0 /dev/net/tun

	Usuarios PPPoE: lean el archivo INSTALL, hay algunos programas que ustedes
	necesitan instalar y configurar.


5.4
================================================================================
P:	¿No estoy en Francia, el driver funcionará con mi módem y mi ISP?

--------------------------------------------------------------------------------
R:	Verifique que su módem sea soportado. Si su módem no está en la lista de los
	módems soportados y no está en la lista de módems NO-soportados, preguntenos
	para ayuda adicional.

	Verifique que protocolo/encapsulado de PPP usa su proveedor bajo MS Windows.
	También verifique si el proveedor utiliza DHCP o si usted usa una dirección IP
	estática.
	Use eciadsl-config-tk o eciadsl-config-text para configurarlo adecuadamente.
	Si no es soportado por el driver o si usted no esta seguro, obtenga la
	última versión del driver o contáctenos.

	Para su Información:

		RFC1483 VC-MUX (o nulo) PPPoA == RFC2364 VC-MUX tambien conocido como
			RFC2364 PPPoATM Encapsulado NULO
		RFC1483 LLC PPPoA == RFC2364 LLC Routed
		RFC1483 BRIDGED ETH sin FCS (ethernet sobre ADSL, ETHoA o ETHoATM)
			es un modo usado tipicamente para encapsular tramas ethernet, y
			se usa a menudo en conjunto con PPPoE, ethernet IP estático, o DHCP
			sobre ADSL.
			Este modo es muy flexible, ya sea que su proveedor le da un nombre
			de usuario/password para conectar con PPPoE, o un IP estático y un
			gateway, o usted usa un cliente de DHCP para conectarse.
		VCM RFC1483 BRIDGED ETH es un modo similar a RFC1483 BRIDGED ETH with
			NO FCS pero no tan ampliamente usado.
		LLC RFC1483 ROUTED IP se usa para la transmision directa de datagramas
			IP sobre ADSL (IPoATM o IPoA) típicamente en el caso de IP
			estática, este protocolo requiere una IP estática y un gateway
			dados por su proveedor.
		VCM RFC1483 ROUTED IP es un modo similar a LLC RFC1483 ROUTED IP
			pero no tan ampliamente usado.

		SNAP es una capa mas de encapsulado, entre ethernet y LLC o entre IP y
		LLC.

		Para más información, por favor refiérase a la aplicación de control de
		su módem bajo MS Windows, debe indicar el modo utilizado, o refiérase a
		su proveedor.

		Usted también puede leer los documentos oficiales RFC1483 y RFC2364
		que describen estos modos en detalle.


5.6
================================================================================
P:	¿Puedo utilizar mi módem USB con un HUB USB2.0?

--------------------------------------------------------------------------------
R:	Nosotros hemos encontrado problemas cuando el módem u otro dispositivo 
	es conectado a un HUB USB 2.0 que no es actualmente soportado. 
	En la mayoría de los casos, el módulo correspondiente a HUB USB2.0 debe
	ser descargado. Por esta razón eciadsl-start descarga incondicionalmente 
	el módulo ehci-hcd cuando encuentra que está cargado.

	Si los puertos USB2.0 se administran por usb-uhci o usb-ohci, esto puede
	funcionar, hemos experimentado tales configuraciones funcionando.


5.7
================================================================================
P:	Poseo varios HUBS USB o varios dispositivos USB. ¿Es esto un problema?

--------------------------------------------------------------------------------
R:	Podría ser. Si usted no tiene éxito inicializando su módem, intente
	desactivar todo el hardware adicional los HUBS USB. Nosotros sólo 
	tenemos una experiencia parcial sobre esto.

	Muchos usuarios tienen webcams, ratones u otros dispositivos USB 
	agregados a su módem USB, ambos trabajando al mismo tiempo. Sin embargo
	esto no excluye los problemas con algún dispositivo USB. 
	A veces el orden de los dispositivos en los puertos USB también puede 
	ser un problema (a ser verificado). También pruebe su módem con todos 
	los otros dispositivos USB desconectados.

	Si usted tiene HUBS USB 1.1 y 2.0 juntos en su máquina, vea 5.6.


5.8
================================================================================
P:	Cambiando de Linux a MS Windows, no puedo usar mi módem y tengo que
	re-instalar el driver de MS Windows para que el modem trabaje 
	nuevamente.

--------------------------------------------------------------------------------
R:	Descargue todos los módulos USB relacionados a su módem al dejar Linux.
	Esto puede ser realizado automáticamente, quizás a través del 
	administrador de USB de su distribución de Linux, o usando init.d, 
	(piense en la descarga de módulos en cascada usando los comandos de 
	post-remove o pre-remove en el archivo /etc/modules.conf).

	Si todavía no funciona, usted tendrá que desconectar manualmente el 
	módem, esperar algunos segundos para que sea reinicializado, y luego 
	reconectelo.
	Ahora usted puede usarlo bajo MS Windows.

	Si el problema todavía persiste, avísenos.

	<POR HACER: problemas no verificados con los módems Ericsson y usermode 0.5>


5.8.1
================================================================================
P:	Cambiando de MS Windows a Linux, el eciadsl-start dice que el firmware ya 
	está cargado y falla.

--------------------------------------------------------------------------------
R:	Usted tiene que desconectar fisicamente el módem, esperar unos segundos
	para que este sea reseteado, luego reconéctelo. Ahora usted puede 
	reintentar el comando eciadsl-start.


5.9
================================================================================
P:	¿Puedo utilizar el driver en *BSD?

--------------------------------------------------------------------------------
R:	La versión 0.5 esta siendo portada a BSD, pero aún está en desarrollo.
	¡Todavía no esta soportado oficialmente!


5.10
================================================================================
P:	¿Puedo utilizar el driver bajo GNU/Hurd, Darwin, QNX o BeOS u otros
	sistemas?

--------------------------------------------------------------------------------
R:	Es sabido que el driver funciona en muchos sistemas GNU/Linux, incluso 
	en *BSD (actualmente en desarrollo) pero no es soportado para otros 
	sistemas basados en kernels no-linux.


5.11
================================================================================
P:	¿Cómo puedo detener la conexión PPP?

--------------------------------------------------------------------------------
R:	`eciadsl-stop`

	Si usted realmente quiere resetear su módem, descargue el módulo que 
	maneja su HUB USB (`modprobe -r usb-uhci` por ejemplo). 
	Pero esto puede llevar a otros problemas porque otros dispositivos de 
	USB pueden pertenecer a este módulo (!), y usted puede encontrar 
	problemas re-iniciando eciadsl-start (probablemente necesitará ejecutarlo
	dos veces o quizá deberá desconectar/reconectar su módem antes de 
	ejecutar `eciadsl-start`).


5.12
================================================================================
P:	¿Es posible ejecutar eciadsl-start como cualquier usuario?

--------------------------------------------------------------------------------
R:	Simplemente asegurese que sudo esta instalado en su sistema.
	Edite su archivo /etc/sudoers como usuario root y agregue la línea
	siguiente:
		username ALL=NOPASSWD:/usr/local/bin/eciadsl-start
	en donde el username es el nombre del usuario que usted quiere permitir
	ejecutar el eciadsl-start.
	Usted puede hacer esto para muchos usuarios o scripts diferentes, 
	agregando la misma línea para un username y el path/name del script que
	desea que ejecuten.
	Los usuarios autorizados ahora pueden ejecutar el eciadsl-start, digitando:
		>sudo eciadsl-start

	Vea también 5.13.


5.13
================================================================================
P:	Cuando ejecuto sudo eciadsl-start, obtengo este error:
	nice: pppd: no such file or directory

--------------------------------------------------------------------------------
R:	Check if pppd is installed on your system and try "su -" to get root
	privileges (instead of "su").

	En la consola o terminal como root, digite:
		>PATH="/sbin:/usr/sbin:/usr/local/sbin:$PATH" sudo eciadsl-start

	Si esto funciona, agregue la línea siguiente a su usuario normal 
	~/.bashrc o
	~/.profile (*):
		>export PATH="/sbin:/usr/sbin:/usr/local/sbin:$PATH"

	La próxima vez que usted abra un terminal (si usted ha modificado 
	~/.bashrc) o la proxima vez que Ud. realice login en la consola o en 
	el ambiente gráfico (si usted ha modificado ~/.profile), ejecutando 
	sudo eciadsl-start este debiera funcionar.

	Usted también puede crear un pequeño script que contenga el primer 
	comando, entonces, sólo tendrá que ejecutar este script. No se olvide
	de realizar chmod 777 a este script para que sea ejecutable.

	(*) Atención: 
	Dependiendo de la distribución de Linux que usted use, estos
	archivos pueden no existir o en su defecto otros sean utilizados.


5.14
================================================================================
P:	My provider use PPPoE (RFC1483, RFC2516), how can I manage to configure
	this?

--------------------------------------------------------------------------------
R:	First, don't use kernel's PPPoE support. It's experimental in 2.4 kernels
	and used by default in Debian kernel. Use rp-pppoe instead as a userland
	tool to manage the PPP connection. See 5.3.1 and 5.4.

	Of course, the PPP mode defined in the ECIADSL config must be one of the
	supported PPPoE modes (see `eciadsl-pppoeci --list`).

	You can find rp-pppoe here:
		http://www.roaringpenguin.com/pppoe

	Install and configure it so that it uses tap0 as ETH interface.

	Run eciadsl-start, then use rp-pppoe to enable the PPP line (see rp-pppoe
	documentation).


5.15
================================================================================
P:	My provider use a PPP mode I cannot find in the PPP modes list.
	What can I do?

--------------------------------------------------------------------------------
R:	Please contact us.


5.16
================================================================================
P:	I don't know which PPP mode is used by my provider.
	What can I do?

--------------------------------------------------------------------------------
R:	See 5.15.


5.17
================================================================================
P:	When EciAdsl is running, some apps like KDE run very slowly.
	What can I do?

--------------------------------------------------------------------------------
R:	Look at ifconfig command output if "lo" (loopback) interface is here.
	Add it if necessary (for example: ifconfig lo up).
