#include <iostream>
#include <cstdlib>
#include <cstring>
#include <string>
#include <ctime>
#include <mpi.h>
#include "monde.hpp"

using namespace std;



struct monde* generation(const char* filename) {
    
    struct monde* m = (struct monde*)malloc(sizeof(struct monde));
    
    ifstream f(filename);
    if (!f) { cerr << "Fichier introuvable : " << filename << endl; MPI_Abort(MPI_COMM_WORLD, 1); }

    f >> m->nb_iles;
    int nb = m->nb_iles;
    m->mes_iles = (struct ile**)malloc(m->nb_iles*sizeof(struct ile*));


    for (int i=0; i<nb; i++) {
        m->mes_iles[i] = (struct ile*)malloc(sizeof(struct ile));
        f>>(m->mes_iles[i])->num>>(m->mes_iles[i])->nom >> (m->mes_iles[i])->nb_hab;

        // On alloue juste le tableau de pointeurs d'habitants, 
        // mais on laisse les habitants vides (NULL) pour l'instant (ils seront créés par les threads)
        m->mes_iles[i]->liste_hab = nullptr;
    }

    if (!f) { cerr << "Erreur de lecture du fichier" << endl; MPI_Abort(MPI_COMM_WORLD, 1); }

    m->mat_adj = (int*)calloc(nb*nb,sizeof(int));

    int nb_arete;
    f>>nb_arete;

    int a, b;
    for (int i=0; i<nb_arete; i++) {
        f>>a>>b;

        if (a < 0 || a >= nb || b < 0 || b >= nb) {
            cerr << "Arete invalide : " << a << " " << b << endl;
            continue;
        }

        m->mat_adj[a*nb+b] = 1;
        m->mat_adj[b*nb+a] = 1;
    }

    m->inflation = inflation_initiale;

    return m;    
}



// On prend un pointeur sur l'île pour modifier directement la vraie île
void creation_habitant(struct ile* ile_ptr) {
    ile_ptr->liste_hab = new habitant*[ile_ptr->nb_hab];

    for (int i = 0; i < ile_ptr->nb_hab; i++) {
        habitant* h = new habitant;
        ile_ptr->liste_hab[i] = h;

        snprintf(h->nom, sizeof(h->nom), "%s_%d", ile_ptr->nom, i);

        h->age = rand() % 100 + 1;
        h->revenu = (h->age < 18) ? 0 : 1000 + rand() % 9001;
    }
}



struct monde* init_world(struct monde* m, int nb_proc, int pid) {

    // Calcul du nombre d'îles par processus et du reste
    int nb_iles = m->nb_iles;
    int base  = nb_iles / nb_proc;
    int reste = nb_iles % nb_proc;

    int debut = pid * base + (pid < reste ? pid : reste);
    int fin   = debut + base + (pid < reste ? 1 : 0); // exclu


    struct monde* sous_monde = (struct monde*)malloc(sizeof(struct monde));
    sous_monde->inflation = m->inflation;
    sous_monde->nb_iles= fin-debut;
    
    // copie complète de la matrice (simple) : nb_total * nb_total
    int n      = m->nb_iles;          // taille du monde complet (mm)
    int nb_loc = fin - debut;

    sous_monde->mat_adj = (int*)malloc(nb_loc * nb_loc * sizeof(int));
    for (int i = 0; i < nb_loc; i++)
        for (int j = 0; j < nb_loc; j++)
            sous_monde->mat_adj[i * nb_loc + j] = m->mat_adj[(debut + i) * n + (debut + j)];

    // copie des îles locales
    sous_monde->mes_iles = (struct ile**)malloc(sous_monde->nb_iles * sizeof(struct ile*));
    for (int i = 0; i < sous_monde->nb_iles; i++) {
        sous_monde->mes_iles[i] = (struct ile*)malloc(sizeof(struct ile));
        *(sous_monde->mes_iles[i]) = *(m->mes_iles[debut + i]);   // copie des champs (num, nom, nb_hab...)
        sous_monde->mes_iles[i]->liste_hab = nullptr;             // ne pas partager le tableau de mm
        creation_habitant(sous_monde->mes_iles[i]);               // alloue liste_hab + habitants
    }
    
    return sous_monde;

}


void niveau1(struct monde* m){
    // les habitants de chaque île prennent tous un ans
    // si un habitant a 18 ans, ont initialise son revenu
    for (int i = 0; i < m->nb_iles; i++){
        for (int j = 0; j < m->mes_iles[i]->nb_hab; j++){
            habitant* h = m->mes_iles[i]->liste_hab[j];
            // habitant prend un ans
            h->age +=1;

            // idexation du salaires en fonction de l'inflation de l'île
            double inflation = m->mes_iles[i]->inflation; // en pourcentage
            h->revenu *= (1 + inflation/100.0);


            // si un habitant a 18 ans, ont initialise son revenu
            if (h->age == 18){
                h->revenu = 1000 + rand() % 9001;
            }
        }
    }



}



void show(int pid, int nb_proc, struct monde* m) {
    for (int p = 0; p < nb_proc; p++) {

        MPI_Barrier(MPI_COMM_WORLD);

        if (pid == p) {
            cout << "\n====================================" << endl;
            cout << "       PROCESSUS " << pid << endl;
            cout << "====================================" << endl;

            affichage_monde(m);

            cout << "Fin du processus " << pid << endl;
            cout << endl;
            cout.flush();
        }
    }
}


int main(int argc, char* argv[]) {
    MPI_Init(&argc, &argv);
    int nb_proc, pid;
    MPI_Comm_size(MPI_COMM_WORLD, &nb_proc);
    MPI_Comm_rank(MPI_COMM_WORLD, &pid);

    // Initialisation du générateur de nombres aléatoires avec une graine différente pour chaque processus
    srand(time(NULL) + pid);

    struct monde* mm = generation("monde2.txt");
    struct monde* sous_monde = init_world(mm, nb_proc, pid);
    
    show(pid, nb_proc, sous_monde);

    MPI_Barrier(MPI_COMM_WORLD);

    for (int i = 0; i < 5; i++) {
        niveau1(sous_monde);
        
        cout << "\n====================================" << endl;
        cout << "       PROCESSUS " << pid << " itération: " << i << endl;
        cout << "====================================" << endl;

        show(pid, nb_proc, sous_monde);
        MPI_Barrier(MPI_COMM_WORLD);
    }







    liberer_monde(&sous_monde);
    liberer_monde(&mm);

    MPI_Finalize();
    return EXIT_SUCCESS;
}