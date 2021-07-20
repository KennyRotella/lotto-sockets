#include <stdio.h>
#include <string.h>
#include "tipo.h"

void help(int sd, int argc, char *argv[]){
  FILE *fptr;
  long int size;
  char buf[BUFFER_SIZE];

  if(argc == 2){
    if(strcmp(argv[1], "help") == 0){
      invia_stringa(sd, "!help <comando> --> mostra i dettagli di un comando");
    } else if(strcmp(argv[1], "signup") == 0){
      invia_stringa(sd, "!signup <username> <password> --> crea un nuovo utente");
    } else if(strcmp(argv[1], "login") == 0){
      invia_stringa(sd, "!login <username> <password> --> autentica un utente");
    } else if(strcmp(argv[1], "invia_giocata") == 0){
      invia_stringa(sd, "!invia_giocata g --> invia una giocata g al server");
    } else if(strcmp(argv[1], "vedi_giocate") == 0){
      invia_stringa(sd, "!vedi_giocate tipo --> visualizza le giocate precedenti dove tipo = {0,1} e permette di visualizzare le giocate passate ‘0’ oppure le giocate attive ‘1’ (ancora non estratte)");
    } else if(strcmp(argv[1], "vedi_estrazione") == 0){
      invia_stringa(sd, "!vedi_estrazione <n> <ruota> --> mostra i numeri delle ultime n estrazioni sulla ruota specificata");
    } else if(strcmp(argv[1], "vedi_vincite") == 0){
      invia_stringa(sd, "!vedi_vincite --> mostra tutte le vincite del client, l’estrazione in cui sono state realizzate e un consuntivo per tipologia di giocata");
    } else if(strcmp(argv[1], "esci") == 0){
      invia_stringa(sd, "!esci --> termina il client");
    } else {
      invia_stringa(sd, "Comando non riconosciuto.");
    }
  } else if(argc == 1){
    if((fptr = fopen("lotto.txt","rb+")) == NULL){
      invia_stringa(sd, "Errore interno");
      printf("%s\n", "Errore di aperura lotto.txt");
      return;
    }
    fseek(fptr, 0, SEEK_END);
    size = ftell(fptr);
    fseek(fptr, 0, SEEK_SET);
    fread((void*)buf, size, 1, fptr);
    invia_stringa(sd, buf);
    fclose(fptr);
  }
}
