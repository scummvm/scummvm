/*****************************************************************
*
*  Copyright (c) 1994 by Boffo Games, All Rights Reserved
*
*
*  errors.h -
*
*  HISTORY
*
*       1.00        05/16/94    BCW     Created this file
*
*  MODULE DESCRIPTION:
*
*       Main header for Error Handler Codes
*
*  RELEVANT DOCUMENTATION:
*
*
*
****************************************************************/
#ifndef ERRORS_H
#define ERRORS_H

//
// error reporting codes
//
typedef unsigned char ERROR_CODE;
#define ERR_NONE     0              // no error
#define ERR_MEMORY   1              // not enough memory
#define ERR_FOPEN    2              // error opening a file
#define ERR_FCLOSE   3              // error closing a file
#define ERR_FREAD    4              // error reading a file
#define ERR_FWRITE   5              // error writing a file
#define ERR_FSEEK    6              // error seeking a file
#define ERR_FDEL     7              // error deleting a file
#define ERR_FFIND    8              // could not find file
#define ERR_FTYPE    9              // invalid file type
#define ERR_PATH    10              // invalid path or filename
#define ERR_DISK    11              // unrecoverable disk error
#define ERR_UNKNOWN 12              // unknown error

#define ERR_FUTURE1 13              // future use
#define ERR_FUTURE2 14              //
#define ERR_FUTURE3 15              //
#define ERR_FUTURE4 16              //
#define ERR_FUTURE5 17              // future use

#endif  // ERRORS_H
