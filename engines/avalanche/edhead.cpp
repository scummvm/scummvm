



const char months[] = "JanFebMarAprMayJunJulAugSepOctNovDec";
const string ednaid = string("TT")+'\261'+'\36'+'\1'+'\113'+'\261'+'\231'+'\261';

struct edhead { /* Edna header */
          /* This header starts at byte offset 177 in the .ASG file. */
          array<1,9,char> id;     /* signature */
          word revision; /* EDNA revision, here 2 (1=dna256) */
          varying_string<50> game; /* Long name, eg Lord Avalot D'Argent */
          varying_string<15> shortname; /* Short name, eg Avalot */
          word number; /* Game's code number, here 2 */
          word ver; /* Version number as integer (eg 1.00 = 100) */
          varying_string<5> verstr; /* Vernum as string (eg 1.00 = "1.00" */
          varying_string<12> filename; /* Filename, eg AVALOT.EXE */
          byte os; /* Saving OS (here 1=DOS. See below for others.) */

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
     3 = OS/2       6 = ST */

untyped_file f;
byte fv;
array<1,255,word> dna256;
boolean ok;
edhead e;

void info(string x)       /* info on .ASG files */
{
 varying_string<40> describe;
;
 assign(f,x);
 /*$I-*/ reset(f,1);
 seek(f,47);
 blockread(f,describe,40);
 blockread(f,dna256,sizeof(dna256));
 close(f); /*$I+*/
  e.revision=1;
  game="Denarius Avaricius Sextus";
  shortname="Avaricius";
  number=1;
  verstr="[?]";
  filename="AVVY.EXE";
  os=1;
  fn=x;
  d=dna256[7]; m=dna256[8]; y=dna256[9];
  desc=describe;
  len=512;
  saves=dna256[6];
  /*money:string[20]; { ditto in string form (eg 5/-, or 1 denarius)*/
  /*points:word; { your score */
}

int main(int argc, const char* argv[])
{pio_initialize(argc, argv);
;
 info("tt.asg");

 output << "Filename: " << x << NL;
 output << "Description: " << desc << NL;
 output << "Cash: " << dna256[30] << NL;
 output << "Score: " << dna256[36] << NL;
 output << "Date: " << dna256[7] << ' ' << copy(months,dna256[8]*3-2,3) << ' ' << dna256[9] << NL;
 output << "Number of saves: " << dna256[6] << NL;
return EXIT_SUCCESS;
}
