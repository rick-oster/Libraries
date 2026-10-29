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
class compCurveDeformable : public MPxSurfaceShape {
public: 
	static MTypeId id ;
	static MObject bboxCorner1 ;
	static MObject bboxCorner2 ;
	static MObject inputSurface ;
	static MObject cachedSurface ;	
	static MObject outputSurface ;
	static MObject worldSurface ;
private:
	bool fHasHistory ;

public:
	compCurveDeformable () ;
	virtual ~compCurveDeformable () ;

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
	virtual void tweakUsing (const MMatrix &mat, const MObjectArray &componentList, MVertexCachingMode cachingMode, MPointArray *pointCache, MArrayDataHandle &handle) ;
	virtual MObject localShapeInAttr () const ;
	virtual MObject worldShapeOutAttr () const ;
	virtual MObject localShapeOutAttr () const ;
	virtual MObject cachedShapeAttr () const ;
	virtual void componentToPlugs (MObject &component, MSelectionList &list) const ;
	virtual MatchResult matchComponent (const MSelectionList &item, const MAttributeSpecArray &spec, MSelectionList &list) ;
	virtual MObject createFullVertexGroup () const ;
	virtual MObject geometryData () const ;
	virtual bool match (const MSelectionMask &mask, const MObjectArray &componentList) const ;

	// Associates a user defined iterator with the shape (components)
	virtual	MPxGeometryIterator * geometryIteratorSetup (MObjectArray &, MObject &, bool forReadOnly =false) ;
	virtual bool acceptsGeometryIterator (bool writeable =true) ;
	virtual bool acceptsGeometryIterator (MObject &, bool writeable =true, bool forReadOnly =false) ;
	virtual MStatus connectionMade (const MPlug &plug, const MPlug &otherPlug, bool asSrc) ;
	virtual MStatus connectionBroken (const MPlug &plug, const MPlug &otherPlug, bool asSrc) ;

	// Utility methods
	void points (MPointArray &points) ;

private:
	void setPointValue (const MPlug &plug, const MDataHandle &handle, unsigned vtInd) ;
	MStatus computeBoundingBox (MDataBlock &block) ;
	void applyTweaks (MDataBlock &block) ;
	void updateControlPoints (const MPointArray &) ;
	compCurveData *getCurveAttr (const MObject &attribute) ;
	compCurveData *cachedCurve () ;
	MStatus evaluateInputSurface (MDataBlock &) ;
	MStatus computeOutputSurface (const MPlug &, MDataBlock &) ;
	MStatus computeWorldSurface (const MPlug &, MDataBlock &) ;

} ;

