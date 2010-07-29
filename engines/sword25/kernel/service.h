// -----------------------------------------------------------------------------
// This file is part of Broken Sword 2.5
// Copyright (c) Malte Thiesen, Daniel Queteschiner and Michael Elsdörfer
//
// Broken Sword 2.5 is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// Broken Sword 2.5 is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Broken Sword 2.5; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA
// -----------------------------------------------------------------------------

/*
	BS_Service
	-------------
	Dies ist die Basisklasse für alle Services der Engine. 
	Ein Service ist ein wesentlicher Bestandteil des Engine, z.B. die Graphiksystem.
	Das Servicesystem macht es möglich mehrere verschiedene Services für ein System zu haben,
	und je nach Betriebssystem einen passenden auszuwählen.
	Denkbar wären z.B. zwei Graphiksysteme von denen eines hardwarebeschleunigt ist und ein
	anderes nicht.
	Die Services werden zur Laufzeit über die Kernelmethode NewService und NIEMALS mit new erzeugt.

	Autor: Malte Thiesen
*/

#ifndef _BS_SERVICE_H
#define _BS_SERVICE_H

// Includes
#include "common.h"

// Klassendefinition
class BS_Kernel;

class BS_Service
{
private:
	BS_Kernel*	_pKernel;
		
protected:
	BS_Service(BS_Kernel* pKernel) : _pKernel(pKernel) {};
	
	BS_Kernel* GetKernel() const { return _pKernel; }
	
public:
	virtual ~BS_Service(){};
};

#endif