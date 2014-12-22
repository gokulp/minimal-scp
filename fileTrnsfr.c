#include <fileTrnsfr.h>

#define SA struct sockaddr
#define LISTENQ 5

int sendFile ( char * fname, char * ipaddr, char * port)
{
       int sockfd, len, fd = 0, numRead = 0;
       struct sockaddr_in servaddr,cliaddr;
       struct stat fst;
       unsigned char fnsize = 0;
       char fileSize[32], buffer[10];

       sockfd = socket( AF_INET, SOCK_STREAM, 0 );
       memset( &servaddr,0, sizeof( servaddr ) );

       servaddr.sin_family = AF_INET;
       servaddr.sin_addr.s_addr = htonl( INADDR_ANY );
       servaddr.sin_port = htons( atoi( port) );
       inet_pton( AF_INET, ipaddr, &servaddr.sin_addr);

       connect( sockfd, (SA*)&servaddr,sizeof( servaddr));

       stat(fname, &fst);
       sprintf(fileSize, "%ld", fst.st_size);
       write(sockfd,fileSize,32);

       fnsize = strlen(fname) + 1;
       write(sockfd,&fnsize, 1);

       write(sockfd, fname, strlen(fname) + 1);

       memset(buffer, 0, 10);
       fd = open(fname, O_RDONLY);

       while ( numRead = read(fd, buffer, 1) > 0 )
       {
	     write(sockfd,buffer, numRead);
             memset(buffer, 0, 10);
       }
       fflush(stdout);
       printf("\nthe file was sent successfully\n");
       return(SUCCESS);
}

int receiveFile (char * fname, char * port)
{
	    int fd, sockfd, lfd, connfd;
	    socklen_t client;
	    struct sockaddr_in servaddr,cliaddr;
	    char buffer[1000];
	    FILE *fp;
	    char fileSize[32];
	    int size =0 ;
	    unsigned char fnsize;
	    char fileName[256] = {0};
	    int numRead = 0;
	    int fileRead = 0;

	    lfd=socket(AF_INET,SOCK_STREAM,0);
	    memset(&servaddr, 0, sizeof(servaddr));

	    servaddr.sin_family=AF_INET;
	    servaddr.sin_addr.s_addr=htonl(INADDR_ANY);
	    servaddr.sin_port=htons(atoi(port));

	    bind(lfd,(SA*)&servaddr,sizeof(servaddr));
	    listen(lfd,LISTENQ);
	    client=sizeof(cliaddr);
	    connfd=accept(lfd,(SA*)&cliaddr,&client);

	    read(connfd,fileSize,32);
	    size = atol(fileSize);
	    printf("Expected to receive %d bytes\n", size);
	    fflush(stdout);

	    read(connfd, &fnsize, 1);
	    read(connfd, fileName, fnsize);
	    fp=fopen(fname,"w");

	    memset(buffer, 0, 1000);
	    while ( fileRead < size )
	    { 
		memset(buffer, 0, 1000);
		numRead = read(connfd, buffer, 1000) ;
		fwrite( buffer, 1, numRead, fp);
		fileRead += numRead;
	    }
            fclose (fp);
            fp = NULL;
	    printf("the file <%s> was received successfully\n", fileName);
}
