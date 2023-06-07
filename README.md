# DispoBureau

Ce projet a été réalisé dans le cadre de la formation adimaker.

Il avait pour but de permettre aux enseignants d'afficher sur un écran s'ils étaient disponibles pour acceuilir des étudiants ou non.

Voici la liste des composants utilisés :

- 6 ESP32 Dev Module
- 5 boutons
- 24 LEDS
- 1 matrice LED 64x64


La matrice LED utilise la blibliothèque SmartMatrix. Vous retrouverez la datasheet de la matrice dans le dossier.

# Configurer plusieurs envoyeurs :

Pour configurer plusieurs envoyeurs, il suffit de changer l'identifiant dans le code "sender". La ligne 44 du code permet de changer la variable "ident" qui correspond à l'identifiant de l'ESP32.

![image](https://github.com/seni28/DispoBureau/assets/95346522/ab6b3be5-764a-4428-a8e8-ae024b40d3fa)


