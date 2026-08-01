# Serveur Web Local ESP32 - Contrôle de Sorties Multiples

Serveur web embarqué sur ESP32 permettant de contrôler jusqu'à **8 sorties numériques** (relais, LEDs, électrovannes...) depuis un tableau de bord web accessible sur le réseau local, avec noms personnalisés par sortie (ex: "Salon", "Portail", "Piscine"...).

![Statut](https://img.shields.io/badge/statut-fonctionnel-brightgreen)
![Plateforme](https://img.shields.io/badge/plateforme-ESP32-blue)
![Carte](https://img.shields.io/badge/multi--cartes-XIAO--C3%20%7C%20C3%20%7C%20S3%20%7C%20D-orange)

## Aperçu

- 🔌 Contrôle individuel de **jusqu'à 8 sorties** (nombre calculé automatiquement selon le tableau `PINS[]`)
- 🏷️ Nom personnalisé par sortie (Salon, Cuisine, Jardin, Portail, Piscine, Sortie, Chambre, Garage...)
- 💡 Boutons **"Tout Allumer"** / **"Tout Éteindre"**
- 🌐 Accès simplifié via **mDNS** (`http://RichardV.local`), configurable
- 🔄 Rafraîchissement automatique de l'état via **AJAX** (toutes les 500 ms), sans recharger la page
- 📡 Popup d'infos réseau : IP, adresse MAC, puissance du signal WiFi (RSSI) et qualité en %
- 🛡️ Reconnexion WiFi automatique (non bloquante) en cas de coupure
- 🛡️ Protection anti-blocage : timeout sur les requêtes client incomplètes
- 🔀 **Multi-cartes** : configuration prête pour XIAO-ESP32C3, ESP32C3 Dev Module, ESP32 S3-N16R8, ESP32-Wroom-DA
- 🔐 Identifiants WiFi séparés dans un fichier `arduino_secrets.h` 

## Matériel compatible

Le sketch inclut des configurations de broches (GPIO) prêtes à l'emploi pour plusieurs cartes — il suffit de décommenter le bloc correspondant à ta carte :

| Carte | Broches utilisables prévues |
|---|---|
| XIAO-ESP32C3 | 2, 3, 4, 5, 6, 7, 8, 9 |
| ESP32C3 Dev Module | 1, 6, 7, 8, 9, 10, 20, 21 |
| **ESP32 S3-N16R8** *(config active par défaut)* | 9, 10, 11, 12, 13, 14, 15, 16 |
| ESP32-Wroom-DA (ESP-32D Dev Module) | 26, 27, 32, 33, 12, 13, 2, 25 |

> ⚠️ Pour le montage final sur carte **XIAO/C3**, pense à configurer `USB CDC on Boot` sur **Disable** dans les options de la carte.

Chaque sortie peut piloter directement une LED (avec résistance), ou un **module relais** pour commander des équipements 230V (éclairage, portail, pompe de piscine...).

## Prérequis logiciels

- [Arduino IDE](https://www.arduino.cc/en/software) 2.x
- Gestionnaire de cartes **ESP32 par Espressif Systems**, version **3.3.11** (ou compatible)
- Aucune librairie tierce à installer : uniquement `WiFi.h` et `ESPmDNS.h`, fournies avec le core ESP32

## Installation

### 1. Créer le fichier des identifiants WiFi

Ce projet nécessite un fichier `arduino_secrets.h`, **placé dans le même dossier** que le sketch `.ino`, contenant :

```cpp
#define SECRET_SSID "TonReseauWiFi"
#define SECRET_PASS "TonMotDePasse"
```

> 🔐 Ce fichier sépare tes identifiants du code principal — pratique si tu partages ton sketch (pense à ne pas partager ce fichier séparé).

### 2. Configurer la carte utilisée

Ouvre le sketch et repère les blocs marqués **🔴 Repere 1**. Décommente le bloc correspondant à ta carte (et commente les autres) :

```cpp
const byte PINS[] = { 9, 10, 11, 12, 13, 14, 15, 16 };
const char* NOM_PANNEAU_CENTRAL = "Serveur Local";
const char* MA_CARTE = "ESP32 S3-N16R8";
```

### 3. Personnaliser les noms des sorties

Repère **🔴 Repere 2** et adapte le tableau `NOMS[]` à tes besoins (doit contenir **exactement le même nombre d'éléments** que `PINS[]`) :

```cpp
const char* NOMS[] = {
  "Salon", "Cuisine", "Jardin", "Portail",
  "Piscine", "Sortie", "Chambre", "Garage"
};
```

> ⚠️ Une vérification automatique au démarrage bloque le programme (avec message dans le moniteur série) si `NOMS[]` et `PINS[]` n'ont pas la même taille.

### 4. Personnaliser le nom mDNS (optionnel)

```cpp
const char* HOSTNAME_MDNS = "RichardV"; // -> http://RichardV.local
```

### 5. Téléverser

1. Sélectionne ta carte dans `Outils > Type de carte`.
2. Sélectionne le bon port dans `Outils > Port`.
3. Clique sur **Téléverser**.
4. Ouvre le moniteur série (115200 bauds) pour suivre la connexion WiFi et récupérer l'adresse IP.

## Utilisation

Une fois connecté au WiFi, ouvre un navigateur sur un appareil du **même réseau** :

```
http://RichardV.local
```

*(remplace `RichardV` par le nom que tu as choisi dans `HOSTNAME_MDNS`)*

Si le nom mDNS ne fonctionne pas (notamment sous Windows sans Bonjour installé), utilise l'adresse IP affichée dans le moniteur série.

### Interface web

- Une **case par sortie**, avec son nom personnalisé, son état (🟢 ALLUMEE / 🔴 ETEINTE) et un bouton de bascule
- Boutons globaux **💡 Tout Allumer** / **🔌 Tout Éteindre**
- Bouton **📡 Infos Réseau** affichant une popup avec IP, MAC, nom mDNS, carte utilisée, puissance et qualité du signal

### Commande via le moniteur série

Tape `i` dans le moniteur série à tout moment pour ré-afficher les informations réseau (IP, MAC, RSSI...).

### Endpoints HTTP disponibles

| Requête | Effet |
|---|---|
| `GET /` | Page HTML complète |
| `GET /P{pin}ON` | Allume la sortie sur le GPIO `{pin}` |
| `GET /P{pin}OFF` | Éteint la sortie sur le GPIO `{pin}` |
| `GET /ALL_ON` | Allume toutes les sorties |
| `GET /ALL_OFF` | Éteint toutes les sorties |
| `GET /ETAT?ajax=1` | Renvoie l'état de toutes les sorties + RSSI/qualité (texte court, utilisé par l'AJAX interne) |

Exemple : `http://RichardV.local/P9ON` allume la sortie branchée sur le GPIO 9.

## Structure du projet

```
.
├── Serveur_Web_Local_ESP32_x_sorties.ino   # Sketch principal
├── arduino_secrets.h                        # Identifiants WiFi (à créer, non fourni)
└── README.md                                # Ce fichier
```

## Dépannage

| Problème | Solution |
|---|---|
| Erreur de compilation `arduino_secrets.h` introuvable | Crée ce fichier dans le même dossier que le `.ino` (voir section Installation) |
| Blocage au démarrage avec message d'erreur `NOMS[]`/`PINS[]` | Vérifie que les deux tableaux ont exactement le même nombre d'éléments |
| `RichardV.local` inaccessible | Utilise l'IP directe affichée dans le moniteur série, ou installe Bonjour (Windows) |
| Page web lente ou boutons qui "clignotent" | Normal si le réseau est chargé ; l'AJAX se relance automatiquement toutes les 500 ms |
| Sortie qui ne répond pas | Vérifie que le GPIO choisi n'est pas dans la liste des broches à éviter pour ta carte (voir tableau Matériel compatible) |
| WiFi qui se déconnecte régulièrement | Le sketch retente automatiquement toutes les 10s ; vérifie la puissance du signal via le bouton Infos Réseau |

## Licence

Libre d'utilisation et de modification pour un usage personnel.

## Auteur

Projet personnel — Richard 🛠️
