===============================================================================

     README Eciconf - Configuration graphique pour le driver Linux ECI ADSL

              Ecrit le 05/05/2002 par FlashCode (Sebastien Helleu)
             Correspond a la version 0.1 d'Eciconf / v0.5 du driver

===============================================================================


                    [1]  Description d'Eciconf

                    [2]  Installation

                    [3]  Lancement

                    [4]  L'interface - Les options a renseigner

                    [5]  Fichiers modifies

                    [6]  Bugs connus

                    [7]  Ce qu'il reste a faire

                    [8]  Faire remonter les bugs

=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

[1] Description d'Eciconf
-------------------------

L'outil graphique Eciconf permet de configurer les principaux fichiers
pour la connexion ADSL avec le modem ECI HiFocus USB sous Linux.

Cet outil est avant tout destine aux debutants sous Linux qui ont
du mal a configurer les fichiers "a la main".
(pas de honte a debuter, nous sommes tous passes par la, moi le premier ! :)

=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

[2] Installation
----------------

Depuis la version 0.5 du driver, Eciconf est integre au driver, aucune
installation n'est donc necessaire en plus du driver.

Pour les versions anterieures a la 0.5, l'outil est encore disponible
separement ici : http://eciadsl.sf.net  ou ici : http://flashcode.free.fr/linux

Eciconf necessite la presence de Tcl/Tk.

=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

[3] Lancement
-------------

Eciconf doit etre lance sous l'utilisateur "root" car le script de configuration
modifie des fichiers systemes qui necessitent les droits administrateur.

Eciconf est ecrit en Tcl/Tk et doit par consequent etre lance sous l'interface
graphique X-Window (XFree86), depuis le repertoire ou il se trouve
(generalement /usr/local/bin)

Lancer Eciconf par la commande :  ./eciconf.sh

=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

[4] L'interface - Les options a renseigner
------------------------------------------

Explication des differents champs / options a renseigner :

1) Chemin vers pppoeci ("Path to pppoeci") :

    Chemin + nom de l'executable pppoeci.
    Ce fichier consitue la base du driver, il permet la communication entre
    ppp et le modem.
    Dans le doute, ne modifiez pas le chemin indique par defaut.

2) Nom d'utilisateur ("User name") :

    C'est le nom d'utilisateur qui vous a ete communique par votre fournisseur
    d'acces a Internet.
    Exemples: pour Wanadoo, ce nom est de la forme: fti/xxxxx@fti
              pour Club Internet, ce nom est de la forme: xxxxx@clubadsl1

3) Mot de passe ("Password") :

    C'est le mot de passe qui vous a ete communique par votre fournisseur
    d'acces a Internet.

4) Mise a jour des DNS du FAI ("Update internet provider DNS") :

    Cochez cette case pour mettre a jour les DNS (voir choix 5).
    Si vous decochez cette case, les addresses IP des serveurs DNS ne seront
    pas mises a jour dans votre fichier de configuration.

5) DNS (liste des fournisseurs d'acces a Internet) :

    Selectionnez simplement votre fournisseur d'acces a Internet.
    Vous pouvez aussi saisir manuellement vos serveurs DNS si les DNS
    par defaut ne sont pas corrects ou si votre fournisseur d'acces ne figure
    pas dans la liste (si tel est le cas, merci de m'avertir en me donnant le
    nom de votre fournisseur d'acces et ses DNS pour que je fasse evoluer
    l'outil, par e-mail: flashcode@free.fr)

6) VPI :

    VPI communique par votre fournisseur d'acces a Internet.
    (8 pour la France)

7) VCI :

    VCI communique par votre fournisseur d'acces a Internet.
    (35 pour la France)

8) Bouton Creation de la config ("Create config !") :

    Un clic sur ce bouton cree les fichiers de configuration conformement
    aux champs et parametres renseignes (cf options 1 a 5).
    Une copie de sauvegarde de tous les fichiers modifies est effectuee
    avant mise a jour des fichiers (voir le chapitre "[5] Fichiers modifies
    par Eciconf").

    La mise a jour des fichiers est effectuee par le script makeconfig,
    qui peut etre egalement lance a la main (voir le source de makeconfig
    pour les parametres a fournir).
    Suite a l'execution de ce script, une fenetre s'ouvre, indiquant si
    la mise a jour s'est correctement deroulee (message sur fond vert), ou si
    elle a echouee (message sur fond rouge).


9) Bouton Annuler ("Cancel") :

    Un clic sur ce bouton permet de quitter immediatement Eciconf, sans
    effectuer la moindre modification dans vos fichiers.

=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

[5] Fichiers modifies
---------------------

Les fichiers suivants sont modifies par Eciconf :
(ou crees s'ils n'existent pas deja)

1)  /etc/resolv.conf
    Ce fichier contient les adresses IP des serveurs DNS pour votre
    fournisseur d'acces a Internet.

2)  /etc/ppp/chap-secrets  et  /etc/ppp/pap-secrets
    Ces fichiers contiennent votre nom d'utilisateur ADSL et votre mot de passe
    (tous deux communiques par votre fournisseur d'acces a Internet).

3)  /etc/ppp/peers/adsl
    Ce script est utilise par ppp pour la connection Internet.
    Il contient toutes les options de connection et notamment votre nom
    d'utilisateur ADSL et le chemin vers "pppoeci".

==> Sauvegarde des anciens fichiers :
Les fichiers modifies sont renommes en {nom}.bak (ou .bak1, .bak2, etc... si
le .bak existe deja).
Exemple: /etc/resolv.conf sera renomme en /etc/resolv.conf.bak

=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

[6] Bugs connus
---------------

Aucun bug n'est connu a ce jour.
Voir neanmoins le chapitre suivant pour ce qu'il reste a faire.

=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

[7] Ce qu'il reste a faire
--------------------------

1) Modifier les fichiers chap-secrets et pap-secrets au lieu d'en creer de
   nouveaux.
   En effet, ces fichiers peuvent deja contenir des mots de passe qui sont
   utilises pour d'autres fournisseurs (exemple: pour votre ancien modem 56K).

2) Modifier le script adsl au lieu d'en creer un nouveau :
   ceci permettra de conserver vos options et d'utiliser la derniere version
   du script, livree et installee avec le driver.

=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

[8] Faire remonter les bugs
---------------------------

Tous les bugs rencontres doivent etre remontes au plus vite a l'une des deux
personnes ci-dessous :

  > FlashCode : flashcode@free.fr - http://flashcode.free.fr - AIM: helleuseb
                IRC: serveur=irc.openprojects.net, nick=FlashCode
  > Crevetor  : ziva@caramail.com
                IRC: serveur=irc.openprojects.net, nick=Crevetor


=========================== END OF README.eciconf =============================
