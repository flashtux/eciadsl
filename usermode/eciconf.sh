#!/bin/sh
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
# 2002/05/19, FlashCode :
#   Added .bin selection feature
# \
exec wish "$0" "$@" & exit 0

set titre_fenetre "ECI Linux driver configuration v0.6"

wm title . $titre_fenetre

#
# ===== Check is user is 'root' =====
#

catch {exec whoami} current_user
if {[string compare $current_user "root"] != 0} {
    frame .baduser
    message .baduser.texte -text "You must be root in order to run the ECI Linux driver configuration." -aspect 900 -padx 15 -pady 15
    button .baduser.ok -text { O K } -command {exit}
    pack .baduser.texte .baduser.ok -side top
    pack configure .baduser.ok -pady 15
    pack .baduser
    tkwait window .
    exit
}

#
# ===== Remove dabusb section =====
#

frame .dabusb
message .dabusb.texte -text "If your modem is running when you start Linux,\nclick here after unplugging your modem :" -aspect 600 -anchor w
button .dabusb.remove -text {Remove Dabusb} -background "#ffccff" -command {run_dabusb} -padx 10
frame .dabusb.espace -width 6
bind .dabusb.remove <Enter> {pushstate "Unplug your modem first and then click on this button to remove dabusb"}
bind .dabusb.remove <Leave> {popstate}
pack .dabusb.texte .dabusb.remove .dabusb.espace -side left

pack .dabusb -padx 10 -pady 15 -side top

#
# ===== pppoeci path, user and password =====
#

frame .frame1
label .frame1.label_chemin -text { Path to pppoeci :} -width 15 -anchor e
set path_pppoeci "/usr/local/bin/pppoeci"
entry .frame1.chemin -textvariable path_pppoeci -background lightblue -width 35
bind .frame1.chemin <Enter> {pushstate "Enter path to run pppoeci (in case of doubt, don't modify this path)"}
bind .frame1.chemin <Leave> {popstate}
pack .frame1.label_chemin .frame1.chemin -side left
pack configure .frame1.chemin -padx 15
pack .frame1 -padx 15 -pady 3

frame .frame2
label .frame2.label_user -text {User :} -anchor e
set username "username@domain"
entry .frame2.user -textvariable username -background lightblue -width 17
bind .frame2.user <Enter> {pushstate "Enter your username and domain (given by your provider)"}
bind .frame2.user <Leave> {popstate}
pack .frame2.label_user .frame2.user -side left
pack configure .frame2.user -padx 10

label .frame2.label_password -text { Password :} -anchor e
set password ""
entry .frame2.password -show * -textvariable password -background lightblue -width 13
bind .frame2.password <Enter> {pushstate "Enter your password (given by your provider)"}
bind .frame2.password <Leave> {popstate}
pack .frame2.label_password .frame2.password -side left
pack configure .frame2.password -padx 10
pack .frame2 -padx 15 -pady 6

frame .ligne_vide1 -height 15
pack .ligne_vide1

#
# ===== Internet provider DNS =====
#

frame .bloc1

frame .bloc1.fai

frame .bloc1.fai.majdns
frame .bloc1.fai.majdns.espace -width 30
checkbutton .bloc1.fai.majdns.checkbox -text { Update internet provider DNS :} -command {invert_majdns} -relief groove -background "#ffcc99" -width 34 -variable majdns -offvalue "non" -onvalue "oui" -selectcolor blue
bind .bloc1.fai.majdns.checkbox <Enter> {pushstate "Check this box if you want to update your DNS (/etc/resolv.conf)"}
bind .bloc1.fai.majdns.checkbox <Leave> {popstate}
pack .bloc1.fai.majdns.espace .bloc1.fai.majdns.checkbox -side left
set majdns "oui"

frame .bloc1.fai.ligne1
frame .bloc1.fai.ligne1.espace -width 30
radiobutton .bloc1.fai.ligne1.wanadoo -text "Wanadoo" -width 12 -variable fai -value wanadoo -command {set dns1 "193.252.19.3"; set dns2 "193.252.19.4"} -padx 5 -selectcolor blue
.bloc1.fai.ligne1.wanadoo configure -anchor w
radiobutton .bloc1.fai.ligne1.clubinternet -text "Club Internet" -width 12 -variable fai -value clubinternet -command {set dns1 "194.117.200.15"; set dns2 "194.117.200.10"} -padx 5 -selectcolor blue
.bloc1.fai.ligne1.clubinternet configure -anchor w
pack .bloc1.fai.ligne1.espace .bloc1.fai.ligne1.wanadoo .bloc1.fai.ligne1.clubinternet -side left

frame .bloc1.fai.ligne2
frame .bloc1.fai.ligne2.espace -width 30
radiobutton .bloc1.fai.ligne2.neuftelecom -text "9 Telecom" -width 12 -variable fai -value 9telecom -command {set dns1 "212.30.96.108"; set dns2 "213.203.124.146"} -padx 5 -selectcolor blue
.bloc1.fai.ligne2.neuftelecom configure -anchor w
radiobutton .bloc1.fai.ligne2.cegetel -text "Cegetel" -width 12 -variable fai -value cegetel -command {set dns1 "194.6.128.3"; set dns2 "194.6.128.4"} -padx 5 -selectcolor blue
.bloc1.fai.ligne2.cegetel configure -anchor w
pack .bloc1.fai.ligne2.espace .bloc1.fai.ligne2.neuftelecom .bloc1.fai.ligne2.cegetel -side left

frame .bloc1.fai.ligne3
frame .bloc1.fai.ligne3.espace -width 30
radiobutton .bloc1.fai.ligne3.worldonline -text "World Online" -width 12 -variable fai -value worldonline -command {set dns1 "212.83.128.3"; set dns2 "212.83.128.4"} -padx 5 -selectcolor blue
.bloc1.fai.ligne3.worldonline configure -anchor w
radiobutton .bloc1.fai.ligne3.telecomitalia -text "Telecom Italia" -width 12 -variable fai -value telecomitalia -command {set dns1 "212.216.112.112"; set dns2 "212.216.172.62"} -padx 5 -selectcolor blue
.bloc1.fai.ligne3.telecomitalia configure -anchor w
pack .bloc1.fai.ligne3.espace .bloc1.fai.ligne3.worldonline .bloc1.fai.ligne3.telecomitalia -side left

frame .bloc1.fai.ligne4
frame .bloc1.fai.ligne4.espace -width 30
radiobutton .bloc1.fai.ligne4.tiscali -text "Tiscali Italia" -width 12 -variable fai -value tiscali -command {set dns1 "195.130.224.18"; set dns2 "195.130.225.129"} -padx 5 -selectcolor blue
.bloc1.fai.ligne4.tiscali configure -anchor w
radiobutton .bloc1.fai.ligne4.pipexuk -text "Pipex UK" -width 12 -variable fai -value pipexuk -command {set dns1 "158.43.240.4"; set dns2 "158.43.240.3"} -padx 5 -selectcolor blue
.bloc1.fai.ligne4.pipexuk configure -anchor w
pack .bloc1.fai.ligne4.espace .bloc1.fai.ligne4.tiscali .bloc1.fai.ligne4.pipexuk -side left

pack .bloc1.fai.majdns .bloc1.fai.ligne1 .bloc1.fai.ligne2 .bloc1.fai.ligne3 .bloc1.fai.ligne4 -side top -anchor w

set fai wanadoo

frame .bloc1.fai.espacedns -width 30
label .bloc1.fai.labeldns -text "DNS : " -width 6
entry .bloc1.fai.dns1 -textvariable dns1 -background lightblue -width 14
entry .bloc1.fai.dns2 -textvariable dns2 -background lightblue -width 14
pack .bloc1.fai.espacedns .bloc1.fai.labeldns .bloc1.fai.dns1 .bloc1.fai.dns2 -side left -anchor e
bind .bloc1.fai.dns1 <Enter> {pushstate "\[OPTIONAL\] Enter your own primay DNS (given by your provider)"}
bind .bloc1.fai.dns1 <Leave> {popstate}
bind .bloc1.fai.dns2 <Enter> {pushstate "\[OPTIONAL\] Enter your own secondary DNS (given by your provider)"}
bind .bloc1.fai.dns2 <Leave> {popstate}

set dns1 "193.252.19.3"
set dns2 "193.252.19.4"

frame .bloc1.espace1 -width 30

#
# ===== Modem selection =====
#

frame .bloc1.modem

label .bloc1.modem.libelle -text "Your modem :" -relief groove -background "#ffcc99" -width 25
pack .bloc1.modem.libelle

radiobutton .bloc1.modem.eci -text "ECI HiFocus/B-Focus" -width 18 -variable modem -value eci -command {set vidpid1 "05472131"; set vidpid2 "09158000"} -padx 10 -selectcolor blue
.bloc1.modem.eci configure -anchor w
pack .bloc1.modem.eci

radiobutton .bloc1.modem.eicon -text "Eicon Diva" -width 18 -variable modem -value eicon -command {set vidpid1 "071dac81"; set vidpid2 "0915ac82"} -padx 10 -selectcolor blue
.bloc1.modem.eicon configure -anchor w
pack .bloc1.modem.eicon

radiobutton .bloc1.modem.ericsson -text "Ericsson hm120dp" -width 18 -variable modem -value ericsson -command {set vidpid1 "08ea00c9"; set vidpid2 "091500ca"} -padx 10 -selectcolor blue
.bloc1.modem.ericsson configure -anchor w
pack .bloc1.modem.ericsson

radiobutton .bloc1.modem.aztech -text "Aztech 100U" -width 18 -variable modem -value aztech -command {set vidpid1 "05090801"; set vidpid2 "09150802"} -padx 10 -selectcolor blue
.bloc1.modem.aztech configure -anchor w
pack .bloc1.modem.aztech

radiobutton .bloc1.modem.fujitsu -text "Fujitsu FDX310" -width 18 -variable modem -value fujitsu -command {set vidpid1 "0e600101"; set vidpid2 "09150102"} -padx 10 -selectcolor blue
.bloc1.modem.fujitsu configure -anchor w
pack .bloc1.modem.fujitsu

set modem "eci"
set vidpid1 "05472131"
set vidpid2 "09158000"

frame .bloc1.espace2 -width 20

pack .bloc1.fai .bloc1.espace1 .bloc1.modem .bloc1.espace2 -side left -anchor n

pack .bloc1

frame .ligne_vide2 -height 20
pack .ligne_vide2

#
# ===== List of .bin =====
#

frame .bloc2

frame .bloc2.listebin

checkbutton .bloc2.listebin.checkbox -text { Change .bin file (only if driver hangs up) :} -command {invert_bin} -relief groove -background "#ffcc99" -width 45 -variable majbin -offvalue "non" -onvalue "oui" -selectcolor blue
bind .bloc2.listebin.checkbox <Enter> {pushstate "Check this box if you want to change your .bin file"}
bind .bloc2.listebin.checkbox <Leave> {popstate}
set majbin "non"

set bin_actuel [exec grep "\$ECILOAD2 0x\$vid2 0x\$pid2 /" /usr/local/bin/startmodem]
set fin ".*"
set bin_actuel [regexp "pid2 ($fin);" $bin_actuel ligne nom_bin_actuel]

label .bloc2.listebin.actuel -text "Current .bin: $nom_bin_actuel" -relief sunken -width 48 -anchor w

frame .bloc2.listebin.liste
listbox .bloc2.listebin.liste.contenu -yscrollcommand ".bloc2.listebin.liste.scroll set" -width 45 -height 5 -foreground darkgray -selectbackground lightgray -selectforeground darkgray

proc add_bins {chemin} {
global .bloc2.listebin nom_bin_actuel
    set returncode [catch {exec find $chemin -name "*.bin" } bin_trouves]
    if {$returncode != 0} {
    } else {
        foreach bin $bin_trouves {
            if {![regexp "_firm_" $bin] && [lsearch -glob [.bloc2.listebin.liste.contenu get 0 end] $bin] == -1} {
                if {[string compare $bin $nom_bin_actuel] != 0} {
                    .bloc2.listebin.liste.contenu insert end $bin
                }
            }
        }
    }
}

add_bins "/etc/eciadsl"

.bloc2.listebin.liste.contenu selection set 0
set bin_choisi [.bloc2.listebin.liste.contenu curselection]
set bin_initial $bin_choisi
bind .bloc2.listebin.liste.contenu <Enter> {pushstate "Choose another .bin (ONLY if driver hangs up into eci-load2 !)"}
bind .bloc2.listebin.liste.contenu <Leave> {popstate}
scrollbar .bloc2.listebin.liste.scroll -command ".bloc2.listebin.liste.contenu yview"
pack .bloc2.listebin.liste.contenu .bloc2.listebin.liste.scroll -side left -fill y

frame .bloc2.listebin.recherche
label .bloc2.listebin.recherche.texte -text {Search .bin here :} -width 15
set chemin_bin /usr/local
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

frame .bloc2.espace1 -width 15

#
# ===== VPI/VCI =====
#

frame .bloc2.vpci

label .bloc2.vpci.libelle -text "Your VPI/VCI :" -relief groove -background "#ffcc99" -width 15

frame .bloc2.vpci.vpi_espace -height 5
frame .bloc2.vpci.vpi
label .bloc2.vpci.vpi.label -text "VPI :"
set vpi "8"
entry .bloc2.vpci.vpi.entry -textvariable vpi -background lightblue -width 4
bind .bloc2.vpci.vpi.entry <Enter> {pushstate "VPI given by your provider (8 for France)"}
bind .bloc2.vpci.vpi.entry <Leave> {popstate}

frame .bloc2.vpci.vci_espace -height 10
frame .bloc2.vpci.vci
label .bloc2.vpci.vci.label -text "VCI :"
set vci "35"
entry .bloc2.vpci.vci.entry -textvariable vci -background lightblue -width 4
bind .bloc2.vpci.vci.entry <Enter> {pushstate "VCI given by your provider (35 for France)"}
bind .bloc2.vpci.vci.entry <Leave> {popstate}

pack .bloc2.vpci.libelle -side top
pack .bloc2.vpci.vpi.label .bloc2.vpci.vpi.entry -side left -padx 5
pack .bloc2.vpci.vci.label .bloc2.vpci.vci.entry -side left -padx 5

# Modem image :

frame .bloc2.vpci.espace -height 3
image create photo modem_eci -file /etc/eciadsl/modemeci.gif
label .bloc2.vpci.image -image modem_eci
bind .bloc2.vpci.image <Enter> {pushstate "ECI HiFocus USB ADSL modem"}
bind .bloc2.vpci.image <Leave> {popstate}

pack .bloc2.vpci.libelle .bloc2.vpci.vpi_espace .bloc2.vpci.vpi .bloc2.vpci.vci .bloc2.vpci.espace .bloc2.vpci.image -side top

#frame .bloc2.vpci -width 25

frame .bloc2.espace2 -width 20

pack .bloc2.listebin .bloc2.espace1 .bloc2.vpci .bloc2.espace2 -side left -anchor n

pack .bloc2 -anchor w

frame .ligne_vide3 -height 15
pack .ligne_vide3

#
# ===== OK and Cancel buttons =====
#

frame .boutons
button .boutons.create -text {Create config !} -background lightgreen -width 15 -height 2 -command {run_makeconfig $username $password $path_pppoeci $dns1 $dns2 $vpi $vci $vidpid1 $vidpid2}
bind .boutons.create <Enter> {pushstate "Save modifications : write configuration to files (backup all files before)"}
bind .boutons.create <Leave> {popstate}
frame .boutons.espace -width 50
button .boutons.cancel -text {Cancel} -background "#ffbbbb" -width 15 -height 2 -command {exit}
bind .boutons.cancel <Enter> {pushstate "Quit without saving"}
bind .boutons.cancel <Leave> {popstate}
pack .boutons.create .boutons.espace .boutons.cancel -side left
pack .boutons

frame .ligne_vide4 -height 15
pack .ligne_vide4

label .state -textvariable statetext -borderwidth 2 -relief sunken -anchor w
pack .state -fill x
set statetext "Ready."

#
# ===== call to makeconfig =====
#

proc run_makeconfig {username password path_pppoeci dns1 dns2 vpi vci vidpid1 vidpid2} {
global titre_fenetre majdns majbin
    if {[string compare $majdns "oui"] == 0} {
        set srvdns1 $dns1
        set srvdns2 $dns2
    } else {
        set srvdns1 0
        set srvdns2 0
    }
    set numero_bin_choix [.bloc2.listebin.liste.contenu curselection]
    set nom_bin_choix [.bloc2.listebin.liste.contenu get $numero_bin_choix]
    if {[string compare $majbin "oui"] == 0} {
        set returncode [catch {exec makeconfig $username $password $path_pppoeci $srvdns1 $srvdns2 $vpi $vci $vidpid1 $vidpid2 $nom_bin_choix} sortie]
    } else {
        set returncode [catch {exec makeconfig $username $password $path_pppoeci $srvdns1 $srvdns2 $vpi $vci $vidpid1 $vidpid2} sortie]
    }
    toplevel .confok
    wm title .confok $titre_fenetre
    if {$returncode != 0} {
        set msg "Makeconfig did not update your files.\n\nThis is the error :"
        set color "#ffbbbb"
    } else {
        set msg "Configuration files updated with success !\n\nThis is makeconfig output :"
        set color lightgreen
    }
    message .confok.texte -text "$msg" -aspect 600 -padx 15 -pady 15
    message .confok.sortie -text "$sortie" -aspect 600 -padx 15 -pady 15 -background $color
    button .confok.ok -text { O K } -command {exit}
    pack .confok.texte .confok.sortie .confok.ok -side top
    pack configure .confok.ok -pady 15
    grab set .confok
    tkwait window .confok
}

#
# ===== call to remove_dabusb =====
#

proc run_dabusb {} {
    set returncode [catch {exec remove_dabusb} sortie]
    toplevel .dab
    wm title .dab "dabusb"
    if {$returncode != 0} {
        set msg "Dabusb couldn't be removed...\n\nThe error is :"
        set vmtitle "Error"
        set color "#ffbbbb"
    } else {
        set msg "Dabusb succesfully removed.\n\nThis is dabusb output :"
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
# ===== invert dns state (enabled/disabled) =====
#

proc invert_majdns {} {
global majdns
    if {[string compare $majdns "oui"] == 0} {
        .bloc1.fai.ligne1.wanadoo configure -state normal -selectcolor blue
        .bloc1.fai.ligne1.clubinternet configure -state normal -selectcolor blue
        .bloc1.fai.ligne2.neuftelecom configure -state normal -selectcolor blue
        .bloc1.fai.ligne2.cegetel configure -state normal -selectcolor blue
        .bloc1.fai.ligne3.worldonline configure -state normal -selectcolor blue
        .bloc1.fai.ligne3.telecomitalia configure -state normal -selectcolor blue
        .bloc1.fai.ligne4.tiscali configure -state normal -selectcolor blue
        .bloc1.fai.ligne4.pipexuk configure -state normal -selectcolor blue
        .bloc1.fai.dns1 configure -state normal -foreground black -background lightblue
        .bloc1.fai.dns2 configure -state normal -foreground black -background lightblue
        .bloc1.fai.labeldns configure -foreground black
    } else {
        .bloc1.fai.ligne1.wanadoo configure -state disabled -selectcolor darkgray
        .bloc1.fai.ligne1.clubinternet configure -state disabled -selectcolor darkgray
        .bloc1.fai.ligne2.neuftelecom configure -state disabled -selectcolor darkgray
        .bloc1.fai.ligne2.cegetel configure -state disabled -selectcolor darkgray
        .bloc1.fai.ligne3.worldonline configure -state disabled -selectcolor darkgray
        .bloc1.fai.ligne3.telecomitalia configure -state disabled -selectcolor darkgray
        .bloc1.fai.ligne4.tiscali configure -state disabled -selectcolor darkgray
        .bloc1.fai.ligne4.pipexuk configure -state disabled -selectcolor darkgray
        .bloc1.fai.dns1 configure -state disabled -foreground darkgray -background lightgray
        .bloc1.fai.dns2 configure -state disabled -foreground darkgray -background lightgray
        .bloc1.fai.labeldns configure -foreground darkgray
    }
}

#
# ===== invert .bin files (enabled/disabled) =====
#

proc invert_bin {} {
global majbin bin_choisi bin_initial
    if {[string compare $majbin "oui"] == 0} {
        if {[.bloc2.listebin.liste.contenu size] > 0} {
            .bloc2.listebin.liste.contenu configure -foreground black -selectbackground "#00ccff" -selectforeground black
            .bloc2.listebin.liste.contenu selection clear 0 end
            .bloc2.listebin.liste.contenu selection set $bin_choisi
            .bloc2.listebin.liste.contenu see $bin_choisi
        } else {
            set majbin "non"
        }
    } else {
        .bloc2.listebin.liste.contenu configure -foreground darkgray -selectbackground lightgray -selectforeground darkgray
        set bin_choisi [.bloc2.listebin.liste.contenu curselection]
        if {[string compare $bin_choisi ""] == 0} {
            set bin_choisi $bin_initial
        }
    }
}