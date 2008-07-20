/*
** $Id$
** a generic input stream interface
** See Copyright Notice in lua.h
*/



#include <stdio.h>
#include <string.h>

#include "lzio.h"



/* ----------------------------------------------------- memory buffers --- */

static int32 zmfilbuf (ZIO* /*z*/)
{
 return EOZ;
}

ZIO* zmopen (ZIO* z, const char* b, int32 size, const char *name)
{
 if (b==NULL) return NULL;
 z->n=size;
 z->p= (const byte *)b;
 z->filbuf=zmfilbuf;
 z->u=NULL;
 z->name=name;
 return z;
}

/* ------------------------------------------------------------ strings --- */

ZIO* zsopen (ZIO* z, const char* s, const char *name)
{
 if (s==NULL) return NULL;
 return zmopen(z,s,strlen(s),name);
}

/* -------------------------------------------------------------- FILEs --- */

static int32 zffilbuf (ZIO* z)
{
 int32 n=fread(z->buffer,1,ZBSIZE,(FILE *)z->u);
 if (n==0) return EOZ;
 z->n=n-1;
 z->p=z->buffer;
 return *(z->p++);
}


ZIO* zFopen (ZIO* z, FILE* f, const char *name)
{
 if (f==NULL) return NULL;
 z->n=0;
 z->p=z->buffer;
 z->filbuf=zffilbuf;
 z->u=f;
 z->name=name;
 return z;
}


/* --------------------------------------------------------------- read --- */
int32 zread (ZIO *z, void *b, int n)
{
  while (n) {
    int32 m;
    if (z->n == 0) {
      if (z->filbuf(z) == EOZ)
        return n;  /* retorna quantos faltaram ler */
      zungetc(z);  /* poe o resultado de filbuf no buffer */
    }
    m = (n <= z->n) ? n : z->n;  /* minimo de n e z->n */
    memcpy(b, z->p, m);
    z->n -= m;
    z->p += m;
    b = (char *)b + m;
    n -= m;
  }
  return 0;
}
