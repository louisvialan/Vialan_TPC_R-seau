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
#include "bal.h"

#define TAILLE 40


void construire_message(char *message, char motif, int lg, int i)
{
    int a;
    sprintf(message, "%5d", i);
    for (a=0 ; a<lg; a++) { 
        if(a>=5) message[a]=motif;
        else if (message[a]==' ') message[a]='-';
    }
    message[a]='\0';
}
void afficher_message(char *message, int lg, int source, int num)
{
	int i;
	if (source == 1)
	{
		printf("SOURCE : Envoi n°%d (%d) [", num, lg);
	}
	else if (source == 0)
	{
		printf("PUITS : Reception n°%d (%d) [", num, lg);
	}
	else
	{
		printf("Erreur source d'affichage : [");
	}
	for (i=0; i<lg; i++) printf("%c", *(message+i));
	printf("]\n");
}
/********************************************/
/*----------------UDP-----------------------*/
/********************************************/

void udp_source (int port, int nb_message, int lg_msg, char* nom_station, char* message)
{
	int sock;
	struct hostent *hp;
	struct sockaddr_in adr_distant;
	char motif = 'a';



        printf("source UDP\n");
 		/*création d'un socket local*/
	  	if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
       	{
        	printf("Echec de la creation du socket source UDP");
            exit(1);
      	}
      	memset((char*)&adr_distant,0,sizeof(adr_distant));
	    adr_distant.sin_family = AF_INET;
	    adr_distant.sin_port = htons(port);
	    //Affectation adresse IP
        if ((hp= gethostbyname(nom_station))==NULL)
        {
            printf("erreur gethostbyname");
            exit(1);
        }
        memcpy((char*)&(adr_distant.sin_addr.s_addr),hp->h_addr,hp->h_length);

        /*Création du message*/
        printf("SOURCE : lg_mess_emis=%d, port=%d, nb_envois=%d, TP=udp, dest=%s\n",lg_msg,port,nb_message,nom_station);
		for (int i=1; i<=nb_message; i++)
	    {
	    	construire_message(message, motif++,lg_msg, i);	
			afficher_message(message, lg_msg, 1, i);			
			/*Envoi d'un message*/
	    		if ((sendto(sock, message, lg_msg, 0, (struct sockaddr*)&adr_distant, sizeof(adr_distant))) == -1)
				{
			  	printf("Erreur lors de l'envoi du message (sendto).\n");
				exit(1);
				}
	    }


        /*Fermeture du socket et libération de la mémoire*/
		if (close(sock) == -1)	
		{
			printf("Echec de destruction du socket\n");
			exit(1);
		}
		else
		{
			printf("SOURCE : fin\n");
		}
 
}

void udp_puits(int port, int nb_message, int lg_msg, char* nom_station, char* message)
{
	int sock; 
	struct sockaddr_in adr_local; //Adresse du socket local
	struct sockaddr_in adr_distant; //Adresse du socket distant
	int lg_adr_local = sizeof(adr_local);
	int lg_adr_distant = sizeof(adr_distant);
	int taille_max_msg = 30;

	/*Création d'un socket local*/
	if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
	{
	       	printf("Echec de la creation du socket puits UDP");
	       	exit(1);
	}
	memset((char*)&adr_local, 0, sizeof(adr_local));
	adr_local.sin_family = AF_INET; //domaine internet
	adr_local.sin_port = htons(port); //num de port
	adr_local.sin_addr.s_addr = INADDR_ANY; //toutes les adresses IP de la machine

	//Affectation de l'adresse au socket local à la représentation interne
	if (bind(sock, (struct sockaddr*) &adr_local, lg_adr_local) == -1)
	{
		printf("Echec du bind.\n");
		exit(1);
	}
	if (nb_message == -1){
			nb_message = 10;
            printf("PUITS : lg_mess_lu=%d, port=%d, nb_receptions=infini, TP=udp\n",lg_msg,port);
        } else printf("PUITS : lg_mess_lu=%d, port=%d, nb_receptions=%d, TP=udp\n",lg_msg,port,nb_message);

	while(1){
		/*Reception des messages et affichage*/
		for (int i=1; i<=nb_message; i++)
		{
			if (recvfrom(sock, message, taille_max_msg, 0, (struct sockaddr*) &adr_distant, (socklen_t *) &lg_adr_distant) == -1)
			{
			  	printf("Erreur lors de la reception du message (recvfrom).\n");
				exit(1);
			}
			else
			{
				/*Affichage du message recu*/
				afficher_message(message, lg_msg, 0, i);
			}
		}
	}

	/*Fermeture du socket et libération de la mémoire*/
	if (close(sock) == -1)	
	{
		printf("Echec de destruction du socket");
		exit(1);
	}
}
/************************************************************/
/*******************************TCP**************************/
/************************************************************/

void tcp_source(int port, int nb_message, int lg_msg, char* nom_station, char* message)
{
	/*Declarations*/
	int sock; //Représentation interne du socket local
	struct sockaddr_in adr_distant; //Adresse du socket distant
	int lg_adr_distant = sizeof(adr_distant);
	struct hostent *hp;
	char motif = 'a';

	/*Phase d'etablissement de connexion*/

	/*Création d'un socket local*/
       	if ((sock = socket(AF_INET, SOCK_STREAM, 0)) == -1)
       	{
               	printf("Echec de la creation du socket source TCP\n");
               	exit(1);
       	}

    	/*Construction de l'adresse de socket distant*/
    	//Affectation domaine et numero de port
       	memset((char*)&adr_distant, 0, sizeof(adr_distant));
       	adr_distant.sin_family = AF_INET; //domaine internet
       	adr_distant.sin_port = htons(port); //num de port
   		//Affectation adresse IP
    	if((hp = gethostbyname(nom_station)) == NULL)
      	{
            printf("Erreur lors de la requete de l'adresse IP (gethostbyname).\n");
        	exit(1);
      	}
    	memcpy((char*)&(adr_distant.sin_addr.s_addr), hp->h_addr, hp->h_length);

	/*Demande de connexion*/
	if(connect(sock,(struct sockaddr*) &adr_distant, lg_adr_distant) == -1)
	{
		perror("Erreur lors de la demande de la connexion (connect).\n");
		exit(1);
	}

	printf("SOURCE : lg_mess_emis=%d, port=%d, nb_envois=%d, TP=tcp, dest=%s\n",lg_msg,port,nb_message,nom_station);

	/*Phase de transfert de donnees*/
	for (int i=1; i<=nb_message; i++)
	{
  		/*Creation du message*/
	    construire_message(message, motif++,lg_msg, i);
		afficher_message(message, lg_msg, 1,i);

		/*Envoi d'un message*/
    	if ((write(sock, message, lg_msg)) == -1)
		{
		  	printf("Erreur lors de l'envoi du message (write).\n");
			exit(1);
		}
	}


	/*Phase de fermeture d'une connexion*/
	if (shutdown(sock, 2) == -1)
	{
		printf("Erreur lors de la fermeture de la connexion (shutdown).\n");
		exit(1);
	}

	/*Destruction du socket*/
	if (close(sock) == -1)	
	{
		printf("Echec de destruction du socket");
		exit(1);
	}
	else
	{
		printf("SOURCE : fin\n");
	}
}


/*-----------------------------------Gestion TCP puits------------------------------*/
void tcp_puits(int port, int nb_message, int lg_msg, char* nom_station, char* message)
{
	/*Declarations*/
	int sock; //Représentation interne du socket local
	int sock_bis; //Représention interne socket local dédié échange 
	struct sockaddr_in adr_distant; //Adresse du socket distant
	struct sockaddr_in adr_local; //Adresse du socket local
	int lg_adr_distant = sizeof(adr_distant);
	int lg_adr_local = sizeof(adr_local);
	int lg_msg_recu = -1; 
	int taille_max_msg = 30;

	/*Phase d'etablissement de connexion*/

	/*Création d'un socket local*/
       	if ((sock = socket(AF_INET, SOCK_STREAM, 0)) == -1)
       	{
               	printf("Echec de la creation du socket puits TCP\n");
               	exit(1);
       	}

	/*Construction de l'adresse de socket local*/
	//Affectation domaine et numero de port
	memset((char*)&adr_local, 0, sizeof(adr_local));
	adr_local.sin_family = AF_INET; //domaine internet
	adr_local.sin_port = htons(port); //num de port
	adr_local.sin_addr.s_addr = INADDR_ANY; //toutes les adresses IP de la machine

	//Affectation de l'adresse au socket local à la représentation interne
	if (bind(sock, (struct sockaddr*) &adr_local, lg_adr_local) == -1)
	{
		printf("Echec du bind.\n");
		exit(1);
	}

	/*Dimensionnement de la file d'attente des demandes de connexion entrantes*/
	if (listen(sock, 5) == -1)
	{
		printf("Erreur de listen.\n");
		exit(1);
	}

	if (nb_message == -1){
            printf("PUITS : lg_mess_lu_max=%d, port=%d, nb_receptions=infini, TP=tcp\n",taille_max_msg,port);
        } else printf("PUITS : lg_mess_lu=%d, port=%d, nb_receptions=%d, TP=tcp\n",taille_max_msg,port,nb_message);

	while(1)
	{
    		/*Acceptation d'une demande de connexion*/
		if ((sock_bis = accept(sock, (struct sockaddr*) &adr_distant, (socklen_t *) &lg_adr_distant)) == -1)
		{
			printf("Echec du accept.\n");
			exit(1);
		}


		int i = 1;
        while ((lg_msg_recu = read(sock_bis, message, taille_max_msg)) > 0) { 
            afficher_message (message, lg_msg_recu, 0,i);
            i++;
        }


		/*Phase de fermeture d'une connexion*/
		if (shutdown(sock_bis, 2) == -1)
		{
			printf("Erreur lors de la fermeture de la connexion (shutdown).\n");
			exit(1);
		}

		/*Destruction du socket*/
		if (close(sock_bis) == -1)	
		{
			printf("Echec de destruction du socket");
			exit(1);
		}
	}
}
/*******************************************************/
/*********************************BAL*******************/
/*******************************************************/

void emetteur(int port, int id, int nb_message, int lg_msg, char* nom_station, char* message)
{
	int sock; //Représentation interne du socket local
	struct sockaddr_in adr_distant; //Adresse du socket distant
	int lg_adr_distant = sizeof(adr_distant);
	struct hostent *hp;
	char motif = 'a';
	int taille_PDU = 10;
	char* pdu = malloc(taille_PDU * sizeof(char)); 
	//int envoi = -1;

	/*Entete dans le terminal*/
	if (nb_message != -1)
	{
    printf("SOURCE : lg_mesg-emis=%i, port=%i, nb_envois= %i, ""TP=TCP, dest=%s\n", lg_msg, port, nb_message, nom_station);
	}
    else
    {
    printf("SOURCE : lg_mesg-emis=%i, port=%i, nb_envois=10 (par défaut), " "TP=TCP, dest=%s\n", lg_msg, port, nom_station);
	}
	//Création d'une socket distante
    if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
       {
        printf("Echec de la creation du socket distant pour l'émetteur");
        exit(1);
       }
      memset((char*)&adr_distant,0,sizeof(adr_distant));
	    adr_distant.sin_family = AF_INET;
	    adr_distant.sin_port = htons(port);
	    //Affectation adresse IP
        if ((hp= gethostbyname(nom_station))==NULL)
        {
            printf("erreur gethostbyname");
            exit(1);
        }
        memcpy((char*)&(adr_distant.sin_addr.s_addr),hp->h_addr,hp->h_length);
        /*Demande de connexion*/
        if(connect(sock,(struct sockaddr*) &adr_distant, lg_adr_distant) == -1)
	    {
		    perror("Erreur lors de la demande de la connexion (connect).\n");
		    exit(1);
	    }
	    if (nb_message == -1) 
		{    
		nb_message = 10;
		}
        /*Envoie du PDU*/

        envoi_PDU(sock, pdu, taille_PDU, 0, id, nb_message, lg_msg);

        for (int i=1; i<=nb_message; i++)
	    {
      		/*Creation du message*/
	        construire_message(message, motif++,lg_msg, i);
		    afficher_msg(message, lg_msg,3, i,id);

		    /*Envoi d'un message*/
        	if ((write(sock, message, lg_msg)) == -1)
		    {
		      	printf("Erreur lors de l'envoi du message (write).\n");
			    exit(1);
		    }
	    }
	    
	    /*Phase de fermeture d'une connexion*/
	    if (shutdown(sock, 2) == -1)
	    {
		    printf("Erreur lors de la fermeture de la connexion (shutdown).\n");
		    exit(1);
	    }

	    /*Destruction du socket*/
	    if (close(sock) == -1)	
	    {
		    printf("Echec de destruction du socket");
		    exit(1);
	    }
	    else
	    {
		    printf("SOURCE : fin\n");
	    }
	    free(pdu);
}
void recepteur(int port, int id, char* nom_station, char* message)
{
	/****** Declarations ******/

	int sock; //Représentation interne du socket local
	struct sockaddr_in adr_serveur; 
	int lg_adr_serveur= sizeof(adr_serveur);
	int lg_msg_recu=-1;
	int taille_max_msg = 30;
	struct hostent *hp;
	int taille_pdu = 10;
	char* pdu = malloc(taille_pdu * sizeof(char)); 
	int i = 0;

	
		/*Entete dans le terminal*/
		printf("Demande de récupération de ses lettres par  le récepteur %i,port=%i,TP=TCP,dest=%s\n",id,port,nom_station);
	    /*Création d'un socket local*/
       	if ((sock = socket(AF_INET, SOCK_STREAM, 0)) == -1)
       	{
               	printf("Echec de la creation du socket source TCP\n");
               	exit(1);
       	}

    	/*Construction de l'adresse de socket distant*/
    	//Affectation domaine et numero de port
       	memset((char*)&adr_serveur, 0, sizeof(adr_serveur));
       	adr_serveur.sin_family = AF_INET; //domaine internet
       	adr_serveur.sin_port = htons(port); //num de port
   		//Affectation adresse IP
    	if((hp = gethostbyname(nom_station)) == NULL)
      	{
            printf("Erreur lors de la requete de l'adresse IP (gethostbyname).\n");
        	exit(1);
      	}
    	memcpy((char*)&(adr_serveur.sin_addr.s_addr), hp->h_addr, hp->h_length);
    	/*Demande de connexion*/
        if(connect(sock,(struct sockaddr*) &adr_serveur, lg_adr_serveur) == -1)
	    {
		    printf("Erreur lors de la demande de la connexion (connect).\n");
		    exit(1);
	    }
        else
	    {
	    	/****** Envoi du PDU à l'initiation de la communication ******/
			envoi_PDU(sock, pdu, taille_pdu, 1, id, 10, 30);

	        while ((lg_msg_recu = read(sock, message, taille_max_msg)) > 0)
	          { 
	            afficher_msg(message, lg_msg_recu, 4, ++i, id);
	          }
		}
		/*Destruction du socket*/
		if (close(sock) == -1)	
		{
			printf("Echec de destruction du socket");
			exit(1);
		}
		free(pdu);
}

void bal (int port, int lg_msg, char* message)
{
    int sock;
    int sock_com;
    struct sockaddr_in adr_client; //Adresse du socket distant
	struct sockaddr_in adr_local; //Adresse du socket local
	int lg_adr_client = sizeof(adr_client);
	int lg_adr_local = sizeof(adr_local);
    int nb_message = -1;
	int envoi = -1;
	int lg_msg_recu = -1; 
	int taille_max_msg = 30;
	//Création d'une liste de BAL vide 
	ListeBal* listeBAL = initialiser_liste_bal();

        printf("PUITS, port=%i,TP=TCP",port);
        
       	if ((sock = socket(AF_INET, SOCK_STREAM, 0)) == -1)
       	{
               	printf("Echec de la creation du socket local TCP\n");
               	exit(1);
       	}
       	if ((sock_com = socket(AF_INET, SOCK_STREAM, 0)) == -1)
       	{
               	printf("Echec de la creation du socket de communication TCP\n");
               	exit(1);
       	}
        memset((char*)&adr_local, 0, sizeof(adr_local));
	    adr_local.sin_family = AF_INET; //domaine internet
	    adr_local.sin_port = htons(port); //num de port
	    adr_local.sin_addr.s_addr = INADDR_ANY; //toutes les adresses IP de la machine

	    //Affectation de l'adresse au socket local à la représentation interne
	    if (bind(sock, (struct sockaddr*) &adr_local, lg_adr_local) == -1)
	    {
		    printf("Echec du bind.\n");
		    exit(1);
	    }

	    /*Dimensionnement de la file d'attente des demandes de connexion entrantes*/
	    if (listen(sock, 5) == -1)
	    {
		    printf("Erreur de listen.\n");
		    exit(1);
	    }
	    while(1)
	        {
	            if ((sock_com = accept(sock, (struct sockaddr*) &adr_client, (socklen_t *) &lg_adr_client)) == -1)
		        {
			        printf("Echec du accept.\n");
			        exit(1);
		        }
		        PDU* pdu = traitement_PDU(sock_com);
			/* MODE DISCUSSION EMETTEUR */
				if (pdu->mode == 0)
				{
					for (int i=1 ; i<=(pdu->nb_msg) ; i++)
					{
						if ((lg_msg_recu = read(sock_com, message, taille_max_msg)) <= 0)
						{
							printf("Erreur dans la réception (read)\n");
						}
						afficher_msg(message, lg_msg_recu, 1, i, pdu->id);
						enregistrer_lettre(pdu, i, message, &listeBAL);
					}
				}
				else if (pdu->mode == 1)
				{
				    Bal* courant = chercherBAL(pdu, &listeBAL, pdu->id);
					if ((courant == NULL) || (courant->premier == NULL))
					{
						printf("Pas de message enregistré");
					}
					else 
        			{
					    Lettre* msg = courant->premier;
					    nb_message = courant->nb_lettres;
	    
					    /*Phase de transfert de donnees*/
					    for (int i=1; i<=nb_message; i++)
					    {
      						/*Affichage du message*/
						    afficher_msg(msg->contenu, pdu->lg_msg, 2,i, pdu->id);
      	
						    /*Envoi d'un message*/
        						if ((envoi = write(sock_com, msg->contenu, pdu->lg_msg)) == -1)
						    {
						      	printf("Erreur lors de l'envoi du message (write).\n");
							    exit(1);
						    }
					        Lettre* deletion = msg;
						    courant->nb_lettres -= 1;
						    courant->premier = msg->suivante;
						    msg = courant->premier;
						    free(deletion->contenu);
						    free(deletion);
						}
				    }
				 }
				/*Phase de fermeture d'une connexion*/
				if (shutdown(sock_com, 2) == -1)
				{
					printf("Erreur lors de la fermeture de la connexion (shutdown).\n");
					exit(1);
				}
				else 
				{
					printf("Erreur de mode dans le serveur\n");
					exit(1);
				}
			/*Destruction du socket*/
			if (close(sock_com) == -1)	
			{
				printf("Echec de destruction du socket");
				exit(1);
			}
         }
}

/*-----------------------------------------------------------------------------*/
/*---------------------------Programme Principal-------------------------------*/
/*-----------------------------------------------------------------------------*/

void main (int argc, char **argv)
{
	int c;
	extern char *optarg;
	extern int optind;
	int lg_msg=10;
	char message[TAILLE];
	int protocole = 0; /* 0=tcp, 1=udp */
	int port;
	char nom_station[TAILLE];
	int nb_message = -1; /* Nb de messages à envoyer ou à recevoir, par défaut : 10 en émission, infini en réception */
	int source = -1 ; /* 0=puits, 1=source */
    int mod = -1; /* 0=serveur BAL, 1=emetteur, 2=recepteur*/
	int id = -1; /* identifient de l'emetteur ou du recepteur */

	while ((c = getopt(argc, argv, "psube:r:n:l:")) != -1) 
	{
		switch (c) {
		    case 'p': 
			    if (source == 1 || mod != -1) {
				    printf("usage: cmd [-p|-s][-u][-n ##][-l ##][station][port]\n"
					    "serveur [-e ##|-r ##|-b][-n ##][-l ##][station][port]\n");
				    exit(1);
			    } else source = 0;
			    break;

		    case 's':
			    if (source == 0 || mod != -1) {
				    printf("usage: cmd [-p|-s][-u][-n ##][-l ##][station][port]\n"
					    "serveur [-e ##|-r ##|-b][-n ##][-l ##][station][port]\n");
				    exit(1) ;
			    } else source = 1;
			    break;

		    case 'n':
			    nb_message = atoi(optarg);
			    break;
		    case 'l':
			    lg_msg= atoi(optarg);
			    break;	

		    case 'u':
		        protocole = 1;
		        printf("on est dans le protocole UDP");
			    break;
		    case 'b': 
		        if (source != -1 || mod == 1 || mod ==2)
		        {
				    printf("usage: cmd [-p|-s][-u][-n ##][-l ##][station][port]\n"
					    "serveur [-e ##|-r ##|-b][-n ##][-l ##][station][port]\n");
				    exit(1) ;
			    } 
			    else mod = 0;
			    break;
		    case 'e':
			    if (source != -1 || mod == 0 || mod ==2) 
				{
				    printf("usage: cmd [-p|-s][-u][-n ##][-l ##][station][port]\n"
					    "serveur [-e ##|-r ##|-b][-n ##][-l ##][station][port]\n");
				    exit(1) ;
			    } 
			    else {
			    mod = 1;
                id = atoi(optarg);
                }
			    break;

		    case 'r':
			    if (source != -1 || mod == 1 || mod ==0) 
			    {
				    printf("usage: cmd [-p|-s][-u][-n ##][-l ##][station][port]\n"
					    "serveur [-e ##|-r ##|-b][-n ##][-l ##][station][port]\n");
				    exit(1) ;
			    } 
			    else 
			    {
			    mod = 2;
                id = atoi(optarg);
                }
			    break;

		    default:
			    printf("usage: cmd [-p|-s][-u][-n ##][-l ##][station][port]\n"
				    "serveur [-e|-r|-b][-n ##][-l ##][station][port]\n");
			    break;
		    }
	}

    /*Recuperation du numero de port en dernier paramètre passé*/
	port = atoi(argv[argc-1]);

	/*Recuperation du nom de station */
	strcpy(nom_station,argv[argc-2]);
	
	if (source == -1 && mod == -1)
	{
		printf("usage: cmd [-p|-s][-u][-n ##][-l ##][station][port]\n"
				"serveur [-e|-r|-b][-n ##][-l ##][station][port]\n");
		exit(1) ;
	}
	if (nb_message != -1)
	 {
		if (source == 1)
		{
		printf("nb de tampons à envoyer : %d\n", nb_message);
		}
		else
		{
		printf("nb de tampons à recevoir : %d\n", nb_message);
		}
	 } 
	 else 
	 	{
		    if (source == 1) 
		    {
			    nb_message = 10 ;
			    printf("nb de tampons à envoyer = 10 par défaut\n");
		    } 
		    else
		    {
		    printf("nb de tampons à envoyer = infini\n");
	        }
	     }

	if (source == 1)
	{
		printf("on est dans la source\n");
		if (nb_message == -1) 
		{
		nb_message=5;
		}
		if (protocole == 1) /*Gestion source UDP*/
		{
			udp_source(port, nb_message, lg_msg, nom_station, message);
		}
		else if (protocole == 0)/*Gestion source TCP*/
		{
			tcp_source(port, nb_message, lg_msg, nom_station, message);
		}
		else
		{
			printf("Erreur dans le choix de protocole.\n");
			exit(1);
		} 
     }   
	else if(source==0)
	{   
		printf("on est dans le puits\n");
		if (protocole == 1) /*Gestion puits UDP*/
		{
			udp_puits(port, nb_message, lg_msg, nom_station, message);
		}
		else if (protocole == 0) /*Gestion source TCP*/
		{
			tcp_puits(port, nb_message, lg_msg, nom_station, message);
		}
		else
		{
			printf("Erreur dans le choix de protocole.\n");
			exit(1);
		}
	}	
	else
	{
		if (mod == 0) /* Mode serveur BAL */
		{
			bal(port, lg_msg, message);
		}
		else if (mod == 1) /* Mode emetteur */
		{
			emetteur(port, id, nb_message, lg_msg, nom_station, message);
		}
		else if (mod == 2) /* Mode recepteur */
		{
			recepteur(port, id, nom_station, message);
		}
		else
		{
			printf("Erreur de mode en BAL\n");
			exit(1);
		}
	}
 exit(0);	
}

	
    


	

