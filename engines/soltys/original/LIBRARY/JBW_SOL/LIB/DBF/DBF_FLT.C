#include	"dbf_inc.h"



Boolean		DbfBof		(void)
{
  register DbfFilter * f = WorkPtr->Filter;
  return (f == NULL) ? _DbfBof() : f->Bof();
}

Boolean		DbfGoTop	(void)
{
  register DbfFilter * f = WorkPtr->Filter;
  return (f == NULL) ? _DbfGoTop() : f->GoTop();
}

Boolean		DbfSkip		(int amount)
{
  register DbfFilter * f = WorkPtr->Filter;
  return (f == NULL) ? _DbfSkip(amount) : f->Skip(amount);
}

Boolean		DbfGoBottom	(void)
{
  register DbfFilter * f = WorkPtr->Filter;
  return (f == NULL) ? _DbfGoBottom() : f->GoBottom();
}

Boolean 	DbfEof		(void)
{
  register DbfFilter * f = WorkPtr->Filter;
  return (f == NULL) ? _DbfEof() : f->Eof();
}
