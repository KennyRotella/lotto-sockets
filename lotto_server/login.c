#include <stdio.h>
#include <string.h>
#include "tipo.h"

void inserisci_bloccati(){
  FILE *fptr;
  FILE *fptrtxt;
  int size;
  struct bloccato bloc;
  struct tm * timeinfo;
  time_t rawtime;
  char tmp[BUFFER_SIZE];

  /*Se il file non esiste viene creato*/
  if((fptr = fopen("bloccati.bin","rb+")) == NULL){
    if((fptr = fopen("bloccati.bin","wb+")) == NULL){
      printf("%s\n", "Errore di aperura bloccati.bin");
      return;
    }
  }

  /*Se il file non esiste viene creato*/
  if((fptrtxt = fopen("bloccati.txt","r+")) == NULL){
    if((fptrtxt = fopen("bloccati.txt","w+")) == NULL){
      printf("%s\n", "Errore di aperura bloccati.txt");
      return;
    }
  }

  /*Salvataggio su file di testo*/
  time(&rawtime);
  timeinfo = localtime(&rawtime);
  fseek(fptrtxt, 0, SEEK_END);
  sprintf(tmp, "IP: %s, ", ip4);
  fwrite((void*)tmp, strlen(tmp), 1, fptrtxt);
  strftime(tmp, sizeof(tmp), "Timestamp: %d-%m-%Y %H:%M:%S\n", timeinfo);
  fwrite((void*)tmp, strlen(tmp), 1, fptrtxt);
  fclose(fptrtxt);

  /*Salvataggio su file binario*/
  fseek(fptr, 0, SEEK_END);
  size = ftell(fptr);
  if(size != 0){
    fseek(fptr, -sizeof(struct bloccato), SEEK_END);
    while(1){
      size = ftell(fptr);
      fread((void*)&bloc, sizeof(struct bloccato), 1, fptr);
      if(strcmp(bloc.ip4, ip4) == 0){
        time(&bloc.rawtime);
        fseek(fptr, -sizeof(struct bloccato), SEEK_CUR);
        fwrite((void*)&bloc, sizeof(struct bloccato), 1, fptr);
        fclose(fptr);
        return;
      }
      fseek(fptr, -2*sizeof(struct bloccato), SEEK_CUR);
      if(size == 0){
        break;
      }
    }
  }

  strncpy(bloc.ip4, ip4, INET_ADDRSTRLEN);
  time(&bloc.rawtime);
  fseek(fptr, 0, SEEK_END);
  fwrite((void*)&bloc, sizeof(struct bloccato), 1, fptr);
  fclose(fptr);
  return;
}

int estrai_attempt(){
  FILE *fptr;
  int size;
  struct bloccato bloc;

  /*Se il file non esiste viene creato*/
  if((fptr = fopen("bloccati.bin","rb+")) == NULL){
    if((fptr = fopen("bloccati.bin","wb+")) == NULL){
      printf("%s\n", "Errore di aperura bloccati.bin");
      return 3;
    }
  }

  fseek(fptr, 0, SEEK_END);
  size = ftell(fptr);
  if(size != 0){
    fseek(fptr, -sizeof(struct bloccato), SEEK_END);
    while(1){
      size = ftell(fptr);
      fread((void*)&bloc, sizeof(struct bloccato), 1, fptr);
      fseek(fptr, -2*sizeof(struct bloccato), SEEK_CUR);
      if(strcmp(bloc.ip4, ip4) == 0){
        fclose(fptr);
        return bloc.attempt;
      }
      if(size == 0){
        break;
      }
    }
  }
  fclose(fptr);
  return 3;
}

void aggiorna_attempt(int att){
  FILE *fptr;
  int size;
  struct bloccato bloc;

  /*Se il file non esiste viene creato*/
  if((fptr = fopen("bloccati.bin","rb+")) == NULL){
    if((fptr = fopen("bloccati.bin","wb+")) == NULL){
      printf("%s\n", "Errore di aperura bloccati.bin");
      return;
    }
  }

  fseek(fptr, 0, SEEK_END);
  size = ftell(fptr);
  if(size != 0){
    fseek(fptr, -sizeof(struct bloccato), SEEK_END);
    while(1){
      size = ftell(fptr);
      fread((void*)&bloc, sizeof(struct bloccato), 1, fptr);
      if(strcmp(bloc.ip4, ip4) == 0){
        bloc.attempt = att;
        fseek(fptr, -sizeof(struct bloccato), SEEK_CUR);
        fwrite((void*)&bloc, sizeof(struct bloccato), 1, fptr);
        fclose(fptr);
        return;
      }
      fseek(fptr, -2*sizeof(struct bloccato), SEEK_CUR);
      if(size == 0){
        break;
      }
    }
  }
  bloc.rawtime = 0;
  bloc.attempt = att;
  strncpy(bloc.ip4, ip4, INET_ADDRSTRLEN);
  fseek(fptr, 0, SEEK_END);
  fwrite((void*)&bloc, sizeof(struct bloccato), 1, fptr);
  fclose(fptr);
}

int controlla_bloccati(){
  FILE *fptr;
  int size;
  time_t rawtime;
  struct bloccato bloc;

  /*Se il file non esiste viene creato*/
  if((fptr = fopen("bloccati.bin","rb+")) == NULL){
    if((fptr = fopen("bloccati.bin","wb+")) == NULL){
      printf("%s\n", "Errore di aperura bloccati.bin");
      return 0;
    }
  }

  fseek(fptr, 0, SEEK_END);
  size = ftell(fptr);
  if(size == 0){
    fclose(fptr);
    return 0;
  }

  fseek(fptr, -sizeof(struct bloccato), SEEK_END);
  while(1){
    size = ftell(fptr);
    fread((void*)&bloc, sizeof(struct bloccato), 1, fptr);
    fseek(fptr, -2*sizeof(struct bloccato), SEEK_CUR);
    if(strcmp(bloc.ip4, ip4) == 0){
      time(&rawtime);
      fclose(fptr);
      if( (rawtime-bloc.rawtime) < 1800 ){
        return 1;
      } else {
        if(bloc.attempt <= 0){
          aggiorna_attempt(3);
        }
        return 0;
      }
    }
    if(size == 0){
      break;
    }
  }
  fclose(fptr);
  return 0;
}

void login(int sd, int argc, char *argv[]){
  struct account utente;
  FILE *fptr;
  int size, attempt;
  char buf[BUFFER_SIZE];

  if(logged){
    invia_stringa(sd, "Login già effettuato.");
    return;
  }

  if(controlla_bloccati()){
    invia_stringa(sd, "Troppi tentativi di connessione, riprovare più tardi.");
    printf("Il client %s è stato bloccato.\n", ip4);
    return;
  }

  attempt = estrai_attempt();

  if(argc != 3){
    invia_stringa(sd, "Parametri non sufficienti.");
    return;
  }

  /*Se il file non esiste viene creato*/
  if((fptr = fopen("login.bin","rb+")) == NULL){
    if((fptr = fopen("login.bin","wb+")) == NULL){
      invia_stringa(sd, "Errore interno.");
      return;
    }
  }
  fseek(fptr, 0, SEEK_END);
  size = ftell(fptr);
  if(size == 0){
    invia_stringa(sd, "E' necessario iscriversi.");
    fclose(fptr);
    return;
  }

  fseek(fptr, -sizeof(struct account), SEEK_END);
  while(1){
    size = ftell(fptr);
    fread((void*)&utente, sizeof(struct account), 1, fptr);
    fseek(fptr, -2*sizeof(struct account), SEEK_CUR);
    if(strcmp(utente.username, argv[1]) == 0 &&
       strcmp(utente.password, argv[2]) == 0){
      aggiorna_attempt(3);
      strncpy(path, "registro/", 10);
      strncat(path, utente.username, 20);
      strncat(path, ".bin", 5);
      sprintf(buf, "Logged: %s", session_id);
      invia_stringa(sd, buf);
      logged = 1;
      break;
    }
    if(size == 0){
      fclose(fptr);
      attempt--;
      if(attempt > 0){
        aggiorna_attempt(attempt);
        sprintf(buf, "Username e/o password non corretti, %d tentativi rimasti.", attempt);
        invia_stringa(sd, buf);
      } else {
        aggiorna_attempt(attempt);
        inserisci_bloccati();
        invia_stringa(sd, "Username e/o password non corretti, l'accesso sarà bloccato per 30 min");
      }
      return;
    }
  }
  fclose(fptr);
}
