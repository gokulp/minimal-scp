#ifndef __ENCDECGENFUNC_H__
#define __ENCDECGENFUNC_H__

#include <public.h>

int initializeProgram ();

int genKeyFromPasswd (unsigned const char * passwd, unsigned char * keybuffer, size_t keysize);

int printKey ( unsigned char * keybuffer, size_t length );

int calcHMACforFile(FILE *instream, unsigned char *keybuffer, unsigned char * o_md_string);

int copyFileGetHMAC (FILE * ifp, FILE *ofp, size_t fileSize, unsigned char * hmac );

int openFile ( char * fileName, char * openMode, FILE **fp);

int authenticateInputFileAndGenTemp (char * ipFullFileName, char * tmpFileName, unsigned char * keybuffer );

#endif
