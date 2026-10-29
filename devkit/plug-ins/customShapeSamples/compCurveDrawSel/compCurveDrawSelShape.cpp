//
//  Copyright 2012 Autodesk, Inc.  All rights reserved.
//
//  Use of this software is subject to the terms of the Autodesk license 
//  agreement provided at the time of installation or download, or which 
//  otherwise accompanies this software in either electronic or hard copy form.   
//
#include "StdAfx.h"

#include <math.h>           
#include "compCurveDrawSelShape.h"
#include "compCurveDrawSelShapeUI.h"
#include "api_macros.h"

//-----------------------------------------------------------------------------
static unsigned sCurvePointCount = 4;
MTypeId compCurveDrawSel::id (0x8701C) ;

// Curve attributes
MObject compCurveDrawSel::bboxCorner1 ;
MObject compCurveDrawSel::bboxCorner2 ;

//-----------------------------------------------------------------------------
compCurveDrawSel::compCurveDrawSel () {
	fCurvePoints.setLength (sCurvePointCount) ;
}

compCurveDrawSel::~compCurveDrawSel () {
}

//-----------------------------------------------------------------------------
// Overrides

// Specifies that this object has a boundingBox.
bool compCurveDrawSel::isBounded () const {
	return (true) ;
}

// Returns the bounding box for this object.
// It is a good idea not to recompute here as this function is called often.
MBoundingBox compCurveDrawSel::boundingBox () const {
	compCurveDrawSel *nonConstThis =const_cast<compCurveDrawSel *>(this) ;
	MDataBlock block =nonConstThis->forceCache () ;
	MDataHandle lowerHandle =block.inputValue (bboxCorner1) ;
	MDataHandle upperHandle =block.inputValue (bboxCorner2) ;
	double3 &lower =lowerHandle.asDouble3 () ;
	double3 &upper =upperHandle.asDouble3 () ;

	MPoint corner1 (lower [0], lower [1], lower [2]) ;
	MPoint corner2 (upper [0], upper [1], upper [2]) ;
	return MBoundingBox (corner1, corner2) ;
}

// When input attributes are dirty this method will be called to
// recompute the output attributes.
MStatus compCurveDrawSel::compute (const MPlug &plug, MDataBlock &datablock) {
	if ( plug == bboxCorner1 || plug == bboxCorner2 )
			return (computeBoundingBox (datablock)) ;
	return (MS::kUnknownParameter) ;
}

bool compCurveDrawSel::setInternalValue (const MPlug &plug, const MDataHandle &handle) {
	if (    (plug == mControlPoints)
		 || (plug == mControlValueX)
		 || (plug == mControlValueY)
		 || (plug == mControlValueZ)
	) {
		if ( plug == mControlPoints && !plug.isArray () ) {
			unsigned index =plug.logicalIndex () ;
			if ( index < sCurvePointCount ) {
				double3 &ptData = handle.asDouble3 () ;
				fCurvePoints [index].x =ptData [0] ;
				fCurvePoints [index].y =ptData [1] ;
				fCurvePoints [index].z =ptData [2] ;
				fCurvePoints [index].w =1.0 ;
			}
		} else if ( plug == mControlValueX ) {
			setPointValue (plug, handle, 0) ;
		} else if ( plug == mControlValueY ) {
			setPointValue (plug, handle, 1) ;
		} else if ( plug == mControlValueZ ) {
			setPointValue (plug, handle, 2) ;
		}
		MDataBlock block =forceCache () ;
		computeBoundingBox (block) ;
		childChanged (MPxSurfaceShape::kBoundingBoxChanged) ;
	}
	return (MPxSurfaceShape::setInternalValue (plug, handle)) ;
}

//-----------------------------------------------------------------------------
void compCurveDrawSel::points (MPointArray &points) {
	MDataBlock block =forceCache () ;
	block.inputArrayValue (mControlPoints) ;
	points.setLength (sCurvePointCount) ;
	for ( unsigned ii =0 ; ii < sCurvePointCount ; ii++ )
		points.set (fCurvePoints [ii], ii) ;
}

void compCurveDrawSel::setPointValue (const MPlug &plug, const MDataHandle &handle, unsigned vtInd) {
	MPlug parentPlug =plug.parent () ;
	unsigned index =parentPlug.logicalIndex () ;
	if ( index < sCurvePointCount )
		fCurvePoints [index] [vtInd] =handle.asDouble () ;
}

// Compute and set the bounding box.
MStatus compCurveDrawSel::computeBoundingBox (MDataBlock &datablock) {
	// Update bounding box
	MDataHandle lowerHandle =datablock.outputValue (bboxCorner1) ;
	MDataHandle upperHandle =datablock.outputValue (bboxCorner2) ;
	double3 &lower =lowerHandle.asDouble3 () ;
	double3 &upper = upperHandle.asDouble3();

	// This clears any old bbox values
	MPoint tmppnt =fCurvePoints [0] ;
	lower [0] =tmppnt [0] ; lower [1] =tmppnt [1] ; lower [2] =tmppnt [2] ;
	upper [0] =tmppnt [0] ; upper [1] =tmppnt [1] ; upper [2] =tmppnt [2] ;

	for ( unsigned ii =0 ; ii < sCurvePointCount ; ii++ ) {
		double x =fCurvePoints [ii] [0] ;
		if ( x > upper [0] )
			upper [0] =x ;
		if ( x < lower [0] )
			lower [0] =x ;

		double y =fCurvePoints [ii] [1] ;
		if ( y > upper [1] )
			upper [1] =y ;
		if ( y < lower [1] )
			lower [1] =y ;

		double z =fCurvePoints [ii] [2] ;
		if ( z > upper [2] )
			upper [2] =z ;
		if ( z < lower [2] )
			lower [2] =z ;
	}

	lowerHandle.setClean () ;
	upperHandle.setClean () ;

	// Signal that the bounding box has changed.
	childChanged (MPxSurfaceShape::kBoundingBoxChanged) ;
	return (MS::kSuccess) ;
}

//-----------------------------------------------------------------------------
void* compCurveDrawSel::creator () {
	return (new compCurveDrawSel ()) ;
}

MStatus compCurveDrawSel::initialize () {
	// bbox attributes
	MAKE_NUMERIC_ATTR(bboxCorner1, "bboxCorner1", "bb1", MFnNumericData::k3Double, 0, false, false, false) ;
	MAKE_NUMERIC_ATTR(bboxCorner2, "bboxCorner2", "bb2", MFnNumericData::k3Double, 0, false, false, false) ;

	ATTRIBUTE_AFFECTS(mControlPoints, bboxCorner1) ;
	ATTRIBUTE_AFFECTS(mControlValueX, bboxCorner1) ;
	ATTRIBUTE_AFFECTS(mControlValueY, bboxCorner1) ;
	ATTRIBUTE_AFFECTS(mControlValueZ, bboxCorner1) ;
	ATTRIBUTE_AFFECTS(mControlPoints, bboxCorner2) ;
	ATTRIBUTE_AFFECTS(mControlValueX, bboxCorner2) ;
	ATTRIBUTE_AFFECTS(mControlValueY, bboxCorner2) ;
	ATTRIBUTE_AFFECTS(mControlValueZ, bboxCorner2) ;
	return (MS::kSuccess) ;
}

MStatus compCurveDrawSel::registerMe (MFnPlugin &plugin) {
	return (plugin.registerShape (
		_T("compCurveDrawSel"),
		compCurveDrawSel::id,
		compCurveDrawSel::creator,
		compCurveDrawSel::initialize,
		compCurveDrawSelUI::creator
	)) ;
}

MStatus compCurveDrawSel::unregisterMe (MFnPlugin &plugin) {
	return (plugin.deregisterNode (compCurveDrawSel::id)) ;
}
