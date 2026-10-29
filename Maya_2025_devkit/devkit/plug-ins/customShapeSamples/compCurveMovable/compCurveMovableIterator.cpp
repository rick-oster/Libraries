//
//  Copyright 2012 Autodesk, Inc.  All rights reserved.
//
//  Use of this software is subject to the terms of the Autodesk license 
//  agreement provided at the time of installation or download, or which 
//  otherwise accompanies this software in either electronic or hard copy form.   
//
#include "StdAfx.h"

#include "compCurveMovableIterator.h"
#include "compCurveMovableShape.h"

//-----------------------------------------------------------------------------
compCurveMovableIterator::compCurveMovableIterator (compCurveMovable *geom, MObjectArray &comps)
	: MPxGeometryIterator((void *)geom, comps), curve(geom)
{
	reset () ;
}

compCurveMovableIterator::compCurveMovableIterator (compCurveMovable *geom, MObject &comps)
	: MPxGeometryIterator((void *)geom, comps), curve(geom)
{
	reset () ;
}

//-----------------------------------------------------------------------------
// Resets the iterator to the start of the components so that another
// pass over them may be made.
void compCurveMovableIterator::reset () {
	MPxGeometryIterator::reset () ;
	setCurrentPoint (0) ;
	if ( curve != NULL ) {
		int maxVertex =curve->pointCount () ;
		setMaxPoints (maxVertex) ;
	}
}

// Returns the point for the current element in the iteration.
// This is used by the transform tools for positioning the
// manipulator in component mode. It is also used by deformers.
MPoint compCurveMovableIterator::point () const {
	MPoint pnt ;
	if ( curve != NULL )
		pnt =curve->point (index ()) ;
	return (pnt) ;
}

// Set the point for the current element in the iteration.
// This is used by deformers.
void compCurveMovableIterator::setPoint (const MPoint &pnt) const {
	if ( curve != NULL )
		curve->setPoint (index (), pnt) ;
}

// Return the number of vertices in the iteration.
// This is used by deformers such as smooth skinning
int compCurveMovableIterator::iteratorCount () const {
	return (curve->pointCount ()) ;
}

// Returns true since the shape data has points.
bool compCurveMovableIterator::hasPoints () const {
	return (true) ;
}
