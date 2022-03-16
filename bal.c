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
/*fichier h de nos boîtes aux lettres*/
#include "bal.h"

/*********Initialisation de nos structures*******/

ListeBal* initialiser_liste_bal(void)
{
    ListeBal* liste;
    liste = (ListeBal*) malloc(sizeof(ListeBal));

    liste->premier = NULL;
    liste->dernier = NULL;
    liste->courant = NULL;
    liste->nb_bal = 0;
    return liste;
}
Bal* initialiser_bal(int numerobal)
{
    Bal* element = (Bal*) malloc(sizeof(Bal));

    element->num_bal = numerobal;
    element->nb_lettres = 0;
    element->suivant = NULL;
    element->premier = NULL;
    element->dernier = NULL;
    return element;
}
Lettre* initialiser_lettre(int numlettre, char* message)
{
    Lettre* element = (Lettre*) malloc(sizeof(Lettre));

    element->id = numlettre;
    element->contenu = message;
    element->suivante = NULL;
    return element;
}

/**********************************************************
Fonction permettent de rechercher une BAL à partir de son identifiant. LA fonction 
retourne le pointeur vers la BAL si elle existe, sinon NULL. 
***********************************************************/
Bal* chercherBAL(PDU* pdu, ListeBal** liste, int numerobal)
{   
    Bal* ptr = NULL;
    int trouve =0;

    /* Si la liste est vide */
    if ((*liste)->premier == NULL)
    {
        printf("Aucune BAL actuellement enregistree\n");
        ptr = NULL;
    }
    /* Si la liste ne contient qu'un seul élément */
    else if ((*liste)->premier == (*liste)->dernier)
    {
        printf("1 BAL actuellement enregistree\n");
        if ((*liste)->premier->num_bal == numerobal)
        {
            ptr = (*liste)->premier;
        }
        else
        {
            ptr = NULL;
        }
    }
    else
    {
        printf("Plusieurs BAL actuellement enregistrees\n");
        ptr = (*liste)->premier;
        while ((ptr != NULL) && (trouve == 0))
        {
            if (ptr->num_bal == numerobal)
            {
                trouve = 1;
            }
            else
            {
                ptr = ptr->suivant;
            }
        }
    }
    return ptr;
}
/**************************************************************************************
Lorsqu'un émetteur envoie une lettre à un récepteur non connu du serveur, celui ci doit 
créer la bal, puis l'ajouter dans la liste existante. Cette fonction est chargée de 
l'ajout 
**************************************************************************************/
void ajouter_bal(PDU* pdu, ListeBal** liste, Bal* element)
{
    if ((*liste)->premier == NULL) /* Si la liste de BAL est vide */
    {
        (*liste)->premier = element;
        (*liste)->dernier = element;
        (*liste)->nb_bal +=1;
    }
    else /* Si la liste n'est pas vide : on met la nouvelle BAL en bout de liste */
    {
        (*liste)->dernier->suivant = element;
        (*liste)->dernier = element;
        (*liste)->nb_bal +=1;
    }
}
/*******************************************************************
RANGER_LETTRE 
Après copie du message dans le serveur et à la création de la lettre, la lettre est 
rangée dans la bonne BAL 
******************************************************************/
void ranger_lettre( PDU* pdu, Lettre* lelement, Bal* bonneBal)
{
    if (bonneBal->premier == NULL)
    {
        bonneBal->premier = lelement;
        bonneBal->dernier = lelement;
        bonneBal->nb_lettres +=1;
    }
    else 
    {
         Lettre* ptrLTR = bonneBal->premier;
        /* On va au dernier élément */
        while (ptrLTR->suivante != NULL)
        {
            ptrLTR = ptrLTR->suivante;
        }
        ptrLTR->suivante= lelement;
        bonneBal->dernier = lelement;
        bonneBal->nb_lettres += 1;
    }
}
/****************************************
 Créer le contenue de notre lettre
 ***************************************/
char* creer_contenu( PDU* pdu, char* message)
{
    int length = pdu->lg_msg;
    char* contenu = (char*) calloc(length, sizeof(char)); 
    for (int i=0; i<length; i++) 
    {
        *(contenu+i) = *(message+i);
    }
    return contenu;
}
/*************************************************************************************
Enregistrement d'une lettre avec création de BAL si nécessaire à partir d'un message 
reçu et du pdu. 
*************************************************************************************/
void enregistrer_lettre(PDU* pdu, int numlettre, char* message,  ListeBal** liste)
{
    /* Creation de la lettre */ 
    Lettre* elementlettre = initialiser_lettre(numlettre, creer_contenu(pdu,message));

    Bal* BALcourante = NULL;
    
    BALcourante = chercherBAL(pdu, liste, pdu->id);

    if (BALcourante == NULL)
    {
        BALcourante = initialiser_bal(pdu->id);
        ajouter_bal(pdu, liste, BALcourante);
    }

    /* Rangement de la lettre dans la BAL trouvee ou initialisee*/ 
    ranger_lettre(pdu, elementlettre, BALcourante);
}
/****************************************/
void afficher_PDU(char* pdu, int lg_pdu)
{
    printf("MODE BAL : [");
    for (int i=0; i<lg_pdu; i++) printf("%c", *(pdu+i));
    printf("]\n");
}
/***********************************************************
 Afiiche le contenu de la lettre
 **********************************************************/
void afficher_lettre(Lettre* element,PDU* pdu)
{
    printf("\nLecture de la lettre n°%d\n", element->id);
    afficher_PDU(element->contenu, pdu->lg_msg); 
    printf("\n");
}
/********************************************
 Permet d'afficher les lettres d'une BAL
 *******************************************/
void parcourir_bal(Bal* element, PDU* pdu)
{
    Lettre* ptrLTR = element->premier;

    if (ptrLTR == NULL)
    {
        printf("Aucune lettre n'est encore enregistrée.\n");
    }
    while (ptrLTR != NULL)
    {
        afficher_lettre(ptrLTR, pdu);
        ptrLTR = ptrLTR->suivante;
    }
}
/***************************************************************
 Fonction Pour les messages sur le terminal
 **************************************************************/
void afficher_msg(char *message, int lg, int mode, int num, int id)
{
    int i;
    if (mode == 1) //Dans le serveur, en réception
    {
        printf("PUITS : Reception et stockage lettre n°%d pour le récepteur n°%d (%d) [", num, id, lg);
    }
    else if (mode == 2) //Dans le serveur, en envoi
    {
        printf("PUITS : Envoi et déstockage lettre n°%d au récepteur n°%d (%d) [", num, id, lg);
    }
    else if (mode == 3) //Dans l'émetteur, en envoi
    {
        printf("SOURCE : Envoi lettre n°%d à destination du récepteur n°%d (%d) [", num, id, lg);
    }
    else if (mode == 4) //Dans le récepteur, en réception
    {
        printf("RECEPTION : Récupération lettre n°%d par le récepteur n°%d (%d) [", num, id, lg);
    }
    else
    {
        printf("Erreur source d'affichage : [");
    }
    for (i=0; i<lg; i++) printf("%c", *(message+i));
    printf("]\n");
}

void creer_PDU(char* pdu, int mode, int id, int nb_msg, int lg_msg)
{
    /*** Rappel : 
    les caractères ASCII de '0' à '9' valent de 48 à 57 ***/

    /*** 1er caractere : mode ***/
     if (mode == 0) /** Emetteur **/
    {
        *(pdu)='1';
    }
    else if (mode ==1) /** Recepteur **/
    {
        *(pdu)='2';
    }
    else 
    {
        printf("Erreur dans le choix de mode (PDU)\n""\n0: Emetteur\n2: Recepteur\n");
    }

    /*** Separateur ':' ***/
    *(pdu+1)=58; 

    /*** 2e et 3e caracteres : identifiant ***/
    if (id < 10)
    {
        *(pdu+2)=48;
        *(pdu+3)=48+id;
    }
    else if (id < 100)
    {
        *(pdu+2)=48+id/10;
        *(pdu+3)=48+id%10;
    }
    else 
    {
        printf("Veuillez saisir un id compris entre 0 et 99.\n");
    }

    /*** Separateur ':' ***/
    *(pdu+4)=58; 

    /*** 4e et 5e caractères : nombre de messages ***/
    if (nb_msg < 10)
    {
        *(pdu+5)=48;
        *(pdu+6)=48+nb_msg;
    }
    else if (nb_msg < 100)
    {
        *(pdu+5)=48+nb_msg/10;
        *(pdu+6)=48+nb_msg%10;
    }
    else 
    {
        printf("Veuillez saisir un nb_msg compris entre 0 et 99.\n");
        *(pdu+5)=48;
        *(pdu+6)=48;
    }   
    
    /*** Separateur ':' ***/
    *(pdu+7)=58; 

    /*** 6e et 7e caractères : nombre de caracteres ***/
    if (lg_msg < 10)
    {
        *(pdu+8)=48;
        *(pdu+9)=48+lg_msg;
    }
    else if (lg_msg < 100)
    {
        *(pdu+8)=48+lg_msg/10;
        *(pdu+9)=48+lg_msg%10;
    }
    else 
    {
        printf("Veuillez saisir un lg_msg compris entre 0 et 99.\n");
        *(pdu+8)=48;
        *(pdu+9)=48;
    }
}
void envoi_PDU(int sock, char* pdu, int lg_pdu, int mode, int id, int nb_msg, int lg_msg)
{
    creer_PDU(pdu, mode, id, nb_msg, lg_msg);
    if (write(sock, pdu, lg_pdu) == -1)
        {
            printf("Erreur lors de l'envoi du pdu (write).\n");
            exit(1);
        }
}
/********************************
Fonction qui permet de traiter le PDU, rentre les valeurs du messages 
dans notre structure
*********************************/
PDU* traitement_PDU(int sock)
{
    int taille_pdu = 10;
    int lg_msg_recu = -1;
    char* pdu = malloc(taille_pdu * sizeof(char));
    PDU* pdutraite = ( PDU*) malloc(sizeof( PDU));   

    if ((lg_msg_recu = read(sock, pdu, taille_pdu)) > 0)    
    {
        pdutraite->mode = atoi(pdu);
        pdutraite->id = atoi(pdu+2);
        pdutraite->nb_msg = atoi(pdu+5);
        pdutraite->lg_msg = atoi(pdu+8);
    }
    return pdutraite;
}
