//
//  Copyright 2012 Autodesk, Inc.  All rights reserved.
//
//  Use of this software is subject to the terms of the Autodesk license 
//  agreement provided at the time of installation or download, or which 
//  otherwise accompanies this software in either electronic or hard copy form.   
//

// Provides a data type for the user-defined comp curve
#pragma once

class compCurveData : public MPxGeometryData {
public:
	static const MString typeName ;
	static const MTypeId id ;

private:	
	// This is the geometry our data will pass though the DG
	MPointArray fPoints ;

public:
	compCurveData () ;
	virtual ~compCurveData () ;

	// Overrides from MPxData
	virtual MStatus readASCII (const MArgList& argList, unsigned& idx) ;
	virtual MStatus readBinary (istream& in, unsigned length) ;
	virtual MStatus writeASCII (ostream& out) ;
	virtual MStatus writeBinary (ostream& out) ;

	virtual	void copy (const MPxData&) ;

	virtual MTypeId typeId () const ;
	virtual MString name () const ;

	// Overrides from MPxGeometryData
	virtual MPxGeometryIterator *iterator (MObjectArray &componentList, MObject &component, bool useComponents) ;
	virtual MPxGeometryIterator *iterator (MObjectArray &componentList, MObject &component, bool useComponents, bool world) const ;
	virtual bool updateCompleteVertexGroup (MObject &component) const ;

	static void *creator () ;
	static MStatus registerMe (MFnPlugin &plugin) ;
	static MStatus unregisterMe (MFnPlugin &plugin) ;

	MPointArray &points () { return (fPoints) ; }

	virtual void smartCopy (const MPxGeometryData *srcGeom) ;

} ;
