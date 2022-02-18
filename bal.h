#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "coureur.h"

#ifndef BAL_H
#define BAL_H

struct lettre {
    int id;
    int lg_lettre;
    char motif;
    struct lettre * suivante;
    };

struct BAL{
    struct lettre * premier;
    struct lettre * courant;
    struct lettre * dernier;
    }

struct PDU{
    int mode; //1 Si émetteur 0 si récpeteur
    int id;
    int nb_msg;
    int lg_msg;
   };
   
typedef struct lettre lettre;
typedef struct BAL BAL;
typedef struct PDU PDU;




