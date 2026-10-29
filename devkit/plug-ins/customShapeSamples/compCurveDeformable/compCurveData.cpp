//
//  Copyright 2012 Autodesk, Inc.  All rights reserved.
//
//  Use of this software is subject to the terms of the Autodesk license 
//  agreement provided at the time of installation or download, or which 
//  otherwise accompanies this software in either electronic or hard copy form.   
//
#include "StdAfx.h"

#include "compCurveData.h"
#include "compCurveDeformableIterator.h"
#include "api_macros.h"

// Ascii file IO defines
#define kDblQteChar "\""
#define kSpaceChar "	"
#define kWrapString "\n\t\t"
#define kVertexKeyword "v"

const MTypeId compCurveData::id (0x8101C) ;
const MString compCurveData::typeName ("compCurveData") ;

compCurveData::compCurveData () {
	fPoints.setLength (4) ;
	for ( unsigned ii =0 ; ii < 4 ; ii++ ) {
		MPoint dummy (0.0, 0.0, 0.0) ;
		fPoints.set (dummy, ii) ;
	}
}

compCurveData::~compCurveData () {
}

// ASCII file input method.
MStatus compCurveData::readASCII (const MArgList &argList, unsigned &index) {
	MString geomStr ;
	MPoint vertex ;
	int vertexCount =0 ;

	MStatus result =argList.get (index, geomStr) ;
	if ( result && (geomStr == kVertexKeyword) ) {
		result =argList.get (++index, vertexCount) ;
		fPoints.setLength (vertexCount) ;
		for ( int ii =0 ; ii < vertexCount ; ii++ ) {
			if ( argList.get (++index, vertex) )
				fPoints[ii] =vertex ;
			else
				result =MS::kFailure ;
		}
	}
	return (MS::kSuccess) ;
}

// NOT IMPLEMENTED
MStatus compCurveData::readBinary (istream & /*in*/, unsigned /*length*/) {
	return (MS::kSuccess) ;
}

// ASCII file output method.
MStatus compCurveData::writeASCII (ostream &out) {
	MPoint vertex ;
	int vertexCount =fPoints.length () ;
	out << kDblQteChar << kVertexKeyword << kDblQteChar << kSpaceChar << vertexCount ;
	for ( int ii =0 ; ii < vertexCount ; ii++ ) {
		vertex =fPoints [ii] ;
		out << kWrapString ;
		out << vertex [0] << kSpaceChar ;
		out << vertex [1] << kSpaceChar ;
		out << vertex [2] ;
	}
	return (MS::kSuccess) ;
}

// NOT IMPLEMENTED
MStatus compCurveData::writeBinary (ostream & /*out*/ ) {
	return (MS::kSuccess) ;
}

// Binary tag used to identify this kind of data
MTypeId compCurveData::typeId () const {
	return (compCurveData::id) ;
}

// String name used to identify this kind of data
MString compCurveData::name () const {
	return (compCurveData::typeName) ;
}

void *compCurveData::creator () {
	return (new compCurveData ()) ;
}

MStatus compCurveData::registerMe (MFnPlugin &plugin) {
	return (plugin.registerData (
		_T("compCurveData"),
		compCurveData::id,
		compCurveData::creator,
		MPxData::kGeometryData
	)) ;
}

MStatus compCurveData::unregisterMe (MFnPlugin &plugin) {
	return (plugin.deregisterData (compCurveData::id)) ;
}


MPxGeometryIterator *compCurveData::iterator (MObjectArray &compList, MObject &component, bool useComponents) {
	compCurveDeformableIterator *result =NULL ;
	MPointArray *points =const_cast<MPointArray *>(&fPoints) ;
	if ( useComponents )
		result =new compCurveDeformableIterator (points, compList) ;
	else
		result =new compCurveDeformableIterator (points, component) ;
	return (result) ;
}

MPxGeometryIterator *compCurveData::iterator (MObjectArray &compList, MObject &component, bool useComponents, bool /*world*/) const {
	compCurveDeformableIterator *result =NULL ;
	MPointArray *points =const_cast<MPointArray *>(&fPoints) ;
	if ( useComponents )
		result =new compCurveDeformableIterator (points, compList) ;
	else
		result =new compCurveDeformableIterator (points, component) ;
	return (result) ;
}

void compCurveData::copy (const MPxData &otherData) {
	compCurveData *other =(compCurveData *)&otherData ;
	fPoints =other->points () ;
}

// Make sure complete vertex group data is up-to-date.
// Returns true if the component was updated, false if it was already ok.
// This is used by deformers when deforming the "whole" object and
// not just selected components.
bool compCurveData::updateCompleteVertexGroup (MObject &component) const {
	MStatus stat ;
	MFnSingleIndexedComponent fnComponent (component, &stat) ;
	// Make sure there is non-null geometry and that the component
	// is "complete". A "complete" component represents every 
	// vertex in the shape.
	if ( stat && (fnComponent.isComplete ()) ) {
		int maxVerts ;
		fnComponent.getCompleteData (maxVerts) ;
		int numVertices =fPoints.length () ;
		if ( (numVertices > 0) && (maxVerts != numVertices) ) {
			// Set the component to be complete, i.e. the elements in
			// the component will be [0:numVertices-1]
			fnComponent.setCompleteData (numVertices) ;
			return (true) ;
		}
	}
	return (false) ;
}

void compCurveData::smartCopy (const MPxGeometryData *srcGeom) {
	compCurveData *curveData =(compCurveData *)srcGeom ;
	fPoints =curveData->fPoints ;
}
