void VGA::Flash (int dly)
{
  DAC old[256], tmp[256];
  int i;

  GetColors(old);
  memcpy(tmp, old, sizeof(tmp));

  for (i = 0; i < ArrayCount(tmp); i ++)
    {
      register int c;
      c = tmp[i].R << 1; tmp[i].R = (c < 64) ? c : 63;
      c = tmp[i].G << 1; tmp[i].G = (c < 64) ? c : 63;
      c = tmp[i].B << 1; tmp[i].B = (c < 64) ? c : 63;
    }
  do SetColors(tmp, 64); while (--dly > 0);
  SetColors(old, 64);
}
