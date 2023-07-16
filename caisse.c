#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <time.h>

#define MAX_TITLE_LENGTH 50

typedef struct {
    int remaining_seats;
    char film_title[MAX_TITLE_LENGTH];
} CinemaData;

void caisse_process(int caisse_id, CinemaData* shared_memory) {
    int num_tickets;
    int processing_time;

    // Boucle jusqu'à ce qu'il n'y ait plus de places disponibles
    while (shared_memory->remaining_seats > 0) {
        // Génération aléatoire du nombre de tickets à acheter en dehors de la boucle
        num_tickets = rand() % 7 + 1;

        // Vérification si le nombre de tickets est inférieur ou égal aux places restantes
        if (num_tickets <= shared_memory->remaining_seats) {
            // Génération aléatoire du temps de traitement
            processing_time = rand() % 3 + 1;

            usleep(processing_time * 1000000); // Attente pour simuler le traitement

            // Mise à jour du nombre de places restantes
            shared_memory->remaining_seats -= num_tickets;

            // Affichage du nombre de tickets vendus et des places restantes
            printf("Caisse %d : Vendu %d place(s). Places restantes : %d\n", caisse_id, num_tickets, shared_memory->remaining_seats);
        }
    }

    // Affichage de la fermeture de la caisse
    printf("Caisse %d : Plus de places disponibles. Fermeture.\n", caisse_id);
}

int main() {
    // Récupération de l'identifiant de la mémoire partagée
    key_t key = ftok(".", 'R');
    int shmid = shmget(key, sizeof(CinemaData), 0666);
    if (shmid == -1) {
        perror("Erreur lors de l'accès à la mémoire partagée");
        return 1;
    }

    // Attachement de la mémoire partagée
    CinemaData* shared_memory = (CinemaData*)shmat(shmid, NULL, 0);
    if (shared_memory == (CinemaData*)-1) {
        perror("Erreur lors de l'attachement de la mémoire partagée");
        return 1;
    }

    // Exécution du processus caisse
    caisse_process(getpid(), shared_memory);

    // Détachement de la mémoire partagée
    shmdt(shared_memory);

    return 0;
}