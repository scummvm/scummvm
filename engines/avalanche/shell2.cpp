#include "ptoc.h"


/*#include "Dos.h"*/
#include "graph.h"

struct infotype {
            matrix<128,255,1,8,byte> chars;
            string data;
};
infotype table;
pointer where;
integer gd,gm;
int main(int argc, const char* argv[])
{pio_initialize(argc, argv);
;
 getintvec(0x1f,where); move(where,table,1280);
 gd=1; gm=0; initgraph(gd,gm,"c:\\turbo");
 output << "Now in CGA low-res 320x200." << NL;
 output << "High ASCII codes work- œœœ °±² ðððññóòôõ" << NL;
 output << "And the code is..." << NL;
 output << table.data << NL;
 output << "Press Enter..." << NL;
 input >> NL; closegraph();
return EXIT_SUCCESS;
}