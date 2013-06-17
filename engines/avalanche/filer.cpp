/*
 * This code is based on the original source code of Lord Avalot d'Argent version 1.3.
 * Copyright (c) 1994-1995 Mike, Mark and Thomas Thurman.
 */



#include "graph.h"
/*#include "Dos.h"*/
/*#include "Crt.h"*/
/*#include "Tommys.h"*/
#include "lucerna.h"

/*$V-*/

struct windowtype {
              integer x1,y1,x2,y2;
              varying_string<20> title;
};

 struct ednahead { /* Edna header */
           /* This header starts at byte offset 177 in the .ASG file. */
           array<1,9,char> id;     /* signature */
           word revision; /* EDNA revision, here 2 (1=dna256) */
           varying_string<50> game; /* Long name, eg Lord Avalot D'Argent */
           varying_string<15> shortname; /* Short name, eg Avalot */
           word number; /* Game's code number, here 2 */
           word ver; /* Version number as integer (eg 1.00 = 100) */
           varying_string<5> verstr; /* Vernum as string (eg 1.00 = "1.00" */
           varying_string<12> filename; /* Filename, eg AVALOT.EXE */
           byte osbyte; /* Saving OS (here 1=DOS. See below for others.*/
           varying_string<5> os; /* Saving OS (here 1=DOS. See below for others.*/

           /* Info on this particular game */

           varying_string<8> fn; /* Filename (not extension ('cos that's .ASG)) */
           byte d,m; /* D, M, Y are the Day, Month & Year this game was... */
           word y;  /* ...saved on. */
           varying_string<40> desc; /* Description of game (same as in Avaricius!) */
           word len; /* Length of DNA (it's not going to be above 65535!) */

           /* Quick reference & miscellaneous */

           word saves; /* no. of times this game has been saved */
           integer cash; /* contents of your wallet in numerical form */
           varying_string<20> money; /* ditto in string form (eg 5/-, or 1 denarius)*/
           word points; /* your score */

           /* DNA values follow, then footer (which is ignored) */
 };

const integer border = 1; /* size of border on shadowboxes */

const array<1,4,varying_string<7> > buttons = {{"Okay","Wipe","Cancel","Info..."}};

const integer files = 3;
const integer dirs = 4;

const integer drlen = 15; /* no. of drives on one line */

const array<1,4,windowtype> threewins = 
 {{{155, 12, 630, 22, "Name"},
  {15, 25, 410, 70, "Drives"},
  {15, 83, 480, 179, "Files (*.ASG)"},
  {490, 83, 630, 179, "Subdirectories"}}};

const integer name_win = 1;
const integer drive_win = 2;
const integer file_win = 3;
const integer subdir_win = 4;

matrix<3,4,1,77,varying_string<12> > lists;
array<1,77,varying_string<40> > descs;
array<3,4,byte> nums,where,top;
searchrec s;
boolean loading;
varying_string<26> drives;
pathstr current;
byte nowwin;
varying_string<17> doing;

pathstr filename;
boolean filefound;

void shadow(integer x1,integer y1,integer x2,integer y2, byte hc,byte sc)
{
    byte fv;
;
 for( fv=0; fv <= border; fv ++)
 {;
  setfillstyle(1,hc);
  bar(x1+fv,y1+fv,x1+fv,y2-fv);
  bar(x1+fv,y1+fv,x2-fv,y1+fv);

  setfillstyle(1,sc);
  bar(x2-fv,y1+fv,x2-fv,y2-fv);
  bar(x1+fv,y2-fv,x2-fv,y2-fv);
 }
}

void shbox(integer x1,integer y1,integer x2,integer y2, string t)
{
      const integer fc = 7;
;
 shadow(x1,y1,x2,y2,15,8);
 setfillstyle(1,fc);
 bar(x1+border+1,y1+border+1,x2-border-1,y2-border-1);
 setcolor(1); x1=(x2-x1) / 2+x1; y1=(y2-y1) / 2+y1;
 outtextxy(x1,y1,t);
 if (length(t)>1) 
 {;
  fillchar(t[2],length(t)-1,'\40'); t[1]='_';
  outtextxy(x1-1,y1+1,t);
 }
}

void show_drives()
{
    byte fv;
;
 settextjustify(1,1);
 for( fv=0; fv <= length(drives)-1; fv ++)
  shbox((fv % drlen)*25+25,(fv / drlen)*19+31,
   (fv % drlen)*25+40,45+(fv / drlen)*19,drives[fv+1]);
 setcolor(11);
 settextjustify(0,2);
}

void box(integer x1,integer y1,integer x2,integer y2, string z)
{;
 rectangle(x1,y1,x2,y2);
 outtextxy(x1+1,y1-10,z+':');
 outtextxy(x1,y1-9,"_");
}

string lowstr(string x)
{
    byte fv;
string lowstr_result;
;
 for( fv=1; fv <= length(x); fv ++)
  if (set::of(range('A','Z'), eos).has(x[fv]))  x[fv] += 32;
 lowstr_result=x;
return lowstr_result;
}

char lowchar(char x)
{
 char lowchar_result;
 if (set::of(range('A','Z'), eos).has(x))  x -= 32;
 lowchar_result=x;
 return lowchar_result;
}

void getcurrent()
{;
 current=lowstr(fexpand("*.asg"));
}

void setup()
{
 integer gd,gm;
 registers r;
 byte floppies;
;
 gd=3; gm=0; initgraph(gd,gm,"c:\\bp\\bgi");
 dusk();
 setfillstyle(1,1); bar(2,2,637,197); shadow(0,0,639,199,15,7);

 if (loading)  doing="LOAD"; else doing="SAV";
 doing=doing+"ING a file...";

 /* Now... find all drives that exist. */
 drives="";
 intr(0x11,r); floppies=(((cardinal)r.ax >> 6) & 0x3)+1; /* useful bit of code! */
 for( gm=1; gm <= floppies; gm ++) drives=drives+chr(64+gm);
 /* Winchesters, etc., can be found the easy way... */
 for( gd=3; gd <= 26; gd ++) /* C: to Z: */
  if (disksize(gd)>-1)  drives=drives+chr(64+gd);

 fillchar(where,sizeof(where),'\1');
 fillchar(top,sizeof(top),'\1');

 settextstyle(0,0,0);
 settextjustify(1,1);
 for( gd=1; gd <= 2; gd ++)
  for( gm=0; gm <= 1; gm ++)
   shbox(420+gm*110,gd*25,520+gm*110,gd*25+20,buttons[gm*2+gd]);
 shbox(15,182,350,196,"Help... (press f1)");
 settextjustify(0,2); setcolor(11);
 setcolor(15); outtextxy(15,5,"The Avvy Filer...");
 setcolor(11); outtextxy(317,3,string('(')+doing+')');
 outtextxy(357,185,"Copyright (c) 1993, Thomas Thurman.");

 /* Draw the boxes and names */
 setcolor(3);
 for( gm=1; gm <= 4; gm ++)
  { windowtype& with = threewins[gm]; 
   box(with.x1,with.y1,with.x2,with.y2,with.title);}
 nowwin=1; getcurrent();
}

void quicksort(byte whichlist, integer lo, integer hi);


static void sort(integer l, integer r, byte& whichlist)
{
  integer i, j;
  varying_string<12> x, y;

  i = l; j = r; x = lists[whichlist][(l+r) / 2];
  do {
    while (lists[whichlist][i] < x)  i = i + 1;
    while (x < lists[whichlist][j])  j = j - 1;
    if (i <= j) 
    {
      y = lists[whichlist][i];
      lists[whichlist][i] = lists[whichlist][j];
      lists[whichlist][j] = y;
      i = i + 1; j = j - 1;
    }
  } while (!(i > j));
  if (l < j)  sort(l, j, whichlist);
  if (i < r)  sort(i, r, whichlist);
}

void quicksort(byte whichlist, integer lo, integer hi)

{     /*QuickSort*/;
  sort(lo,hi, whichlist);
}

void scandir()
{
 pathstr nix;
 namestr name;
 untyped_file f;
 ednahead eh;
 array<1,4,char> dna_type;

;
 nums[files]=0;
 findfirst("*.asg",archive+hidden+readonly,s);

 while (doserror==0) 
 {;
  fsplit(s.name,nix,name,nix);
  nums[files] += 1;
  lists[files][nums[files]]=lowstr(name);

  assign(f,s.name);
  reset(f,1);
  seek(f,11);
  blockread(f,dna_type,4);

  if (dna_type=="Avvy") 
  {;     /* A DNA256 file. */
   descs[nums[files]]="* Saved by Avaricius!";
  } else
  {;     /* EDNA-based files. */
   if (dna_type=="EDNA") 
   {;
    seek(f,177);
    blockread(f,eh,sizeof(eh));

     if (eh.revision!=2) 
      descs[nums[files]]="* Unknown EDNA type!";
     else
     {;

      if (eh.number!=2) 
       descs[nums[files]]=string("* Saved by ")+eh.shortname+'!';
      else
      {;     /* Well... everything seems to have gone OK! */
       descs[nums[files]]=eh.desc;
      }
     }
   } else
    descs[nums[files]]="* Not an Avvy saved game!";
  }
  close(f);
  findnext(s);
 }
 nums[dirs]=0; findfirst("*.*",directory,s);
 while (doserror==0) 
 {;
  if (((s.attr & directory)>0) && ((length(s.name))>1)) 
  {;
   nums[dirs] += 1;
   lists[dirs][nums[dirs]]=lowstr(s.name);
  }
  findnext(s);
 }

 /* Now sort 'em! */

 quicksort ( dirs,1,nums[ dirs]);
 quicksort (files,1,nums[files]);

 where[dirs]=1; where[files]=1;
   top[dirs]=1; top[files]=1;
}

void show_file(integer x,integer y, byte which)
{
 varying_string<58> z;
;
 fillchar(z[1],13,'\40');
 z=lists[files][which]+".asg";
 z[0]='\15'; z=z+descs[which];

 if (descs[which][1]=='*')  /* Can't load these! */
  setcolor(red);
 else
  setcolor(lightcyan);

 outtextxy(x,y,z);
}

void showfiles()
{
    byte fv;
;
 if (loading)  setcolor(11); else setcolor(3);
 if (nums[3]==0) 
 {;
  outtextxy(22,86,"(None here!)");
  return;
 }
 for( fv=0; fv <= 8; fv ++)
  if (where[3]+fv<=nums[3]) 
   show_file(19,87+fv*10,where[3]+fv);
}

void showdirs()
{
    byte fv;
;
 setcolor(11);
 for( fv=0; fv <= 8; fv ++)
  if (where[4]+fv<=nums[4]) 
   outtextxy(497,87+fv*10,string('[')+lists[dirs][fv+where[4]]+']');
}

void show()
{
 byte fv;
 dirstr d; namestr n; extstr e;
;
 setfillstyle(1,1);
 for( fv=1; fv <= 4; fv ++)
  if (fv!=2) 
   { windowtype& with = threewins[fv]; 
    bar(with.x1+1,with.y1+1,with.x2-1,with.y2-1);}
 showfiles();
 showdirs();
 setcolor(7); outtextxy(159,14,current);
}

void blip()
{;
 sound(177); delay(77); nosound;
}

void invert(integer x1,integer y1,integer x2,integer y2)
{
    pointer p,restore; word s;
;
 s=imagesize(x1,y1,x2,y2);
 mark(restore); getmem(p,s);
 getimage(x1,y1,x2,y2,p);
 putimage(x1,y1,p,notput);
 release(restore);
}

void changedrive(char drive)
{
    byte fv;
;
 fv=pos(drive,drives);
 if (fv==0)  {; blip(); return; }
 fv -= 1;
 shadow((fv % drlen)*25+25,(fv / drlen)*19+31,
   (fv % drlen)*25+40,45+(fv / drlen)*19,8,7);
 chdir(string(drive)+':');
 getcurrent(); scandir(); show();
 shadow((fv % drlen)*25+25,(fv / drlen)*19+31,
   (fv % drlen)*25+40,45+(fv / drlen)*19,15,8);
}

void highlight(byte win,byte line)
{;
 switch (win) {
  case 3: invert(16,75+line*10,479,85+line*10); break;
  case 4: invert(491,75+line*10,619,85+line*10); break;
 }
}

void repaint(byte whichwindow)
{
 setfillstyle(1,1);
 { windowtype& with = threewins[whichwindow]; 
  bar(with.x1+1,with.y1+1,with.x2-1,with.y2-1);}
 switch (whichwindow) {
  case file_win: showfiles(); break;
  case subdir_win: showdirs(); break;
 }
 highlight(whichwindow,1);
 top[whichwindow]=where[whichwindow];
}

void fileblit(word xpos,word xlen,word y1,word y2, shortint dir, word ylen)
{
    word fv; byte bit;
;
 for( bit=0; bit <= 3; bit ++)
 {;
  fv=0;
  while (fv<ylen) 
  {;
   port[0x3c4]=2; port[0x3ce]=4; port[0x3c5]=1 << bit; port[0x3cf]=bit;
   move(mem[0xa000*(y1+fv*dir)*80+xpos],mem[0xa000*(y2+fv*dir)*80+xpos],xlen);
   fv += 1;
  }
 }
 bit=getpixel(0,0);
}

void seekthrough(byte whichlist, byte& wherenow, char whatfor)
{
    byte startedat;

 startedat=wherenow;
 do {
  wherenow += 1;
  if (wherenow>nums[whichlist])  wherenow=1;
 } while (!((startedat==wherenow) || (lists[whichlist][wherenow][1]==whatfor)));
 repaint(whichlist);
}

void gotohome(byte whichlist)
{
 where[whichlist]=1;
 repaint(whichlist);
}

void gotoend(byte whichlist)
{
 where[whichlist]=nums[whichlist];
 repaint(whichlist);
}

void pageup(byte whichlist)
{
 if (where[whichlist]>9) 
 {
  where[whichlist] -= 9;
  repaint(whichlist);
 }
}

void pagedown(byte whichlist)
{
 if (where[whichlist]<nums[whichlist]-9) 
 {
  where[whichlist] += 9;
  repaint(whichlist);
 }
}

void subdirparse(string r);

static void movehl(byte which, shortint howmuch)
{;
 highlight(4,where[4]-top[4]+1);
 if (((where[which]+howmuch)>0) && ((where[which]+howmuch)<=(unsigned char)nums[4]))
       where[which]=where[which]+howmuch;
 highlight(4,where[4]-top[4]+1);
}



static void change_dir()
{;
 dusk();
 /*$I-*/ chdir(lists[4][where[4]]);
 /*$I+*/
 if (ioresult!=0)  {; dawn(); blip(); return; }
 where[4]=1; top[4]=1;
 getcurrent(); scandir(); show();
 highlight(4,1);
 dawn();
}

void subdirparse(string r)

{;
 switch (r[1]) {
  case creturn: change_dir(); break;

  case '\0': switch (r[2]) {
       case cup: if (where[4]-top[4]>0)  /* Up */
             movehl(4,-1); /* Within range */
            else if (top[4]>1) 
            {;     /* Outside range- must scroll */
             highlight(4,1);
             top[4] -= 1; where[4] -= 1;
             fileblit(61,18,166,176,-1,80);
             setfillstyle(1,1); bar(490,85,630,95); setcolor(11);
             outtextxy(497,87,string('[')+lists[dirs][where[4]]+']');
             highlight(4,1);
            }
            break;
       case cdown:
            if (where[4]-top[4]<8)    /* Down */
             movehl(4,1);
            else if (top[4]+8<nums[4]) 
            {;
             highlight(4,9);
             top[4] += 1; where[4] += 1;
             fileblit(60,18,97,87,1,80);
             setfillstyle(1,1); bar(490,165,630,175); setcolor(11);
             outtextxy(497,167,string('[')+lists[dirs][where[4]]+']');
             highlight(4,9);
            }
            break;
       case chome: gotohome(4); break;
       case cend: gotoend(4); break;
       case cpgup: pageup(4); break;
       case cpgdn: pagedown(4); break;
       case c_ao: change_dir(); break;
      }
      break;
   default: seekthrough(4,where[4],lowchar(r[1]));
 }
}

void fileinfo(byte which);


static void display(integer y, string left,string right)
{;
 y=17+y*12;
 settextjustify(2,1); setcolor(11); outtextxy(315,y,left);
 settextjustify(0,1); setcolor(15); outtextxy(325,y,right);
}

void fileinfo(byte which)
 /* This gives information on the file whose name is in lists[files,which]. */
{
 ednahead eh;
 untyped_file f;
 varying_string<4> os;
 char r;

;

 /* Firstly, we must check whether or not it's an Avalot file. This is easily
   done, since the descriptions of all others always begin with a star. */

 if ((descs[which][1]=='*') || (descs[which][1]=='(')) 
 {;     /* it is. */
  blip(); /* Naaaarghh! */
  return;
 }

 /* Anyway... it wasn't. */

 assign(f,lists[files][which]+".asg");
 reset(f,1);
 seek(f,177);
 blockread(f,eh,sizeof(eh));
 close(f);

 /* We now hold its EDNA record. */

 setfillstyle(1,1); bar(2,2,637,197); /* Interesting information coming up! */

 {;
  display(2,"Saved by:", eh.game);
  display(3,"version:", eh.verstr);

/*  display(4,'under', os);*/

  display(6,"Saved on ",strf(eh.d)+'-'+strf(eh.m)+'-'+strf(eh.y));

  display(9,"No. of times saved:",strf(eh.saves));

  display(11,"Money:",eh.money);
  display(12,"Score:",strf(eh.points));
 }

 shbox(500,177,650,200,"Press any key...");
 r=readkey();

 setfillstyle(1,1); bar(2,2,637,197);
}

void filer_help()
 /* This gives general help. */
{
 char r;
;
 outtextxy(100,100,"Just general help here.");
 shbox(500,177,650,200,"Press any key...");
 r=readkey();

 setfillstyle(1,1); bar(2,2,637,197);
}

void wipe()
 /* This allows you to delete files. */
{
 char r;
;
 outtextxy(100,100,"Are you sure you want to delete \"foo.bar\"?");
 shbox(500,177,650,200,"[Y/N]");
 r=readkey();

 setfillstyle(1,1); bar(2,2,637,197);
}

void filesparse(string r);

static void movehl1(byte which, shortint howmuch)
{;
 highlight(3,where[3]-top[3]+1);
 if (((where[which]+howmuch)>0) && ((where[which]+howmuch)<=(unsigned char)nums[3]))
       where[which]=where[which]+howmuch;
 highlight(3,where[3]-top[3]+1);
}



static boolean selected_file()
{boolean selected_file_result;
;
 if (descs[where[file_win]][1]=='*') 
 {
  blip();
  selected_file_result=false;
 } else
 {
  filename=lists[file_win][where[file_win]];
  filefound=true;
  selected_file_result=true;
 }
return selected_file_result;
}

void filesparse(string r)

{;
 switch (r[1]) {
  case creturn: if (selected_file())  return; break;
  case '\0': switch (r[2]) {
       case cup: if (where[3]-top[3]>0)  /* Up */
             movehl1(3,-1); /* Within range */
            else if (top[3]>1) 
            {;     /* Outside range- must scroll */
             highlight(3,1);
             top[3] -= 1; where[3] -= 1;
             fileblit(1,59,166,176,-1,80);
             setfillstyle(1,1); bar( 15,85,480,95);
             show_file( 19,87,where[3]);
             highlight(3,1);
            }
            break;
       case cdown: if (where[3]-top[3]<8)  /* Down */
             movehl1(3,1);
            else if (top[3]+8<nums[3]) 
            {;
             highlight(3,9);
             top[3] += 1; where[3] += 1;
             fileblit(1,59,97,87,1,80);
             setfillstyle(1,1); bar( 15,165,480,175);
             show_file( 19,167,where[3]);
             highlight(3,9);
            }
            break;
       case c_ai: fileinfo(where[3]); break; /* alt-I: information. */
       case c_ah:case c_f1: filer_help(); break; /* alt-I: information. */
       case chome: gotohome(3); break;
       case cend: gotoend(3); break;
       case cpgup: pageup(3); break;
       case cpgdn: pagedown(3); break;
       case c_ao: if (selected_file())  return; break;
      }
      break;
  default: seekthrough(3,where[3],lowchar(r[1]));
 }
}

string playaround();

static void changewin(byte i)
{;
 switch (nowwin) {
  case 3:case 4: highlight(nowwin,where[nowwin]-top[nowwin]+1); break;
 }
 setcolor(3); { windowtype& with = threewins[nowwin];  box(with.x1,with.y1,with.x2,with.y2,with.title);}
 nowwin=i;
 if (nowwin<1)  nowwin=4; if (nowwin>4)  nowwin=1;
 switch (nowwin) {
  case 3:case 4: highlight(nowwin,where[nowwin]-top[nowwin]+1); break;
 }
}

string playaround()
{
    char r,r2;
string playaround_result;
;
 filefound=false; dawn();

 do {
  setcolor(14); { windowtype& with = threewins[nowwin];  box(with.x1,with.y1,with.x2,with.y2,with.title);}
  r=readkey();
  switch (r) {
   case ctab: changewin(nowwin+1); break;
   case cescape: {;
             playaround_result="";
             return playaround_result;
            }
            break;
   case '\0': {; /* parse extd keystroke */
        r2=readkey();
        switch (r2) {
         case cs_tab: changewin(nowwin-1); break;
         case c_an: changewin(1); break;
         case c_ad: changewin(2); break;
         case c_af: changewin(3); break;
         case c_as: changewin(4); break;
         case c_ac: {;
                playaround_result="";
                return playaround_result;
               }
               break;
         default:
          switch (nowwin) {
           case 3: filesparse(string('\0')+r2); break;
           case 4: subdirparse(string('\0')+r2); break;
          }
         }
        }
        break;
  default:
   {;     /* Pass keystroke to current window */
    switch (nowwin) {
     case 2: changedrive(upcase(r)); break;
     case 4: subdirparse(r); break;
     case 3: filesparse(r); break;
     default: blip();
    }
   }

  }

  if (filefound) 
  {;
   dusk();
   playaround_result=filename;
   return playaround_result;
  }
 } while (!false);
return playaround_result;
}

void do_filer()
{
    pathstr p;
void do_filer_result;
;
 loading=true;
 setup();
 scandir();
 show(); show_drives();
 p=playaround();
 if (p!="")  p=fexpand(p+".ASG");
 do_filer_result=p;
return do_filer_result;
}

int main(int argc, const char* argv[])
{pio_initialize(argc, argv);
;
 filename=do_filer();
 closegraph();
 if (filename=="") 
  output << "*** CANCELLED! ***" << NL;
 else
  output << "Selected: " << filename << NL;
 input >> NL;
return EXIT_SUCCESS;
}
