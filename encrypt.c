#include <public.h>

int main( int argc, char * argv[])
{
       char passwd[PASSWD_LENGTH], keybuffer[100], hmac_val[65]= {0}, ipFile[512], opFile[512], ipaddr[15] = {0}, port[5]= {0};
       size_t passLen, outlen = OUT_BUF_SIZE, keysize = 64;
       int status = 0, mode;
       FILE * ofp;
       struct stat st;

       if (argc != 3)
       {
            printf("Use:\n\t gatorenc <file_name> <IP:PORT>\n\t\t OR\n\t gatorenc <file_name> -l\n");
            exit(1);
       }

       if (strcmp(argv[2],"-l") == 0 )
       {
            printf("Operating for Local Mode\n");
            mode = LOCAL;
       }
       else
       {
            /* code for file transfer to server */
            mode = TRANSFER;
            memset ( ipaddr, 0, sizeof(ipaddr));
            memset ( port,   0, sizeof(port));
            status = splitPortAndIP (argv[2], ipaddr, port);
            STD_CHECK_ERR_REPORT_AND_RETURN(status);
       }

       strcpy(ipFile, argv[1]);
       strcpy(opFile, ipFile);
       strcat(opFile, ".uf");

       status = checkIfOpFileExist ( opFile );
       STD_CHECK_ERR_REPORT_AND_RETURN(status);

       status = initializeProgram ();
       STD_CHECK_ERR_REPORT_AND_RETURN(status);

       memset ( passwd, 0, PASSWD_LENGTH);
       memset ( keybuffer, 0, 100);

       printf("Enter the password: ");
       scanf("%s", passwd );
       passLen = strlen(passwd);

       status = genKeyFromPasswd ( passwd, keybuffer, keysize);
       STD_CHECK_ERR_REPORT_AND_RETURN(status);

       status = printKey(keybuffer, keysize);
       STD_CHECK_ERR_REPORT_AND_RETURN(status);

       status = encryptDecryptFile(ipFile, opFile, keybuffer, ENCRYPT);
       STD_CHECK_ERR_REPORT_AND_RETURN(status);
 
       /*reOpen encrypted file in append mode */
       status = openFile(opFile, "a+", &ofp);
       STD_CHECK_ERR_REPORT_AND_RETURN(status);

       status = calcHMACforFile(ofp, keybuffer,hmac_val);
       STD_CHECK_ERR_REPORT_AND_RETURN(status);

       status = writeFile (ofp, hmac_val, 1, 64, &outlen);
       STD_CHECK_ERR_REPORT_AND_RETURN(status);
       
       fclose(ofp);
       ofp= NULL;

       if ( mode == TRANSFER )
       {
           printf("Calling sendFile\n");
           status = sendFile (opFile, ipaddr, port);
           STD_CHECK_ERR_REPORT_AND_RETURN(status);
       }

       return(0);
}
