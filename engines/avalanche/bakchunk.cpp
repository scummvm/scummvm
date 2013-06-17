


#include "graph.h"
/*#include "Celer.h"*/
/*#include "Lucerna.h"*/

integer gd,gm;
int main(int argc, const char* argv[])
{
 pio_initialize(argc, argv);
 gd=3; gm=0; initgraph(gd,gm,"");
 setvisualpage(3);
 load_chunks('1');

 for( gd=0; gd <= num_chunks; gd ++)
  show_one_at(gd,0,gd*40);

 mblit(0,0,79,200,3,0);

 gd=getpixel(0,0);
 setvisualpage(0); setactivepage(0);

 settextstyle(0,0,4); setcolor(15);
 outtextxy(100,50,"Chunk1");
 input >> NL;
 return EXIT_SUCCESS;
}