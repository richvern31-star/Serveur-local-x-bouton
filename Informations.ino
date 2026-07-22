/*
   ============================================================
   Serveur Web ESP32-S3  —  VERSION COMPACTE OPTIMISÉE
   ------------------------------------------------------------
   - Commande de N sorties numériques (définies dans PINS[])
   - Interface Web en grille (2 boutons par ligne), adaptée aux smartphones
   - Affichage de l'état de chaque sortie
   - Code factorisé grâce à un tableau de broches et des boucles,
     ce qui évite de dupliquer le code pour chaque sortie.
   - Le tableau JavaScript "pins" utilisé côté navigateur est
     généré AUTOMATIQUEMENT à partir de PINS[] : il n'y a
     donc plus besoin de le modifier à la main si PINS[] change.

   ------------------------------------------------------------
   OPTIMISATIONS RÉSEAU (version >8 sorties sans blocage) :
   1. Génération des "cases" HTML dans un buffer String unique,
      envoyé en UN SEUL client.print() -> beaucoup moins de
      paquets TCP, page plus rapide.
   2. En-tête "Connection: close" -> libère le socket rapidement.
   3. Intervalle AJAX porté de 200 ms à 500 ms côté navigateur.
   4. Verrou JS "requeteEnCours" -> empêche l'empilement de
      requêtes AJAX si une réponse tarde.

   ------------------------------------------------------------
   NOUVELLES OPTIONS DE ROBUSTESSE AJOUTÉES :
   1. Timeout de lecture client : évite qu'un client silencieux
      ou une requête incomplète ne bloque loop() indéfiniment.
   2. Reconnexion WiFi automatique en cas de coupure (non-bloquante avec millis()).
   3. Protection contre une ligne de requête anormalement longue
      (sécurité anti-débordement mémoire).
   4. Noms/labels personnalisés par sortie : tableau NOMS[] en
      parallèle de PINS[], affiché sur chaque case à la place de
      "Sortie Dx" (ex: "Lampe salon", "Prise cuisine"...).
   5. Utilisation de F() et reserve() pour empêcher la fragmentation de la RAM.
   6. Analyseur de commande optimisé (évite les allocations de String répétées).
   
      adresse IP trouvée  (affichée dans le moniteur série au démarrage)
   ============================================================
*/

/* REMARQUE ESP32-S3 : Choix de GPIO sûres et sans risques de conflit au boot.
👍 Broches recommandées pour vos sorties (PINS)
 Les broches (GPIO) à utiliser sur les versions ESP32
//⚠️---------------(Adapté a ESP32C3 Dev Module)-------------------------------
//const byte PINS[] = { 1, 2, 3, 4, 5, 6, 7, 8};   // Tableau des broches GPIO  de 1 a 10 et 20 21  Pin 8 allume la led bleue
//⚠️---------------(Adapté a ESP32 S3-N16R8----------------------------
//const byte PINS[] = {1, 2, 9, 10, 11, 12, 13, 14};   // Tableau broches GPIO libre 1, 2, 9, 10, 11, 12, 13, 14, 21, 38, 47, 48 ❌ NE PAS UTILISER 3, 35, 36, 37, 45, 46
//⚠️---------------(Adapté a ESP32-Dev Module---------------------------
const byte PINS[] = {4, 16, 17, 18, 19, 21, 22, 23};   // Tableau des broches GPIO libres 4, 16, 17, 18, 19, 21, 22, 23, 25, 26, 27, 32, 33(❌ NE PAS UTILISER 6, 7 )

// 🔴 Repere 1
//------------------------------------------------------------
// Déclaration des sorties : il suffit de modifier ce tableau
// (ajouter/retirer un numéro de broche) pour changer le nombre
// de sorties gérées par le programme, sans toucher au reste du code.
// Le tableau JS côté navigateur sera régénéré automatiquement
// à partir de ce même tableau (voir fonction genererTableauJSPins).
//
// 🔴 Repere 2
//------------------------------------------------------------
// OPTION 4 : Noms personnalisés affichés dans l'interface Web.
// Ce tableau doit avoir EXACTEMENT le même nombre d'éléments,
// et dans le même ordre, que PINS[] ci-dessus (le nom de l'index i
// correspond à la broche PINS[i]). Modifiez simplement le texte
// entre guillemets pour renommer une sortie.
//------------------------------------------------------------

// 🔴 Repere 3
  // On vérifie que le tableau NOMS[] a bien été mis à jour avec
  // le même nombre d'éléments que PINS[]. Sans cette vérification, un
  // oubli lors de l'ajout/retrait d'une sortie provoquerait un accès
  // hors tableau (comportement indéfini) lors de l'affichage de la page.

// 🔴 Repere 4
//============================================================
// verifierWiFi 
// ------------------------------------------------------------
// Vérifie à chaque tour de loop() que la carte est toujours
// connectée au réseau WiFi. En cas de coupure (box redémarrée,
// carte hors de portée temporairement...), tente une reconnexion
// automatique de manière non bloquante en respectant un intervalle.
//============================================================

// 🔴 Repere 5
//============================================================
// traiterCommande — Analyse des commandes HTTP (Optimisée)
// ------------------------------------------------------------
// Analyse la commande reçue en évitant les allocations dynamiques
// lourdes en mémoire RAM (suppression de la création de Strings temporaires).
//============================================================

// 🔴 Repere 6
//============================================================
// genererTableauJSPins — Écrit la ligne JS "let pins=[...]"
// en la construisant dynamiquement à partir de PINS[].
// Ainsi, si vous modifiez PINS[] côté Arduino, le JavaScript
// envoyé au navigateur reste automatiquement synchronisé,
// sans devoir éditer le code JS à la main.
//============================================================

/ 🔴 Repere 7
//============================================================
// genererCasesHTML — Construit en mémoire, dans une seule String,
// tout le bloc HTML des "cases" (une par sortie), afin de l'envoyer
// en UN SEUL client.print() plutôt qu'une dizaine par sortie.
// C'est ce qui règle le ralentissement observé au-delà de 8 sorties.
//============================================================

// 🔴 Repere 8
  // --- PARTIE 1 : début du document HTML, jusqu'à juste avant
  // le tableau "pins" qui doit être généré dynamiquement.
  // Ce bloc est statique, donc regroupé en une seule chaîne
  // stockée en mémoire flash (F()) plutôt qu'en dizaines de println().
  
// 🔴 Repere 9
//============================================================
// Réponse AJAX : renvoie l'état de toutes les sorties (ex: "1;0;1;0")
// Utilisée par le JavaScript pour éviter de recharger la page
// MODIFICATION : Envoie aussi la puissance et qualité réseau lues en temps réel.
//============================================================

*/


  