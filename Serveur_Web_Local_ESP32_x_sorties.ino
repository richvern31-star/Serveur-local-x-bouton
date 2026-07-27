// Gestionaire de carte ESP32 par Expressif Systems Ver 3.3.11
#include <WiFi.h>              // Bibliothèque WiFi standard pour ESP32 (gère la connexion et le serveur)
#include <ESPmDNS.h>           // Bibliothèque pour gerer un serveur Local
#include "arduino_secrets.h"   // Fichier séparé contenant le nom du réseau (SSID) et le mot de passe WiFi

char ssid[] = SECRET_SSID;       // Récupère le nom du réseau WiFi défini dans arduino_secrets.h
char pass[] = SECRET_PASS;       // Récupère le mot de passe WiFi défini dans arduino_secrets.h
int status = WL_IDLE_STATUS;     // Variable qui mémorise l'état de la connexion WiFi (au repos au départ)

// 🔴 Repere 1
//⚠️---------------(Adapté a XIAO-ESP32C3)-------------------------------
// 📌Pour le montage Final penser a programmer l'ESP32 C3 avec USB CDC on Boot sur Disable
/*const byte PINS[] = { 3, 4, 5, 6, 2, 7, 8, 9 };   // Tableau des broches GPIO  de 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 (❌ A Eviter  0, 20 et 21 )  
const char* NOM_PANNEAU_CENTRAL = "Serveur Local"; // Nom du panneau affiché sur la page HTML LIGNE ⚫⚪⚫
const char* MA_CARTE ="XIAO-ESP32C3";// 🟡🟢*/
//⚠️---------------(Adapté a ESP32C3 Dev Module)-------------------------------
// 📌Pour le montage Final penser a programmer l'ESP32 C3 avec USB CDC on Boot sur Disable
/*const byte PINS[] = {  1,6,7,8,9,10,20,21 };   // Tableau des broches GPIO   1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 20, 21  Pin 8 allume la led bleue(❌ A Eviter  0 )  👍
const char* NOM_PANNEAU_CENTRAL = "Serveur Local"; // Nom du panneau affiché sur la page HTML LIGNE ⚫⚪⚫
const char* MA_CARTE ="ESP32 C3"; // 🟡🟢*/
//⚠️---------------(Adapté a ESP32 S3-N16R8 -> ✅ ESP32 S3 Dev Module ----------------------------
const byte PINS[] = {8,97 ,2,3,4,5,7,9};   // Tableau broches GPIO  1 a 48-- 97 led RGB ❌ A Eviter  0,22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37  👍
const char* NOM_PANNEAU_CENTRAL = "Serveur Local"; // Nom du panneau affiché sur la page HTML LIGNE ⚫⚪⚫
const char* MA_CARTE ="ESP32 S3-N16R8";// 🟡🟢*/

//⚠️---------------(Adapté a ESP32-Wroom-DA Module  -> ✅ ESP-32D---------------------------
/*const byte PINS[] = { 18, 19, 20, 21, 22, 23, 24, 25 };   // Tableau broches GPIO 0 a 38 ⚠️ en entrée seul 34, 35, 36, 39 (❌ NE PAS UTILISER  0, 6, 7, 8, 9, 10, 11)
const char* NOM_PANNEAU_CENTRAL = "Serveur Local"; // Nom du panneau affiché sur la page HTML LIGNE ⚫⚪⚫
const char* MA_CARTE = "ESP32-D Wroom-DA Module";// 🟡🟢 */

// Calcule automatiquement le nombre de sorties
const byte NB_SORTIES = sizeof(PINS) / sizeof(PINS[0]);

// 🔴 Repere 2
const char* NOMS[] = {
  "Salon D4",       // Nom affiché pour PINS[0] (broche 4)
  "Cuisine D5",     // Nom affiché pour PINS[1] (broche 5)
  "Jardin D13",     // Nom affiché pour PINS[2] (broche 13)
  "Portail D14",    // Nom affiché pour PINS[3] (broche 14)
  "Piscine D15",    // Nom affiché pour PINS[4] (broche 15)
  "Sortie D16",      // Nom affiché pour PINS[5] (broche 16)
  "Salon D4",       // Nom affiché pour PINS[0] (broche 4)
  "Cuisine D5"     // Nom affiché pour PINS[1] (broche 5)
  
 };

const byte NB_NOMS = sizeof(NOMS) / sizeof(NOMS[0]);   // Calcule le nombre de noms fournis dans NOMS[], pour vérification au démarrage
//⚠️-------------Nom du serveur Local----------
const char* HOSTNAME_MDNS = "RichardV"; // Changez le nom ici si besoin (ex: "esp32.local")

WiFiServer server(80);           // Crée un serveur web qui écoute sur le port 80 (port HTTP standard)

// --- Protection anti-blocage sur la lecture client ---
const unsigned long TIMEOUT_CLIENT_MS = 2000;   // Délai max (ms) accordé à un client pour envoyer sa requête complète
const int TAILLE_MAX_LIGNE = 200;               // Taille max (caractères) tolérée pour une ligne de requête HTTP

// --- Chronomètre pour reconnexion WiFi non-bloquante ---
unsigned long dernierEssaiWiFi = 0;             // Mémorise le moment du dernier essai de connexion WiFi
const unsigned long INTERVALLE_WIFI_MS = 10000; // Délai d'attente minimal (10s) entre chaque tentative de reconnexion

// Déclarations des fonctions
void traiterCommande(const String &req);      // Prototype : analyse une ligne de requête HTTP reçue (par référence constante pour éviter la copie)
void envoyerPage(WiFiClient &client);         // Prototype : envoie la page HTML complète au navigateur
void envoyerEtat(WiFiClient &client);         // Prototype : envoie juste l'état des sorties (pour l'AJAX)
void afficherInfosWiFi();                     // Prototype : affiche les informations WiFi
void genererTableauJSPins(WiFiClient &client);// Prototype : génère la ligne JS "let pins=[...]" à partir de PINS[]
void verifierWiFi();                          // Prototype : vérifie et rétablit la connexion WiFi si besoin
String genererCasesHTML();                    // Prototype : construit en une seule fois le HTML de toutes les "cases"

//============================================================
// SETUP
//============================================================
void setup()
{
  Serial.begin(115200);          // Démarre la liaison série à 115200 bauds pour les messages de débogage

// Attend que le moniteur série soit ouvert
  while (!Serial) { ; }

     // Remonte Infos carte
  Serial.println("\n--- SPÉCIFICATIONS ESP32 ---");
  Serial.printf("Modèle de puce  : %s\n", ESP.getChipModel());
  Serial.printf("Nb de Cœurs CPU : %d\n", ESP.getChipCores());
  Serial.printf("Fréquence CPU   : %d MHz\n", ESP.getCpuFreqMHz());
  Serial.printf("Fréquence Flash : %d MHz\n", ESP.getFlashChipSpeed() / 1000000); // au lieu de / (1024 * 1024));
  Serial.printf("Taille Flash    : %d Mo\n", ESP.getFlashChipSize() / 1000000);
  Serial.printf("Taille PSRAM    : %d Mo\n", ESP.getPsramSize()  / 1000000);
  Serial.println("----------------------------");

Serial.println("\n=== Configuration Réseau ===");

// 🔴 Repere 3
  if (NB_NOMS != NB_SORTIES)
  {
    Serial.println(F("ERREUR : NOMS[] n'a pas le même nombre d'éléments que PINS[] !")); // Utilisation de F() pour économiser la RAM
    Serial.println(F("Vérifiez que chaque broche de PINS[] a bien un nom correspondant dans NOMS[]."));
    while (true);   // Bloque le programme : mieux vaut un blocage visible qu'un plantage silencieux plus tard
  }

  for (byte i = 0; i < NB_SORTIES; i++)      // Parcourt chaque broche déclarée dans le tableau PINS[]
  {
    pinMode(PINS[i], OUTPUT);                // Déclare la broche courante comme une sortie numérique
    digitalWrite(PINS[i], LOW);              // Force la sortie courante à l'état bas (éteinte) au démarrage
  }

  // Connexion au réseau WiFi (Corrigé pour ESP32 standard : boucle stable avec retour visuel)
  WiFi.mode(WIFI_STA);                       // Configure l'ESP32-S3 en mode station (client de votre box)
  WiFi.begin(ssid, pass);

   Serial.print(F("Connexion au réseau : "));  // Affiche un message de connexion en cours
  Serial.println(ssid);                    // Affiche le nom du réseau auquel on essaie de se connecter

  int temporisation = 0;
  while (WiFi.status() != WL_CONNECTED && temporisation < 30) {
    delay(500);
    Serial.print(".");
    temporisation++;
  }

  if (WiFi.status() == WL_CONNECTED)
{
    server.begin(); // Démarre le serveur
    if (MDNS.begin(HOSTNAME_MDNS)) {
      Serial.println("mDNS démarré");
      Serial.print("Accès par : http://");
      Serial.print(HOSTNAME_MDNS);
      Serial.println(".local");
    } else {
      Serial.println("Erreur de démarrage du mDNS");
    }
    delay(1000);// Delais entre mise en route du serveur et affichage Infos IMPORTANT
   Serial.println(F("\nServeur initialisé, attente de l'adresse IP..."));
    // On attend que l'IP soit valide 
    // (Utile si le routeur met un peu de temps à donner l'IP via DHCP)
    int tentatives = 0;
    while (WiFi.localIP().toString() == "0.0.0.0" && tentatives < 10) {
        delay(100); 
        tentatives++;
    }

    afficherInfosWiFi(); 
}
}

void loop()
{

  // 🔴 Rappel Vérifie si l'utilisateur a tapé quelque chose dans le moniteur série pour renvoyer les infos réseau
  if (Serial.available() > 0) {
    char commande = Serial.read();
    // Si la commande est 'i', on affiche les infos réseau
    if (commande == 'i') {
            afficherInfosWiFi();
    }
  }
  // OPTION 2 : vérifie la connexion WiFi et la rétablit si elle a été coupée (de manière non-bloquante)
  verifierWiFi();

  // CORRECTION : Remplacement de "server.available()" déprécié par "server.accept()" sur l'ESP32 v3.x
  WiFiClient client = server.accept();      // Vérifie si un client (navigateur) tente de se connecter

  if (!client)          // Si aucun client n'est connecté...
    return;              // ...on sort immédiatement de loop() et on recommence au tour suivant

  Serial.println(F("Client connecté"));   // Signale dans le moniteur série qu'un client vient de se connecter

  String requete = "";           // Chaîne qui accumule les caractères de la ligne HTTP en cours de lecture
  requete.reserve(TAILLE_MAX_LIGNE); // OPTIMISATION : Alloue de la mémoire à l'avance pour éviter la fragmentation de la RAM

  String premiereLigne = "";     // Mémorise la toute première ligne de la requête (contient l'URL demandée)
  premiereLigne.reserve(80);     // Évite également la fragmentation pour la première ligne
  
  bool premiereLigneLue = false; // Indique si la première ligne a déjà été capturée

  unsigned long debutAttente = millis();   // OPTION 1 : mémorise l'heure de début pour le calcul du timeout

  while (client.connected())     // Tant que le navigateur reste connecté...
  {
    // OPTION 1 : si le client met trop de temps à envoyer sa requête, on abandonne
    // proprement au lieu de rester bloqué ici indéfiniment.
    if (millis() - debutAttente > TIMEOUT_CLIENT_MS)
    {
      Serial.println(F("Timeout client : requête incomplète, abandon."));
      break;
    }

    if (client.available())      // ...et qu'il y a des données à lire...
    {
      char c = client.read();    // Lit un caractère envoyé par le navigateur
      debutAttente = millis();   // Réinitialise le timeout à chaque caractère reçu (le client est toujours actif)

      if (c == '\n')             // Si on reçoit un retour à la ligne, la ligne courante est terminée
      {
        if (requete.length() == 0)  // Une ligne vide signale la fin des en-têtes HTTP
        {
          if (premiereLigne.indexOf("ajax=1") >= 0)   // Vérifie si l'URL contient le paramètre ajax=1
              envoyerEtat(client);                     // Répond uniquement avec l'état des sorties (texte court)
          else
              envoyerPage(client);                     // Chargement complet de la page HTML
          break;      // Sort de la boucle de lecture car la réponse a été envoyée
        }

        if (!premiereLigneLue)      // Si on n'a pas encore mémorisé la première ligne de la requête
        {
          premiereLigne = requete;  // Sauvegarde la ligne actuelle (ex: "GET /P5ON?ajax=1 HTTP/1.1")
          premiereLigneLue = true;  // Marque que la première ligne est désormais connue
        }
        traiterCommande(requete);   // Analyse la ligne reçue pour voir si elle contient une commande ON/OFF ou globale
        requete = "";                // Réinitialise la chaîne pour lire la ligne suivante
      }
      else if (c != '\r')      // Ignore le caractère de retour chariot ('\r'), ne garde que le texte utile
      {
        // OPTION 3 : sécurité anti-débordement. Si une ligne dépasse une taille
        // raisonnable (requête malformée, cliente buguée ou malveillante...),
        // on ignore les caractères supplémentaires plutôt que de laisser la String grossir indéfiniment.
        if (requete.length() < TAILLE_MAX_LIGNE)
        {
          requete += c;          // Ajoute le caractère lu à la ligne en cours de construction
        }
      }
    }
  }

  client.stop();                          // Ferme la connexion avec le client une fois la réponse envoyée
  Serial.println(F("Client déconnecté"));    // Indique dans le moniteur série qu'un client s'est déconnecté
}

// 🔴 Repere 4
void verifierWiFi()
{
  if (WiFi.status() != WL_CONNECTED)   // Si la carte n'est plus connectée au réseau
  {
    unsigned long tempsActuel = millis();
    if (tempsActuel - dernierEssaiWiFi >= INTERVALLE_WIFI_MS) // Tente une reconnexion seulement si le délai est écoulé
    {
      dernierEssaiWiFi = tempsActuel;
      Serial.println(F("WiFi déconnecté, tentative de reconnexion..."));  // Informe dans le moniteur série
      WiFi.disconnect(); 
      WiFi.begin(ssid, pass);    // Relance une tentative de connexion avec les identifiants habituels (rapide et non bloquant s'il échoue directement)
    }
  }
}

// 🔴 Repere 5
void traiterCommande(const String &req)          // Reçoit la ligne HTTP par référence constante pour éviter de la dupliquer
{
  if (req.startsWith("GET /P"))                  // Vérification rapide pour s'assurer qu'il s'agit d'une commande de broche individuelle
  {
    for (byte i = 0; i < NB_SORTIES; i++)      // Teste chaque sortie du tableau, une par une
    {
      // Création d'un mini-buffer de caractères pour générer les requêtes à tester (ex: "/P5ON")
      char cmdOn[12];
      char cmdOff[12];
      sprintf(cmdOn, "/P%dON", PINS[i]);
      sprintf(cmdOff, "/P%dOFF", PINS[i]);

      if (req.indexOf(cmdOn) >= 0)      // Si la ligne demande d'allumer cette sortie
      {
        digitalWrite(PINS[i], HIGH);                  // Met la broche à l'état haut (allumée)
        break;                                        // Sortie trouvée, inutile de continuer la boucle pour ce tour
      }
      if (req.indexOf(cmdOff) >= 0)     // Si la ligne demande d'éteindre cette sortie
      {
        digitalWrite(PINS[i], LOW);                   // Met la broche à l'état bas (éteinte)
        break;                                        // Sortie trouvée, inutile de continuer la boucle
      }
    }
  }
  // --- AJOUT : Prise en charge des commandes globales ---
  else if (req.startsWith("GET /ALL_ON"))        // Si la ligne demande de TOUT allumer
  {
    for (byte i = 0; i < NB_SORTIES; i++)
    {
      digitalWrite(PINS[i], HIGH);                // Force toutes les broches à l'état haut
    }
  }
  else if (req.startsWith("GET /ALL_OFF"))       // Si la ligne demande de TOUT éteindre
  {
    for (byte i = 0; i < NB_SORTIES; i++)
    {
      digitalWrite(PINS[i], LOW);                  // Force toutes les broches à l'état bas
    }
  }
}

// 🔴 Repere 6
void genererTableauJSPins(WiFiClient &client)
{
  client.print("let pins=[");                  // Début du tableau JS, ex: "let pins=["
  for (byte i = 0; i < NB_SORTIES; i++)        // Parcourt chaque broche déclarée dans PINS[]
  {
    client.print(PINS[i]);                      // Écrit le numéro de broche courant (ex: 5)
    if (i < NB_SORTIES - 1) client.print(","); // Ajoute une virgule séparatrice sauf après le dernier élément
  }
  client.println("];");                        // Ferme le tableau JS, ex: "5,6,7,8];"
}

// 🔴 Repere 7
String genererCasesHTML()
{
  String html;                          // Buffer qui va accumuler tout le HTML des cases
  html.reserve(NB_SORTIES * 220UL);     // Réserve à l'avance ~220 caractères par case pour éviter les réallocations

  for (byte i = 0; i < NB_SORTIES; i++)     // Parcourt chaque sortie pour générer sa "case" HTML
  {
    byte pin = PINS[i];                     // Récupère le numéro de broche de la sortie courante
    bool etat = digitalRead(pin);           // Lit l'état actuel de cette broche (0 ou 1)
    String p = String(pin);                 // Convertit une seule fois le numéro de broche en texte

    html += "<!-- Case pour la sortie D"; html += p; html += " -->";        // Repère HTML pour cette case
    html += "<div class='case'>";                                           // Ouvre la case de la sortie courante
    html += "<div class='titre'>"; html += NOMS[i]; html += "</div>";      // affiche le nom personnalisé au lieu de "Sortie Dx"
    html += "<div class='etat' id='txtD"; html += p; html += "'>";         // Ouvre le libellé d'état
    html += etat ? "🟢 ALLUMEE" : "🔴 ETEINTE";                             // Affiche le rond et le texte selon l'état actuel
    html += "</div>";                                                       // Ferme le libellé d'état
    html += "<button id='btnD"; html += p;                                 // Ouvre le bouton avec son identifiant unique
    html += "' class='toggle "; html += (etat ? "on" : "off");            // Ajoute la classe "on" ou "off"
    html += "' data-etat='"; html += etat;                                 // Mémorise l'état actuel dans l'attribut data-etat
    html += "' onclick=\"basculer("; html += p; html += ")\">";            // Définit le clic pour basculer(pin)
    html += etat ? "ON" : "OFF";                                            // Écrit "ON" ou "OFF" à l'intérieur du bouton
    html += "</button></div>";                                             // Ferme le bouton et la case
  }

  return html;      // Renvoie le bloc HTML complet, prêt à être envoyé en un seul print()
}

//============================================================
// Génération de la page Web
//============================================================
void envoyerPage(WiFiClient &client)      // Construit et envoie la page HTML complète au navigateur
{
  client.println("HTTP/1.1 200 OK");                       // Ligne de statut HTTP : la requête a réussi
  client.println("Content-type:text/html; charset=utf-8"); // Indique que la réponse est du HTML en UTF-8
  client.println("Connection: close");                     // Ferme proprement et rapidement la connexion après la réponse
  client.println();                                         // Ligne vide obligatoire séparant les en-têtes du contenu

// 🔴 Repere 8
  client.print(F(R"=====(
<!DOCTYPE html>
<html>
<head>
<!-- Force l'affichage correct sur mobile (zoom initial à 100%) -->
<meta name='viewport' content='width=device-width, initial-scale=1'>

<script>
// Variables globales pour stocker la qualité réseau dynamique mise à jour via AJAX
let dernierRSSI = 0;
let derniereQualite = 0;

// Met à jour visuellement UN bouton/case selon l'état reçu (0 ou 1)
function majBouton(num, etat){
  let btn = document.getElementById('btnD'+num);   // Récupère le bouton correspondant à la broche "num"
  let txt = document.getElementById('txtD'+num);   // Récupère le texte d'état correspondant à la broche "num"
  if(!btn || !txt) return;                         // Vérification de sécurité pour éviter une erreur JS
  btn.setAttribute('data-etat', etat);              // Mémorise l'état actuel dans l'attribut data-etat du bouton
  if(etat=='1'){                                    // Si la sortie est allumée
    btn.className='toggle on';                      // Applique le style vert (classe "on")
    btn.innerHTML='ON';                              // Affiche "ON" sur le bouton
    txt.innerHTML = '🟢 ALLUMEE';                    // Affiche le rond vert + "ALLUMEE"
  } else {                                          // Sinon (sortie éteinte)
    btn.className='toggle off';                     // Applique le style rouge (classe "off")
    btn.innerHTML='OFF';                              // Affiche "OFF" sur le bouton
    txt.innerHTML = '🔴 ETEINTE';                   // Affiche le rond rouge + "ETEINTE"
  }
}

// Reçoit une chaîne et extrait à la fois les états des broches et les données réseau dynamiques
function majAffichage(rawText){
  if(!rawText) return;
  // Découpe la partie états de la partie réseau via le séparateur '|'
  let parties = rawText.split('|');
  let dataEtats = parties[0];
  
  // Si les infos réseau sont présentes dans le paquet AJAX, on les actualise dynamiquement
  if(parties.length > 1){
    let infosReseau = parties[1].split(';');
    dernierRSSI = infosReseau[0];
    derniereQualite = infosReseau[1];
  }

  let etats = dataEtats.split(';');   // Découpe la chaîne reçue en tableau d'états, ex: ["1","0","1","0"]
  // Le tableau ci-dessous est généré AUTOMATIQUEMENT par l'Arduino
  // à partir de PINS[] (voir fonction genererTableauJSPins côté C++).
  // Ne pas modifier cette ligne à la main : elle est réécrite à chaque
  // chargement de page en fonction du contenu réel de PINS[].
)====="));

  // --- PARTIE DYNAMIQUE : génère ici la ligne "let pins=[5,6,7,8];"
  // en fonction du contenu réel de PINS[], au lieu d'un tableau figé.
  genererTableauJSPins(client);

  // --- PARTIE 2 : suite du document HTML (JS + CSS + début du body),
  // toujours statique, donc de nouveau envoyée en un seul print().
  client.print(F(R"=====(
  for(let i=0;i<etats.length;i++){ if(pins[i]) majBouton(pins[i], etats[i]); }  // Applique l'état reçu à chaque broche
}

// Appelée quand on clique sur un bouton : inverse l'état de la sortie "num"
function basculer(num){
  let btn = document.getElementById('btnD'+num);                                                                        // Récupère le bouton cliqué
  if(!btn) return;
  let commande = (btn.getAttribute('data-etat')=='1') ? 'OFF' : 'ON';    // Détermine la commande inverse (ON<->OFF)
  fetch('/P'+num+commande+'?ajax=1').then(r=>r.text()).then(majAffichage); // Envoie la commande au serveur, puis met à jour l'affichage
}

// AJOUT : Appelée lors d'un clic sur l'un des boutons globaux (Tout Allumer / Tout Éteindre)
function commandeGlobale(action){
  fetch('/'+action+'?ajax=1').then(r=>r.text()).then(majAffichage); // Envoie l'action groupée puis actualise l'affichage global
}

// Verrou empêchant l'envoi d'une nouvelle requête AJAX tant que la
// précédente n'a pas reçu sa réponse (évite l'empilement de requêtes).
let requeteEnCours = false;

// Interroge périodiquement le serveur pour rafraîchir l'état affiché
function actualiserEtat(){
  if (requeteEnCours) return;              // Une requête est déjà en cours : on n'en relance pas une nouvelle
  requeteEnCours = true;                  // Verrouille avant l'envoi
  fetch('/ETAT?ajax=1')
    .then(r=>r.text())
    .then(majAffichage)
    .catch(()=>{})                        // Ignore silencieusement une éventuelle erreur réseau ponctuelle
    .finally(()=>{ requeteEnCours = false; });  // Déverrouille dans tous les cas une fois la requête terminée
}

// Intervalle d'actualisation porté à 500 ms pour laisser à la carte
// le temps de répondre avant la requête suivante.
setInterval(actualiserEtat, 500);
</script>

<style>
/* Style général de la page */
body{font-family:Arial,sans-serif;background:#f0f0f0;text-align:center;margin:0;padding:10px;}

/* Le Panneau Blanc central qui contient tout le contenu */
.carte{background:white;padding:20px 15px;margin:20px auto;max-width:360px;border-radius:15px;box-shadow:0px 0px 10px gray;}

/* Grille à 2 colonnes pour organiser les cases de sorties */
.grille{display:grid;grid-template-columns:1fr 1fr;gap:15px;margin-top:15px;}

/* Chaque case individuelle de la grille (une par sortie) */
.case{background:#f8f8f8;border-radius:12px;padding:12px 8px;}

/* Titre affiché en haut de chaque case (ex: "Sortie D5")  Taille du texte*/
.titre{font-size:16px;font-weight:bold;margin-bottom:6px;}

/* Texte affichant l'état (ex: "ALLUMEE"/"ETEINTE") */
.etat{font-size:16px;font-weight:bold;margin-bottom:10px;}

/* Style de base du bouton ON/OFF */
.toggle{width:100%;height:60px;border:none;border-radius:12px;color:white;font-size:18px;font-weight:bold;cursor:pointer;transition: 0.2s;}

/* Couleur verte quand l'état est "ON" */
.on{background:#28a745;}

/* Couleur rouge quand l'état est "OFF" */
.off{background:#d9534f;}

/* Bouton bleu affichant l'adresse IP et la qualité du signal */
.ip{background:#007bff;color:white;padding:12px 20px;border-radius:8px;border:none;font-size:16px;cursor:pointer;margin-top:15px;width:100%;font-weight:bold;}

/* AJOUT : Bouton vert pour tout allumer (même taille que le bouton IP) */
.all-on{background:#28a745;color:white;padding:12px 20px;border-radius:8px;border:none;font-size:16px;cursor:pointer;margin-top:15px;width:100%;font-weight:bold;}

/* AJOUT : Bouton rouge pour tout éteindre (même taille que le bouton IP) */
.all-off{background:#dc3545;color:white;padding:12px 20px;border-radius:8px;border:none;font-size:16px;cursor:pointer;margin-top:15px;width:100%;font-weight:bold;}
</style>
</head>
<body>
<!-- Carte principale contenant le titre et la grille des sorties -->
<div class='carte'>
<!-- Titre du panneau central -->
<h2>
)====="));

  // ⚫⚪⚫Insertion dynamique de la variable texte
  client.print(NOM_PANNEAU_CENTRAL);

  client.print(F(R"=====(</h2>
<!-- Grille qui accueillera une "case" générée par sortie (boucle côté C++) -->
<div class='grille'>
)====="));          // Fin du bloc CSS/JS/en-tête, envoyé en un seul print()

  // --- PARTIE 3 : bloc dynamique — toutes les "cases" sont construites en
  // mémoire par genererCasesHTML() puis envoyées en UN SEUL client.print().
  client.print(genererCasesHTML());

  client.print(F("</div><!-- fin .grille --><hr>"));   // Ferme la grille des sorties et trace une ligne de séparation

  // --- AJOUT : Boutons d'actions globales (mêmes dimensions que le bouton IP) ---
  // Bouton Tout Allumer
  client.println(F("<button class='all-on' onclick=\"commandeGlobale('ALL_ON')\">💡 Tout Allumer</button>"));
  
  // Bouton Tout Éteindre
  client.println(F("<button class='all-off' onclick=\"commandeGlobale('ALL_OFF')\">🔌 Tout Éteindre</button>"));


// --- ✅PARTIE 4 : bouton "Adresse IP" et infos (affiche une popup JS) (Adapté : avec Nom Serveur mDNS dynamique)
  String macStr = WiFi.macAddress();

  client.print("<!-- Bouton affichant l'IP, l'adresse MAC, le nom local et la qualité dynamique via une popup alert() -->");
  client.print("<button class='ip' onclick=\"alert('Serveur local : http://"); // Ouvre le bouton bleu et début du message de la popup
  client.print(HOSTNAME_MDNS); // Nom du serveur local
  client.print(".local\\nCarte : "); 
  client.print(MA_CARTE); // 🟡🟢 Affiche le nom de la carte ESP utilisée
  client.print("\\nAdresse IP : "); client.print(WiFi.localIP());// Insère l'adresse IP locale de la carte dans le message
  client.print("\\nAdresse MAC : "); client.print(macStr); // AJOUT MAC : Insère l'adresse MAC formatée dans la popup
  client.print("\\nPuissance : ' + dernierRSSI + ' dBm\\nQualité : ' + derniereQualite + ' %')\">📡 Infos Réseau</button>");  
  
  client.println("</div><!-- fin .carte --></body></html>");   // Ferme la carte, le corps et le document HTML
}

// 🔴 Repere 9
void envoyerEtat(WiFiClient &client)     // Envoie une réponse courte contenant seulement l'état des sorties
{
  client.println("HTTP/1.1 200 OK");                        // Ligne de statut HTTP : la requête a réussi
  client.println("Content-type: text/plain");
  client.println("Connection: close");
  client.println();

  // 1. Construit la liste des états des broches
  for (byte i = 0; i < NB_SORTIES; i++) 
  {
    client.print(digitalRead(PINS[i]));
    if (i < NB_SORTIES - 1) client.print(";");
  }

  // 2. Calcule et ajoute les données réseau dynamiques (RSSI et %) séparées par '|'
  long rssi = WiFi.RSSI();
  long qualite = 0;
  if (rssi <= -100) qualite = 0;
  else if (rssi >= -50) qualite = 100;
  else qualite = 2 * (rssi + 100);

  client.print("|");
  client.print(rssi);
  client.print(";");
  client.print(qualite);
  //client.println();
}

//============================================================
// afficherInfosWiFi (Version ultra-stable pour ESP32 standard / S3)
//============================================================
void afficherInfosWiFi()      // Affiche dans le moniteur série les informations de connexion WiFi
{
   Serial.println("-------------------------------------");  // Ligne vide pour aérer l'affichage
  Serial.print("Connecté avec succès à : ");   // Affiche le libellé "Connecté au réseau :" (macro F() ajoutée)
  Serial.println(WiFi.SSID());              // Affiche le nom (SSID) du réseau WiFi connecté

  Serial.print("Adresse IP obtenue     : ");   // Affiche le libellé "Adresse IP :"        
  Serial.println(WiFi.localIP());           // Affiche l'adresse IP attribuée à la carte

  String macStr = WiFi.macAddress();     // Récupère l'adresse MAC (nécessaire pour la réservation DHCP dans la box)
  Serial.print("Adresse MAC de la carte: ");    // Affiche le libellé "Adresse MAC :"      
  Serial.println(macStr);

  Serial.print("Puissance du signal    : ");  // Affiche le libellé "Puissance du signal :"
  Serial.print(WiFi.RSSI());                // Affiche la puissance du signal WiFi (en dBm)
  Serial.println(" dBm");                   // Ajoute l'unité "dBm" et termine la ligne
  Serial.println("-------------------------------------");
  
  Serial.print("Ouvrez votre navigateur sur : http://");  
  Serial.println(WiFi.localIP());         // Affiche le libellé d'invitation à ouvrir le navigateur  
  Serial.print("Ou ouvrez votre navigateur sur : http://");
  Serial.print(HOSTNAME_MDNS);            // Affiche dynamiquement le nom mDNS configuré
  Serial.println(".local");
  Serial.println("-------------------------------------\n");
}
