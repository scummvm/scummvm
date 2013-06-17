


#include "graph.h"
/*#include "Crt.h"*/


struct scoretype {
             varying_string<39> name;
             word score;
};

struct ratetype {
            varying_string<10> rank;
            word lowest;
};

struct tabletype {
             array<1,12,scoretype> a;
             byte light;
};

const array<1,9,ratetype> ratings = 
 {{{"Rubbish",    0},
  {"Beginner",   1},
  {"Poor",       10},
  {"Average",    30},
  {"Fair",       50},
  {"Not bad",    70},
  {"Good",       100},
  {"Great",      200},
  {"Fantastic!", 330}}};

integer gd,gm;
tabletype table;

byte ratingname(word x)
{
    byte fv;
byte ratingname_result;
;
 for( fv=9; fv >= 1; fv --)
  if (x>=ratings[fv].lowest) 
  {;
   ratingname_result=fv;
   return ratingname_result;
  }    /* bad style */
return ratingname_result;
}

void title();

const string message = "A v a l o t  :  H i g h - S c o r e s";


static void sayfast(integer x,integer y)
{
 integer anchor; byte fv;
;
 anchor=-296;
 for( fv=1; fv <= length(message); fv ++)
 {;
  if (message[fv-1]!='\40')  outtextxy(x+anchor,y-8,message[fv-1]);
  anchor += 16;
 }
}

void title()
{
 const array<0,6,byte> shades = 
  {{blue,lightgray,darkgray,blue,lightblue,blue,darkgray}};
 byte x;
 integer len;

;
 settextstyle(0,0,2); /*settextjustify(1,1);*/
 len=textheight(message);
 for( x=6; x >= 0; x --)
 {;
  setcolor(shades[x]);
  sayfast(320-x*2,20-x);
  if (x>0) 
  {;
   sayfast(320+x*2,20-x);
   sayfast(320+x*2,20+x);
   sayfast(320-x*2,20+x);
  }
 }
}

void newtable()
{
 const array<1,12,varying_string<15> > names = 
  {{"Mike","Liz","Thomas","Mark","Mandy","Andrew","Lucy Tryphena","",
   "Thanks to all","who helped...","","Have fun!"}};
    byte fv;
;
 fillchar(table,sizeof(table),'\261');
 for( fv=1; fv <= 12; fv ++)
  {
  scoretype& with = table.a[fv]; 
  ;
   with.name=names[fv];
   with.score=193-fv*16;
  }
 table.light=1;
}

string strf(longint x)           /* From Gyro. Delete when integrated. */
{
    string q;
string strf_result;
;
 str(x,q); strf_result=q;
return strf_result;
}

void sparkle(integer x,integer y, string z)
{;
 setcolor(cyan);  outtextxy(x-1,y-1,z);
 setcolor(blue);  outtextxy(x+1,y+1,z);
 setcolor(white); outtextxy(x  ,y  ,z);
}

void drawtable()
{
    byte fv,last,now;
;
 setfillstyle(1,8);
 bar(  0, 40,105, 58); bar(110, 40,400, 58);
 bar(405, 40,490, 58); bar(495, 40,640, 58);
 bar(  5, 60,105,181); bar(110, 60,400,181);
 bar(405, 60,490,181); bar(495, 60,635,181);
 bar(  0,185,640,190);
 setcolor(lightred); settextstyle(0,0,1); settextjustify(0,0);
 outtextxy( 45,55,"Number:");
 outtextxy(120,55,"Name:");
 outtextxy(420,55,"Score:");
 outtextxy(500,55,"Rating:");
 setcolor(white); last=177;
 for( fv=1; fv <= 12; fv ++)
  {
  scoretype& with = table.a[fv]; 
  ;
   settextjustify(righttext,bottomtext);
   sparkle(100,60+fv*10,strf(fv)+'.');
   sparkle(455,60+fv*10,strf(with.score));
   if (fv==table.light)  sparkle(70,60+fv*10,string("ออ")+'\20');

   settextjustify(lefttext,bottomtext);
   sparkle(120,60+fv*10,with.name);
   now=ratingname(with.score);
   if (now!=last) 
    sparkle(510,60+fv*10,ratings[ratingname(with.score)].rank);
   else
    sparkle(517,57+fv*10,",,");
   last=now;
  }
}

void message1(string x)
{;
 setfillstyle(1,8); bar(0,190,640,200);
 settextjustify(1,1); sparkle(320,195,x);
}

void sorthst()
{
    byte fv; boolean ok; scoretype temp;
;
 do {
  ok=true;
  for( fv=1; fv <= 11; fv ++)
    if (table.a[fv].score<table.a[fv+1].score) 
    {;
     temp=table.a[fv]; table.a[fv]=table.a[fv+1]; table.a[fv+1]=temp; /* swap 'em */
     table.light=fv; /* must be- that's the only unsorted one */
     ok=false;  /* So, we try again. */
    }
 } while (!ok);
}

void entername();
static integer x,y; 


static void cursor(byte col)
{;
 setcolor(col);
 outtextxy(x,y,"?");
}

void entername()
{
    varying_string<34> i;char r; integer counter; byte flash;

;
 y=60+table.light*10; i=""; settextjustify(2,0); counter=999; flash=0;
 do {
  x=128+length(i)*8;
  do {
   counter += 1;
   if (counter==1000) 
   {;
    cursor(4+flash*10);
    flash=1-flash;
    counter=0;
   }
   delay(1);
  } while (!keypressed());
  cursor(8);
  sound(17177); r=readkey(); nosound;
  if (r=='\10')  {;
   if (i[0]>'\0')  {;
    bar(x-17,y-10,x-8,y);
    i[0] -= 1; sparkle(x-16,y,i[length(i)]);
   }
  } else {;
   if ((i[0]<'\42') && (r!='\15'))  {;
    sparkle(x,y,r);
    i=i+r;
   }
  }
  counter=999;
 } while (!(r=='\15'));
}

void newscore(word sc)
{;
 { scoretype& with = table.a[12]; 
  if (sc>with.score) 
  {;
   with.name=""; with.score=sc; table.light=10; sorthst(); drawtable();
   message1("Well done! Please enter your name, then press Return...");
   entername();
  } else drawtable();} /* too low for score */
 message1("Press Space to continue...");
 do {; } while (!(keypressed() && (readkey()=='\40')));
}

int main(int argc, const char* argv[])
{pio_initialize(argc, argv);
;
 gd=3; gm=0; initgraph(gd,gm,"");
 title();
 newtable();
 newscore(/*177*/0);
return EXIT_SUCCESS;
}