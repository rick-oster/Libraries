//
//  Copyright 2012 Autodesk, Inc.  All rights reserved.
//
//  Use of this software is subject to the terms of the Autodesk license 
//  agreement provided at the time of installation or download, or which 
//  otherwise accompanies this software in either electronic or hard copy form.   
//
#include "StdAfx.h"

#include "compCurveDrawSelShape.h" 
#include "compCurveDrawSelShapeUI.h"

// Object and component color defines
#define LEAD_COLOR				18	// green
#define ACTIVE_COLOR			15	// white
#define ACTIVE_AFFECTED_COLOR	8	// purple
#define DORMANT_COLOR			4	// blue
#define HILITE_COLOR			17	// pale blue

#define LINE_WIDTH				4.0

//-----------------------------------------------------------------------------
compCurveDrawSelUI::compCurveDrawSelUI () {
}

compCurveDrawSelUI::~compCurveDrawSelUI () {
}

void *compCurveDrawSelUI::creator () {
	return (new compCurveDrawSelUI ()) ;
}

//-----------------------------------------------------------------------------
// Overrides

// Add draw requests to the draw queue
//     info                 - current drawing state
//     objectsAndActiveOnly - no components if true
//     queue                - queue of draw requests to add to
void compCurveDrawSelUI::getDrawRequests (const MDrawInfo &info, bool objectAndActiveOnly, MDrawRequestQueue &queue) {
	// Get the data necessary to draw the shape
	MDrawData data ;
	compCurveDrawSel *compCurveNode =(compCurveDrawSel *)surfaceShape () ;
	if ( compCurveNode == NULL )
		cerr << "NO DrawRequest for compCurveDrawSel\n" ;

	// This call creates a prototype draw request that we can fill
	// in and then add to the draw queue.
	MDrawRequest request =info.getPrototype (*this) ;
	getDrawData (compCurveNode , data) ;
	request.setDrawData (data) ;

	// Set the color based on the display status
	M3dView::DisplayStatus displayStatus =info.displayStatus () ;
	M3dView::ColorTable activeColorTable =M3dView::kActiveColors ;
	M3dView::ColorTable dormantColorTable =M3dView::kDormantColors ;

	switch ( displayStatus ) {
		case M3dView::kLead:
			request.setColor (LEAD_COLOR, activeColorTable) ;
			break ;
		case M3dView::kActive:
			request.setColor (ACTIVE_COLOR, activeColorTable) ;
			break ;
		case M3dView::kActiveAffected:
			request.setColor (ACTIVE_AFFECTED_COLOR, activeColorTable) ;
			break ;
		case M3dView::kDormant:
			request.setColor (DORMANT_COLOR, dormantColorTable) ;
			break ;
		case M3dView::kHilite:
			request.setColor (HILITE_COLOR, activeColorTable) ;
			break ;
		default:	
			break ;
	}
	queue.add (request) ;
}

// Main (OpenGL) draw routine. We implement it to invoke a separate
// draw routine which is also called from select.
//     request - request to be drawn
//     view    - view to draw into
void compCurveDrawSelUI::draw (const MDrawRequest &/* request */, M3dView &view) const {
	drawWireframe (view) ;
}

void compCurveDrawSelUI::drawWireframe (M3dView &view) const { 
	view.beginGL () ;
	// Draw the wireframe compCurve
	compCurveDrawSel *compCurveNode =(compCurveDrawSel *)surfaceShape () ;
	MPointArray points ;
	compCurveNode->points (points) ;

	GLfloat currWidth ; 
	glGetFloatv (GL_LINE_WIDTH, &currWidth) ;
	glLineWidth (LINE_WIDTH) ;
	glBegin (GL_LINES) ;
	unsigned steps =20 ;
	const MPoint &vertex0 =points [0] ;
	const MPoint &vertex3 =points [3] ;
	MVector tangent0 =points [1] - points [0] ;
	MVector tangent1 =points [3] - points [2] ;
	MPoint previous =vertex0 ;
	MVector vertex0v (vertex0.x, vertex0.y, vertex0.z) ;
	MVector vertex3v (vertex3.x, vertex3.y, vertex3.z) ;
	for ( unsigned ii =1 ; ii < steps + 1 ; ++ii ) {
		double s  =(double)ii / (double)steps ;
		double s2 =s * s ;
		double s3 =s2* s ;
		double h1 =2.0 * s3 - 3.0 * s2 + 1 ;
		double h2 =-2.0 * s3 + 3.0 * s2 ;
		double h3 =s3 - 2.0 * s2 + s ;
		double h4 =s3 - s2 ;
		MVector next =h1 * vertex0v + h2 * vertex3v + h3 * tangent0 + h4 * tangent1 ;

		glVertex3f ((float)previous [0], (float)previous [1], (float)previous [2]) ;
		glVertex3f ((float)next [0], (float)next [1], (float)next [2]) ;
		previous.x =next.x ;
		previous.y =next.y ;
		previous.z =next.z ;
	}
	glEnd () ;

	view.endGL () ; 
	glLineWidth (currWidth) ;
}

// Main selection routine. Use OpenGL selection.
//     selectInfo           - the selection state information
//     selectionList        - the list of selected items to add to
//     worldSpaceSelectPts  -
bool compCurveDrawSelUI::select (MSelectInfo &selectInfo, MSelectionList &selectionList, MPointArray &worldSpaceSelectPts) const {
	bool selected =false ;
	M3dView myView =selectInfo.view () ;
	myView.beginSelect () ;
	drawWireframe (myView) ;
	selected =(myView.endSelect () > 0) ;
	if ( selected ) {
		MSelectionMask priorityMask (MSelectionMask::kSelectNurbsCurves) ;
		MSelectionList item ;
		item.add (selectInfo.selectPath ()) ;
		MPoint xformedPt ;
		if ( selectInfo.singleSelection () ) {
			compCurveDrawSel *compCurveNode =(compCurveDrawSel *)surfaceShape () ;
			MPoint center =compCurveNode->boundingBox ().center () ;
			xformedPt =center ;
			xformedPt *=selectInfo.selectPath ().inclusiveMatrix () ;
		}
		selectInfo.addSelection (item, xformedPt, selectionList, worldSpaceSelectPts, priorityMask, false) ;
	}
	return (selected) ;
}
