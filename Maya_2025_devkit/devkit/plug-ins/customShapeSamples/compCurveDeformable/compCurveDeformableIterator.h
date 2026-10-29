//
//  Copyright 2012 Autodesk, Inc.  All rights reserved.
//
//  Use of this software is subject to the terms of the Autodesk license 
//  agreement provided at the time of installation or download, or which 
//  otherwise accompanies this software in either electronic or hard copy form.   
//

// Point iterator for control-point based geometry
//
// This is used by the translate/rotate/scale manipulators to 
// determine where to place the manipulator when components are
// selected.
//
// As well deformers use this class to deform points of the shape.
//
#pragma once

//-----------------------------------------------------------------------------
class compCurveDeformable;

//-----------------------------------------------------------------------------
class compCurveDeformableIterator : public MPxGeometryIterator {
public:
	MPointArray *curvePoints ;

public:
	compCurveDeformableIterator (MPointArray *curve, MObjectArray &components) ;
	compCurveDeformableIterator (MPointArray *userGeometry, MObject &components) ;

	// Overrides
	virtual void reset () ;
	virtual MPoint point () const ;
	virtual void setPoint (const MPoint &) const ;
	virtual int iteratorCount () const ;
	virtual bool hasPoints () const ;

} ;

