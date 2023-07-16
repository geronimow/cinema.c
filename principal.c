#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <string.h>
#include <time.h>

#define MAX_CAISSES 10
#define MAX_TITLE_LENGTH 50

typedef struct {
    int remaining_seats;
    char film_title[MAX_TITLE_LENGTH];
} CinemaData;

int main(int argc, char *argv[]) {
    if (argc < 4) {
        printf("Usage: %s <nombre de caisses> <titre du film> <nombre de places>\n", argv[0]);
        return 1;
    }

    int num_caisses = atoi(argv[1]);
    char film_title[MAX_TITLE_LENGTH];
    strncpy(film_title, argv[2], MAX_TITLE_LENGTH);
    int total_seats = atoi(argv[3]);

    // Création de la mémoire partagée
    key_t key = ftok(".", 'R');
    int shmid = shmget(key, sizeof(CinemaData), IPC_CREAT | 0666);
    if (shmid == -1) {
        perror("Erreur lors de la création de la mémoire partagée");
        return 1;
    }

    // Attachement de la mémoire partagée
    int* shared_memory = (int*)shmat(shmid, NULL, 0);
    if (shared_memory == (int*)-1) {
        perror("Erreur lors de l'attachement de la mémoire partagée");
        return 1;
    }

    CinemaData* cinema_data = (CinemaData*)shared_memory;
    cinema_data->remaining_seats = total_seats;
    strncpy(cinema_data->film_title, film_title, MAX_TITLE_LENGTH);

    srand(time(NULL)); // Initialisation de la graine pour les nombres aléatoires

    // Création des processus caisse
    int i;
    pid_t pid;
    for (i = 0; i < num_caisses; i++) {
        pid = fork();
        if (pid == 0) {
            // Exécution du programme caisse avec execl
            execl("./caisse", "caisse", NULL);
            perror("Erreur lors de l'exécution du programme caisse");
            return 1;
        } else if (pid == -1) {
            perror("Erreur lors de la création d'un processus caisse");
            return 1;
        }
    }

    if (pid != 0) {
        // Processus parent

        // Création du processus afficheur
        pid = fork();
        if (pid == 0) {
            // Exécution du programme afficheur avec execl
            execl("./afficheur", "afficheur", NULL);
            perror("Erreur lors de l'exécution du programme afficheur");
            return 1;
        } else if (pid == -1) {
            perror("Erreur lors de la création du processus afficheur");
            return 1;
        }

        // Attendre la fin des processus caisse
        for (i = 0; i < num_caisses; i++) {
            wait(NULL);
        }

        // Attendre la fin du processus afficheur
        wait(NULL);

        // Détachement de la mémoire partagée
        shmdt(shared_memory);

        // Suppression de la mémoire partagée
        shmctl(shmid, IPC_RMID, NULL);
    }

    return 0;
}