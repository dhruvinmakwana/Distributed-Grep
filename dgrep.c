#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h>
#include <signal.h>
#define IPADD "137.207.82.51"
#define PORTNO "5317"
#define BUFFERSIZE 5
 char* mergeStrings(char* str1,char* str2){

	 char * new_str ;
	 if (!str1){
		 new_str = malloc(strlen(str2)+1);
		 new_str[0] = '\0';
		 strcat(new_str,str2);
		 return new_str;

	 }
	 if((new_str = malloc(strlen(str1)+strlen(str2)+1)) != NULL){
	     new_str[0] = '\0';   // ensures the memory is an empty string
	     strcat(new_str,str1);
	     strcat(new_str,str2);
	 }
	 return new_str;
 }
int main(int argc, char *argv[]){
  char message[255];
  int server, pid, n;
  struct sockaddr_in servAdd;     // server socket address
  int port;
 if(argc != 4){
    printf("Call model: %s <Whole word to search> <file1> <file1> %d\n", argv[0],argc);
    exit(0);
  }

  if ((server = socket(AF_INET, SOCK_STREAM, 0)) < 0){
     fprintf(stderr, "Cannot create socket\n");
     exit(1);
  }

  servAdd.sin_family = AF_INET;
  sscanf(PORTNO, "%d", &port);
  servAdd.sin_port = htons((uint16_t)port);

  if(inet_pton(AF_INET, IPADD, &servAdd.sin_addr) < 0){
  fprintf(stderr, " inet_pton() has failed\n");
  exit(2);
}

 if(connect(server, (struct sockaddr *) &servAdd, sizeof(servAdd))<0){
	 perror("");
  fprintf(stderr, "connect() has failed, exiting\n");
  exit(3);
 }

 FILE *fp;
 char buffer[BUFFERSIZE];
 //send wor to be searched
 write(server, argv[1], strlen(argv[1])+1);
 //send second file name
 write(server, argv[3], strlen(argv[3])+1);
 fp = fopen(argv[3], "r");

 if (!fp){
	  printf("Cannot Open file %s\n",argv[3]);
	  exit(22);
 }
  pid=fork();

  if(pid){
		char command[100];
		sprintf(command, "grep --color=always -w %s %s /dev/null", argv[1],argv[2]);
		system(command);
                     /* reading server's messages */
		char* mergedmessage=NULL;
    	 while(n=read(server, message, 1)){
//          message[n]='\0';
//          printf("%s", message);
		 mergedmessage=mergeStrings(mergedmessage,message);
          if(!strcasecmp(message, "Bye\n")){
             exit(0);
           }
         }
	if(mergedmessage){
    	 printf("%s",mergedmessage);
	close(server);
}
  }else{
      fseek(fp, 0L, SEEK_END);
      int sz = ftell(fp);
      fseek(fp, 0L, SEEK_SET);
      char filesize[BUFFERSIZE];
      sprintf(filesize, "%d", sz);
      write(server, filesize, strlen(filesize)+1);
      while(fread(buffer, 1, 1, fp)){
    	  write(server, buffer, strlen(buffer));
      }
	close(server);
  }



}

