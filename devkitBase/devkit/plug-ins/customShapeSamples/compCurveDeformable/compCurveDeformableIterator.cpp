//
//  Copyright 2012 Autodesk, Inc.  All rights reserved.
//
//  Use of this software is subject to the terms of the Autodesk license 
//  agreement provided at the time of installation or download, or which 
//  otherwise accompanies this software in either electronic or hard copy form.   
//
#include "StdAfx.h"

#include "compCurveData.h"
#include "compCurveDeformableIterator.h"
#include "compCurveDeformableShape.h"

//-----------------------------------------------------------------------------
compCurveDeformableIterator::compCurveDeformableIterator (MPointArray *geom, MObjectArray &comps)
	: MPxGeometryIterator((void *)geom, comps), curvePoints(geom)
{
	reset () ;
}

compCurveDeformableIterator::compCurveDeformableIterator (MPointArray *geom, MObject &comps)
	: MPxGeometryIterator((void *)geom, comps), curvePoints(geom)
{
	reset () ;
}

//-----------------------------------------------------------------------------
// Resets the iterator to the start of the components so that another
// pass over them may be made.
void compCurveDeformableIterator::reset () {
	MPxGeometryIterator::reset () ;
	setCurrentPoint (0) ;
	if ( curvePoints != NULL ) {
		int maxVertex =curvePoints->length () ;
		setMaxPoints (maxVertex) ;
	}
}

// Returns the point for the current element in the iteration.
// This is used by the transform tools for positioning the
// manipulator in component mode. It is also used by deformers.
MPoint compCurveDeformableIterator::point () const {
	MPoint pnt ;
	if ( curvePoints != NULL )
		pnt =(*curvePoints) [index ()] ;
	return (pnt) ;
}

// Set the point for the current element in the iteration.
// This is used by deformers.
void compCurveDeformableIterator::setPoint (const MPoint &pnt) const {
	if ( curvePoints != NULL )
		(*curvePoints) [index ()] =pnt ;
}

// Return the number of vertices in the iteration.
// This is used by deformers such as smooth skinning
int compCurveDeformableIterator::iteratorCount () const {
	return (curvePoints->length ()) ;
}

// Returns true since the shape data has points.
bool compCurveDeformableIterator::hasPoints () const {
	return (true) ;
}
