//
//  Copyright 2012 Autodesk, Inc.  All rights reserved.
//
//  Use of this software is subject to the terms of the Autodesk license 
//  agreement provided at the time of installation or download, or which 
//  otherwise accompanies this software in either electronic or hard copy form.   
//
#include "StdAfx.h"

#include "compCurveSimpHistIterator.h"
#include "compCurveSimpHistShape.h"

//-----------------------------------------------------------------------------
compCurveSimpHistIterator::compCurveSimpHistIterator (compCurveSimpHist *geom, MObjectArray &comps)
	: MPxGeometryIterator((void *)geom, comps), curve(geom)
{
	reset () ;
}

compCurveSimpHistIterator::compCurveSimpHistIterator (compCurveSimpHist *geom, MObject &comps)
	: MPxGeometryIterator((void *)geom, comps), curve(geom)
{
	reset () ;
}

//-----------------------------------------------------------------------------
// Resets the iterator to the start of the components so that another
// pass over them may be made.
void compCurveSimpHistIterator::reset () {
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
MPoint compCurveSimpHistIterator::point () const {
	MPoint pnt ;
	if ( curve != NULL )
		pnt =curve->point (index ()) ;
	return (pnt) ;
}

// Set the point for the current element in the iteration.
// This is used by deformers.
void compCurveSimpHistIterator::setPoint (const MPoint &pnt) const {
	if ( curve != NULL )
		curve->setPoint (index (), pnt) ;
}

// Return the number of vertices in the iteration.
// This is used by deformers such as smooth skinning
int compCurveSimpHistIterator::iteratorCount () const {
	return (curve->pointCount ()) ;
}

// Returns true since the shape data has points.
bool compCurveSimpHistIterator::hasPoints () const {
	return (true) ;
}
