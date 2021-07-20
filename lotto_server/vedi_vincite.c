#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tipo.h"

void formatta_vincita(char *buf, int *count, struct schedina giocata, double *vincite){
  struct tm *timeinfo;
  int i, j, k;
  int flag = 1;
  char tmp[BUFFER_SIZE];

  *count = BUFFER_SIZE-strlen(buf)-1;
  for(i=0; i<11; i++){
    if(giocata.presi[i][0]){
      if(flag){
        timeinfo = localtime(&giocata.rawtime);
        strftime(tmp, sizeof(tmp), "Estrazione del %d-%m-%Y ore %H:%M:%S\n", timeinfo);
        strncat(buf, tmp, *count);
        flag = 0;
      }
      sprintf(tmp, "%-10s", ruote[i]);
      strncat(buf, tmp, *count);
      *count = BUFFER_SIZE-strlen(buf)-1;
    } else continue;
    for(j=0; j<5; j++){
      if(giocata.presi[i][j])
        sprintf(tmp, "%d ", giocata.presi[i][j]);
      else break;
      strncat(buf, tmp, *count);
      *count = BUFFER_SIZE-strlen(buf)-1;
    }
    strncat(buf, "   >>   ", *count);
    *count = BUFFER_SIZE-strlen(buf)-1;
    for(k=j-1; k>=0; k--){
      sprintf(tmp, "%s ", tipo_giocata[k]);
      strncat(buf, tmp, *count);
      *count = BUFFER_SIZE-strlen(buf)-1;
      vincite[k] += giocata.vincite[k]*giocata.tipo[k]*coeff_binomiale(j,k+1);
      sprintf(tmp, "%.2f ", giocata.vincite[k]*giocata.tipo[k]*coeff_binomiale(j,k+1));
      strncat(buf, tmp, *count);
      *count = BUFFER_SIZE-strlen(buf)-1;
    }
    strncat(buf, "\n", *count);
    *count = BUFFER_SIZE-strlen(buf)-1;
  }
  if(!flag){
    strncat(buf, "***********************************************", *count);
    *count = BUFFER_SIZE-strlen(buf)-1;
  }
}

void vedi_vincite(int sd, int argc, char *argv[]){
  FILE *fptr;
  int size, count, i;
  double vincite[5];
  char buf[BUFFER_SIZE];
  char tmp[BUFFER_SIZE];
  struct schedina giocata;

  if(!logged){
    invia_stringa(sd, "Necessario fare il login.");
    return;
  }

  controlla_vincite();

  *buf = '\0';
  count = BUFFER_SIZE;
  for(i=0; i<5; i++){
    vincite[i] = 0;
  }

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

  fseek(fptr, -sizeof(struct schedina), SEEK_END);
  while(1){
    size = ftell(fptr);
    fread((void*)&giocata, sizeof(struct schedina), 1, fptr);

    if(giocata.estratta){
      *buf = '\0';
      count = BUFFER_SIZE;
      formatta_vincita(buf, &count, giocata, vincite);
      invia_stringa(sd, buf);
    }

    if(size == 0) break;
    fseek(fptr, -2*sizeof(struct schedina), SEEK_CUR);
  }
  fclose(fptr);

  strncat(buf, "\n", count);
  count = BUFFER_SIZE-strlen(buf)-1;
  for(i=0; i<5; i++){
    sprintf(tmp, "Vincite su %s: %.2f\n", tipo_giocata[i], vincite[i]);
    strncat(buf, tmp, count);
    count = BUFFER_SIZE-strlen(buf)-1;
  }
  invia_stringa(sd, buf);
  invia_stringa(sd, "Termine.");
}
