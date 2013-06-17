#include "ptoc.h"

                            /*$S-*/
#include "graph.h"


typedef array<1,50000,byte> buffertype;

integer gd,gm;
varying_string<29> describe;
byte method;
shortint bit;
word offset;
byte a0; /*absolute $A000:800;*/
array<1,12080,byte> a7;     /*absolute $A000:800;*/
buffertype* buffer;
word bufsize;

void putup(byte what)
{;
 if (offset>12080) 
 {;
  bit += 1;
  port[0x3c4]=2; port[0x3ce]=4; port[0x3c5]=1 << bit; port[0x3cf]=bit;
  offset=1;
 }

 a7[offset]=what;
 offset += 1;
}

void load_uncomp(string xx)       /* Load2, actually */
/* a1:byte absolute $A000:17184;*/
{
 byte this_;
 untyped_file f;
 word place;
;
 assign(f,string("v:place")+xx+".avd"); reset(f,1); seek(f,146);
 blockread(f,describe,30); blockread(f,method,1);
 bufsize=filesize(f)-177; blockread(f,*buffer,bufsize);
 close(f);

 bit=-1; offset=12081; place=1;

 while (place<=bufsize) 
 {;
  this_=(*buffer)[place];
  place += 1;
  putup(this_);
 }

 output << method << " : \"" << describe << '"' << NL;
}

void load_comp(string xx);

const integer maxtab = 4095     /* Table size - 1 ==> 2**10-1 ==> 12 bits */;

const integer no_prev = -1      /* Special code for no previous character */;

const integer end_list = -1     /* Marks end of a list                    */;

const integer maxstack = 4096   /* Decompression stack size  */;




                                /* One node in parsing table.             */
struct string_table_entry {
                        boolean unused      /* Is this node *NOT* used yet?*/;
                        integer prevchar    /* Code for preceding string   */;
                        integer follchar    /* Code for current character  */;
                        integer next        /* Next dupl in collision list */;
};



typedef array<0,maxtab,string_table_entry> sttype;


                                             /* String table */

static sttype* string_table;


static integer table_used                 /* # string table entries used */;

static byte input_code                    /* Input compressed code       */;

static boolean inempty,popempty;

                                   /* Decompression stack       */

static array<1,maxstack,byte> stack;


static integer stack_pointer    /* Decompression stack depth */;

static word place;


static integer get_hash_code( integer& prevc, integer& follc )

{
   integer index;
   integer index2;

      /* Get_Hash_Code */
                                   /* Get initial index using hashing */

   integer get_hash_code_result;
   index = ( ( prevc << 5 ) ^ follc ) & maxtab;

                                   /* If entry not already used, return */
                                   /* its index as hash code for <w>C.  */

   if ( (*string_table)[index].unused ) 
      get_hash_code_result = index;
   else
                                   /* If entry already used, search to  */
                                   /* end of list of hash collision     */
                                   /* entries for this hash code.       */
                                   /* Do linear probe to find an        */
                                   /* available slot.                   */
      {

                                   /* Skip to end of collision list ... */

         while ( (*string_table)[index].next != end_list ) 
            index = (*string_table)[index].next;

                                   /* Begin linear probe down a bit from  */
                                   /* last entry in collision list ...    */

         index2 = ( index + 101 ) & maxtab;

                                   /* Look for unused entry using linear  */
                                   /* probing ...                         */

         while ( ! (*string_table)[index2].unused ) 
            index2 = succ(integer, index2 ) & maxtab;

                                   /* Point prior end of collision list   */
                                   /* to this new node.                   */

         (*string_table)[index].next = index2;

                                   /* Return hash code for <w>C           */

         get_hash_code_result          = index2;

      }

   return get_hash_code_result;
}     /* Get_Hash_Code */



       /*--------------------------------------------------------------------------*/
       /*          Make_Table_Entry --- Enter <w>C string in string table          */
       /*--------------------------------------------------------------------------*/

static void make_table_entry( integer& prevc, integer& follc )

{     /* Make_Table_Entry */
                                   /* Only enter string if there is room left */

   if ( table_used <= maxtab ) 
      {
         {
               string_table_entry& with = (*string_table)[ get_hash_code( prevc , follc ) ]; 

               with.unused   = false;
               with.next     = end_list;
               with.prevchar = prevc;
               with.follchar = follc;
         }
                                   /* Increment count of items used */

         table_used += 1;
/*
                IF ( Table_Used > ( MaxTab + 1 ) ) THEN
                   BEGIN
                      WRITELN('Hash table full.');
                   END;
       */
      }

}     /* Make_Table_Entry */



static void firstentries()
/* This is just a fast version of the above, when PrevC = No_Prev. TT. */

{
    integer i,j;


  /* There MUST be room- we've only just started! */

  j=no_prev;

  for( i=0; i <= 255; i ++)
   {
    string_table_entry& with = (*string_table)[ ((no_prev << 5 ) ^ i) & maxtab]; 

    with.unused   = false;
    with.next     = end_list;
    with.prevchar = no_prev;
    with.follchar = i;
   }

  table_used += 256;      /* Increment count of items used */

}



       /*--------------------------------------------------------------------------*/
       /*            Initialize_String_Table --- Initialize string table           */
       /*--------------------------------------------------------------------------*/

static void initialize_string_table()

{
   integer i;

      /* Initialize_String_Table */

                                   /* No entries used in table yet */
   table_used  = 0;

 fillchar(*string_table,(maxtab+1)*sizeof((*string_table)[1]),'\377');
                                   /* Enter all single characters into */
                                   /* table                            */
 firstentries();

}     /* Initialize_String_Table */



       /*--------------------------------------------------------------------------*/
       /*            Lookup_String --- Look for string <w>C in string table        */
       /*--------------------------------------------------------------------------*/

static integer lookup_string( integer prevc, integer follc )

{
   integer index;
   integer index2;
   boolean found;

      /* Lookup_String */
                                   /* Initialize index to check from hash */

   integer lookup_string_result;
   index       = ( ( prevc << 5 ) ^ follc ) & maxtab;

                                   /* Assume we won't find string */
   lookup_string_result = end_list;
                                   /* Search through list of hash collision */
                                   /* entries for one that matches <w>C     */
   do {

      found = ( (*string_table)[index].prevchar == prevc ) &&
               ( (*string_table)[index].follchar == follc );

      if ( ! found ) 
         index = (*string_table)[index].next;

   } while (!(found || ( index == end_list )));

                                   /* Return index if <w>C found in table. */
   if (found) 
      lookup_string_result = index;

   return lookup_string_result;
}     /* Lookup_String */



       /*--------------------------------------------------------------------------*/
       /*                  Push --- Push character onto stack                      */
       /*--------------------------------------------------------------------------*/

static void push( byte c)

{     /* Push */

  stack_pointer += 1;
  stack[ stack_pointer ] = c;

  if ( stack_pointer >= maxstack ) 
     {
        output << "Stack overflow!" << NL;
        exit(0);
     }

}    /* Push */



       /*--------------------------------------------------------------------------*/
       /*                  Pop --- Pop character from stack                        */
       /*--------------------------------------------------------------------------*/

static void pop( integer& c )

{     /* Pop */

 popempty=stack_pointer==0;

   if (! popempty) 
      {
         c = stack[stack_pointer];
         stack_pointer -= 1;
      }

}     /* Pop */



       /*--------------------------------------------------------------------------*/
       /*            Get_Code --- Get compression code from input file             */
       /*--------------------------------------------------------------------------*/

static void get_code( integer& hash_code )

{
   byte local_buf;

      /* Get_Code */

   if (inempty) 
      {

         if (place>bufsize)  return; else
         {;
          local_buf=(*buffer)[place];
          place += 1;
         }

         if (place>bufsize)  return; else
         {;
          input_code=(*buffer)[place];
          place += 1;
         }

         hash_code  = ( ( local_buf << 4  ) & 0xff0 ) +
                       ( ( (cardinal)input_code >> 4 ) & 0xf );

         input_code = input_code & 0xf;
         inempty=false;

      }
   else
      {

         if (place>bufsize)  return; else
         {;
          local_buf=(*buffer)[place];
          place += 1;
         }

         hash_code  = local_buf + ( ( input_code << 8 ) & 0xf00 );
         inempty=true;

      }

}     /* Get_Code */



       /*--------------------------------------------------------------------------*/
       /*            Do_Decompression --- Perform decompression                    */
       /*--------------------------------------------------------------------------*/

static void do_decompression()

{
   integer c                       /* Current input character */;
   integer code                    /* Current code string     */;
   integer old_code                /* Previous code string    */;
   integer fin_char                /* Final input character   */;
   integer in_code                 /* Current input code      */;
   integer last_char               /* Previous character      */;
   boolean unknown                 /* TRUE if code not found  */;
   integer temp_c                  /* Char popped off stack   */;

      /* Do_Decompression */

  stack_pointer = 0;      /* Decompression stack is empty */
  unknown       = false;  /* First string is always known */
  get_code( old_code );   /* Get first string == Step 1   */
  code          = old_code;

  c=(*string_table)[code].follchar; /* Output corresponding character */
  putup( c );
  fin_char = c;  /* Remember this character  -- it    */
                 /* is final character of next string */

  get_code( in_code ); /* Get next code  == Step 2 */

  while (place<=bufsize) 
     {
        code = in_code;  /* Set code to this input code */

        /* If code not in table, do special */
        /* case ==> Step 3                  */

        if ( (*string_table)[code].unused ) 
           {
              last_char = fin_char;
              code      = old_code;
              unknown   = true;
           }
                                   /* Run through code extracting single */
                                   /* characters from code string until  */
                                   /* no more characters can be removed. */
                                   /* Push these onto stack.  They will  */
                                   /* be entered in reverse order, and   */
                                   /* will come out in forwards order    */
                                   /* when popped off.                   */
                                   /*                                    */
                                   /* ==> Step 4                         */

        while( (*string_table)[code].prevchar != no_prev ) 
           {
                 string_table_entry& with = (*string_table)[code]; 

                 push( with.follchar );
                 code = with.prevchar;
           }
                                   /* We now have the first character in */
                                   /* the string.                        */

        fin_char = (*string_table)[code].follchar;

                                   /* Output first character  ==> Step 5   */
        putup( fin_char );
                                   /* While the stack is not empty, remove */
                                   /* and output all characters from stack */
                                   /* which are rest of characters in the  */
                                   /* string.                              */
                                   /*                                      */
                                   /* ==> Step 6                           */
        pop( temp_c );

        while (! popempty) 
           {
              putup( temp_c );
              pop( temp_c );
           }
                                   /* If code isn't known, output the      */
                                   /* follower character of last character */
                                   /* of string.                           */
        if (unknown) 
           {
              fin_char = last_char;
              putup( fin_char );
              unknown  = false;
           }
                                   /* Enter code into table ==> Step 7 */

        make_table_entry( old_code , fin_char );

                                   /* Make current code the previous code */
        old_code = in_code;

                                   /* Get next code  == Step 2 */
        get_code( in_code );

     }

}     /* Do_Decompression */

void load_comp(string xx){

                                /* This loads in the compressed file. */

const integer maxbuff = 8192    /* Buffer size for input and output files */;

integer output_code                       /* Output compressed code      */;
boolean if_compressing                    /* TRUE if compressing file    */;

byte this_;
untyped_file f;

;
string_table = new sttype;
inempty=true;
initialize_string_table();

assign(f,string("v:compr")+xx+".avd"); reset(f,1); seek(f,146);
blockread(f,describe,30); blockread(f,method,1);
bufsize=filesize(f)-177; blockread(f,*buffer,bufsize);
close(f);

bit=-1; offset=12081; place=1;

do_decompression();

output << method << " : \"" << describe << '"' << NL;

delete string_table;
                         }

int main(int argc, const char* argv[])
{pio_initialize(argc, argv);
;
gd=3; gm=0; initgraph(gd,gm,"c:\\bp\\bgi");
buffer = new buffertype;
#ifdef uncomp
load_uncomp("21");
#else
load_comp("21");
#endif
delete buffer;
return EXIT_SUCCESS;
}


