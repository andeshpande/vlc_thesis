/************* UDP CLIENT CODE *******************/

#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

#define _ADDR "192.168.7.1" // IP address of the PC
#define _LADDR "0.0.0.0"

#define _PORT   7891  // data sent to this port of the PC
#define _LPORT  12345 //orientation information received at this port from phone
#define _USECS  500

char pStr[100];

struct thread_args{
  int phoneSocket;
  struct sockaddr *selfStorage;
  socklen_t *self_addr_size;
}_args;

//------------------------------------------------------------------------------
void * get_orientation(void * data) {
  struct thread_args *t = data;
  char *token;
  char d[3] = ", ";
    int pBytes;

    while(1)
    {
      pBytes = recvfrom(t->phoneSocket,
                        pStr,
                        100,
                        0,
                        t->selfStorage, 
                        t->self_addr_size); 

      token = strtok(pStr, d);
      token = strtok(NULL, d);
      token = strtok(NULL, d);
      token = strtok(NULL, d);

      strcpy(pStr, token);
      usleep(_USECS);
  }
}
//------------------------------------------------------------------------------
void get_state_values(FILE *lf, FILE *pf, char * str) {
  char lval[3] = "\0";
  char pval[3] = "\0";
  
  fscanf(lf, "%s", lval);
  rewind(lf);

  fscanf(pf, "%s", pval);
  rewind(pf);

  if (atoi(pval) > 500 || atoi(pval) == 0) printf("%s\n", pval);
  snprintf(str, 10, "%3s %3s", lval, pval);
}

//------------------------------------------------------------------------------
void get_rss(FILE* fp, char *str) {
  fscanf(fp, "%s", str);
  rewind(fp);
}
//------------------------------------------------------------------------------
int main(void) {
  pthread_t pth;

  int listenerSocket, portNum, nBytes;  
  char buffer[100], str[10];
  struct sockaddr_in senderAddr;
  socklen_t addr_size;

  int phoneSocket;
  struct sockaddr_in selfAddr;
  struct sockaddr_storage selfStorage;
  socklen_t self_addr_size;

  /*Create UDP socket*/
  listenerSocket = socket(PF_INET, SOCK_DGRAM, 0);
  phoneSocket    = socket(AF_INET, SOCK_DGRAM, 0);

  /*Configure settings in address struct*/
  senderAddr.sin_family = AF_INET;
  senderAddr.sin_port = htons(_PORT);
  senderAddr.sin_addr.s_addr = inet_addr(_ADDR);
  memset(senderAddr.sin_zero, '\0', sizeof senderAddr.sin_zero);

  /*Initialize size variable to be used later on*/
  addr_size = sizeof senderAddr;

  /*Configure settings in address struct*/
  selfAddr.sin_family = AF_INET;
  selfAddr.sin_port = htons(_LPORT);
  selfAddr.sin_addr.s_addr = inet_addr(_LADDR);
  memset(selfAddr.sin_zero, '\0', sizeof selfAddr.sin_zero);

  
  /*Bind socket with address struct*/
  bind(phoneSocket, (struct sockaddr *) &selfAddr, sizeof(selfAddr));

  /*Initialize size variable to be used later on*/
  self_addr_size = sizeof selfStorage;
 
  FILE *lf  = fopen("/proc/adcval/lval", "r");
  FILE *pf  = fopen("/proc/adcval/pval", "r");
  // FILE *rss = fopen("/proc/vlc/rss", "r");
  _args.phoneSocket   = phoneSocket;
  _args.selfStorage   = (struct sockaddr*) &selfStorage;
  _args.self_addr_size = &self_addr_size;

  pthread_create(&pth, NULL, get_orientation, (void *) &_args);

  while(1) {
    get_state_values(lf, pf, str);
    // get_rss(rss, str);
    snprintf (buffer, 100, "%s %3s", str, pStr);
    // printf("%s\n", buffer );
    // strcpy(buffer, str);
    /*Send message to server*/
    nBytes = strlen(buffer) + 1;
    sendto(listenerSocket,buffer,nBytes,0,(struct sockaddr *)&senderAddr,addr_size);
    memset(buffer, 0, 100);
    usleep(_USECS);
  }

  return 0;
}
