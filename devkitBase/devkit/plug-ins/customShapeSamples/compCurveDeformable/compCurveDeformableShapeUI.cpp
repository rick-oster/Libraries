//
// Copyright 2012 Autodesk, Inc. All rights reserved.
//
// Use of this software is subject to the terms of the Autodesk license 
// agreement provided at the time of installation or download, or which 
// otherwise accompanies this software in either electronic or hard copy form. 
//
#include "StdAfx.h"

#include "compCurveData.h"
#include "compCurveDeformableIterator.h"
#include "compCurveDeformableShape.h"
#include "compCurveDeformableShapeUI.h"

// Object and component color defines
#define LEAD_COLOR				18	// green
#define ACTIVE_COLOR			15	// white
#define ACTIVE_AFFECTED_COLOR	8	// purple
#define DORMANT_COLOR			4	// blue
#define HILITE_COLOR			17	// pale blue
#define DORMANT_VERTEX_COLOR	8	// purple
#define ACTIVE_VERTEX_COLOR		16	// yellow

#define POINT_SIZE				5.0
#define LINE_WIDTH				4.0

//-----------------------------------------------------------------------------
compCurveDeformableUI::compCurveDeformableUI () {
}

compCurveDeformableUI::~compCurveDeformableUI () {
}

//-----------------------------------------------------------------------------
void *compCurveDeformableUI::creator () {
	return (new compCurveDeformableUI ()) ;
}

//-----------------------------------------------------------------------------
// Overrides

// Add draw requests to the draw queue
//    info - current drawing state
//    objectsAndActiveOnly - no components if true
//    queue - queue of draw requests to add to
void compCurveDeformableUI::getDrawRequests (const MDrawInfo &info, bool objectAndActiveOnly, MDrawRequestQueue &queue) {
	// Get the data necessary to draw the shape
	MDrawData data ;
	compCurveDeformable *compCurveNode =(compCurveDeformable *)surfaceShape () ;
	if ( compCurveNode == NULL )
		cerr << "NO DrawRequest for compCurveDeformable\n" ;

	// This call creates a prototype draw request that we can fill
	// in and then add to the draw queue.
	MDrawRequest request =info.getPrototype (*this) ;
	getDrawData (compCurveNode , data) ;
	request.setDrawData (data) ;
	request.setToken (kDrawWireframe) ;

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
			request.setColor (DORMANT_COLOR, dormantColorTable) ;
			break ;
	}
	queue.add (request) ;

	// Add draw requests for components
	if ( !objectAndActiveOnly ) {
		// Inactive components
		M3dView::DisplayStyle appearance =info.displayStyle () ;
		if ( (appearance ==M3dView::kPoints ) || (displayStatus ==M3dView::kHilite) ) {
			MDrawRequest vertexRequest =info.getPrototype (*this) ; 
			vertexRequest.setDrawData (data) ;
			vertexRequest.setToken (kDrawVertices) ;
			vertexRequest.setColor (DORMANT_VERTEX_COLOR, M3dView::kActiveColors) ;
			queue.add (vertexRequest) ;
		}
		// Active components
		if ( surfaceShape ()->hasActiveComponents () ) {
			MDrawRequest activeVertexRequest =info.getPrototype (*this) ; 
			activeVertexRequest.setDrawData (data) ;
			activeVertexRequest.setToken (kDrawVertices) ;
			activeVertexRequest.setColor (ACTIVE_VERTEX_COLOR, M3dView::kActiveColors) ;

			MObjectArray clist =surfaceShape ()->activeComponents () ;
			MObject vertexComponent =clist [0] ; // Should filter list
			activeVertexRequest.setComponent (vertexComponent) ;
			queue.add (activeVertexRequest) ;
		}
	}
}

// Main (OpenGL) draw routine. Based on the type of drawing, draw the curve or the points.
//    request - request to be drawn
//    view - view to draw into
void compCurveDeformableUI::draw (const MDrawRequest &request, M3dView &view) const { 
	int token =request.token () ;
	MPointArray points ;
	switch ( token ) {
		case kDrawWireframe:
		case kDrawWireframeOnShaded:
			drawHermite (view) ;
			break ;
		case kDrawVertices:
			compCurveDeformable *curveNode =(compCurveDeformable *)surfaceShape () ;
			curveNode->points (points) ;
			drawPoints (request, points) ;
			break ;
	}
}

// Draw a hermite curve instead of straight line segments, mostly just for fun & to 
// make it easier to see the components since they'll be off the curve.
void compCurveDeformableUI::drawHermite (M3dView &view) const { 
	// Get the curve
	compCurveDeformable* curveNode =(compCurveDeformable*)surfaceShape () ;
	MPointArray points ;
	curveNode->points (points) ;

	view.beginGL () ; 
	GLfloat currWidth ; 
	glGetFloatv (GL_LINE_WIDTH, &currWidth) ;
	glLineWidth (LINE_WIDTH) ;
	glBegin (GL_LINES) ;
	unsigned steps =20 ;
	const MPoint& vertex0 =points [0] ;
	const MPoint& vertex3 =points [3] ;
	MVector tangent0 =(points [1] - points [0]) * 4.0 ;
	MVector tangent1 =(points [3] - points [2]) * 4.0 ;
	MPoint previous =vertex0 ;
	MVector vertex0v (vertex0.x, vertex0.y, vertex0.z) ;
	MVector vertex3v (vertex3.x, vertex3.y, vertex3.z) ;
	for ( unsigned ii =1 ; ii < steps+1 ; ++ii ) {
		double s =(double)ii / (double)steps ;
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
//    selectInfo - the selection state information
//    selectionList - the list of selected items to add to
//    worldSpaceSelectPts -
bool compCurveDeformableUI::select (MSelectInfo &selectInfo, MSelectionList &selectionList, MPointArray &worldSpaceSelectPts) const {
	bool selected =false ;
	bool componentSelected =false ;
	bool hilited =false ;

	hilited =(selectInfo.displayStatus () ==M3dView::kHilite) ;
	if ( hilited || surfaceShape ()->hasActiveComponents () ) {
		componentSelected =selectPoints (selectInfo, selectionList, worldSpaceSelectPts) ;
		selected =selected || componentSelected ;
	}
	if ( !selected ) {
		M3dView myView =selectInfo.view () ;
		myView.beginSelect () ;
		drawHermite (myView) ;
		selected =(myView.endSelect () > 0) ;
		if ( selected ) {
			compCurveDeformable *curveNode =(compCurveDeformable *)surfaceShape () ;
			MSelectionMask priorityMask (MSelectionMask::kSelectNurbsCurves) ;
			MSelectionList item ;
			item.add (selectInfo.selectPath ()) ;
			MPoint xformedPt ;
			if ( selectInfo.singleSelection () ) {
				MPoint center =curveNode->boundingBox ().center () ;
				xformedPt =center ;
				xformedPt *=selectInfo.selectPath ().inclusiveMatrix () ;
			}
			selectInfo.addSelection (item, xformedPt, selectionList, worldSpaceSelectPts, priorityMask, false) ;
		}
	}
	return (selected) ;
}

// Check to see whether points have been selected.
bool compCurveDeformableUI::selectPoints (MSelectInfo &selectInfo, MSelectionList &selectionList, MPointArray &worldSpaceSelectPts) const {
	bool selected =false ;
	M3dView view =selectInfo.view () ;

	MPoint xformedPoint ;
	MPoint currentPoint ;
	MPoint selectionPoint ;
	double z,previousZ =0.0 ;
	int closestPointVertexIndex =-1 ;
	const MDagPath &path =selectInfo.multiPath () ;

	// Create a component that will store the selected vertices
	MFnSingleIndexedComponent fnComponent ;
	MObject surfaceComponent =fnComponent.create (MFn::kCurveCVComponent) ;
	int vertexIndex ;

	// if the user did a single mouse click and we find > 1 selection
	// we will use the alignmentMatrix to find out which is the closest
	MMatrix	alignmentMatrix ;
	MPoint singlePoint ; 
	bool singleSelection =selectInfo.singleSelection () ;
	if ( singleSelection )
		alignmentMatrix =selectInfo.getAlignmentMatrix () ;

	// Get the geometry information
	compCurveDeformable *curveNode =(compCurveDeformable *)surfaceShape () ;
	MPointArray points ;
	curveNode->points (points) ;
	// Loop through all vertices of the mesh and
	// see if they lie withing the selection area
	int numVertices =points.length () ;
	for ( vertexIndex=0 ; vertexIndex<numVertices ; vertexIndex++ ) {
		const MPoint &currentPoint =points [vertexIndex] ;
		// Sets OpenGL's render mode to select and stores
		// selected items in a pick buffer
		view.beginSelect () ;
		glBegin (GL_POINTS) ;
		glVertex3f ((float)currentPoint [0], (float)currentPoint [1], (float)currentPoint [2]) ;
		glEnd () ;
		if ( view.endSelect () > 0 ) { // Hit count > 0
			selected =true ;
			if ( singleSelection ) {
				xformedPoint =currentPoint ;
				xformedPoint.homogenize () ;
				xformedPoint*=alignmentMatrix ;
				z =xformedPoint.z ;
				if ( closestPointVertexIndex < 0 || z > previousZ ) {
					closestPointVertexIndex =vertexIndex ;
					singlePoint =currentPoint ;
					previousZ =z ;
				}
			} else {
				// multiple selection, store all elements
				fnComponent.addElement (vertexIndex) ;
			}
		}
	}

	// If single selection, insert the closest point into the array
	//
	if ( selected && selectInfo.singleSelection () ) {
		fnComponent.addElement (closestPointVertexIndex) ;
		// need to get world space position for this vertex
		selectionPoint =singlePoint ;
		selectionPoint *=path.inclusiveMatrix () ;
	}

	// Add the selected component to the selection list
	if ( selected ) {
		MSelectionList selectionItem ;
		selectionItem.add (path, surfaceComponent) ;
		MSelectionMask mask (MSelectionMask::kSelectComponentsMask) ;
		selectInfo.addSelection (selectionItem, selectionPoint, selectionList, worldSpaceSelectPts, mask, true) ;
	}

	return (selected) ;
}

void compCurveDeformableUI::drawPoints (const MDrawRequest &request, const MPointArray &points) const {
	glPointSize (POINT_SIZE) ;
	glBegin (GL_POINTS) ;
	MObject comp =request.component () ;
	if ( !comp.isNull () ) {
		MFnSingleIndexedComponent fnComponent (comp) ;
		unsigned count =fnComponent.elementCount () ; 
		for ( unsigned ii=0 ; ii < count ; ii++ ) {
			int index =fnComponent.element (ii) ;
			const MPoint& vertex =points [index] ;
			glVertex3f ((float)vertex [0], (float)vertex [1], (float)vertex [2]) ;
		}
	} else {
		unsigned count =points.length () ;
		for ( unsigned ii =0 ; ii < count ; ++ii ) {
			const MPoint& point =points [ii] ;
			glVertex3f ((float)point.x, (float)point.y, (float)point.z) ;
		}
	}
	glEnd () ;
}
