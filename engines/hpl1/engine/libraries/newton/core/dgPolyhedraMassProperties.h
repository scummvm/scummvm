/* Copyright (c) <2003-2011> <Julio Jerez, Newton Game Dynamics>
* 
* This software is provided 'as-is', without any express or implied
* warranty. In no event will the authors be held liable for any damages
* arising from the use of this software.
* 
* Permission is granted to anyone to use this software for any purpose,
* including commercial applications, and to alter it and redistribute it
* freely, subject to the following restrictions:
* 
* 1. The origin of this software must not be misrepresented; you must not
* claim that you wrote the original software. If you use this software
* in a product, an acknowledgment in the product documentation would be
* appreciated but is not required.
* 
* 2. Altered source versions must be plainly marked as such, and must not be
* misrepresented as being the original software.
* 
* 3. This notice may not be removed or altered from any source distribution.
*/

#ifndef __dgPolyhedraMassProperties__
#define __dgPolyhedraMassProperties__

class dgPolyhedraMassProperties
{
	public:
	dgPolyhedraMassProperties();

	void AddCGFace (dgInt32 indexCount, const dgVector* faceVertex);
	void AddInertiaFace (dgInt32 indexCount, const dgFloat32* faceVertex);
	void AddInertiaAndCrossFace (dgInt32 indexCount, const dgFloat32* faceVertex);
	
	dgFloat32 MassProperties (dgVector& cg, dgVector& inertia, dgVector& crossInertia);

	private:
	

	dgFloat32 intg[10];
	static dgFloat32 mult[10];

};

#endif
