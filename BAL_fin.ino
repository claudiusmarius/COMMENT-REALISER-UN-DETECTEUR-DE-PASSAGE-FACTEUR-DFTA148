//-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//                                                                 CLAUDE DUFOURMONT BAL schéma V1.0
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++CODE : BAL_fin+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°     claude.dufourmont@laposte.net    °°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°
// °°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°     https://www.youtube.com/results?search_query=claude+dufourmont    °°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°
//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- 
//                                              Finalisé le 03/06/20
// Pour ATtiny85
// Fonctionne en mode deep sleep logiciel
// Alimentation unique en 5V, le courant de sommeil est de 0,55µA (photo transistor dans le noir absolu)
// Le bouton poussoir "résistif" BP2 permet d'interroger le contenu de la BAL.
// Le pont diviseur a été modifié pour bien discriminer les 2 "BP"
// Modifié, Testé et fonctionne le 13/06/20 avec schéma BAL schéma V1.0
// Utilisation des fonctions pour WSrouge amelioré, WSbleu, WSvert, AnimationFacteurBatBas, AnimationFacteurBatCorrect, WSmagenta1, DownSleep et Buzzer
// Utilisation de la mémoire "facteur" avec son adresse EEPROM1
// Amelioration codage WS2812 ROUGE
//-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//
//                                        "Une entrée ANA pour déterminer 2 états logiques"
//  
//-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// ATTENTION --- ATTENTION --- ATTENTION --- ATTENTION --- ATTENTION --- ATTENTION --- ATTENTION --- ATTENTION --- ATTENTION --- ATTENTION --- ATTENTION --- ATTENTION --- ATTENTION --- 
// En cas de fort ensoleillement, une entrée de lumière parasite peut venir éclairer légèrement le photo transistor, ce qui aller augmenter le courant de repos d'un ordre de 2 à 3 cad
// que le courant de repos peut atteindre 1,2 à 1,5µA
//-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
// Ici le code a été considéré comme du setup,  (un seul démarrage, pas de bouclage)
// **************************Fonctionne en mode sleep logiciel, il faut lire Val en début de boucle cad avant ladésaturation des mosfet d'entréé***************************************
//-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
  #include <EEPROM.h>                                                   
  #include <Adafruit_NeoPixel.h>
  #define PIN      4
  #define BrocheBuzzer  1
  #define N_LEDS 16
  Adafruit_NeoPixel strip(N_LEDS, PIN, NEO_GRB + NEO_KHZ800);
  #include<avr/sleep.h>
  #define adc_disable()(ADCSRA &=~(1<<ADEN))       
  
  bool facteur;                                               // Memoire etat BAL avant interrogation
  bool etatBP1 = HIGH;                                               
  bool etatBP2 = HIGH;                                               
  bool etatBATBAS = HIGH;
    
  uint8_t AdresseEEPROM1 = 131;                               // Adresse facteur
     
  int CommutGnd = 0;
  int Val;
  int ImageTensionBat;
  int SeuilTension = 350;                                     // Mettre 350 (correct) ou 1000(incorrect) pour les essais
 
  void setup()
  {
  pinMode (CommutGnd, OUTPUT);                                 // Pinoche servant à commuter les masses de certains via mosfet BS170 (bonnepratique pour réduire Isleep)
  digitalWrite(CommutGnd, HIGH); 
  
  Val = analogRead (A3);                                       // Lecture en 0 - 1023 points de la tension BP
  ImageTensionBat = analogRead (A1);                           // Lecture en 0 - 1023 points de l'image de la tension d'alimentation
  
   
  pinMode (BrocheBuzzer, OUTPUT);
  
  strip.begin();
  strip.clear();
  strip.setBrightness(4); 
  strip.show();                                                // Initialise toute les led du ring WS2812 à  'off'
  
  adc_disable();
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);
//-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 
 
 //================DEBUT DETERMINATION ETATS BP================
  if (Val > 860)
  {
  etatBP1 = HIGH;
  etatBP2 = HIGH; 
  }
   
  if (790 <= Val  && Val <= 860)
  {
  etatBP1 = LOW;
  etatBP2 = HIGH; 
  }

  if (495 <=  Val  && Val < 790)
  {
  etatBP1 = HIGH;
  etatBP2 = LOW; 
  }

  if ( Val < 495)
  {
  etatBP1 = HIGH;
  etatBP2 = HIGH; 
  }
  delay(10);
  //================FIN DETERMINATION ETATS BP=====================
 

   
  //================DEBUT DETERMINATION état tension===============
  delay(50);
  ImageTensionBat = analogRead (A1); 
    
  if (ImageTensionBat < SeuilTension)
  {
  etatBATBAS = LOW;
  }
  
  else 
  {
  etatBATBAS = HIGH;
  }
  //================FIN DETERMINATION état tension================
  
  
  
  //====DEBUT RENVOI VERS FONCTIONS Animation ring WS2812 FONCTION ETATS BP et état Batterie====
  if (etatBP1 == LOW && etatBP2 == HIGH && etatBATBAS == HIGH)
  {
  AnimationFacteurBatCorrect ();                              //Renvoi vers fonction animation Ring (et Buzzer) sur détection ouverture BAL et Tension correcte
  }

  if (etatBP1 == LOW && etatBP2 == HIGH && etatBATBAS == LOW)
  {
  AnimationFacteurBatBas ();                                  //Renvoi vers fonction animation Ring (et Buzzer) sur détection ouverture BAL et Tension basse
  }    
   
  facteur = EEPROM.read (AdresseEEPROM1);
  
  if (etatBP2 == LOW && etatBP1 == HIGH && facteur == 0 && etatBATBAS == HIGH)
  {
  WSvert ();                                                  //Renvoi vers fonction animation Ring (et Buzzer) sur détection "Pas de nouveau courrier ET tension correcte"
  }  
  
  facteur = EEPROM.read (AdresseEEPROM1);
  if (etatBP2 == LOW && etatBP1 == HIGH && facteur == 1 && etatBATBAS == HIGH)
  {
  
  WSbleu ();                                                  //Renvoi vers fonction animation Ring (et Buzzer) sur détection "Nouveau courrier ET tension correcte"
  }
    
  if (etatBP2 == LOW && etatBP1 == HIGH && facteur ==1 && etatBATBAS == LOW)
  {
  
  WSmagenta1 ();                                              //Renvoi vers fonction animation Ring (et Buzzer) sur détection "Tension correcte et appui sur BP sensitif"
  }
  
  /*if (etatBP1 == HIGH && etatBP2 == HIGH)
  {
  
  Bug ();                                              //Renvoi vers fonction Bug
  }*/
  
  
  delay(10);
  //=====FIN RENVOI VERS FONCTIONS Animation ring WS2812 FONCTIONdes ETATS BP et état Batterie=====
 
   
  //================DEBUT RENVOI VERS FONCTION DEEP SLEEP==============
  DownSleep ();
  //================FIN RENVOI VERS FONCTION DEEP SLEEP================ 
  
  
  //-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
  
  }                                                               // Accolade de fermeture void setup

   
  //------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------  


  //================DEBUT FONCTION AnimationFacteurBatCorrect==========
  void AnimationFacteurBatCorrect ()
  {

  for (int i = 0; i <= 15; i++)
  {
  strip.setPixelColor(i, 255, 0, 0);
  strip.show ();
  Buzzer(50, 25, 1);
  }
  
  delay (100);
  strip.clear ();
  strip.show ();
  delay (100);
  for (int i = 0; i <= 15; i++)
  {
  strip.setPixelColor(i, 0, 255, 0);
  strip.show ();
  }
  Buzzer(50, 25, 1);  
  
  delay (100);
  strip.clear ();
  strip.show ();
  delay (100);
  for (int i = 0; i <= 15; i++)
  {
  strip.setPixelColor(i, 0, 0, 255);
  strip.show ();
  }
  Buzzer(50, 25, 1);
  
  delay (100);
  strip.clear ();
  strip.show ();
  delay (100);
  for (int i = 0; i <= 15; i++)
  {
  strip.setPixelColor(i, 0, 255, 255);
  strip.show ();
  }
  Buzzer(50, 25, 1); 
  
  delay (100);
  strip.clear ();
  strip.show ();
  delay (100);
  for (int i = 0; i <= 15; i++)
  {
  strip.setPixelColor(i, 255, 255, 0);
  strip.show ();
  }
  Buzzer(50, 25, 1); 
     
  delay (100);
  strip.clear ();
  strip.show ();
  delay (100);
  for (int i = 0; i <= 15; i++)
  {
  strip.setPixelColor(i, 255, 0, 0);
  strip.show ();
  }
  Buzzer(50, 25, 1); 
  
  delay (100);
  Buzzer(600, 25, 1);
  strip.clear ();
  strip.show ();
  delay (100);
  
  facteur =1;
  EEPROM.write (AdresseEEPROM1,facteur);
  }
  //================FIN FONCTION AnimationFacteurBatCorrect========



  //================DEBUT FONCTION ALLUMAGE WS2812 BLEU============
  void WSbleu ()
  {
  for (int i = 0; i <= 7; i++) 
  {
  strip.setPixelColor(i, 0, 0, 255);
  strip.show (); 
  }
  
  Buzzer(100, 50, 4);
  delay (1000);
  strip.clear ();
  strip.show();
  facteur =0;
  EEPROM.write (AdresseEEPROM1,facteur);
  }
  //==============FIN FONCTION ALLUMAGE WS2812 BLEU================ 
  
  
  
  //==============DEBUT FONCTION ALLUMAGE WS2812 VERT=============
  void WSvert ()
  {
  for (int i = 8; i <= 15; i++) 
  {
  strip.setPixelColor(i, 0, 255, 0);
  strip.show (); 
  }
  
  Buzzer(100, 50, 1);
  Buzzer(800, 100, 1);
  delay(100);
  
  delay (1000);
  strip.clear ();
  strip.show();
  }
  //=======FIN FONCTION ALLUMAGE WS2812 VERT=====================
  

    
  //=======DEBUT FONCTION ALLUMAGE WS2812 MAGENTA1================
  void WSmagenta1 ()
  {
  for (int i = 0; i <= 7; i++) {
  strip.setPixelColor(i,255, 0, 255);
  strip.show (); 
  }
  Buzzer(2, 2, 200);
  delay (1000);
  strip.clear ();
  strip.show();
 





  
  /*Buzzer(100, 50, 1);
  Buzzer(700, 100, 1);
  delay (1000);
  strip.clear ();
  strip.show();*/
  }
  //========FIN FONCTION ALLUMAGE WS2812 MAGENTA1================
  
  

 //=========DEBUT FONCTION AnimationFacteurBatBas================
  void AnimationFacteurBatBas ()
  {
  for (int i = 0; i <= 14; i=i+2) {
  strip.setPixelColor(i,0, 0, 255);
  strip.show (); 
  }
  for (int i = 1; i <= 15; i=i+2) {
  strip.setPixelColor(i,0, 255, 0);
  strip.show (); 
  }
  Buzzer(2, 2, 200);
  delay (1000);
  strip.clear ();
  strip.show();
  facteur =0;
  facteur =1;
  EEPROM.write (AdresseEEPROM1,facteur);
  }
  //===============FIN FONCTION AnimationFacteurBatBas======


  //==================DEBUT FONCTION Bug==================
  void Bug ()
  {
  Buzzer(20, 20, 4);
  }
  //==================FIN FONCTION Bug==================


  
  //================DEBUT FONCTION DownSleep================
  void DownSleep ()
  {
  delay(100);                                                 
  digitalWrite (CommutGnd, LOW);                                // Preparation SLEEP : passage à zero des sorties annexxes
  delay(1000);                         
  sleep_enable();                                               // Activation deep sleep
  sleep_cpu(); 
  }
  //================FIN FONCTION DownSleep================
   
  
  //================DEBUT FONCTION BUZZER================
  void Buzzer (int TempsH, int TempsL, int nb)                // TempsH => délai buzzer ON, TempsL => délai buzzer OFF, nb => nombre de bips
  {
  for (int x = 1; x <= nb; x++)                               // Boucle le nombre de fois voulu passée par l'argument "int nb"
  {
  digitalWrite(BrocheBuzzer, HIGH);                           // Active le buzzer
  delay (TempsH);                                             // Temporisation à  l'état haut du buzzer pendant la durée passée par l'argument "int TempsH"
  digitalWrite(BrocheBuzzer, LOW);                            // Désactive le buzzer
  delay (TempsL);                                             // Temporisation à  l'état bas du buzzer pendant la durée passée par l'argument "int TempsL"
  } 
  //================FIN FONCTION BUZZER================
  }
  
  void loop () {}
  //***************************************************************************************************************************************************************************************
  //                                                                                  VOLONTAIREMENT PAS DE BOUCLE  
  //***************************************************************************************************************************************************************************************

  
