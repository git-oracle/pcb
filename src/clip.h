/*
 *                            COPYRIGHT
 *
 *  PCB, interactive printed circuit board design
 *  Copyright (C) 1994,1995,1996 Thomas Nau
 *  Copyright (C) 2004 harry eaton
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 *  Contact addresses for paper mail and Email:
 *  Thomas Nau, Schlehenweg 15, 88471 Baustetten, Germany
 *  Thomas.Nau@rz.uni-ulm.de
 *
 *  RCS: $Id$
 */

/* prototypes for inserting points into objects
 */

#ifndef	__CLIP_INCLUDED__
#define	__CLIP_INCLUDED__

#include "global.h"

/* ---------------------------------------------------------------------------
 * prototypes
 */
void XDrawCLine (GdkDrawable *, GdkGC *, int, int, int, int);
void XDrawCArc (GdkDrawable *, GdkGC *, int, int, unsigned int, unsigned int,
		int, int);
void DrawCPolygon (GdkDrawable *, PolygonTypePtr);
extern LocationType dxo, dyo;
extern Boolean SwapOutput;
extern float Local_Zoom;
#endif
