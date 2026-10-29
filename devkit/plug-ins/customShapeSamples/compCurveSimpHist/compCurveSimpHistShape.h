//
// Copyright 2012 Autodesk, Inc. All rights reserved.
//
// Use of this software is subject to the terms of the Autodesk license 
// agreement provided at the time of installation or download, or which 
// otherwise accompanies this software in either electronic or hard copy form. 
//

// Implements a hermite curve shape node in maya.
#pragma once

//-----------------------------------------------------------------------------
class compCurveSimpHist : public MPxSurfaceShape {
public: 
	static MTypeId id ;
	static MObject bboxCorner1 ;
	static MObject bboxCorner2 ;
	static MObject inputSurface ;
private:
	MPointArray fCurvePoints ;
	MPointArray fInputCurvePoints ;
	bool fHasHistory ;

public:
	compCurveSimpHist () ;
	virtual ~compCurveSimpHist () ;

	// Overrides

	// Bounding box methods
	virtual bool isBounded () const ;
	virtual MBoundingBox boundingBox () const ; 

	static void *creator () ;
	static MStatus initialize () ;
	static MStatus registerMe (MFnPlugin &plugin) ;
	static MStatus unregisterMe (MFnPlugin &plugin) ;

	virtual bool setInternalValue (const MPlug &, const MDataHandle &) ;
	virtual bool getInternalValue (const MPlug &plug, MDataHandle &result) ;
	virtual MStatus compute (const MPlug &plug, MDataBlock &datablock) ;

	virtual void transformUsing (const MMatrix &mat, const MObjectArray &componentList, MVertexCachingMode cachingMode, MPointArray *pointCache) ;
	virtual MObject localShapeInAttr () const ;
	virtual void componentToPlugs (MObject &component, MSelectionList &list) const ;
	virtual MatchResult matchComponent (const MSelectionList &item, const MAttributeSpecArray &spec, MSelectionList &list) ;

	// Associates a user defined iterator with the shape (components)
	virtual	MPxGeometryIterator * geometryIteratorSetup (MObjectArray &, MObject &, bool forReadOnly =false) ;
	virtual bool acceptsGeometryIterator (bool writeable =true) ;
	virtual bool acceptsGeometryIterator (MObject &, bool writeable =true, bool forReadOnly =false) ;
	virtual MStatus connectionMade (const MPlug &plug, const MPlug &otherPlug, bool asSrc) ;
	virtual MStatus connectionBroken (const MPlug &plug, const MPlug &otherPlug, bool asSrc) ;

	// Utility methods
	void points (MPointArray &points) ;
	MPoint point (unsigned index) const ;
	unsigned pointCount () const ;
	void setPoint (unsigned index, const MPoint &val) ;

private:
	void setPointValue (const MPlug &plug, const MDataHandle &handle, unsigned vtInd) ;
	MStatus computeBoundingBox (MDataBlock &block) ;
	void applyTweaks (MDataBlock &block) ;
	void updateControlPoints () ;

} ;

