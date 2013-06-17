


#include "graph.h"


typedef matrix<'\0','\377',0,15,byte> fonttype;

integer gd,gm;
fonttype f;
file<fonttype> ff;

int main(int argc, const char* argv[])
{pio_initialize(argc, argv);
;
 assign(ff,"v:avalot.fnt"); reset(ff); ff >> f; close(ff);
 gd=3; gm=0; initgraph(gd,gm,"");
 for( gd=0; gd <= 15; gd ++) mem[0xa000*gd*80]=f['A'][gd];
return EXIT_SUCCESS;
}
