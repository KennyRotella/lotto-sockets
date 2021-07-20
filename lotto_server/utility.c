#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <ctype.h>
#include "tipo.h"

/*Estrae da una stringa di caratteri tutte le sottostringhe delimitate
da almeno uno spazio, le sottostringhe sono copiate in argv, argc estrai_indice_ruota
 il numero di sottostringhe*/
void dividi_stringa(char *buf, int *argc, char *argv[]){
  char *tmp, buf_cp[BUFFER_SIZE], i;

  strcpy(buf_cp,buf);
  *argc = 0;
  tmp = strtok(buf_cp, " ");
  for(i=0; i<30 && tmp != NULL; i++){
    strncpy(argv[*argc], tmp, ARG_MAX);
    (*argc)++;
    tmp = strtok(NULL, " ");
  }
}

/*Comparazione stringhe case-insensitive*/
int strcicmp(char const *a, char const *b)
{
    for (;; a++, b++) {
        int d = tolower((unsigned char)*a) - tolower((unsigned char)*b);
        if (d != 0 || !*a)
            return d;
    }
}

/*Dato il nome di una ruota ne restituisce l'indice
corrispondente dell'array che la contiene in const.c*/
int estrai_indice_ruota(char *buf){
  int i;
  for(i=0; i<11; i++){
    if(strcicmp(buf, ruote[i]) == 0)
      return i;
  }
  return -1;
}

/*Prima di ogni scambio, il ricevente è informato su quanti byte
deve leggere dal socket. Vengono inviate al max BUFFER_SIZE bytes*/
int invia_stringa(int sd, char *buf){
  int ret, len;
  len = strlen(buf)+1;
  ret = send(sd, (void*)&len, sizeof(int), 0);
  if(ret < 0){
      perror("Errore in fase di invio");
      exit(EXIT_FAILURE);
  } else if(ret == 0) return ret;

  ret = send(sd, (void*)buf, len, 0);
  if(ret < 0){
      perror("Errore in fase di invio");
      exit(EXIT_FAILURE);
  }

  return ret;
}

/*Prima di ogni scambio, il ricevente è informato su quanti byte
deve leggere dal socket. Vengono inviate al max BUFFER_SIZE bytes*/
int ricevi_stringa(int sd, char *buf){
  int ret, len;
  ret = recv(sd, (void*)&len, sizeof(int), 0);
  if(ret < 0){
    perror("Errore in fase di ricezione");
    exit(EXIT_FAILURE);
  } else if(ret == 0) return ret;

  ret = recv(sd, (void*)buf, len, 0);
  if(ret < 0){
    perror("Errore in fase di ricezione");
    exit(EXIT_FAILURE);
  } else if(ret == 0) return ret;
  printf("%s: %s\n", ip4, buf);

  return ret;
}

void formatta_estrazione(char *buf, int *count, struct estrazione estr){
  int i, j;
  struct tm *timeinfo;
  char tmp[BUFFER_SIZE];

  timeinfo = localtime(&estr.rawtime);
  strftime(tmp, sizeof(tmp), "\nEstrazione del %d-%m-%Y ore %H:%M:%S\n", timeinfo);
  strncat(buf, tmp, *count);
  *count = BUFFER_SIZE-strlen(buf)-1;
  for(i=0; i<11; i++){
    sprintf(tmp, "%-10s", ruote[i]);
    strncat(buf, tmp, *count);
    *count = BUFFER_SIZE-strlen(buf)-1;
    for(j=0; j<5; j++){
      sprintf(tmp, "%2d\t", estr.num[i][j]);
      strncat(buf, tmp, *count);
      *count = BUFFER_SIZE-strlen(buf)-1;
    }
    strncat(buf, "\n", *count);
    *count = BUFFER_SIZE-strlen(buf)-1;
  }
}

/*Calcola il numero di sottoinsiemi non ordinati (di k elementi)
generabili da un insieme di n elementi*/
int coeff_binomiale(int n, int k){
  if (k == 0 || k == n)
      return 1;

  return coeff_binomiale(n - 1, k - 1) + coeff_binomiale(n - 1, k);
}

/*Controlla i numeri di una 'giocata' per ogni ruota se sono stati
estratti e li memorizza in presi[i][j] (i=ruota, j=numero)*/
void controlla_giocata(struct schedina *giocata, struct estrazione estr){
  int i, j, k, count;

  giocata->rawtime = estr.rawtime;
  giocata->estratta = 1;
  for(i=0;i<11;i++){
    count = 0;
    for(j=0;j<5;j++){
      if(giocata->ruote[i]){
        for(k=0; k<10; k++){
          if(giocata->numeri[k] == estr.num[i][j]){
            giocata->presi[i][count++] = estr.num[i][j];
          }
        }
      }
    }
  }
}

/*Cerca l'intervallo ]estr_prv,estr_nxt] che contiene il timestamp della giocata
e salva l'estrazione in estr (ret = 1), altrimenti ret = 0*, al primo avvio del server
viene inserita una dummy_estrazione per evitare il caso ]NULL,estr_nxt]*/
int cerca_estrazione(struct schedina giocata, struct estrazione *estr){
  FILE *fptr;
  int size;
  struct estrazione estr_prv;
  struct estrazione estr_nxt;

  /*Se il file non esiste viene creato*/
  if((fptr = fopen("estrazioni.bin","rb+")) == NULL){
    if((fptr = fopen("estrazioni.bin","wb+")) == NULL){
      printf("%s\n", "Errore di aperura estrazioni.bin");
      return 0;
    }
  }

  fseek(fptr, 0, SEEK_END);
  size = ftell(fptr);
  if(size == 0){
    printf("%s\n", "Il file estrazioni.bin è vuoto.");
    fclose(fptr);
    return 0;
  }

  fseek(fptr, -2*sizeof(struct estrazione), SEEK_END);
  while(1){
    size = ftell(fptr);
    fread((void*)&estr_prv, sizeof(struct estrazione), 1, fptr);
    fread((void*)&estr_nxt, sizeof(struct estrazione), 1, fptr);
    fseek(fptr, -3*sizeof(struct estrazione), SEEK_CUR);

    if(giocata.rawtime > estr_prv.rawtime && giocata.rawtime <= estr_nxt.rawtime){
      memcpy((void*)estr, (void*)&estr_nxt, sizeof(struct estrazione));
      break;
    }
    if(size == 0){
      fclose(fptr);
      return 0;
    }
  }

  fclose(fptr);
  return 1;
}

/*Utilizza le strutture dati globali associate al processo per estrarre le giocate
di tipo=1 dal file registro contenuto in 'path'*/
void controlla_vincite(){
  FILE *fptr;
  int size;
  struct schedina giocata;
  struct estrazione estr;

  /*Se il file non esiste viene creato*/
  if((fptr = fopen(path,"rb+")) == NULL){
    if((fptr = fopen(path,"wb+")) == NULL){
      printf("Errore apertura file %s.\n", path);
      return;
    }
  }

  fseek(fptr, 0, SEEK_END);
  size = ftell(fptr);
  if(size == 0){
    printf("Il file %s è vuoto.\n", path);
    fclose(fptr);
    return;
  }

  fseek(fptr, -sizeof(struct schedina), SEEK_END);
  while(1){
    size = ftell(fptr);
    fread((void*)&giocata, sizeof(struct schedina), 1, fptr);

    if(!giocata.estratta){
      if(cerca_estrazione(giocata, &estr)){
        controlla_giocata(&giocata, estr);
        fseek(fptr, -sizeof(struct schedina), SEEK_CUR);
        fwrite((void*)&giocata, sizeof(struct schedina), 1, fptr);
        printf("Scritto %s.\n", path);
      }
    } else break;

    if(size == 0) break;
    fseek(fptr, -2*sizeof(struct schedina), SEEK_CUR);
  }
  fclose(fptr);
}

/*Genera il session_id di 'size' caratteri alfanumerici casuali*/
char *rand_string(char *str, size_t size){
  size_t n;
  const char charset[] = "abcdefghijklmnopqrstuvwxyz1234567890";

  if (size) {
    --size;
    for (n = 0; n < size; n++) {
        int key = rand() % (int) (sizeof charset - 1);
        str[n] = charset[key];
    }
    str[size] = '\0';
  }
  return str;
}
