Si usted ha instalado el driver desde un paquete pre-compilado,
vaya directo a la secci�n de CONFIGURACI�N.


COMPILACI�N
===========

Todo lo que usted tiene que hacer es ejecutar en una consola o en
un terminal el script de configuraci�n:
	./configure

Luego ejecute,
	make

Verifique con el comando ./configure --help la lista de opciones de
configuraci�n.  Usted puede instalar el software en un directorio
diferente del predeterminado (/usr/local), usando por ejemplo la
opci�n "--prefix=/opt".

Los archivos de configuraci�n del driver pueden ser instalados en un directorio
de su preferencia (el directorio predeterminado es /etc/eciadsl), usando las
opciones de configuraci�n "--conf-prefix" (el valor predeterminado es /),
y "--conf-dir" (el valor predeterminado es eci/adsl).
Por ejemplo:
	--conf-prefix=/opt --conf-dir=etc/eciadsl
 o	--conf-prefix=/opt/eciadsl --conf-dir=etc

Tenga cuidado, la opci�n --etc-prefix tambien puede cambiarse
(por omisi�n es /, por eso se usa /etc),  esto se usa para obtener
los archivos de configuraci�n del sistema tales como como resolv.conf
o el archivo de configuraci�n del pppd.
Utilice la opci�n "--etc-prefix" s�lo si usted est� seguro de lo
que est� haciendo.

Vea las otras opciones de configuraci�n utilizando "./configure --help".


INSTALACI�N
===========

Digite como usuario root:
	make install


CONFIGURACI�N
=============

A�n como usuario root:
 - bajo X11, abra un terminal y ejecute el script eciadsl-config-tk
   (requiere tener instalado tcl/tk),
   si no lo tiene instalado en tonces en modo consola,
 - ejecute el script eciadsl-config-text

Digite lo siguiente:
	make cfg

autom�ticamente se ejecutar� la herramienta de configuraci�n.

Si usted encuentra problemas, pruebe usando el script eciadsl-doctor
y por favor lea el archivo TROUBLESHOOTING antes de solicitar
soporte en l�nea: -)

Si por alguna razon necesita cambiar manualmente los archivos de
configuraci�n, verifique en el directorio /etc/eciadsl (o en su
defecto /opt/etc/eciadsl si es que usted utiliz� la opci�n
"--etc-prefix=/opt" cuando ejecut� el script ./configure).
Vea tambi�n el archivo /etc/ppp/peers/asdl para las opciones del pppd
relacionadas con el driver instalado y /etc/ppp/chap-secrets si su ISP
espera la autenticaci�n v�a CHAP.


UTILIZACI�N
===========

Como usuario root ejecute eciadsl-start, desde cualquier parte menos en el
directorio de los programas fuentes.

Para los usuarios de PPPoE:
Deben configurar y usar un cliente PPPoE normal tal como el rp-pppoe
(http://www.roaringpenguin.com/pppoe) para conectar su m�dem adsl a
trav�s del dispositivo tap0.


DESINSTALACION
===============

Desde dentro el �rbol de los archivos fuente (en modo consola o en
un terminal):
	make uninstall
y opcionalmente
	make distclean

Y en el caso de un archivo .rpm:
	rpm -e eciadsl-usermode
