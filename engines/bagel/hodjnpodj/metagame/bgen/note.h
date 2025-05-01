/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef HODJNPODJ_METAGAME_BGEN_NOTE_H
#define HODJNPODJ_METAGAME_BGEN_NOTE_H

#include "bagel/afxwin.h"
#include "bagel/hodjnpodj/metagame/bgen/notelist.h"

namespace Bagel {
namespace HodjNPodj {
namespace Metagame {

#define NOTE_REPEAT_MAX	8


class CNote : public CObject
{
	DECLARE_DYNCREATE(CNote)

// Constructors
public:
	CNote(int nID = -1, int nClue = -1, int nRepeat = 1, int nPerson = -1, int nPlace = -1);

// Destructors
public:
	~CNote();

// Implementation
public:
	int GetID(void)
		{ return(m_nID); }
	void SetID(int nID)
		{ m_nID = nID; }

	int GetClueID(void)
		{ return(m_nClueID); }
	void SetClueID(int nClue)
		{ m_nClueID = nClue; }

	int GetRepeatCount(void)
		{ return(m_nRepeatCount); }
 	void SetRepeatCount(int nRepeat)
		{ m_nRepeatCount = nRepeat; }

	int GetPersonID(void)
		{ return(m_nPersonID); }
	void SetPersonID(int nPerson)
		{ m_nPersonID = nPerson; }

	int GetPlaceID(void)
		{ return(m_nPlaceID); }
	void SetPlaceID(int nPlace)
		{ m_nPlaceID = nPlace; }

#ifndef FRAME_EXE
	char * GetClueArtSpec(void)
		{ if ((m_nClueID >= NOTE_ICON_BASE) &&
			  (m_nClueID < NOTE_ICON_BASE + NOTE_ICON_COUNT))
			  return(m_pNoteBitmapPath[m_nClueID]);
		  else
		      return(NULL);
		}

	char * GetPersonArtSpec(void)
		{ if ((m_nPersonID >= NOTE_ICON_BASE) &&
			  (m_nPersonID < NOTE_ICON_BASE + NOTE_ICON_COUNT))
			  return(m_pNoteBitmapPath[m_nPersonID]);
		  else
		      return(NULL);
		}

	char * GetPlaceArtSpec(void)
		{ if ((m_nPlaceID >= NOTE_ICON_BASE) &&
			  (m_nPlaceID < NOTE_ICON_BASE + NOTE_ICON_COUNT))
			  return(m_pNoteBitmapPath[m_nPlaceID]);
		  else
		      return(NULL);
		}
		
	char * GetDescription(void)
		{ if ((m_nID >= 0) &&
			  (m_nID < NOTE_COUNT))
			  return(m_pNoteText[m_nID]);
		  else
		      return(NULL);
		}
	
	char * GetPersonSoundSpec(void)
		{ if ((m_nPersonID >= NOTE_ICON_BASE) &&
			  (m_nPersonID < NOTE_ICON_BASE + NOTE_ICON_COUNT))
			  return(m_pNoteSoundPath[m_nPersonID]);
		  else
		      return(NULL);
		}

	char * GetPlaceSoundSpec(void)
		{ if ((m_nPlaceID >= NOTE_ICON_BASE) &&
			  (m_nPlaceID < NOTE_ICON_BASE + NOTE_ICON_COUNT))
			  return(m_pNoteSoundPath[m_nPlaceID]);
		  else
		      return(NULL);
		}
#endif

	CNote *	GetNext(void)
		{ return(m_pNext); }
	CNote * GetPrev(void)
		{ return(m_pPrev); }


static char * GetDescription(int nID, int nQuantity);

		  
private:
	int		m_nID;					// note identifier code
	int		m_nPersonID;			// identifier of person giving note 
	int		m_nPlaceID;				// identifier for place note given
	int		m_nClueID;				// identifier for clue book entry
	int		m_nRepeatCount;			// number of times to repeat clue image

public:
	CNote	*m_pNext;				// pointer to next note in the list
	CNote	*m_pPrev;				// pointer to previous note in the list

private:

#ifndef FRAME_EXE
static char	*m_pNoteText[NOTE_COUNT];	// descriptive text for each note
static char	*m_pNoteSoundPath[NOTE_ICON_COUNT];	// sound file specifications for each note
static char	*m_pNoteBitmapPath[NOTE_ICON_COUNT];// bitmap file specifications for each note
#endif

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

};

} // namespace Metagame
} // namespace HodjNPodj
} // namespace Bagel

#endif
