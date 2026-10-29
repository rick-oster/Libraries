//
// Copyright 2012 Autodesk, Inc. All rights reserved.
//
// Use of this software is subject to the terms of the Autodesk license 
// agreement provided at the time of installation or download, or which 
// otherwise accompanies this software in either electronic or hard copy form. 
//

// The UI portion of the hermite curve shape.
#pragma once

//-----------------------------------------------------------------------------
class compCurveMovableUI : public MPxSurfaceShapeUI {
private:
	// Draw Tokens
	enum {
		kDrawVertices, // component token
		kDrawWireframe,
		kDrawWireframeOnShaded,
		kLastToken
	} ;

public:
	compCurveMovableUI () ;
	virtual ~compCurveMovableUI () ;

	// Overrides
	virtual void getDrawRequests (const MDrawInfo &info, bool objectAndActiveOnly, MDrawRequestQueue &requests) ;
	virtual void draw (const MDrawRequest &request, M3dView &view) const ;
	virtual bool select (MSelectInfo &selectInfo, MSelectionList &selectionList, MPointArray &worldSpaceSelectPts) const ;

	// Helper routines
	static void *creator () ;

private:
	void drawHermite (M3dView &view) const ;
	void drawPoints (const MDrawRequest &request, const MPointArray &) const ;
	bool selectPoints (MSelectInfo &selectInfo, MSelectionList &selectionList, MPointArray &worldSpaceSelectPts) const ;

} ;
