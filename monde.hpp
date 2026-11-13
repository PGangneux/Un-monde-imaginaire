#ifndef MONDE_HPP
#define MONDE_HPP

#include <stdlib.h>
#include <fstream>
#include <iostream>
#include <string.h>

using namespace std;

#define inflation_initiale 2.5
#define revenu_min 1.0
#define revenu_max 10.0

struct habitant
{
    char nom[32];
    int age;
    float revenu;
};
struct ile
{
    int num;
    char nom[10];
    int nb_hab;
    struct habitant** liste_hab;
    float gini;      // L ’ indice de Gini `a partir du revenu mensuel
    int surface;     // La surface en hectare ( ha )
    float inflation; // une inflation locale `a l ’^ı le
};
struct monde
{
    struct ile **mes_iles; // pour la liste des îles
    int nb_iles;
    int *mat_adj;    // la matrice d ’ adjacence ( comment les ^ı les sont connect ´e es ) .
    float inflation; // une inflation initiale pour toutes les ^ı les ( en pourcent )
};

void affichage_monde(struct monde* m);

void affichage_ile(struct ile* ile);

void affichage_habitant(struct habitant* h);

void liberer_monde(struct monde** m);

#endif