#ifndef __PUBLIC_H__
#define __PUBLIC_H__

#include <stdio.h>
#include <gcrypt.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>

#define PASSWD_LENGTH 30
#define IN_BUF_SIZE    4096
#define OUT_BUF_SIZE   4096

/* Always keep read Buffer size more than 64 */
#define READ_BUF_SIZE 256
#define SUCCESS 0
#define FAILURE -1

enum en_dc_modes {ENCRYPT=1, DECRYPT};
enum prog_modes  {LOCAL=1, TRANSFER};

#define CHECK_IF_ERR(err) \
       if ( err != 0 )\
       {\
           fprintf ( stderr, "Failure: %s/%s\n",\
                     gcry_strsource ( err ),\
                     gcry_strerror ( err ) );\
           fprintf ( stdout, "Failure: %s/%s\n",\
                     gcry_strsource ( err ),\
                     gcry_strerror ( err ) );\
       }

#define GPG_CHECK_ERR_AND_RETURN(err) \
       if ( err != 0 )\
       {\
           fprintf ( stderr, "Failure: %s/%s %s %d\n",\
                     gcry_strsource ( err ),\
                     gcry_strerror ( err ) , __FILE__, __LINE__);\
           fprintf ( stdout, "Failure: %s/%s %s %d\n",\
                     gcry_strsource ( err ),\
                     gcry_strerror ( err ) , __FILE__, __LINE__);\
           return(FAILURE);\
       }

#define STD_CHECK_ERR_REPORT_AND_RETURN(status) \
       if ( status != SUCCESS )\
       {\
           fprintf ( stderr, "Failure in function: %s on line %d\n",\
                     __FILE__,\
                     __LINE__ );\
           fprintf ( stdout, "Failure in function: %s on line %d\n",\
                     __FILE__,\
                     __LINE__ );\
           return(status);\
       }

#endif
