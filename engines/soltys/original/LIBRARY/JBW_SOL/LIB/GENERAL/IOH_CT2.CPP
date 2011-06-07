#include	<general.h>





IOHAND::IOHAND (IOMODE mode, CRYPT * crpt)
: XFILE(mode), Handle(-1), Crypt(crpt), Seed(SEED)
{
}

