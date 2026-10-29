//
//  Copyright 2012 Autodesk, Inc.  All rights reserved.
//
//  Use of this software is subject to the terms of the Autodesk license 
//  agreement provided at the time of installation or download, or which 
//  otherwise accompanies this software in either electronic or hard copy form.   
//

// Implements a hermite curve shape node in Maya.
#pragma once

class curveDrawSel : public MPxSurfaceShape {

public: 
	static MTypeId id ;
private:
	MPointArray basicCurvePoints ;

public:
	curveDrawSel () ;
	virtual ~curveDrawSel () ; 

	// Overrides

	// Bounding box methods
	virtual bool isBounded() const;
	virtual MBoundingBox boundingBox() const; 

	static void *creator () ;
	static MStatus initialize () ;
	static MStatus registerMe (MFnPlugin &plugin) ;
	static MStatus unregisterMe (MFnPlugin &plugin) ;

	// Utility methods
	void points (MPointArray &points) const ;

} ;
