#!/bin/sh
# ****************************************************************************
# *                                                                          *
# *   eciconf - Tcl/tk GUI for ECI Linux driver configuration (makeconfig)   *
# *             by FlashCode and Crevetor (c) 14/04/2002                     *
# *                                                                          *
# *          For any support, contact one of us :                            *
# *           - FlashCode: flashcode@free.fr  http://flashcode.free.fr       *
# *           - Crevetor : ziva@caramail.com                                 *
# *                                                                          *
# ****************************************************************************
# \
exec wish "$0" "$@" & exit 0

set titre_fenetre "ECI Linux driver configuration v0.1"

wm title . $titre_fenetre

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

frame .dabusb
message .dabusb.texte -text "If your modem is running when you start Linux,\nclick here after unplugging your modem :" -aspect 600 -anchor w
button .dabusb.remove -text {Remove Dabusb} -background "#ffccff" -command {run_dabusb} -padx 10
frame .dabusb.espace -width 6
bind .dabusb.remove <Enter> {pushstate "Unplug your modem first and then click on this button to remove dabusb"}
bind .dabusb.remove <Leave> {popstate}
pack .dabusb.texte .dabusb.remove .dabusb.espace -side left

pack .dabusb -padx 10 -pady 15 -side top

frame .frame1
label .frame1.label_chemin -text { Path to pppoeci :} -width 15 -anchor e
set path_pppoeci "/usr/local/bin/pppoeci"
entry .frame1.chemin -textvariable path_pppoeci -background lightblue -width 35
bind .frame1.chemin <Enter> {pushstate "Enter path to run pppoeci"}
bind .frame1.chemin <Leave> {popstate}
pack .frame1.label_chemin .frame1.chemin -side left
pack configure .frame1.chemin -padx 15
pack .frame1 -padx 15 -pady 6

frame .frame2
label .frame2.label_user -text {User name :} -width 15 -anchor e
set username "username@domain"
entry .frame2.user -textvariable username -background lightblue -width 35
bind .frame2.user <Enter> {pushstate "Enter your username and domain (given by your provider)"}
bind .frame2.user <Leave> {popstate}
pack .frame2.label_user .frame2.user -side left
pack configure .frame2.user -padx 15
pack .frame2 -padx 15 -pady 6

frame .frame3
label .frame3.label_password -text { Password :} -width 15 -anchor e
set password ""
entry .frame3.password -show * -textvariable password -background lightblue -width 35
bind .frame3.password <Enter> {pushstate "Enter your password (given by your provider)"}
bind .frame3.password <Leave> {popstate}
pack .frame3.label_password .frame3.password -side left
pack configure .frame3.password -padx 15
pack .frame3 -padx 15 -pady 6

frame .ligne_vide1 -height 25
pack .ligne_vide1

frame .fai

frame .fai.fai1

frame .fai.fai1.majdns
frame .fai.fai1.majdns.espace -width 25
checkbutton .fai.fai1.majdns.checkbox -text { Update internet provider DNS :} -command {invert_majdns} -relief groove -background "#ffcc99" -width 32 -variable majdns -offvalue "non" -onvalue "oui" -selectcolor blue
bind .fai.fai1.majdns.checkbox <Enter> {pushstate "Check this box if you want to update your DNS (/etc/resolv.conf)"}
bind .fai.fai1.majdns.checkbox <Leave> {popstate}
pack .fai.fai1.majdns.espace .fai.fai1.majdns.checkbox -side left
set majdns "oui"

frame .fai.fai1.wanadoo
frame .fai.fai1.wanadoo.espace -width 70
radiobutton .fai.fai1.wanadoo.radio -text "Wanadoo" -variable fai -value wanadoo -command {set dns1 "193.252.19.3"; set dns2 "193.252.19.4"} -padx 10 -selectcolor blue
pack .fai.fai1.wanadoo.espace .fai.fai1.wanadoo.radio -side left

frame .fai.fai1.clubinternet
frame .fai.fai1.clubinternet.espace -width 70
radiobutton .fai.fai1.clubinternet.radio -text "Club Internet" -variable fai -value clubinternet -command {set dns1 "194.117.200.15"; set dns2 "194.117.200.10"} -padx 10 -selectcolor blue
pack .fai.fai1.clubinternet.espace .fai.fai1.clubinternet.radio -side left

frame .fai.fai1.9telecom
frame .fai.fai1.9telecom.espace -width 70
radiobutton .fai.fai1.9telecom.radio -text "9 Telecom" -variable fai -value 9telecom -command {set dns1 "212.30.96.108"; set dns2 "213.203.124.146"} -padx 10 -selectcolor blue
pack .fai.fai1.9telecom.espace .fai.fai1.9telecom.radio -side left

frame .fai.fai1.cegetel
frame .fai.fai1.cegetel.espace -width 70
radiobutton .fai.fai1.cegetel.radio -text "Cegetel" -variable fai -value cegetel -command {set dns1 "194.6.128.3"; set dns2 "194.6.128.4"} -padx 10 -selectcolor blue
pack .fai.fai1.cegetel.espace .fai.fai1.cegetel.radio -side left

frame .fai.fai1.worldonline
frame .fai.fai1.worldonline.espace -width 70
radiobutton .fai.fai1.worldonline.radio -text "World Online" -variable fai -value worldonline -command {set dns1 "212.83.128.3"; set dns2 "212.83.128.4"} -padx 10 -selectcolor blue
pack .fai.fai1.worldonline.espace .fai.fai1.worldonline.radio -side left

frame .fai.fai1.telecomitalia
frame .fai.fai1.telecomitalia.espace -width 70
radiobutton .fai.fai1.telecomitalia.radio -text "Telecom Italia" -variable fai -value telecomitalia -command {set dns1 "212.216.112.112"; set dns2 "212.216.172.62"} -padx 10 -selectcolor blue
pack .fai.fai1.telecomitalia.espace .fai.fai1.telecomitalia.radio -side left

frame .fai.fai1.tiscali
frame .fai.fai1.tiscali.espace -width 70
radiobutton .fai.fai1.tiscali.radio -text "Tiscali Italia" -variable fai -value tiscali -command {set dns1 "195.130.224.18"; set dns2 "195.130.225.129"} -padx 10 -selectcolor blue
pack .fai.fai1.tiscali.espace .fai.fai1.tiscali.radio -side left

pack .fai.fai1.majdns .fai.fai1.wanadoo .fai.fai1.clubinternet .fai.fai1.9telecom .fai.fai1.cegetel .fai.fai1.worldonline .fai.fai1.telecomitalia .fai.fai1.tiscali -side top -anchor w

set fai wanadoo

foreach dns {1 2} {
    frame .fai.fai1.framedns${dns}
    frame .fai.fai1.framedns${dns}.espace${dns} -width 60
    set texte${dns} "DNS${dns} :"
    label .fai.fai1.framedns${dns}.label_dns${dns} -textvariable texte${dns} -width 6
    set dns${dns} ""
    entry .fai.fai1.framedns${dns}.dns${dns} -textvariable dns${dns} -background lightblue -width 14
    pack .fai.fai1.framedns${dns}.espace${dns} .fai.fai1.framedns${dns}.label_dns${dns} .fai.fai1.framedns${dns}.dns${dns} -side left
    pack .fai.fai1.framedns${dns} -anchor w
}
pack configure .fai.fai1.framedns1 -pady 8
bind .fai.fai1.framedns1.dns1 <Enter> {pushstate "\[OPTIONAL\] Enter your own primay DNS (given by your provider)"}
bind .fai.fai1.framedns1.dns1 <Leave> {popstate}
bind .fai.fai1.framedns2.dns2 <Enter> {pushstate "\[OPTIONAL\] Enter your own secondary DNS (given by your provider)"}
bind .fai.fai1.framedns2.dns2 <Leave> {popstate}

set dns1 "193.252.19.3"
set dns2 "193.252.19.4"

frame .fai.espace -width 40

frame .fai.fai2

label .fai.fai2.libelle -text "Your VPI/VCI :" -relief groove -background "#ffcc99" -width 15

frame .fai.fai2.vpi_espace -height 10
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

frame .fai.fai2.espace -height 40
image create photo modem_eci -file /etc/eciadsl/modemeci.gif
label .fai.fai2.image -image modem_eci
bind .fai.fai2.image <Enter> {pushstate "ECI HiFocus USB ADSL modem"}
bind .fai.fai2.image <Leave> {popstate}

pack .fai.fai2.libelle .fai.fai2.vpi_espace .fai.fai2.vpi .fai.fai2.vci_espace .fai.fai2.vci .fai.fai2.espace .fai.fai2.image -side top

pack .fai.fai1 .fai.espace .fai.fai2 -side left -anchor n

pack .fai -anchor w

frame .ligne_vide2 -height 25
pack .ligne_vide2

frame .boutons
button .boutons.create -text {Create config !} -background lightgreen -width 15 -height 2 -command {run_makeconfig $username $password $path_pppoeci $dns1 $dns2 $vpi $vci}
bind .boutons.create <Enter> {pushstate "Save modifications : write configuration to files (backup all files before)"}
bind .boutons.create <Leave> {popstate}
frame .boutons.espace -width 35
button .boutons.cancel -text {Cancel} -background "#ffbbbb" -width 15 -height 2 -command {exit}
bind .boutons.cancel <Enter> {pushstate "Quit without saving"}
bind .boutons.cancel <Leave> {popstate}
pack .boutons.create .boutons.espace .boutons.cancel -side left
pack .boutons

frame .ligne_vide3 -height 20
pack .ligne_vide3

label .state -textvariable statetext -borderwidth 2 -relief sunken -anchor w
pack .state -fill x
set statetext "Ready."


proc run_makeconfig {username password path_pppoeci dns1 dns2 vpi vci} {
global titre_fenetre majdns
    if {[string compare $majdns "oui"] == 0} {
        set returncode [catch {exec makeconfig $username $password $path_pppoeci $dns1 $dns2 $vpi $vci} sortie]
    } else {
        set returncode [catch {exec makeconfig $username $password $path_pppoeci 0 0 $vpi $vci} sortie]
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

proc invert_majdns {} {
global majdns
    if {[string compare $majdns "oui"] == 0} {
        .fai.fai1.wanadoo.radio configure -state normal -selectcolor blue
        .fai.fai1.clubinternet.radio configure -state normal -selectcolor blue
        .fai.fai1.9telecom.radio configure -state normal -selectcolor blue
        .fai.fai1.cegetel.radio configure -state normal -selectcolor blue
        .fai.fai1.worldonline.radio configure -state normal -selectcolor blue
        .fai.fai1.telecomitalia.radio configure -state normal -selectcolor blue
        .fai.fai1.tiscali.radio configure -state normal -selectcolor blue
        .fai.fai1.framedns1.dns1 configure -state normal -foreground black -background lightblue
        .fai.fai1.framedns2.dns2 configure -state normal -foreground black -background lightblue
        .fai.fai1.framedns1.label_dns1 configure -foreground black
        .fai.fai1.framedns2.label_dns2 configure -foreground black
    } else {
        .fai.fai1.wanadoo.radio configure -state disabled -selectcolor darkgray
        .fai.fai1.clubinternet.radio configure -state disabled -selectcolor darkgray
        .fai.fai1.9telecom.radio configure -state disabled -selectcolor darkgray
        .fai.fai1.cegetel.radio configure -state disabled -selectcolor darkgray
        .fai.fai1.worldonline.radio configure -state disabled -selectcolor darkgray
        .fai.fai1.telecomitalia.radio configure -state disabled -selectcolor darkgray
        .fai.fai1.tiscali.radio configure -state disabled -selectcolor darkgray
        .fai.fai1.framedns1.dns1 configure -state disabled -foreground darkgray -background lightgray
        .fai.fai1.framedns2.dns2 configure -state disabled -foreground darkgray -background lightgray
        .fai.fai1.framedns1.label_dns1 configure -foreground darkgray
        .fai.fai1.framedns2.label_dns2 configure -foreground darkgray
    }
}
