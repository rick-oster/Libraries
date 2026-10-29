//
//  Copyright 2012 Autodesk, Inc.  All rights reserved.
//
//  Use of this software is subject to the terms of the Autodesk license 
//  agreement provided at the time of installation or download, or which 
//  otherwise accompanies this software in either electronic or hard copy form.   
//
#include "StdAfx.h"

#include <math.h>           

#include "curveDrawSelShape.h"
#include "curveDrawSelShapeUI.h"
#include "api_macros.h"

//-----------------------------------------------------------------------------
MTypeId curveDrawSel::id (0x8701F) ;

//-----------------------------------------------------------------------------
// Curve Points
static float sCurve [] [3] ={
	{ 0.0f, 0.0f, 0.0f },
	{ 1.0f, 1.0f, 0.0f },
	{ 2.0f, 1.0f, 0.0f },
	{ 3.0f, 0.0f, 0.0f }
} ; 

static unsigned sCurvePointCount =4 ;

//-----------------------------------------------------------------------------
curveDrawSel::curveDrawSel () {
	basicCurvePoints.setLength (sCurvePointCount) ;
	for ( unsigned ii =0 ; ii < sCurvePointCount; ii++ ) {
		basicCurvePoints [ii].x =sCurve [ii] [0] ;
		basicCurvePoints [ii].y =sCurve [ii] [1] ;
		basicCurvePoints [ii].z =sCurve [ii] [2] ;
		basicCurvePoints [ii].w =1.0;
	}
}

curveDrawSel::~curveDrawSel () {
}

// Overrides

// Specifies that this object has a boundingBox.
bool curveDrawSel::isBounded () const { 
	return (true) ;
}

// Returns the bounding box for this object.
// It is a good idea not to recompute here as this funcion is called often.
MBoundingBox curveDrawSel::boundingBox () const {
	MPoint corner1 (0.0, 0.0, 0.0) ;
	MPoint corner2 (3.0, 1.0, 0.0) ;
	return (MBoundingBox (corner1, corner2)) ;
}

void curveDrawSel::points (MPointArray &points) const {
	points.setLength (sCurvePointCount) ;
	for ( unsigned ii =0 ; ii < sCurvePointCount ; ii++ )
		points.set (basicCurvePoints [ii], ii) ;
}

void *curveDrawSel::creator () {
	return (new curveDrawSel ()) ;
}

MStatus curveDrawSel::initialize () { 
	return (MS::kSuccess) ;
}

MStatus curveDrawSel::registerMe (MFnPlugin &plugin) {
	return (plugin.registerShape (
		_T("curveDrawSel"),
		curveDrawSel::id,
		curveDrawSel::creator,
		curveDrawSel::initialize,
		curveDrawSelUI::creator
	)) ;
}

MStatus curveDrawSel::unregisterMe (MFnPlugin &plugin) {
	return (plugin.deregisterNode (curveDrawSel::id)) ;
}

