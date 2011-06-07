#include	"dbf_inc.h"



char * ErrMsg (Errors err)
{
  switch(err)
    {
      case NoBaseErr    : return "No database in use";
      case RangeErr     : return "Request out of range";
      case NoCoreErr    : return "Out of memory";
      case NoWorkErr    : return "No more work areas";
      case NoFileErr    : return "No such database file";
      case BadFileErr   : return "Bad database file format";
      case FRdOnlyErr   : return "No write permission for database";
      case FOpenErr     : return "Database open error";
      case FSeekErr     : return "Database seek error";
      case FReadErr     : return "Database read error";
      case FWriteErr    : return "Database write error";
      case FCloseErr    : return "Database close error";
      case NoIxErr      : return "No index currently in use";
      case NoIxFileErr  : return "No such index file";
      case BadIxFileErr : return "Bad index file";
      case IxRdOnlyErr  : return "No write permission for index";
      case IxOpenErr    : return "Index open error";
      case IxSeekErr    : return "Index seek error";
      case IxReadErr    : return "Index read error";
      case IxWriteErr   : return "Index write error";
      case IxCloseErr   : return "Index close error";
      case ShareErr     : return "SHARE not loaded";
      case LockErr      : return "File lock failed";
      default           : return "Unrecognized database error";
    }
}
