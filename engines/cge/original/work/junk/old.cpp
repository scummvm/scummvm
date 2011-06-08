/*
static void NumShow (word b, SPRITE *spr[], int radix, int size)
{
  int i;
  for (i = 0; i < size; i ++)
    {
      spr[i]->Phase = b % radix;
      b /= radix;
    }
}
*/




/*
static word VgaRegVal (VgaRegBlk * vrb)
{
  asm	push	di
  asm	les	di,vrb		// take address of parameter table
  asm	mov	dx,VGAST1_	// read from this I/O address...
  asm	in	al,dx		// ...enables access to another registers

  asm	mov	ax,es:[di]	// take lower byte of I/O address and index
  asm	mov	dl,al		// complete I/O read address - single
  asm	or	al,al		// indexed register?
  asm	js	read		// continue standard job

  asm	mov	dl,ah		// complete I/O read address - indexed
  asm	out	dx,al		// put index into control register
  asm	inc	dx		// data register is next to control

  read:
  asm	in	al,dx		// take data byte
  asm	xor	ah,ah
  asm	pop	di

  return _AX;
}
*/





/*
static void MakeScrBak (const char * fn)
{
  BMP * bmp = new BMP(fn);
  byte * d;
  int i;

  if (BakShp == NULL) BakShp = new byte[SCR_HIG*SCR_WID];
  if (BakShp == NULL) exit(1);
  d = BakShp;

  for (i = 0; i < 4; i ++)
    {
      byte * s = bmp->Map() + i;
      word n;

      for (n = 0; n < (SCR_HIG*SCR_WID)/4; n ++)
	{
	  *d = *s;
	  ++ d;
	  s += 4;
	}
    }
  delete bmp;
}
*/
