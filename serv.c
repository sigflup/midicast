/*
 * midicast is released under the BSD 3-Clause license
 * read LICENSE for more info
 */


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/audioio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#include "link.h"
#include "msg.h"

#define MAX_LINE 	1024
#define MAX_CLIENTS	128

unsigned short default_port = 1337;
char *default_addr = "127.0.0.1";
char *default_midi_dev = "/dev/rmidi0";
int default_forward = 0;

int midi_fp;

int sockfd;
char buffer[MAX_LINE];
struct sockaddr_in servaddr, cliaddr;

typedef struct {
 struct list_head node;
 struct sockaddr_in info;
} client_t;

client_t *list = (client_t *)0;

void broadcast_list(int a) {
 char buffer[MAX_LINE];
 client_t *walker;
 int j;

 snprintf(buffer, MAX_LINE, "byte: %d", a);
 walker=list;
 if(walker == (client_t *)0) return;
 for(;;) {
  snprintf(buffer, MAX_LINE, "%d", a);
  j = sendto(sockfd, (char *)buffer, strlen(buffer),
         0, (const struct sockaddr *)&walker->info, sizeof(cliaddr)); 
//  printf("%d broadcast to %04X data:%x\n",j, walker->info.sin_port, a);
  walker = (client_t *)walker->node.next;
  if(walker == list) break;
 }
}

int get_midi_byte() {
 unsigned char byte;
 if(read(midi_fp, &byte, sizeof(unsigned char))!=1)
  return  -1;
 return (int)byte;
}

void write_midi_byte(unsigned char data) {
 write(midi_fp, &data, sizeof(unsigned char));
}

int poll_midi(void) {
 int midi_byte;

 midi_byte = get_midi_byte();
 if(midi_byte != -1) {
  if(default_forward == 1) {
   write(midi_fp, &midi_byte, sizeof(unsigned char));
  }
  broadcast_list(midi_byte);
  fflush(stdout);
 }

 return 1;
}

int check_client(struct sockaddr_in *a) {
 client_t *walker;

 walker = list;
 if(walker == (client_t *)0) return 0;
 
 for(;;) {
  if(walker->info.sin_port == a->sin_port) return 1;
  walker = (client_t *)walker->node.next;
  if(walker == list) break;
 }

 return 0;
}

void add_client(struct sockaddr_in *a, char *name) {
 client_t *new;

 if(check_client(a) == 1) {
  printf("duplicit client port\n");
  return;
 }

 new = (client_t *)malloc(sizeof(client_t));

 memcpy(&new->info, a, sizeof(struct sockaddr_in));

 if(list == (client_t *)0) {
   list = new;
   INIT_LIST_HEAD(&list->node);
 } else
  list_add(&new->node, &list->node); 
 
 printf("connect %s\n", name);

}

void poll_request(void) {
 int len, n=0;
 len = sizeof(cliaddr);
 n = recvfrom(sockfd, (char *)buffer, MAX_LINE, 
		 MSG_WAITALL, (struct sockaddr *)&cliaddr,
		 &len);
 if(n!=-1) {
  buffer[n] = '\0';

  if(strncmp(buffer, "client request", 13)==0) {
   add_client(&cliaddr, &buffer[14]);

  } else {
   write_midi_byte(buffer[1]);
  }

 }
}

void usage(void) { 
 printf("usage:\n"
	"\tmidicast [-t] [-d device] [-a listen address] [-p port] [-h]\n");
 exit(0);
}

int main(int argc, char *argv[]) {
 int i;
 int opt;

 opterr = 0;
 while ((opt = getopt(argc, argv, "td:a:p:h")) != -1) { 
  switch(opt) {
   case 't':
    default_forward = 1;
    break;
   case 'd':
    default_midi_dev = optarg;
    break;
   case 'a':
    default_addr = optarg;
    break;
   case 'p':
    default_port = atoi(optarg);
    break;
   case 'h':
    usage();
    break;
   default:
    usage();
    break;
   
  }
 }
 

 puts(msg);
 printf("using %s listening on %s:%d\n", default_midi_dev, default_addr, default_port);
 if(default_forward == 1) 
  printf("midi through enabled\n");


 if(!(midi_fp=open(default_midi_dev, O_RDWR | O_NONBLOCK))) {
  perror(default_midi_dev);
  exit(0);
 }

 
 if((sockfd = socket(AF_INET, SOCK_DGRAM | SOCK_NONBLOCK, 0))<0) {
  perror("socket");
  exit(0);
 }

 memset(&servaddr, 0, sizeof(servaddr));
 memset(&cliaddr, 0, sizeof(cliaddr));

 servaddr.sin_family = AF_INET;
// servaddr.sin_addr.s_addr = "127.0.0.1"; 
 inet_aton(default_addr, &servaddr.sin_addr);
 servaddr.sin_port = htons(default_port);
 
 if(bind(sockfd, (const struct sockaddr *)&servaddr, sizeof(servaddr))<0) {
  perror("can't bind");
  exit(0);
 }

 for(i=0;;i++) {
  poll_request();
  poll_midi();
 }

}

