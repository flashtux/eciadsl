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

frame .images
image create photo logo_eci -file /etc/eciadsl/eci_logo.gif
image create photo modem_eci -file /etc/eciadsl/modemeci.gif
label .images.image1 -image logo_eci
label .images.image2 -image modem_eci
pack .images.image1 .images.image2 -padx 30 -pady 10 -side left
pack .images

frame .dabusb
message .dabusb.texte -text "If your modem is running when you start Linux,\nclick here after unplugging your modem :" -aspect 600 -anchor w
button .dabusb.remove -text {Remove Dabusb module} -background "#ffccff" -command {run_dabusb} -padx 10
frame .dabusb.espace -width 10
bind .dabusb.remove <Enter> {pushstate "Unplug your modem first and then click on this button to remove dabusb"}
bind .dabusb.remove <Leave> {popstate}
pack .dabusb.texte .dabusb.remove .dabusb.espace -side left
pack configure .dabusb.texte -anchor w
pack .dabusb

frame .ligne_vide1 -height 30
pack .ligne_vide1

frame .frame1
label .frame1.label_chemin -text { Path to pppoeci :} -width 15 -anchor e
set path_pppoeci "/usr/local/bin/pppoeci"
entry .frame1.chemin -textvariable path_pppoeci -background lightblue -width 35
bind .frame1.chemin <Enter> {pushstate "Enter path to run pppoeci"}
bind .frame1.chemin <Leave> {popstate}
pack .frame1.label_chemin .frame1.chemin -side left
pack configure .frame1.chemin -padx 25
pack .frame1 -padx 25 -pady 6

frame .frame2
label .frame2.label_user -text {User name :} -width 15 -anchor e
set username "username@domain"
entry .frame2.user -textvariable username -background lightblue -width 35
bind .frame2.user <Enter> {pushstate "Enter your username and domain (given by your provider)"}
bind .frame2.user <Leave> {popstate}
pack .frame2.label_user .frame2.user -side left
pack configure .frame2.user -padx 25
pack .frame2 -padx 25 -pady 6

frame .frame3
label .frame3.label_password -text { Password :} -width 15 -anchor e
set password ""
entry .frame3.password -show * -textvariable password -background lightblue -width 35
bind .frame3.password <Enter> {pushstate "Enter your password (given by your provider)"}
bind .frame3.password <Leave> {popstate}
pack .frame3.label_password .frame3.password -side left
pack configure .frame3.password -padx 25
pack .frame3 -padx 25 -pady 6

frame .ligne_vide2 -height 25
pack .ligne_vide2

frame .fai
frame .fai.espace -width 125
checkbutton .fai.majdns -text { Update internet provider DNS :} -command {invert_majdns} -relief groove -background "#ffcc99" -width 32 -variable majdns -offvalue "non" -onvalue "oui"
bind .fai.majdns <Enter> {pushstate "Check this box if you want to update your DNS (/etc/resolv.conf)"}
bind .fai.majdns <Leave> {popstate}
pack .fai.espace .fai.majdns -side left
pack .fai -anchor w
set majdns "oui"

frame .wanadoo
frame .wanadoo.espace -width 170
radiobutton .wanadoo.radio -text "Wanadoo" -variable fai -value wanadoo -command {set dns1 "193.252.19.3"; set dns2 "193.252.19.4"} -padx 10 -anchor w
pack .wanadoo.espace .wanadoo.radio -side left
pack .wanadoo -anchor w

frame .clubinternet
frame .clubinternet.espace -width 170
radiobutton .clubinternet.radio -text "Club Internet" -variable fai -value clubinternet -command {set dns1 "194.117.200.15"; set dns2 "194.117.200.10"} -padx 10
pack .clubinternet.espace .clubinternet.radio -side left
pack .clubinternet -anchor w

frame .9telecom
frame .9telecom.espace -width 170
radiobutton .9telecom.radio -text "9 Telecom" -variable fai -value 9telecom -command {set dns1 "212.30.96.108"; set dns2 "213.203.124.146"} -padx 10
pack .9telecom.espace .9telecom.radio -side left
pack .9telecom -anchor w

frame .cegetel
frame .cegetel.espace -width 170
radiobutton .cegetel.radio -text "Cegetel" -variable fai -value cegetel -command {set dns1 "194.6.128.3"; set dns2 "194.6.128.4"} -padx 10
pack .cegetel.espace .cegetel.radio -side left
pack .cegetel -anchor w

frame .worldonline
frame .worldonline.espace -width 170
radiobutton .worldonline.radio -text "World Online" -variable fai -value worldonline -command {set dns1 "212.83.128.3"; set dns2 "212.83.128.4"} -padx 10
pack .worldonline.espace .worldonline.radio -side left
pack .worldonline -anchor w

frame .telecomitalia
frame .telecomitalia.espace -width 170
radiobutton .telecomitalia.radio -text "Telecom Italia" -variable fai -value telecomitalia -command {set dns1 "212.216.112.112"; set dns2 "212.216.172.62"} -padx 10
pack .telecomitalia.espace .telecomitalia.radio -side left
pack .telecomitalia -anchor w

frame .tiscali
frame .tiscali.espace -width 170
radiobutton .tiscali.radio -text "Tiscali Italia" -variable fai -value tiscali -command {set dns1 "195.130.224.18"; set dns2 "195.130.225.129"} -padx 10
pack .tiscali.espace .tiscali.radio -side left
pack .tiscali -anchor w

set fai wanadoo

foreach dns {1 2} {
    frame .framedns${dns}
    frame .framedns${dns}.espace${dns} -width 160
    set texte${dns} "DNS${dns} :"
    label .framedns${dns}.label_dns${dns} -textvariable texte${dns} -width 6
    set dns${dns} ""
    entry .framedns${dns}.dns${dns} -textvariable dns${dns} -background lightblue -width 14
    pack .framedns${dns}.espace${dns} .framedns${dns}.label_dns${dns} .framedns${dns}.dns${dns} -side left
    pack .framedns${dns} -anchor w
}
pack configure .framedns1 -pady 8
bind .framedns1.dns1 <Enter> {pushstate "Enter your primay DNS (given by your provider)"}
bind .framedns1.dns1 <Leave> {popstate}
bind .framedns2.dns2 <Enter> {pushstate "Enter your secondary DNS (given by your provider)"}
bind .framedns2.dns2 <Leave> {popstate}

set dns1 "193.252.19.3"
set dns2 "193.252.19.4"

frame .ligne_vide3 -height 25
pack .ligne_vide3

frame .boutons
button .boutons.create -text {Create config !} -background lightgreen -command {run_makeconfig $username $password $path_pppoeci $dns1 $dns2}
bind .boutons.create <Enter> {pushstate "Save modifications : write configuration to files (backup all files before)"}
bind .boutons.create <Leave> {popstate}
frame .boutons.espace -width 35
button .boutons.cancel -text {Cancel} -background "#ffbbbb" -command {exit}
bind .boutons.cancel <Enter> {pushstate "Quit without saving"}
bind .boutons.cancel <Leave> {popstate}
pack .boutons.create .boutons.espace .boutons.cancel -side left
pack .boutons

frame .ligne_vide4 -height 20
pack .ligne_vide4

label .state -textvariable statetext -borderwidth 2 -relief sunken -anchor w
pack .state -fill x
set statetext "Ready."


proc run_makeconfig {username password path_pppoeci dns1 dns2} {
global titre_fenetre majdns
    if {[string compare $majdns "oui"] == 0} {
        set returncode [catch {exec ./makeconfig $username $password $path_pppoeci $dns1 $dns2} sortie]
    } else {
        set returncode [catch {exec ./makeconfig $username $password $path_pppoeci 0 0} sortie]
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
        .wanadoo.radio configure -state normal
        .clubinternet.radio configure -state normal
        .9telecom.radio configure -state normal
        .cegetel.radio configure -state normal
        .worldonline.radio configure -state normal
        .telecomitalia.radio configure -state normal
        .tiscali.radio configure -state normal
        .framedns1.dns1 configure -state normal -background lightblue
        .framedns2.dns2 configure -state normal -background lightblue
    } else {
        .wanadoo.radio configure -state disabled
        .clubinternet.radio configure -state disabled
        .9telecom.radio configure -state disabled
        .cegetel.radio configure -state disabled
        .worldonline.radio configure -state disabled
        .telecomitalia.radio configure -state disabled
        .tiscali.radio configure -state disabled
        .framedns1.dns1 configure -state disabled -background lightgray
        .framedns2.dns2 configure -state disabled -background lightgray
    }
}
