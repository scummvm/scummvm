#include	<general.h>
#include	<alloc.h>
#include	<dos.h>

#define		EMS_ADR(a)	(FP_SEG(a) > 0xA000)
#define		HNODE_OK(p)	(heapchecknode(p)==4)



MEM_TYPE MemType (void far * mem)
{
  if (FP_SEG(mem) == _DS)
    {
      if (heapchecknode((void *)mem)==4) return NEAR_MEM;
    }
  else
    {
      if (FP_SEG(mem) > 0xA000) return EMS_MEM;
      else
	{
	  if (farheapchecknode(mem)==4) return FAR_MEM;
	}
    }
  return BAD_MEM;
}
