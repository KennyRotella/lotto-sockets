#include <stdio.h>
#include <string.h>
#include "tipo.h"

int trova_username(FILE *fptr, char *usr){
  int size;
  struct account utente;

  fseek(fptr, -sizeof(struct account), SEEK_END);
  while(1){
    size = ftell(fptr);
    fread((void*)&utente, sizeof(struct account), 1, fptr);
    fseek(fptr, -2*sizeof(struct account), SEEK_CUR);
    if(strcmp(utente.username, usr) == 0){
      return 1;
    }
    if(size == 0){
      return 0;
    }
  }
}

void signup(int sd, int argc, char *argv[]){
  FILE *fptr;
  int size;
  char buf[BUFFER_SIZE];
  char usr[20];
  char pas[20];
  struct account utente;

  if(logged){
    invia_stringa(sd, "Si dispone già di un account.");
    return;
  }

  if(argc != 3){
    invia_stringa(sd, "Parametri non sufficienti.");
    return;
  } else if(strlen(pas) > 19){
    sprintf(buf, "Username e password al più di 19 caratteri.");
    invia_stringa(sd, buf);
    return;
  }

  strcpy(usr, argv[1]);
  strcpy(pas, argv[2]);
  argc = 1;
  /*Se il file non esiste viene creato*/
  if((fptr = fopen("login.bin","rb+")) == NULL){
    if((fptr = fopen("login.bin","wb+")) == NULL){
      invia_stringa(sd, "Errore interno.");
      return;
    }
  }
  fseek(fptr, 0, SEEK_END);
  size = ftell(fptr);
  if(size != 0){
    while(1){
      if(strlen(usr) > 19 || argc != 1){
        sprintf(buf, "Username non valido, riprovare...");
        invia_stringa(sd, buf);
      } else if(trova_username(fptr, usr)){
        sprintf(buf, "Username già utilizzato, sceglierne un altro...");
        invia_stringa(sd, buf);
      } else break;
      ricevi_stringa(sd, buf);
      dividi_stringa(buf, &argc, argv);
      strcpy(usr, argv[0]);
    }
  }
  strcpy(utente.username, usr);
  strcpy(utente.password, pas);

  fseek(fptr, 0, SEEK_END);
  fwrite((void*)&utente, sizeof(struct account), 1, fptr);
  fclose(fptr);

  invia_stringa(sd, "Signup effettuato.");
}
