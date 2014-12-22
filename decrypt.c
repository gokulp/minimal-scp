#include <public.h>
#include <math.h>

int decryptFile(int argc, char * argv[])
{
       char ipFile[512], opFile[512];
       int status = 0, mode = 0;
       /* handle for encryption of data */
       gcry_cipher_hd_t handle;
       gcry_error_t err = 0;
       char passwd[PASSWD_LENGTH];
       char keybuffer[100];
       char * ptr;
       size_t keysize = 64;

       /* Form target file name */
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

       initializeProgram ();

       memset ( passwd, 0, PASSWD_LENGTH);
       printf("Enter the password: ");
       scanf("%s", passwd );

       /* This function generates key from the input string(passwd)    *\
       |* Keysize can be changed to any desirable value for now it     *|
       \* is kept as 64 bit. We will use part of it as per requirement */
       status = genKeyFromPasswd ( passwd, keybuffer, keysize);
       STD_CHECK_ERR_REPORT_AND_RETURN(status);

       printKey(keybuffer, keysize);

       /* this function generates a temporary file which contains all the encrypted       *\
       \* information except the hash key which is already authenticated by same function */
       status = authenticateInputFileAndGenTemp ( ipFile, "tmp.tmp", keybuffer);
       STD_CHECK_ERR_REPORT_AND_RETURN(status);

       /* This function is responsible for encryption or decrypting the input stream depending *\
       \* on the mode specified. Input is tmp file generated at last step                      */
       status = encryptDecryptFile( "tmp.tmp", opFile, keybuffer, DECRYPT);
       STD_CHECK_ERR_REPORT_AND_RETURN(status);

       /* Remove the temporary file */
       remove ("tmp.tmp");

       return(0);
}
