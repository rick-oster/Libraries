//
//  Copyright 2012 Autodesk, Inc.  All rights reserved.
//
//  Use of this software is subject to the terms of the Autodesk license 
//  agreement provided at the time of installation or download, or which 
//  otherwise accompanies this software in either electronic or hard copy form.   
//

// Implements a hermite curve shape node in Maya.
#pragma once

class compCurveDrawSel : public MPxSurfaceShape {

public: 
	static MTypeId id ;
	static MObject bboxCorner1 ;
	static MObject bboxCorner2 ;
private:
	MPointArray fCurvePoints ;

public:
	compCurveDrawSel () ;
	virtual ~compCurveDrawSel () ;

	// Overrides

	// Bounding box methods
	virtual bool isBounded () const ;
	virtual MBoundingBox boundingBox () const ;

	static void *creator () ;
	static MStatus initialize () ;
	static MStatus registerMe (MFnPlugin &plugin) ;
	static MStatus unregisterMe (MFnPlugin &plugin) ;

	virtual bool setInternalValue (const MPlug &, const MDataHandle &) ;
	virtual MStatus compute (const MPlug &plug, MDataBlock &datablock) ;

	// Utility methods
	void points (MPointArray &points) ;

private:
	void setPointValue (const MPlug &plug, const MDataHandle &handle, unsigned vtInd) ;
	MStatus computeBoundingBox (MDataBlock &block) ;

} ;
