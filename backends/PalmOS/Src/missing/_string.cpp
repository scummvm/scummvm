#include "string.h"

Char *StrTokNext;
///////////////////////////////////////////////////////////////////////////////
Char *strtok(Char *str, const Char *sep)
{
	Char	*position = NULL,
			*found,
			*end;

	UInt16	loop = 0,
			chars= StrLen(sep);

	str			= (str)?(str):(StrTokNext);
	StrTokNext	= NULL;

	if (!str)
		return NULL;

	end = str+StrLen(str);

	while (loop<chars)
	{
		found = StrChr(str,sep[loop]);
		loop++;

		if (found == str)
		{
			str++;
			loop = 0;
		}
		else if (position == NULL || position > found)
			position = found;
	}

	if (position == NULL)
		if (str==end)
			return NULL;
		else
			return str;

	position[0] = 0;
	StrTokNext	= position+1;

	return str;
}
///////////////////////////////////////////////////////////////////////////////
Char *strpbrk(const Char *s1, const Char *s2)
{
	Char *found;
	UInt32 n;
	
	for (n=0; n <= StrLen(s2); n++) {
		found = StrChr(s1, s2[n]);
		if (found)
			return found;
	}
	
	return NULL;
}
///////////////////////////////////////////////////////////////////////////////
Char *strrchr(const Char *s, int c)
{
	UInt32 chr;
	UInt32 n = StrLen(s);

	for(chr = n; chr >= 0; chr--)
		if ( *((UInt8 *)s+chr) == c)
			return (Char *)(s+chr);

	return NULL;
} 
///////////////////////////////////////////////////////////////////////////////
Char *strdup(const Char *s1)
{
	Char* buf = (Char *)MemPtrNew(StrLen(s1)+1);

	if(buf)
		StrCopy(buf, s1);

	return buf;
}
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
