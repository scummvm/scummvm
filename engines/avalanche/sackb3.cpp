#include "ptoc.h"

/*$M 16384,0,0*/

              /* to demonstrate the SBVoice Unit */
              /* Copyright 1991 Amit K. Mathur, Windsor, Ontario */

/*#include "SBVoice.h"*/


int main(int argc, const char* argv[])
{
pio_initialize(argc, argv);
if (paramcount>0)  {
    loadvoice(paramstr(1),0,0);
    sb_output(seg(soundfile),ofs(soundfile)+26);
    do {
         output << "Demo of the SBVoice Unit, Copyright 1991 by Amit K. Mathur --- ";
    } while (!(statusword==0));
} else
    output << "Usage: DEMO [d:\\path\\]filename.voc" << NL;
return EXIT_SUCCESS;
}
