#ifndef Linky_H
#define Linky_H
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "driver/uart.h"
#include "esp_log.h"
#include "string.h"

// clang-format off
// #define MODE_HISTORIQUE 0
// #define MODE_STANDARD   1

#define START_OF_FRAME  0x02 // The start of frame character
#define END_OF_FRAME    0x03   // The end of frame character

#define START_OF_GROUP  0x0A  // The start of group character
#define END_OF_GROUP    0x0D    // The end of group character


#define RX_BUF_SIZE     1024 // The size of the UART buffer
#define BUFFER_SIZE     1024 // The size of the UART buffer
#define FRAME_SIZE      500   // The size of one frame buffer
#define GROUP_COUNT     50

#define LINKY_TAG "Linky"

typedef struct {
    uint64_t value = UINT64_MAX;
    time_t timestamp = 0;
} TimeLabel;

typedef struct
{
    //  Variables                   Taille      Unité       Description
    //-----------------------------------------------------------------------
    char ADCO[13]       = {0};          //    12                     Adresse du compteur 
    char OPTARIF[5]     = {0};          //     4                     Option tarifaire choisie
    uint16_t ISOUSC     = UINT16_MAX;   //     2         A           Intensité souscrite 

    uint64_t BASE       = UINT64_MAX;   //     9         Wh          Index option Base 
    //----------------------Index option Heures Creuses ----------------------
    uint64_t HCHC       = UINT64_MAX;   //     9         Wh          Index option Heures Creuses: Heures Creuses
    uint64_t HCHP       = UINT64_MAX;   //     9         Wh          Index option Heures Creuses: Heures Pleines
    
    //---------------------- Index option EJP  (Effacement des Jours de Pointe) --> 22 jours par an prix du kWh plus cher
    uint64_t EJPHN      = UINT64_MAX;   //     9         Wh          Heures Normales
    uint64_t EJPHPM     = UINT64_MAX;   //     9         Wh          Heures de Pointe Mobile
    uint64_t PEJP       = UINT64_MAX;   //     2         minutes     Préavis Début EJP (30 min)

    //---------------------- Index option Tempo ----------------------
    uint64_t BBRHCJB    = UINT64_MAX;   //     9         Wh          Heures Creuses Jours Bleus
    uint64_t BBRHPJB    = UINT64_MAX;   //     9         Wh          Heures Pleines Jours Bleus
    uint64_t BBRHCJW    = UINT64_MAX;   //     9         Wh          Heures Creuses Jours Blancs
    uint64_t BBRHPJW    = UINT64_MAX;   //     9         Wh          Heures Pleines Jours Blancs
    uint64_t BBRHCJR    = UINT64_MAX;   //     9         Wh          Heures Creuses Jours Rouges
    uint64_t BBRHPJR    = UINT64_MAX;   //     9         Wh          Heures Pleines Jours Rouges

    char PTEC[5]        = {0};          //     4                     Période Tarifaire en cours: TH.. Heures Creuses, HP.. Heures Pleines, HC.. Heures Creuses, HN.. Heures Normales, PM.. Heures de Pointe Mobile, HCJB.. Heures Creuses Jours Bleus, HPJB.. Heures Pleines Jours Bleus, HCJW.. Heures Creuses Jours Blancs, HPJW.. Heures Pleines Jours Blancs, HCJR.. Heures Creuses Jours Rouges, HPJR.. Heures Pleines Jours Rouges
    char DEMAIN[5]      = {0};          //     4                     Couleur du lendemain 

    uint16_t IINST      = UINT16_MAX;   //     3         A           Intensité Instantanée
    uint16_t IINST1     = UINT16_MAX;   //     3         A           Intensité Instantanée Phase 1
    uint16_t IINST2     = UINT16_MAX;   //     3         A           Intensité Instantanée Phase 2
    uint16_t IINST3     = UINT16_MAX;   //     3         A           Intensité Instantanée Phase 3
    uint16_t IMAX       = UINT16_MAX;   //     3         A           Intensité maximale appelée
    uint16_t IMAX1      = UINT16_MAX;   //     3         A           Intensité maximale appelée Phase 1
    uint16_t IMAX2      = UINT16_MAX;   //     3         A           Intensité maximale appelée Phase 2
    uint16_t IMAX3      = UINT16_MAX;   //     3         A           Intensité maximale appelée Phase 3
    uint16_t ADPS       = UINT16_MAX;   //     3         A           Avertissement de Dépassement de Puissance Souscrite
    uint16_t ADIR1      = UINT16_MAX;   //     3         A           Avertissement de Dépassement d'intensité de réglage Phase 1
    uint16_t ADIR2      = UINT16_MAX;   //     3         A           Avertissement de Dépassement d'intensité de réglage Phase 2
    uint16_t ADIR3      = UINT16_MAX;   //     3         A           Avertissement de Dépassement d'intensité de réglage Phase 3

    uint32_t PAPP       = UINT32_MAX;   //     5         VA          Puissance apparente
    uint32_t PMAX       = UINT32_MAX;   //     5         W           Puissance maximale triphasée atteinte 
    uint32_t PPOT       = UINT32_MAX;   //     2                     Présence des potentiels??????????????????????????????

    char HHPHC[4]       = {0};          //     1                     Horaire Heures Pleines Heures Creuses
    char MOTDETAT[7]    = {0};          //     6                     Mot d'état du compteur                            
    
    time_t timestamp    = 0;
} LinkyDataHist;

typedef struct
{
    //  Variables                           Taille      Unité       Description
    //--------------------------------------------------------------------------------------
    char ADSC[13]       = {0};          //     12                   Adresse Secondaire du Compteur
    char VTIC[3]        = {0};          //     2                    Version de la TIC
    TimeLabel DATE      = {0};          //     0                    Date du jour
    char NGTF[17]       = {0};          //     16                   Nom du calendrier tarifaire fournisseur
    char LTARF[17]      = {0};          //     16                   Libellé du calendrier tarifaire
    
    uint64_t EAST       = UINT64_MAX;   //     9         Wh         Energie active soutirée totale
    uint64_t EASF01     = UINT64_MAX;   //     9         Wh         Energie active soutirée Fournisseur, index 01
    uint64_t EASF02     = UINT64_MAX;   //     9         Wh         Energie active soutirée Fournisseur, index 02
    uint64_t EASF03     = UINT64_MAX;   //     9         Wh         Energie active soutirée Fournisseur, index 03
    uint64_t EASF04     = UINT64_MAX;   //     9         Wh         Energie active soutirée Fournisseur, index 04
    uint64_t EASF05     = UINT64_MAX;   //     9         Wh         Energie active soutirée Fournisseur, index 05
    uint64_t EASF06     = UINT64_MAX;   //     9         Wh         Energie active soutirée Fournisseur, index 06
    uint64_t EASF07     = UINT64_MAX;   //     9         Wh         Energie active soutirée Fournisseur, index 07
    uint64_t EASF08     = UINT64_MAX;   //     9         Wh         Energie active soutirée Fournisseur, index 08
    uint64_t EASF09     = UINT64_MAX;   //     9         Wh         Energie active soutirée Fournisseur, index 09
    uint64_t EASF10     = UINT64_MAX;   //     9         Wh         Energie active soutirée Fournisseur, index 10

    uint64_t EASD01     = UINT64_MAX;  //      9         Wh         Energie active soutirée Distributeur, index 01
    uint64_t EASD02     = UINT64_MAX;  //      9         Wh         Energie active soutirée Distributeur, index 02
    uint64_t EASD03     = UINT64_MAX;  //      9         Wh         Energie active soutirée Distributeur, index 03
    uint64_t EASD04     = UINT64_MAX;  //      9         Wh         Energie active soutirée Distributeur, index 04

    uint64_t EAIT       = UINT64_MAX;  //      9         Wh         Energie active injectée totale

    uint64_t ERQ1       = UINT64_MAX;  //      9         Wh         Energie réactive Q1 totale 
    uint64_t ERQ2       = UINT64_MAX;  //      9         Wh         Energie réactive Q2 totale
    uint64_t ERQ3       = UINT64_MAX;  //      9         Wh         Energie réactive Q3 totale
    uint64_t ERQ4       = UINT64_MAX;  //      9         Wh         Energie réactive Q4 totale

    uint16_t IRMS1      = UINT16_MAX;  //      3         A          Courant efficace, phase 1
    uint16_t IRMS2      = UINT16_MAX;  //      3         A          Courant efficace, phase 2
    uint16_t IRMS3      = UINT16_MAX;  //      3         A          Courant efficace, phase 3

    uint16_t URMS1      = UINT16_MAX;  //      3         V          Tension efficace, phase 1
    uint16_t URMS2      = UINT16_MAX;  //      3         V          Tension efficace, phase 2
    uint16_t URMS3      = UINT16_MAX;  //      3         V          Tension efficace, phase 3

    uint8_t PREF        = UINT8_MAX;   //      2         kVA        Puissance app. de référence (PREF) 
    uint8_t PCOUP       = UINT8_MAX;   //      2         kVA        Puissance app. de coupure (PCOUP)

    uint32_t SINSTS     = UINT32_MAX;  //      5         VA         Puissance apparente soutirée instantanée
    uint32_t SINSTS1    = UINT32_MAX;  //      5         VA         Puissance apparente soutirée instantanée, phase 1
    uint32_t SINSTS2    = UINT32_MAX;  //      5         VA         Puissance apparente soutirée instantanée, phase 2
    uint32_t SINSTS3    = UINT32_MAX;  //      5         VA         Puissance apparente soutirée instantanée, phase 3

    TimeLabel SMAXSN     = {0};        //      5         VA         Puissance app. max. soutirée n avec date et heure
    TimeLabel SMAXSN1    = {0};        //      5         VA         Puissance app. max. soutirée n avec date et heure, phase 1
    TimeLabel SMAXSN2    = {0};        //      5         VA         Puissance app. max. soutirée n avec date et heure, phase 2
    TimeLabel SMAXSN3    = {0};        //      5         VA         Puissance app. max. soutirée n avec date et heure, phase 3

    TimeLabel SMAXSN_1   = {0};        //      5         VA         Puissance app max. soutirée n-1 
    TimeLabel SMAXSN1_1  = {0};        //      5         VA         Puissance app max. soutirée n-1, phase 1
    TimeLabel SMAXSN2_1  = {0};        //      5         VA         Puissance app max. soutirée n-1, phase 2
    TimeLabel SMAXSN3_1  = {0};        //      5         VA         Puissance app max. soutirée n-1, phase 3

    uint32_t SINSTI    = UINT32_MAX;   //      5         VA         Puissance app. Instantanée injectée
    
    TimeLabel SMAXIN   = {0};          //      5         VA         Puissance app. max. injectée n avec date et heure
    TimeLabel SMAXIN_1 = {0};          //      5         VA         Puissance app. max. injectée n-1 avec date et heure

    TimeLabel CCASN    = {0};          //      5         VA         Point n de la courbe de charge active soutirée
    TimeLabel CCASN_1  = {0};          //      5         VA         Point n-1 de la courbe de charge active soutirée
    TimeLabel CCAIN    = {0};          //      5         VA         Point n de la courbe de charge active injectée
    TimeLabel CCAIN_1  = {0};          //      5         VA         Point n-1 de la courbe de charge active injectée

    TimeLabel UMOY1  = {0};            //      3         V          Tension moyenne, phase 1
    TimeLabel UMOY2  = {0};            //      3         V          Tension moyenne, phase 2
    TimeLabel UMOY3  = {0};            //      3         V          Tension moyenne, phase 3

    char STGE[9]     = {0};            //      8         -          Registre de Statuts

    TimeLabel DPM1  = {0};             //      2         -          Début Pointe Mobile 1
    TimeLabel FPM1  = {0};             //      2         -          Fin Pointe Mobile 1
    TimeLabel DPM2  = {0};             //      2         -          Début Pointe Mobile 2
    TimeLabel FPM2  = {0};             //      2         -          Fin Pointe Mobile 2
    TimeLabel DPM3  = {0};             //      2         -          Début Pointe Mobile 3
    TimeLabel FPM3  = {0};             //      2         -          Fin Pointe Mobile 3

    char MSG1[33]   = {0};             //      32        -          Message court
    char MSG2[17]   = {0};             //      16        -          Message Ultra court 
    char PRM[15]    = {0};             //      14        -          PRM En mode standard la TIC retransmet le PRM.
    char RELAIS[4]  = {0};             //      3         -          Etat des relais: Les données transmises correspondent à l’état des 8 relais dont 1 réel et 7 virtuels.
    char NTARF[3]   = {0};             //      2         -          Numéro de l’index tarifaire en cours
    char NJOURF[3]  = {0};             //      2         -          Numéro du jour en cours calendrier fournisseur
    char NJOURF_1[3]= {0};             //      2         -          Numéro du prochain jour calendrier fournisseur
    char PJOURF_1[99]= {0};            //      98        -          Profil du prochain jour calendrier fournisseur 
    char PPOINTE[99]= {0};             //      98        -          Profil du prochain jour de pointe

}LinkyDataStd;

#define TYPE_STRING 0
#define TYPE_UINT8  1
#define TYPE_UINT16 2
#define TYPE_UINT32 3
#define TYPE_UINT64 4

struct LinkyGroup
{
    char label[10] = {0};
    void *data = NULL;
    uint8_t type = TYPE_STRING;
};

// clang-format on
enum LinkyMode
{
    MODE_HISTORIQUE,
    MODE_STANDARD,
    AUTO
}; // The state of the UART buffer

typedef union
{
    LinkyDataHist *hist;
    LinkyDataStd *std;
} LinkyData;

class Linky
{

public:
    Linky(LinkyMode mode, int RX, int TX); // Constructor
                                           //
    char update();                         // Update the data
    void print();                          // Print the data
                                           //
    LinkyData data;                        // The data
    void begin();                          // Begin the linky
    // void rx_task(void *arg);

    uint16_t index = 0;             // The index of the UART buffer
    char buffer[BUFFER_SIZE] = {0}; // The UART buffer
    LinkyMode mode = MODE_HISTORIQUE;
    void setMode(LinkyMode mode);
    time_t decodeTime(char *time); // Decode the time

private:
    char UARTRX = 0;                // The RX pin of the linky
    char UARTTX = 0;                // The TX pin of the linky
    uint8_t GROUP_SEPARATOR = 0x20; // The group separator character (changes depending on the mode) (0x20 in historique mode, 0x09 in standard mode)

    char *frame = NULL;     // The frame to send to the linky
    uint16_t frameSize = 0; // The size of the frame

    void read();                                        // Read the UART buffer
    char decode();                                      // Decode the frame
    char checksum(char *label, char *data, char *time); // Check the checksum
};

extern Linky linky;

#endif