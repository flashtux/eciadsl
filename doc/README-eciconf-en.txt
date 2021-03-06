===============================================================================

     Eciconf README - Linux ECI ADSL driver graphical configuration

              Written on 2002/05/05 by FlashCode (Sebastien Helleu)
              Translated on 2002/05/05 by Michel Stempin
              Last update on 2002/06/13 by FlashCode
              Corresponds to Eciconf version 0.6 / driver v0.6

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

1) User name:

    This is the user name that has been given to you by your Internet service
    provider.
    Examples: for Wanadoo, this name looks like: fti/xxxxx@fti
              for Club Internet, this name looks like: xxxxx@clubadsl1

2) Password:

    This is the password that has been given to you by your Internet service
    provider.

3) Update Internet provider DNS:

    Check this box to update the DNS (see choice 5).
    If you uncheck this box, the server's IP addresses will not be updated in
    your configuration file.

4) DNS (Internet service providers list):

    Simply select your Internet service provider.
    You can also input manually your DNS servers if the default DNS are not
    correct or if your Internet service provider is not listed (if this is the
    case, please send me your Internet service provider and the DNS, so I can
    update the tool, by email: flashcode@free.fr)

5) VPI :

    This is the VPI that has been given to you by your Internet service
    provider (8 for France).

6) VCI :

    This is the VCI that has been given to you by your Internet service
    provider (35 for France).

7) Modem selection :

    Choose your modem (this is for VendorID/ProductID) used in
    many driver scripts.

8) Selection of .bin :

    Choose another .bin if driver hangs up at a given block.

9) Create config button:

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

10) Change Synch .bin button:

    Only change the .bin file (does not save any other parameters).

11) Cancel button:

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

4)  /etc/eciadsl/vidpid
    This file contains VendorID/ProductID for your modem.

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

Nothing today.

=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

[8] How to report bugs
---------------------------

All the bugs found should be sent as soon as possible to one of the two persons
below:

  > FlashCode : flashcode@flashtux.org - http://www.flashtux.org - AIM: FlashCode AIM
                IRC: server = irc.openprojects.net, nick = FlashCode
  > wwp       : subcript@free.fr
                IRC: server = irc.openprojects.net, nick = wwp
  > Crevetor  : ziva@caramail.com
                IRC: server = irc.openprojects.net, nick = Crevetor


=========================== END OF README.eciconf =============================
