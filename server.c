#include<stdio.h>
#include<sys/types.h>
#include<string.h>
#include<stdlib.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<unistd.h>
#include <public.h>

int main(int argc,char**argv)
{
       int mode =0, status = 0;
       char ipFile[512] = {0}, opFile[512] = {0};
       char * ptr;

       if (argc != 3)
       {
            printf("Use:\n\t gatordec <file_name> <PORT>\n\t\t OR\n\t gatordec <file_name> -l\n");
            exit(1);
       }

       strcpy(ipFile, argv[1]);
       ptr = strstr(ipFile, ".uf");
       if (ptr != NULL)
       {
	    strncpy(opFile, ipFile, abs(ptr-ipFile));
       }
       else
       {
	    strcpy(opFile, ipFile);
	    strcat(opFile, ".dec");
       }

       status = checkIfOpFileExist ( opFile );
       STD_CHECK_ERR_REPORT_AND_RETURN(status);

       if (strcmp(argv[2],"-l") == 0 )
       {
            mode = LOCAL;
            printf("Operating for Local Mode\n");
       }
       else
       {
            /* code for file reception on server */
            receiveFile(ipFile, argv[2]);
       }

       decryptFile ( argc, argv);

       return(0);
}
