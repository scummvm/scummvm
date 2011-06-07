SHP::SHP (BMP * bmp, int x, int y, int w, int h)
{
  word i, j, mw = bmp->Bitmap.W, mh = bmp->Bitmap.H;
  byte * im;
  int bpl;
  word * cp = (word *) (map = new byte[(w + 16) * h + 8]);

  for (bpl = 0; bpl < 4; bpl ++)
    {
      byte * bm = bmp->Bitmap.M + mw * (mh-1-y) + x;
      for (i = 0; i < h; i ++)
	{
	  *cp = CPY + w / 4;
	  im = (byte *) (cp+1);
	  for (j = bpl; j < w; j += 4) * (im ++) = bm[j];
	  cp = (word *) im;
	  * (cp ++) = SKP + (SCR_WID - j + 3) / 4;
	  bm -= mw;
	}
      * (cp ++) = EOI;
    }
  W = w;
  H = h;
}






