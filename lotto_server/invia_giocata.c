#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tipo.h"

int duplicato_num(int *arr, int num){
  int i;

  for(i=0; i<10; i++){
    if(arr[i] == num)
      return 1;
  }
  return 0;
}

void invia_giocata(int sd, int argc, char *argv[]){
  FILE *fptr;
  int i, j, indice, num;
  double imp, k;
  int ruo_c = 0;
  int num_c = 0;
  int imp_c = 0;
  char buf[BUFFER_SIZE];
  char opt = '0';
  struct schedina giocata;

  if(!logged){
    invia_stringa(sd, "Necessario fare il login.");
    return;
  }

  memset(&giocata, 0, sizeof(giocata));
  time(&giocata.rawtime);
  for(i=1; i<argc; i++){
    if(*argv[i] == '-'){
      if(strlen(argv[i]) == 2){
        opt = argv[i][1];
      } else {
        invia_stringa(sd, "Errore nei parametri, utilizzare \"help invia_giocata\".");
        return;
      }
    } else if(opt == 'r'){
      indice = estrai_indice_ruota(argv[i]);
      ruo_c++;
      if(indice >= 0){
        if(!giocata.ruote[indice]){
          giocata.ruote[indice] = 1;
        } else {
          sprintf(buf, "Nome della ruota \"%s\" duplicato.", argv[i]);
          invia_stringa(sd, buf);
          return;
        }
      } else if(strcmp(argv[i], "tutte") == 0){
          ruo_c = 11;
          for(j=0; j<11; j++){
            if(!giocata.ruote[j]){
              giocata.ruote[j] = 1;
            } else {
              sprintf(buf, "Nome della ruota \"%s\" duplicato.", argv[j]);
              invia_stringa(sd, buf);
              return;
            }
          }
      } else {
        sprintf(buf, "Nome della ruota \"%s\" non valido.", argv[i]);
        invia_stringa(sd, buf);
        return;
      }
    } else if(opt == 'n'){
      num = atoi(argv[i]);
      if(num_c > 9){
        invia_stringa(sd, "Superato il limite di 10 numeri.");
      } else if(num > 0 && num <= 90){
        if(duplicato_num(giocata.numeri, num)){
          sprintf(buf, "Il numero \"%s\" è duplicato.", argv[i]);
          invia_stringa(sd, buf);
          return;
        }
        giocata.numeri[num_c++] = num;
      } else {
        sprintf(buf, "Il numero \"%s\" non è valido.", argv[i]);
        invia_stringa(sd, buf);
        return;
      }
    } else if(opt == 'i'){
      imp = atof(argv[i]);
      if(imp_c > 4){
        invia_stringa(sd, "Sono consentiti solo 5 tipi di giocate.");
      } else if(imp >= 0){
        giocata.tipo[imp_c++] = imp;
      } else {
        sprintf(buf, "L'importo \"%s\" non è valido.", argv[i]);
        invia_stringa(sd, buf);
        return;
      }
    } else {
      invia_stringa(sd, "Errore nei parametri, info riguardo al formato su \"help invia_giocata\".");
      return;
    }
  }

  if(ruo_c == 0 || num_c == 0 || imp_c == 0){
    invia_stringa(sd, "Parametri non sufficienti.");
    return;
  }

  for(i=0; i<5 && i<num_c; i++){
    k = coeff_binomiale(num_c,i+1)*ruo_c;
    giocata.vincite[i] = premi[i]/k;
  }

  /*Se il file non esiste viene creato*/
  if((fptr = fopen(path,"ab")) == NULL){
    printf("Errore nella scrittura di %s.\n", path);
  } else {
    fwrite((void*)&giocata, sizeof(struct schedina), 1, fptr);
    fclose(fptr);
    printf("Scritto %s.\n", path);
  }
  invia_stringa(sd, "Giocata effettuata con successo.");
}
