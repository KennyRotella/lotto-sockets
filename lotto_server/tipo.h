#include <time.h>
#include <netinet/in.h>
#define BUFFER_SIZE 4096
#define ARG_MAX 20

/*Ogni processo conterrà valori diversi a seconda del client che sta gestendo,
queste varibili hanno visibilità globale*/
const char *ruote[11]; //definito in const.c
const char *tipo_giocata[5]; //definito in const.c
const double premi[5]; //definito in const.c
int logged; //variabile binaria per stabilire quando controllare il session_id
char session_id[11]; //stringa alfanumerica di 10 caratteri più fine stringa
char path[35]; //contiene il percorso del file registro associato al client
char ip4[INET_ADDRSTRLEN]; //contiene l'ip in forma leggibile del client gestito

/*contiene le informazioni delle giocate e delle vincite
una volta estratte*/
struct schedina {
  time_t rawtime; //timestamp della giocata se estratta=0, altrimenti dell'estrazione
  int numeri[10]; //contiene i numeri giocati, 0 se non giocati.
  int ruote[11]; //array di variabili binarie per sapere le ruote giocate
  double tipo[5]; //contiene l'importo per tipo di giocata
  int presi[11][5]; //una volta estratta salva i numeri presi per ogni ruota
  double vincite[5]; //ammontare della vincita per ogni tipo di giocata
  int estratta; //variabile binaria 0=non estratta, 1=estratta.
};

/*contiene informazioni di login per effettuare l'accesso, salvate in login.bin*/
struct account {
  char username[ARG_MAX];
  char password[ARG_MAX];
};

struct estrazione {
  time_t rawtime; //timestamp dell'estrazione
  int num[11][5]; //numeri dell'estrazione per ogni ruota
};

struct bloccato {
  time_t rawtime; //timestamp del terzo tentativo fallito di login
  int attempt; // numero di tentativi rimasti, ripristinati a 3 dopo il login o dopo 30 min dall'ultimo blocco
  char ip4[INET_ADDRSTRLEN]; //ip del client bloccato
};

/*funzioni di utilità definite in utility.c*/
int invia_stringa(int sd, char *buf);
int ricevi_stringa(int sd, char *buf);
void dividi_stringa(char *buf, int *argc, char *argv[]);
int estrai_indice_ruota(char *buf);
void formatta_estrazione(char *buf, int *count, struct estrazione estr);
int coeff_binomiale(int n, int k);
void controlla_giocata(struct schedina *giocata, struct estrazione estr);
int cerca_estrazione(struct schedina giocata, struct estrazione *estr);
void controlla_vincite();
char *rand_string(char *str, size_t size);
/*funzioni di utilità definite in file omonimi*/
void help(int sd, int argc, char *argv[]);
void signup(int sd, int argc, char *argv[]);
void login(int sd, int argc, char *argv[]);
void invia_giocata(int sd, int argc, char *argv[]);
void vedi_estrazione(int sd, int argc, char *argv[]);
void vedi_giocate(int sd, int argc, char *argv[]);
void vedi_vincite(int sd, int argc, char *argv[]);
