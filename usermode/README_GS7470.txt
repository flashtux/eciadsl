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
           
           It should work also for all GS7470 chipset modems
           
           ***** IMPORTANT *****
           
           INSTALLATION :
           	  follow standard eci installation instruction
           
           CONFIGURATION :
           
			       - configure your connection following doc instructions - [point 3: Configuration]

           SYNCH FILES :
           	 the bin files included in this package (directory GS7470_SynchFiles) are the only ones that 
           	 must be used for this driver here is the list with modem supported and params needed for it
           	 if you do not have success using the one specified for your modem pls try all the other.
           
             - gs7470_synch01.bin 
               (gs7470_synch01a.bin and gs7470_synch01b.bin are part of it, put them all in the /etc/eciadsl directory) 
               + for modems : 	Nortek 2021 (see also gs7470_synch08.bin & gs7470_synch12.bin)
               					BT Voyager 100
               					Siemens Santis 10
                                Askey ALE130
               
             - gs7470_synch02.bin
               (gs7470_synch02a.bin and gs7470_synch02b.bin are part of it put them all in the /etc/eciadsl directory) 
               + for modem  :	LinkMax HSA 100
 
             - gs7470_synch03.bin
               + for modem  :	BT Voyager 105

            - gs7470_synch04.bin 
               + for modems : 	D-Link DSL200 rev B (see also gs7470_synch06.bin)
               					Xavi X7005Q2
                                Askey ALE150
         
            - gs7470_synch05.bin 
               + for modems : 	IPM Datacom Dataway

            - gs7470_synch06.bin 
               + for modems : 	D-Link DSL200 rev B (see also gs7470_synch04.bin)
               + non special parameters needed.

            - gs7470_synch07.bin 
               + for modems : 	SMC 7003 V.2 USB

            - gs7470_synch08.bin 
               + for modems : 	Nortek 2021 (see also gs7470_synch01.bin & gs7470_synch12.bin)
                                Allnet ALL768UB

            - gs7470_synch09.bin 
               + for modems : 	IPM Datacom SpeedWeb (see also gs7470_synch11.bin)

            - gs7470_synch10.bin 
               + for modems : 	Kraun Adsl usb modem

            - gs7470_synch11.bin 
               + for modems : 	IPM Datacom SpeedWeb (see also gs7470_synch09.bin)

            - gs7470_synch12.bin 
               + for modems : 	Nortek 2021 (see also gs7470_synch01.bin & gs7470_synch08.bin)

			 - If you cannot get connection with the bin suggested please try also all the other

             - If synch or connection is not ok, please add parameters to your config for more debug infos :
	             * modify /etc/eciadsl/eciadsl.conf adding the following line :
	               PPPOECI_OPTIONS= -v 2
	             * Modify /etc/ppp/peers/adsl on [pty "/usr/local/bin/pppoeci ...] line adding parameter :
	               -v 2
			     * execute startmodem saving its output
			     * save a copy of /tmp/pppoeci.log file (if exists)
	     		 * contact EciAdsl team (see eci support page)
	        	 * try also to make a sniff log (look at EciAdsl documentation following points 5.2 and 5.3)
	     		   [remember that for new chipset modems you must use your own win driver]
			              

           => Contact EciAdsl team with IRC for any problem:
              irc.freenode.net, channel: #eci