
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <sys/wait.h>
#include <sys/shm.h>
#include <semaphore.h>
#include <fcntl.h>
#include <math.h>

#define Drapeau 0666
#define RandMaxi 100000
#define Fils 6
#define NB 1000000000


int creationSegmentMemoire(){
    int shmid = shmget(IPC_PRIVATE, RandMaxi * sizeof(int), Drapeau);
    if(shmid == -1){
        printf("Erreur : Memoire partagee non cree");
        exit(1);
    }
    return shmid;
}
unsigned int generationALea() {
    static unsigned int seed = 123456789;

    seed = (seed * 279470273UL) % 4294967291UL;

    return seed;
}

int nombreAleaDansIntervalle(int min, int max) {
    unsigned int nombreAleatoire = generationALea();
    int result = (int)((double)nombreAleatoire / 4294967291UL * (max - min + 1)) + min;
    return result;
}

void coefficientvariation(int *tab){
    long double somme = 0.0;
    double moy = 0.0;
    float var = 0.0, ecartType = 0.0, coefficientVar = 0.0;

    for(int i = 0; i < RandMaxi; i++){
        somme += tab[i];
    }
    moy = somme / (RandMaxi + 1);
    for (int i = 0; i < RandMaxi; ++i) {
        var += (tab[i] - moy) * (tab[i] - moy);
    }
    var /= (RandMaxi + 1);
    ecartType = sqrtf(var);
    coefficientVar = (ecartType / moy);
    printf("Coefficient de variation : %f\n", coefficientVar);
    if(coefficientVar < 0.05){
        printf("C 'est equilibree\n");
    }
    else{
        printf("Ce n'est pas equilibree\n");
    }
}

void destructionSegmentMemoire(int *adresse){
    if(shmdt(adresse) == -1){
        printf("Erreur : Memoire partagee non detruite");
        exit(1);
    }
}


int main(){
    int shmid = creationSegmentMemoire();
    int *tableau = (int*)shmat(shmid, (int*)0, Drapeau);
    srand(time(NULL));
    sem_t *sem = sem_open("semaphore", O_CREAT|O_EXCL, 0, 1);
    sem_unlink("semaphore");
    for(int i = 0; i < Fils; i++){
        if(fork() == 0){
            int* tmpRandom = (int*)calloc(RandMaxi, sizeof(int));
            int indiceRand;
            for(int j = 0; j < NB; j++){
                indiceRand = rand() % RandMaxi;
                tmpRandom[indiceRand]++;
            }
            sem_wait(sem);
            for(int j = 0; j < RandMaxi; j++){
                tableau[j] += tmpRandom[j];
                tmpRandom[j] = 0;
            }
            sem_post(sem);
            free(tmpRandom);
            destructionSegmentMemoire(tableau);
            exit(0);
        }
    }
    for(int i = 0; i < Fils; i++){
        wait(NULL);
    }
    printf("Pour le random :\n");
    coefficientvariation(tableau);
    printf("Nombre d'occurrences de chaque chiffre :\n");
    for (int i = 0; i < 10; ++i) {
        printf("%d a ete choisi %d fois.\n", i, tableau[i]);
    }
    int shmid2 = shmget(IPC_PRIVATE, RandMaxi * sizeof(int), Drapeau);
    int *tableau2 = (int*)shmat(shmid2, (int*)0, Drapeau);
    for(int i = 0; i < Fils; i++){
        if(fork() == 0){
            int* tmpPseudoAlea = (int*)calloc(RandMaxi, sizeof(int));
            int IndicePseudoAlea;
            for(int j = 0; j < NB; j++){

                IndicePseudoAlea = nombreAleaDansIntervalle(0, RandMaxi - 1);

                tmpPseudoAlea[IndicePseudoAlea]++;
            }
            sem_wait(sem);
            for(int j = 0; j < RandMaxi; j++){
                tableau2[j] += tmpPseudoAlea[j];
                tmpPseudoAlea[j] = 0;
            }
            sem_post(sem);
            free(tmpPseudoAlea);
            destructionSegmentMemoire(tableau2);
            exit(0);
        }
    }
    for(int i = 0; i < Fils; i++){
        wait(NULL);
    }
    coefficientvariation(tableau2);
    printf("Nombre d'occurrences de chaque chiffre :\n");
    for (int i = 0; i < 10; ++i) {
        printf("%d a ete choisi %d fois.\n", i, tableau2[i]);
    }
    exit(EXIT_SUCCESS);
}