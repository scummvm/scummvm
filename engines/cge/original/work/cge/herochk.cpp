static void HeroCheck (int mode)
{
  static BMP_PTR * tab;
  static word chk[200];
  int i;

  switch (mode)
    {
      case 0 : int nr = -1; char t[80];
	       for (i = 0; tab[i]; i ++)
	       //for (i = 19; i == 19; i ++)
		 {
		   BMP_PTR b = tab[i];
		   if (chk[i] != ChkSum(b->V, (word) ((byte far *) (b->B + b->H) - b->V)))
		     {
		       sprintf(t, "%p, %d", tab[i], (word) ((byte far *) (b->B + b->H) - b->V));
		       StdLog(NumStr("Corrupted Hero shape ######", i), t);
		       nr = i;
		     }
		 }
	       if (nr >= 0)
		 {
		   BMP_PTR b = tab[nr];
		   char t[80];
		   sprintf(t, "%p, %d", tab[nr], (word) ((byte far *) (b->B + b->H) - b->V));
		   VGA::Exit(NumStr("Corrupted Hero shape ######", nr), t);
		 }
	       break;
      case 1 : tab = Hero->SetShapeList(NULL);
	       Hero->SetShapeList(tab);
	       for (i = 0; tab[i]; i ++)
		 {
		   BMP_PTR b = tab[i];
		   chk[i] = ChkSum(b->V, (word) ((byte far *) (b->B + b->H) - b->V));
		 }
	       break;
    }
}



