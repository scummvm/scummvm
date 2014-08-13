/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

/*
 * This file is based on, or a modified version of code from TinyGL (C) 1997-1998 Fabrice Bellard,
 * which is licensed under the zlib-license (see LICENSE).
 * It also has modifications by the ResidualVM-team, which are covered under the GPLv2 (or later).
 */


ADD_OP(Color, 7, "%f %f %f %f %d %d %d")
ADD_OP(TexCoord, 4, "%f %f %f %f")
ADD_OP(EdgeFlag, 1, "%d")
ADD_OP(Normal, 3, "%f %f %f")

ADD_OP(Begin, 1, "%C")
ADD_OP(Vertex, 4, "%f %f %f %f")
ADD_OP(End, 0, "")

ADD_OP(EnableDisable, 2, "%C %d")

ADD_OP(MatrixMode, 1, "%C")
ADD_OP(LoadMatrix, 16, "")
ADD_OP(LoadIdentity, 0, "")
ADD_OP(MultMatrix, 16, "")
ADD_OP(PushMatrix, 0, "")
ADD_OP(PopMatrix, 0, "")
ADD_OP(Rotate, 4, "%f %f %f %f")
ADD_OP(Translate, 3, "%f %f %f")
ADD_OP(Scale, 3, "%f %f %f")
ADD_OP(Ortho, 6, "%f %f %f %f %f %f")

ADD_OP(Viewport, 4, "%d %d %d %d")
ADD_OP(Frustum, 6, "%f %f %f %f %f %f")

ADD_OP(Material, 6, "%C %C %f %f %f %f")
ADD_OP(ColorMaterial, 2, "%C %C")
ADD_OP(Light, 6, "%C %C %f %f %f %f")
ADD_OP(LightModel, 5, "%C %f %f %f %f")

ADD_OP(Clear, 1, "%d")
ADD_OP(ClearColor, 4, "%f %f %f %f")
ADD_OP(ClearDepth, 1, "%f")

ADD_OP(InitNames, 0, "")
ADD_OP(PushName, 1, "%d")
ADD_OP(PopName, 0, "")
ADD_OP(LoadName, 1, "%d")

ADD_OP(TexImage2D, 9, "%d %d %d %d %d %d %d %d %d")
ADD_OP(BindTexture, 2, "%C %d")
ADD_OP(TexEnv, 7, "%C %C %C %f %f %f %f")
ADD_OP(TexParameter, 7, "%C %C %C %f %f %f %f")
ADD_OP(PixelStore, 2, "%C %C")

ADD_OP(ShadeModel, 1, "%C")
ADD_OP(CullFace, 1, "%C")
ADD_OP(FrontFace, 1, "%C")
ADD_OP(PolygonMode, 2, "%C %C")
ADD_OP(ColorMask, 1, "%08x")
ADD_OP(DepthMask, 1, "%d")
ADD_OP(BlendFunc, 2, "%d %d")
ADD_OP(AlphaFunc, 2, "%d %f")
ADD_OP(DepthFunc, 1, "%d")

ADD_OP(CallList, 1, "%d")
ADD_OP(Hint, 2, "%C %C")

// special opcodes
ADD_OP(EndList, 0, "")
ADD_OP(NextBuffer, 1, "%p")

// opengl 1.1 arrays
ADD_OP(ArrayElement, 1, "%d")
ADD_OP(EnableClientState, 1, "%C")
ADD_OP(DisableClientState, 1, "%C")
ADD_OP(VertexPointer, 4, "%d %C %d %p")
ADD_OP(ColorPointer, 4, "%d %C %d %p")
ADD_OP(NormalPointer, 3, "%C %d %p")
ADD_OP(TexCoordPointer, 4, "%d %C %d %p")

// opengl 1.1 polygon offset
ADD_OP(PolygonOffset, 2, "%f %f")

#undef ADD_OP
