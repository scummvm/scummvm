/*
 * This code is based on the original source code of Lord Avalot d'Argent version 1.3.
 * Copyright (c) 1994-1995 Mike, Mark and Thomas Thurman.
 */



/*$M 10000,0,1000*/ /*$V-*/

/*#include "Dos.h"*/
/*#include "Tommys.h"*/


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
          varying_string<5> os; /* Saving OS in text format. */

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

/* Possible values of edhead.os:
     1 = DOS        4 = Mac
     2 = Windows    5 = Amiga
     3 = OS/2       6 = ST
     7 = Archimedes */

typedef array<1,4,char> fourtype;

struct avaricius_dna {
                 varying_string<39> desc;
                 array<1,256,integer> dna;
};

const string ednaid = string("TT")+'\261'+'\60'+'\1'+'\165'+'\261'+'\231'+'\261';
const fourtype avaricius_file = "Avvy";

string filename;
boolean quiet,info;
ednahead eh;

boolean avaricius;
fourtype id4;
avaricius_dna av_eh;
varying_string<4> av_ver;

boolean ok;

string first_dir;

void explain()
{
  output << "EDNA Bucket v1.0 (c) 1993 Thomas Thurman." << NL;
  output << "  To load Avvy files." << NL;
  output << NL;
  output << "Associate .ASG files with EDNA.EXE to load them directly." << NL;
  output << NL;
  output << "Switches:" << NL;
  output << "  /q (quiet) will stop EDNA from printing text to the screen." << NL;
  output << "  /i (info) will print info about the file, but won't load it." << NL;
  output << NL;
  exit(1);
}

void fix_filename()
{
    string p,n,groi;

  fsplit(filename,p,n,groi);
  filename=p+n+".ASG";
}

void error(string x)
{
  output << "EDNA : " << x << NL;
  exit(255);
}

void paramparse()
{
 byte fv;
 string x;

  if (paramcount==0)  explain();

  filename="";
  quiet=false; info=false;

  for( fv=1; fv <= paramcount; fv ++)
  {
    x=paramstr(fv);

    if ((x[1]=='/') || (x[1]=='-')) 
       switch (upcase(x[2])) { /* Parse switches */
         case 'Q': quiet=! quiet; break;
         case 'I': info=! info; break;
         default: error(string("Unknown switch! (")+x[2]+')');
       }
    else
       if (filename=="")  filename=x;
         else error("Please, only one filename at a time!");
  }

  if (quiet && info)  error("How can you give info quietly??");

  if (filename=="")  error("No filename given! Use EDNA alone for help.");

  fix_filename();
}

void getfile()
{
    untyped_file f;

  assign(f,filename);
 /*$I-*/
  reset(f,1);
 /*$I+*/
  if (ioresult!=0)  error(string("Can't read file \"")+filename+"\".");

  seek(f,11); blockread(f,id4,4);
  avaricius=id4==avaricius_file;

  if (avaricius) 
  {
    seek(f,47);
    blockread(f,av_eh,sizeof(av_eh));
    av_ver[0]='\4'; seek(f,31); blockread(f,av_ver[1],4);
  } else
  {
    seek(f,177);
    blockread(f,eh,sizeof(eh));
  }

  close(f);
}

string plural(byte x)
{
  string plural_result;
  if (x==1)  plural_result=""; else plural_result='s';
  return plural_result;
}

void show_info()
{
    string _game,_shortname,_verstr,_filename,_os,_fn,_desc,_money;
   integer _revision,_number,_d,_m,_y,_saves,_points;
   boolean readable,understandable;

  output << "Info on file " << filename << ':' << NL;
  output << NL;
    if (avaricius) 
    {     /* DNA-256 file. */
      _verstr=av_ver;
      _game="Denarius Avaricius Sextus"; _shortname="Avaricius";
      _filename="AVVY.EXE"; _os="DOS"; _desc=av_eh.desc;
      _revision=1; _number=1; _fn="(as above)";

      _money=strf(av_eh.dna[30])+" denari";
      if (av_eh.dna[30]==1)  _money=_money+"us"; else _money=_money+'i';
      _d=av_eh.dna[7]; _m=av_eh.dna[8]; _y=av_eh.dna[9];
      _saves=av_eh.dna[6]; _points=av_eh.dna[36];

      readable=true; understandable=true;
    } else
     {
       if (eh.id==ednaid) 
       {      /* EDNA file. */
        _game=eh.game;
        _shortname=eh.shortname;
        _verstr=eh.verstr;
        _filename=eh.filename;
        _os=eh.os; _fn=eh.fn; _desc=eh.desc;
        _money=eh.money; _revision=eh.revision;
        _number=eh.number; _d=eh.d; _m=eh.m; _y=eh.y;
        _saves=eh.saves; _points=eh.points;

         readable=true; understandable=eh.revision==2;
       } else
       {
         output << "Unknown format." << NL;
         readable=false;
       }
     }

    if (_desc=="")  _desc="<none>";

    if (readable) 
    {
      output << "Signature is valid." << NL;
      output << "Revision of .ASG format: " << _revision << NL;
      output << NL;
      if (understandable) 
      {
        output << "The file was saved by " << _game << '.' << NL;
        output << "Game number " << _number << " (" << _shortname << "), version " << 
            _verstr << ", filename " << _filename << '.' << NL;
        output << "Saved under " << _os << '.' << NL;
        output << NL;
        output << "This is " << _fn << ".ASG, saved on " << _d << '/' << _m << '/' << _y << " (d/m/y)." << NL;
        output << "Description: " << _desc << NL;
        output << "It has been saved " << _saves << " time" << plural(_saves) << 
                    ". You have " << _points << " point" << plural(_points) << ',' << NL;
        output << "and " << _money << " in cash." << NL;
      } else output << "Nothing else can be discovered." << NL;
    }

  exit(2);
}

void load_file()
{
 string progname,gamename,shortname,listname,exname,prog_dir;

 string localdir,groi;

 string x,y;

 text t;

 integer i,ii;

  gamename=fexpand(filename);
  fsplit(fexpand(paramstr(0)),localdir,groi,groi);
  listname=localdir+"EDNA.DAT";

  if (avaricius) 
       { shortname="Avaricius"; exname="AVVY"; }
  else { shortname=eh.shortname; fsplit(eh.filename,groi,exname,groi); }

  assign(t,listname);
 /*$I-*/
  reset(t);
 /*$I+*/
  progname="";
  if (ioresult==0) 
  {
    do {
      t >> x >> NL; t >> y >> NL;
      if (x==shortname) 
      {
        progname=y;
        flush();
      }
    } while (!(eof(t)));
  }

  if (progname=="") 
  {      /* No entry in EDNA.DAT */
    output << "This file was saved by " << shortname << '.' << NL;
    output << "However, no entry was found in EDNA.DAT for that game." << NL;
    output << NL;
    output << "Please give the full path to that game, or press Enter to cancel." << NL;
    output << string("(Example: C:\\")+exname+'\\'+exname+".EXE)" << NL;
    output << NL;
    input >> progname >> NL;
    if (progname=="")  exit(254);   /* Quick exit! */

   /*$I-*/
    append(t);
    if (ioresult!=0)  rewrite(t);

    t << shortname << NL;
    t << progname << NL;

    if (ioresult!=0) 
    {
      output << "Strange... could not write to EDNA.DAT. (Disk read-only or full?)" << NL;
      output << "The path will be used this time only." << NL;
      output << "Press Enter..."; input >> NL;
    }
    close(t);
    /*$I+*/
  }

  if (! quiet)  output << "Running " << shortname << ": " << progname << "..." << NL;

  fsplit(fexpand(progname),prog_dir,groi,groi);
  if (prog_dir[length(prog_dir)]=='\\')  prog_dir[0] -= 1;

  /*$I-*/
  chdir(prog_dir); i=ioresult;
  swapvectors;
  exec(progname,gamename); ii=ioresult;
  swapvectors;
  chdir(first_dir);
  /*$I+*/

  if ((i!=0) || (ii!=0)) 
  {
    output << "WARNING: DOS reported an error. This probably means that the entry" << NL;
    output << "for this game in " << listname << " is wrong." << NL;
    output << NL;
    output << "Please edit this file, using" << NL;
    output << NL;
    if (lo(dosversion)<0x5) 
      output << "  edlin " << listname << " (or similar)" << NL;
    else
      output << "  edit " << listname << NL;
    output << NL;
    output << "and change the line after \"" << shortname << "\" to show the correct path." << NL;
    output << NL;
    output << "More info is in the Avvy documentation. Good luck!" << NL;
    output << NL;
    output << "Press Enter..."; input >> NL;
  }
}

int main(int argc, const char* argv[])
{
  pio_initialize(argc, argv);
  getdir(0,first_dir);
  paramparse();
  getfile();
  if (info)  show_info();
  load_file();
  return EXIT_SUCCESS;
}