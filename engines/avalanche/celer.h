#ifndef __celer_h__
#define __celer_h__


#include "closing.h"
#include "incline.h"
#include "gyro.h"


enum flavourtype {ch_ega,ch_bgi, last_flavourtype};

struct chunkblocktype {
                  flavourtype flavour;
                  integer x,y;
                  integer xl,yl;
                  longint size;
                  boolean natural;

                  boolean memorise; /* Hold it in memory? */
};

struct memotype {
            integer x,y;
            integer xl,yl;
            flavourtype flavour;
            word size;
};


#ifdef __celer_implementation__
#undef EXTERN
#define EXTERN
#endif

EXTERN array<1,40,longint> offsets;
EXTERN byte num_chunks;
EXTERN array<1,40,memotype> memos;
EXTERN array<1,40,pointer> memory;
#undef EXTERN
#define EXTERN extern


   void pics_link();

   void load_chunks(string xx);

   void forget_chunks();

   void show_one(byte which);

   void show_one_at(byte which, integer xxx,integer yyy);

#endif
