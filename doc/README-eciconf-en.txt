===============================================================================

     Eciconf README - Linux ECI ADSL driver graphical configuration

              Written on 2002/05/05 by FlashCode (Sebastien Helleu)
              Translated on 2002/05/05 by Michel Stempin
              Corresponds to Eciconf version 0.1 / driver v0.5

===============================================================================


                    [1]  Eciconf description

                    [2]  Installation

                    [3]  Launching the program

                    [4]  The interface - The options to fill in

                    [5]  Modified files

                    [6]  Known bugs

                    [7]  TODO

                    [8]  How to report bugs

=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

[1] Eciconf description
-----------------------

The Eciconf graphical tool allows you to configure the main files required to
achieve an ADSL connection using the ECI HiFocus USB modem under Linux.

This tool's main target is for beginners who have difficulties in configuring
these file "by hand".
(there is no shame to begin, we all went through this step, me first! :)

=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

[2] Installation
----------------

Since version 0.5 of the driver, Eciconf is integrated into it, no other
installation is required anymore beside the driver.

For versions before 0.5, the tool is still available separately from:
http://eciadsl.sf.net  or here: http://flashcode.free.fr/linux

Eciconf requires Tcl/Tk.

=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

[3] Launching the program
-------------------------

Eciconf must be launched under the "root" user account, as the configuration
script modifies system files that require administrator privileges.

Eciconf is written in Tcl/Tk and thus must be launched under the X-window
(XFree86) graphical interface, from the directory where it is installed
(normally /usr/local/bin).

Launch Eciconf with the command:  ./eciconf.sh

=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

[4] The interface - The options to fill in
------------------------------------------

Explanation of the different fields / options to fill in:

1) Path to pppoeci:

    Path + executable name of pppoeci.
    This file is the driver base, it provides the communication between ppp and
    the modem.
    In case of doubt, don't modify the default path.

2) User name:

    This is the user name that has been given to you by your Internet service
    provider.
    Examples: for Wanadoo, this name looks like: fti/xxxxx@fti
              for Club Internet, this name looks like: xxxxx@clubadsl1

3) Password:

    This is the password that has been given to you by your Internet service
    provider.

4) Update Internet provider DNS:

    Check this box to update the DNS (see choice 5).
    If you uncheck this box, the server's IP addresses will not be updated in
    your configuration file.

5) DNS (Internet service providers list):

    Simply select your Internet service provider.
    You can also input manually your DNS servers if the default DNS are not
    correct or if your Internet service provider is not listed (if this is the
    case, please send me your Internet service provider and the DNS, so I can
    update the tool, by email: flashcode@free.fr)

6) VPI :

    This is the VPI that has been given to you by your Internet service
    provider (8 for France).

7) VCI :

    This is the VCI that has been given to you by your Internet service
    provider (35 for France).

8) Create config button:

    A click on this button creates the configuration files according to the
    given parameters (cf. options 1 to 5).
    A backup copy of the modified files is made before updating the files
    (see chapter "[5] Modified files").

    The file update is made by the makeconfig script, which can also be
    launched manually (see makeconfig source to know the parameters to
    provide).
    After executing this script, a window will open if the operation was
    successful (message on a green background), or if it failed (message on a
    red background).


9) Cancel button:

    A click on this button allows you to leave immediately Eciconf, without
    doing any modification into your files.

=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

[5] Modified files
------------------

The following files are modified by Eciconf:
(or created if they do not already exist)

1)  /etc/resolv.conf
    This file contains the DNS server's IP addresses for your Internet service
    provider.

2)  /etc/ppp/chap-secrets and /etc/ppp/pap-secrets
    These files contain your ADSL user name and password (both given to you by
    your Internet service provider).

3)  /etc/ppp/peers/adsl
    This script is used by ppp to connect to the Internet.
    It contains all the connection options and among other things, your ADSL
    user name and the path to "pppoeci".

==> Original files backups:
The modified files are renamed in {name}.bak (or .bak1, .bak2, etc. if the
.bak already exists).
Example: /etc/resolv.conf will be renamed into /etc/resolv.conf.bak

=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

[6] Known bugs
--------------

No bug known so far.
Nevertheless, check next chapter to see what remains to do.

=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

[7] TODO
--------

1) Modify the chap-secrets and pap-secrets files instead of creating new ones,
   as these files may already contain passwords that are used for other
   Internet service providers (example: for your old 56K modem).

2) Modify the adsl script instead of creating a new one:
   This will allow us to keep the options and use the script's last version,
   provided and installed with the driver.

=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

[8] How to report bugs
---------------------------

All the bugs found should be sent as soon as possible to one of the two persons
below:

  > FlashCode: flashcode@free.fr - http://flashcode.free.fr - AIM: helleuseb
                IRC: serveur=irc.openprojects.net, nick=FlashCode
  > Crevetor: ziva@caramail.com
                IRC: serveur=irc.openprojects.net, nick=Crevetor


=========================== END OF README.eciconf =============================
