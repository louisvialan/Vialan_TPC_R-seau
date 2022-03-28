#ifndef BAL_H
#define BAL_H

#include <stdio.h>
#include <string.h>
/* librairie standard ... */
#include <stdlib.h>
/* pour getopt */
#include <unistd.h>
/* déclaration des types de base */
#include <sys/types.h>
/* constantes relatives aux domaines, types et protocoles */
#include <sys/socket.h>
/* constantes et structures propres au domaine UNIX */
#include <sys/un.h>
/* constantes et structures propres au domaine INTERNET */
#include <netinet/in.h>
/* structures retournées par les fonctions de gestion de la base de
données du réseau */
#include <netdb.h>
/* pour les entrées/sorties */
#include <stdio.h>
/* pour la gestion des erreurs */
#include <errno.h>


struct lettre {
    int id;
    char* contenu;
    struct lettre * suivante;
    };

struct Bal{
    int num_bal;
    int nb_lettres;
    struct lettre* premier;
    struct lettre* dernier;
    struct Bal  *suivant ;
    };
/****Liste chainée generale des BAL****/

struct ListeBal
{
    int nb_bal;
    struct Bal* premier;
    struct Bal* dernier;
    struct Bal* courant;
};


struct PDU{
    int mode; //1 Si émetteur 0 si récpeteur
    int id;
    int nb_msg;
    int lg_msg;
   };
   

typedef struct lettre Lettre;
typedef struct Bal Bal;
typedef struct ListeBal ListeBal;
typedef struct PDU PDU;

/**************Fonctions***********/
ListeBal* initialiser_liste_bal(void);
Bal* initialiser_bal(int numerobal);
Lettre* initialiser_lettre(int numlettre, char* message);
Bal* chercherBAL(PDU* pdu, ListeBal** liste, int numerobal);
void ajouter_bal(PDU* pdu, ListeBal** liste, Bal* element);
void ranger_lettre( PDU* pdu, Lettre* lelement, Bal* bonneBal);
char* creer_contenu( PDU* pdu, char* message);
void enregistrer_lettre(PDU* pdu, int numlettre, char* message,  ListeBal** liste);
void afficher_PDU(char* pdu, int lg_pdu);
void afficher_lettre(Lettre* element,PDU* pdu);
void parcourir_bal(Bal* element, PDU* pdu);
void afficher_msg(char *message, int lg, int mode, int num, int id);
void creer_PDU(char* pdu, int mode, int id, int nb_msg, int lg_msg);
void envoi_PDU(int sock, char* pdu, int lg_pdu, int mode, int id, int nb_msg, int lg_msg);
PDU* traitement_PDU(int sock);

#endif

