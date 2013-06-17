/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

/*
 * This code is based on the original source code of Lord Avalot d'Argent version 1.3.
 * Copyright (c) 1994-1995 Mike, Mark and Thomas Thurman.
 */

/*
  €ﬂ‹ €ﬂ‹ ‹ﬂﬂ‹  ﬂ€ﬂ €ﬂﬂ  ‹ﬂ ﬂ€ﬂ      ‹ﬂ€ﬂ‹  ﬂ€ﬂ €‹  € ‹€ﬂﬂ  ﬂ€ﬂ €ﬂ‹ €ﬂﬂ €
  €ﬂ  €€  €  € ‹ €  €ﬂﬂ ﬂ‹   €      €  €  €  €  € €‹€  ﬂﬂﬂ‹  €  €€  €ﬂﬂ €
  ﬂ   ﬂ ﬂ  ﬂﬂ   ﬂﬂ  ﬂﬂﬂ   ﬂ  ﬂ      ﬂ  ﬂ  ﬂ ﬂﬂﬂ ﬂ  ﬂﬂ  ﬂﬂﬂ   ﬂ  ﬂ ﬂ ﬂﬂﬂ ﬂﬂﬂ

                 ENID             Edna's manager. */

#define __enid_implementation__
           /* Loads/ saves files. */

/*$V-*/

#include "enid.h"


/*#include "Dos.h"*/
#include "scrolls.h"
#include "lucerna.h"
#include "trip5.h"
#include "timeout.h"
#include "Celer.h"
#include "sequence.h"
#include "fileunit.h"
#include "basher.h"


const string crlf = string('\15')+'\12';
const char tab = '\11';
const char eof_ = '\32';

const array<1,177,char> ednafirst = 
 string("This is an EDNA-based file, saved by a Thorsoft game. Good luck!")+ /*64*/
 crlf+eof_+crlf+crlf+ /*7*/
 tab+"Glory to God in the highest,"+crlf+ /*31*/
 tab+"and on earth peace, goodwill toward men."+ /*42*/
 crlf+tab+tab+tab+tab+ /*6*/
 "Luke 2:14."+ /*10*/
 crlf+crlf+crlf+ /* 6 */
 "1234567890"+crlf; /*11*/

const string ednaid = string("TT")+'\261'+'\60'+'\1'+'\165'+'\261'+'\231'+'\261';

const integer ttage = 18;
const array<1,16,char> ttwashere = "Thomas was here ";

boolean bug;

string expanddate(byte d,byte m, word y);

const array<1,12,varying_string<7> > months = 
{{"Jan#","Febr#","March","April","May","June","July","August",
"Septem*","Octo*","Novem*","Decem*"}};


static varying_string<10> month;


static void addon(string x) { month[0] -= 1; month=month+x; }

string expanddate(byte d,byte m, word y)
{
 varying_string<4> day;


 string expanddate_result;
 month=months[m];
 switch (month[length(month)]) {
  case '#': addon("uary"); break;
  case '*': addon("ber"); break;
 }

 day=strf(d);

 if (set::of(range(1,9),range(21,31), eos).has(d)) 
  switch (d % 10) {
   case 1: day=day+"st"; break;
   case 2: day=day+"nd"; break;
   case 3: day=day+"rd"; break;
   default: day=day+"th";
  }

 expanddate_result=day+' '+month+' '+strf(y);
 return expanddate_result;
}

void edna_save(string name);


static void show_bug(char icon, string strn)
{ display(string("\7\6\23")+icon+"\26\r"+strn+'\15'); }



static boolean test_bug(byte what)
{
 boolean test_bug_result;
 if (what==0)  { test_bug_result=false; return test_bug_result; }
 switch (what) {
  case 2: show_bug('7',"Error in filename!"); break;
  case 101: show_bug('6',"Disk full!"); break;
  case 150: show_bug('4',"Disk is write-protected!"); break;
  default: show_bug('B',"Saving error!");
 }
 test_bug_result=true;
 return test_bug_result;
}

void edna_save(string name)
{
 untyped_file f;
 ednahead eh;
 word groi;
 string groi2,path;
 word tempd,tempm;

 if (name=="") 
 {         /* We were given no name. Do we have a default? */
  if (enid_filename=="") 
  {        /* No */
   filename_edit(); /* Request one. */
   return;
  } else   /* Yes */
   name=enid_filename;
 }

 wait(); /* Put up hourglass pointer */

 fillchar(eh,sizeof(eh),'\261'); /* Fill up the edhead */

 dna.saves += 1; /* It's been saved one more time... */

 {

  /* Info about this program */

  eh.id=ednaid;  /* Edna's signature */
  eh.revision=thisgamecode; /* 2- second revision of .ASG format */
  eh.game="Lord Avalot d'Argent"; /* Title of game */
  eh.shortname="Avalot";
  eh.number=2; /* Second Avvy game */
  eh.ver=thisvercode; /* Version 1.00 */
  eh.verstr=vernum; /* ditto */
  eh.filename="AVALOT.EXE"; /* program's filename */
  eh.osbyte=1; /* Saved under DOS */
  eh.os="DOS";

  /* Info on this particular game */

  fsplit(name,path,eh.fn,groi2); /* fn = filename of this game */
  getdate(eh.y,tempm,tempd,groi); /* Day, month & year when the game was saved */
  eh.d=tempd; eh.m=tempm;
  eh.desc=roomname; /* Description of game (same as in Avaricius!) */
  eh.len=sizeof(dna); /* Length of DNA. */

  /* Quick reference & miscellaneous */

  eh.saves=dna.saves; /* no. of times this game has been saved */
  eh.cash=dna.pence; /* contents of your wallet in numerical form */
  eh.money=lsd(); /* ditto in string form (eg 5/-, or 1 denarius)*/
  eh.points=dna.score; /* your score */

  name=path+eh.fn+".ASG";
 }

 assign(f,name);
 /*$I-*/
 rewrite(f,1);
 if (test_bug(ioresult))  return;

 blockwrite(f,ednafirst,177); if (test_bug(ioresult))  return;
 blockwrite(f,eh,sizeof(eh)); if (test_bug(ioresult))  return;
 blockwrite(f,dna,sizeof(dna)); if (test_bug(ioresult))  return;

 for( groi=1; groi <= numtr; groi ++)
  { triptype& with = tr[groi]; 
   if (with.quick) 
   {
    blockwrite(f,groi,1); if (test_bug(ioresult))  return;
    savedata(f); if (test_bug(ioresult))  return;
   }}

 groi=177; blockwrite(f,groi,1);

 blockwrite(f,times,sizeof(times)); /* Timeout.times: Timers. */

  if (test_bug(ioresult))  return;

 blockwrite(f,seq,sizeof(seq)); /* Sequencer information. */

  if (test_bug(ioresult))  return;

 for( groi=1; groi <= ttage; groi ++)
  blockwrite(f,ttwashere[1-1],16);

  if (test_bug(ioresult))  return;

 close(f);
  if (test_bug(ioresult))  return;
 /*$I+*/

 display(string('\6')+"Saved: "+'\22'+name+'.');
 enid_filename=name;
}

void loaderror(string x, char icon)
{
 if (holdthedawn) 
 {
  holdthedawn=false;
  dawn();
 }
 display(string('\7')+'\6'+'\23'+icon+'\26'+"Loading error:  "+"\r\r\22"+x);
 bug=true;
}



typedef array<1,4,char> fourtype;



const fourtype avaricius_file = "Avvy";

void edna_load(string name)

{
 untyped_file f;
 ednahead eh;
 byte fv;
 byte io;
 string path,fn,groi;
 fourtype id4;

 word len2load;



 if (name=="") 
 {                       /* No filename specified, so let's call the filer. */
  name=do_filer();
  if (name=="")  return;  /* STILL no filename, so they must have cancelled. */
 }

 bug=false;

 wait(); /* Put up hourglass pointer */

 fsplit(name,path,fn,groi);
 name=path+fn+".ASG";

 /* Load the file into memory */

 /*$I-*/
 assign(f,name);
 reset(f,1);

 io=ioresult;
 if (io!=0) 
  switch (io) {
   case 2: loaderror("File not found!",'8'); break;
   case 3: loaderror("Directory not found!",'3'); break;
   default: loaderror(string("Error no.")+strf(io),'1');
  }

 if (bug)  return;

 seek(f,11); blockread(f,id4,4);
 if (id4==avaricius_file) 
 {
  loaderror("That's an Avaricius file!",'1');
  close(f);
  return;
 }

 seek(f,177); /* bypass ednafirst */

 blockread(f,eh,sizeof(eh)); /* load ednahead */

 /* Check ednahead for errors */

 {
  if ((eh.id!=ednaid) || (eh.revision!=2))  loaderror("Not an EDNA file!",'7'); else
  if (eh.number!=2)  loaderror(string("That file was saved by ")+eh.shortname+'!','1');
 }

 if (bug) 
 {
  close(f);
  return;
 }

 len2load=eh.len;

 if (eh.len!=(cardinal)sizeof(dna)) 
 {
  if (holdthedawn) 
  {
   holdthedawn=false;
   dawn();
  }
  display("\233\26Warning: \3\rEDNA size doesn't match.\f");
  if (eh.len>(cardinal)sizeof(dna)) 
   len2load=sizeof(dna); /* BIGGER than ours */
  else fillchar(dna,sizeof(dna),'\0'); /* Otherwise, smaller. */
 }

 blockread(f,dna,len2load);

 for( fv=1; fv <= numtr; fv ++)
  { triptype& with = tr[fv]; 
  if (with.quick)  done();} /* Deallocate sprite */

 do {
  blockread(f,fv,1);
  if (fv!=177)  tr[fv].loaddata(f);
 } while (!(fv==177));

 blockread(f,times,sizeof(times)); /* Timeout.times: Timers. */

 blockread(f,seq,sizeof(seq)); /* Sequencer information. */

 close(f);

 seescroll=true;  /* This prevents display of the new sprites before the
  new picture is loaded. */

 if (holdthedawn) 
 {
  holdthedawn=false;
  dawn();
 }

  display(string('\6')+"Loaded: "+'\22'+name+'\3'+"\r\r"+eh.desc+"\r\r"+"saved on "+
   expanddate(eh.d,eh.m,eh.y)+'.');

 forget_chunks();

 minor_redraw();

 whereis[pavalot]=dna.room;
/* showscore;*/
 alive=true;

 objectlist();
/*$I+*/

 enid_filename=name;
}

void dir(string where);

static string path,groi;


static void showheader()
{
 display(string("Dir: ")+path+"\r\r\4");
}

void dir(string where)
 /* OK, it worked in Avaricius, let's do it in Avalot! */
{
 searchrec s;
 byte count;

 if ((where!="") && (!(set::of('\\',':', eos).has(where[length(where)])))) 
  where=where+'\\';
 fsplit(where,path,groi,groi);
 path=path+"*.asg";
 count=0;

 findfirst(path,anyfile,s);
 showheader();

 while (doserror==0) 
 {
  count += 1;
  if (count==11) 
  {
   display("\r\nPress Enter...");
   showheader();
   count=1;
  }

  display(s.name+"\r\4");

  findnext(s);
 }

 if (count==0) 
  display("No files found!");
 else display("\nThat's all!");

}

void avvy_background()     /* Not really a filing procedure,
 but it's only called just before edna_load, so I thought I'd put it
 in Enid instead of, say, Lucerna. */
{
/* port[$3c4]:=2; port[$3ce]:=4; port[$3C5]:=1; port[$3CF]:=1; { Blue. }*/
/*
 asm
  mov dx,$3c4; mov al,2; out dx,al; { Set up the VGA to use the "blue" }
  mov dx,$3ce; mov al,4; out dx,al; { register. }
  mov dx,$3c5; mov al,1; out dx,al;
  mov dx,$3cf;           out dx,al;

  mov bx,$A000; call far ptr @drawup;
  mov bx,$A400; call far ptr @drawup;

  jmp @the_end;

 @drawup:

  mov es,bx; { The segment to copy it to... }
  mov di,$370;  { The offset (10 pixels down, plus 1 offset.) }

  mov cx,10;
  mov ax,$AA4A; call far ptr @sameline; { Print "AVVY" }
  mov ax,$AEAA; call far ptr @sameline;
  mov ax,$A4EA; call far ptr @sameline;
  mov ax,$44A4; call far ptr @sameline;

  mov cx,9;
  mov ax,$AAA4; call far ptr @sameline; { Print "YAVV" }
  mov ax,$AAEA; call far ptr @sameline;
  mov ax,$AA4E; call far ptr @sameline;
  mov ax,$444A; call far ptr @sameline;

  mov ax,$4AAA; call far ptr @sameline; { Print "VYAV" }
  mov ax,$AAAE; call far ptr @sameline;
  mov ax,$EAA4; call far ptr @sameline;
  mov ax,$A444; call far ptr @sameline;

  mov ax,$A4AA; call far ptr @sameline; { Print "VVYA" }
  mov ax,$EAAA; call far ptr @sameline;
  mov ax,$4EAA; call far ptr @sameline;
  mov ax,$4A44; call far ptr @sameline;

  ret;


  { Replicate the same line many times. }

  @sameline:
   { Requires:
      what to copy in AX,
      how many lines in CX, and
      original offset in DI. }
   push cx;
   push di;

   @samelineloop:

    push cx;
    mov cx,40; { No. of times to repeat it on one line. }

    repz stosw; { Fast word-copying }

    pop cx;

    add di,1200; { The next one will be 16 lines down. }

   loop @samelineloop;
   pop di;
   add di,80;
   pop cx;

   ret;

  @the_end:
 end;
*/
blitfix();
}

void to_sundry(sundry& sund)
{
 {
  sund.qenid_filename=enid_filename;
  sund.qsoundfx=soundfx;
  sund.qthinks=thinks;
  sund.qthinkthing=thinkthing;
 }
}

void from_sundry(sundry sund)
{
 {
  enid_filename=sund.qenid_filename;
  soundfx=sund.qsoundfx;
  thinks=sund.qthinks;
  thinkthing=sund.qthinkthing;
 }
}

void restore_dna()
{
 word here,fv;
 sundry sund;

 move(mem[storage_seg*storage_ofs+3],dna,sizeof(dna));
 move(mem[storage_seg*storage_ofs+3+sizeof(dna)],times,sizeof(times));
 move(mem[storage_seg*storage_ofs+3+sizeof(dna)+sizeof(times)],
                                                 seq,sizeof(seq));
 move(mem[storage_seg*storage_ofs+3+sizeof(dna)+sizeof(times)+sizeof(seq)],
                                                 sund,sizeof(sund));
 from_sundry(sund);

 here=storage_ofs+3+sizeof(dna)+sizeof(times)+sizeof(seq)+sizeof(sund);
 do {
  fv=mem[storage_seg*here]; here += 1;
  if (fv!=177)  tr[fv].load_data_from_mem(here);
 } while (!(fv==177));
}

void edna_reload()
{

 restore_dna();

 seescroll=true;  /* This prevents display of the new sprites before the
  new picture is loaded. */

 major_redraw();

 whereis[pavalot]=dna.room;

 alive=true;

 objectlist();

 if (holdthedawn) 
 {
  holdthedawn=false;
  dawn();
 }
}

void back_to_bootstrap(byte what)
{
 byte fv;
 word here;
 sundry sund;

 mem[storage_seg*storage_ofs]=what;  /* Save the operation code. */
 to_sundry(sund); /* Save the sundry information. */

 /* Save the DNA, times and sequencer info: */
 move(dna,mem[storage_seg*storage_ofs+3],sizeof(dna));
 move(times,mem[storage_seg*storage_ofs+3+sizeof(dna)],sizeof(times));
 move(seq,mem[storage_seg*storage_ofs+3+sizeof(dna)+sizeof(times)],
                                                 sizeof(seq));
 move(sund,
   mem[storage_seg*storage_ofs+3+sizeof(dna)+sizeof(times)+sizeof(seq)],
                                                     sizeof(sund));

 here=storage_ofs+3+sizeof(dna)+sizeof(times)+sizeof(seq)+sizeof(sund);

 for( fv=1; fv <= numtr; fv ++)
  { triptype& with = tr[fv]; 
   if (with.quick) 
   {
    mem[storage_seg*here]=fv; here += 1;
    save_data_to_mem(here);
   }}
  mem[storage_seg*here]=177;

 exit(77); /* Code to return to the Bootstrap. */
}

boolean there_was_a_problem()
{
 boolean there_was_a_problem_result;
 there_was_a_problem_result=bug;
 return there_was_a_problem_result;
}


