==============================================================================
                               E C I A D S L
==============================================================================

           This is EciAdsl version for Nortek 2021 modems.
           
           It is for these modems :
           - Nortek 2021
           - Siemens Santis 10
           - BT Voyager 100
		   - BT Voyager 105
           - Askey ALE130/150
           - LinkMax HSA 100 
           - D-Link DSL200 rev B
           - SMC 7003 V.2 USB
		   - IPM Datacom Dataway (may be supported)
		   - IPM Datacom SpeedWeb (may be supported)
		   - Xavi X7005Q2
		   - Kraun Adsl usb modem
           - Allnet ALL768UB
           - Turbocomm EA103
           
           It should work also for all GS7470 chipset modems
           
           ***** IMPORTANT *****
           
           INSTALLATION :
           	  follow standard eci installation instruction
           
           CONFIGURATION :
           
			       - configure your connection following doc instructions - [point 3: Configuration]

            SYNCH FILES :
           	 the bin files included in this package (directory GS7470_SynchFiles) are the only ones that 
           	 must be used for this driver. Try all the bin files using eciadsl-start or eciadsl-probe-synch script
           	 as reported in standard doc to find the one ok for you.

             - If synch or connection is not ok, please add parameters to your config for more debug infos :
	             * modify /etc/eciadsl/eciadsl.conf adding the following line :
	               PPPOECI_OPTIONS= -v 2
	             * Modify /etc/ppp/peers/adsl on [pty "/usr/local/bin/pppoeci ...] line adding parameter :
	               -v 2
			     * execute eciadsl-start saving its output
			     * save a copy of /tmp/eciadsl-pppoeci.log file (if exists)
	     		 * contact EciAdsl team (see eci support page)
	        	 * try also to make a sniff log (look at EciAdsl documentation following points 5.2 and 5.3)
	     		   Remember that for new chipset modems you must:
	     		   * Use your own win driver, skip any doc part telling you to install other win driver than yours
	     		   * Execute eciadsl-vendor-device.pl script adding -chipset=GS7470 param (do eciadsl-probe-device.pl -h for more help).
			              
           => Contact EciAdsl team with IRC for any problem:
              irc.freenode.net, channel: #eci