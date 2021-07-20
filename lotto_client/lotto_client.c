#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#define BUFFER_SIZE 4096
#define ARG_MAX 20

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
  printf("%s\n", buf);
  return ret;
}

int main(int argc, char* argv[]){
  int ret, sd, len, srv_port, argc1, count, i;
  int logged = 0;
  char *srv_ip;
  struct sockaddr_in srv_addr;
  char buf[BUFFER_SIZE];
  char tmp[BUFFER_SIZE];
  char *argv1[30];
  char session_id[11];

  /* Verifica del numero di parametri */
  if(argc == 3){
    srv_ip = argv[1];
    srv_port = atoi(argv[2]); //ASCII to integer
  } else {
    printf("Numero dei parametri errato.\n");
    exit(EXIT_FAILURE);
  }

  /* Creazione socket*/
  sd = socket(AF_INET, SOCK_STREAM, 0);

  /* Creazione indirizzo del server */
  memset(&srv_addr, 0, sizeof(srv_addr)); // Pulizia
  srv_addr.sin_family = AF_INET;
  srv_addr.sin_port = htons(srv_port);
  inet_pton(AF_INET, srv_ip, &srv_addr.sin_addr);

  ret = connect(sd, (struct sockaddr*)&srv_addr, sizeof(srv_addr));

  if(ret < 0){
      perror("Errore in fase di connessione");
      exit(EXIT_FAILURE);
  }

  /*Allocazione di array di char per fare il parsing dei comandi*/
  for(i=0; i<30; i++){
    argv1[i] = malloc(ARG_MAX);
  }

  /*Messaggio iniziale*/
  ricevi_stringa(sd, buf);
  while(1){
    printf("%s", "> ");
    /* Forzare l'output senza il linefeed */
    fflush(stdout);
    fgets(buf, BUFFER_SIZE, stdin);
    len = strlen(buf);
    /* Eliminazione del line feed */
    buf[len-1] = '\0';

    /*Concatena il session_id alla stringa dei comandi come ultimo parametro*/
    if(logged){
      count = BUFFER_SIZE - strlen(buf) - 1;
      sprintf(tmp, " %s", session_id);
      strncat(buf, tmp, count);
    }

    invia_stringa(sd, buf);
    dividi_stringa(buf, &argc1, argv1);

    if(strcmp(argv1[0], "help") == 0){
      ret = ricevi_stringa(sd, buf);
    } else if(strcmp(argv1[0], "signup") == 0){
      ret = ricevi_stringa(sd, buf);
    } else if(strcmp(argv1[0], "login") == 0){
      ret = ricevi_stringa(sd, buf);
    } else if(strcmp(argv1[0], "invia_giocata") == 0){
      ret = ricevi_stringa(sd, buf);
    } else if(strcmp(argv1[0], "vedi_giocate") == 0 ||
              strcmp(argv1[0], "vedi_estrazione") == 0 ||
              strcmp(argv1[0], "vedi_vincite") == 0){
      while(1){
        ret = ricevi_stringa(sd, buf);
        if(!ret) break;
        if(!logged) break;
        if(strcmp(buf, "Termine.") == 0){ //Riceve stringhe fino a quando riceve "Termine."
            break;
        }
      }
    } else if(strcmp(argv1[0], "esci") == 0){
      ret = ricevi_stringa(sd, buf); //Attende la conferma dal server
      break;
    } else {
      ret = ricevi_stringa(sd, buf);
    }

    if(!ret) break;
    /*Estrazione del session_id*/
    dividi_stringa(buf, &argc1, argv1);
    if(strcmp(argv1[0], "Logged:") == 0){
      strncpy(session_id, argv1[1], 11);
      logged = 1;
    }
  }

  for(i=0; i<30; i++){
    free(argv1[i]);
  }

  close(sd);
  return 0;
}
