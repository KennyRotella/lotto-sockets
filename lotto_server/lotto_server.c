#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include "tipo.h"

/*controlla che non venga estratto lo stesso numero*/
int controlla_num(int *num, int x){
  int i;

  for(i=0; i<5; i++){
    if(num[i] == x){
      printf("A\n");
      return 0;
    }
  }
  return 1;
}

void estrattore(int period){
  FILE *fptr;
  char buf[BUFFER_SIZE];
  int i, j, x, count;
  struct estrazione estr;

  if(fork() != 0) return;

  while(1){
    *buf = '\0';
    count = BUFFER_SIZE;
    time(&estr.rawtime);
    for(i=0;i<11;i++){
      for(j=0;j<5;j++){
        x = rand()%90+1;
        while(!controlla_num(estr.num[i], x))
          x = rand()%90+1;
        estr.num[i][j] = x;
      }
    }

    /*Se il file non esiste viene creato*/
    if((fptr = fopen("estrazioni.bin","rb+")) == NULL){
      if((fptr = fopen("estrazioni.bin","wb+")) == NULL){
        printf("%s\n", "Errore di aperura estrazioni.bin");
        return;
      }
    }
    fseek(fptr, 0, SEEK_END);
    fwrite((void*)&estr, sizeof(struct estrazione), 1, fptr);
    formatta_estrazione(buf, &count, estr);
    printf("%s\n", buf);
    fclose(fptr);
    sleep(period);
  }
}

void req_handler(int cl_sd){
  char buf[BUFFER_SIZE];
  int i, argc;
  char *argv[30];

  if(fork() != 0){
    printf("Avviato il processo %d.\n", getpid());
    return;
  }

  for(i=0; i<30; i++){
    argv[i] = malloc(ARG_MAX);
  }

  while(1){
    ricevi_stringa(cl_sd, buf);
    strncpy(argv[0], " ", ARG_MAX);
    dividi_stringa(buf, &argc, argv);

    if(logged){
      argc--;
      if(strcmp(session_id, argv[argc]) != 0){
        invia_stringa(cl_sd, "Session_id non valido.");
        continue;
      }
    }

    if(strcmp(argv[0], "help") == 0){
      help(cl_sd, argc, argv);
    } else if(strcmp(argv[0], "signup") == 0){
      signup(cl_sd, argc, argv);
    } else if(strcmp(argv[0], "login") == 0){
      login(cl_sd, argc, argv);
    } else if(strcmp(argv[0], "invia_giocata") == 0){
      invia_giocata(cl_sd, argc, argv);
    } else if(strcmp(argv[0], "vedi_giocate") == 0){
      vedi_giocate(cl_sd, argc, argv);
    } else if(strcmp(argv[0], "vedi_estrazione") == 0){
      vedi_estrazione(cl_sd, argc, argv);
    } else if(strcmp(argv[0], "vedi_vincite") == 0){
      vedi_vincite(cl_sd, argc, argv);
    } else if(strcmp(argv[0], "esci") == 0){
      invia_stringa(cl_sd, "Connessione terminata.");
      break;
    } else {
      invia_stringa(cl_sd, "Comando non riconosciuto, utilizzare \"help\".");
    }
  }
  for(i=0; i<30; i++){
    free(argv[i]);
  }

  printf("Il client %s si è disconnesso.\n", session_id);
  close(cl_sd);
  exit(EXIT_SUCCESS);
}

int main(int argc, char* argv[]){
  int ret, sd, cl_sd, port, period, status;
  pid_t pid;
  socklen_t len;
  struct sockaddr_in my_addr, cl_addr;

  /* Verifica del numero di parametri */
  if(argc == 3){
    port = atoi(argv[1]);
    period = atoi(argv[2]);
  } else if(argc == 2){
    port = atoi(argv[1]);
    period = 300;
  } else {
    printf("Numero dei parametri errato.\n");
    exit(EXIT_FAILURE);
  }

  /* Verifica della porta */
  if(port < 1024 || port >= 65536){
    printf("Numero della porta errato.\n");
    exit(EXIT_FAILURE);
  }

  /* Verifica del periodo */
  if(period < 1){
    printf("Periodo di estrazione errato.\n");
    exit(EXIT_FAILURE);
  }

  estrattore(period);

  /* Creazione socket */
  printf("Creazione socket di ascolto.\n");
  sd = socket(AF_INET, SOCK_STREAM, 0);

  /* Creazione indirizzo di bind */
  memset(&my_addr, 0, sizeof(my_addr)); // Pulizia
  my_addr.sin_family = AF_INET;
  my_addr.sin_port = htons(port);
  my_addr.sin_addr.s_addr = INADDR_ANY;

  ret = bind(sd, (struct sockaddr*)&my_addr, sizeof(my_addr) );
  ret = listen(sd, 10);
  if(ret < 0){
      perror("Errore in fase di bind");
      exit(EXIT_FAILURE);
  }

  while(1){
    /* Accetto nuove connessioni */
    len = sizeof(cl_addr);
    cl_sd = accept(sd, (struct sockaddr*) &cl_addr, &len);
    inet_ntop(AF_INET, &(cl_addr.sin_addr), ip4, INET_ADDRSTRLEN);
    printf("Il client %s è connesso.\n", ip4);

    help(cl_sd, 1, 0);
    rand_string(session_id, 11);
    /* Creazione processi per gestire le richieste */
    req_handler(cl_sd);

    waitpid(-1, &status, WNOHANG);
  }
  pid = close(sd);
  if(pid)
    printf("Il processo %d è terminato.\n", pid);
  return 0;
}
