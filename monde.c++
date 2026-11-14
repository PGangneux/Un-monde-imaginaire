#include "monde.hpp"


void affichage_ile(struct ile* ile) {
    cout << ile->nom << "->" << ile->nb_hab << " habitants : ";
    for (int i=0; i<ile->nb_hab; i++) {
        affichage_habitant(ile->liste_hab[i]);
        cout << endl;
    }
    cout << "inflation : " << ile->inflation << endl;
}

void affichage_monde(struct monde* m) {
    cout << m->nb_iles << " iles dans ce monde :" << endl;
    for (int i=0; i<m->nb_iles; i++)
        affichage_ile(m->mes_iles[i]);
    cout << "connectivité : " << endl;
    for (int i=0; i<m->nb_iles; i++) {
        for (int j=0; j<m->nb_iles; j++)
            cout << m->mat_adj[i*m->nb_iles+j] << " ";
        cout << endl;
    }
}

void affichage_habitant(struct habitant* h) {
    cout << h->nom << " \t" << h->age << " ans \t(" << h->revenu << "€)"; 
}

void liberer_monde(struct monde** m) {
    for (int i = 0; i < (*m)->nb_iles; i++) {
        struct ile* il = (*m)->mes_iles[i];
        if (il->liste_hab) {
            for (int j = 0; j < il->nb_hab; j++)
                delete il->liste_hab[j];
            delete[] il->liste_hab;
        }
        free(il);
    }
    free((*m)->mes_iles);
    free((*m)->mat_adj);
    free(*m);
    *m = NULL;
}