/*************************************************************************
 *
 * note.cpp - CNote class implementation file
 *
 * Version 1.0	9-March-1994
 *
 * Edit History
 *
 *	1.0	  3/9/94	BAR		started
 *
 * 
 * Derivation:			Derived from the Foundation Class "CObject"
 * Constructors:
 *
 *	CNote();			create a note object
 *
 * Destructors:
 *
 *	~CNote();			destroy a note object and release its resources
 *
 * Public:
 *
 *	SetValue         	set the new id for a note (no clicking is done)
 *	GetValue        	get the value of a note (which button it represents)
 *	LinkNote			place note at tail of list
 *	UnLinkNote			place note at tail of list
 *	GetNextNote       	get the next note in the note list
 *	GetPrevNote       	get the previous note in the note list
 *
 * Public Global:
 *
 *	GetNoteHead			get the first note in the note list
 *	FlushNoteList	    destroy all notes in the list and release resources
 *
 ************************************************************************/

#include "stdafx.h"

#include "note.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

CNote	*CNote::m_pNoteHead = NULL;            // pointer to list of linked notes
CNote	*CNote::m_pNoteTail = NULL;            // pointer to tail of list of linked notes

IMPLEMENT_DYNCREATE(CNote, CObject)

/*************************************************************************
 *
 * CNote()
 *
 * Parameters:		none
 *
 * Return Value:	none
 *
 * Description:		Constructor for note class.
 *
 ************************************************************************/

CNote::CNote()
{ 
m_nValue = 1;
m_pNext = NULL;										// it is not yet in the sprite chain and
m_pPrev = NULL;                                     // ... thus has no links to other sprites
}


/*************************************************************************
 *
 * ~CNote()
 *
 * Parameters:		none
 *
 * Return Value:	none
 *
 * Description:		Destructor for note class.
 *
 ************************************************************************/

CNote::~CNote()
{
}


/*************************************************************************
 *
 * SetValue()
 *
 * Parameters:
 *	int	nValue		value which identifies the button associated with the note
 *
 * Return Value:	none
 *
 * Description:		Identify the button to be played at this point in the sequence
 *
 ************************************************************************/

void CNote::SetValue(int nValue)
{
	m_nValue = nValue;					// identify the button to be played
}

/*************************************************************************
 *
 * LinkNote()
 *
 * Parameters:		none
 *
 * Return Value:	none
 *
 * Description:		Link this note into the list by placing it at the
 *					the tail of the list
 *
 ************************************************************************/

void CNote::LinkNote(void)
{
	m_pNext = NULL;					// link note onto tail of list
	m_pPrev = m_pNoteTail;          //... by pointing it back at the current
                                    //... tail, making it the tail, and
	if (m_pNoteTail)                //... pointing it at NULL (the list terminator)
		(*m_pNoteTail).m_pNext = this;
	else
		m_pNoteHead = this;
	m_pNoteTail = this;
}

/*************************************************************************
 *
 * UnLinkNote()
 *
 * Parameters:		none
 *
 * Return Value:	none
 *
 * Description:		Remove this sprite from the sprite chain and point its
 *					neighbors at each other to fill the gap
 *
 ************************************************************************/

void CNote::UnLinkNote(void)
{
	if (m_pPrev)							// disconnect us from the note chain
		(*m_pPrev).m_pNext = m_pNext;       // ... by pointing the one before us, and
	else                                    // ... the one after us, at each other
		m_pNoteHead = m_pNext;              // special case the instances where the 
                                            // ... note to be removed is the first
	if (m_pNext)                            // ...  or the last in the list, update
		(*m_pNext).m_pPrev = m_pPrev;       // ... the head of chain pointer 
	else
		m_pNoteTail = m_pPrev;
    
	m_pNext = m_pPrev = NULL;
}

/*************************************************************************
 *
 * FlushNoteList()
 *
 * Parameters:		none
 *
 * Return Value:	none
 *
 * Description:		Remove all notes from the chain and delete them via
 *					the standard destructor
 *
 ************************************************************************/

void CNote::FlushNoteList(void)
{
	CNote	*pNote;

	while (pNote = CNote::GetNoteHead()) {
		(*pNote).UnLinkNote();
		delete pNote;
	}
}


/////////////////////////////////////////////////////////////////////////////
// CNote diagnostics

#ifdef _DEBUG
void CNote::AssertValid() const
{
	CObject::AssertValid();
}

void CNote::Dump(CDumpContext& dc) const
{
	CObject::Dump(dc);
}

#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
