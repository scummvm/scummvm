


#include "graph.h"

integer gd,gm;

void data(integer y, string a,string b)
{;
 setcolor(1); settextjustify(2,0); outtextxy(315,y,a+':');
 setcolor(0); settextjustify(0,0); outtextxy(325,y,b);
}

int main(int argc, const char* argv[])
{pio_initialize(argc, argv);
;
 gd=3; gm=0; initgraph(gd,gm,"c:\\turbo");
 setcolor(1); setfillstyle(1,7); bar(0,0,640,200);
 settextstyle(1,0,0); setusercharsize(10,4,8,6); settextjustify(1,1);
 outtextxy(317,20,"Status Screen");
 settextstyle(0,0,1);
 data( 60,"Current game","NoName.ASG");
 data( 70,"Saved?","Yes");
 data( 90,"Current town","Thurringham");
 data(100,"Score","177");
 data(110,"Cash","1/-");
 data(130,"Game","Lord Avalot D'Argent");
 data(140,"Version","1.00 (beta)");
 data(150,"Registered to","Unregistered!");
 data(160,"Number","nix");
 data(177,"Bios/keyboard","AT (enhanced)"); /* or XT (bog standard) */
 data(187,"Display","VGA");
return EXIT_SUCCESS;
}