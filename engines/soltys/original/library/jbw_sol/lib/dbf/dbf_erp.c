#include	"dbf_inc.h"



char * PolErrMsg (Errors err)
{
  switch(err)
    {
      case NoBaseErr    : return "Nie otwarto bazy";
      case RangeErr     : return "¡†danie operacji poza zakresem";
      case NoCoreErr    : return "Brak pami‘ci";
      case NoWorkErr    : return "Zaj‘te wszystkie obszary robocze";
      case NoFileErr    : return "Brak podanej bazy";
      case BadFileErr   : return "Z’y format bazy";
      case FRdOnlyErr   : return "Brak praw zapisu bazy";
      case FOpenErr     : return "B’†d otwarcia bazy";
      case FSeekErr     : return "B’†d pozycjonowania bazy";
      case FReadErr     : return "B’†d odczytu bazy";
      case FWriteErr    : return "B’†d zapisu bazy";
      case FCloseErr    : return "B’†d zamkni‘cia bazy";
      case NoIxErr      : return "Indeks nie jest aktywny";
      case NoIxFileErr  : return "Brak podanego indeksu";
      case BadIxFileErr : return "Z’y format indeksu";
      case IxRdOnlyErr  : return "Brak praw zapisu indeksu";
      case IxOpenErr    : return "B’†d otwarcia indeksu";
      case IxSeekErr    : return "B’†d pozycjonowania indeksu";
      case IxReadErr    : return "B’†d odczytu indeksu";
      case IxWriteErr   : return "B’†d zapisu indeksu";
      case IxCloseErr   : return "B’†d zamkni‘cia indeksu";
      case ShareErr     : return "Nie zainstalowano SHARE";
      case LockErr      : return "Nieskuteczna pr¢ba uzyskania wy’†cznožci";
      default           : return "Niezidentyfikowany b’†d bazy danych";
    }
}
