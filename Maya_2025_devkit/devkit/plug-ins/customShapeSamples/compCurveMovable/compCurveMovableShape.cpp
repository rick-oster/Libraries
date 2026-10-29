//
// Copyright 2012 Autodesk, Inc. All rights reserved.
//
// Use of this software is subject to the terms of the Autodesk license 
// agreement provided at the time of installation or download, or which 
// otherwise accompanies this software in either electronic or hard copy form. 
//
#include "StdAfx.h"

#include <math.h>

#include "compCurveMovableShape.h"
#include "compCurveMovableShapeUI.h"
#include "compCurveMovableIterator.h"
#include "api_macros.h"

//-----------------------------------------------------------------------------
static unsigned sCurvePointCount =4 ;
MTypeId compCurveMovable::id (0x8701D) ;

// Curve attributes
MObject compCurveMovable::bboxCorner1 ;
MObject compCurveMovable::bboxCorner2 ;

//-----------------------------------------------------------------------------
compCurveMovable::compCurveMovable () {
	fCurvePoints.setLength (sCurvePointCount) ;
}

compCurveMovable::~compCurveMovable () {
}

//-----------------------------------------------------------------------------
// Overrides

// Specifies that this object has a boundingBox.
bool compCurveMovable::isBounded () const { 
	return (true) ;
}

// Returns the bounding box for this object.
// It is a good idea not to recompute here as this function is called often.
MBoundingBox compCurveMovable::boundingBox () const {
	compCurveMovable *nonConstThis =const_cast<compCurveMovable *>(this) ;
	MDataBlock block =nonConstThis->forceCache () ;
	MDataHandle lowerHandle =block.inputValue (bboxCorner1) ;
	MDataHandle upperHandle =block.inputValue (bboxCorner2) ;
	double3 &lower =lowerHandle.asDouble3 () ;
	double3 &upper =upperHandle.asDouble3 () ;
	
	MPoint corner1 (lower [0], lower [1], lower [2]) ;
	MPoint corner2 (upper [0], upper [1], upper [2]) ;
	return (MBoundingBox (corner1, corner2)) ;
}

// When input attributes are dirty this method will be called to
// recompute the output attributes.
//    plug - the attribute that triggered the compute
//    datablock - the nodes data
MStatus compCurveMovable::compute (const MPlug &plug, MDataBlock &datablock) { 
	if ( plug ==bboxCorner1 ||  plug ==bboxCorner2 )
		return (computeBoundingBox (datablock)) ;
	return (MS::kUnknownParameter) ;
}

// Compute and set the bounding box.
MStatus compCurveMovable::computeBoundingBox (MDataBlock &datablock) {
	MDataHandle lowerHandle =datablock.outputValue (bboxCorner1) ;
	MDataHandle upperHandle =datablock.outputValue (bboxCorner2) ;
	double3 &lower =lowerHandle.asDouble3 () ;
	double3 &upper =upperHandle.asDouble3 () ;

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

bool compCurveMovable::setInternalValue (const MPlug &plug, const MDataHandle &handle) {
	bool isOk =true ;
	if (   (plug ==mControlPoints)
		|| (plug ==mControlValueX)
		|| (plug ==mControlValueY)
		|| (plug ==mControlValueZ)
	) {
		if ( plug == mControlPoints && !plug.isArray () ) {
			unsigned index =plug.logicalIndex () ;
			if ( index < sCurvePointCount ) {
				double3& ptData =handle.asDouble3 () ;
				fCurvePoints [index].x =ptData [0] ;
				fCurvePoints [index].y =ptData [1] ;
				fCurvePoints [index].z =ptData [2] ;
				fCurvePoints [index].w =1.0 ;
			}
		} else if( plug ==mControlValueX ) {
			setPointValue (plug, handle, 0) ;
		} else if ( plug ==mControlValueY ) {
			setPointValue (plug, handle, 1) ;
		} else if ( plug ==mControlValueZ ) {
			setPointValue (plug, handle, 2) ;
		}
		MDataBlock block =forceCache () ;
		computeBoundingBox (block) ;
	}
	isOk =MPxSurfaceShape::setInternalValue (plug, handle) ;
	return (isOk) ;
}

// Transforms the given components. This method is used by
// the move, rotate, and scale tools in component mode.
// The bounding box has to be updated here, so do the normals and
// any other attributes that depend on vertex positions.
//     mat - matrix to tranform the components by
//     componentList - list of components to be transformed, 
//                     or an empty list to indicate the whole surface 	
//     cachingMode - how to use the supplied pointCache
//     pointCache - if non-null, save or restore points from this list base
//                  on the cachingMode
void compCurveMovable::transformUsing (const MMatrix &mat, const MObjectArray &componentList, MVertexCachingMode cachingMode, MPointArray *pointCache) {
	bool savePoints =(cachingMode == MPxSurfaceShape::kSavePoints) ;
	unsigned int i =0, j =0 ;
	unsigned int len =componentList.length () ;
	
	if ( cachingMode == MPxSurfaceShape::kRestorePoints ) {
		// restore the points based on the data in the pointCache attribute
		unsigned int cacheLen =pointCache->length () ;
		if ( len > 0 ) {
			// traverse the component list
			for ( i =0 ; i < len && j < cacheLen ; i++ ) {
				MObject comp =componentList [i] ;
				MFnSingleIndexedComponent fnComp (comp) ;
				int elemCount =fnComp.elementCount () ;
				for ( int idx =0 ; idx < elemCount && j < cacheLen ; idx++, ++j ) {
					int elemIndex =fnComp.element (idx) ;
					fCurvePoints [elemIndex] =(*pointCache) [j] ;
				}
			}
		} else {
			// if the component list is of zero-length, it indicates that we
			// should transform the entire surface
			unsigned idx ;
			len =fCurvePoints.length () ;
			for ( idx =0 ; idx < len && j < cacheLen ; ++idx, ++j ) {
				fCurvePoints [idx] =(*pointCache) [j] ;
			}
		}
	} else {	
		// Transform the surface vertices with the matrix.
		// If savePoints is true, save the points to the pointCache.
		if ( len > 0 ) {
			// Traverse the componentList 
			for ( i=0 ; i<len ; i++ ) {
				MObject comp =componentList [i] ;
				MFnSingleIndexedComponent fnComp (comp) ;
				int elemCount =fnComp.elementCount () ;

				if ( savePoints && i == 0 )
					pointCache->setSizeIncrement (elemCount) ;
				for ( int idx =0 ; idx < elemCount ; idx++ ) {
					int elemIndex =fnComp.element (idx) ;
					if ( savePoints )
						pointCache->append (fCurvePoints [elemIndex]) ;
					fCurvePoints [elemIndex] *=mat ;
				}
			}
		} else {
			// If the component list is of zero-length, it indicates that we
			// should transform the entire surface
			len =fCurvePoints.length () ;
			if ( savePoints )
				pointCache->setSizeIncrement(len) ;
			for ( unsigned int idx =0 ; idx < len ; ++idx ) {
				if ( savePoints )
					pointCache->append (fCurvePoints [idx]) ;
				fCurvePoints [idx] *=mat ;
			}
		}
	}

	// Update the bounding box
	MDataBlock block =forceCache () ;
	computeBoundingBox (block) ;
}

// Handle internal attributes.
//
// Attributes that require special storage, bounds checking, 
// or other non-standard behavior can be marked as "Internal" by
// using the "MFnAttribute::setInternal" method.
//
// The get/setInternalValue methods will get called for internal
// attributes whenever the attribute values are stored or retrieved
// using getAttr/setAttr or MPlug getValue/setValue.
// 
// The inherited attribute mControlPoints is internal and we want
// its values to get stored only if there is input history. Otherwise
// any changes to the vertices are stored in the cachedMesh and outputMesh
// directly.
//
// If values are retrieved then we want the controlPoints value
// returned if there is history, this will be the offset or tweak.
// In the case of no history, the vertex position of the cached mesh
// is returned.
bool compCurveMovable::getInternalValue (const MPlug &plug, MDataHandle &result) {
	bool isOk =true ;
	if (   (plug ==mControlPoints)
		|| (plug ==mControlValueX)
		|| (plug ==mControlValueY)
		|| (plug ==mControlValueZ)
	) {
		double val =0.0 ;
		if ( (plug ==mControlPoints) && !plug.isArray () ) {
			int index =plug.logicalIndex () ;
			const MPoint& pnt =fCurvePoints [index] ;
			result.set (pnt [0], pnt [1], pnt [2]) ;
		} else if ( plug ==mControlValueX ) {
			MPlug parentPlug =plug.parent () ;
			int index =parentPlug.logicalIndex () ;
			val =fCurvePoints [index].x ;
			result.set (val) ;
		} else if ( plug ==mControlValueY ) {
			MPlug parentPlug =plug.parent () ;
			int index =parentPlug.logicalIndex () ;
			val =fCurvePoints [index].y ;
			result.set (val) ;
		} else if ( plug ==mControlValueZ ) {
			MPlug parentPlug =plug.parent () ;
			int index =parentPlug.logicalIndex () ;
			val =fCurvePoints [index].z ;
			result.set (val) ;
		}
	} else {
		isOk =MPxSurfaceShape::getInternalValue (plug, result) ;
	}
	return (isOk) ;
}

// Converts the given component values into a selection list of plugs.
// This method is used to map components to attributes.
//     component - the component to be translated to a plug/attribute
//     list - a list of plugs representing the passed in component
void compCurveMovable::componentToPlugs (MObject &component, MSelectionList &list) const {
	if ( component.hasFn (MFn::kSingleIndexedComponent) ) {
		MFnSingleIndexedComponent fnVtxComp (component) ;
		MObject thisNode =thisMObject () ;
		MPlug plug (thisNode, mControlPoints) ;

		int len =fnVtxComp.elementCount () ;
		for ( int i =0 ; i < len ; i++ ) {
			plug.selectAncestorLogicalIndex (fnVtxComp.element (i), plug.attribute ()) ;
			list.add (plug) ;
		}
	}
}

// Component/attribute matching method.
// This method validates component names and indices which are
// specified as a string and adds the corresponding component
// to the passed in selection list.
//
// For instance, select commands such as "select shape1.vtx [0:7]"
// are validated with this method and the corresponding component
// is added to the selection list.
//
//    item - DAG selection item for the object being matched
//    spec - attribute specification object
//    list - list to add components to
MPxSurfaceShape::MatchResult compCurveMovable::matchComponent (const MSelectionList &item, const MAttributeSpecArray &spec, MSelectionList &list) {
	MPxSurfaceShape::MatchResult result =MPxSurfaceShape::kMatchOk ;
	MAttributeSpec attrSpec =spec [0] ;
	int dim =attrSpec.dimensions () ;

	// Look for attributes specifications of the form :
	// vtx [ index ]
	// vtx [ lower:upper ]
	if ( (spec.length () == 1) && (dim > 0) && (attrSpec.name () == "cv") ) {
		int numVertices =pointCount () ;
		MAttributeIndex attrIndex =attrSpec [0] ;

		int upper =0 ;
		int lower =0 ;
		if ( attrIndex.hasLowerBound () )
			attrIndex.getLower (lower) ;
		if ( attrIndex.hasUpperBound () )
			attrIndex.getUpper (upper) ;
		// Check the attribute index range is valid
		if ( (lower > upper) || (upper >= numVertices) ) {
			result =MPxSurfaceShape::kMatchInvalidAttributeRange ;	
		} else {
			MDagPath path ;
			item.getDagPath (0, path) ;
			MFnSingleIndexedComponent fnVtxComp ;
			MObject vtxComp =fnVtxComp.create (MFn::kCurveCVComponent) ;
			for ( int i=lower ; i<=upper ; i++ )
				fnVtxComp.addElement (i) ;
			list.add (path, vtxComp) ;
		}
	} else {
		// Pass this to the parent class
		return (MPxSurfaceShape::matchComponent (item, spec, list)) ;
	}
	return (result) ;
}

// Creates a geometry iterator compatible with this shape.
//    componentList - list of components to be iterated
//    components - component to be iterator
//    forReadOnly -
MPxGeometryIterator* compCurveMovable::geometryIteratorSetup (MObjectArray &componentList, MObject &components, bool forReadOnly) {
	compCurveMovableIterator *result =NULL ;
	if ( components.isNull () )
		result =new compCurveMovableIterator (this, componentList) ;
	else
		result =new compCurveMovableIterator (this, components) ;
	return (result) ;
}

// Specifies that this shape can provide an iterator for getting/setting
// control point values.
//      writable - maya asks for an iterator that can set points if this is true
bool compCurveMovable::acceptsGeometryIterator (bool writeable) {
	return (true) ;
}

// Specifies that this shape can provide an iterator for getting/setting
// control point values.
//     writable - maya asks for an iterator that can set points if this is true
//     forReadOnly - maya asking for an iterator for querying only
bool compCurveMovable::acceptsGeometryIterator (MObject &, bool writeable, bool forReadOnly) {
	return (true) ;
}

void compCurveMovable::points (MPointArray &points) {
	MDataBlock block =forceCache () ;
	block.inputArrayValue (mControlPoints) ;
	
	points.setLength (sCurvePointCount) ;
	for ( unsigned ii =0 ; ii < sCurvePointCount ; ii++ )
		points.set (fCurvePoints [ii], ii) ;
}

unsigned compCurveMovable::pointCount () const {
	return (fCurvePoints.length ()) ;
}

void compCurveMovable::setPoint (unsigned index, const MPoint &val) {
	if ( index < fCurvePoints.length () )
		fCurvePoints [index] =val ;
}

MPoint compCurveMovable::point (unsigned index) const {
	if ( index < fCurvePoints.length () )
		return (fCurvePoints [index]) ;
	return (MPoint ()) ;
}

void compCurveMovable::setPointValue (const MPlug &plug, const MDataHandle &handle, unsigned vtInd) {
	MPlug parentPlug =plug.parent () ;
	unsigned index =parentPlug.logicalIndex () ;
	if ( index < sCurvePointCount ) {
		fCurvePoints [index] [vtInd] =handle.asDouble () ;
		fCurvePoints [index].w =1.0 ;
	}
}

//-----------------------------------------------------------------------------
// Called internally to create a new instance of the users MPx node.
void *compCurveMovable::creator () {
	return (new compCurveMovable ()) ;
}

// Attribute (static) initialization.
// See api_macros.h.
MStatus compCurveMovable::initialize () { 
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

MStatus compCurveMovable::registerMe (MFnPlugin &plugin) {
	return (plugin.registerShape (
		_T("compCurveMovable"),
		compCurveMovable::id,
		compCurveMovable::creator,
		compCurveMovable::initialize,
		compCurveMovableUI::creator
	)) ;
}

MStatus compCurveMovable::unregisterMe (MFnPlugin &plugin) {
	return (plugin.deregisterNode (compCurveMovable::id)) ;
}
