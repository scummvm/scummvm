#include "ptoc.h"


/*$M $800,0,0*/
/*#include "Dos.h"*/

struct infotype {
            matrix<128,255,1,8,byte> chars;
            string data;
};
infotype table;
pointer where;
string comspec;
int main(int argc, const char* argv[])
{pio_initialize(argc, argv);
;
 getintvec(0x1f,where); move(where,table,1280);
 comspec=getenv("comspec");
 table.data=table.data+"Avalot;";
 setintvec(0x1f,&table);
 output << NL;
 output << "The Astounding Avvy DOS Shell...       don't forget to register!" << NL;
 output << NL;
 output << "Use the command EXIT to return to your game." << NL;
 output << NL;
 output << "Remember: You *mustn't* load any TSRs while in this DOS shell." << NL;
 output << NL;
 output << "This includes: GRAPHICS, PRINT, DOSKEY, and pop-up programs (like Sidekick.)" << NL;
 output << NL;
 output << "About to execute \"" << comspec << "\"..." << NL;
 swapvectors;
 exec("c:\\command.com","");
 swapvectors;
 setintvec(0x1f,where);
return EXIT_SUCCESS;
}