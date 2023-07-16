#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <string.h>
#include <time.h>

#define MAX_TITLE_LENGTH 50

typedef struct {
    int remaining_seats;
    char film_title[MAX_TITLE_LENGTH];
} CinemaData;

void afficheur_process(int* shared_memory) {
    CinemaData* cinema_data = (CinemaData*)shared_memory;

    // Boucle jusqu'à ce qu'il n'y ait plus de places disponibles
    while (cinema_data->remaining_seats > 0) {
        // Affichage du nombre de places restantes pour le film
        printf("Afficheur : Places restantes pour le film \"%s\" : %d\n", cinema_data->film_title, cinema_data->remaining_seats);
        sleep(5); // Attente de 5 secondes avant d'afficher à nouveau
    }
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
    int* shared_memory = (int*)shmat(shmid, NULL, 0);
    if (shared_memory == (int*)-1) {
        perror("Erreur lors de l'attachement de la mémoire partagée");
        return 1;
    }

    // Exécution du processus afficheur
    afficheur_process(shared_memory);

    // Détachement de la mémoire partagée
    shmdt(shared_memory);

    return 0;
}