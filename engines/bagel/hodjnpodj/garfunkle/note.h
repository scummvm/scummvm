// note.h : header file
//
// Version 1.0	9-March-1994
//

/////////////////////////////////////////////////////////////////////////////
// CNote Class		Derived from the CObject Foundation Class


#include "dibapi.h"

class CNote : public CObject
{
	DECLARE_DYNCREATE(CNote)

// Constructors
public:
	CNote();	// use "new" operator to create notes, then SetValue

// Destructors
public:
	~CNote();

// Implementation
public:
	void	SetValue(int nValue);
	int	GetValue(void) { return(m_nValue); } 
	void LinkNote(void);
	void UnLinkNote(void);
	CNote * GetNextNote(void) { return(m_pNext); }
	CNote * GetPrevNote(void) { return(m_pPrev); }
	
static CNote * GetNoteHead(void) { return(m_pNoteHead); }
static void FlushNoteList(void);

private:
	int			m_nValue;
	CNote		*m_pNext;			// pointer to next note in chain
	CNote		*m_pPrev;			// pointer to previous note in chain

static CNote	*m_pNoteHead;	// pointer to linked chain of notes
static CNote	*m_pNoteTail;	// pointer to tail of list of notes

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

};

/////////////////////////////////////////////////////////////////////////////
