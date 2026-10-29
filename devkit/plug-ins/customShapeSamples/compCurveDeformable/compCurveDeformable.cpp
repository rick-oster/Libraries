//
//  Copyright 2012 Autodesk, Inc.  All rights reserved.
//
//  Use of this software is subject to the terms of the Autodesk license 
//  agreement provided at the time of installation or download, or which 
//  otherwise accompanies this software in either electronic or hard copy form.   
//

//-----------------------------------------------------------------------------
//- compCurveDeformable.cpp : Initialization functions
//-----------------------------------------------------------------------------
#include "StdAfx.h"
#include "resource.h"

#include <maya/MFnPlugin.h>

#include "compCurveData.h"
#include "compCurveDeformableIterator.h"
#include "compCurveDeformableShape.h"
#include "compCurveDeformableShapeUI.h"

//-----------------------------------------------------------------------------
MStatus initializePlugin (MObject obj) {
	#pragma EXPORT
	//	Description:
	//		this method is called when the plug-in is loaded into Maya. It 
	//		registers all of the services that this plug-in provides with 
	//		Maya.
	//	Arguments:
	//		obj - a handle to the plug-in object (use MFnPlugin to access it)
	MFnPlugin plugin (obj, _T("Vendor Name"), _T("Version"), _T("Any")) ;

	//- Examples:
	//NodeRegisterOk(plugin.registerNode (_T("myNode"), myNode::id, myNode::creator, myNode::initialize)) ;
	//NodeRegisterOk(myNode::registerMe (plugin)) ;

	//-{{MAYA_REGISTER
	//-MAYA_REGISTER}}
	compCurveData::registerMe (plugin) ;
	compCurveDeformable::registerMe (plugin) ;

	return (MS::kSuccess) ;
}

MStatus uninitializePlugin (MObject obj) {
	#pragma EXPORT
	//	Description:
	//		this method is called when the plug-in is unloaded from Maya. It 
	//		deregisters all of the services that it was providing.
	//	Arguments:
	//		obj - a handle to the plug-in object (use MFnPlugin to access it)
	MFnPlugin plugin (obj) ;

	//- Examples:
	//NodeUnregisterOk(plugin.deregisterNode (myNode::id)) ;
	//NodeUnregisterOk(myNode::unregisterMe (plugin)) ;
	
	//-{{MAYA_UNREGISTER
	//-MAYA_UNREGISTER}}
	compCurveDeformable::unregisterMe (plugin) ;
	compCurveData::unregisterMe (plugin) ;

	return (MS::kSuccess) ;
}
