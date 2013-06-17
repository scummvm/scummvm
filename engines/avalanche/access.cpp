

/*
  ÛßÜ ÛßÜ ÜßßÜ  ßÛß Ûßß  Üß ßÛß      ÜßÛßÜ  ßÛß ÛÜ  Û ÜÛßß  ßÛß ÛßÜ Ûßß Û
  Ûß  ÛÛ  Û  Û Ü Û  Ûßß ßÜ   Û      Û  Û  Û  Û  Û ÛÜÛ  ßßßÜ  Û  ÛÛ  Ûßß Û
  ß   ß ß  ßß   ßß  ßßß   ß  ß      ß  ß  ß ßßß ß  ßß  ßßß   ß  ß ß ßßß ßßß

                 ACCESS           The temporary Sez handler. */

#define __access_implementation__


#include "access.h"


/*#include "Gyro.h"*/
/*#include "Scrolls.h"*/
/*#include "Acci.h"*/
/*#include "Trip5.h"*/
/*#include "Lucerna.h"*/


boolean int_say_went_ok;

void int_say(string filename, boolean bubble)
 /* Internal use ONLY! */
{
    untyped_file f;
;
 /*$I-*/
 assign(f,filename);
 reset(f,1);
 if (ioresult!=0) 
 {;
  int_say_went_ok=false;
  return;
 }
 bufsize=filesize(f);
 blockread(f,buffer,bufsize);
 if (bubble) 
 {;
  bufsize += 1;
  buffer[bufsize]='\2';
 }
 close(f);
 /*$I+*/

 calldrivers;

 int_say_went_ok=true;
}

void dixi(char p, byte n)
{; exit(153);
 int_say(string('s')+p+strf(n)+".raw",false);
}

void talkto(byte whom)
{
 byte fv;
 boolean no_matches;
; exit(153);
 if (person==pardon) 
 {;
  person=chr(subjnumber);
  subjnumber=0;
 }

 switch (chr(whom)) {
  case pspludwick:

    if ((dna.lustie_is_asleep) & (~ dna.obj[potion])) 
    {;
     dixi('q',68);
     dna.obj[potion]=true;
     objectlist; points(3); return;
    } else
    {;
     if (dna.talked_to_crapulus) 
      switch (dna.given2spludwick) { /* Spludwick - what does he need? */
         /* 0 - let it through to use normal routine. */
         case RANGE_2(1,2): {;
                display(string("Can you get me ")+
                 get_better(spludwick_order[dna.given2spludwick])+", please?"+
                  "\232\2");
                return;
               }
               break;
         case 3: {;
             dixi('q',30); /* need any help with the game? */
             return;
            }
            break;
       }
     else dixi('q',42); /* Haven't talked to Crapulus. Go and talk to him. */
    }
    break;

  case pibythneth: if (dna.givenbadgetoiby) 
              {;
               dixi('q',33); /* Thanks a lot! */
               return; /* And leave the proc. */
              }
              break; /* Or... just continue, 'cos he hasn't got it. */
  case pdogfood: if (dna.wonnim) 
            {;     /* We've won the game. */
             dixi('q',6); /* "I'm Not Playing!" */
             return; /* Zap back. */
            }
            break;
  case payles: if (~ dna.ayles_is_awake) 
          {;
           dixi('q',43); /* He's fast asleep! */
           return;
          }
          break;
  case pgeida: if (dna.geida_given_potion) 
           dna.geida_follows=true; else
          {;
           dixi('u',17);
           return;
          }
          break;
 }

 if (whom>149)  whom -= 149;

 no_matches=true;
 for( fv=1; fv <= numtr; fv ++)
  if (tr[fv].a.accinum==whom) 
  {;
   display(string('\23')+chr(fv+48)+'\4');
   no_matches=false;
   flush();
  }

 if (no_matches)  display("\23\23\4");

 if (subjnumber==0)   /* For the moment... later we'll parse "say". */
  int_say(string("ss")+strf(whom)+".raw",true);
 else
 {;
  int_say(string("ss")+strf(whom)+'-'+strf(subjnumber)+".raw",true);
  if (! int_say_went_ok)      /* File not found! */
   dixi('n',whom);
 }

 switch (chr(whom+149)) {
  case pcrapulus:
     {;     /* Crapulus: get the badge - first time only */
      dna.obj[badge]=true;
      objectlist;
      dixi('q',1); /* Circular from Cardiff. */
      dna.talked_to_crapulus=true;

      whereis[pcrapulus]=177;  /* Crapulus walks off. */

      tr[2].vanishifstill=true;
      tr[2].walkto(4); /* Walks away. */

      points(2);
     }
     break;

  case payles: dixi('q',44); break; /* Can you get me a pen? */

 }
}


