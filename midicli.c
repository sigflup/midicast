/*
 * midicast is released under the BSD 3-Clause license
 * read LICENSE for more info
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/audioio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#include "midicli.h" 

#define MAX_LINE 	1024

int sockfd;
char buffer[MAX_LINE];
struct sockaddr_in servaddr;

char *g_address;
unsigned short g_port; 

int poll_server(void) {
 int n;
 int len;
 len = sizeof(servaddr);
 n= recvfrom(sockfd, (char *)buffer, MAX_LINE,
             MSG_WAITALL, (struct sockaddr *)&servaddr, &len);
 if(n!=-1) {
  buffer[n] = '\0';
  return atoi(buffer);
 // printf("%s\n", buffer);
 }
 return -1;
}

void init_midi_client(char *name, char *address, unsigned short port) {
 int n, len;
 char hello[1024];

 g_address = strdup(address);
 g_port = port; 

 if( (sockfd = socket(AF_INET, SOCK_DGRAM | SOCK_NONBLOCK, 0)) <0) {
  perror("socket");
  exit(0);
 }

 memset(&servaddr, 0, sizeof(servaddr));

 servaddr.sin_family = AF_INET;
 servaddr.sin_port = htons(g_port);
 servaddr.sin_addr.s_addr = inet_addr(g_address);

 snprintf(hello, 1024, "client request%s", name);

 sendto(sockfd, (const char *)hello, strlen(hello),
		 0, (const struct sockaddr *)&servaddr,
		 sizeof(servaddr));

 printf("sent hello to %s:%d\n", g_address, g_port);
}

void midi_out(unsigned char data) {
 unsigned char send_buf[2];
 send_buf[0] = 'M';
 send_buf[1] = data;
 sendto(sockfd, (const char *)send_buf, 2,  0, 
		 (const struct sockaddr *)&servaddr,
		 sizeof(servaddr));
}

/*int main(void){
 int poll;
 init_midi_client();

 for(;;) {
  poll = poll_server();
  if(poll!=0) printf("::%02X::\n", poll);
   
 }
 close(sockfd);

}
*/


