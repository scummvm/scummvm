/* defines.h */

#ifndef __DEFINES_H__
#define __DEFINES_H__

typedef unsigned char u8;
typedef signed char s8;
typedef unsigned short u16;
typedef short s16;
typedef unsigned int u32;
typedef int s32;

//#define	LITTLE_ENDIAN
/*
typedef union
  {
#ifdef LITTLE_ENDIAN
    struct
      {
        u8 low;
        u8 high;
      } byte;
#else
    struct
      {
        u8 high;
        u8 low;
      } byte;
#endif
    u16 word;
  } word;


typedef union
  {
#ifdef	LITTLE_ENDIAN
    struct
      {
        word low;
        word high;
      } word;
#else
    struct
      {
        word high;
        word low;
      } word;
#endif
    u32	dword;
  } dword;





#ifndef BOOL
#define	BOOL	int
#endif
#ifndef TRUE
#define	TRUE	1
#endif
#ifndef FALSE
#define	FALSE	0
#endif

#ifndef	NULL
#define	NULL	0
#endif
*/
#endif

