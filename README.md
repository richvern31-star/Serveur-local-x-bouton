# Serveur-local-x-bouton
Serveur ESP32 pour piloter x sorties en fonction du choix d'un ESP32 avec un Smartphone
Commande de N sorties numériques (définies dans PINS[])
Interface Web en grille (2 boutons par ligne), adaptée aussi aux smartphones
Affichage de l'état de chaque sortie
Code factorisé grâce à un tableau de broches et des boucles, ce qui évite de dupliquer le code pour chaque sortie.
Le tableau JavaScript "pins" utilisé côté navigateur est désormais généré AUTOMATIQUEMENT à partir de PINS[]
il n'y a donc plus besoin de le modifier à la main si PINS[] change.
