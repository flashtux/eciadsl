#!/usr/bin/wish
# ****************************************************************************
# *                                                                          *
# *   eciconf - Tcl/tk GUI for ECI Linux driver configuration (makeconfig)   *
# *   v0.6      by FlashCode and Crevetor (c) 2002/04/14                     *
# *                                                                          *
# *          For any support, contact one of us :                            *
# *           - FlashCode: flashcode@free.fr  http://flashcode.free.fr       *
# *           - Crevetor : ziva@caramail.com                                 *
# *                                                                          *
# ****************************************************************************
#
# 2002/11/03, FlashCode :
#   Added 3 providers : Infostrada, Sunrise, Econophone
# 2002/10/19, FlashCode :
#   Added Belgacom provider
# 2002/10/07, FlashCode :
#   Added modem Zyxel Prestige 630-41 and provider Bluewin
# 2002/10/05, FlashCode :
#   Added modems US Robotics 8500, BT Voyager, Xentrix USB
# 2002/06/13, FlashCode :
#   Added modem selection
# 2002/05/19, FlashCode :
#   Added .bin selection
# \
exec wish "$0" "$@" & exit 0

# <CONFIG>
set BIN_DIR "/usr/local/bin"
set ETC_DIR "/etc"
set CONF_DIR "/etc/eciadsl"
set PPPD_DIR "/etc/ppp"
set VERSION ""
# </CONFIG>

set titre_fenetre "ECI Linux driver configuration v$VERSION"

wm title . $titre_fenetre


#
# ===== Check is user is 'root' =====
#

catch {exec id -u} current_user
if {$current_user != 0} {
    frame .baduser
    message .baduser.texte -text "You must be root in order to run the ECI Linux driver configuration." -aspect 900 -padx 15 -pady 15
    button .baduser.ok -text { O K } -command {exit}
    pack .baduser.texte .baduser.ok -side top
    pack configure .baduser.ok -pady 15
    pack .baduser
    tkwait window .
    exit
}


# default values

set username "username@domain"
set password ""
set vpi "8"
set vci "35"
set dns1 ""
set dns2 ""
set vid1 ""
set pid1 ""
set vid2 ""
set pid2 ""
set use_dhcp "no"
set use_staticip "no"
set staticip ""
set gateway ""
set mode ""
set synch ""
set firm ""
set modem ""
set provider ""

# try to get previously defined values

set file -1
catch {set file [open "$CONF_DIR/eciadsl.conf" r]}
if {$file!=-1} {
	fconfigure $file -buffering line
	while {[eof $file]!=1} {
		set line [string trim [gets $file]]
		regsub -all {[\t]+} $line { } line
		set line [string trim $line]
		if {"$line"!="" && ![regexp {^#} $line]} {
			set pos1 0
			set pos2 [string first "=" "$line" [expr $pos1+1]]
			if {$pos2!=-1} {
				# skip trailing blanks
				set pos2 [expr $pos2-1]
			} else {
				set pos2 end
			}
			set entry [string trim [string range "$line" $pos1 $pos2]]
			set value [string trim [string range "$line" [expr $pos2+2] end]]
			switch $entry {
				PPPD_USER		{ set username "$value" }
				PPPD_PASSWD		{ set password "$value" }
				VID1			{ set vid1 "$value" }
				PID1			{ set pid1 "$value" }
				VID2			{ set vid2 "$value" }
				PID2			{ set pid2 "$value" }
				VPI				{ set vpi "$value" }
				VCI				{ set vci "$value" }
				MODE			{ set mode "$value" }
				SYNCH			{ set synch "$value" }
				FIRM			{ set firm "$value" }
				USE_DHCP		{ set use_dhcp "$value" }
				USE_STATICIP	{ set use_staticip "$value" }
				STATICIP		{ set staticip "$value" }
				GATEWAY			{ set gateway "$value" }
				MODEM			{ set modem "$value" }
				PROVIDER		{ set provider "$value" }
				DNS1			{ set dns1 "$value" }
				DNS2			{ set dns2 "$value" }
			}		
		}
	}
	close $file
}

#
# ===== modem logo =====
#

frame .bloc0

frame .bloc0.logo

image create photo modem_eci -file "$CONF_DIR/modemeci.gif"
label .bloc0.logo.picture -image modem_eci
bind .bloc0.logo.picture <Enter> {pushstate "ECI HiFocus USB ADSL modem"}
bind .bloc0.logo.picture <Leave> {popstate}
pack .bloc0.logo.picture

#
# ===== user and password =====
#

frame .bloc0.ident

frame .bloc0.ident.user
label .bloc0.ident.user.label -text {User:} -width 15 -anchor e
entry .bloc0.ident.user.edit -textvariable username -background lightblue -width 17
bind .bloc0.ident.user.edit <Enter> {pushstate "Enter your username and domain (given by your provider)"}
bind .bloc0.ident.user.edit <Leave> {popstate}
pack .bloc0.ident.user.label .bloc0.ident.user.edit -side left

frame .bloc0.ident.password
label .bloc0.ident.password.label -text {Password:} -width 15 -anchor e
entry .bloc0.ident.password.edit -show * -textvariable password -background lightblue -width 17
bind .bloc0.ident.password.edit <Enter> {pushstate "Enter your password (given by your provider)"}
bind .bloc0.ident.password.edit <Leave> {popstate}
pack .bloc0.ident.password.label .bloc0.ident.password.edit -side left

pack .bloc0.ident.user .bloc0.ident.password

#
# ===== VPI/VCI =====
#

frame .bloc0.call

frame .bloc0.call.vpi
label .bloc0.call.vpi.label -text {VPI:} -width 10 -anchor e
entry .bloc0.call.vpi.edit -textvariable vpi -background lightblue -width 4
bind .bloc0.call.vpi.edit <Enter> {pushstate "Virtual Path Identifier of your provider (8 for France)"}
bind .bloc0.call.vpi.edit <Leave> {popstate}
pack .bloc0.call.vpi.label .bloc0.call.vpi.edit -side left

frame .bloc0.call.vci
label .bloc0.call.vci.label -text {VCI:} -width 10 -anchor e
entry .bloc0.call.vci.edit -textvariable vci -background lightblue -width 4
bind .bloc0.call.vci.edit <Enter> {pushstate "Virtual Channel Identifier of your provider (35 for France)"}
bind .bloc0.call.vci.edit <Leave> {popstate}
pack .bloc0.call.vci.label .bloc0.call.vci.edit -side left

pack .bloc0.call.vpi .bloc0.call.vci

pack .bloc0 .bloc0.logo .bloc0.ident .bloc0.call -side left

pack .bloc0 -side top

#
# ===== provider DNS selection =====
#

frame .bloc1

frame .bloc1.espace1 -width 15

frame .bloc1.fai

frame .bloc1.fai.majdns
checkbutton .bloc1.fai.majdns.checkbox -text { Update provider DNS:} -command {invert_fai} -relief groove -background "#ffcc99" -width 28 -variable majfai -offvalue "non" -onvalue "oui" -selectcolor blue
bind .bloc1.fai.majdns.checkbox <Enter> {pushstate "Check this box if you want to update your DNS ($ETC_DIR/resolv.conf)"}
bind .bloc1.fai.majdns.checkbox <Leave> {popstate}
pack .bloc1.fai.majdns.checkbox -side left
pack .bloc1.fai.majdns  -side top -anchor w

# build the list of providers
list providers
set file [open "$CONF_DIR/providers.db" r]
fconfigure $file -buffering line
while {[eof $file]!=1} {
	set line [string trim [gets $file]]
	regsub -all {[\t]+} $line {^} line
	if {"$line"!="" && "$line"!="^" && ![regexp {^[ \t]*#} $line]} {
		set pos1 0
		set pos2 [string first "^" "$line" [expr $pos1+1]]
		set pos3 [string first "^" "$line" [expr $pos2+1]]
		set pos4 [string first "^" "$line" [expr $pos3+1]]
		if {$pos4!=-1} {
			# skip trailing blanks
			set pos4 [expr $pos4-1]
		} else {
			set pos4 end
		}
		lappend providers [string trim [string range "$line" $pos1 [expr $pos2-1]]]
		lappend providers [string trim [string range "$line" [expr $pos2+1] [expr $pos3-1]]]
		lappend providers [string trim [string range "$line" [expr $pos3+1] $pos4]]
	}
}
close $file
lappend providers "Other" "" ""

# fill the widget with the list of providers
frame .bloc1.fai.liste
listbox .bloc1.fai.liste.contenu -yscrollcommand {.bloc1.fai.liste.scroll set} -width 27 -height 7 -foreground black -selectbackground "#00ccff" -selectforeground black
.bloc1.fai.liste.contenu configure -exportselection 0
set i 0
set len [llength $providers]
while {$i<$len} {
	.bloc1.fai.liste.contenu insert end [lindex $providers $i]
	incr i 3
}
# previously defined provider set?
if {"$provider"!=""} {
	set selected_provider [expr [lsearch $providers $provider]/3]
	if {$selected_provider!=-1} {
		set dns1_other [lindex $providers [expr $selected_provider*3+1]]
		set dns2_other [lindex $providers [expr $selected_provider*3+2]]
		# if dns don't match the provider name
		if {"$dns1"!="$dns1_other" || "$dns2"!="$dns2_other"} {
			set selected_provider -1
		} else {
			.bloc1.fai.liste.contenu selection set $selected_provider
			.bloc1.fai.liste.contenu see [.bloc1.fai.liste.contenu curselection]
		}
	}
} else {
	set selected_provider -1
}
set dns1_other ""
set dns2_other ""
# no previous provider or bad one
if {"$provider"=="" || $selected_provider==-1} {
	# set Other provider if at least one DNS has been set
	if {"$dns1"!="" || "$dns2"!=""} {
		set provider "Other"
		.bloc1.fai.liste.contenu selection set [expr $len/3-1]
		.bloc1.fai.liste.contenu see [.bloc1.fai.liste.contenu curselection]
	}
}

scrollbar .bloc1.fai.liste.scroll -command ".bloc1.fai.liste.contenu yview"

pack .bloc1.fai.liste.contenu .bloc1.fai.liste.scroll -side left -fill y
pack .bloc1.fai.liste

frame .bloc1.fai.espacevertic -height 5
pack .bloc1.fai.espacevertic

frame .bloc1.fai.dns1
label .bloc1.fai.dns1.label -text "DNS 1: " -width 8
entry .bloc1.fai.dns1.entry -textvariable dns1 -background lightblue -width 15
pack .bloc1.fai.dns1.label .bloc1.fai.dns1.entry -side left
pack .bloc1.fai.dns1

frame .bloc1.fai.dns2
label .bloc1.fai.dns2.label -text "DNS 2: " -width 8
entry .bloc1.fai.dns2.entry -textvariable dns2 -background lightblue -width 15
pack .bloc1.fai.dns2.label .bloc1.fai.dns2.entry -side left -anchor e
pack .bloc1.fai.dns2

.bloc1.fai.liste.contenu configure -foreground darkgray -selectbackground lightgray -selectforeground darkgray
.bloc1.fai.dns1.entry configure -state disabled -foreground darkgray -background lightgray
.bloc1.fai.dns2.entry configure -state disabled -foreground darkgray -background lightgray
.bloc1.fai.dns1.label configure -foreground darkgray
.bloc1.fai.dns2.label configure -foreground darkgray
set majfai "non"
set fai_choisi ""

bind .bloc1.fai.liste.contenu <ButtonRelease> {select_provider}
bind .bloc1.fai.liste.contenu <Enter> {pushstate "Select a provider"}
bind .bloc1.fai.liste.contenu <Leave> {popstate}
bind .bloc1.fai.dns1 <Enter> {pushstate "\[OPTIONAL\] Enter your own primary DNS (given by your provider)"}
bind .bloc1.fai.dns1 <Leave> {popstate}
bind .bloc1.fai.dns2 <Enter> {pushstate "\[OPTIONAL\] Enter your own secondary DNS (given by your provider)"}
bind .bloc1.fai.dns2 <Leave> {popstate}


frame .bloc1.espace2 -width 15

#
# ===== modem selection =====
#

frame .bloc1.modem

label .bloc1.modem.libelle -text "Select your modem:" -relief groove -background "#ffcc99" -width 46
pack .bloc1.modem.libelle

# build the list of modems
list modems
set file [open "$CONF_DIR/modems.db" r]
fconfigure $file -buffering line
while {[eof $file]!=1} {
	set line [string trim [gets $file]]
	regsub -all {[\t]+} $line {^} line
	if {"$line"!="" && "$line"!="^" && ![regexp {^[ \t]*#} $line]} {
		set pos1 0
		set pos2 [string first "^" "$line" [expr $pos1+1]]
		set pos3 [string first "^" "$line" [expr $pos2+1]]
		set pos4 [string first "^" "$line" [expr $pos3+1]]
		set pos5 [string first "^" "$line" [expr $pos4+1]]
		set pos6 [string first "^" "$line" [expr $pos5+1]]
		if {$pos6!=-1} {
			# skip trailing blanks
			set pos6 [expr $pos6-1]
		} else {
			set pos6 end
		}
		lappend modems [string trim [string range "$line" $pos1 [expr $pos2-1]]]
		lappend modems [string trim [string range "$line" [expr $pos2+1] [expr $pos3-1]]]
		lappend modems [string trim [string range "$line" [expr $pos3+1] [expr $pos4-1]]]
		lappend modems [string trim [string range "$line" [expr $pos4+1] [expr $pos5-1]]]
		lappend modems [string trim [string range "$line" [expr $pos5+1] $pos6]]
	}
}
close $file
lappend modems "Other" "" "" "" ""

# fill the widget with the list of modems
frame .bloc1.modem.liste
listbox .bloc1.modem.liste.contenu -yscrollcommand {.bloc1.modem.liste.scroll set} -width 43 -height 7 -foreground black -selectbackground "#00ccff" -selectforeground black
.bloc1.modem.liste.contenu configure -exportselection 0
set i 0
set len [llength $modems]
while {$i<$len} {
	.bloc1.modem.liste.contenu insert end [lindex $modems $i]
	incr i 5
}
# previously defined modem set?
if {"$modem"!=""} {
	set selected_modem [expr [lsearch $modems $modem]/5]
	if {$selected_modem!=-1} {
		set vid1_other [lindex $modems [expr $selected_modem*5+1]]
		set pid1_other [lindex $modems [expr $selected_modem*5+2]]
		set vid2_other [lindex $modems [expr $selected_modem*5+3]]
		set pid2_other [lindex $modems [expr $selected_modem*5+4]]
		# if vidpid don't match the modem name
		if {"$vid1"!="$vid1_other" || "$pid1"!="$pid1_other" || "$vid2"!="$vid2_other" || "$pid2"!="$pid2_other"} {
			set selected_modem -1
		} else {
			.bloc1.modem.liste.contenu selection set $selected_modem
			.bloc1.modem.liste.contenu see [.bloc1.modem.liste.contenu curselection]
		}
	}
} else {
	set selected_modem -1
}
set vid1_other ""
set pid1_other ""
set vid2_other ""
set pid2_other ""
# no previous modem or bad one
if {"$modem"=="" || $selected_modem==-1} {
	# set Other provider if at least one VID/PID has been set
	if {"$vid1"!="" || "$pid1"!="" || "$vid2"!="" || "$pid2"!=""} {
		set modem "Other"
		.bloc1.modem.liste.contenu selection set [expr $len/5-1]
		.bloc1.modem.liste.contenu see [.bloc1.modem.liste.contenu curselection]
	}
}

scrollbar .bloc1.modem.liste.scroll -command ".bloc1.modem.liste.contenu yview"

pack .bloc1.modem.liste.contenu .bloc1.modem.liste.scroll -side left -fill y
pack .bloc1.modem.liste

frame .bloc1.modem.espacevertic -height 5
pack .bloc1.modem.espacevertic

frame .bloc1.modem.vidpid1
label .bloc1.modem.vidpid1.labelvid -text "VID 1: " -width 8
entry .bloc1.modem.vidpid1.entryvid -textvariable vid1 -background lightblue -width 7
pack .bloc1.modem.vidpid1.labelvid .bloc1.modem.vidpid1.entryvid -side left
label .bloc1.modem.vidpid1.labelpid -text "PID 1: " -width 8
entry .bloc1.modem.vidpid1.entrypid -textvariable pid1 -background lightblue -width 7
pack .bloc1.modem.vidpid1.labelpid .bloc1.modem.vidpid1.entrypid -side left
pack .bloc1.modem.vidpid1

frame .bloc1.modem.vidpid2
label .bloc1.modem.vidpid2.labelvid -text "VID 2: " -width 8
entry .bloc1.modem.vidpid2.entryvid -textvariable vid2 -background lightblue -width 7
pack .bloc1.modem.vidpid2.labelvid .bloc1.modem.vidpid2.entryvid -side left
label .bloc1.modem.vidpid2.labelpid -text "PID 2: " -width 8
entry .bloc1.modem.vidpid2.entrypid -textvariable pid2 -background lightblue -width 7
pack .bloc1.modem.vidpid2.labelpid .bloc1.modem.vidpid2.entrypid -side left
pack .bloc1.modem.vidpid2

.bloc1.modem.liste.contenu configure -foreground black -selectbackground "#00ccff" -selectforeground black
.bloc1.modem.vidpid1.entryvid configure -state normal -foreground darkgray -background lightgray
.bloc1.modem.vidpid1.entrypid configure -state normal -foreground darkgray -background lightgray
.bloc1.modem.vidpid2.entryvid configure -state normal -foreground darkgray -background lightgray
.bloc1.modem.vidpid2.entrypid configure -state normal -foreground darkgray -background lightgray
.bloc1.modem.vidpid1.labelvid configure -foreground darkgray
.bloc1.modem.vidpid1.labelpid configure -foreground darkgray
.bloc1.modem.vidpid2.labelvid configure -foreground darkgray
.bloc1.modem.vidpid2.labelpid configure -foreground darkgray

bind .bloc1.modem.liste.contenu <ButtonRelease> {select_modem}
bind .bloc1.modem.liste.contenu <Enter> {pushstate "Select a modem"}
bind .bloc1.modem.liste.contenu <Leave> {popstate}
bind .bloc1.modem.vidpid1 <Enter> {pushstate "These are the Vendor ID and Product ID of your modem (4-digit hexa)"}
bind .bloc1.modem.vidpid1 <Leave> {popstate}
bind .bloc1.modem.vidpid2 <Enter> {pushstate "These are the Vendor ID and Product ID of your modem once initialized (4-digit hexa)"}
bind .bloc1.modem.vidpid2 <Leave> {popstate}

frame .bloc1.espace3 -width 15

pack .bloc1.espace1 .bloc1.fai .bloc1.espace2 .bloc1.modem .bloc1.espace3 -side left -anchor n

pack .bloc1

frame .ligne_vide2 -height 20
pack .ligne_vide2

#
# ===== synch .bin selection =====
#

frame .bloc2

frame .bloc2.listebin

checkbutton .bloc2.listebin.checkbox -text { Change synch .bin file (only if driver hangs up):} -command {invert_bin} -relief groove -background "#ffcc99" -width 45 -variable majbin -offvalue "non" -onvalue "oui" -selectcolor blue
bind .bloc2.listebin.checkbox <Enter> {pushstate "Check this box if you want to change your synch .bin file"}
bind .bloc2.listebin.checkbox <Leave> {popstate}
set majbin "non"

label .bloc2.listebin.actuel -text "Current .bin: $synch" -relief sunken -width 48 -anchor w

frame .bloc2.listebin.liste
listbox .bloc2.listebin.liste.contenu -yscrollcommand ".bloc2.listebin.liste.scroll set" -width 45 -height 4 -foreground darkgray -selectbackground lightgray -selectforeground darkgray
.bloc2.listebin.liste.contenu configure -exportselection 0

proc add_bins {chemin} {
	global .bloc2.listebin

    set returncode [catch {exec find $chemin -name "*.bin" } bin_trouves]
    if {$returncode != 0} {
    } else {
        foreach bin $bin_trouves {
            if {![regexp "firmware" $bin] && [lsearch -glob [.bloc2.listebin.liste.contenu get 0 end] $bin] == -1} {
                .bloc2.listebin.liste.contenu insert end $bin
            }
        }
    }
}

add_bins "$CONF_DIR"
.bloc2.listebin.liste.contenu selection set 0

set bin_choisi [.bloc2.listebin.liste.contenu curselection]
set bin_initial $bin_choisi
bind .bloc2.listebin.liste.contenu <Enter> {pushstate "Choose another .bin (ONLY if driver hangs up into eci-load2 or LCP timeouts !)"}
bind .bloc2.listebin.liste.contenu <Leave> {popstate}
scrollbar .bloc2.listebin.liste.scroll -command ".bloc2.listebin.liste.contenu yview"
pack .bloc2.listebin.liste.contenu .bloc2.listebin.liste.scroll -side left -fill y

frame .bloc2.listebin.recherche
label .bloc2.listebin.recherche.texte -text {Search .bin here:} -width 15
set chemin_bin $CONF_DIR
entry .bloc2.listebin.recherche.chemin -textvariable chemin_bin -background "#CCEEEE" -width 27
bind .bloc2.listebin.recherche.chemin <Enter> {pushstate "Enter path for searching .bin files"}
bind .bloc2.listebin.recherche.chemin <Leave> {popstate}
image create bitmap bitmaptest -data "\
#define loupe_width 25 \
#define loupe_height 15 \
static unsigned char loupe_bits[] = { \
   0x00, 0xe0, 0x01, 0x00, 0x00, 0x10, 0x02, 0x00, 0x00, 0x08, 0x04, 0x00, \
   0x00, 0x04, 0x08, 0x00, 0x00, 0x04, 0x08, 0x00, 0x00, 0x04, 0x08, 0x00, \
   0x00, 0x04, 0x08, 0x00, 0x00, 0x08, 0x04, 0x00, 0x00, 0x1c, 0x02, 0x00, \
   0x00, 0xee, 0x01, 0x00, 0x00, 0x07, 0x00, 0x00, 0x80, 0x03, 0x00, 0x00, \
   0xc0, 0x01, 0x00, 0x00, 0xe0, 0x00, 0x00, 0x00, 0x40, 0x00, 0x00, 0x00 };"
button .bloc2.listebin.recherche.search -image bitmaptest -background "#99CCCC" -command {add_bins $chemin_bin}
bind .bloc2.listebin.recherche.search <Enter> {pushstate "Click to search .bin files (warning: can take a moment!)"}
bind .bloc2.listebin.recherche.search <Leave> {popstate}
pack .bloc2.listebin.recherche.texte .bloc2.listebin.recherche.chemin .bloc2.listebin.recherche.search -side left

pack .bloc2.listebin.checkbox
pack .bloc2.listebin.actuel
pack .bloc2.listebin.liste
pack .bloc2.listebin.recherche
pack .bloc2.listebin -padx 15

frame .bloc2.espace1 -width 5

#
# ===== PPP mode selection and advanced settings =====
#

frame .bloc2.modes

label .bloc2.modes.libelle -text "Select a PPP mode:" -relief groove -background "#ffcc99" -width 43
pack .bloc2.modes.libelle

list modes
set tmpfile "/tmp/eciconf.sh.tmp"
catch {exec $BIN_DIR/pppoeci --modes >& "$tmpfile"}
set file [open "$tmpfile" r]
fconfigure $file -buffering line
while {[eof $file]!=1} {
	set line [string trim [gets $file]]
	if {"$line"!=""} {
		set pos1 0
		set pos2 [string first " " "$line" [expr $pos1+1]]
		if {$pos2!=-1} {
			# skip trailing blanks
			set pos2 [expr $pos2-1]
		} else {
			set pos2 end
		}
		lappend modes [string trim [string range "$line" $pos1 $pos2]]
	}
}
close $file
catch {exec rm -f "$tmpfile"}

frame .bloc2.modes.liste
listbox .bloc2.modes.liste.contenu -yscrollcommand {.bloc2.modes.liste.scroll set} -width 40 -height 4 -foreground black -selectbackground "#00ccff" -selectforeground black
.bloc2.modes.liste.contenu configure -exportselection 0

set i 0
set len [llength $modes]
while {$i<$len} {
	.bloc2.modes.liste.contenu insert end [lindex $modes $i]
	incr i
}
set selected_mode [lsearch $modes $mode]
if {$selected_mode==-1} {
	set selected_mode 0
}
.bloc2.modes.liste.contenu selection set $selected_mode
.bloc2.modes.liste.contenu see [.bloc2.modes.liste.contenu curselection]

scrollbar .bloc2.modes.liste.scroll -command ".bloc2.modes.liste.contenu yview"

pack .bloc2.modes.liste.contenu .bloc2.modes.liste.scroll -side left -fill y

frame .bloc2.modes.espacevertic -height 5
pack .bloc2.modes.espacevertic

frame .bloc2.modes.advanced
checkbutton .bloc2.modes.advanced.use_dhcp -text {Use DHCP} -command {onclick_use_dhcp} -variable use_dhcp -offvalue "no" -onvalue "yes"
checkbutton .bloc2.modes.advanced.use_staticip -text {Use static IP} -command {onclick_use_staticip} -variable use_staticip -offvalue "no" -onvalue "yes"
pack .bloc2.modes.advanced.use_dhcp .bloc2.modes.advanced.use_staticip -side left

frame .bloc2.modes.staticip
label .bloc2.modes.staticip.label -text "IP: " -width 10
entry .bloc2.modes.staticip.edit -textvariable staticip -background lightblue -width 15
pack .bloc2.modes.staticip.label .bloc2.modes.staticip.edit -side left

frame .bloc2.modes.gateway
label .bloc2.modes.gateway.label -text "Gateway: " -width 10
entry .bloc2.modes.gateway.edit -textvariable gateway -background lightblue -width 15
pack  .bloc2.modes.gateway.label .bloc2.modes.gateway.edit -side left

proc onclick_use_staticip {} {
	global .bloc2.modes.staticip.edit .bloc2.modes.gateway.edit
	global .bloc2.modes.staticip.label .bloc2.modes.gateway.label
	global use_staticip use_dhcp

	if {"$use_staticip"=="yes"} {
		set use_dhcp "no"
		.bloc2.modes.staticip.edit configure -state normal -foreground black -background lightblue
		.bloc2.modes.gateway.edit configure -state normal -foreground black -background lightblue
		.bloc2.modes.staticip.label configure -foreground black
		.bloc2.modes.gateway.label configure -foreground black
		focus .bloc2.modes.staticip.edit
	} else {
		.bloc2.modes.staticip.edit configure -state disabled -background lightgray
		.bloc2.modes.gateway.edit configure -state disabled -background lightgray
		.bloc2.modes.staticip.label configure -foreground darkgray
		.bloc2.modes.gateway.label configure -foreground darkgray
	}
}

proc onclick_use_dhcp {} {
	global use_staticip use_dhcp
	
	if {"$use_dhcp"=="yes"} {
		set use_staticip "no"
		onclick_use_staticip
	}
}

onclick_use_dhcp
onclick_use_staticip

bind .bloc2.modes.liste.contenu <ButtonRelease> {select_mode}
bind .bloc2.modes.liste.contenu <Enter> {pushstate "Select a PPP mode/encapsulation (leave the first one selected if you don't know what it means)"}
bind .bloc2.modes.liste.contenu <Leave> {popstate}
bind .bloc2.modes.advanced.use_dhcp <Enter> {pushstate "Click here if you use DHCP to get an IP from your provider (MOST users should DISABLE this)"}
bind .bloc2.modes.advanced.use_dhcp <Leave> {popstate}
bind .bloc2.modes.advanced.use_staticip <Enter> {pushstate "Click here if you get a static IP from your provider (MOST users should DISABLE this)"}
bind .bloc2.modes.advanced.use_staticip <Leave> {popstate}
bind .bloc2.modes.staticip.edit <Enter> {pushstate "Your static IP (ONLY IF given by your provider)"}
bind .bloc2.modes.staticip.edit <Leave> {popstate}
bind .bloc2.modes.gateway.edit <Enter> {pushstate "Your provider's gateway IP (ONLY IF given by your provider)"}
bind .bloc2.modes.gateway.edit <Leave> {popstate}

pack .bloc2.modes.liste
pack .bloc2.modes.advanced .bloc2.modes.staticip .bloc2.modes.gateway

frame .bloc2.espace2 -width 15

pack .bloc2.listebin .bloc2.espace1 .bloc2.modes .bloc2.espace2 -side left -anchor n
pack .bloc2

frame .ligne_vide3 -height 15
pack .ligne_vide3


#
# ===== buttons =====
#

frame .boutons
button .boutons.create -text {Create config !} -width 15 -height 1 -command {run_makeconfig} -state disabled -background lightgray
bind .boutons.create <Enter> {pushstate "Save modifications then quit: select a modem first"}
bind .boutons.create <Leave> {popstate}

proc enable_create_button {} {
	global .boutons.create

	.boutons.create configure -state normal -background lightgreen
	bind .boutons.create <Enter> {pushstate "Save settings: write configuration files and set synch .bin if enabled, then quit"}
	bind .boutons.create <Leave> {popstate}
}

proc enable_vid_pid {} {
	global .bloc1.modem.vidpid1.entryvid .bloc1.modem.vidpid1.entrypid
	global .bloc1.modem.vidpid2.entryvid .bloc1.modem.vidpid2.entrypid
	global .bloc1.modem.vidpid1.labelvid .bloc1.modem.vidpid1.labelpid
	global .bloc1.modem.vidpid2.labelvid .bloc1.modem.vidpid2.labelpid

	.bloc1.modem.vidpid1.entryvid configure -state normal -foreground black -background lightblue
	.bloc1.modem.vidpid1.entrypid configure -state normal -foreground black -background lightblue
	.bloc1.modem.vidpid2.entryvid configure -state normal -foreground black -background lightblue
	.bloc1.modem.vidpid2.entrypid configure -state normal -foreground black -background lightblue
	.bloc1.modem.vidpid1.labelvid configure -foreground black
	.bloc1.modem.vidpid1.labelpid configure -foreground black
	.bloc1.modem.vidpid2.labelvid configure -foreground black
	.bloc1.modem.vidpid2.labelpid configure -foreground black
}

# if all VID/PID are already set, enable the CREATE button
if {"$modem"!="" && "$vid1"!="" && "$pid1"!="" && "$vid2"!="" && "$pid2"!=""} {
	enable_vid_pid
	enable_create_button
}


frame .boutons.espace -width 20
button .boutons.dabusb -text {Remove Dabusb} -background "#ffccff" -command {run_dabusb} -padx 10
bind .boutons.dabusb <Enter> {pushstate "Unplug your modem first and then click on this button to remove dabusb"}
bind .boutons.dabusb <Leave> {popstate}

frame .boutons.espace1 -width 20
button .boutons.set_bin -text {Change synch .bin} -width 15 -height 1 -command {run_makeconfig_synch} -state disabled -background lightgray
bind .boutons.set_bin <Enter> {pushstate "Only change current synch .bin: enable it above and select a .bin"}
bind .boutons.set_bin <Leave> {popstate}

frame .boutons.espace2 -width 20
button .boutons.cancel -text {Cancel} -background "#ffbbbb" -width 15 -height 1 -command {exit}
bind .boutons.cancel <Enter> {pushstate "Quit without saving"}
bind .boutons.cancel <Leave> {popstate}

pack .boutons.create .boutons.espace .boutons.dabusb .boutons.espace1 .boutons.set_bin .boutons.espace2 .boutons.cancel -side left
pack .boutons

frame .ligne_vide4 -height 15
pack .ligne_vide4

label .state -textvariable statetext -borderwidth 2 -relief sunken -anchor w
pack .state -fill x
set statetext "Ready."


proc conf_report {exit msg color sortie} {
	global titre_fenetre

    if {"$exit"=="oui"} {
		catch {destroy .confok}
	}
	toplevel .confok
    wm title .confok $titre_fenetre
    message .confok.texte -text "$msg" -aspect 600 -padx 15 -pady 15
	
    if {"$sortie"!=""} {
	    message .confok.sortie -text "$sortie" -aspect 600 -padx 15 -pady 15 -background $color
	} else {
	    message .confok.sortie
	}

    if {"$exit"=="oui"} {
	    button .confok.ok -text { Quit } -command {exit}
	} else {
	    button .confok.ok -text { Close } -command {destroy .confok}
	}

    pack .confok.texte .confok.sortie .confok.ok -side top
    pack configure .confok.ok -pady 15
    grab set .confok
    tkwait window .confok
}

#
# ===== calls to makeconfig =====
#

proc run_makeconfig_synch {} {
	global BIN_DIR majbin

    if {[string compare $majbin "oui"] == 0} {
        set numero_bin_choix [.bloc2.listebin.liste.contenu curselection]
        set synch [.bloc2.listebin.liste.contenu get $numero_bin_choix]
	    set returncode [catch {exec $BIN_DIR/makeconfig --synch "$synch"} sortie]
    	if {$returncode != 0} {
	        conf_report "non" "Synch .bin has NOT been set.\n\nThis is the error from makeconfig:" "#ffbbbb" $sortie
    	} else {
	        conf_report "non" "Synch .bin has been changed and will\nbe used the next time you run startmodem." lightgreen ""
    	}
	}
}

proc run_makeconfig {} {
	global majfai majbin BIN_DIR synch firm
	global mode use_staticip use_dhcp staticip gateway modem provider
	global username password dns1 dns2 vpi vci vid1 pid1 vid2 pid2

    if {[string compare $majbin "oui"] == 0} {
        set synch [.bloc2.listebin.liste.contenu get [.bloc2.listebin.liste.contenu curselection]]
    }

    set returncode [catch {exec $BIN_DIR/makeconfig "$mode" "$username" "$password" "$BIN_DIR/pppoeci" "$dns1" "$dns2" $vpi $vci "$vid1$pid1" "$vid2$pid2" "$synch" "$firm" "$staticip" "$gateway" "$use_staticip" "$use_dhcp" "$modem" "$provider"} sortie]
    if {$returncode != 0} {
        conf_report "non" "Makeconfig did not update your files.\n\nThis is the error:" "#ffbbbb" $sortie
    } else {
        conf_report "oui" "Configuration files updated with success!\n\nThis is makeconfig output:" lightgreen $sortie
    }
}

#
# ===== call to remove_dabusb =====
#

proc run_dabusb {} {
    set returncode [catch {exec remove_dabusb} sortie]
    toplevel .dab
    wm title .dab "dabusb"
    if {$returncode != 0} {
        set msg "Dabusb couldn't be removed...\n\nThe error is:"
        set vmtitle "Error"
        set color "#ffbbbb"
    } else {
        set msg "Dabusb succesfully removed.\n\nThis is dabusb output:"
        set vmtitle "OK"
        set color lightgreen
    }
    message .dab.texte -text "$msg" -aspect 600 -padx 15 -pady 15
    message .dab.sortie -text "$sortie" -aspect 600 -padx 15 -pady 15 -background $color
    button .dab.ok -text { O K } -command {destroy .dab}
    pack .dab.texte .dab.sortie .dab.ok -side top
    pack configure .dab.ok -pady 15
    grab set .dab
    tkwait window .dab
}

#
# ===== display field help =====
#

proc pushstate {msg} {
	global statetext statepile topstatepile

    if {[info exists topstatepile] == 0} {
        set topstatepile 0
    }
    incr topstatepile 1
    set statepile{$topstatepile} $msg
    set statetext $msg
}

proc popstate {} {
	global statetext statepile topstatepile

    if {[info exists topstatepile] != 0} {
        if {$topstatepile > 1} {
            unset statepile($topstatepile)
            incr topstatepile -1
            set statetext $statepile($topstatepile)
        } else {
            set topstatepile 0
            set statetext "Ready."
        }
    }
}

#
# ===== invert fai state (enabled/disabled) =====
#

proc invert_fai {} {
	global majfai fai_choisi providers

    if {"$majfai"=="oui"} {
    	.bloc1.fai.liste.contenu configure -foreground black -selectbackground "#00ccff" -selectforeground black
		if {$fai_choisi!=""} {
	    	.bloc1.fai.dns1.entry configure -state normal -foreground black -background lightblue
			.bloc1.fai.dns2.entry configure -state normal -foreground black -background lightblue
	    	.bloc1.fai.dns1.label configure -foreground black
	    	.bloc1.fai.dns2.label configure -foreground black
			.bloc1.fai.liste.contenu selection clear 0 [expr [llength $providers]-1]
			.bloc1.fai.liste.contenu selection set $fai_choisi
		} else {
			if {[.bloc1.fai.liste.contenu curselection]==""} {
				.bloc1.fai.liste.contenu selection set 0
			}
		}
		select_provider
    } else {
        set fai_choisi [.bloc1.fai.liste.contenu curselection]
        .bloc1.fai.liste.contenu configure -foreground darkgray -selectbackground lightgray -selectforeground darkgray
        .bloc1.fai.dns1.entry configure -state disabled -foreground darkgray -background lightgray
        .bloc1.fai.dns2.entry configure -state disabled -foreground darkgray -background lightgray
        .bloc1.fai.dns1.label configure -foreground darkgray
        .bloc1.fai.dns2.label configure -foreground darkgray
    }
}

#
# ===== invert .bin files (enabled/disabled) =====
#

proc invert_bin {} {
	global majbin bin_choisi bin_initial
	global .boutons.set_bin

    if {[string compare $majbin "oui"] == 0} {
        if {[.bloc2.listebin.liste.contenu size] > 0} {
            if {[string compare $bin_choisi ""] == 0} {
                .bloc2.listebin.liste.contenu selection set 0
                set bin_choisi [.bloc2.listebin.liste.contenu curselection]
            }
            .bloc2.listebin.liste.contenu configure -foreground black -selectbackground "#00ccff" -selectforeground black
            .bloc2.listebin.liste.contenu selection clear 0 end
            .bloc2.listebin.liste.contenu selection set $bin_choisi
            .bloc2.listebin.liste.contenu see $bin_choisi
			.boutons.set_bin configure -state normal -background lightblue
			bind .boutons.set_bin <Enter> {pushstate "Only change current synch .bin with the selected one"}
			bind .boutons.set_bin <Leave> {popstate}
        } else {
            set majbin "non"
        }
    } else {
        .bloc2.listebin.liste.contenu configure -foreground darkgray -selectbackground lightgray -selectforeground darkgray
        set bin_choisi [.bloc2.listebin.liste.contenu curselection]
        if {[string compare $bin_choisi ""] == 0} {
            set bin_choisi $bin_initial
        }
		.boutons.set_bin configure -state disabled -background lightgray
		bind .boutons.set_bin <Enter> {pushstate "Only change current synch .bin: enable it above and select a .bin"}
		bind .boutons.set_bin <Leave> {popstate}
    }
}

proc select_modem {} {
	global .bloc1.modem.liste.contenu vid1_other pid1_other vid2_other pid2_other
	global modems vid1 pid1 vid2 pid2 modem

	# save VID/PID for Other if it was the previous selection
	if {"$modem"=="Other"} {
		set vid1_other "$vid1"
		set pid1_other "$pid1"
		set vid2_other "$vid2"
		set pid2_other "$pid2"
	}
	set index [.bloc1.modem.liste.contenu curselection]
	if {"$index"!=""} {
		# update variables from the selection
		set modem [.bloc1.modem.liste.contenu get $index]
		set vid1 [lindex $modems [expr $index*5+1]]
		set pid1 [lindex $modems [expr $index*5+2]]
		set vid2 [lindex $modems [expr $index*5+3]]
		set pid2 [lindex $modems [expr $index*5+4]]
		# if Other has been selected, restore VID/PID if exist:
		# got from stored .conf at startup or if previously selected
		if {"$modem"=="Other"} {
		 	if {"$vid1_other"!=""} {
				set vid1 "$vid1_other"
			}
		 	if {"$pid1_other"!=""} {
				set pid1 "$pid1_other"
			}
		 	if {"$vid2_other"!=""} {
				set vid2 "$vid2_other"
			}
		 	if {"$pid2_other"!=""} {
				set pid2 "$pid2_other"
			}
		}
		enable_vid_pid
		enable_create_button
		focus .bloc1.modem.vidpid1.entryvid
	}
}

proc select_provider {} {
	global .bloc1.fai.liste.contenu .bloc1.fai.dns1.entry
	global providers dns1 dns2 majfai provider dns1_other dns2_other

	# save DNS for Other if it was the previous selection
	if {"$provider"=="Other"} {
		set dns1_other "$dns1"
		set dns2_other "$dns2"
	}
	# if selection is valid (should be)
	set index [.bloc1.fai.liste.contenu curselection]
	if {"$index"!=""} {
		# update variables from the selection
	    set provider [.bloc1.fai.liste.contenu get $index]
		set dns1 [lindex $providers [expr $index*3+1]]
		set dns2 [lindex $providers [expr $index*3+2]]
		# if Other has been selected, restore DNS if exist:
		# got from stored .conf at startup or if previously selected
		if {"$provider"=="Other"} {
		 	if {"$dns1_other"!=""} {
				set dns1 "$dns1_other"
			}
		 	if {"$dns2_other"!=""} {
				set dns2 "$dns2_other"
			}
		}
		if {"$majfai"=="oui"} {
			.bloc1.fai.dns1.entry configure -state normal -foreground black -background lightblue
			.bloc1.fai.dns2.entry configure -state normal -foreground black -background lightblue
			.bloc1.fai.dns1.label configure -foreground black
			.bloc1.fai.dns2.label configure -foreground black
			focus .bloc1.fai.dns1.entry
		}
	}
}

proc select_mode {} {
	global .bloc2.modes.liste.contenu modes mode

	set index [.bloc2.modes.liste.contenu curselection]
	if {"$index"!=""} {
		set mode [lindex $modes $index]
	}
}
