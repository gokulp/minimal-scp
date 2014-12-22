
#include <encDecGenFunc.h>

/*---------------------------------------------------------------------------------------*\
 * This function handles all the initialization activities                               *
\*---------------------------------------------------------------------------------------*/
int initializeProgram ()
{
       /* Version check should be the very first call because it
          makes sure that important subsystems are initialized. */
       if (!gcry_check_version (GCRYPT_VERSION))
         {
           fputs ("libgcrypt version mismatch\n", stderr);
           exit (2);
         }

       gcry_control (GCRYCTL_SUSPEND_SECMEM_WARN);

       gcry_control (GCRYCTL_INIT_SECMEM, 16384, 0);

       gcry_control (GCRYCTL_RESUME_SECMEM_WARN);

       gcry_control (GCRYCTL_INITIALIZATION_FINISHED, 0);

       if (!gcry_control (GCRYCTL_INITIALIZATION_FINISHED_P))
       {
           fputs ("libgcrypt has not been initialized\n", stderr);
           return (FAILURE);
       }
       return(SUCCESS);
}

/*---------------------------------------------------------------------------------------*\
 * This function generates the key using PBKDF2 + SHA512 from input string and returns   *
 *  keybuffer and keysize                                                                *
\*---------------------------------------------------------------------------------------*/
int genKeyFromPasswd (unsigned const char * passwd, unsigned char * keybuffer, size_t keysize)
{
       gcry_error_t err = 0;
       char salt[] = { "NaCl" };
       size_t salten = 0;
       unsigned long iterations = 4096;
       size_t passLen;

       memset( keybuffer, 0, 100);
       passLen = strlen(passwd);
       salten = strlen (salt);

       /* algo: PBKDF2 subalgo: SHA512 iterations: 4096*/
       err = gcry_kdf_derive ( passwd, passLen, GCRY_KDF_PBKDF2, GCRY_MD_SHA512, salt, salten, iterations, keysize, keybuffer);
       GPG_CHECK_ERR_AND_RETURN(err);

       return(SUCCESS);
}

/*---------------------------------------------------------------------------------------*\
 * Prints given keybuffer in hexadecimal form.                                           *
\*---------------------------------------------------------------------------------------*/
int printKey ( unsigned char * keybuffer, size_t length )
{
       int i = 0;
       for ( i = 0; i < length ; i++ )
       {
           printf("%x ", (unsigned char)keybuffer[i] );
       }
       printf("\n");
       return(SUCCESS);
}

/*---------------------------------------------------------------------------------------*\
 * This function calculates HMAC value for the input stream specified in arguments.      *
 * Output MAC value is returned in o_md_string.                                          *
\*---------------------------------------------------------------------------------------*/

int calcHMACforFile(FILE *instream, unsigned char *keybuffer, unsigned char * o_md_string)
{
    size_t num;
    gcry_error_t err = 0;
    gcry_md_hd_t handle = NULL;
    static const size_t bufsize = 4096;
    void *buffer = calloc(bufsize, sizeof(char));

    err = gcry_md_open( &handle, GCRY_MD_SHA512, GCRY_MD_FLAG_HMAC);
    GPG_CHECK_ERR_AND_RETURN(err);

    size_t keyLength = gcry_cipher_get_algo_keylen(GCRY_MD_SHA512);

    err = gcry_md_setkey(handle, keybuffer, keyLength);
    GPG_CHECK_ERR_AND_RETURN(err);

    while ((num = fread(buffer, 1, bufsize, instream)) != 0) {
        gcry_md_write(handle, buffer, num);
    }

    strncpy( o_md_string, gcry_md_read(handle, GCRY_MD_SHA512), 64 );
    gcry_md_close(handle);
    free(buffer);
    buffer=NULL;

    return(0);
}

/*---------------------------------------------------------------------------------------*\
 * Retrieve MAC and encrypted data seperately. Encrypted data is dump into file stream   *
 * pointed by ofp. FileSize is of input stream.                                          *
\*---------------------------------------------------------------------------------------*/

int copyFileGetHMAC (FILE * ifp, FILE *ofp, size_t fileSize, unsigned char * hmac )
{
       int i , j = 0;
       size_t numBytesRead = 0;

       for ( i = 0 ; i < fileSize - 64; i++)
       {
           fputc(fgetc(ifp), ofp);
       }

       for ( ; i < fileSize ; i++)
       {
           hmac[j++]=fgetc(ifp);
       }

       return(0);
}

/*---------------------------------------------------------------------------------------*\
 * Function to handle file open operation and perform appropriate error handling         *
\*---------------------------------------------------------------------------------------*/

int openFile ( char * fileName, char * openMode, FILE **fp)
{
       *fp = NULL;
       *fp = fopen(fileName, openMode);
       if (!(*fp))
       {
            fprintf(stderr, "Error while opening file\nerrno: %d\n", errno);
            fprintf(stdout, "Error while opening file\nerrno: %d\n", errno);
	    return(FAILURE);
       }
       return(0);
}

/*---------------------------------------------------------------------------------------*\
 * Function to handle read operation and perform appropriate error handling              *
\*---------------------------------------------------------------------------------------*/

int readFile ( FILE * fp, void * buffer, size_t structSize, size_t bufSize, size_t * itemsRead)
{

       *itemsRead = fread(buffer, structSize, bufSize, fp);
       if ( ferror(fp) )
       {
            fprintf(stderr, "Error while reading data from file\nerrno: %d\n", errno);
            fprintf(stdout, "Error while reading data from file\nerrno: %d\n", errno);
	    return(FAILURE);
       }
       return(0);
}

/*---------------------------------------------------------------------------------------*\
 * Function to handle write operation and perform appropriate error handling             *
\*---------------------------------------------------------------------------------------*/

int writeFile ( FILE * fp, void * buffer, size_t structSize, size_t bufSize, size_t * itemsWritten)
{

       *itemsWritten = fwrite(buffer, structSize, bufSize, fp);
       if ( ferror(fp) )
       {
            fprintf(stderr, "Error while reading data from file\nerrno: %d\n", errno);
            fprintf(stdout, "Error while reading data from file\nerrno: %d\n", errno);
	    return(FAILURE);
       }
       return(SUCCESS);
}

/*---------------------------------------------------------------------------------------*\
 * This funtion is responsible for reading input stream and dumping only encrypted data  *
 * to the temporary file. Read HMAC key from end of input to temporary buffer. Generate  *
 * HMAC locally with key and compare with one from file.                                 *
\*---------------------------------------------------------------------------------------*/

/* Temporary file will be generated in present working directory */
int authenticateInputFileAndGenTemp (char * ipFullFileName, char * tmpFileName, unsigned char * keybuffer )
{
       int status = 0;
       FILE *fp = NULL, *tmpfp = NULL;
       struct stat fst;
       unsigned char hmac_val[65] = {0};
       unsigned char calculate_hmac[65] = {0};
        
       stat ( ipFullFileName, &fst);

       status = openFile ( ipFullFileName, "rb", &fp);
       STD_CHECK_ERR_REPORT_AND_RETURN(status);

       status = openFile ( tmpFileName, "wb", &tmpfp);
       STD_CHECK_ERR_REPORT_AND_RETURN(status);

       /* copy the complete input encrypted file in temp file except HMAC value *\
       \* extract HMAC in seperate string to be compared for authentication     */
       status = copyFileGetHMAC(fp, tmpfp, fst.st_size, hmac_val);
       STD_CHECK_ERR_REPORT_AND_RETURN(status);

       /* close temp and input files - No need of input encrypted file from this point*\
       \* all operations will be carried out on temporary file                        */
       fclose(tmpfp);
       tmpfp=NULL;
       fclose(fp);
       fp = NULL;

       /* Make new truncated(HMAC removed) file as input file */ 
       status = openFile ( tmpFileName, "rb", &tmpfp);
       STD_CHECK_ERR_REPORT_AND_RETURN(status);

       /* Calculate HMAC for the file locally*/
       status = calcHMACforFile (tmpfp, keybuffer, calculate_hmac);
       STD_CHECK_ERR_REPORT_AND_RETURN(status);

       /* Compare HMAC from input file and one created locally */
       if (strncmp(hmac_val, calculate_hmac, 64) != 0)
       {
           printf("Signature Authentication failed\nExit code: 62\n");
           exit(62);
       }
       else
       {
           printf("Signature Authenticated\n");
       }

       fclose(tmpfp);
       tmpfp = NULL;

       return(SUCCESS);
}

/*---------------------------------------------------------------------------------------*\
 * This function is responsible for encrypting or decrypting the input stream and saving *
 * output to the output stream specified by opFullFileName.                              *
 * mode has to valid values ENCRYPT(1) and DECRYPT(2)                                    *
\*---------------------------------------------------------------------------------------*/

int encryptDecryptFile ( char * ipFullFileName, char * opFullFileName, unsigned char * keybuffer, int mode )
{
       /* handle for encryption of data */
       gcry_cipher_hd_t handle;
       gcry_error_t err = 0;

       FILE * fp = NULL, *ofp=NULL;
       int intIV = 5844, status;
       char inbuf[IN_BUF_SIZE], outbuf[OUT_BUF_SIZE], iv[16]= {0};
       size_t inlen = IN_BUF_SIZE, outlen = OUT_BUF_SIZE;

       status = openFile ( ipFullFileName, "rb", &fp);
       STD_CHECK_ERR_REPORT_AND_RETURN(status);

       memset(iv,0,16);
       memcpy(iv,&intIV, sizeof(intIV));

       status = openFile ( opFullFileName, "wb", &ofp);
       STD_CHECK_ERR_REPORT_AND_RETURN(status);

       err = gcry_cipher_open ( &handle, GCRY_CIPHER_AES128, GCRY_CIPHER_MODE_CBC, GCRY_CIPHER_CBC_CTS );
       GPG_CHECK_ERR_AND_RETURN(err);

       while (!feof(fp))
       {
	    memset (inbuf, 0, IN_BUF_SIZE);
	    memset (outbuf, 0, OUT_BUF_SIZE);
            outlen = OUT_BUF_SIZE;

            status = readFile(fp, inbuf, 1, IN_BUF_SIZE, &inlen);
            STD_CHECK_ERR_REPORT_AND_RETURN(status);
            
            err = gcry_cipher_reset(handle);
	    GPG_CHECK_ERR_AND_RETURN(err);

            size_t blkLength = gcry_cipher_get_algo_blklen(GCRY_CIPHER_AES128);
	    err = gcry_cipher_setiv ( handle, &iv, blkLength);
	    GPG_CHECK_ERR_AND_RETURN(err);

            size_t keyLength = gcry_cipher_get_algo_keylen(GCRY_CIPHER_AES128);
	    err = gcry_cipher_setkey ( handle, keybuffer, keyLength);
	    GPG_CHECK_ERR_AND_RETURN(err);

            switch ( mode )
            {
            case ENCRYPT: 
	                  err = gcry_cipher_encrypt ( handle, outbuf, inlen, inbuf, inlen);
	                  GPG_CHECK_ERR_AND_RETURN(err);
                          break;
            case DECRYPT:
	                  err = gcry_cipher_decrypt ( handle, outbuf, inlen, inbuf, inlen);
	                  GPG_CHECK_ERR_AND_RETURN(err);
                          break;
            default:
                          printf("encryptDecryptFile called in invalid mode!!!\n");
                          return(FAILURE);
            }

            status = writeFile (ofp, outbuf, 1, inlen, &outlen);
            STD_CHECK_ERR_REPORT_AND_RETURN(status);
            
       } 
       gcry_cipher_close(handle);
       
       fclose(fp);
       fp = NULL;
       fclose(ofp);
       ofp = NULL;

       return(SUCCESS);
}

/*---------------------------------------------------------------------------------------*\
 * This function is responsible for dividing input string into two parts IP and port.    *
 * If IP is not mentioned in input then machines self IP is stored in ipaddr and         *
 * default value for port is set to 8080. This is done to handle exceptional scenarios.  *
\*---------------------------------------------------------------------------------------*/

int splitPortAndIP (char * argument, char * ipaddr, char * port)
{
       int i=0, j=0;
       if ( strchr(argument, ':') != NULL )
       {
            for ( i=0; argument[i] != ':'; i++ )
            {
                 ipaddr[i] = argument[i];
            }
            for ( i++ ; i < strlen(argument); i++, j++ )
            {
                 port[j] = argument[i];
            }
       }
       else
       {
            for ( i = 0 ; i < strlen(argument); i++ )
            {
                 port[i] = argument[i];
            }
            if (strlen(ipaddr) == 0)
            {
                 strcpy ( ipaddr, "127.0.0.1" );
            }
       }

       if (atol(port) == 0)
       {
	    strcpy ( port, "8080" );
       }
       printf("IPADDR: %s PORT: %s", ipaddr, port);

       return(SUCCESS);
}

/*---------------------------------------------------------------------------------------*\
 * This function check if the filename specified by input argument exists or not.        *
 * If file exists then program is terminated with exit code 33.                          *
\*---------------------------------------------------------------------------------------*/

int checkIfOpFileExist ( char * fileName )
{
       int status = 0;
       struct stat fst;
       status = stat(fileName, &fst);

       switch ( errno )
       {
       case SUCCESS:
                    printf("Target file already exists: Terminate!!!\n");
                    exit(33);
                    break;
       case ENOENT:
                    break;
       default:
                    printf("Error occured while doing stat on file\nerrno %d", errno);
                    return(FAILURE);
       }

       return(SUCCESS);
}
