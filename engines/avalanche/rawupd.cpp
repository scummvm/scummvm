


/*#include "Dos.h"*/
/*#include "Crt.h"*/

searchrec s;
string x,y;
longint hash_time,s_time;
boolean s_exists;

void find_out_about_s(string name)
{
    searchrec ss;
;
 findfirst(name,anyfile,ss);
 s_exists=doserror==0;

 if (s_exists) 
  s_time=ss.time;
}

void get_y()
{
 y=x;
 if (set::of(range('0','9'), eos).has(x[2])) 
  y[1]='h'; else
  if (x[3]=='K') 
   y[1]='a'; else
    y[1]='s';
}

void rename_it()
{
    untyped_file f;

 output << x << " -> " << y;
 assign(f,x); reset(f); rename(f,y); close(f);
 output << " ...done." << NL;
}

int main(int argc, const char* argv[])
{
 pio_initialize(argc, argv);
 output << NL;
 findfirst("#*.*",anyfile,s);
 while (doserror==0) 
 {
  x=s.name;
  get_y();
  hash_time=s.time;
  output << format(x,15); clreol;
  find_out_about_s(y);
  if (s_exists) 
  {
   output << ": s exists and is ";
   if (s_time<hash_time) 
    output << "NEWER!" << NL;
    else if (s_time==hash_time) 
     output << string("equal.")+'\15';
      else output << "older." << NL;
  } else
  {
   output << " ... NO S FOUND! Renaming...";
   rename_it();
  }

  findnext(s);
 }
 return EXIT_SUCCESS;
}