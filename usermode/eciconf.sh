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

frame .fai

frame .fai.fai1

frame .fai.fai1.majdns
frame .fai.fai1.majdns.espace -width 25
checkbutton .fai.fai1.majdns.checkbox -text { Update internet provider DNS :} -command {invert_majdns} -relief groove -background "#ffcc99" -width 32 -variable majdns -offvalue "non" -onvalue "oui" -selectcolor blue
bind .fai.fai1.majdns.checkbox <Enter> {pushstate "Check this box if you want to update your DNS (/etc/resolv.conf)"}
bind .fai.fai1.majdns.checkbox <Leave> {popstate}
pack .fai.fai1.majdns.espace .fai.fai1.majdns.checkbox -side left
set majdns "oui"

frame .fai.fai1.ligne1
frame .fai.fai1.ligne1.espace -width 25
radiobutton .fai.fai1.ligne1.wanadoo -text "Wanadoo" -width 11 -variable fai -value wanadoo -command {set dns1 "193.252.19.3"; set dns2 "193.252.19.4"} -padx 10 -selectcolor blue
.fai.fai1.ligne1.wanadoo configure -anchor w
radiobutton .fai.fai1.ligne1.clubinternet -text "Club Internet" -width 11 -variable fai -value clubinternet -command {set dns1 "194.117.200.15"; set dns2 "194.117.200.10"} -padx 10 -selectcolor blue
.fai.fai1.ligne1.clubinternet configure -anchor w
pack .fai.fai1.ligne1.espace .fai.fai1.ligne1.wanadoo .fai.fai1.ligne1.clubinternet -side left

frame .fai.fai1.ligne2
frame .fai.fai1.ligne2.espace -width 25
radiobutton .fai.fai1.ligne2.neuftelecom -text "9 Telecom" -width 11 -variable fai -value 9telecom -command {set dns1 "212.30.96.108"; set dns2 "213.203.124.146"} -padx 10 -selectcolor blue
.fai.fai1.ligne2.neuftelecom configure -anchor w
radiobutton .fai.fai1.ligne2.cegetel -text "Cegetel" -width 11 -variable fai -value cegetel -command {set dns1 "194.6.128.3"; set dns2 "194.6.128.4"} -padx 10 -selectcolor blue
.fai.fai1.ligne2.cegetel configure -anchor w
pack .fai.fai1.ligne2.espace .fai.fai1.ligne2.neuftelecom .fai.fai1.ligne2.cegetel -side left

frame .fai.fai1.ligne3
frame .fai.fai1.ligne3.espace -width 25
radiobutton .fai.fai1.ligne3.worldonline -text "World Online" -width 11 -variable fai -value worldonline -command {set dns1 "212.83.128.3"; set dns2 "212.83.128.4"} -padx 10 -selectcolor blue
.fai.fai1.ligne3.worldonline configure -anchor w
radiobutton .fai.fai1.ligne3.telecomitalia -text "Telecom Italia" -width 11 -variable fai -value telecomitalia -command {set dns1 "212.216.112.112"; set dns2 "212.216.172.62"} -padx 10 -selectcolor blue
.fai.fai1.ligne3.telecomitalia configure -anchor w
pack .fai.fai1.ligne3.espace .fai.fai1.ligne3.worldonline .fai.fai1.ligne3.telecomitalia -side left

frame .fai.fai1.ligne4
frame .fai.fai1.ligne4.espace -width 25
radiobutton .fai.fai1.ligne4.tiscali -text "Tiscali Italia" -width 11 -variable fai -value tiscali -command {set dns1 "195.130.224.18"; set dns2 "195.130.225.129"} -padx 10 -selectcolor blue
.fai.fai1.ligne4.tiscali configure -anchor w
pack .fai.fai1.ligne4.espace .fai.fai1.ligne4.tiscali -side left

pack .fai.fai1.majdns .fai.fai1.ligne1 .fai.fai1.ligne2 .fai.fai1.ligne3 .fai.fai1.ligne4 -side top -anchor w

set fai wanadoo

foreach dns {1 2} {
    frame .fai.fai1.framedns${dns}
    frame .fai.fai1.framedns${dns}.espace${dns} -width 70
    set texte${dns} "DNS${dns} :"
    label .fai.fai1.framedns${dns}.label_dns${dns} -textvariable texte${dns} -width 6
    set dns${dns} ""
    entry .fai.fai1.framedns${dns}.dns${dns} -textvariable dns${dns} -background lightblue -width 14
    pack .fai.fai1.framedns${dns}.espace${dns} .fai.fai1.framedns${dns}.label_dns${dns} .fai.fai1.framedns${dns}.dns${dns} -side left
    pack .fai.fai1.framedns${dns} -anchor w
}
pack configure .fai.fai1.framedns1 -pady 2
bind .fai.fai1.framedns1.dns1 <Enter> {pushstate "\[OPTIONAL\] Enter your own primay DNS (given by your provider)"}
bind .fai.fai1.framedns1.dns1 <Leave> {popstate}
bind .fai.fai1.framedns2.dns2 <Enter> {pushstate "\[OPTIONAL\] Enter your own secondary DNS (given by your provider)"}
bind .fai.fai1.framedns2.dns2 <Leave> {popstate}

set dns1 "193.252.19.3"
set dns2 "193.252.19.4"

frame .fai.espace -width 40

#
# ===== VPI/VCI =====
#

frame .fai.fai2

label .fai.fai2.libelle -text "Your VPI/VCI :" -relief groove -background "#ffcc99" -width 15

frame .fai.fai2.vpi_espace -height 5
frame .fai.fai2.vpi
label .fai.fai2.vpi.label -text "VPI :"
set vpi "8"
entry .fai.fai2.vpi.entry -textvariable vpi -background lightblue -width 4
bind .fai.fai2.vpi.entry <Enter> {pushstate "VPI given by your provider (8 for France)"}
bind .fai.fai2.vpi.entry <Leave> {popstate}

frame .fai.fai2.vci_espace -height 10
frame .fai.fai2.vci
label .fai.fai2.vci.label -text "VCI :"
set vci "35"
entry .fai.fai2.vci.entry -textvariable vci -background lightblue -width 4
bind .fai.fai2.vci.entry <Enter> {pushstate "VCI given by your provider (35 for France)"}
bind .fai.fai2.vci.entry <Leave> {popstate}

pack .fai.fai2.libelle -side top
pack .fai.fai2.vpi.label .fai.fai2.vpi.entry -side left -padx 5
pack .fai.fai2.vci.label .fai.fai2.vci.entry -side left -padx 5

#
# ===== Modem image =====
#

frame .fai.fai2.espace -height 3
image create photo modem_eci -file /etc/eciadsl/modemeci.gif
label .fai.fai2.image -image modem_eci
bind .fai.fai2.image <Enter> {pushstate "ECI HiFocus USB ADSL modem"}
bind .fai.fai2.image <Leave> {popstate}

pack .fai.fai2.libelle .fai.fai2.vpi_espace .fai.fai2.vpi .fai.fai2.vci .fai.fai2.espace .fai.fai2.image -side top

pack .fai.fai1 .fai.espace .fai.fai2 -side left -anchor n

pack .fai -anchor w

frame .ligne_vide2 -height 5
pack .ligne_vide2

#
# ===== List of .bin =====
#

frame .listebin

checkbutton .listebin.checkbox -text { Change .bin file (only if driver hangs up) :} -command {invert_bin} -relief groove -background "#ffcc99" -width 51 -variable majbin -offvalue "non" -onvalue "oui" -selectcolor blue
bind .listebin.checkbox <Enter> {pushstate "Check this box if you want to change your .bin file"}
bind .listebin.checkbox <Leave> {popstate}
set majbin "non"

set bin_actuel [exec grep "\$ECILOAD2 /" /usr/local/bin/startmodem]
set fin ".*"
set bin_actuel [regexp "ECILOAD2 ($fin);" $bin_actuel ligne nom_bin_actuel]

label .listebin.actuel -text "Current .bin: $nom_bin_actuel" -relief sunken -width 54 -anchor w

frame .listebin.liste
listbox .listebin.liste.contenu -yscrollcommand ".listebin.liste.scroll set" -width 51 -height 4 -foreground darkgray -selectbackground lightgray -selectforeground darkgray

proc add_bins {chemin} {
global .listebin nom_bin_actuel
    set returncode [catch {exec find $chemin -name "*.bin" } bin_trouves]
    if {$returncode != 0} {
    } else {
        foreach bin $bin_trouves {
            if {![regexp "_firm_" $bin] && [lsearch -glob [.listebin.liste.contenu get 0 end] $bin] == -1} {
                if {[string compare $bin $nom_bin_actuel] == 0} {
                    #set bin2 "$bin  \[CURRENT\]"
                    #.listebin.liste.contenu insert end $bin2
                    #.listebin.liste.contenu selection set end
                } else {
                    .listebin.liste.contenu insert end $bin
                }
            }
        }
    }
}

add_bins "/etc/eciadsl"

.listebin.liste.contenu selection set 0
set bin_choisi [.listebin.liste.contenu curselection]
set bin_initial $bin_choisi
bind .listebin.liste.contenu <Enter> {pushstate "Choose another .bin (ONLY if driver hangs up into eci-load2 !)"}
bind .listebin.liste.contenu <Leave> {popstate}
scrollbar .listebin.liste.scroll -command ".listebin.liste.contenu yview"
pack .listebin.liste.contenu .listebin.liste.scroll -side left -fill y

frame .listebin.recherche
label .listebin.recherche.texte -text {Search .bin here :} -width 15
set chemin_bin /usr/local
entry .listebin.recherche.chemin -textvariable chemin_bin -background "#CCEEEE" -width 33
bind .listebin.recherche.chemin <Enter> {pushstate "Enter path for searching .bin files"}
bind .listebin.recherche.chemin <Leave> {popstate}
image create bitmap bitmaptest -data "\
#define loupe_width 25 \
#define loupe_height 15 \
static unsigned char loupe_bits[] = { \
   0x00, 0xe0, 0x01, 0x00, 0x00, 0x10, 0x02, 0x00, 0x00, 0x08, 0x04, 0x00, \
   0x00, 0x04, 0x08, 0x00, 0x00, 0x04, 0x08, 0x00, 0x00, 0x04, 0x08, 0x00, \
   0x00, 0x04, 0x08, 0x00, 0x00, 0x08, 0x04, 0x00, 0x00, 0x1c, 0x02, 0x00, \
   0x00, 0xee, 0x01, 0x00, 0x00, 0x07, 0x00, 0x00, 0x80, 0x03, 0x00, 0x00, \
   0xc0, 0x01, 0x00, 0x00, 0xe0, 0x00, 0x00, 0x00, 0x40, 0x00, 0x00, 0x00 };"
button .listebin.recherche.search -image bitmaptest -background "#99CCCC" -command {add_bins $chemin_bin}
bind .listebin.recherche.search <Enter> {pushstate "Click to search .bin files (warning: can take a moment!)"}
bind .listebin.recherche.search <Leave> {popstate}
pack .listebin.recherche.texte .listebin.recherche.chemin .listebin.recherche.search -side left

pack .listebin.checkbox
pack .listebin.actuel
pack .listebin.liste
pack .listebin.recherche
pack .listebin -padx 15 -pady 6

frame .ligne_vide3 -height 15
pack .ligne_vide3

#
# ===== OK and Cancel buttons =====
#

frame .boutons
button .boutons.create -text {Create config !} -background lightgreen -width 15 -height 2 -command {run_makeconfig $username $password $path_pppoeci $dns1 $dns2 $vpi $vci}
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

proc run_makeconfig {username password path_pppoeci dns1 dns2 vpi vci} {
global titre_fenetre majdns majbin
    if {[string compare $majdns "oui"] == 0} {
        set srvdns1 $dns1
        set srvdns2 $dns2
    } else {
        set srvdns1 0
        set srvdns2 0
    }
    set numero_bin_choix [.listebin.liste.contenu curselection]
    set nom_bin_choix [.listebin.liste.contenu get $numero_bin_choix]
    if {[string compare $majbin "oui"] == 0} {
        set returncode [catch {exec makeconfig $username $password $path_pppoeci $srvdns1 $srvdns2 $vpi $vci $nom_bin_choix} sortie]
    } else {
        set returncode [catch {exec makeconfig $username $password $path_pppoeci $srvdns1 $srvdns2 $vpi $vci} sortie]
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
        .fai.fai1.ligne1.wanadoo configure -state normal -selectcolor blue
        .fai.fai1.ligne1.clubinternet configure -state normal -selectcolor blue
        .fai.fai1.ligne2.neuftelecom configure -state normal -selectcolor blue
        .fai.fai1.ligne2.cegetel configure -state normal -selectcolor blue
        .fai.fai1.ligne3.worldonline configure -state normal -selectcolor blue
        .fai.fai1.ligne3.telecomitalia configure -state normal -selectcolor blue
        .fai.fai1.ligne4.tiscali configure -state normal -selectcolor blue
        .fai.fai1.framedns1.dns1 configure -state normal -foreground black -background lightblue
        .fai.fai1.framedns2.dns2 configure -state normal -foreground black -background lightblue
        .fai.fai1.framedns1.label_dns1 configure -foreground black
        .fai.fai1.framedns2.label_dns2 configure -foreground black
    } else {
        .fai.fai1.ligne1.wanadoo configure -state disabled -selectcolor darkgray
        .fai.fai1.ligne1.clubinternet configure -state disabled -selectcolor darkgray
        .fai.fai1.ligne2.neuftelecom configure -state disabled -selectcolor darkgray
        .fai.fai1.ligne2.cegetel configure -state disabled -selectcolor darkgray
        .fai.fai1.ligne3.worldonline configure -state disabled -selectcolor darkgray
        .fai.fai1.ligne3.telecomitalia configure -state disabled -selectcolor darkgray
        .fai.fai1.ligne4.tiscali configure -state disabled -selectcolor darkgray
        .fai.fai1.framedns1.dns1 configure -state disabled -foreground darkgray -background lightgray
        .fai.fai1.framedns2.dns2 configure -state disabled -foreground darkgray -background lightgray
        .fai.fai1.framedns1.label_dns1 configure -foreground darkgray
        .fai.fai1.framedns2.label_dns2 configure -foreground darkgray
    }
}

#
# ===== invert .bin files (enabled/disabled) =====
#

proc invert_bin {} {
global majbin bin_choisi bin_initial
    if {[string compare $majbin "oui"] == 0} {
        if {[.listebin.liste.contenu size] > 0} {
            .listebin.liste.contenu configure -foreground black -selectbackground "#00ccff" -selectforeground black
            .listebin.liste.contenu selection clear 0 end
            .listebin.liste.contenu selection set $bin_choisi
            .listebin.liste.contenu see $bin_choisi
        } else {
            set majbin "non"
        }
    } else {
        .listebin.liste.contenu configure -foreground darkgray -selectbackground lightgray -selectforeground darkgray
        set bin_choisi [.listebin.liste.contenu curselection]
        if {[string compare $bin_choisi ""] == 0} {
            set bin_choisi $bin_initial
        }
    }
}