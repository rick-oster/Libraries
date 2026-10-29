//
// Copyright 2012 Autodesk, Inc. All rights reserved.
//
// Use of this software is subject to the terms of the Autodesk license 
// agreement provided at the time of installation or download, or which 
// otherwise accompanies this software in either electronic or hard copy form. 
//
#include "StdAfx.h"

#include <math.h>

#include "compCurveData.h"
#include "compCurveDeformableIterator.h"
#include "compCurveDeformableShape.h"
#include "compCurveDeformableShapeUI.h"
#include "api_macros.h"

//-----------------------------------------------------------------------------
static unsigned sCurvePointCount =4 ;
MTypeId compCurveDeformable::id (0x8701B) ;

// Curve attributes
MObject compCurveDeformable::bboxCorner1 ;
MObject compCurveDeformable::bboxCorner2 ;
MObject compCurveDeformable::inputSurface ;
MObject compCurveDeformable::outputSurface;
MObject compCurveDeformable::cachedSurface;
MObject compCurveDeformable::worldSurface;

//-----------------------------------------------------------------------------
compCurveDeformable::compCurveDeformable () {
	fHasHistory =false ;
}

compCurveDeformable::~compCurveDeformable () {
}

//-----------------------------------------------------------------------------
// Overrides

// Specifies that this object has a boundingBox.
bool compCurveDeformable::isBounded () const { 
	return (true) ;
}

// Returns the bounding box for this object.
// It is a good idea not to recompute here as this function is called often.
MBoundingBox compCurveDeformable::boundingBox () const {
	compCurveDeformable *nonConstThis =const_cast<compCurveDeformable *>(this) ;
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
MStatus compCurveDeformable::compute (const MPlug &plug, MDataBlock &datablock) { 
	if ( plug == outputSurface ) {
		return (computeOutputSurface (plug, datablock)) ;
	} else if ( plug == worldSurface ) {
		return (computeWorldSurface (plug, datablock)) ;
	} else if ( plug == cachedSurface ) {
		return (computeOutputSurface (plug, datablock)) ;
	} else if ( plug ==bboxCorner1 ||  plug ==bboxCorner2 ) {
		return (computeBoundingBox (datablock)) ;
	}
	return (MS::kUnknownParameter) ;
}

// If there is input history, evaluate the input attribute
MStatus compCurveDeformable::evaluateInputSurface (MDataBlock &datablock) {
	MStatus stat =MS::kSuccess ;
	// Get the input surface if there is history
	if ( fHasHistory ) {
		MDataHandle inputHandle =datablock.inputValue (inputSurface, &stat) ;
		compCurveData *curve =(compCurveData *)inputHandle.asPluginData () ;
		if ( curve == NULL ) {
			cerr << "NULL inputSurface data found\n" ;
			return (stat) ;
		}
		// Create the cachedSurface and copy the input surface into it
		MFnPluginData fnDataCreator ;
		MTypeId tmpid (compCurveData::id) ;
		MObject newCachedObject =fnDataCreator.create (tmpid, &stat) ;
		compCurveData *newCachedData =(compCurveData *)fnDataCreator.data (&stat) ;
		*newCachedData =*curve ;
		MDataHandle cachedHandle =datablock.outputValue (cachedSurface, &stat) ;
		cachedHandle.set (newCachedData) ;
	}
	return (stat) ;
}

// Compute the outputSurface attribute.
//
// If there is no history, use cachedSurface as the
// input surface. All tweaks will get written directly
// to it. Output is just a copy of the cached surface
// that can be connected etc.
MStatus compCurveDeformable::computeOutputSurface (const MPlug &plug, MDataBlock &datablock) {
	MStatus stat ;
	// Update input if there is a connection.
	if ( fHasHistory ) {
		if ( ! evaluateInputSurface (datablock) )
			return (MS::kFailure) ;
		// Apply any vertex offsets.
		applyTweaks (datablock) ;
	} else {
		MArrayDataHandle cpHandle =datablock.inputArrayValue (mControlPoints, &stat) ;
		cpHandle.setAllClean () ;
	}
	// Get a handle to the cached data
	MDataHandle cachedHandle =datablock.outputValue (cachedSurface, &stat) ;
	compCurveData *cached =(compCurveData *)cachedHandle.asPluginData () ;
	if ( cached == NULL )
		cerr << "NULL cachedSurface data found\n" ;

	// Create some output data
	MFnPluginData fnDataCreator ;
	MTypeId tmpid (compCurveData::id) ;
	MObject newDataObject =fnDataCreator.create (tmpid, &stat) ;
	MCHECKERROR(stat, "compute : error creating compCurveData")
	compCurveData *newData =(compCurveData *)fnDataCreator.data (&stat) ;
	MCHECKERROR(stat, "compute : error gettin at proxy compCurveData object")
	// Copy the data
	if ( cached != NULL ) {
		MPointArray &newPoints =newData->points () ;
		newPoints.copy (cached->points ()) ;
	} else {
		cerr << "computeOutputSurface: NULL cachedSurface data\n" ;
	}
	// Assign the new data to the outputSurface handle
	MDataHandle outHandle =datablock.outputValue (outputSurface) ;
	outHandle.set (newData) ;
	return (stat) ;
}

// Compute the worldSurface attribute.
MStatus compCurveDeformable::computeWorldSurface (const MPlug &plug, MDataBlock &datablock) {
	MStatus stat ;
	computeOutputSurface (plug, datablock) ;
	MDataHandle inHandle =datablock.outputValue (outputSurface) ;
	compCurveData *outSurf =(compCurveData *)inHandle.asPluginData () ;
	if ( outSurf == NULL ) {
		cerr << "computeWorldSurface: outSurf NULL\n" ;
		return (MS::kFailure) ;
	}
	// Create some output data
	//
	MFnPluginData fnDataCreator ;
	MTypeId tmpid (compCurveData::id) ;
	MObject newDataObject =fnDataCreator.create (tmpid, &stat) ;
	MCHECKERROR(stat, "compute : error creating compCurveData")
	compCurveData *newData =(compCurveData *)fnDataCreator.data (&stat) ;
	MCHECKERROR( stat, "compute : error gettin at proxy compCurveData object")

	// Get worldMatrix from MPxSurfaceShape and set it to MPxGeometryData
	MMatrix worldMat =getWorldMatrix (datablock, 0) ;
	newData->setMatrix (worldMat) ;
	// Copy the data
	MPointArray &newPoints =newData->points () ;
	newPoints.copy (outSurf->points ()) ;
	// Assign the new data to the outputSurface handle
	int arrayIndex =plug.logicalIndex (&stat) ;
	MCHECKERROR(stat, "computeWorldSurface : logicalIndex")
	MArrayDataHandle worldHandle =datablock.outputArrayValue (worldSurface, &stat) ;
	MCHECKERROR(stat, "computeWorldSurface : outputArrayValue")
	MArrayDataBuilder builder =worldHandle.builder (&stat) ;
	MCHECKERROR(stat, "computeWorldSurface : builder")
	MDataHandle outHandle =builder.addElement (arrayIndex, &stat) ;
	MCHECKERROR(stat, "computeWorldSurface : addElement")
	outHandle.set (newData) ;
	return (stat) ;
}

// Compute and set the bounding box.
MStatus compCurveDeformable::computeBoundingBox (MDataBlock &datablock) {
	if ( fHasHistory ) {
		datablock.inputValue (inputSurface) ;
		applyTweaks (datablock) ;
	}

	MDataHandle lowerHandle =datablock.outputValue (bboxCorner1) ;
	MDataHandle upperHandle =datablock.outputValue (bboxCorner2) ;
	double3 &lower =lowerHandle.asDouble3 () ;
	double3 &upper =upperHandle.asDouble3 () ;

	compCurveData *curveData =getCurveAttr (outputSurface) ;
	if ( curveData != NULL ) {
		MPointArray &curvePoints =curveData->points () ;

		// This clears any old bbox values
		MPoint tmppnt =curvePoints [0] ;
		lower [0] =tmppnt [0] ; lower [1] =tmppnt [1] ; lower [2] =tmppnt [2] ;
		upper [0] =tmppnt [0] ; upper [1] =tmppnt [1] ; upper [2] =tmppnt [2] ;

		for ( unsigned ii =0 ; ii < sCurvePointCount ; ii++ ) {
			double x =curvePoints [ii] [0] ;
			if ( x > upper [0] )
				upper [0] =x ;
			if ( x < lower [0] )
				lower [0] =x ;

			double y =curvePoints [ii] [1] ;
			if ( y > upper [1] )
				upper [1] =y ;
			if ( y < lower [1] )
				lower [1] =y ;

			double z =curvePoints [ii] [2] ;
			if ( z > upper [2] )
				upper [2] =z ;
			if ( z < lower [2] )
				lower [2] =z ;
		}

		// Signal that the bounding box has changed.
		childChanged (MPxSurfaceShape::kBoundingBoxChanged) ;
	}

	lowerHandle.setClean () ;
	upperHandle.setClean () ;
	return (MS::kSuccess) ;	
}

bool compCurveDeformable::setInternalValue (const MPlug &plug, const MDataHandle &handle) {
	bool isOk =true ;
	if (   (plug ==mControlPoints)
		|| (plug ==mControlValueX)
		|| (plug ==mControlValueY)
		|| (plug ==mControlValueZ)
	) {
		if ( fHasHistory ) {
			return (MPxSurfaceShape::setInternalValue (plug, handle)) ;
		} else {
			if ( plug == mControlPoints && !plug.isArray () ) {
				unsigned index =plug.logicalIndex () ;
				compCurveData *curve =cachedCurve () ;
				if ( curve != NULL && index < sCurvePointCount ) {
					double3& ptData =handle.asDouble3 () ;
					MPointArray& curvePoints =curve->points () ;
					curvePoints [index].x =ptData [0] ;
					curvePoints [index].y =ptData [1] ;
					curvePoints [index].z =ptData [2] ;
					curvePoints [index].w =1.0 ;
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
	} else if ( plug == mHasHistoryOnCreate) {
		fHasHistory =handle.asBool () ;
	} else {
		isOk =MPxSurfaceShape::setInternalValue (plug, handle) ;
	}
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
void compCurveDeformable::transformUsing (const MMatrix &mat, const MObjectArray &componentList, MVertexCachingMode cachingMode, MPointArray *pointCache) {
	bool savePoints =(cachingMode == MPxSurfaceShape::kSavePoints) ;
	unsigned int i =0, j =0 ;
	unsigned int len =componentList.length () ;
	
	compCurveData *curve =cachedCurve () ;
	if ( curve == NULL )
		return ;
	MPointArray& curvePoints =curve->points () ;
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
					curvePoints [elemIndex] =(*pointCache) [j] ;
				}
			}
		} else {
			// if the component list is of zero-length, it indicates that we
			// should transform the entire surface
			unsigned idx ;
			len =curvePoints.length () ;
			for ( idx =0 ; idx < len && j < cacheLen ; ++idx, ++j ) {
				curvePoints [idx] =(*pointCache) [j] ;
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
						pointCache->append (curvePoints [elemIndex]) ;
					curvePoints [elemIndex] *=mat ;
				}
			}
		} else {
			// If the component list is of zero-length, it indicates that we
			// should transform the entire surface
			len =curvePoints.length () ;
			if ( savePoints )
				pointCache->setSizeIncrement(len) ;
			for ( unsigned int idx =0 ; idx < len ; ++idx ) {
				if ( savePoints )
					pointCache->append (curvePoints [idx]) ;
				curvePoints [idx] *=mat ;
			}
		}
	}

	if ( fHasHistory )
		updateControlPoints (curvePoints) ;

	// Update the bounding box
	MDataBlock block =forceCache () ;
	computeBoundingBox (block) ;
}

// Transforms the given components. This method is used by
// the move, rotate, and scale tools in component mode when the
// tweaks for the shape are stored on a separate tweak node.
// The bounding box has to be updated here, so do the normals and
// any other attributes that depend on vertex positions.
//    mat           - matrix to tranform the components by
//    componentList - list of components to be transformed,
//                    or an empty list to indicate the whole surface 	
//    cachingMode   - how to use the supplied pointCache
//    pointCache    - if non-null, save or restore points from this list base
//					  on the cachingMode
//    handle	    - handle to the attribute on the tweak node where the
//					  tweaks should be stored
void compCurveDeformable::tweakUsing (const MMatrix &mat, const MObjectArray &componentList, MVertexCachingMode cachingMode, MPointArray *pointCache, MArrayDataHandle &handle) {
	compCurveData *curve =cachedCurve () ;
	if ( curve == NULL )
		return ;
	MPointArray& curvePoints =curve->points () ;
	bool savePoints =(cachingMode == MPxSurfaceShape::kSavePoints) ;
	bool updatePoints =(cachingMode == MPxSurfaceShape::kUpdatePoints) ;
	MArrayDataBuilder builder =handle.builder () ;
	MPoint delta, currPt, newPt ;
	unsigned int i =0 ;
	unsigned int len =componentList.length () ;
	unsigned int cacheIndex =0 ;
	unsigned int cacheLen =(pointCache != NULL) ? pointCache->length () : 0 ;
	if ( cachingMode == MPxSurfaceShape::kRestorePoints ) {
		// restore points from the pointCache
		if ( len > 0 ) {
			// traverse the component list
			for ( i =0 ; i < len ; i++ ) {
				MObject comp =componentList [i] ;
				MFnSingleIndexedComponent fnComp (comp) ;
				int elemCount =fnComp.elementCount () ;
				for ( int idx =0 ; idx < elemCount && cacheIndex < cacheLen ; idx++, cacheIndex++ ) {
					int elemIndex =fnComp.element (idx) ;
					double3 & pt =builder.addElement (elemIndex).asDouble3 () ;
					MPoint& cachePt =(*pointCache) [cacheIndex] ;
					pt [0] +=cachePt.x ;
					pt [1] +=cachePt.y ;
					pt [2] +=cachePt.z ;
				}
			}
		} else {
			// if the component list is of zero-length, it indicates that we
			// should transform the entire surface
			len =curvePoints.length () ;
			for ( unsigned int idx =0 ; idx < len && idx < cacheLen ; ++idx ) {
				double3 & pt =builder.addElement (idx).asDouble3 () ;
				MPoint& cachePt =(*pointCache) [cacheIndex] ;
				pt [0] +=cachePt.x ;
				pt [1] +=cachePt.y ;
				pt [2] +=cachePt.z ;
			}
		}
	} else {
		// Tweak the points. If savePoints is true, also save the tweaks in the
		// pointCache. If updatePoints is true, add the new tweaks to the existing
		// data in the pointCache.
		if ( len > 0 ) {
			for ( i =0 ; i < len ; i++ ) {
				MObject comp =componentList [i] ;
				MFnSingleIndexedComponent fnComp (comp) ;
				int elemCount =fnComp.elementCount () ;
				if ( savePoints )
					pointCache->setSizeIncrement (elemCount) ;
				for ( int idx =0 ; idx < elemCount ; idx++ ) {
					int elemIndex =fnComp.element (idx) ;
					double3 & pt =builder.addElement (elemIndex).asDouble3 () ;
					currPt =newPt =curvePoints [elemIndex] ;
					newPt *=mat ;
					delta.x =newPt.x - currPt.x ;
					delta.y =newPt.y - currPt.y ;
					delta.z =newPt.z - currPt.z ;
					pt [0] +=delta.x ;
					pt [1] +=delta.y ;
					pt [2] +=delta.z ;
					if ( savePoints ) {
						// store the points in the pointCache for undo
						pointCache->append (delta * -1.0) ;
					} else if ( updatePoints && cacheIndex < cacheLen ) {
						MPoint& cachePt =(*pointCache) [cacheIndex] ;
						cachePt [0] -=delta.x ;
						cachePt [1] -=delta.y ;
						cachePt [2] -=delta.z ;
						cacheIndex++ ;
					}
				}
			}
		} else {
			// if the component list is of zero-length, it indicates that we
			// should transform the entire surface
			len =curvePoints.length () ;
			if ( savePoints )
				pointCache->setSizeIncrement (len) ;
			for ( unsigned int idx =0 ; idx < len ; ++idx ) {
				double3 & pt =builder.addElement (idx).asDouble3 () ;
				currPt =newPt =curvePoints [idx] ;
				newPt *=mat ;
				delta.x =newPt.x - currPt.x ;
				delta.y =newPt.y - currPt.y ;
				delta.z =newPt.z - currPt.z ;
				pt [0] +=delta.x ;
				pt [1] +=delta.y ;
				pt [2] +=delta.z ;
				if ( savePoints ) {
					// store the points in the pointCache for undo
					pointCache->append (delta * -1.0) ;
				} else if ( updatePoints && idx < cacheLen ) {
					MPoint& cachePt =(*pointCache) [idx] ;
					cachePt [0] -=delta.x ;
					cachePt [1] -=delta.y ;
					cachePt [2] -=delta.z ;
				}
			}
		}
	}
	// Set the builder into the handle.
	handle.set (builder) ;
	// Tell maya the bounding box for this object has changed
	// and thus "boundingBox()" needs to be called.
	childChanged (MPxSurfaceShape::kBoundingBoxChanged) ;
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
bool compCurveDeformable::getInternalValue (const MPlug &plug, MDataHandle &result) {
	bool isOk =true ;
	if (   (plug ==mControlPoints)
		|| (plug ==mControlValueX)
		|| (plug ==mControlValueY)
		|| (plug ==mControlValueZ)
	) {
		if ( fHasHistory ) {
			return (MPxSurfaceShape::getInternalValue (plug, result)) ;
		} else {
			compCurveData *curve =cachedCurve () ;
			if ( curve != NULL ) {
				MPointArray& curvePoints =curve->points () ;
				double val =0.0 ;
				if ( (plug ==mControlPoints) && !plug.isArray () ) {
					int index =plug.logicalIndex () ;
					const MPoint& pnt =curvePoints [index] ;
					result.set (pnt [0], pnt [1], pnt [2]) ;
				} else if ( plug ==mControlValueX ) {
					MPlug parentPlug =plug.parent () ;
					int index =parentPlug.logicalIndex () ;
					val =curvePoints [index].x ;
					result.set (val) ;
				} else if ( plug ==mControlValueY ) {
					MPlug parentPlug =plug.parent () ;
					int index =parentPlug.logicalIndex () ;
					val =curvePoints [index].y ;
					result.set (val) ;
				} else if ( plug ==mControlValueZ ) {
					MPlug parentPlug =plug.parent () ;
					int index =parentPlug.logicalIndex () ;
					val =curvePoints [index].z ;
					result.set (val) ;
				}
			}
		}
	} else if ( plug == mHasHistoryOnCreate ) {
		result.set (fHasHistory) ;
	} else {
		isOk =MPxSurfaceShape::getInternalValue (plug, result) ;
	}
	return (isOk) ;
}

// Converts the given component values into a selection list of plugs.
// This method is used to map components to attributes.
//     component - the component to be translated to a plug/attribute
//     list - a list of plugs representing the passed in component
void compCurveDeformable::componentToPlugs (MObject &component, MSelectionList &list) const {
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
// For instance, select commands such as "select shape1.cv[0:7]"
// are validated with this method and the corresponding component
// is added to the selection list.
//
//    item - DAG selection item for the object being matched
//    spec - attribute specification object
//    list - list to add components to
MPxSurfaceShape::MatchResult compCurveDeformable::matchComponent (const MSelectionList &item, const MAttributeSpecArray &spec, MSelectionList &list) {
	MPxSurfaceShape::MatchResult result =MPxSurfaceShape::kMatchOk ;
	MAttributeSpec attrSpec =spec [0] ;
	int dim =attrSpec.dimensions () ;

	// Look for attributes specifications of the form :
	// vtx [ index ]
	// vtx [ lower:upper ]
	if ( (spec.length () == 1) && (dim > 0) && (attrSpec.name () == "cv") ) {
		int numVertices =sCurvePointCount ;
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

// Check for matches between selection type / component list, and
// the type of this shape / or it's components
// This is used by sets and deformers to make sure that the selected
// components fall into the "vertex only" category.
//		mask          - selection type mask
//		componentList - possible component list
bool compCurveDeformable::match (const MSelectionMask &mask, const MObjectArray &componentList) const {
	bool result =false ;
	if ( componentList.length () == 0 ) {
		result =mask.intersects (MSelectionMask::kSelectMeshes) ;
	} else {
		for ( int i =0 ; i < (int)componentList.length () ; i++ ) {
			if (   (componentList [i].apiType () == MFn::kCurveCVComponent )
				&& (mask.intersects (MSelectionMask::kSelectVertices))
			) {
				result =true ;
				break ;
			}
		}
	}
	return (result);
}

// Creates a geometry iterator compatible with this shape.
//    componentList - list of components to be iterated
//    components - component to be iterator
//    forReadOnly -
MPxGeometryIterator *compCurveDeformable::geometryIteratorSetup (MObjectArray &componentList, MObject &components, bool forReadOnly) {
	compCurveDeformableIterator *result =NULL ;
	compCurveData *curve =getCurveAttr (outputSurface) ;
	if ( curve != NULL ) {
		MPointArray& points =curve->points () ;
		if ( components.isNull () )
			result =new compCurveDeformableIterator (&points, componentList) ;
		else
			result =new compCurveDeformableIterator (&points, components) ;
	}
	return (result) ;
}

// Specifies that this shape can provide an iterator for getting/setting
// control point values.
//      writable - maya asks for an iterator that can set points if this is true
bool compCurveDeformable::acceptsGeometryIterator (bool writeable) {
	return (true) ;
}

// Specifies that this shape can provide an iterator for getting/setting
// control point values.
//     writable - maya asks for an iterator that can set points if this is true
//     forReadOnly - maya asking for an iterator for querying only
bool compCurveDeformable::acceptsGeometryIterator (MObject &, bool writeable, bool forReadOnly) {
	return (true) ;
}

// Keep track of whether there is history
MStatus compCurveDeformable::connectionMade (const MPlug &plug, const MPlug &otherPlug, bool asSrc) {
	if ( !asSrc && plug == inputSurface )
		fHasHistory =true ;
	return (MPxSurfaceShape::connectionMade (plug, otherPlug, asSrc)) ;
}

// Track whether the shape has history.
MStatus compCurveDeformable::connectionBroken (const MPlug &plug, const MPlug &otherPlug, bool asSrc) {
	if ( !asSrc && plug == inputSurface )
		fHasHistory =false ;
	return (MPxSurfaceShape::connectionBroken (plug, otherPlug, asSrc)) ;
}

// This method is used by Maya when it needs to create a component
// containing every vertex (or control point) in the shape.
// This will get called if you apply some deformer to the whole
// shape, i.e. select the shape in object mode and add a deformer to it.
MObject compCurveDeformable::createFullVertexGroup () const {
	// Create a vertex component
	MFnSingleIndexedComponent fnComponent ;
	MObject fullComponent =fnComponent.create (MFn::kCurveCVComponent) ;
	// Set the component to be complete, i.e. the elements in
	// the component will be [0:numVertices-1]
	unsigned numPoints =0 ;
	compCurveDeformable *nonConstThis =const_cast<compCurveDeformable *>(this) ;
	compCurveData *curve =nonConstThis->cachedCurve () ;
	if ( curve != NULL )
		numPoints =curve->points ().length () ;
	fnComponent.setCompleteData (numPoints) ;
	return (fullComponent) ;
}

// Returns the input attribute of the shape. This is used by
// maya to establish input connections for deformers etc.
// This attribute must be data of tye kGeometryData.
MObject compCurveDeformable::localShapeInAttr () const {
	return (inputSurface) ;
}

// Returns the output attribute of the shape. This is used by
// maya to establish out connections for deformers etc.
// This attribute must be data of tye kGeometryData.
MObject compCurveDeformable::localShapeOutAttr () const {
	return (outputSurface) ;
}

// Returns the world space output "array" attribute of the shape.
// This is used by maya to establish out connections for deformers etc.
// This attribute must be an array attribute, each element representing
// a particular instance of the shape.
// This attribute must be data of type kGeometryData.
MObject compCurveDeformable::worldShapeOutAttr () const {
	return (worldSurface) ;
}

// Returns the cached shape attribute of the shape.
// This attribute must be data of type kGeometryData.
MObject compCurveDeformable::cachedShapeAttr () const {
	return (cachedSurface) ;
}

// Returns the data object for the surface. This gets
// called internally for grouping (set) information.
MObject compCurveDeformable::geometryData () const {
	compCurveDeformable *nonConstThis =(compCurveDeformable *)this ;
	MDataBlock datablock =nonConstThis->forceCache () ;
	MDataHandle handle =datablock.inputValue (inputSurface) ;
	return (handle.data ()) ;
}

// Place the tweaks into the controlPoints attribute
void compCurveDeformable::updateControlPoints (const MPointArray &curvePoints) {
	compCurveData *curve =getCurveAttr (inputSurface) ;
	if ( curve == NULL )
		return ;
	MDataBlock datablock =forceCache () ;
	MArrayDataHandle cpH =datablock.outputArrayValue (mControlPoints) ;
	MArrayDataBuilder oldBuilder =cpH.builder () ;
	unsigned pointCount =curvePoints.length () ;
	if ( pointCount != oldBuilder.elementCount () ) {
		MArrayDataBuilder builder (oldBuilder) ;
		for ( unsigned ii =0 ; ii < pointCount ; ii++ )
			builder.addElement (ii).asDouble3 () ;
		cpH.set (builder) ;
	}
	cpH.setAllClean () ;
	MPointArray& inputCurvePoints =curve->points () ;
	cpH =datablock.outputArrayValue (mControlPoints) ;
	for ( unsigned ii =0 ; ii < pointCount ; ii++ ) {
		MDataHandle pntHandle =cpH.outputValue () ;
		double3& pnt =pntHandle.asDouble3 () ;
		pnt[0] =curvePoints [ii].x - inputCurvePoints [ii].x ;
		pnt[1] =curvePoints [ii].y - inputCurvePoints [ii].y ;
		pnt[2] =curvePoints [ii].z - inputCurvePoints [ii].z ;
		cpH.next () ;
	}
}

// Get the current curve data.
compCurveData *compCurveDeformable::getCurveAttr (const MObject &attribute) {
	// Get the datablock for this node
	MDataBlock datablock =forceCache () ;
	// Calling inputValue will force a recompute if the
	// connection is dirty. This means the most up-to-date
	// mesh data will be returned by this method.
	MDataHandle handle =datablock.inputValue (attribute) ;
	MObject dataObj =handle.data () ;
	MFnPluginData fnData (dataObj) ;
	compCurveData *result =(compCurveData *)fnData.data () ;
	return (result) ;
}

// Get the current cached curve data.
compCurveData *compCurveDeformable::cachedCurve () {
	// Get the datablock for this node
	MDataBlock datablock =forceCache () ;
	MDataHandle handle =datablock.outputValue (cachedSurface) ;
	MObject dataObj =handle.data () ;
	MFnPluginData fnData (dataObj) ;
	compCurveData *result =(compCurveData *)fnData.data () ;
	if ( result == NULL ) {
		MFnPluginData fnDataCreator ;
		MTypeId tmpid (compCurveData::id) ;
		MObject newDataObject =fnDataCreator.create (tmpid) ;
		handle.set (newDataObject) ;
		dataObj =handle.data () ;
		fnData.setObject (dataObj) ;
		result =(compCurveData *)fnData.data () ;
	}
	return (result) ;
}

void compCurveDeformable::applyTweaks (MDataBlock &block) {
	compCurveData *cached =cachedCurve () ;
	compCurveData *input =getCurveAttr (inputSurface) ;
	MPointArray& basicCurvePoints =cached->points () ;
	MPointArray& inputCurvePoints =input->points () ;

	MArrayDataHandle hPoints =block.inputArrayValue (mControlPoints) ;
	for ( unsigned idx=0 ; idx < sCurvePointCount ; idx++ ) {
		if ( hPoints.jumpToElement (idx) == MS::kSuccess ) {
			MDataHandle pntHandle =hPoints.outputValue () ;
			double3& pnt =pntHandle.asDouble3 () ;
			// Apply the tweaks to the data
			MPoint& origPoint =inputCurvePoints [idx] ;
			MPoint& newPoint =basicCurvePoints [idx] ;
			newPoint.x =origPoint.x + pnt [0] ;
			newPoint.y =origPoint.y + pnt [1] ;
			newPoint.z =origPoint.z + pnt [2] ;
			newPoint.w =1.0 ;
		} else {
			basicCurvePoints [idx] =inputCurvePoints [idx] ;
		}
	}
}

void compCurveDeformable::points (MPointArray &points) {
	compCurveData *curve =getCurveAttr (outputSurface) ;
	if ( curve != NULL ) {
		MPointArray& basicCurvePoints =curve->points () ;
		points.setLength (sCurvePointCount) ;
		for ( unsigned ii =0 ; ii < sCurvePointCount ; ii++ )
			points.set (basicCurvePoints [ii], ii) ;
	} else {
		points.clear () ;
	}
}

void compCurveDeformable::setPointValue (const MPlug &plug, const MDataHandle &handle, unsigned vtInd) {
	MPlug parentPlug =plug.parent () ;
	unsigned index =parentPlug.logicalIndex () ;
	compCurveData *curve =cachedCurve () ;
	if ( curve != NULL && index < sCurvePointCount ) {
		MPointArray& curvePoints =curve->points () ;
		curvePoints [index] [vtInd] =handle.asDouble () ;
		curvePoints [index].w =1.0 ;
	}
}

//-----------------------------------------------------------------------------
// Called internally to create a new instance of the users MPx node.
void *compCurveDeformable::creator () {
	return (new compCurveDeformable ()) ;
}

// Attribute (static) initialization.
// See api_macros.h.
MStatus compCurveDeformable::initialize () { 
	// bbox attributes
	MAKE_NUMERIC_ATTR(bboxCorner1, "bboxCorner1", "bb1", MFnNumericData::k3Double, 0, false, false, false) ;
	MAKE_NUMERIC_ATTR(bboxCorner2, "bboxCorner2", "bb2", MFnNumericData::k3Double, 0, false, false, false) ;
	
	MStatus stat ;
	MFnTypedAttribute typedAttr ;
	inputSurface =typedAttr.create ("inputSurface", "is", compCurveData::id, MObject::kNullObj, &stat) ;
	MCHECKERROR (stat, "create inputSurface attribute")
	typedAttr.setStorable (false) ;
	ADD_ATTRIBUTE(inputSurface) ;

	// local/world output surface attributes
	//
	outputSurface = typedAttr.create( "outputSurface", "os",
		compCurveData::id,
		MObject::kNullObj, &stat );
	ADD_ATTRIBUTE( outputSurface );
	typedAttr.setWritable( false );

	worldSurface = typedAttr.create( "worldSurface", "ws",
		compCurveData::id,
		MObject::kNullObj, &stat );
	typedAttr.setCached( false );
	typedAttr.setWritable( false );
	stat = typedAttr.setArray( true );
	stat = typedAttr.setUsesArrayDataBuilder( true );
	stat = typedAttr.setDisconnectBehavior( MFnAttribute::kDelete );

	ADD_ATTRIBUTE( worldSurface );

	// Cached surface used for file IO
	//
	cachedSurface = typedAttr.create( "cachedSurface", "cs",
		compCurveData::id,
		MObject::kNullObj, &stat );
	MCHECKERROR( stat, "create cachedSurface attribute" )
		typedAttr.setReadable( true );
	typedAttr.setWritable( true );
	typedAttr.setStorable( true );
	ADD_ATTRIBUTE( cachedSurface );

	ATTRIBUTE_AFFECTS(inputSurface, outputSurface)
	ATTRIBUTE_AFFECTS(inputSurface, worldSurface)
	ATTRIBUTE_AFFECTS(cachedSurface, outputSurface)
	ATTRIBUTE_AFFECTS(cachedSurface, worldSurface)

	ATTRIBUTE_AFFECTS(mControlPoints, outputSurface)
	ATTRIBUTE_AFFECTS(mControlValueX, outputSurface)
	ATTRIBUTE_AFFECTS(mControlValueY, outputSurface)
	ATTRIBUTE_AFFECTS(mControlValueZ, outputSurface)
	ATTRIBUTE_AFFECTS(mControlPoints, cachedSurface)
	ATTRIBUTE_AFFECTS(mControlValueX, cachedSurface)
	ATTRIBUTE_AFFECTS(mControlValueY, cachedSurface)
	ATTRIBUTE_AFFECTS(mControlValueZ, cachedSurface)
	ATTRIBUTE_AFFECTS(mControlPoints, worldSurface)
	ATTRIBUTE_AFFECTS(mControlValueX, worldSurface)
	ATTRIBUTE_AFFECTS(mControlValueY, worldSurface)
	ATTRIBUTE_AFFECTS(mControlValueZ, worldSurface)

	ATTRIBUTE_AFFECTS(inputSurface, bboxCorner1)
	ATTRIBUTE_AFFECTS(inputSurface, bboxCorner2)
	ATTRIBUTE_AFFECTS(mControlPoints, bboxCorner1)
	ATTRIBUTE_AFFECTS(mControlValueX, bboxCorner1)
	ATTRIBUTE_AFFECTS(mControlValueY, bboxCorner1)
	ATTRIBUTE_AFFECTS(mControlValueZ, bboxCorner1)
	ATTRIBUTE_AFFECTS(mControlPoints, bboxCorner2)
	ATTRIBUTE_AFFECTS(mControlValueX, bboxCorner2)
	ATTRIBUTE_AFFECTS(mControlValueY, bboxCorner2)
	ATTRIBUTE_AFFECTS(mControlValueZ, bboxCorner2)

	return (MS::kSuccess) ;
}

MStatus compCurveDeformable::registerMe (MFnPlugin &plugin) {
	return (plugin.registerShape (
		_T("compCurveDeformable"),
		compCurveDeformable::id,
		compCurveDeformable::creator,
		compCurveDeformable::initialize,
		compCurveDeformableUI::creator
	)) ;
}

MStatus compCurveDeformable::unregisterMe (MFnPlugin &plugin) {
	return (plugin.deregisterNode (compCurveDeformable::id)) ;
}
