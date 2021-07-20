#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "tipo.h"

void formatta_ruota(char *buf, int *count, struct estrazione estr, int ind){
  int j;
  struct tm * timeinfo;
  char tmp[BUFFER_SIZE];

  timeinfo = localtime(&estr.rawtime);
  strftime(tmp, sizeof(tmp), "%d-%m-%Y %H:%M:%S\n", timeinfo);
  strncat(buf, tmp, *count);
  *count = BUFFER_SIZE-strlen(buf)-1;
  sprintf(tmp, "%-10s", ruote[ind]);
  strncat(buf, tmp, *count);
  *count = BUFFER_SIZE-strlen(buf)-1;
  for(j=0; j<5; j++){
    sprintf(tmp, "%2d\t", estr.num[ind][j]);
    strncat(buf, tmp, *count);
    *count = BUFFER_SIZE-strlen(buf)-1;
  }
  strncat(buf, "\n", *count);
  *count = BUFFER_SIZE-strlen(buf)-1;
}

void vedi_estrazione(int sd, int argc, char *argv[]){
  int k, quanti, ind, size, count;
  FILE *fptr;
  char buf[BUFFER_SIZE];
  struct estrazione estr;

  if(!logged){
    invia_stringa(sd, "Necessario fare il login.");
    return;
  }

  /*Se il file non esiste viene creato*/
  if((fptr = fopen("estrazioni.bin","rb+")) == NULL){
    if((fptr = fopen("estrazioni.bin","wb+")) == NULL){
      printf("%s\n", "Errore di aperura estrazioni.bin");
      invia_stringa(sd, "Errore interno.");
      invia_stringa(sd, "Termine.");
      return;
    }
  }

  fseek(fptr, 0, SEEK_END);
  size = ftell(fptr);
  if(size == 0){
    printf("%s\n", "Il file estrazioni.bin è vuoto");
    invia_stringa(sd, "Non ci sono ancora estrazioni.");
    invia_stringa(sd, "Termine.");
    return;
  }

  if(argc == 2){
    quanti = atoi(argv[1]);
    fseek(fptr, -sizeof(struct estrazione), SEEK_END);
    for(k=quanti; k>0; k--){
      size = ftell(fptr);
      fread((void*)&estr, sizeof(struct estrazione), 1, fptr);
      *buf = '\0';
      count = BUFFER_SIZE;
      formatta_estrazione(buf, &count, estr);
      invia_stringa(sd, buf);
      if(size == 0){
        break;
      }
      fseek(fptr, -2*sizeof(struct estrazione), SEEK_CUR);
    }
  } else if(argc == 3){
    quanti = atoi(argv[1]);
    ind = estrai_indice_ruota(argv[2]);

    if(ind > 0){
      fseek(fptr, -sizeof(struct estrazione), SEEK_END);
      for(k=quanti; k>0; k--){
        size = ftell(fptr);
        fread((void*)&estr, sizeof(struct estrazione), 1, fptr);
        *buf = '\0';
        count = BUFFER_SIZE;
        formatta_ruota(buf, &count, estr, ind);
        invia_stringa(sd, buf);
        if(size == 0){
          break;
        }
        fseek(fptr, -2*sizeof(struct estrazione), SEEK_CUR);
      }
    } else {
      sprintf(buf, "La ruota \"%s\" non è valida.", argv[2]);
      invia_stringa(sd, buf);
    }
  } else {
    invia_stringa(sd, "Parametri non sufficienti.");
  }
  invia_stringa(sd, "Termine.");
  fclose(fptr);
}
