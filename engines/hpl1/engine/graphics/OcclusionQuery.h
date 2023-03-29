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

/*
 * Copyright (C) 2006-2010 - Frictional Games
 *
 * This file is part of HPL1 Engine.
 */

#ifndef HPL_OCCLUSION_QUERY_H
#define HPL_OCCLUSION_QUERY_H

namespace hpl {

class iOcclusionQuery {
public:
	virtual ~iOcclusionQuery() {}

	/**
	 * Starts the query. Must be followed by an End().
	 */
	virtual void Begin() = 0;
	/**
	 * Ends the current query. Must have been started with a Begin()
	 */
	virtual void End() = 0;

	/**
	 * Fetches the result of he query. Should not be done until all is rendered. (Is called internally by Renderer3D)
	 */
	virtual bool FetchResults() = 0;

	/**
	 * Get the number of samples that passed stencil and depth test in the query begin/end.
	 */
	virtual unsigned int GetSampleCount() = 0;
};

} // namespace hpl

#endif // HPL_OCCLUSION_QUERY_H
