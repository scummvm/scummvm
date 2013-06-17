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

/* This is the first version. Thanks to Pib. */

/*--------------------------------------------------------------------------*/
/*                                                                          */
/*                  Global declarations for PIBLZW                          */
/*                                                                          */
/*--------------------------------------------------------------------------*/

namespace Avalanche {

const integer maxbuff = 8192    /* Buffer size for input and output files */;
const integer maxtab = 4095     /* Table size - 1 ==> 2**10-1 ==> 12 bits */;
const integer no_prev = 0x7fff  /* Special code for no previous character */;
const integer eof_char = -2     /* Marks end of file                      */;
const integer end_list = -1     /* Marks end of a list                    */;
const integer empty = -3        /* Indicates empty                        */;

typedef varying_string<255> anystr/* General string type                  */;

                                /* One node in parsing table.             */
struct string_table_entry {
                        boolean used        /* Is this node used yet?      */;
                        integer prevchar    /* Code for preceding string   */;
                        integer follchar    /* Code for current character  */;
                        integer next        /* Next dupl in collision list */;
};

untyped_file input_file                   /* Input file   */;
untyped_file output_file                  /* Output file  */;

integer inbufsize                         /* Count of chars in input buffer */;

array<1,maxbuff,byte> input_buffer         /* Input buffer area         */;
array<1,maxbuff,byte> output_buffer        /* Output buffer area        */;

integer input_pos                         /* Cur. pos. in input buffer   */;
integer output_pos                        /* Cur. pos. in output buffer  */;

                                          /* String table */

array<0,maxtab,string_table_entry> string_table;

integer table_used                        /* # string table entries used */;
integer output_code                       /* Output compressed code      */;
integer input_code                        /* Input compressed code       */;
boolean if_compressing                    /* TRUE if compressing file    */;
integer ierr                              /* Input/output error          */;

string header; varying_string<30> describe; byte method;

void terminate()

{     /* Terminate */
                                   /* Write any remaining characters */
                                   /* to output file.                */
   if ( output_pos > 0 ) 
      blockwrite( output_file, output_buffer, output_pos );

   ierr = ioresult;
                                   /* Close input and output files   */
   close( input_file  );
   ierr = ioresult;

   close( output_file );
   ierr = ioresult;

   output << "done." << NL;

}     /* Terminate */

/*--------------------------------------------------------------------------*/
/*          Get_Hash_Code --- Gets hash code for given <w>C string          */
/*--------------------------------------------------------------------------*/

integer get_hash_code( integer prevc, integer follc )

{
   integer index;
   integer index2;

      /* Get_Hash_Code */
                                   /* Get initial index using hashing */

   integer get_hash_code_result;
   index = ( ( prevc << 5 ) ^ follc ) & maxtab;

                                   /* If entry not already used, return */
                                   /* its index as hash code for <w>C.  */

   if ( ! string_table[index].used ) 
      get_hash_code_result = index;
   else
                                   /* If entry already used, search to  */
                                   /* end of list of hash collision     */
                                   /* entries for this hash code.       */
                                   /* Do linear probe to find an        */
                                   /* available slot.                   */
      {

                                   /* Skip to end of collision list ... */

         while ( string_table[index].next != end_list ) 
            index = string_table[index].next;

                                   /* Begin linear probe down a bit from  */
                                   /* last entry in collision list ...    */

         index2 = ( index + 101 ) & maxtab;

                                   /* Look for unused entry using linear  */
                                   /* probing ...                         */

         while ( string_table[index2].used ) 
            index2 = succ(integer, index2 ) & maxtab;

                                   /* Point prior end of collision list   */
                                   /* to this new node.                   */

         string_table[index].next = index2;

                                   /* Return hash code for <w>C           */

         get_hash_code_result          = index2;

      }

   return get_hash_code_result;
}     /* Get_Hash_Code */

/*--------------------------------------------------------------------------*/
/*          Make_Table_Entry --- Enter <w>C string in string table          */
/*--------------------------------------------------------------------------*/

void make_table_entry( integer prevc, integer follc )

{     /* Make_Table_Entry */
                                   /* Only enter string if there is room left */

   if ( table_used <= maxtab ) 
      {
         {
               string_table_entry& with = string_table[ get_hash_code( prevc , follc ) ]; 

               with.used     = true;
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

/*--------------------------------------------------------------------------*/
/*            Initialize_String_Table --- Initialize string table           */
/*--------------------------------------------------------------------------*/

void initialize_string_table()

{
   integer i;

      /* Initialize_String_Table */

                                   /* No entries used in table yet */
   table_used  = 0;
                                   /* Clear all table entries      */
   for( i = 0; i <= maxtab; i ++)
      {
            string_table_entry& with = string_table[i]; 

            with.prevchar = no_prev;
            with.follchar = no_prev;
            with.next     = -1;
            with.used     = false;
      }
                                   /* Enter all single characters into */
                                   /* table                            */
   for( i = 0; i <= 255; i ++)
      make_table_entry( no_prev , i );

}     /* Initialize_String_Table */

/*--------------------------------------------------------------------------*/
/*            Initialize --- Initialize compression/decompression           */
/*--------------------------------------------------------------------------*/

void initialize()

{
   anystr input_name               /* Input file name  */;
   anystr output_name              /* Output file name */;

      /* Initialize */

 output << "Number of file to compress:"; input >> input_name >> NL;
 output << "For the moment, I'm writing the compressed version to v:compr" << 
            input_name << ".avd." << NL;
 output_name=string("d:compr")+input_name+".avd";
  input_name=string("d:place")+input_name+".avd";

 output << "Wait... ";

                                   /* Open input file */

   assign ( input_file , input_name );
   reset  ( input_file , 1 );
   ierr = ioresult;

 blockread(input_file,header,146);
 blockread(input_file,describe,30);
 blockread(input_file,method,1);

 if (method==177) 
 {;
  output << "It's already compressed!" << NL;
  exit(177);
 }

                                   /* Open output file */

   assign ( output_file , output_name );
   rewrite( output_file , 1 );
   ierr = ioresult;

 method=177;

 blockwrite(output_file,header,146);
 blockwrite(output_file,describe,30);
 blockwrite(output_file,method,1);

                                   /* Point input point past end of */
                                   /* buffer to force initial read  */
   input_pos  = maxbuff + 1;
                                   /* Nothing written out yet       */
   output_pos = 0;
                                   /* Nothing read in yet           */
   inbufsize  = 0;
                                   /* No input or output codes yet  */
                                   /* constructed                   */
   output_code = empty;
   input_code  = empty;
                                   /* Initialize string hash table  */
   initialize_string_table();

}     /* Initialize */

/*--------------------------------------------------------------------------*/
/*            Lookup_String --- Look for string <w>C in string table        */
/*--------------------------------------------------------------------------*/

integer lookup_string( integer prevc, integer follc )

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

      found = ( string_table[index].prevchar == prevc ) &&
               ( string_table[index].follchar == follc );

      if ( ! found ) 
         index = string_table[index].next;

   } while (!(found || ( index == end_list )));

                                   /* Return index if <w>C found in table. */
   if (found) 
      lookup_string_result = index;

   return lookup_string_result;
}     /* Lookup_String */

/*--------------------------------------------------------------------------*/
/*              Get_Char  ---  Read character from input file               */
/*--------------------------------------------------------------------------*/

void get_char( integer& c )

{     /* Get_Char */
                                   /* Point to next character in buffer */
   input_pos += 1;
                                   /* If past end of block read in, then */
                                   /* reset input pointer and read in    */
                                   /* next block.                        */

   if ( input_pos > inbufsize ) 
      {
         blockread( input_file, input_buffer, maxbuff, inbufsize );
         input_pos = 1;
         ierr      = ioresult;
      }
                                  /* If end of file hit, return EOF_Char */
                                  /* otherwise return next character in  */
                                  /* input buffer.                       */
   if ( inbufsize == 0 ) 
      c = eof_char;
   else
      c = input_buffer[input_pos];

}     /* Get_Char */

/*--------------------------------------------------------------------------*/
/*             Write_Char  ---  Write character to output file              */
/*--------------------------------------------------------------------------*/

void put_char( integer c )

{     /* Put_Char */
                                   /* If buffer full, write it out and */
                                   /* reset output buffer pointer.     */

   if ( output_pos >= maxbuff ) 
      {
         blockwrite( output_file, output_buffer, maxbuff );
         output_pos = 0;
         ierr       = ioresult;
      }
                                   /* Place character in next slot in  */
                                   /* output buffer.                   */

   output_pos += 1;
   output_buffer[output_pos] = c;

}     /* Put_Char */

/*--------------------------------------------------------------------------*/
/*             Put_Code  ---  Write hash code to output file.               */
/*--------------------------------------------------------------------------*/

void put_code( integer hash_code )

{     /* Put_Code */
                                   /* Output code word is empty.        */
                                   /* Put out 1st 8 bits of compression */
                                   /* code and save last 4 bit for next */
                                   /* time through.                     */

   if ( output_code == empty ) 
      {
         put_char( ( (cardinal)hash_code >> 4 ) & 0xff );
         output_code = hash_code & 0xf;
      }
   else
                                   /* Output code word not empty.         */
                                   /* Put out last 4 bits of previous     */
                                   /* code appended to 1st 4 bits of this */
                                   /* code.  Then put out last 8 bits of  */
                                   /* this code.                          */
      {
         put_char( ( ( output_code << 4 ) & 0xff0 ) +
                   ( ( (cardinal)hash_code >> 8 ) & 0xf ) );
         put_char( hash_code & 0xff );
         output_code = empty;
      }

}     /* Put_Code */

/*--------------------------------------------------------------------------*/
/*             Do_Compression --- Perform Lempel-Ziv-Welch compression      */
/*--------------------------------------------------------------------------*/

void do_compression()

{
   integer c                /* Current input character = C */;
   integer wc               /* Hash code value for <w>C    */;
   integer w                /* Hash code value for <w>     */;

      /* Do_Compression */
                                   /* Read first character ==> Step 2 */
   get_char( c );
                                   /* Initial hash code -- first character */
                                   /* has no previous string (<w> is null) */

   w = lookup_string( no_prev , c );

                                   /* Get next character ==> Step 3    */
   get_char( c );
                                   /* Loop over input characters until */
                                   /* end of file reached ==> Step 4.  */
   while( c != eof_char ) 
      {
                                   /* See if <w>C is in table. */

         wc = lookup_string( w , c );

                                   /* If <w>C is not in the table, */
                                   /* enter it into the table and  */
                                   /* output <w>.  Reset <w> to    */
                                   /* be the code for C ==> Step 6 */

         if ( wc == end_list ) 
            {

               make_table_entry( w , c );
               put_code( w );
               w = lookup_string( no_prev , c );

            }
         else                      /* If <w>C is in table, keep looking */
                                   /* for longer strings == Step 5      */

            w = wc;

                                   /* Get next input character ==> Step 3 */
         get_char( c );

      }
                                   /* Make sure last code is       */
                                   /* written out ==> Step 4.      */
   put_code( w );

}     /* Do_Compression */

/*--------------------------------------------------------------------------*/
/*                     PibCompr --- Main program                            */
/*--------------------------------------------------------------------------*/

int main(int argc, const char* argv[])
{     /* PibCompr */
                                   /* We are doing compression */
   pio_initialize(argc, argv);
   if_compressing = true;
                                   /* Initialize compression   */
   initialize();
                                   /* Perform compression      */
   do_compression();
                                   /* Clean up and exit        */
   terminate();

   return EXIT_SUCCESS;
}     /* PibCompr */

} // End of namespace Avalanche.