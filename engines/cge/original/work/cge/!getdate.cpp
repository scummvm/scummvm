      {
	union { dosdate_t d; dword n; } D;
	_dos_getdate(&D.d);
	Summa = *(dword *) (Copr + strlen(copr) + 1) < D.n;
      }
