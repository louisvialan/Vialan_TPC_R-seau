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

void main (int argc, char **argv)
{
	int c;
	int sock;
	extern char *optarg;
	struct sockaddr_in adr_distant;
	int lg_adr_distant = sizeof(adr_distant);
	char M[10]="aaaaaaaa";
	int lg_M= sizeof(M);
	int message;
	int lg_emis;
	int lg_adr_local;
	struct hostent *hp;
	struct sockaddr_in adr_local;
	extern int optind;
	int nb_message = -1; /* Nb de messages à envoyer ou à recevoir, par défaut : 10 en émission, infini en réception */
	int source = -1 ; /* 0=puits, 1=source */
	while ((c = getopt(argc, argv, "pn:s")) != -1) {
		switch (c) {
		case 'p':
			if (source == 1) {
				printf("usage: cmd [-p|-s][-n ##]\n");
				exit(1);
			}
			source = 0;
			break;

		case 's':
			if (source == 0) {
				printf("usage: cmd [-p|-s][-n ##]\n");
				exit(1) ;
			}
			source = 1;
			break;
		case 'u':
		    printf("on est en UDP");

		case 'n':
			nb_message = atoi(optarg);
			break;

		default:
			printf("usage: cmd [-p|-s][-n ##]\n");
			break;
		}
	}

	if (source == -1) {
		printf("usage: cmd [-p|-s][-n ##]\n");
		exit(1) ;
	}

	if (source == 1)
	{
		printf("on est dans le source\n");
	    sock = socket (AF_INET,SOCK_DGRAM,0);
	    if (sock==-1)
	    {
	     printf("échec de création du socket\n");
	    }
	    memset((char*)&adr_distant,0,sizeof(adr_distant));
	    adr_distant.sin_family = AF_INET;
	    adr_distant.sin_port = 9000;
        if ((hp= gethostbyname("vialan@insa-20744"))==NULL)
        {
            printf("erreur gethostbyname");
            exit(1);
        }
        memcpy((char*)&(adr_distant.sin_addr.s_addr),hp->h_addr,hp->h_length);
        //association adresse de socket  représentation interne 
        
        if(bind(sock,(struct sockaddr*)&adr_distant,lg_adr_distant==-1))
            {
             printf("échec du bind \n");
             exit(1);
            }
        //envoie du message M
        message= sendto(sock,M,lg_M,0,(struct sockaddr*)&adr_distant,lg_adr_distant); 
        if (message==-1)
        {
            printf("pas de message envoyé");
        }
      }
	else
	{
		printf("on est dans le puits\n");
		// création d'une socket local à un récepteur
		///reset de la structure
		/*memset((char*)& adr_local,0,sizeof(adr_local));
		
		adr_local.sin_family=AF_INET;
		adr_local.sin_port=9000;
		adr_local.sin_addr.s_addr=INADDR_ANY;
        if(bind(sock,(struct sockaddr*)&adr_local,lg_adr_local==-1))
            {
             printf("échec du bind \n");
             exit(1);
            }*/
     }
        		
	if (nb_message != -1) {
		if (source == 1)
			printf("nb de tampons à envoyer : %d\n", nb_message);
		else
			printf("nb de tampons à recevoir : %d\n", nb_message);
	} else {
		if (source == 1) {
			nb_message = 10 ;
			printf("nb de tampons à envoyer = 10 par défaut\n");
		} else
		printf("nb de tampons à envoyer = infini\n");

	}
}


