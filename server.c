#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <time.h>
#define BUFFERSIZE 250
int loop=1;
void handle_client(int);
static void handler(const int signo)
{
	loop=0;
}

int main(int argc, char *argv[]){
  int sd, client, portNumber, status;
  struct sockaddr_in servAdd;      // client socket address
  struct sigaction a;
  a.sa_handler = handler;
  a.sa_flags = 0;
  sigemptyset( &a.sa_mask );
  sigaction( SIGINT, &a, NULL );
 if(argc != 2){
    printf("Call model: %s <Port Number>\n", argv[0]);
    exit(0);
  }
  if ((sd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
    fprintf(stderr, "Cannot create socket\n");
    exit(1);
  }
  sd = socket(AF_INET, SOCK_STREAM, 0);
  if (sd < 0) {
      perror("ERROR opening socket");
      exit(1);
  }
  if (setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int)) < 0){
    perror("setsockopt(SO_REUSEADDR) failed");
  }
  servAdd.sin_family = AF_INET;
  servAdd.sin_addr.s_addr = htonl(INADDR_ANY);
  sscanf(argv[1], "%d", &portNumber);
  servAdd.sin_port = htons((uint16_t)portNumber);

  if ( bind(sd, (struct sockaddr *) &servAdd, sizeof(servAdd))<0){
	perror("ERROR on binding");
      exit(1);
  }
  listen(sd, 5);


  while(loop){

    printf("Waiting for a client to process with\n");
    client = accept(sd, NULL, NULL);
    if(client<0){
    	exit(0);
    }
    printf("Got a client:%d, start processing\n",client);

    if(!fork())
    	handle_client(client);
    close(client);

    wait(&status);

 }
 shutdown(sd, SHUT_RD);
 close(sd);

}
void readUptoNull(int sd,char* buffer){
	char currChar;
	int n;
	int i=0;
	while(n=read(sd, &currChar, 1) && i<BUFFERSIZE){
		if(currChar!='\0'){
			buffer[i]=currChar;
		}else{
			break;
		}
		i++;
	}
	buffer[i]='\0';
}
void handle_client(int sd){
	char word[BUFFERSIZE];
	char filename[BUFFERSIZE];
	char rcvdfilename[BUFFERSIZE];	
	char filesize[BUFFERSIZE];
	int filesizeInt;
	char *fsptr;
	char buffer[BUFFERSIZE];

	int n, pid;

	/* reading client messages */
    readUptoNull(sd, word);

    readUptoNull(sd, filename);

    readUptoNull(sd, filesize);
    filesizeInt=(int)strtol(filesize,&fsptr,10);
    if(filesizeInt==0){
    	printf("Invalid file");
    	return;
    }
    FILE *fp;
	printf("\n\nReceiveing file - %s from client...\n",filename);
	//sprintf(filename,"%d_%s",time(NULL),rcvdfilename);
	//printf("saving file %s to temporary file %s\n",rcvdfilename,filename); 
	fp = fopen(filename, "w" );
	  if (fp<0){
		  perror("open");
		  printf("Cannot Open file");
		  exit(22);
	  }
	 int totalread=0;
	while(n=read(sd, buffer, 1)){
		fwrite(&buffer, 1, sizeof(buffer[0]), fp);
//		printf(" got word %s\n", buffer);
		totalread=totalread+n;
//		printf(" total read  %d\n", totalread);
		if(totalread==3231){
			printf("here");
		}
		if(totalread==filesizeInt){
			break;
		}
	}
	fclose(fp);
	printf("%d bytes received.\n",filesizeInt);
	printf("File transfer complete.\n");

char command[100];
    sprintf(command, "grep --color=always  -w %s %s /dev/null", word,filename);
	int status;
    if(fork()){
    	int orig=dup(STDOUT_FILENO);
        dup2(sd, STDOUT_FILENO);
//        dup2(sd, STDERR_FILENO);
        int ret=system(command);
        dup2(orig, STDOUT_FILENO);
        write(sd,'\0',1);
        if(ret==-1){
        	perror("msg");
        }
	//remove(filename);
	close(sd);
    }else{
    	wait(&status);
	printf("Sending Results.\n\n",filename);
    	close(sd);
//    	write(sd,"cl2",4);
    }

}
