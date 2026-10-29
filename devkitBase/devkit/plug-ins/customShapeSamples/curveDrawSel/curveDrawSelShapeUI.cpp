//
//  Copyright 2012 Autodesk, Inc.  All rights reserved.
//
//  Use of this software is subject to the terms of the Autodesk license 
//  agreement provided at the time of installation or download, or which 
//  otherwise accompanies this software in either electronic or hard copy form.   
//
#include "StdAfx.h"

#include "curveDrawSelShapeUI.h"
#include "curveDrawSelShape.h"

// Object and component color defines
#define LEAD_COLOR				18	// green
#define ACTIVE_COLOR			15	// white
#define ACTIVE_AFFECTED_COLOR	8	// purple
#define DORMANT_COLOR			4	// blue
#define HILITE_COLOR			17	// pale blue

#define LINE_WIDTH				4.0

//-----------------------------------------------------------------------------
curveDrawSelUI::curveDrawSelUI () {
}

curveDrawSelUI::~curveDrawSelUI () {
}

void *curveDrawSelUI::creator () {
	return (new curveDrawSelUI ()) ;
}

// Overrides

// Add draw requests to the draw queue
//     info                 - current drawing state
//     objectsAndActiveOnly - no components if true
//     queue                - queue of draw requests to add to
void curveDrawSelUI::getDrawRequests (const MDrawInfo &info, bool objectAndActiveOnly, MDrawRequestQueue &queue) {
	// Get the data necessary to draw the shape
	MDrawData data ;
	curveDrawSel *curveNode =(curveDrawSel *)surfaceShape () ;
	if ( curveNode == NULL )
		cerr << "NO DrawRequest for curveDrawSel\n";

	// This call creates a prototype draw request that we can fill
	// in and then add to the draw queue.
	MDrawRequest request =info.getPrototype (*this) ;
	getDrawData (curveNode , data) ;
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
void curveDrawSelUI::draw (const MDrawRequest &/* request */, M3dView &view) const {
	drawWireframe (view) ;
}

void curveDrawSelUI::drawWireframe (M3dView &view) const { 
	view.beginGL () ;

	// Draw the wireframe curve
	curveDrawSel *curveNode =(curveDrawSel *)surfaceShape () ;

	MPointArray points ;
	curveNode->points (points) ;
	unsigned pointCount =points.length () ;

	GLfloat currWidth ;
	glGetFloatv (GL_LINE_WIDTH, &currWidth) ;
	glLineWidth (LINE_WIDTH) ;
	glBegin (GL_LINES) ;
	for ( unsigned ii =0 ; ii < pointCount - 1 ; ++ii ) {
		MPoint vertex =points [ii] ;
		MPoint vertex1 =points [ii + 1] ;
		glVertex3f ((float)vertex [0], (float)vertex [1], (float)vertex [2]) ;
		glVertex3f ((float)vertex1 [0], (float)vertex1 [1], (float)vertex1 [2]) ;
	}
	glEnd () ;

	view.endGL () ;
	glLineWidth (currWidth) ;
}

// Main selection routine. Use OpenGL selection.
//     selectInfo           - the selection state information
//     selectionList        - the list of selected items to add to
//     worldSpaceSelectPts  -
bool curveDrawSelUI::select (MSelectInfo &selectInfo, MSelectionList &selectionList, MPointArray &worldSpaceSelectPts ) const {
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
			curveDrawSel *curveNode =(curveDrawSel *)surfaceShape () ;
			MPoint center =curveNode->boundingBox ().center () ;
			xformedPt =center ;
			xformedPt *=selectInfo.selectPath ().inclusiveMatrix () ;
		}
		selectInfo.addSelection (item, xformedPt, selectionList, worldSpaceSelectPts, priorityMask, false) ;
	}
	return (selected) ;
}

