Si usted ha instalado el driver desde un paquete pre-compilado,
vaya directo a la sección de CONFIGURACIÓN.


COMPILACIÓN
===========

Todo lo que usted tiene que hacer es ejecutar en una consola o en
un terminal el script de configuración:
	./configure

Luego ejecute,
	make

Verifique con el comando ./configure --help la lista de opciones de
configuración.  Usted puede instalar el software en un directorio
diferente del predeterminado (/usr/local), usando por ejemplo la
opción "--prefix=/opt".

Los archivos de configuración del driver pueden ser instalados en un directorio
de su preferencia (el directorio predeterminado es /etc/eciadsl), usando las
opciones de configuración "--conf-prefix" (el valor predeterminado es /),
y "--conf-dir" (el valor predeterminado es eci/adsl).
Por ejemplo:
	--conf-prefix=/opt --conf-dir=etc/eciadsl
 o	--conf-prefix=/opt/eciadsl --conf-dir=etc

Tenga cuidado, la opción --etc-prefix tambien puede cambiarse
(por omisión es /, por eso se usa /etc),  esto se usa para obtener
los archivos de configuración del sistema tales como como resolv.conf
o el archivo de configuración del pppd.
Utilice la opción "--etc-prefix" sólo si usted está seguro de lo
que está haciendo.

Vea las otras opciones de configuración utilizando "./configure --help".


INSTALACIÓN
===========

Digite como usuario root:
	make install


CONFIGURACIÓN
=============

Aún como usuario root:
 - bajo X11, abra un terminal y ejecute el script eciadsl-config-tk
   (requiere tener instalado tcl/tk),
   si no lo tiene instalado en tonces en modo consola,
 - ejecute el script eciadsl-config-text

Digite lo siguiente:
	make cfg

automáticamente se ejecutará la herramienta de configuración.

Si usted encuentra problemas, pruebe usando el script eciadsl-doctor
y por favor lea el archivo TROUBLESHOOTING antes de solicitar
soporte en línea: -)

Si por alguna razon necesita cambiar manualmente los archivos de
configuración, verifique en el directorio /etc/eciadsl (o en su
defecto /opt/etc/eciadsl si es que usted utilizó la opción
"--etc-prefix=/opt" cuando ejecutó el script ./configure).
Vea también el archivo /etc/ppp/peers/asdl para las opciones del pppd
relacionadas con el driver instalado y /etc/ppp/chap-secrets si su ISP
espera la autenticación vía CHAP.


UTILIZACIÓN
===========

Como usuario root ejecute eciadsl-start, desde cualquier parte menos en el
directorio de los programas fuentes.

Para los usuarios de PPPoE:
Deben configurar y usar un cliente PPPoE normal tal como el rp-pppoe
(http://www.roaringpenguin.com/pppoe) para conectar su módem adsl a
través del dispositivo tap0.


DESINSTALACION
===============

Desde dentro el árbol de los archivos fuente (en modo consola o en
un terminal):
	make uninstall
y opcionalmente
	make distclean

Y en el caso de un archivo .rpm:
	rpm -e eciadsl-usermode
