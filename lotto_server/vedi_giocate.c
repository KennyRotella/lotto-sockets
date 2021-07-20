#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "tipo.h"

void formatta_giocata(char *buf, int *count, struct schedina giocata){
  int i, num_r = 0;

  for(i=0; i<11; i++){
    num_r += giocata.ruote[i];
  }

  if(num_r == 11){
    strncat(buf, "Tutte ", *count);
    *count = BUFFER_SIZE-strlen(buf)-1;
  } else {
    for(i=0; i<11; i++){
      if(giocata.ruote[i]){
        strncat(buf, ruote[i], *count);
        *count = BUFFER_SIZE-strlen(buf)-1;
        strncat(buf, " ", *count);
        *count = BUFFER_SIZE-strlen(buf)-1;
      }
    }
  }
  for(i=0; i<10; i++){
    if(giocata.numeri[i]){
      char num[10];
      sprintf(num, "%d ", giocata.numeri[i]);
      strncat(buf, num, *count);
      *count = BUFFER_SIZE-strlen(buf)-1;
    }
  }
  for(i=0; i<5; i++){
    char imp[10];
    if(giocata.tipo[i] > 0){
      sprintf(imp, "* %.2f %s ", giocata.tipo[i], tipo_giocata[i]);
      strncat(buf, imp, *count);
      *count = BUFFER_SIZE-strlen(buf)-1;
    }
  }
}

void vedi_giocate(int sd, int argc, char *argv[]){
  FILE *fptr;
  char buf[BUFFER_SIZE];
  int tipo, count, size, num = 1;
  struct schedina giocata;

  if(!logged){
    invia_stringa(sd, "Necessario fare il login.");
    return;
  }

  controlla_vincite();

  /*Se il file non esiste viene creato*/
  if((fptr = fopen(path,"rb+")) == NULL){
    if((fptr = fopen(path,"wb+")) == NULL){
      printf("Errore apertura file %s.\n", path);
      invia_stringa(sd, "Errore interno.");
      invia_stringa(sd, "Termine.");
      return;
    }
  }

  fseek(fptr, 0, SEEK_END);
  size = ftell(fptr);
  if(size == 0){
    printf("Il file %s è vuoto.\n", path);
    invia_stringa(sd, "Non ci sono giocate.");
    invia_stringa(sd, "Termine.");
    fclose(fptr);
    return;
  }

  if(argc != 2){
    invia_stringa(sd, "Parametri non sufficienti.");
    invia_stringa(sd, "Termine.");
    fclose(fptr);
    return;
  }

  tipo = atoi(argv[1]);
  count = BUFFER_SIZE;
  fseek(fptr, -sizeof(struct schedina), SEEK_END);
  if(tipo == 1){
    while(1){
      size = ftell(fptr);
      fread((void*)&giocata, sizeof(struct schedina), 1, fptr);
      fseek(fptr, -2*sizeof(struct schedina), SEEK_CUR);
      if(!giocata.estratta){
        sprintf(buf, "%d) ", num++);
        count = BUFFER_SIZE-strlen(buf)-1;
        formatta_giocata(buf, &count, giocata);
        invia_stringa(sd, buf);
      }
      if(size == 0) break;
    }
  } else if(tipo == 0){
    while(1){
      size = ftell(fptr);
      fread((void*)&giocata, sizeof(struct schedina), 1, fptr);
      fseek(fptr, -2*sizeof(struct schedina), SEEK_CUR);
      if(giocata.estratta){
        sprintf(buf, "%d) ", num++);
        count = BUFFER_SIZE-strlen(buf)-1;
        formatta_giocata(buf, &count, giocata);
        invia_stringa(sd, buf);
      }
      if(size == 0) break;
    }
  } else {
    sprintf(buf, "Il tipo \"%s\" non è valido.", argv[1]);
    invia_stringa(sd, buf);
    invia_stringa(sd, "Termine.");
    fclose(fptr);
    return;
  }

  if(count == BUFFER_SIZE){
    sprintf(buf, "Non ci sono giocate di tipo %d.", tipo);
    invia_stringa(sd, buf);
  }
  invia_stringa(sd, "Termine.");

  fclose(fptr);
}
