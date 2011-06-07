int InitHook (int drv)
{
  return drv < 26;
}


int CopyHook (char c)
{
  return c;
}
