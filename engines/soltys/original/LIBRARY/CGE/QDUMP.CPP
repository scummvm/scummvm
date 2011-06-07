#ifdef DEBUG
void SNTest (SPRITE * spr)
{
  char txt[256];
  int i;

  StdLog("-------- begin of queue dump ---------", "");
  for (; spr; spr = spr->Next)
    {
      sprintf(txt, "%5d %p [%-8s] w%03d h%03d z%03d",
		   spr->Ref, spr, spr->File, spr->W, spr->H, spr->Z);
      StdLog(txt, spr->Name);
    }
  StdLog("--------- end of queue dump ----------", "");
  StdLog("------- begin of palette dump --------", "");
  for (i = 0; i < 256; i ++)
    {
      sprintf(txt, "[%3d]     %3dr   %3dg   %3db", i,
	      SysPal[i].R, SysPal[i].G, SysPal[i].B);
      StdLog(txt, "");
    }
  StdLog("-------- end of palette dump ---------", "");
}
#endif
