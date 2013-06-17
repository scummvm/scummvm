



enum flavourtype {ch_ega,ch_bgi, last_flavourtype};

struct chunkblocktype {
                  flavourtype flavour;
                  integer x,y;
                  integer xl,yl;
                  longint size;
                  boolean natural;

                  boolean memorise; /* Hold it in memory? */
};

untyped_file f;
string fn;
byte num_chunks,fv;
longint offset;
chunkblocktype ch;

int main(int argc, const char* argv[])
{
   pio_initialize(argc, argv);
   output << NL;
   output << "CHUNKER 12/3/1995 TT" << NL;
   output << NL;

   if (paramcount!=1) 
   {
      output << "which chunk file?" << NL;
      exit(0);
   }

   fn=paramstr(1);
   assign(f,fn);
   reset(f,1);
   output << "----- In chunk file " << fn << ", there are: -----" << NL;

   seek(f,44);
   blockread(f,num_chunks,1);
   output << format(num_chunks,4) << " chunks:" << NL;

   output << "  No  Hdr    Offset  Flvr  Mem Nat      X      Y  Width Height Size of image" << NL;

   for( fv=1; fv <= num_chunks; fv ++)
   {

      output << "Ch" << format(fv,2) << ':';

      seek(f,41+fv*4);

      output << format(41+fv*4,4);
      blockread(f,offset,4);
      output << format(offset,10);

      seek(f,offset);
      blockread(f,ch,sizeof(ch));
      {
         if (ch.flavour==ch_bgi) 
            output << " ch_BGI";
         else
            output << " ch_EGA";

         if (ch.memorise) 
            output << " yes";
         else
            output << " no ";

         if (ch.natural) 
            output << " yes";
         else
            output << " no ";

         output << format(ch.x,7) << format(ch.y,7) << format(ch.xl,7) << format(ch.yl,7) << format(ch.size,10);
      }

      output << NL;
   }

   output << "---ENDS---" << NL;

   close(f);
   return EXIT_SUCCESS;
}