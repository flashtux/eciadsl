===============================================================================

     README Eciconf - Configuration graphique pour le driver Linux ECI ADSL

              Ecrit le 05/05/2002 par FlashCode (Sebastien Helleu)
              Derniere modification le 13/06/2002 par FlashCode
             Correspond a la version 0.6 d'Eciconf / driver v0.6

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

1) Nom d'utilisateur ("User name") :

    C'est le nom d'utilisateur qui vous a ete communique par votre fournisseur
    d'acces a Internet.
    Exemples: pour Wanadoo, ce nom est de la forme: fti/xxxxx@fti
              pour Club Internet, ce nom est de la forme: xxxxx@clubadsl1

2) Mot de passe ("Password") :

    C'est le mot de passe qui vous a ete communique par votre fournisseur
    d'acces a Internet.

3) Mise a jour des DNS du FAI ("Update internet provider DNS") :

    Cochez cette case pour mettre a jour les DNS (voir choix 5).
    Si vous decochez cette case, les addresses IP des serveurs DNS ne seront
    pas mises a jour dans votre fichier de configuration.

4) DNS (liste des fournisseurs d'acces a Internet) :

    Selectionnez simplement votre fournisseur d'acces a Internet.
    Vous pouvez aussi saisir manuellement vos serveurs DNS si les DNS
    par defaut ne sont pas corrects ou si votre fournisseur d'acces ne figure
    pas dans la liste (si tel est le cas, merci de m'avertir en me donnant le
    nom de votre fournisseur d'acces et ses DNS pour que je fasse evoluer
    l'outil, par e-mail: flashcode@free.fr)

5) VPI :

    VPI communique par votre fournisseur d'acces a Internet.
    (8 pour la France)

6) VCI :

    VCI communique par votre fournisseur d'acces a Internet.
    (35 pour la France)

7) Choix du modem :

    Choisissez votre modem (ceci affecte le VendorID/ProductID) utilisés dans
    les différents scripts du driver.

8) Changement de .bin :

    Choisissez un autre .bin si vous obtenez un blocage du driver à un bloc
    particulier.

9) Bouton Creation de la config ("Create config !") :

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

10) Bouton de changement de .bin ("Change synch .bin") :

    Change seulement le fichier .bin (n'enregistre aucun autre parametre).

11) Bouton Annuler ("Cancel") :

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

4)  /etc/eciadsl/vidpid
    Ce fichier contient les VendorID/ProductID du modem selectionne.

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

Rien à ce jour.

=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

[8] Faire remonter les bugs
---------------------------

Tous les bugs rencontres doivent etre remontes au plus vite a l'une des deux
personnes ci-dessous :

  > FlashCode : flashcode@flashtux.org - http://www.flashtux.org - AIM: FlashCode AIM
                IRC: serveur = irc.openprojects.net, nick = FlashCode
  > wwp       : subcript@free.fr
                IRC: serveur = irc.openprojects.net, nick = wwp
  > Crevetor  : ziva@caramail.com
                IRC: serveur = irc.openprojects.net, nick = Crevetor


=========================== END OF README.eciconf =============================
