#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/wait.h>
#include <unistd.h>
#include <pthread.h>
#include <stdbool.h>

//Auteur : Nicolas Conguisti

//REMARQUE : Ce code fonctionne parfaitement bien :)

//LES COMMANDES DE COMPILATION ET D'EXECUTION
//gcc projet.c -o compil -Wall -lpthread
// ./compil


//INITIALISATION

//Création du tableau de chaises
#define nb_chaises 5
int tableau_chaises[nb_chaises];

//Création d'un mutex
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

//Création du signal coiffeur
pthread_cond_t demande_coiffeur = PTHREAD_COND_INITIALIZER;


//DEFINITION DES FONCTIONS ET PROCEDURES NECESSAIRES

int generer_nb_aleatoire(int nbMax) //Cette fonction permet de générer un nombre aléatoire entre 1 et nbMax passé en paramètre
{
  int nb;
  srandom(time(NULL));
  nb = rand() % (nbMax-1) + 1;

  return nb;
}

bool salle_attente_vide() //Cette fonction parcourt le tableau de chaise pour regarder si celui-ci est vide - Renvoie TRUE si le tableau est vide, FALSE s'il y a au moins 1 client
{
    bool estVide = true;
    //Parcours complet du tableau de chaises
    for( int i = 1; i <= nb_chaises; i++)
    {
        if(tableau_chaises[i] != 0)
        {
            estVide = false;
        }
    }
    return estVide;
}

bool salle_attente_pleine() //Cette fonction parcourt le tableau de chaise pour regarder si celui-ci est plein - Renvoie TRUE si le tableau est plein, FALSE s'il y a au moins 1 place libre
{
    bool estpleine = true;
    //Parcours complet du tableau de chaises
    for( int i = 1; i <= nb_chaises; i++)
    {
        if(tableau_chaises[i] == 0)
        {
            estpleine = false;
        }
    }

    return estpleine;
}

int premiere_place_dispo() //Cette fonction parcourt le tableau de chaise pour connaître la première place disponible - Renvoie -1 si le tableau est plein, l'indice de la premiere place dispo sinon
{
    int place = -1; // -1 = salle pleine
    for( int i = nb_chaises; i >= 1; i--)
    {
        //ajout de la place dispo si cette place est libre
        if(tableau_chaises[i] == 0)
        {
            place = i;
        }
    }
    return place;
}

void afficher_tableau_chaises() //Cette procédure parcourt le tableau de chaise en l'affichant
{
    printf("Salle d'attente : ");
    for( int i = 1; i <= nb_chaises; i++)
    {
        if(tableau_chaises[i] == 0)
        {
            printf("|.|");
        }
        else
        {
            printf("|%d|", tableau_chaises[i]);
        }
    }
    printf("\n \n");
}



//PROCEDURE EXECUTEE PAR LE COIFFEUR
void proc_coiffeur()
{

    while(1) //Le coiffeur ne s'arrète jamais. Il cherche et coiffe des clients à l'infini
    {
        //Le coiffeur regarde si un ou plusieurs clients sont dans la salle d'attente
        pthread_mutex_lock(&mutex);
        bool est_salle_attente_vide = salle_attente_vide();
        pthread_mutex_unlock(&mutex);

        //Si un ou plusieurs clients sont présents :
        if(est_salle_attente_vide == 0)
        {
            //On attend que le coiffeur coiffe le premier client arrivé
            sleep(5);

            //Le coiffeur envoie un signal au client pour lui dire qu'il est coiffé
            pthread_mutex_lock(&mutex);
            pthread_cond_signal(&demande_coiffeur);
            pthread_mutex_unlock(&mutex);

            sleep(1);
        }
        //Si la salle est vide, le coiffeur dort
        else
        {
           //printf("Coiffeur : Il n'y a personne, je dors");
        }
    }
    pthread_exit(NULL);
}



//PROCEDURE EXECUTEE PAR LES CLIENTS
void proc_client(void *args)
{
  int id = *(int*)args; //Attribution d'un id au client

  //Le client attend entre 1 et 10 secondes à entrer dans la salle d'attente
  int temps_pour_entrer_salle_attente = generer_nb_aleatoire(10);
  sleep(temps_pour_entrer_salle_attente);

  //Le client n'est pas coiffé
  bool coiffe = false;

  //Tant que le client n'est pas coiffé, il voudra toujours entrer dans le salon
  while(coiffe == false)
  {
    //Le client demande à entrer, on regarde si la salle d'attente est pleine
    pthread_mutex_lock(&mutex);
    bool salle_est_pleine = salle_attente_pleine();
    pthread_mutex_unlock(&mutex);

    //Si la salle d'attente n'est pas pleine, le client entre
    if(salle_est_pleine == 0)
    {
        //Le client entre
        printf("Client %d: j'entre en regardant la premiere place disponible \n \n", id);

        //Le client veut s'installer à la première place disponible
        pthread_mutex_lock(&mutex);
        int place = premiere_place_dispo();
        pthread_mutex_unlock(&mutex);

        if(place != -1) //Si la salle n'est pas pleine :
        {
            //Le client s'installe
            tableau_chaises[place] = id;

            printf("La place prise par le client %d est la place %d \n \n",id, place);

            afficher_tableau_chaises();

            printf("Client %d: j'attends d etre coiffe ! \n \n", id);


            //Le client attend d'être coiffé
            pthread_mutex_lock(&mutex);
            pthread_cond_wait(&demande_coiffeur, &mutex);
            pthread_mutex_unlock(&mutex);


            //Dès qu'il est coiffé, le client part
            printf("Client %d: je suis coiffe, Au revoir ! \n", id);

            //Le client libere la chaise
            pthread_mutex_lock(&mutex);
            tableau_chaises[place] = 0;
            pthread_mutex_unlock(&mutex);

            afficher_tableau_chaises();
            coiffe = true;
        }
    }
    //Si la salle d'attente est pleine, le client attend à l'extérieur
    else
    {
        printf("Client %d: j'attends a l exterieur ! \n", id);
        //Le client attend 10 secondes à l'exterieur
        sleep(10);
    }
  }

  pthread_exit(NULL);
}


//PROGRAMME PRINCIPAL
int main (void)
{
  //INITIALISATION
  printf("Il y a %d chaises dans le salon \n \n", nb_chaises);

  //Nombre de clients dans la rue
  //int nombre_clients = 3;
  int nombre_clients = generer_nb_aleatoire(10);
  printf("Il y a : %d clients dans la rue \n", nombre_clients);

  sleep(3);

  //Creation du tableau pour identifier les clients
  int tableau_id_clients [nombre_clients];


  //CREATION DES THREADS

  //Création du coiffeur
  pthread_attr_t coiffeur;
  pthread_t coiffeur_id;
  pthread_attr_init(&coiffeur);
  pthread_create(&coiffeur_id, &coiffeur, (void*)proc_coiffeur, (void*)NULL);


  //Création des clients
  pthread_attr_t client;
  for (int i = 1; i <= nombre_clients; i++)
    {
      //Création de l'identifiant du client dans le tableau des id clients
      tableau_id_clients[i] = i;

      //Création d'un client
      pthread_attr_init(&client);
      pthread_t client_id[i];
      pthread_create(&client_id[i], &client, (void*)proc_client, &tableau_id_clients[i]);

      sleep(1);
    }

    //Le coiffeur attend l'arrêt des clients
    pthread_join(coiffeur_id, NULL);

    //Destruction du mutex et du signal
    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&demande_coiffeur);

  return 0;
}
