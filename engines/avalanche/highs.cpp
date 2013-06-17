/*
 * This code is based on the original source code of Lord Avalot d'Argent version 1.3.
 * Copyright (c) 1994-1995 Mike, Mark and Thomas Thurman.
 */



/*
  €ﬂ‹ €ﬂ‹ ‹ﬂﬂ‹  ﬂ€ﬂ €ﬂﬂ  ‹ﬂ ﬂ€ﬂ      ‹ﬂ€ﬂ‹  ﬂ€ﬂ €‹  € ‹€ﬂﬂ  ﬂ€ﬂ €ﬂ‹ €ﬂﬂ €
  €ﬂ  €€  €  € ‹ €  €ﬂﬂ ﬂ‹   €      €  €  €  €  € €‹€  ﬂﬂﬂ‹  €  €€  €ﬂﬂ €
  ﬂ   ﬂ ﬂ  ﬂﬂ   ﬂﬂ  ﬂﬂﬂ   ﬂ  ﬂ      ﬂ  ﬂ  ﬂ ﬂﬂﬂ ﬂ  ﬂﬂ  ﬂﬂﬂ   ﬂ  ﬂ ﬂ ﬂﬂﬂ ﬂﬂﬂ

                 HIGHS            This handles the high-scores. */

#define __highs_implementation__


#include "highs.h"


/*#include "Gyro.h"*/
/*#include "Scrolls.h"*/


typedef array<1,12,struct A1 {
                                 varying_string<30> name;
                                 word score;
                                 varying_string<12> rank;
                   }> highscoretype;

highscoretype h;

void get_new_highs()
{
    byte fv;
;
 for( fv=1; fv <= 12; fv ++)
  {
  A1& with = h[fv]; 
  ;
   with.score=32-fv*2;
   with.rank="...";
  }
 h[1].name="Mike"; h[2].name="Liz";  h[3].name="Thomas"; h[4].name="Mark";
 h[5].name="Mandy"; h[6].name="Andrew";  h[7].name="Lucy Tryphena";
 h[8].name="Tammy the dog";
 h[9].name="Avaricius"; h[10].name="Spellchick";  h[11].name="Caddelli";
 h[12].name="Spludwick";
}

void show_highs()
 /* This procedure shows the high-scores. */
{
 byte fv;
 varying_string<40> x;
 varying_string<5> y;
;
 display("HIGH SCORERS\3\r  Name\n\nScore   Rank\r  \"\"\"\"\n\n\"\"\"\"\"   \"\"\"\"\f\4");
 for( fv=1; fv <= 12; fv ++)
  {
  A1& with = h[fv]; 
  ;
   display(string('\15')+with.name+'\4');
   fillchar(x,sizeof(x),'\40');
   y=strf(with.score);
   x[0]=chr(29-(length(with.name+y)));
   display(x+y+' '+with.rank+'\4');
  }

 display("");
}

void store_high(string who)
 /* This procedure shows the high-scores. */
{
 byte fv,ff;
;

 for( fv=1; fv <= 12; fv ++)
  if (h[fv].score<dna.score)  flush();

 /* Shift all the lower scores down a space. */
 for( ff=fv; ff <= 11; ff ++)
  h[ff+1]=h[ff];

 {
 A1& with = h[fv]; 
 ;
  with.name=who;
  with.score=dna.score;
 }

}

void get_highs()
{
    file<highscoretype> f;
;
 /*$I-*/
 assign(f,"scores.avd");
 reset(f);
  /* Did we get it? */

 if (ioresult!=0) 
 {;     /* No. */
  get_new_highs(); /* Invent one. */
 } else
 {;     /* Yes. */
  f >> h;
  close(f);
 }
}

class unit_highs_initialize {
  public: unit_highs_initialize();
};
static unit_highs_initialize highs_constructor;

unit_highs_initialize::unit_highs_initialize() {;
 get_highs();
}