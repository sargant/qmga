/******************************************************************************
 ** This file is part of QMGA a tool to display convex bodies.
 ** Copyright (C) 2005 Adrian Gabriel 
 ** Phillips-University of Marburg (Germany)
 ** qmga@users.sourceforge.net
 **
 ** QMGA is free software; you can redistribute it and/or modify
 ** it under the terms of the GNU General Public License as published by
 ** the Free Software Foundation; either version 2 of the License, or
 ** (at your option) any later version.
 **
 ** QMGA is distributed in the hope that it will be useful,
 ** but WITHOUT ANY WARRANTY; without even the implied warranty of
 ** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 ** GNU General Public License for more details.
 **
 ** You should have received a copy of the GNU General Public License
 ** along with QMGA; if not, write to the Free Software
 ** Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 ******************************************************************************/

#define GL_GLEXT_PROTOTYPES 1
#include <GL/gl.h>
#include <GL/glut.h>
#include <GL/glext.h>

#include "renderer.h"
#include "psEncode.h"

#include <qfile.h>
#include <qimage.h>
#include <qdatetime.h>
#include <qdragobject.h>

#include <cmath>
#include <iostream>

using std::cerr;
using std::cout;
using std::endl;


#if defined(Q_CC_MSVC)
#pragma warning(disable:4305) // init: truncation from const double to float
#endif

/*!
  Create a Renderer widget
*/

/*!
 *	Main constructor.
 *  \param  updAngle function-pointer used to update the angles in the gui
 *	\param 	updZ function-pointer used to update the depth information in the gui (zoom)
 *	\param 	par the parent MainForm, where this Widget is used in
 *	\param	parent the parent widget
 *	\param	name
 *  \return void.
 *  \author Timm Meyer
 */
Renderer::Renderer( QWidget* parent, const char* name )
    : QGLWidget( parent, name ) {
    //cout << "Renderer::Renderer() beg" << endl;
    
    renderSet_RenderAsLines               = true;
    renderSet_occlusionExtensionSupported = false;
    renderSet_UseOcclusionTest            = true;
    renderSet_UseAutoLOD                  = true;
    renderSet_BoxCount                    = 20;
    renderSet_occlusionLayerStartFactor   = 10;
    renderSet_minimumPixelsToDraw         = 1;
    renderSet_RenderSideStartingPercent   = 40;
    renderSet_RenderSide                  = RENDERSIDE_NONE;
    
    updateGui = false;
    
    distAbsolut = false;
    
    nearClip = 0.1f;
    farClip = 180.0f;
    
    drawColorMap = true;
    drawAxis = true;
    drawBoundingBox = true;
    
    maxBoundingDiff = 0.0f;
    
    currentState = false;
    
    recreateModel = true;
    modelXComplexity_max = 12;
    modelYComplexity_max = 4;
    modelXComplexity_min = 4;
    modelYComplexity_min = 0;
    modelLevels = 6;
    
    LODdelta = 0;
    drawBoxes = false;
    
    startBox_X = 0;
    startBox_DeltaX = 1;
    startBox_Y = 0;
    startBox_DeltaY = 1;
    startBox_Z = 0;
    startBox_DeltaZ = 1;
    drawOptimized = true;
    
    viewWidth = 800;
    viewHeigth = 600;
    
    distance = 1.0f;
    distanceUpdate = 1.0f;
    XMousedelta = 0.0f;
    YMousedelta = 0.0f;
    
    XRealRot = 0.0f;
    YRealRot = 0.0f;
    ZRealRot = 0.0f;
    
    lastUpdate = 0;
    
    colorMapLeftDelta = 0;
    
    pressedButton = 0;
    //dragging = false;
    dragLeft  = FALSE;
    dragRight = FALSE;
    
    colorMap = NULL;
    
    lineSizes[0] = 1;
    middle = 0;
    rot    = 0;
    color  = 0;
    
    boundingBoxXDraw = 0;
    boundingBoxYDraw = 0;
    boundingBoxZDraw = 0;
    
    // default axis colors
    axisColors[0][0] = 1;
    axisColors[0][1] = 0;
    axisColors[0][2] = 0;
    axisColors[1][0] = 0;
    axisColors[1][1] = 1;
    axisColors[1][2] = 0;
    axisColors[2][0] = 0;
    axisColors[2][1] = 0;
    axisColors[2][2] = 1;
    
    
    drawAsSlice = false;
    sliceXLow = 0;
    sliceXHigh = 2;
    sliceYLow = 0;
    sliceYHigh = 2;
    sliceZLow = 0;
    sliceZHigh = 2;
    
    xAdditionalRotation = 0;
    yAdditionalRotation = 0;
    zAdditionalRotation = 0;
    
    
    offScreen = false;
    resetAngles = false;
    
    shininess = 0;
    
    globalMaxModelSize = 0;    
    
    boundingBoxColor.resize(3,1.0);
    lineSizeRelative = 0.0f;
    setFocusPolicy(QWidget::StrongFocus);
    setAcceptDrops( true );
    initialized = false;
    
    //cout << "Renderer::Renderer() end" << endl;
}

/*!
  Release allocated resources
*/
Renderer::~Renderer() {
    makeCurrent();
}

/*!
  Paint the box. The actual openGL commands for drawing the box are
  performed here.
*/
void Renderer::paintGL() {
    //cout << "Renderer::paintGL() beg" << endl;
    
    static QTime startTime;
    startTime.start();
    
    // Clear screen
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    if(!offScreen) {
	// process user input
	bool forceProcess = false;
	
	if(xAdditionalRotation != 0) {
	    glRotatef(xAdditionalRotation,1,0,0);
	    xAdditionalRotation = 0;
	    forceProcess = true;
	    updateGui = true;
	}
	if(yAdditionalRotation != 0) {
	    glRotatef(yAdditionalRotation,0,1,0);
	    yAdditionalRotation = 0;
	    forceProcess = true;
	    updateGui = true;
	}
	if(zAdditionalRotation != 0) {
	    glRotatef(zAdditionalRotation,0,0,1);
	    zAdditionalRotation = 0;
	    updateGui = true;
	    forceProcess = true;
	}
	processInput(forceProcess);
    }
    
    calculateRotationMatrix();
    
    glLoadIdentity();
    
    translateAndRotateCamera();
    
    // always force gui update because we only draw if a change happens, so we need an update!
    updateGuiF(true);
    updateGuiF();
    
    // update line width, because we change it when we draw the axis
    glLineWidth(lineSizes[0] + (lineSizes[1]-lineSizes[0])*lineSizeRelative );
    
    // main render function  
    displayModels();
    
    if(drawBoundingBox) {
	displayBoundingBox();
    }
    
    glLoadIdentity();
    
    renderColormap();
    
    glFlush();
    
    processFPS(startTime.elapsed());
    
    int err = glGetError();
    if (err != GL_NO_ERROR) {
	cerr << "OpenGL Error: " << err << endl;
    }
    
    // signal that we are done with rendering
    rendering = false;
    //cout << "Renderer::paintGL() end" << endl;
}

void Renderer::setLineSize(float f) {
    //cout << "Renderer::setLineSize() beg" << endl;
    lineSizeRelative = f;
    repaint();
    //cout << "Renderer::setLineSize() end" << endl;
}

/*!
 *	Function used to specify the lighting parameters
 *
 *  \param amb vector of values for the ambient light
 *  \param diff vector of values for the diffuse light
 *  \param spec vector of values for the specular light
 *  \param pos vector of values for the light position
 *  \param shini value for the shininess
 *  \param bgColor background Color
 *  \param bbColor bounding-box Color (also used for the axis and colormap description)
 *  \author Timm Meyer
 */
void Renderer::setLight(vector<float> amb, vector<float> diff, vector<float> spec, vector<float> pos, float shini, vector<float> bgColor, vector<float> bbColor) {
    //cout << "Renderer::setLight() beg" << endl;
    glLightfv(GL_LIGHT1, GL_AMBIENT, (float*)&amb[0]);
    glLightfv(GL_LIGHT1, GL_DIFFUSE, (float*)&diff[0]);
    glLightfv(GL_LIGHT2, GL_AMBIENT, (float*)&amb[0]);
    glLightfv(GL_LIGHT2, GL_DIFFUSE, (float*)&diff[0]);
    glLightfv(GL_LIGHT1, GL_SPECULAR, (float*)&spec[0]);
    glLoadIdentity();
    glLightfv(GL_LIGHT1, GL_POSITION, (float*)&pos[0]);
    //glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, shini);
    glMaterialf(GL_FRONT, GL_SHININESS, shini);
    shininess = shini;
    glClearColor(bgColor.at(0), bgColor.at(1), bgColor.at(2), 0.5f);
    boundingBoxColor = bbColor;
    repaint();
    //cout << "Renderer::setLight() end" << endl;
}

/*!
  Set up the OpenGL rendering state, and define display list
*/
void Renderer::initializeGL() {
    //cout << "Renderer::initializeGL() beg" << endl;
    initialized = true;
    
    string ext = (char*)(glGetString(GL_EXTENSIONS));
    renderSet_occlusionExtensionSupported = (ext.find("GL_ARB_occlusion_query") != string::npos);
    
    glShadeModel(GL_SMOOTH);
    
    glLightfv(GL_LIGHT1, GL_AMBIENT, (float[]) {
	0.0f, 0.0f, 0.0f, 0.0f
	    }
    );
    glLightfv(GL_LIGHT1, GL_DIFFUSE, (float[]) {
	0.85f, 0.85f, 0.85f, 1.0f
	    }
    );
    glLightfv(GL_LIGHT1, GL_SPECULAR, (float[]) {
	1.0f, 1.0f, 1.0f, 1.0f
	    }
    );
    glLightfv(GL_LIGHT1, GL_POSITION, (float[]) {
	0.0f, 0.0f, 1.0f, 0.0f
	    }
    );
    glEnable(GL_LIGHT1);
    glLightfv(GL_LIGHT2, GL_AMBIENT, (float[]) {
	0.0f, 0.0f, 0.0f, 0.0f
	    }
    );
    glLightfv(GL_LIGHT2, GL_DIFFUSE, (float[]) {
	0.85f, 0.85f, 0.85f, 1.0f
	    }
    );
    glLightfv(GL_LIGHT2, GL_SPECULAR, (float[]) {
	0.0f, 0.0f, 0.0f, 0.0f
	    }
    );
    glLightfv(GL_LIGHT2, GL_POSITION, (float[]) {
	0.0f, 0.0f, 1.0f, 0.0f
	    }
    );
    glDisable(GL_LIGHT0);
    glDisable(GL_LIGHT2);
    glDisable(GL_LIGHT3);
    glDisable(GL_LIGHT4);
    glDisable(GL_LIGHT5);
    glDisable(GL_LIGHT6);
    glDisable(GL_LIGHT7);
    glEnable(GL_LIGHTING);
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, (float[]) { 0.1f,0.1f,0.1f,1.0f } );
    
    /*
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, (float[]) { 1.0f,1.0f,1.0f,1.0f } );
    glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, (float[]) { 20.0f } );
    glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
    */
    glMaterialfv(GL_FRONT, GL_SPECULAR, (float[]) { 1.0f,1.0f,1.0f,1.0f } );
    glMaterialfv(GL_FRONT, GL_SHININESS, (float[]) { 20.0f } );
    glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);
   
    glEnable(GL_COLOR_MATERIAL);
    
    glClearColor(1.0f, 1.0f, 1.0f, 0.5f);
    glClearDepth(1.0f);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    
    // Backface Culling
    glEnable(GL_CULL_FACE);
    glFrontFace(GL_CCW);
    
    // Load axis and colormap description
    glGenTextures(1, textures);
    
    QImage tex1, buf1;
    
    // FIXME : Windows issue (maybe) because of differences in data structure
    buf1.load( "images/text.png" );
    tex1 = QGLWidget::convertToGLFormat( buf1 ); // flipped 32bit RGBA
    
    glBindTexture(GL_TEXTURE_2D, textures[0]);
    glTexImage2D( GL_TEXTURE_2D, 0, 4, tex1.width(), tex1.height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, tex1.bits() );
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
    
    render_LastFrameStartTime = time(NULL);
    lastUpdate = time(NULL);
    //cout << "Renderer::initializeGL() end" << endl;
}

/*!
  Set up the OpenGL view port, matrix mode, etc.
*/
void Renderer::resizeGL( int width, int height ) {
    //cout << "Renderer::resizeGL beg" << endl;
    initialized = true;
    
    if (height <= 0) // avoid divide by zero
	height = 1;
    sideRelation = (float) width / (float) height;
    glViewport(0, 0, width, height);
    distAbsolut = true;
    emit sizeChanged();
    //cout << "Renderer::resizeGL end" << endl;    
}

/*!
 *	Function used to process user inpuit
 *
 *  \param  force	if set to true forces processing, i.e. revalidating scene position etc.
 *  \author Timm Meyer
 */
void Renderer::processInput(bool force) {
    //cout << "Renderer::processInput beg" << endl;
    if (offScreen) {
	return;
    }
    
    // if screen has been rotated or distance has been changed => update 
    // the engine settings
    if (XMousedelta != 0 || YMousedelta != 0 || distanceUpdate != 0 || distAbsolut || force) {
	float vector[3];
	GLfloat matrix[16];
	// get openGL's current modelview matrix
	glGetFloatv(GL_MODELVIEW_MATRIX, matrix);
	
	// change camera depending on user values
	if (distAbsolut && distanceUpdate == 0) {
	    distAbsolut = false;
	    glMatrixMode(GL_PROJECTION);
	    glLoadIdentity();
	    glOrtho(-sideRelation * (distance / 2), sideRelation * (distance / 2), -(distance / 2), (distance / 2), nearClip, farClip+abs(distance));
	    glMatrixMode(GL_MODELVIEW);
	    
	    resetAngles = true;
	} else if (distanceUpdate != 0) {
	    
	    distance += distanceUpdate;
	    distanceUpdate = 0.0f;
	    if (distance < 1) {
		distance = 1;
	    }
	    
	    emit zoomChanged((int)distance);
	    
	    glMatrixMode(GL_PROJECTION);
	    glLoadIdentity();
	    glOrtho(-sideRelation * (distance / 2), sideRelation * (distance / 2), -(distance / 2), (distance / 2), nearClip, farClip+abs(distance));
	    glMatrixMode(GL_MODELVIEW);
	    
	    resetAngles = true;
	}
	
	if(!resetAngles) {
	    
	    // get the current x-axis
	    vector[0] = matrix[0];
	    vector[1] = matrix[4];
	    vector[2] = matrix[8];
	    glRotatef(XMousedelta, vector[0], vector[1], vector[2]);
	    
	    // get the current y-axis
	    vector[0] = matrix[1];
	    vector[1] = matrix[5];
	    vector[2] = matrix[9];
	    glRotatef(YMousedelta, vector[0], vector[1], vector[2]);
	    
	    XMousedelta = 0.0f;
	    YMousedelta = 0.0f;
	    
	    // calculate new rotation
	    // get openGL's current modelview matrix
	    glGetFloatv(GL_MODELVIEW_MATRIX, matrix);
	    
	    double thetaX = asin(-matrix[6]);
	    double thetaY, thetaZ;
	    if (thetaX < M_PI / 2) {
		if (thetaX > -M_PI / 2) {
		    thetaY = atan2(matrix[2], matrix[10]);
		    thetaZ = atan2(matrix[4], matrix[5]);
		} else {
		    thetaY = -atan2(-matrix[1], matrix[0]);
		    thetaZ = 0;
		}
	    } else {
		thetaY = atan2(-matrix[1], matrix[0]);
		thetaZ = 0;
	    }
	    
	    XRealRot = (float) (thetaX / M_PI * 180.0);
	    if (XRealRot < 0) {
		XRealRot += 360;
	    }
	    YRealRot = (float) (thetaY / M_PI * 180.0);
	    if (YRealRot < 0) {
		YRealRot += 360;
	    }
	    ZRealRot = (float) (thetaZ / M_PI * 180.0);
	    if (ZRealRot < 0) {
		ZRealRot += 360;
	    }
	} else {
	    resetAngles = false;
	}
	
	
    } // Update done
    
    //cout << "Renderer::processInput end" << endl;
}

/*!
 *	Calculates the rotation Matrix out of the rotation values.
 */
void Renderer::calculateRotationMatrix() {
    //cout << "Renderer::calculateRotationMatrix beg" << endl;
    
    //cout << "x: " << XRealRot << endl;
    //cout << "y: " << YRealRot << endl;
    //cout << "z: " << ZRealRot << endl;
    
    double cx = cos(M_PI * XRealRot / 180.0);
    double sx = sin(M_PI * XRealRot / 180.0);
    double cy = cos(M_PI * YRealRot / 180.0);
    double sy = sin(M_PI * YRealRot / 180.0);
    double cz = cos(M_PI * ZRealRot / 180.0);
    double sz = sin(M_PI * ZRealRot / 180.0);
    
    render_RotationMatrix[0] = (float) (cy * cz + sx * sy * sz);
    render_RotationMatrix[1] = (float) (cz * sx * sy - cy * sz);
    render_RotationMatrix[2] = (float) (cx * sy);
    render_RotationMatrix[4] = (float) (cx * sz);
    render_RotationMatrix[5] = (float) (cx * cz);
    render_RotationMatrix[6] = (float) (-sx);
    render_RotationMatrix[8] = (float) (-cz * sy + cy * sx * sz);
    render_RotationMatrix[9] = (float) (cy * cz * sx + sy * sz);
    render_RotationMatrix[10] = (float) (cx * cy);
    
    //     render_RotationMatrix[0]  = float( cy*cz );
    //     render_RotationMatrix[1]  = float( cy*sz );
    //     render_RotationMatrix[2]  = float(-sy );
    //     render_RotationMatrix[4]  = float( cz*sx*sy-cx*sz );
    //     render_RotationMatrix[5]  = float( cx*cz+sx*sy*sz );
    //     render_RotationMatrix[6]  = float( cy*sx );
    //     render_RotationMatrix[8]  = float( cx*cz*sy+sx*sz );
    //     render_RotationMatrix[9]  = float(-cz*sx+cx*sy*sz );
    //     render_RotationMatrix[10] = float( cx*cy );
    
    //     render_RotationMatrix[0]  = float( 1 );
    //     render_RotationMatrix[1]  = float( 0 );
    //     render_RotationMatrix[2]  = float( 0 );
    //     render_RotationMatrix[4]  = float( 0 );
    //     render_RotationMatrix[5]  = float( cx );
    //     render_RotationMatrix[6]  = float( sx );
    //     render_RotationMatrix[8]  = float( 0 );
    //     render_RotationMatrix[9]  = float(-sx );
    //     render_RotationMatrix[10] = float( cx );
    
    //     render_RotationMatrix[0]  = float( sy );
    //     render_RotationMatrix[1]  = float( 0 );
    //     render_RotationMatrix[2]  = float( sy );
    //     render_RotationMatrix[4]  = float( 0 );
    //     render_RotationMatrix[5]  = float( 1 );
    //     render_RotationMatrix[6]  = float( 0 );
    //     render_RotationMatrix[8]  = float(-sy );
    //     render_RotationMatrix[9]  = float( 0 );
    //     render_RotationMatrix[10] = float( cy );
    
    //     render_RotationMatrix[0]  = float( cz );
    //     render_RotationMatrix[1]  = float( sz );
    //     render_RotationMatrix[2]  = float( 0 );
    //     render_RotationMatrix[4]  = float(-sz );
    //     render_RotationMatrix[5]  = float( cz );
    //     render_RotationMatrix[6]  = float( 0 );
    //     render_RotationMatrix[8]  = float( 0 );
    //     render_RotationMatrix[9]  = float( 0 );
    //     render_RotationMatrix[10] = float( 1 );
    
    render_RotationMatrix[3] = render_RotationMatrix[7] = render_RotationMatrix[11] = render_RotationMatrix[12] = render_RotationMatrix[13] = render_RotationMatrix[14] = 0;
    render_RotationMatrix[15] = 1;
    
    //cout << "Renderer::calculateRotationMatrix end" << endl;
}

/*!
 *	Resets the rotation of the current modelview matrix
 */
void Renderer::resetRotation() {
    //cout << "Renderer::resetRotation() beg" << endl;
    float modelview[16];
    int i,j;
    
    // save the current modelview matrix
    glPushMatrix();
    
    // get the current modelview matrix
    glGetFloatv(GL_MODELVIEW_MATRIX , modelview);
    
    // undo all rotations
    for( i=0; i<3; i++ )
	for( j=0; j<3; j++ ) {
	if ( i==j )
	    modelview[i*4+j] = 1.0;
	else
	    modelview[i*4+j] = 0.0;
    }
    
    // set the modelview with no rotations and scaling
    glLoadMatrixf(modelview);
    //cout << "Renderer::resetRotation() end" << endl;
}

/*!
 *	Render the colormap and axis
 *
 *  \author Timm Meyer
 */
void Renderer::renderColormap() {
    //cout << "Renderer::renderColormap() beg" << endl;
    glDisable(GL_DEPTH_TEST);
    
    void (Renderer::*rasterPos3f)(float,float,float) = &Renderer::tglRasterPos3f;
    if(offScreen) {
	rasterPos3f = &Renderer::ttrRasterPos3f;
    }
    
    if (colorMap != NULL && colorMap->size() > 0 && drawColorMap) {
	float topPosX = -sideRelation * (distance/2) + (0.1) * (distance/2);
	float topPosY = 0.9 * (distance/2);
	float inc = (0.9*distance) / (colorMap->size() - 1);
	float y_inc = distance/2.0*0.1;
	float zpos = -0.11;
	
	float textMargin = 0;//(y_inc/2);
	
	// Reset lighting for colormap
	glDisable(GL_LIGHT1);
	glEnable(GL_LIGHT2);
	
	glBegin(GL_TRIANGLES);
	glNormal3f(0,0,1);
	
	// Render the actual colormap
	for (int i = 0; i < (int)colorMap->size() - 1; i++) {
	    glColor3fv(colorMap->at(i));
	    glVertex3f(topPosX + y_inc, topPosY, zpos);
	    glVertex3f(topPosX, topPosY, zpos);
	    glColor3fv(colorMap->at(i + 1));
	    glVertex3f(topPosX + y_inc, topPosY - inc, zpos);
	    
	    glVertex3f(topPosX, topPosY - inc, zpos);
	    glVertex3f(topPosX + y_inc, topPosY - inc, zpos);
	    glColor3fv(colorMap->at(i));
	    glVertex3f(topPosX, topPosY, zpos);
	    topPosY -= inc;
	}
	glEnd();
	
	// Reset Lighting to normal
	glDisable(GL_LIGHT2);
	glEnable(GL_LIGHT1);
	
	glDisable(GL_LIGHTING);
	glEnable( GL_BLEND );
	glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glColor4f(boundingBoxColor.at(0),boundingBoxColor.at(1),boundingBoxColor.at(2),1);
	glEnable(GL_TEXTURE_2D);
	
	// render the colormap descriptions
	
	glBegin(GL_QUADS);
	glTexCoord2f(0.0, 1.0-15.0/79.0);
	glVertex3f(topPosX + textMargin, topPosY - 3*inc, zpos);
	glTexCoord2f(40.0/43.0, 1.0-15.0/79.0);
	glVertex3f(topPosX + textMargin + y_inc, topPosY - 3*inc, zpos);
	glTexCoord2f(40.0/43.0, 1.0-0.0/79.0);
	glVertex3f(topPosX + textMargin + y_inc, topPosY - 3*inc + inc*1.8, zpos);
	glTexCoord2f(0.0, 1.0-0.0/79.0);
	glVertex3f(topPosX + textMargin, topPosY - 3*inc + inc*1.8, zpos);
	glEnd();
	
	// first lower than upper
	
	topPosY = 0.9 * (distance/2);
	
	glBegin(GL_QUADS);
	glTexCoord2f(0.0, 1.0-31.0/79.0);
	glVertex3f(topPosX + textMargin, topPosY + inc, zpos);
	glTexCoord2f(33.0/43.0, 1.0-31.0/79.0);
	glVertex3f(topPosX + textMargin + y_inc, topPosY + inc, zpos);
	glTexCoord2f(33.0/43.0, 1.0-16.0/79.0);
	glVertex3f(topPosX + textMargin + y_inc, topPosY + inc + inc*1.8, zpos);
	glTexCoord2f(0.0, 1.0-16.0/79.0);
	glVertex3f(topPosX + textMargin, topPosY + inc + inc*1.8, zpos);
	glEnd();
	
	glDisable(GL_TEXTURE_2D);
	glColor3f(0,0,0);
	glDisable( GL_BLEND );
    }
    
    glDisable(GL_LIGHTING);
    glEnable(GL_DEPTH_TEST);
    
    float halfDist = distance/2.0;
    float tempDist = halfDist/10.0;
    
    // Axis
    glTranslatef(sideRelation * (distance/2) - (0.2) * (distance/2),-0.8f * halfDist,-(nearClip + abs(halfDist/4.9)));
    //cout << nearClip << " " << farClip << " " << distance << " " << halfDist << " " << (nearClip + abs(halfDist/4))*0.85f << " " << halfDist/20.0 << endl;
    
    //cout << "[Renderer::renderColormap] here"<<endl;
    
    glMultMatrixf(render_RotationMatrix);
    
    if(!drawAxis) {
	return;
    }
    
    // reset to minimun line size  
    glLineWidth(lineSizes[0]);
    
    // axis lines
    glBegin(GL_LINES);
    glColor3fv(axisColors[0]);
    glVertex3f(0,0,0);
    glVertex3f(halfDist/10,0,0);
    glColor3fv(axisColors[1]);
    glVertex3f(0,0,0);
    glVertex3f(0,halfDist/10,0);
    glColor3fv(axisColors[2]);
    glVertex3f(0,0,0);
    glVertex3f(0,0,halfDist/10);
    glEnd();
    
    // axis cones
    
    const int complex = 8;
    const float height = halfDist/50.0;
    const float len = halfDist/20.0;
    float angle = (float) (2.0*M_PI/complex);
    
    glColor3f(0,0,0);
    
    glBegin(GL_TRIANGLE_FAN);
    glColor3fv(axisColors[0]);
    glVertex3f(tempDist  + len,0,0);
    for(int i = 0; i <= complex; i++) {
	if(i == 0 || i == complex) {
	    glVertex3f(tempDist ,height,0);
	} else {
	    glVertex3f(tempDist ,(float) cos(angle * i)*height,(float) sin(angle * i)*height);
	}
    }
    glEnd();
    
    glBegin(GL_TRIANGLE_FAN);
    glVertex3f(tempDist ,0,0);
    for(int i = 0; i <= complex; i++) {
	if(i == 0 || i == complex) {
	    glVertex3f(tempDist ,height,0);
	} else {
	    glVertex3f(tempDist ,(float) cos(angle * i)*height,(float) -sin(angle * i)*height);
	}
    }
    glEnd();
    
    glBegin(GL_TRIANGLE_FAN);
    glColor3fv(axisColors[1]);
    glVertex3f(0,tempDist + len,0);
    for(int i = 0; i <= complex; i++) {
	if(i == 0 || i == complex) {
	    glVertex3f(height,tempDist ,0);
	} else {
	    glVertex3f((float) cos(angle * i)*height,tempDist ,(float) sin(angle * i)*height);
	}
    }
    glEnd();
    
    glBegin(GL_TRIANGLE_FAN);
    glVertex3f(0,tempDist ,0);
    for(int i = 0; i <= complex; i++) {
	if(i == 0 || i == complex) {
	    glVertex3f(height,tempDist ,0);
	} else {
	    glVertex3f((float) cos(angle * i)*height,tempDist ,(float) -sin(angle * i)*height);
	}
    }
    glEnd();
    
    glBegin(GL_TRIANGLE_FAN);
    glColor3fv(axisColors[2]);
    glVertex3f(0,0,tempDist  + len);
    for(int i = 0; i <= complex; i++) {
	if(i == 0 || i == complex) {
	    glVertex3f(height,0,tempDist );
	} else {
	    glVertex3f((float) cos(angle * i)*height,(float) sin(angle * i)*height,tempDist );
	}
    }
    glEnd();
    
    glBegin(GL_TRIANGLE_FAN);
    glVertex3f(0,0,tempDist );
    for(int i = 0; i <= complex; i++) {
	if(i == 0 || i == complex) {
	    glVertex3f(height,0,tempDist );
	} else {
	    glVertex3f((float) cos(angle * i)*height,(float) -sin(angle * i)*height,tempDist );
	}
    }
    glEnd();
    
    glPushMatrix();
    
    // axis descriptions
    
    glEnable( GL_BLEND );
    glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
    glColor4f(boundingBoxColor.at(0),boundingBoxColor.at(1),boundingBoxColor.at(2),1);
    glEnable(GL_TEXTURE_2D);
    
    //cout << (tempDist  + len)*1.18f << endl;
    
    glTranslatef((tempDist  + len)*1.18f,0,0);
    resetRotation();
    
    glBegin(GL_QUADS);
    glTexCoord2f(0.0, 1.0-45.0/79.0);
    glVertex3f(-height,-height,0);
    glTexCoord2f(9.0/43.0, 1.0-45.0/79.0);
    glVertex3f(height,-height,0);
    glTexCoord2f(9.0/43.0, 1.0-37.0/79.0);
    glVertex3f(height,height,0);
    glTexCoord2f(0.0, 1.0-37.0/79.0);
    glVertex3f(-height,height,0);
    glEnd();
    
    // restores the modelview matrix
    glPopMatrix();
    
    glTranslatef(-(tempDist  + len)*1.18f,(tempDist  + len)*1.18f,0);
    resetRotation();
    
    glBegin(GL_QUADS);
    glTexCoord2f(0.0, 1.0-65.0/79.0);
    glVertex3f(-height,-height,0);
    glTexCoord2f(9.0/43.0, 1.0-65.0/79.0);
    glVertex3f(height,-height,0);
    glTexCoord2f(9.0/43.0, 1.0-53.0/79.0);
    glVertex3f(height,height,0);
    glTexCoord2f(0.0, 1.0-53.0/79.0);
    glVertex3f(-height,height,0);
    glEnd();
    
    // restores the modelview matrix
    glPopMatrix();
    
    glTranslatef(0,-(tempDist  + len)*1.18f,(tempDist  + len)*1.18f);
    resetRotation();
    
    glBegin(GL_QUADS);
    glTexCoord2f(0.0, 1.0-79.0/79.0);
    glVertex3f(-height,-height,0);
    glTexCoord2f(9.0/43.0, 1.0-79.0/79.0);
    glVertex3f(height,-height,0);
    glTexCoord2f(9.0/43.0, 1.0-70.0/79.0);
    glVertex3f(height,height,0);
    glTexCoord2f(0.0, 1.0-70.0/79.0);
    glVertex3f(-height,height,0);
    glEnd();
    
    glDisable(GL_TEXTURE_2D);
    glColor3f(0,0,0);
    glDisable( GL_BLEND );
    
    glPopMatrix();
    
    // restores the modelview matrix
    glPopMatrix();
    
    glEnable(GL_LIGHTING);
    //cout << "Renderer::renderColormap() end" << endl;
}

/*!
 *
 *  \author Timm Meyer
 */
void Renderer::translateAndRotateCamera() {
    //cout << "Renderer::translateAndRotateCamera() beg" << endl;
    glTranslatef(0.0f, 0.0f, -maxBoundingDiff);
    
    //     gluLookAt(0.0, -100.0, 0.0,
    // 	      0.0, 0.0, 0.0,
    // 	      0.0, 0.0, 1.0); // eye, centre, up
    
    glMultMatrixf(render_RotationMatrix);
    
    if (distance < 1) {
	distance = 1;
    }
    //cout << "Renderer::translateAndRotateCamera() end" << endl;
}

/*!
 *	Renders the models on screen
 *
 *  \author Timm Meyer
 */
void Renderer::displayModels() 	
{
    //cout << "Renderer::displayModels() beg" << endl;
    if (recreateModel) {
	for( uint i=0; i < objectParams.size(); i++ )
	{
	    // create the needed models
	    switch( int(objectParams.at(i).at(0)) )
	    {
	    case 0:
		createModels1(i);
		break;
	    case 1:
		createModels2(i);
		break;
	    case 2:
		createModels3(i);
		break;
	    case 3:
		createModels4(i);
		break;
	    case 4:
		createModels5(i);
		break;
	    default:
		cout << "Warning: unknown model index: " <<  objectParams.at(i).at(0) << " ... using ellipsoid" << endl;
		createModels1(i);
		break;
	    }
	    /*
	    if(objectParams.at(i).at(4) <= 0 )
	    {
		createModels1(i);
	    } 
	    else
	    {
		createModels2(i);
	    }
	    createModels4(i);
	    */
	}
	recreateModel = false;
	glGetFloatv(GL_LINE_WIDTH_RANGE, lineSizes);
	// Need to recreate subboxes
	createGridBoxes(renderSet_BoxCount);
	currentState = false;
	updateGui = true;
	updateGuiF();
    }
    
    int tempLOD = renderSet_UseAutoLOD?LODdelta:0;
    vector<int>* callIndex = &modelListIndex;
    
    if (renderSet_RenderAsLines) {
	callIndex = &modelListSimpleIndex;
	tempLOD = 0;
	glDisable(GL_LIGHTING);
    } else {
	glEnable(GL_LIGHTING);
    }
    
    // Anything to draw?
    if (middle->size() > 0 && rot->size() > 0 && color->size() > 0) {
	
	if (drawBoxes)
	{
	    renderSubBoundingBoxes();
	} else {
	    
	    if (renderSet_RenderAsLines || !drawOptimized) {
		// Draw everything
		
		if(drawAsSlice) {
		    
		    for (int i = 0; i < (int)middle->size(); i++) {
			if(sliceXLow <= middle->at(i)->at(0) && sliceXHigh >= middle->at(i)->at(0)
			    && sliceYLow <= middle->at(i)->at(1) && sliceYHigh >= middle->at(i)->at(1)
			    && sliceZLow <= middle->at(i)->at(2) && sliceZHigh >= middle->at(i)->at(2)) {
			    glPushMatrix();
			    glTranslatef(middle->at(i)->at(0), middle->at(i)->at(1), middle->at(i)->at(2));
			    
			    if (rot->at(i)->at(0) != 0 || rot->at(i)->at(1) != 0 || rot->at(i)->at(2) != 0) {
				glRotatef(rot->at(i)->at(3), rot->at(i)->at(0), rot->at(i)->at(1), rot->at(i)->at(2));
			    }
			    
			    glColor3fv(((float*)&((*(color->at(i)))[0]))) ;
			    glCallList(callIndex->at(modelInd->at(i)) + tempLOD);
			    
			    glPopMatrix();
			}
		    }
		} else {
		    for (int i = 0; i < (int)middle->size(); i++) {
			glPushMatrix();
			glTranslatef(middle->at(i)->at(0), middle->at(i)->at(1), middle->at(i)->at(2));
			
			if (rot->at(i)->at(0) != 0 || rot->at(i)->at(1) != 0 || rot->at(i)->at(2) != 0) {
			    //cout << "[displayModels()] " << rot->at(i)->at(3) << " " << rot->at(i)->at(0) << " " <<  rot->at(i)->at(1) << " " <<  rot->at(i)->at(2)<< endl;
			    glRotatef(rot->at(i)->at(3), rot->at(i)->at(0), rot->at(i)->at(1), rot->at(i)->at(2));
			}
			
			glColor3fv(((float*)&((*(color->at(i)))[0])));
			
			//cout << "-------" << endl;
			//cout << "callIndex->size() " << callIndex->size() << endl;
			//cout << "modelInd->size() " << modelInd->size() << endl;
			//cout << "i " << i << endl;
			//cout << "-------" << endl;
			
			// 			try
			// 			  {
			glCallList(callIndex->at(modelInd->at(i)) + tempLOD);
			// 			  }
			// 			catch(...)
			// 			  {
			// 			    cout << "modelInd " << modelInd->at(i) << endl;
			// 			    cout << "callIndex " << callIndex->at(modelInd->at(i)) << endl;
			// 			    cerr << "caught\n";
			// 			  }
			
			// 			cout << "modelInd " << modelInd->at(i) << endl;
			
			glPopMatrix();
		    }
		}
	    } else {
		if (renderSet_RenderSide == RENDERSIDE_NONE) {
		    renderFromCorner();
		} else {
		    renderFromSide();
		}
		currentState = !currentState;
	    }
	}
    }
    //cout << "Renderer::displayModels() end" << endl;
}

///*!
// *	Renders the Bounding Box
// *  \author Timm Meyer
// */
//void Renderer::displayBoundingBox() {
//    
//    glDisable(GL_LIGHTING);
//    
//    glBegin(GL_LINES);
//    // BoundingBox corners
//    glColor3f(boundingBoxColor.at(0), boundingBoxColor.at(1) , boundingBoxColor.at(2));
//    
//    glVertex3f(boundingBoxXDraw, boundingBoxYDraw, boundingBoxZDraw);
//    glVertex3f(boundingBoxXDraw, boundingBoxYDraw, -boundingBoxZDraw);
//    
//    glVertex3f(-boundingBoxXDraw, boundingBoxYDraw, boundingBoxZDraw);
//    glVertex3f(-boundingBoxXDraw, boundingBoxYDraw, -boundingBoxZDraw);
//    
//    glVertex3f(-boundingBoxXDraw, -boundingBoxYDraw, boundingBoxZDraw);
//    glVertex3f(-boundingBoxXDraw, -boundingBoxYDraw, -boundingBoxZDraw);
//    
//    glVertex3f(boundingBoxXDraw, -boundingBoxYDraw, boundingBoxZDraw);
//    glVertex3f(boundingBoxXDraw, -boundingBoxYDraw, -boundingBoxZDraw);
//    
//    
//    glVertex3f(boundingBoxXDraw, boundingBoxYDraw, boundingBoxZDraw);
//    glVertex3f(boundingBoxXDraw, -boundingBoxYDraw, boundingBoxZDraw);
//    
//    glVertex3f(-boundingBoxXDraw, boundingBoxYDraw, boundingBoxZDraw);
//    glVertex3f(-boundingBoxXDraw, -boundingBoxYDraw, boundingBoxZDraw);
//    
//    glVertex3f(-boundingBoxXDraw, boundingBoxYDraw, -boundingBoxZDraw);
//    glVertex3f(-boundingBoxXDraw, -boundingBoxYDraw, -boundingBoxZDraw);
//    
//    glVertex3f(boundingBoxXDraw, boundingBoxYDraw, -boundingBoxZDraw);
//    glVertex3f(boundingBoxXDraw, -boundingBoxYDraw, -boundingBoxZDraw);
//    
//    
//    glVertex3f(boundingBoxXDraw, boundingBoxYDraw, boundingBoxZDraw);
//    glVertex3f(-boundingBoxXDraw, boundingBoxYDraw, boundingBoxZDraw);
//    
//    glVertex3f(boundingBoxXDraw, -boundingBoxYDraw, boundingBoxZDraw);
//    glVertex3f(-boundingBoxXDraw, -boundingBoxYDraw, boundingBoxZDraw);
//    
//    glVertex3f(boundingBoxXDraw, -boundingBoxYDraw, -boundingBoxZDraw);
//    glVertex3f(-boundingBoxXDraw, -boundingBoxYDraw, -boundingBoxZDraw);
//    
//    glVertex3f(boundingBoxXDraw, boundingBoxYDraw, -boundingBoxZDraw);
//    glVertex3f(-boundingBoxXDraw, boundingBoxYDraw, -boundingBoxZDraw);
//    
//    glColor3f(1.0f, 0, 1);
//    
//    glEnd();
//    
//    glEnable(GL_LIGHTING);
//}

/*!
 *	Renders the Bounding Box
 *  \author Timm Meyer
 */
void Renderer::displayBoundingBox() 
{
    //cout << "Renderer::displayBoundingBox beg" << endl;
    glDisable(GL_LIGHTING);
    
    glBegin(GL_LINES);
    // BoundingBox corners
    glColor3f(boundingBoxColor.at(0), boundingBoxColor.at(1) , boundingBoxColor.at(2));
    
    glVertex3f( boundingBoxCoordinates.at( 0).at(0), boundingBoxCoordinates.at( 0).at(1), boundingBoxCoordinates.at( 0).at(2) );
    glVertex3f( boundingBoxCoordinates.at( 1).at(0), boundingBoxCoordinates.at( 1).at(1), boundingBoxCoordinates.at( 1).at(2) );
    
    glVertex3f( boundingBoxCoordinates.at( 2).at(0), boundingBoxCoordinates.at( 2).at(1), boundingBoxCoordinates.at( 2).at(2) );
    glVertex3f( boundingBoxCoordinates.at( 3).at(0), boundingBoxCoordinates.at( 3).at(1), boundingBoxCoordinates.at( 3).at(2) );
    
    glVertex3f( boundingBoxCoordinates.at( 4).at(0), boundingBoxCoordinates.at( 4).at(1), boundingBoxCoordinates.at( 4).at(2) );
    glVertex3f( boundingBoxCoordinates.at( 5).at(0), boundingBoxCoordinates.at( 5).at(1), boundingBoxCoordinates.at( 5).at(2) );
    
    glVertex3f( boundingBoxCoordinates.at( 6).at(0), boundingBoxCoordinates.at( 6).at(1), boundingBoxCoordinates.at( 6).at(2) );
    glVertex3f( boundingBoxCoordinates.at( 7).at(0), boundingBoxCoordinates.at( 7).at(1), boundingBoxCoordinates.at( 7).at(2) );
    
    glVertex3f( boundingBoxCoordinates.at( 8).at(0), boundingBoxCoordinates.at( 8).at(1), boundingBoxCoordinates.at( 8).at(2) );
    glVertex3f( boundingBoxCoordinates.at( 9).at(0), boundingBoxCoordinates.at( 9).at(1), boundingBoxCoordinates.at( 9).at(2) );
    
    glVertex3f( boundingBoxCoordinates.at(10).at(0), boundingBoxCoordinates.at(10).at(1), boundingBoxCoordinates.at(10).at(2) );
    glVertex3f( boundingBoxCoordinates.at(11).at(0), boundingBoxCoordinates.at(11).at(1), boundingBoxCoordinates.at(11).at(2) );
    
    glVertex3f( boundingBoxCoordinates.at(12).at(0), boundingBoxCoordinates.at(12).at(1), boundingBoxCoordinates.at(12).at(2) );
    glVertex3f( boundingBoxCoordinates.at(13).at(0), boundingBoxCoordinates.at(13).at(1), boundingBoxCoordinates.at(13).at(2) );
    
    glVertex3f( boundingBoxCoordinates.at(14).at(0), boundingBoxCoordinates.at(14).at(1), boundingBoxCoordinates.at(14).at(2) );
    glVertex3f( boundingBoxCoordinates.at(15).at(0), boundingBoxCoordinates.at(15).at(1), boundingBoxCoordinates.at(15).at(2) );
    
    glVertex3f( boundingBoxCoordinates.at(16).at(0), boundingBoxCoordinates.at(16).at(1), boundingBoxCoordinates.at(16).at(2) );
    glVertex3f( boundingBoxCoordinates.at(17).at(0), boundingBoxCoordinates.at(17).at(1), boundingBoxCoordinates.at(17).at(2) );
    
    glVertex3f( boundingBoxCoordinates.at(18).at(0), boundingBoxCoordinates.at(18).at(1), boundingBoxCoordinates.at(18).at(2) );
    glVertex3f( boundingBoxCoordinates.at(19).at(0), boundingBoxCoordinates.at(19).at(1), boundingBoxCoordinates.at(19).at(2) );
    
    glVertex3f( boundingBoxCoordinates.at(20).at(0), boundingBoxCoordinates.at(20).at(1), boundingBoxCoordinates.at(20).at(2) );
    glVertex3f( boundingBoxCoordinates.at(21).at(0), boundingBoxCoordinates.at(21).at(1), boundingBoxCoordinates.at(21).at(2) );
    
    glVertex3f( boundingBoxCoordinates.at(22).at(0), boundingBoxCoordinates.at(22).at(1), boundingBoxCoordinates.at(22).at(2) );
    glVertex3f( boundingBoxCoordinates.at(23).at(0), boundingBoxCoordinates.at(23).at(1), boundingBoxCoordinates.at(23).at(2) );
    
    glColor3f(1.0f, 0, 1);
    
    glEnd();
    
    glEnable(GL_LIGHTING);
    //cout << "Renderer::displayBoundingBox end" << endl;
}

/*!
 *	Auto-increases the lod layers
 *
 *  \param lev how much the lod should be increased
 *  \return true if any value has changed
 *  \author Timm Meyer
 */
bool Renderer::increaseLOD(int lev) {
    //cout << "Renderer::increaseLOD() beg" << endl;
    LODdelta -= 1;
    LODdelta = max(LODdelta, 0);
    
    int startlev = lev;
    for (int i = 0; i < (int)layerLOD.size(); i++) {
	if (layerLOD[i] > 0) {
	    if (layerLOD[i] >= lev) {
		layerLOD[i] -= lev;
		//cout << "Renderer::increaseLOD() end" << endl;
		return true;
	    }
	    lev -= layerLOD[i];
	    layerLOD[i] = 0;
	}
    }
    if (startlev == lev) {
	//cout << "Renderer::increaseLOD() end" << endl;
	return false; // nothing changed
    }
    //cout << "Renderer::increaseLOD() end" << endl;
    return true;
}

/*!
 *	Auto-decreases the lod layers
 *
 *  \param lev how much the lod should be decreased
 *  \return true if any value has changed
 *  \author Timm Meyer
 */
bool Renderer::decreaseLOD(int lev) {
    //cout << "Renderer::decreaseLOD() beg" << endl;
    LODdelta += 1;
    LODdelta = min(LODdelta,modelLevels - 1);
    
    int startlev = lev;
    for (int i = layerLOD.size() - 1; i >= 0; i--) {
	if (layerLOD[i] < modelLevels - 1) {
	    if (((modelLevels - 1) - layerLOD[i]) >= lev) {
		layerLOD[i] += lev;
		//cout << "Renderer::decreaseLOD() end" << endl;
		return true;
	    }
	    lev -= (modelLevels - 1) - layerLOD[i];
	    layerLOD[i] = (modelLevels - 1);
	}
    }
    if (startlev == lev) {
	//cout << "Renderer::decreaseLOD() end" << endl;
	return false; // nothing changed
    }
    //cout << "Renderer::decreaseLOD() end" << endl;
    return true;
}

/*!
 *	Checks if the fps amount is to low, so that we have to change the lod values, or
 *	if the fps amount is to high, and we could change the lod values to get higher quality.
 *
 * 	\param elapsed the time needed to draw this frame (in millisecs)
 *  \author Timm Meyer
 */
void  Renderer::processFPS(int elapsed) {
    //cout << "Renderer::processFPS() beg" << endl;
    
    emit fpsUpdate(1000.0/(elapsed ? elapsed : 1.0) );
    
    if (difftime(time(NULL), render_LastFrameStartTime) >= 0.3) {
	render_LastFrameStartTime = time(NULL);
	
	
	if (renderSet_UseAutoLOD) {
	    if (elapsed > 1000/2) {
		if (decreaseLOD(25)) {
		    //		    cout << "AUTO-ADJUST -10" << endl;
		    //		    printLOD();
		}
	    } else if (elapsed > 1000/4) {
		if (decreaseLOD(15)) {
		    //		    cout << "AUTO-ADJUST -5" << endl;
		    //		    printLOD();
		}
	    } else if (elapsed > 1000/8) {
		if (decreaseLOD(3)) {
		    //		    cout << "AUTO-ADJUST -3" << endl;
		    //		    printLOD();
		}
	    } else if (elapsed < 1000/30) {
		if (increaseLOD(13)) {
		    //		    cout << "AUTO-ADJUST +8" << endl;
		    //		    printLOD();
		}
	    } else if (elapsed < 1000/15) {
		if (increaseLOD(6)) {
		    //		    cout << "AUTO-ADJUST +3" << endl;
		    //		    printLOD();
		}
	    }
	    
	}
    }
    //cout << "Renderer::processFPS() end" << endl;
}

inline void Renderer::swap(int i, int j) {
    //cout << "Renderer::swap() beg" << endl;
    float ftemp = 0;
    int itemp = 0;
    ftemp = sqDist[i];
    sqDist[i] = sqDist[j];
    sqDist[j] = ftemp;
    ftemp = xCoord[i];
    xCoord[i] = xCoord[j];
    xCoord[j] = ftemp;
    ftemp = yCoord[i];
    yCoord[i] = yCoord[j];
    yCoord[j] = ftemp;
    ftemp = zCoord[i];
    zCoord[i] = zCoord[j];
    zCoord[j] = ftemp;
    itemp = xDir[i];
    xDir[i] = xDir[j];
    xDir[j] = itemp;
    itemp = yDir[i];
    yDir[i] = yDir[j];
    yDir[j] = itemp;
    itemp = zDir[i];
    zDir[i] = zDir[j];
    zDir[j] = itemp;
    //cout << "Renderer::swap() end" << endl;
}

/*!
 *	Used to force gui update, either if force is true or the last is longer than 0.3 secs away
 *
 *  \param force used to force update
 *  \author Timm Meyer
 */
void Renderer::updateGuiF(bool force) {
    //cout << "Renderer::updateGuiF(bool) beg" << endl;
    if (force || difftime(time(NULL), lastUpdate) > 0.3) {
	updateGui = true;
	
	lastUpdate = time(NULL);
    }
    //cout << "Renderer::updateGuiF(bool) end" << endl;
}

/*!
 *	Update the gui values
 *
 *  \author Timm Meyer
 */
void Renderer::updateGuiF() {
    //cout << "Renderer::updateGuiF() beg" << updateGui << endl;
    if (updateGui) {
	updateGui = false;
	lastUpdate = time(NULL);
	
	float angle_x = XRealRot;
	float angle_y = YRealRot;
	float angle_z = ZRealRot;
	
	emit angleXChanged((int)angle_x);
	emit angleYChanged((int)angle_y);
	emit angleZChanged((int)angle_z);
	
	float xPos = (float) (sin(M_PI * 2 * YRealRot / 360) * abs(cos(M_PI * 2 * XRealRot / 360)) * distance);
	float yPos = (float) (-sin(M_PI * 2 * XRealRot / 360) * distance);
	float zPos = (float) (cos(M_PI * 2 * YRealRot / 360) * abs(cos(M_PI * 2 * XRealRot / 360)) * distance);
	
	int count = 0;
	// calculate the corner distances
	for (int i = -1; i < 2; i += 2) {
	    for (int j = -1; j < 2; j += 2) {
		for (int k = -1; k < 2; k += 2) {
		    xCoord[count] = i == -1 ? boundingBoxCoords[BOX_COORD_X_LOW] : boundingBoxCoords[BOX_COORD_X_HIGH];
		    yCoord[count] = j == -1 ? boundingBoxCoords[BOX_COORD_Y_LOW] : boundingBoxCoords[BOX_COORD_Y_HIGH];
		    zCoord[count] = k == -1 ? boundingBoxCoords[BOX_COORD_Z_LOW] : boundingBoxCoords[BOX_COORD_Z_HIGH];
		    sqDist[count] = (xPos - xCoord[count]) * (xPos - xCoord[count]) + (yPos - yCoord[count]) * (yPos - yCoord[count]) + (zPos - zCoord[count]) * (zPos - zCoord[count]);
		    xDir[count] = i;
		    yDir[count] = j;
		    zDir[count] = k;
		    count++;
		}
	    }
	}
	
	// sort after the 4 nearest and the farthest => 1 loop
	// for nearest and afterwars 3 for the remaining
	
	int nearestIndex = 0;
	int furthestIndex = 0;
	for (int i = 1; i < 8; i++) {
	    if (sqDist[i] < sqDist[nearestIndex]) {
		nearestIndex = i;
	    } else if (sqDist[i] > sqDist[furthestIndex]) {
		furthestIndex = i;
	    }
	}
	// nearest
	swap(0, nearestIndex);
	if (furthestIndex == 0) {
	    furthestIndex = nearestIndex; // because of swap
	}
	swap(7, furthestIndex);
	
	for (int j = 0; j < 3; j++) {
	    nearestIndex = 1 + j;
	    for (int i = 2 + j; i < 7; i++) {
		if (sqDist[i] < sqDist[nearestIndex]) {
		    nearestIndex = i;
		}
	    }
	    swap(1 + j, nearestIndex);
	}
	
	int lastPerc = (int) (100 * (sqDist[3] - sqDist[0]) / (sqDist[7] - sqDist[0]));
	if (xDir[0] == xDir[1] && xDir[1] == xDir[2] && xDir[2] == xDir[3] && lastPerc <= renderSet_RenderSideStartingPercent) {
	    renderSet_RenderSide = RENDERSIDE_X;
	} else if (yDir[0] == yDir[1] && yDir[1] == yDir[2] && yDir[2] == yDir[3] && lastPerc <= renderSet_RenderSideStartingPercent) {
	    renderSet_RenderSide = RENDERSIDE_Y;
	} else if (zDir[0] == zDir[1] && zDir[1] == zDir[2] && zDir[2] == zDir[3] && lastPerc <= renderSet_RenderSideStartingPercent) {
	    renderSet_RenderSide = RENDERSIDE_Z;
	} else {
	    renderSet_RenderSide = RENDERSIDE_NONE;
	}
	
	if (xDir[0] < 0) {
	    startBox_X = 0;
	    startBox_DeltaX = 1;
	} else {
	    startBox_X = smallBoxes.size() - 1;
	    startBox_DeltaX = -1;
	}
	
	if (yDir[0] < 0) {
	    startBox_Y = 0;
	    startBox_DeltaY = 1;
	} else {
	    startBox_Y = smallBoxes[0].size() - 1;
	    startBox_DeltaY = -1;
	}
	
	if (zDir[0] < 0) {
	    startBox_Z = 0;
	    startBox_DeltaZ = 1;
	} else {
	    startBox_Z = smallBoxes[0][0].size() - 1;
	    startBox_DeltaZ = -1;
	}
    }    
    //cout << "Renderer::updateGuiF() end" << endl;
}

/*!
 *	Used to set the model params
 *
 *  \param compxmax, compymax, compxmin, compymin, levels the complexity (min, max) of the models and the number of interpolated levels.
 *  \param data the data for defining the model (elliptical, spherocyl, wireframe)
 *  \return 
 *  \author Timm Meyer
 */
void Renderer::setModelParams(int compxmax, int compymax, int compxmin, int compymin, int levels, vector<vector<float> > data) {
    //cout << "Renderer::setModelParams beg" << endl;    
    
    if (levels < 1) {
	levels = 1;
    }
    
    // reihenfolge in data scaleX,Y,Z,sphRadius,sphLänge, wireframe (!= 0)
    objectParams = data;
    modelListIndex.resize(data.size());
    
    //cout << "modelListIndex.resize(data.size()) " << data.size() << endl;
    
    modelListSimpleIndex.resize(data.size());
    
    modelXComplexity_max = compxmax;
    modelYComplexity_max = compymax;
    modelXComplexity_min = compxmin;
    modelYComplexity_min = compymin;
    //    modelXScale = scalX/2.0;
    //    modelYScale = scalY/2.0;
    //    modelZScale = scalZ/2.0;
    if (levels < modelLevels) {
	for (int i = 0; i < (int)layerLOD.size(); i++) {
	    if (layerLOD[i] >= levels) {
		layerLOD[i] = levels - 1;
	    }
	}
    }
    modelLevels = levels;
    
    recreateModel = true;
    repaint();
    
    //cout << "Renderer::setModelParams end" << endl;    
}

/*!
 *
 *
 *  \param 
 *  \return 
 *  \author Timm Meyer
 */
float Renderer::getLength(float* b) {
    //cout << "Renderer::getLength(float*) beg" << endl;    
    //cout << "Renderer::getLength(flaot*) end" << endl;    
    return (float) sqrt(b[0] * b[0] + b[1] * b[1] + b[2] * b[2]);
}

/*!
 *
 *
 *  \param 
 *  \return 
 *  \author Timm Meyer
 */
float Renderer::getLength(float x, float y, float z) {
    //cout << "Renderer::getLength(float,float,float) beg" << endl;    
    //cout << "Renderer::getLength(float,float,float) end" << endl;    
    return (float) sqrt(x * x + y * y + z * z);
}

/*!
 *
 *
 *  \param 
 *  \return 
 *  \author Timm Meyer
 */
float* Renderer::normalizeV(float x, float y, float z, float scaleY) {
    //cout << "Renderer::normalizeV(float,float,float,float) beg" << endl;    
    y /= (scaleY * scaleY);
    float len = getLength(x, y, z);
    speedArr[0] = x / len;
    speedArr[1] = y / len;
    speedArr[2] = z / len;
    //cout << "Renderer::normalizeV(float,float,float,float) end" << endl;    
    return speedArr;
}

/*!
 *
 *
 *  \param 
 *  \return 
 *  \author Timm Meyer
 */
float* Renderer::normalizeV(float* b, float scaleX, float scaleY, float scaleZ) {
    //cout << "Renderer::normalizeV(float*,float,float,float) beg" << endl;    
    float temp[] = {b[0], b[1], b[2]};
    
    b[0] /= (scaleX * scaleX);
    b[1] /= (scaleY * scaleY);
    b[2] /= (scaleZ * scaleZ);
    float len = getLength(b);
    
    speedArr[0] = b[0] / len;
    speedArr[1] = b[1] / len;
    speedArr[2] = b[2] / len;
    b[0] = temp[0];
    b[1] = temp[1];
    b[2] = temp[2];
    //cout << "Renderer::normalizeV(float*,float,float,float) end" << endl;    
    return speedArr;
}

/*!
 *
 *
 *  \param 
 *  \return 
 *  \author Timm Meyer
 */
float* Renderer::normalizeV(float* b, float scaleY) {
    //cout << "Renderer::normalizeV(float*,float) beg" << endl;    
    float temp = b[1];
    b[1] /= (scaleY * scaleY);
    float len = getLength(b);
    speedArr[0] = b[0] / len;
    speedArr[1] = b[1] / len;
    speedArr[2] = b[2] / len;
    b[1] = temp;
    //cout << "Renderer::normalizeV(float*,float) end" << endl;    
    return speedArr;
}

void Renderer::createModels2Wireframe(int index)
{
    //cout << "Renderer::createModels2Wireframe() beg" << endl;    
    float radius    = objectParams.at(index).at(4);
    float cylLength = objectParams.at(index).at(5);
    
    float fv[3];
    for( int actLev = 0; actLev < modelLevels; ++actLev )
    {
	glNewList(modelListIndex.at(index) + actLev, GL_COMPILE);
	
	int xCompl = (int) (modelXComplexity_max + actLev * ((modelXComplexity_min - modelXComplexity_max) / (modelLevels - 1.0)));
	
	int yCompl = xCompl/4;
	
	float xPiece = M_PI*2/xCompl;
	float yPiece = M_PI*2/(yCompl*4);
	
	for( int yEbene = 0; yEbene < (yCompl-1); ++yEbene )
	{
	    glBegin(GL_LINES);
	    for( int xPart = 0; xPart <= xCompl; ++xPart )
	    {
		// Oben
		if( xPart  == 0 || xPart  == xCompl )
		{
		    fv[0] = (float) ( -sin((yEbene + 1) * yPiece) * radius );
		    fv[1] = 0;
		}
		else
		{
		    fv[0] =  (float) ( -cos(xPart  * xPiece) * sin((yEbene  + 1) * yPiece) * radius );
		    fv[1] = -(float) (  sin(xPart  * xPiece) * sin((yEbene  + 1) * yPiece) * radius );
		}
		fv[2] = (float) ( cos((yEbene + 1) * yPiece) * radius );
		
		glNormal3fv( normalizeV(fv, 1, 1, 1) );
		fv[2] += cylLength/2;
		glVertex3fv(fv);
		
		// Unten
		if( xPart == 0 || xPart == xCompl )
		{
		    fv[0] = (float) ( -sin((yEbene+2) * yPiece) * radius );
		    fv[1] = 0;
		}
		else
		{
		    fv[0] =  (float) ( -cos(xPart * xPiece) * sin((yEbene+2) * yPiece) * radius );
		    fv[1] = -(float) (  sin(xPart * xPiece) * sin((yEbene+2) * yPiece) * radius );
		}
		fv[2] = (float) ( cos((yEbene+2) * yPiece) * radius );
		
		glNormal3fv( normalizeV(fv, 1,1,1) );
		fv[2] += cylLength/2;
		glVertex3fv(fv);
		
		// Oben - waagerecht
		if( xPart  == 0 || xPart  == xCompl )
		{
		    fv[0] = (float) ( -sin((yEbene + 1) * yPiece) * radius );
		    fv[1] = 0;
		} 
		else
		{
		    fv[0] =  (float) ( -cos(xPart  * xPiece) * sin((yEbene  + 1) * yPiece) ) * radius;
		    fv[1] = -(float) (  sin(xPart  * xPiece) * sin((yEbene  + 1) * yPiece) ) * radius;
		}
		fv[2] = (float) ( cos((yEbene  + 1) * yPiece) * radius );
		
		glNormal3fv( normalizeV(fv, 1, 1, 1) );
		fv[2] += cylLength/2;
		glVertex3fv(fv);
		
		if( (xPart+1)  == 0 || (xPart+1)  == xCompl )
		{
		    fv[0] = (float) ( -sin((yEbene + 1) * yPiece) * radius );
		    fv[1] = 0;
		}
		else
		{
		    fv[0] =  (float) ( -cos((xPart+1)  * xPiece) * sin((yEbene  + 1) * yPiece) * radius );
		    fv[1] = -(float) (  sin((xPart+1)  * xPiece) * sin((yEbene  + 1) * yPiece) * radius );
		}
		fv[2] = (float) ( cos((yEbene  + 1) * yPiece) * radius );
		
		glNormal3fv( normalizeV(fv, 1, 1, 1) );
		fv[2] += cylLength/2;
		glVertex3fv(fv);
		
		// Unten - waagerecht
		if( xPart == 0 || xPart == xCompl )
		{
		    fv[0] = (float) ( -sin((yEbene+2) * yPiece) * radius );
		    fv[1] = 0;
		}
		else
		{
		    fv[0] =  (float) ( -cos(xPart * xPiece) * sin((yEbene+2) * yPiece) * radius );
		    fv[1] = -(float) (  sin(xPart * xPiece) * sin((yEbene+2) * yPiece) * radius );
		}
		fv[2] = (float) ( cos((yEbene+2) * yPiece) * radius );
		
		glNormal3fv( normalizeV(fv, 1,1,1) );
		fv[2] += cylLength/2;
		glVertex3fv(fv);
		
		if( (xPart+1) == 0 || (xPart+1) == xCompl )
		{
		    fv[0] = (float) ( -sin((yEbene+2) * yPiece) * radius );
		    fv[1] = 0;
		}
		else
		{
		    fv[0] =  (float) ( -cos((xPart+1) * xPiece) * sin((yEbene+2) * yPiece) * radius );
		    fv[1] = -(float) (  sin((xPart+1) * xPiece) * sin((yEbene+2) * yPiece) * radius );
		}
		fv[2] = (float) ( cos((yEbene+2) * yPiece) * radius );
		
		glNormal3fv( normalizeV(fv, 1,1,1) );
		fv[2] += cylLength/2;
		glVertex3fv(fv);
	    }
	    glEnd();
	}
	
	glBegin(GL_LINES);
	for( int xPart = 0; xPart <= xCompl; ++xPart )
	{
	    if (xPart == 0 || xPart == xCompl )
	    {
		fv[0] = -radius;
		fv[1] = 0;
	    }
	    else
	    {
		fv[0] =  (float) ( -cos(xPart * xPiece) * radius );
		fv[1] = -(float) (  sin(xPart * xPiece) * radius );
	    }
	    fv[2] = 0;
	    
	    glNormal3fv( normalizeV(fv, 1,1,1) );
	    fv[2] += cylLength/2;
	    glVertex3fv(fv);
	    
	    fv[2] = 0;
	    glNormal3fv( normalizeV(fv, 1,1,1) );
	    fv[2] -= cylLength/2;
	    glVertex3fv(fv);
	}
	glEnd();
	
	for( int yEbene = yCompl-1; yEbene < 2*(yCompl-1); ++yEbene )
	{
	    glBegin(GL_LINES);
	    for( int xPart = 0; xPart <= xCompl; ++xPart )
	    {
		// Oben
		if( xPart  == 0 || xPart  == xCompl )
		{
		    fv[0] = (float) ( -sin((yEbene + 1) * yPiece) * radius );
		    fv[1] = 0;
		}
		else
		{
		    fv[0] =  (float) ( -cos(xPart  * xPiece) * sin((yEbene  + 1) * yPiece) * radius );
		    fv[1] = -(float) (  sin(xPart  * xPiece) * sin((yEbene  + 1) * yPiece) * radius );
		}
		fv[2] = (float) ( cos((yEbene  + 1) * yPiece) * radius );
		
		glNormal3fv( normalizeV(fv, 1, 1, 1) );
		fv[2] -= cylLength/2;
		glVertex3fv(fv);
		
		// Unten
		if( xPart == 0 || xPart == xCompl )
		{
		    fv[0] = (float) ( -sin((yEbene+2) * yPiece) * radius );
		    fv[1] = 0;
		}
		else
		{
		    fv[0] =  (float) ( -cos(xPart * xPiece) * sin((yEbene+2) * yPiece) * radius );
		    fv[1] = -(float) (  sin(xPart * xPiece) * sin((yEbene+2) * yPiece) * radius );
		}
		fv[2] = (float) ( cos((yEbene+2) * yPiece) * radius );
		
		glNormal3fv( normalizeV(fv, 1,1,1) );
		fv[2] -= cylLength/2;
		glVertex3fv(fv);
		
		// Oben - waagerecht
		if( xPart  == 0 || xPart  == xCompl )
		{
		    fv[0] = (float) ( -sin((yEbene + 1) * yPiece) * radius );
		    fv[1] = 0;
		}
		else
		{
		    fv[0] =  (float) ( -cos(xPart * xPiece) * sin((yEbene  + 1) * yPiece) * radius );
		    fv[1] = -(float) (  sin(xPart * xPiece) * sin((yEbene  + 1) * yPiece) * radius );
		}
		fv[2] = (float) ( cos((yEbene  + 1) * yPiece) * radius );
		
		glNormal3fv( normalizeV(fv, 1, 1, 1) );
		fv[2] -= cylLength/2;
		glVertex3fv(fv);
		
		if( (xPart+1)  == 0 || (xPart+1)  == xCompl )
		{
		    fv[0] = (float) ( -sin((yEbene + 1) * yPiece) * radius );
		    fv[1] = 0;
		}
		else
		{
		    fv[0] =  (float) ( -cos((xPart+1) * xPiece) * sin((yEbene  + 1) * yPiece) * radius );
		    fv[1] = -(float) (  sin((xPart+1) * xPiece) * sin((yEbene  + 1) * yPiece) * radius );
		}
		fv[2] = (float) ( cos((yEbene + 1) * yPiece) * radius );
		
		glNormal3fv( normalizeV(fv, 1, 1, 1) );
		fv[2] -= cylLength/2;
		glVertex3fv(fv);
		
		// Unten - waagerecht
		if( xPart == 0 || xPart == xCompl )
		{
		    fv[0] = (float) ( -sin((yEbene+2) * yPiece) * radius );
		    fv[1] = 0;
		}
		else
		{
		    fv[0] =  (float) ( -cos(xPart * xPiece) * sin((yEbene+2) * yPiece) * radius );
		    fv[1] = -(float) (  sin(xPart * xPiece) * sin((yEbene+2) * yPiece) * radius );
		}
		fv[2] = (float) ( cos((yEbene+2) * yPiece) * radius );
		
		glNormal3fv( normalizeV(fv, 1,1,1) );
		fv[2] -= cylLength/2;
		glVertex3fv(fv);
		
		if( (xPart+1) == 0 || (xPart+1) == xCompl )
		{
		    fv[0] = (float) ( -sin((yEbene+2) * yPiece) * radius );
		    fv[1] = 0;
		}
		else
		{
		    fv[0] =  (float) ( -cos((xPart+1) * xPiece) * sin((yEbene+2) * yPiece) * radius );
		    fv[1] = -(float) (  sin((xPart+1) * xPiece) * sin((yEbene+2) * yPiece) * radius );
		}
		fv[2] = (float) ( cos((yEbene+2) * yPiece) * radius );
		
		glNormal3fv( normalizeV(fv, 1,1,1) );
		fv[2] -= cylLength/2;
		glVertex3fv(fv);
	    }
	    glEnd();
	}
	
	// OBEN
	glBegin(GL_LINES);
	for( int j = 0; j <=  xCompl; ++j )
	{
	    fv[0] = 0;
	    fv[1] = 0;
	    fv[2] = radius;
	    
	    glNormal3fv( normalizeV(fv, 1, 1, 1) );
	    fv[2] += cylLength/2;
	    glVertex3fv(fv);
	    
	    if( j == 0 || j == xCompl )
	    {
		fv[0] = (float) ( -sin(yPiece) * radius );
		fv[1] = 0;
	    } 
	    else
	    {
		fv[0] =  (float) ( -cos(j * xPiece) * sin(yPiece) * radius );
		fv[1] = -(float) (  sin(j * xPiece) * sin(yPiece) * radius );
	    }
	    fv[2] = (float) ( cos(yPiece) * radius );
	    
	    glNormal3fv( normalizeV(fv, 1,1,1) );
	    fv[2] += cylLength/2;
	    glVertex3fv(fv);
	    
	    // Waagerecht
	    if( j == 0 || j == xCompl )
	    {
		fv[0] = (float) ( -sin(yPiece) * radius );
		fv[1] = 0;
	    }
	    else
	    {
		fv[0] =  (float) ( -cos(j * xPiece) * sin(yPiece) * radius );
		fv[1] = -(float) (  sin(j * xPiece) * sin(yPiece) * radius );
	    }
	    fv[2] = (float) ( cos(yPiece) * radius );
	    
	    glNormal3fv( normalizeV(fv, 1,1,1) );
	    fv[2] += cylLength/2;
	    glVertex3fv(fv);
	    
	    if( (j+1) == 0 || (j+1) == xCompl )
	    {
		fv[0] = (float) ( -sin(yPiece) * radius );
		fv[1] = 0;
	    }
	    else 
	    {
		fv[0] =  (float) ( -cos((j+1) * xPiece) * sin(yPiece) * radius );
		fv[1] = -(float) (  sin((j+1) * xPiece) * sin(yPiece) * radius );
	    }
	    fv[2] = (float) ( cos(yPiece) * radius );
	    
	    glNormal3fv( normalizeV(fv, 1,1,1) );
	    fv[2] += cylLength/2;
	    glVertex3fv(fv);
	}
	glEnd();
	
	// UNTEN
	glBegin(GL_LINES);
	for( int j = xCompl; j >= 0; --j )
	{
	    fv[0] = 0;
	    fv[1] = 0;
	    fv[2] = -radius;
	    
	    glNormal3fv( normalizeV(fv, 1,1,1) );
	    fv[2] -= cylLength/2;
	    glVertex3fv(fv);
	    
	    if( j == 0 || j == xCompl )
	    {
		fv[0] = (float) ( -sin(yPiece) * radius );
		fv[1] = 0;
	    }
	    else
	    {
		fv[0] =  (float) ( -cos(j * xPiece) * sin(yPiece) * radius );
		fv[1] = -(float) (  sin(j * xPiece) * sin(yPiece) * radius );
	    }
	    fv[2] = (float) ( -cos(yPiece) * radius );
	    
	    glNormal3fv( normalizeV(fv, 1,1,1) );
	    fv[2] -= cylLength/2;
	    glVertex3fv(fv);
	    
	    // Waagerecht
	    if( j == 0 || j == xCompl )
	    {
		fv[0] = (float) ( -sin(yPiece) * radius );
		fv[1] = 0;
	    }
	    else
	    {
		fv[0] =  (float) ( -cos(j * xPiece) * sin(yPiece) * radius );
		fv[1] = -(float) (  sin(j * xPiece) * sin(yPiece) * radius );
	    }
	    fv[2] = (float) ( -cos(yPiece) * radius );
	    
	    glNormal3fv( normalizeV(fv, 1,1,1) );
	    fv[2] -= cylLength/2;
	    glVertex3fv(fv);
	    
	    if( (j+1) == 0 || (j+1) == xCompl )
	    {
		fv[0] = (float) ( -sin(yPiece) * radius );
		fv[1] = 0;
	    }
	    else
	    {
		fv[0] =  (float) ( -cos((j+1) * xPiece) * sin(yPiece) * radius );
		fv[1] = -(float) (  sin((j+1) * xPiece) * sin(yPiece) * radius );
	    }
	    fv[2] = (float) ( -cos(yPiece) * radius );
	    
	    glNormal3fv( normalizeV(fv, 1,1,1) );
	    fv[2] -= cylLength/2;
	    glVertex3fv(fv);
	}
	glEnd();
	
	glEndList();
    }
    
    //cout << "Renderer::createModels2Wireframe() end" << endl;    
}

void Renderer::createModels2(int index) 
{
    //cout << "Renderer::createModels2() beg" << endl;    
    modelListSimpleIndex.at(index) = glGenLists(1);
    
    glNewList(modelListSimpleIndex.at(index), GL_COMPILE);
    glBegin(GL_LINES);
    glVertex3f( 0, 0,  0.5 );
    glVertex3f( 0, 0, -0.5 );
    glEnd();
    glEndList();
    
    modelListIndex.at(index) = glGenLists( modelLevels );
    
    if( objectParams.at(index).at(12) != 0 )
    {
	createModels2Wireframe(index);
	return;
    }
    
    float radius    = objectParams.at(index).at(4);
    float cylLength = objectParams.at(index).at(5);
    
    float fv[3];
    for( int actLev = 0; actLev < modelLevels; ++actLev )
    {
	glNewList( modelListIndex.at(index) + actLev, GL_COMPILE );
	
	int xCompl = (int) ( modelXComplexity_max + actLev * ( (modelXComplexity_min - modelXComplexity_max) / (modelLevels - 1.0) ) );
	int yCompl = xCompl/4;
	
	float xPiece = M_PI*2 /  xCompl;
	float yPiece = M_PI*2 / (yCompl*4);
	
	for( int yEbene = 0; yEbene < (yCompl-1); ++yEbene )
	{
	    glBegin(GL_TRIANGLE_STRIP);
	    for( int xPart = 0; xPart <= xCompl; ++xPart ) 
	    {
		if( xPart == 0 || xPart == xCompl ) 
		{
		    fv[0] = (float) ( -1 * sin((yEbene + 1) * yPiece) ) * radius ;
		    fv[1] = 0;
		} 
		else
		{
		    fv[0] =  (float) ( -cos(xPart  * xPiece) * sin((yEbene + 1) * yPiece) ) * radius;
		    fv[1] = -(float) (  sin(xPart  * xPiece) * sin((yEbene + 1) * yPiece) ) * radius;
		}
		fv[2] = (float) ( cos((yEbene + 1) * yPiece) ) * radius;
		
		glNormal3fv( normalizeV(fv, 1, 1, 1) );
		fv[2] += cylLength/2;
		glVertex3fv(fv);
		
		// Unten
		if( xPart == 0 || xPart == xCompl )
		{
		    fv[0] = (float) ( -1 * sin((yEbene+2) * yPiece) ) * radius;
		    fv[1] = 0;
		}
		else
		{
		    fv[0] =  (float) ( -cos(xPart * xPiece) * sin((yEbene + 2) * yPiece) ) * radius;
		    fv[1] = -(float) (  sin(xPart * xPiece) * sin((yEbene + 2) * yPiece) ) * radius;
		}
		fv[2] = (float) ( cos((yEbene+2) * yPiece) ) * radius;
		
		glNormal3fv( normalizeV(fv, 1, 1, 1) );
		fv[2] += cylLength/2;
		glVertex3fv(fv);
	    }
	    glEnd();
	}
	
	glBegin(GL_TRIANGLE_STRIP);
	for( int xPart = 0; xPart <= xCompl; ++xPart ) 
	{
	    if( xPart == 0 || xPart == xCompl ) 
	    {
		fv[0] = -1 * radius;
		fv[1] = 0;
	    }
	    else
	    {
		fv[0] =  (float) ( -cos(xPart * xPiece) ) * radius;
		fv[1] = -(float) (  sin(xPart * xPiece) ) * radius;
	    }
	    fv[2] = 0;
	    
	    glNormal3fv( normalizeV(fv, 1, 1, 1) );
	    fv[2] += cylLength/2;
	    glVertex3fv(fv);
	    
	    fv[2] = 0;
	    glNormal3fv( normalizeV(fv, 1, 1, 1) );
	    fv[2] -= cylLength/2;
	    glVertex3fv(fv);
	}
	glEnd();
	
	for( int yEbene = yCompl-1; yEbene < 2*(yCompl-1); ++yEbene ) 
	{
	    glBegin(GL_TRIANGLE_STRIP);
	    for( int xPart = 0; xPart <= xCompl; ++xPart ) 
	    {
		if( xPart  == 0 || xPart  == xCompl ) 
		{
		    fv[0] = (float) ( -1 * sin((yEbene + 1) * yPiece) ) * radius;
		    fv[1] = 0;
		} 
		else
		{
		    fv[0] =  (float) ( -cos(xPart  * xPiece) * sin((yEbene + 1) * yPiece) ) * radius;
		    fv[1] = -(float) (  sin(xPart  * xPiece) * sin((yEbene + 1) * yPiece) ) * radius;
		}
		fv[2] = (float) ( cos((yEbene  + 1) * yPiece) ) * radius;
		
		glNormal3fv( normalizeV(fv, 1, 1, 1) );
		fv[2] -= cylLength/2;
		glVertex3fv(fv);
		
		// Unten
		if( xPart == 0 || xPart == xCompl )
		{
		    fv[0] = (float) ( -1 * sin((yEbene+2) * yPiece) ) * radius;
		    fv[1] = 0;
		} 
		else 
		{
		    fv[0] =  (float) ( -cos(xPart * xPiece) * sin((yEbene + 2) * yPiece) ) * radius;
		    fv[1] = -(float) (  sin(xPart * xPiece) * sin((yEbene + 2) * yPiece) ) * radius;
		}
		fv[2] = (float) ( cos((yEbene + 2) * yPiece) ) * radius;
		
		glNormal3fv( normalizeV(fv, 1, 1, 1) );
		fv[2] -= cylLength/2;
		glVertex3fv(fv);
	    }
	    glEnd();
	}
	
	// OBEN
	glBegin(GL_TRIANGLE_FAN);
	fv[0] = 0;
	fv[1] = 0;
	fv[2] = radius;
	
	glNormal3fv( normalizeV(fv, 1, 1, 1) );
	fv[2] += cylLength/2;
	glVertex3fv(fv);
	
	for( int j = 0; j <=  xCompl; ++j )
	{
	    if( j == 0 || j == xCompl )
	    {
		fv[0] = (float) ( - sin(yPiece) * radius );
		fv[1] = 0;
	    }
	    else
	    {
		fv[0] =  (float) ( - cos(j * xPiece) * sin(yPiece) * radius );
		fv[1] = -(float) (   sin(j * xPiece) * sin(yPiece) * radius );
	    }
	    fv[2] = (float) ( cos(yPiece) * radius );
	    
	    glNormal3fv( normalizeV(fv, 1, 1, 1) );
	    fv[2] += cylLength/2;
	    glVertex3fv(fv);
	}
	glEnd();
	
	// UNTEN
	glBegin(GL_TRIANGLE_FAN);
	fv[0] = 0;
	fv[1] = 0;
	fv[2] = -radius;
	
	glNormal3fv( normalizeV(fv, 1, 1, 1) );
	fv[2] -= cylLength/2;
	glVertex3fv(fv);
	
	for( int j = xCompl; j >= 0; --j )
	{
	    if( j == 0 || j == xCompl )
	    {
		fv[0] = (float) ( -1 * sin(yPiece) * radius );
		fv[1] = 0;
	    } 
	    else
	    {
		fv[0] =  (float) ( -cos(j * xPiece) * sin(yPiece) * radius );
		fv[1] = -(float) (  sin(j * xPiece) * sin(yPiece) * radius );
	    }
	    fv[2] = (float) ( -cos(yPiece) * radius );
	    
	    glNormal3fv( normalizeV(fv, 1, 1, 1) );
	    fv[2] -= cylLength/2;
	    glVertex3fv(fv);
	}
	glEnd();
	
	glEndList();
    }
    //cout << "Renderer::createModels2() end" << endl;    
}

void Renderer::createModels1Wireframe(int index)
{
    //cout << "Renderer::createModels1Wireframe() beg" << endl;    
    float modelXScale = objectParams.at(index).at(1);
    float modelYScale = objectParams.at(index).at(2);
    float modelZScale = objectParams.at(index).at(3);
    
    for (int actLev = 0; actLev < modelLevels; actLev++) 
    {
	int actXCompl = (int) (modelXComplexity_max + actLev * ((modelXComplexity_min - modelXComplexity_max) / (modelLevels - 1.0)));
	int actYCompl = (int) (modelYComplexity_max + actLev * ((modelYComplexity_min - modelYComplexity_max) / (modelLevels - 1.0)));
	
	glNewList(modelListIndex.at(index) + actLev, GL_COMPILE);
	
	float yPiece = (float) (M_PI / ((actYCompl + 1) * 2));
	float xPiece = (float) (2 * M_PI / actXCompl);
	
	glBegin(GL_LINES);
	// WARNING - highly ineffizient... wireframe is JUST used for screenshots, so this is NOT optimized code!
	float fv[3];
	for( int i = 0; i < actYCompl * 2; ++i )
	{
	    for( int j = 0; j < actXCompl + 1; ++j )
	    {
		// Oben
		if( j == 0 || j == actXCompl )
		{
		    fv[0] = (float) ( -modelXScale * sin((i + 1) * yPiece) );
		    fv[1] = 0;
		}
		else
		{
		    fv[0] =  (float) ( -cos(j * xPiece) * modelXScale * sin((i + 1) * yPiece) );
		    fv[1] = -(float) (  sin(j * xPiece) * modelYScale * sin((i + 1) * yPiece) );
		}
		fv[2] = (float) ( cos((i + 1) * yPiece) * modelZScale );
		
		glNormal3fv( normalizeV(fv, modelXScale, modelYScale, modelZScale) );
		glVertex3fv(fv);
		
		// Unten
		if( j == 0 || j == actXCompl )
		{
		    fv[0] = (float) ( -modelXScale * sin((i + 2) * yPiece) );
		    fv[1] = 0;
		}
		else
		{
		    fv[0] =  (float) ( -cos(j * xPiece) * modelXScale * sin((i + 2) * yPiece) );
		    fv[1] = -(float) (  sin(j * xPiece) * modelYScale * sin((i + 2) * yPiece) );
		}
		fv[2] = (float) ( cos((i + 2) * yPiece) * modelZScale );
		
		glNormal3fv( normalizeV(fv, modelXScale, modelYScale, modelZScale) );
		glVertex3fv(fv);
		
		// Oben - waagerecht
		if( j == 0 || j == actXCompl )
		{
		    fv[0] = (float) ( -modelXScale * sin((i + 1) * yPiece) );
		    fv[1] = 0;
		}
		else
		{
		    fv[0] =  (float) ( -cos(j * xPiece) * modelXScale * sin((i + 1) * yPiece) );
		    fv[1] = -(float) (  sin(j * xPiece) * modelYScale * sin((i + 1) * yPiece) );
		}
		fv[2] = (float) (cos((i + 1) * yPiece) * modelZScale);
		
		glNormal3fv( normalizeV(fv, modelXScale, modelYScale, modelZScale) );
		glVertex3fv(fv);
		
		if( j+1 == 0 || j+1 == actXCompl )
		{
		    fv[0] = (float) ( -modelXScale * sin((i + 1) * yPiece) );
		    fv[1] = 0;
		}
		else
		{
		    fv[0] =  (float) ( -cos((j+1) * xPiece) * modelXScale * sin((i + 1) * yPiece) );
		    fv[1] = -(float) (  sin((j+1) * xPiece) * modelYScale * sin((i + 1) * yPiece) );
		}
		fv[2] = (float) ( cos((i + 1) * yPiece) * modelZScale );
		
		glNormal3fv( normalizeV(fv, modelXScale, modelYScale, modelZScale) );
		glVertex3fv(fv);
		
		// Unten
		if( j == 0 || j == actXCompl )
		{
		    fv[0] = (float) ( -modelXScale * sin((i + 2) * yPiece) );
		    fv[1] = 0;
		}
		else
		{
		    fv[0] =  (float) ( -cos(j * xPiece) * modelXScale * sin((i + 2) * yPiece) );
		    fv[1] = -(float) (  sin(j * xPiece) * modelYScale * sin((i + 2) * yPiece) );
		}
		fv[2] = (float) ( cos((i + 2) * yPiece) * modelZScale );
		
		glNormal3fv( normalizeV(fv, modelXScale, modelYScale, modelZScale) );
		glVertex3fv(fv);
		
		if( (j+1) == 0 || (j+1) == actXCompl ) 
		{
		    fv[0] = (float) ( -modelXScale * sin((i + 2) * yPiece) );
		    fv[1] = 0;
		} 
		else
		{
		    fv[0] =  (float) ( -cos((j+1) * xPiece) * modelXScale * sin((i + 2) * yPiece) );
		    fv[1] = -(float) (  sin((j+1) * xPiece) * modelYScale * sin((i + 2) * yPiece) );
		}
		fv[2] = (float) ( cos((i + 2) * yPiece) * modelZScale );
		
		glNormal3fv( normalizeV(fv, modelXScale, modelYScale, modelZScale) );
		glVertex3fv(fv);
	    }
	}
	glEnd();
	
	// OBEN
	glBegin(GL_LINES);
	for( int j = 0; j < actXCompl + 1; ++j )
	{
	    fv[0] = 0;
	    fv[1] = 0;
	    fv[2] = modelZScale;
	    
	    glNormal3fv( normalizeV(fv, modelXScale, modelYScale, modelZScale) );
	    glVertex3fv(fv);
	    
	    if( j == 0 || j == actXCompl )
	    {
		fv[0] = (float) ( -modelXScale * sin(yPiece) );
		fv[1] = 0;
	    }
	    else
	    {
		fv[0] =  (float) ( -cos(j * xPiece) * modelXScale * sin(yPiece) );
		fv[1] = -(float) (  sin(j * xPiece) * modelYScale * sin(yPiece) );
	    }
	    fv[2] = (float) ( cos(yPiece) * modelZScale );
	    
	    glNormal3fv( normalizeV(fv, modelXScale, modelYScale, modelZScale) );
	    glVertex3fv(fv);
	}
	glEnd();
	
	// UNTEN
	glBegin(GL_LINES);
	for( int j = actXCompl; j >= 0; --j )
	{
	    fv[0] = 0;
	    fv[1] = 0;
	    fv[2] = -modelZScale;
	    
	    glNormal3fv( normalizeV(fv, modelXScale, modelYScale, modelZScale) );
	    glVertex3fv(fv);
	    if( j == 0 || j == actXCompl )
	    {
		fv[0] = (float) ( -modelXScale * sin(yPiece) );
		fv[1] = 0;
	    }
	    else
	    {
		fv[0] =  (float) ( -cos(j * xPiece) * modelXScale * sin(yPiece) );
		fv[1] = -(float) (  sin(j * xPiece) * modelYScale * sin(yPiece) );
	    }
	    fv[2] = (float) ( -cos(yPiece) * modelZScale );
	    
	    glNormal3fv( normalizeV(fv, modelXScale, modelYScale, modelZScale) );
	    glVertex3fv(fv);
	}
	glEnd();
	
	glEndList();
    }
    //cout << "Renderer::createModels1Wireframe() end" << endl;    
}

void Renderer::createModels1(int index)
{
    //cout << "Renderer::createModels1() beg" << endl;    
    // simple Model
    
    float modelXScale = objectParams.at(index).at(1);
    float modelYScale = objectParams.at(index).at(2);
    float modelZScale = objectParams.at(index).at(3);
    
    modelListSimpleIndex.at(index) = glGenLists(1);
    glNewList(modelListSimpleIndex.at(index), GL_COMPILE);
    glBegin(GL_LINES);
    glVertex3f(0, 0,  modelZScale );
    glVertex3f(0, 0, -modelZScale );
    glEnd();
    glEndList();
    
    modelListIndex.at(index) = glGenLists(modelLevels);
    
    if( objectParams.at(index).at(12) != 0 )
    {
	createModels1Wireframe(index);
	return;
    }
    
    for( int actLev = 0; actLev < modelLevels; ++actLev )
    {
	int actXCompl = (int) ( modelXComplexity_max + actLev * ((modelXComplexity_min - modelXComplexity_max) / (modelLevels - 1.0)) );
	int actYCompl = (int) ( modelYComplexity_max + actLev * ((modelYComplexity_min - modelYComplexity_max) / (modelLevels - 1.0)) );
	
	glNewList(modelListIndex.at(index) + actLev, GL_COMPILE);
	float yPiece = (float) (M_PI / ((actYCompl + 1) * 2));
	float xPiece = (float) (2 * M_PI / actXCompl);
	float fv[3];
	
	glBegin(GL_TRIANGLE_STRIP);
	for( int i = 0; i < actYCompl * 2; ++i )
	{
	    for( int j = 0; j < actXCompl + 1; ++j )
	    {
		// Oben
		if( j == 0 || j == actXCompl )
		{
		    fv[0] = (float) ( -modelXScale * sin((i + 1) * yPiece) );
		    fv[1] = 0;
		}
		else
		{
		    fv[0] =  (float) ( -cos(j * xPiece) * modelXScale * sin((i + 1) * yPiece) );
		    fv[1] = -(float) (  sin(j * xPiece) * modelYScale * sin((i + 1) * yPiece) );
		}
		fv[2] = (float) ( cos((i + 1) * yPiece) * modelZScale );
		
		glNormal3fv( normalizeV(fv, modelXScale, modelYScale, modelZScale) );
		glVertex3fv(fv);
		
		// Unten
		if( j == 0 || j == actXCompl )
		{
		    fv[0] = (float) ( -modelXScale * sin((i + 2) * yPiece) );
		    fv[1] = 0;
		}
		else
		{
		    fv[0] =  (float) ( -cos(j * xPiece) * modelXScale * sin((i + 2) * yPiece) );
		    fv[1] = -(float) (  sin(j * xPiece) * modelYScale * sin((i + 2) * yPiece) );
		}
		fv[2] = (float) ( cos((i + 2) * yPiece) * modelZScale );
		
		glNormal3fv( normalizeV(fv, modelXScale, modelYScale, modelZScale) );
		glVertex3fv(fv);
	    }
	}
	glEnd();
	
	// OBEN
	glBegin(GL_TRIANGLE_FAN);
	fv[0] = 0;
	fv[1] = 0;
	fv[2] = modelZScale;
	
	glNormal3fv( normalizeV(fv, modelXScale, modelYScale, modelZScale) );
	glVertex3fv(fv);
	
	for( int j = 0; j < actXCompl + 1; ++j )
	{
	    if( j == 0 || j == actXCompl )
	    {
		fv[0] = (float) ( -modelXScale * sin(yPiece) );
		fv[1] = 0;
	    }
	    else
	    {
		fv[0] =  (float) ( -cos(j * xPiece) * modelXScale * sin(yPiece) );
		fv[1] = -(float) (  sin(j * xPiece) * modelYScale * sin(yPiece) );
	    }
	    fv[2] = (float) ( cos(yPiece) * modelZScale );
	    
	    glNormal3fv( normalizeV(fv, modelXScale, modelYScale, modelZScale) );
	    glVertex3fv(fv);
	}
	glEnd();
	
	// UNTEN
	glBegin(GL_TRIANGLE_FAN);
	fv[0] = 0;
	fv[1] = 0;
	fv[2] = -modelZScale;
	
	glNormal3fv( normalizeV(fv, modelXScale, modelYScale, modelZScale) );
	glVertex3fv(fv);
	
	for( int j = actXCompl; j >= 0; --j )
	{
	    if( j == 0 || j == actXCompl )
	    {
		fv[0] = (float) ( -modelXScale * sin(yPiece) );
		fv[1] = 0;
	    }
	    else
	    {
		fv[0] =  (float) ( -cos(j * xPiece) * modelXScale * sin(yPiece) );
		fv[1] = -(float) (  sin(j * xPiece) * modelYScale * sin(yPiece) );
	    }
	    fv[2] = (float) ( -cos(yPiece) * modelZScale );
	    
	    glNormal3fv( normalizeV(fv, modelXScale, modelYScale, modelZScale) );
	    glVertex3fv(fv);
	}
	glEnd();
	
	glEndList();
    }
    //cout << "Renderer::createModels1() end" << endl;    
}

//------------------------------------------------------------
//------------ createModels3Wireframe
//------------------------------------------------------------
void Renderer::createModels3Wireframe(int index)
{
    //cout << "Renderer::createModels3Wireframe() beg" << endl;    
    cout << "Sorry, wireframe display for spheroplatelets is not implemented yet. Index:" << index << endl;
    //cout << "Renderer::createModels3Wireframe() end" << endl;    
}

//------------------------------------------------------------
//------------ createModels3
//------------------------------------------------------------
void Renderer::createModels3(int index)
{
    //cout << "Renderer::createModels3() beg" << endl;    
    // spheroplatelet
    // a sphere moved on a circle (like a torus but closed)
    
    float radSphere = objectParams.at(index).at(6);
    float radCircle = objectParams.at(index).at(7);
    
    float plusZ[3] = { 0.0, 0.0, 1.0 };
    float minusZ[3] = { 0.0, 0.0, -1.0 };
    
    modelListSimpleIndex.at(index) = glGenLists(1);
    glNewList(modelListSimpleIndex.at(index), GL_COMPILE);
    glBegin(GL_LINES);
    glVertex3f( 0, 0,  radSphere );
    glVertex3f( 0, 0, -radSphere );
    glEnd();
    glEndList();
    
    modelListIndex.at(index) = glGenLists(modelLevels);
    
    if( objectParams.at(index).at(12) != 0 )
    {
	createModels3Wireframe(index);
	//return;
    }
    
    float xyProjectionUp = 0.0;
    float xyProjectionDown = 0.0;
    
    for( int actLev = 0; actLev < modelLevels; ++actLev )
    {
	int actXCompl = (int) ( modelXComplexity_max + actLev * ((modelXComplexity_min - modelXComplexity_max) / (modelLevels - 1.0)) );
	int actYCompl = (int) ( modelYComplexity_max + actLev * ((modelYComplexity_min - modelYComplexity_max) / (modelLevels - 1.0)) );
	
	glNewList(modelListIndex.at(index) + actLev, GL_COMPILE);
	float yPiece = (float) ( M_PI / (actYCompl * 2) );
	float xPiece = (float) ( 2 * M_PI / actXCompl );
	float fv[3];
	
	glBegin(GL_TRIANGLE_STRIP);
	for( int i = 0; i < actYCompl * 2; ++i )
	{
	    xyProjectionUp   = radSphere * sin(  i      * yPiece );
	    xyProjectionDown = radSphere * sin( (i + 1) * yPiece );
	    
	    for( int j = 0; j < actXCompl + 1; ++j )
	    {
		// Upper part of triangle strip
		if( j == 0 || j == actXCompl )
		{
		    fv[0] = -(float) ( xyProjectionUp );
		    fv[1] = 0;
		}
		else
		{
		    fv[0] = -(float) ( xyProjectionUp * cos(j * xPiece) );
		    fv[1] = -(float) ( xyProjectionUp * sin(j * xPiece) );
		}
		fv[2] = (float) ( cos( i * yPiece) * radSphere );
		
		glNormal3fv( normalizeV(fv, 1.0, 1.0, 1.0) );
		
		if( j == 0 || j == actXCompl )
		{
		    fv[0] -= radCircle;
		}
		else
		{
		    fv[0] -= radCircle * cos(j * xPiece);
		    fv[1] -= radCircle * sin(j * xPiece);
		}
		
		glVertex3fv(fv);
		
		// Lower part of triangle strip
		if( j == 0 || j == actXCompl )
		{
		    fv[0] = -(float) ( xyProjectionDown );
		    fv[1] = 0;
		}
		else
		{
		    fv[0] = -(float) ( xyProjectionDown * cos(j * xPiece) );
		    fv[1] = -(float) ( xyProjectionDown * sin(j * xPiece) );
		}
		fv[2] = (float) ( cos( (i + 1) * yPiece) * radSphere );
		
		glNormal3fv( normalizeV(fv, 1.0, 1.0, 1.0) );
		
		if( j == 0 || j == actXCompl )
		{
		    fv[0] -= radCircle;
		}
		else
		{
		    fv[0] -= radCircle * cos(j * xPiece);
		    fv[1] -= radCircle * sin(j * xPiece);
		}
		
		glVertex3fv(fv);
	    }
	}
	glEnd();
	
	// upper plane
	glBegin(GL_TRIANGLE_FAN);
	fv[0] = 0;
	fv[1] = 0;
	fv[2] = radSphere;
	
	glNormal3fv( normalizeV(fv, 1.0, 1.0, 1.0) );
	glVertex3fv(fv);
	
	for( int j = 0; j < actXCompl + 1; ++j )
	{
	    if( j == 0 || j == actXCompl )
	    {
		fv[0] = -radCircle;
		fv[1] =  0;
	    }
	    else
	    {
		fv[0] =  (float) ( -cos(j * xPiece) * radCircle );
		fv[1] = -(float) (  sin(j * xPiece) * radCircle );
	    }
	    
	    glNormal3fv( plusZ );
	    glVertex3fv(fv);
	}
	glEnd();
	
	// lower plane
	glBegin(GL_TRIANGLE_FAN);
	fv[0] = 0;
	fv[1] = 0;
	fv[2] = -radSphere;
	
	glNormal3fv( normalizeV(fv, 1.0, 1.0, 1.0) );
	glVertex3fv(fv);
	
	for( int j = actXCompl; j >= 0; --j )
	{
	    if( j == 0 || j == actXCompl )
	    {
		fv[0] = -radCircle;
		fv[1] = 0;
	    }
	    else
	    {
		fv[0] =  (float) ( -cos(j * xPiece) * radCircle );
		fv[1] = -(float) (  sin(j * xPiece) * radCircle );
	    }
	    
	    glNormal3fv( minusZ );
	    glVertex3fv(fv);
	}
	glEnd();
	
	glEndList();
    }
    //cout << "Renderer::createModels3() end" << endl;    
}

//------------------------------------------------------------
//------------ createModels4Wireframe
//------------------------------------------------------------
void Renderer::createModels4Wireframe(int index)
{
    //cout << "Renderer::createModels4Wireframe() beg" << endl;    
    cout << "Sorry, wireframe display for cut spheres is not implemented yet. Index:" << index << endl;
    //cout << "Renderer::createModels4Wireframe() end" << endl;    
}

//------------------------------------------------------------
//------------ createModels4
//------------------------------------------------------------
void Renderer::createModels4(int index)
{
    //cout << "Renderer::createModels4() beg" << endl;    
    // cut sphere (sphere cut by two parallel (x,y) planes with equal distance (z) to center)
    
    float radius    = objectParams.at(index).at(8);
    float zCut      = objectParams.at(index).at(9);
    float angle     = acos( zCut/radius );
    float radiusFan = radius * sin(angle);
    
    float plusZ[3] = { 0.0, 0.0, 1.0 };
    float minusZ[3] = { 0.0, 0.0, -1.0 };
    
    modelListSimpleIndex.at(index) = glGenLists(1);
    glNewList(modelListSimpleIndex.at(index), GL_COMPILE);
    glBegin(GL_LINES);
    glVertex3f( 0, 0,  zCut );
    glVertex3f( 0, 0, -zCut );
    glEnd();
    glEndList();
    
    modelListIndex.at(index) = glGenLists(modelLevels);
    
    if( objectParams.at(index).at(12) != 0 )
    {
	createModels4Wireframe(index);
	//return;
    }
    
    for( int actLev = 0; actLev < modelLevels; ++actLev )
    {
	int actXCompl = (int) ( modelXComplexity_max + actLev * ((modelXComplexity_min - modelXComplexity_max) / (modelLevels - 1.0)) );
	int actYCompl = (int) ( modelYComplexity_max + actLev * ((modelYComplexity_min - modelYComplexity_max) / (modelLevels - 1.0)) );
	
	glNewList(modelListIndex.at(index) + actLev, GL_COMPILE);
	float yPiece = (float) ( (M_PI-2*angle) / (actYCompl * 2) );
	float xPiece = (float) ( 2 * M_PI / actXCompl );
	float fv[3];
	
	glBegin(GL_TRIANGLE_STRIP);
	for( int i = 0; i < actYCompl * 2; ++i )
	{
	    for( int j = 0; j < actXCompl + 1; ++j )
	    {
		// Oben
		if( j == 0 || j == actXCompl )
		{
		    fv[0] = (float) ( -radius * sin(angle + i * yPiece) );
		    fv[1] = 0;
		}
		else
		{
		    fv[0] =  (float) ( -cos(j * xPiece) * radius * sin(angle + i * yPiece) );
		    fv[1] = -(float) (  sin(j * xPiece) * radius * sin(angle + i * yPiece) );
		}
		fv[2] = (float) ( cos(angle + i * yPiece) * radius );
		
		glNormal3fv( normalizeV(fv, 1.0, 1.0, 1.0) );
		glVertex3fv(fv);
		
		// Unten
		if( j == 0 || j == actXCompl )
		{
		    fv[0] = (float) ( -radius * sin(angle + (i + 1) * yPiece) );
		    fv[1] = 0;
		}
		else
		{
		    fv[0] =  (float) ( -cos(j * xPiece) * radius * sin(angle + (i + 1) * yPiece) );
		    fv[1] = -(float) (  sin(j * xPiece) * radius * sin(angle + (i + 1) * yPiece) );
		}
		fv[2] = (float) ( cos(angle + (i + 1) * yPiece) * radius );
		
		glNormal3fv( normalizeV(fv, 1.0, 1.0, 1.0) );
		glVertex3fv(fv);
	    }
	}
	glEnd();
	
	// upper cut plane
	glBegin(GL_TRIANGLE_FAN);
	fv[0] = 0;
	fv[1] = 0;
	fv[2] = zCut;
	
	glNormal3fv( normalizeV(fv, 1.0, 1.0, 1.0) );
	glVertex3fv(fv);
	
	for( int j = 0; j < actXCompl + 1; ++j )
	{
	    if( j == 0 || j == actXCompl )
	    {
		fv[0] = -radiusFan;
		fv[1] =  0;
	    }
	    else
	    {
		fv[0] =  (float) ( -cos(j * xPiece) * radiusFan );
		fv[1] = -(float) (  sin(j * xPiece) * radiusFan );
	    }
	    
	    glNormal3fv( plusZ );
	    glVertex3fv(fv);
	}
	glEnd();
	
	// lower cut plane
	glBegin(GL_TRIANGLE_FAN);
	fv[0] = 0;
	fv[1] = 0;
	fv[2] = -zCut;
	
	glNormal3fv( normalizeV(fv, 1.0, 1.0, 1.0) );
	glVertex3fv(fv);
	
	for( int j = actXCompl; j >= 0; --j )
	{
	    if( j == 0 || j == actXCompl )
	    {
		fv[0] = -radiusFan;
		fv[1] = 0;
	    }
	    else
	    {
		fv[0] =  (float) ( -cos(j * xPiece) * radiusFan );
		fv[1] = -(float) (  sin(j * xPiece) * radiusFan );
	    }
	    
	    glNormal3fv( minusZ );
	    glVertex3fv(fv);
	}
	glEnd();
	
	glEndList();
    }
    //cout << "Renderer::createModels4() end" << endl;    
}


void Renderer::createModels5Wireframe(int index)
{
    //cout << "Renderer::createModels1Wireframe() beg" << endl;    
    cout << "Sorry, wireframe display for eyelenses is not implemented yet. Index:" << index << endl;
    //cout << "Renderer::createModels5Wireframe() end" << endl;    
}

void Renderer::createModels5(int index)
{
    //cout << "Renderer::createModels5() beg" << endl;    
    // eyelens (contakt lens like) model
    
    float radius = objectParams.at(index).at(10);
    float angle  = objectParams.at(index).at(11);
    
    //float i_radius = 0.8*radius;
   
    //cout << "rad: " << radius << endl;
    //cout << "ang: " << angle << endl;
    
    modelListSimpleIndex.at(index) = glGenLists(1);
    glNewList(modelListSimpleIndex.at(index), GL_COMPILE);
    glBegin(GL_LINES);
    glVertex3f(0, 0,  0.5 );
    glVertex3f(0, 0, -0.5 );
    glEnd();
    glEndList();
    
    modelListIndex.at(index) = glGenLists(modelLevels);
    
    if( objectParams.at(index).at(12) != 0 )
    {
	createModels5Wireframe(index);
	return;
    }
    
    glMaterialfv( GL_BACK, GL_AMBIENT, (float[]) { 0.0, 0.0, 0.0, 1 } );
    glMaterialfv( GL_BACK, GL_DIFFUSE, (float[]) { 0.8, 0.8, 0.8, 1 } );
    glMaterialfv( GL_BACK, GL_SPECULAR, (float[]) { 0, 0, 0, 1 } );
    glMaterialf( GL_BACK, GL_SHININESS, 128.0 );
    
    for( int actLev = 0; actLev < modelLevels; ++actLev )
    {
	int actXCompl = (int) ( modelXComplexity_max + actLev * ((modelXComplexity_min - modelXComplexity_max) / (modelLevels - 1.0)) );
	int actYCompl = (int) ( modelYComplexity_max + actLev * ((modelYComplexity_min - modelYComplexity_max) / (modelLevels - 1.0)) );
	
	glNewList(modelListIndex.at(index) + actLev, GL_COMPILE);
	
	glDisable(GL_CULL_FACE);
	glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE);
	
	
	float yPiece = (float) (angle / ((actYCompl + 1) * 2));
	float xPiece = (float) (2 * M_PI / actXCompl);
	float fv[3];
	
	glBegin(GL_TRIANGLE_STRIP);
	for( int i = 0; i < actYCompl * 2; ++i )
	{
	    for( int j = 0; j < actXCompl + 1; ++j )
	    {
		// Oben
		if( j == 0 || j == actXCompl )
		{
		    fv[0] = (float) ( -radius * sin((i + 1) * yPiece) );
		    fv[1] = 0;
		}
		else
		{
		    fv[0] =  (float) ( -cos(j * xPiece) * radius * sin((i + 1) * yPiece) );
		    fv[1] = -(float) (  sin(j * xPiece) * radius * sin((i + 1) * yPiece) );
		}
		fv[2] = (float) ( cos((i + 1) * yPiece) * radius );
		
		glNormal3fv( normalizeV(fv, radius, radius, radius) );
		fv[2] -=  radius;
		glVertex3fv(fv);
		
		// Unten
		if( j == 0 || j == actXCompl )
		{
		    fv[0] = (float) ( -radius * sin((i + 2) * yPiece) );
		    fv[1] = 0;
		}
		else
		{
		    fv[0] =  (float) ( -cos(j * xPiece) * radius * sin((i + 2) * yPiece) );
		    fv[1] = -(float) (  sin(j * xPiece) * radius * sin((i + 2) * yPiece) );
		}
		fv[2] = (float) ( cos((i + 2) * yPiece) * radius );
		
		glNormal3fv( normalizeV(fv, radius, radius, radius) );
		fv[2] -=  radius;
		glVertex3fv(fv);
	    }
	}
	glEnd();

	// OBEN
	glBegin(GL_TRIANGLE_FAN);
	fv[0] = 0;
	fv[1] = 0;
	fv[2] = radius;
	
	glNormal3fv( normalizeV(fv, radius, radius, radius) );
	fv[2] -=  radius;
	glVertex3fv(fv);
	
	for( int j = 0; j < actXCompl + 1; ++j )
	{
	    if( j == 0 || j == actXCompl )
	    {
		fv[0] = (float) ( -radius * sin(yPiece) );
		fv[1] = 0;
	    }
	    else
	    {
		fv[0] =  (float) ( -cos(j * xPiece) * radius * sin(yPiece) );
		fv[1] = -(float) (  sin(j * xPiece) * radius * sin(yPiece) );
	    }
	    fv[2] = (float) ( cos(yPiece) * radius );
	    
	    glNormal3fv( normalizeV(fv, radius, radius, radius) );
	    fv[2] -=  radius;
	    glVertex3fv(fv);
	}
	glEnd();
	
	glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_FALSE);
	glEnable(GL_CULL_FACE);
	glEndList();
    }
    //cout << "Renderer::createModels5() end" << endl;    
}

/*!
 *  Create the optimization boxes.
 *
 *  \param count of the boxes (in all 3 dimensions) 
 *  \author Timm Meyer
 */
void Renderer::createGridBoxes(int x) {
    //cout << "Renderer::createGridBoxes() beg" << endl;    
    
    if(!initialized) {
	makeCurrent();
    }
    
    if(middle->size() == 0) {
	//cout << "Renderer::createGridBoxes() end" << endl;    
	return;
    }
    
    smallBoxes.clear();
    smallBoxCounter.clear();
    smallBoxState.clear();
    
    smallBoxes.resize(x, vector< vector <vector <int > > >(x,vector< vector< int > > (x, vector<int>() )));
    smallBoxCounter.resize(x, vector< vector <int > >(x,vector<  int > (x, 0)));
    smallBoxState.resize(x, vector < vector <bool> >(x, vector < bool >(x, false)));
    
    static int xOld = 0;
    for (int i = 0; i < xOld; i++) {
	for (int j = 0; j < xOld; j++) {
	    for (int k = 0; k < xOld; k++) {
		if(smallBoxCoords[i][j][k] != 0)
		    delete smallBoxCoords[i][j][k];
	    }
	}
    }
    xOld = x;
    
    smallBoxCoords.clear();   
    smallBoxCoords.resize(x, vector< vector <float * > >(x,vector<  float * > (x)));
    for (int i = 0; i < x; i++) {
	for (int j = 0; j < x; j++) {
	    for (int k = 0; k < x; k++) {
		//cout << i << " " << j << " " << k << endl;
		smallBoxCoords[i][j][k] = new float[6];
	    }
	}
    }
    
    layerLOD.resize(x,0);
    
    for (int i = 0; i < x; i++) {
	for (int j = 0; j < x; j++) {
	    for (int k = 0; k < x; k++) {
		smallBoxCounter[i][j][k] = 0;
	    }
	}
    }
    
    float subLengthX = (boundingBoxCoords[BOX_COORD_X_HIGH] - boundingBoxCoords[BOX_COORD_X_LOW]) / x;
    float subLengthY = (boundingBoxCoords[BOX_COORD_Y_HIGH] - boundingBoxCoords[BOX_COORD_Y_LOW]) / x;
    float subLengthZ = (boundingBoxCoords[BOX_COORD_Z_HIGH] - boundingBoxCoords[BOX_COORD_Z_LOW]) / x;
    
    // Count
    for (int i = 0; i < (int)middle->size(); i++) {
	int boxX = (int) (((middle->at(i)->at(0) - boundingBoxCoords[BOX_COORD_X_LOW]) / subLengthX));
	int boxY = (int) (((middle->at(i)->at(1) - boundingBoxCoords[BOX_COORD_Y_LOW]) / subLengthY));
	int boxZ = (int) (((middle->at(i)->at(2) - boundingBoxCoords[BOX_COORD_Z_LOW]) / subLengthZ));
	
	if (boxX >= x) {
	    boxX = x-1;
	}
	if (boxY >= x) {
	    boxY = x-1;
	}
	if (boxZ >= x) {
	    boxZ = x-1;
	}
	smallBoxCounter[boxX][boxY][boxZ]++;
    }
    
    // Allocate
    for (int i = 0; i < x; i++) {
	for (int j = 0; j < x; j++) {
	    for (int k = 0; k < x; k++) {
		smallBoxes[i][j][k].resize(smallBoxCounter[i][j][k]);
	    }
	}
    }
    
    // Save
    for (int i = 0; i < (int)middle->size(); i++) {
	int boxX = (int) (((middle->at(i)->at(0) - boundingBoxCoords[BOX_COORD_X_LOW]) / subLengthX));
	int boxY = (int) (((middle->at(i)->at(1) - boundingBoxCoords[BOX_COORD_Y_LOW]) / subLengthY));
	int boxZ = (int) (((middle->at(i)->at(2) - boundingBoxCoords[BOX_COORD_Z_LOW]) / subLengthZ));
	
	if (boxX >= x) {
	    boxX = x-1;
	}
	if (boxY >= x) {
	    boxY = x-1;
	}
	if (boxZ >= x) {
	    boxZ = x-1;
	}
	smallBoxes[boxX][boxY][boxZ][--smallBoxCounter[boxX][boxY][boxZ]] = i;
    }
    
    // Small Boundingboxes berechnen
    float maxModelSize[objectParams.size()];
    for(uint i = 0; i < objectParams.size(); i++) {
	maxModelSize[i] = 0;
	/*
	if( objectParams.at(i).at(4) != 0 && objectParams.at(i).at(5) != 0 )
	{
	    maxModelSize[i] = objectParams.at(i).at(4)+objectParams.at(i).at(5)/2;
	}
	else
	{
	    maxModelSize[i] = max(objectParams.at(i).at(1), max(objectParams.at(i).at(2),objectParams.at(i).at(3)));
	}
	*/
	if     ( objectParams.at(i).at(0) == 0 )
	{
	    maxModelSize[i] = max( objectParams.at(i).at(1), max(objectParams.at(i).at(2),objectParams.at(i).at(3)) );
	}
	else if( objectParams.at(i).at(0) == 1 )
	{
	    maxModelSize[i] = objectParams.at(i).at(4) + objectParams.at(i).at(5)/2.0;
	}
	else if( objectParams.at(i).at(0) == 2 )
	{
	    maxModelSize[i] = objectParams.at(i).at(6) + objectParams.at(i).at(7);
	}
	else if( objectParams.at(i).at(0) == 3 )
	{
	    maxModelSize[i] = objectParams.at(i).at(8);
	}
	
	globalMaxModelSize = max(globalMaxModelSize, maxModelSize[i]);
	//cout << maxModelSize[i] << " " << objectParams.at(i).at(0) << " " << objectParams.at(i).at(1) << " " << objectParams.at(i).at(2) << " " << (objectParams.at(i).at(3)*2+objectParams.at(i).at(4)/2) << endl;
    }
    for (int i = 0; i < x; i++) {
	for (int j = 0; j < x; j++) {
	    for (int k = 0; k < x; k++) {
		int usedModels[objectParams.size()];
		for(uint hg = 0; hg < objectParams.size(); hg++) {
		    usedModels[hg] = 0;
		}
		// Coordinaten berechnen
		if (smallBoxes[i][j][k].size() > 0) {
		    float x_low, x_high, y_low, y_high, z_low, z_high;
		    x_low = x_high = middle->at(smallBoxes[i][j][k][0])->at(0);
		    y_low = y_high = middle->at(smallBoxes[i][j][k][0])->at(1);
		    z_low = z_high = middle->at(smallBoxes[i][j][k][0])->at(2);
		    
		    for (int index = 0; index < (int)smallBoxes[i][j][k].size(); index++) {
			if (middle->at(smallBoxes[i][j][k][index])->at(0) < x_low) {
			    x_low = middle->at(smallBoxes[i][j][k][index])->at(0);
			} else if (middle->at(smallBoxes[i][j][k][index])->at(0) > x_high) {
			    x_high = middle->at(smallBoxes[i][j][k][index])->at(0);
			}
			if (middle->at(smallBoxes[i][j][k][index])->at(1) < y_low) {
			    y_low = middle->at(smallBoxes[i][j][k][index])->at(1);
			} else if (middle->at(smallBoxes[i][j][k][index])->at(1) > y_high) {
			    y_high = middle->at(smallBoxes[i][j][k][index])->at(1);
			}
			if (middle->at(smallBoxes[i][j][k][index])->at(2) < z_low) {
			    z_low = middle->at(smallBoxes[i][j][k][index])->at(2);
			} else if (middle->at(smallBoxes[i][j][k][index])->at(2) > z_high) {
			    z_high = middle->at(smallBoxes[i][j][k][index])->at(2);
			}
			usedModels[modelInd->at(smallBoxes[i][j][k][index])] = 1;
		    }
		    
		    float maxUseSize = 0;
		    for(uint hg = 0; hg < objectParams.size(); hg++) {
			maxUseSize = max(maxUseSize, usedModels[hg]*(maxModelSize[hg]));
		    }
		    
		    smallBoxCoords.at(i).at(j).at(k)[BOX_COORD_X_LOW] = x_low - maxUseSize;
		    smallBoxCoords.at(i).at(j).at(k)[BOX_COORD_X_HIGH] = x_high + maxUseSize;
		    smallBoxCoords.at(i).at(j).at(k)[BOX_COORD_Y_LOW] = y_low - maxUseSize;
		    smallBoxCoords.at(i).at(j).at(k)[BOX_COORD_Y_HIGH] = y_high + maxUseSize;
		    smallBoxCoords.at(i).at(j).at(k)[BOX_COORD_Z_LOW] = z_low - maxUseSize;
		    smallBoxCoords.at(i).at(j).at(k)[BOX_COORD_Z_HIGH] = z_high + maxUseSize;
		    //		    cout << i << " " << j << " " << k << " " << maxModelSize << " " << endl;
		    //		    cout << smallBoxCoords.at(i).at(j).at(k)[BOX_COORD_X_LOW] << endl;
		    //		    cout << smallBoxCoords.at(i).at(j).at(k)[BOX_COORD_X_HIGH] << endl;
		    //		    cout << smallBoxCoords.at(i).at(j).at(k)[BOX_COORD_Y_LOW] << endl;
		    //		    cout << smallBoxCoords.at(i).at(j).at(k)[BOX_COORD_Y_HIGH] << endl;
		    //		    cout << smallBoxCoords.at(i).at(j).at(k)[BOX_COORD_Z_LOW] << endl;
		    //		    cout << smallBoxCoords.at(i).at(j).at(k)[BOX_COORD_Z_HIGH] << endl;
		}
	    }
	}
    }  
    
    //    float d = max ( 
    //	    max(abs(boundingBoxCoords[BOX_COORD_X_LOW]),abs(boundingBoxCoords[BOX_COORD_X_HIGH])) ,
    //	      max(
    //	      max(abs(boundingBoxCoords[BOX_COORD_Y_LOW]),abs(boundingBoxCoords[BOX_COORD_Y_HIGH])),
    //	      max(abs(boundingBoxCoords[BOX_COORD_Z_LOW]),abs(boundingBoxCoords[BOX_COORD_Z_HIGH]))
    //	      )
    //	      ) + globalMaxModelSize;
    //	      ;
    float a = max(abs(boundingBoxCoords[BOX_COORD_X_LOW]),abs(boundingBoxCoords[BOX_COORD_X_HIGH]));
    float b = max(abs(boundingBoxCoords[BOX_COORD_Y_LOW]),abs(boundingBoxCoords[BOX_COORD_Y_HIGH]));
    float c = max(abs(boundingBoxCoords[BOX_COORD_Z_LOW]),abs(boundingBoxCoords[BOX_COORD_Z_HIGH]));
    float d = sqrt(a*a + b*b + c*c) + globalMaxModelSize;
    
    nearClip = -d;
    farClip = 120 + 2*d;
    distAbsolut = true;
    
    processInput(false);
    if(!initialized) {
	glGetError(); /* To remove error because of too fast file loading,
	while screen context not ready	*/
    }
    //cout << "Renderer::createGridBoxes end" << endl;
}

/*
 * 
 * RENDERING 
 * 
 */
inline void Renderer::glTranslateRotateCallList(int objectIndex, int glListIndex) {
    //cout << "Renderer::glTranslateRotateCallList() beg" << endl;    
    glPushMatrix();
    glTranslatef(middle->at(objectIndex)->at(0), middle->at(objectIndex)->at(1), middle->at(objectIndex)->at(2));
    
    if (rot->at(objectIndex)->at(0) != 0 || rot->at(objectIndex)->at(1) != 0 || rot->at(objectIndex)->at(2) != 0) {
	glRotatef(rot->at(objectIndex)->at(3), rot->at(objectIndex)->at(0), rot->at(objectIndex)->at(1), rot->at(objectIndex)->at(2));
    }
    
    //mga::MoleculeBiax::QuaternionFromAxisAngle(rot->at(objectIndex)->at(3), rot->at(objectIndex)->at(0), rot->at(objectIndex)->at(1), rot->at(objectIndex)->at(2));
    
    glColor3fv(((float*)&((*(color->at(objectIndex)))[0])));
    glCallList(glListIndex);
    
    glPopMatrix();
    //cout << "Renderer::glTranslateRotateCallList() end" << endl;    
}

/*!
 *
 *
 *  \param 
 *  \return 
 *  \author Timm Meyer
 */
void Renderer::renderFromSide() {
    //cout << "Renderer::renderFromSide() beg" << endl;    
    
    int layerCounter = 0; // für LOD!
    int delta = 1;
    int start = 0;
    
    int countBlockedBoxes = 0;
    int countNonBlockedBoxes = 0;
    
    if (renderSet_RenderSide == RENDERSIDE_X) {
	//cout << "-------- hallo x" << endl;
	if (xDir[0] > 0) {
	    delta = -1;
	    start = smallBoxes.size() - 1;
	}
	for (int x = start; x < (int)smallBoxes.size() && x >= 0; x += delta) {
	    
	    if (renderSet_UseOcclusionTest && renderSet_occlusionExtensionSupported && layerCounter >= (int)smallBoxes.size() / renderSet_occlusionLayerStartFactor) {
		int querLen = smallBoxes.size() * smallBoxes.size();
		GLuint queries[smallBoxes.size() * smallBoxes.size()];
		
		glGenQueriesARB(smallBoxes.size() * smallBoxes.size(), queries);
		
		glColorMask(false, false, false, false);
		glDepthMask(false);
		
		
		int count = 0;
		for (int y = 0; y < (int)smallBoxes.size(); y++) {
		    for (int z = 0; z < (int)smallBoxes.size(); z++) {
			glBeginQueryARB(GL_SAMPLES_PASSED_ARB, queries[count]);
			// ========================================
			// ========================================
			// ========================================
			if (smallBoxes[x][y][z].size() > 0) {
			    glBegin(GL_TRIANGLES);
			    // left side
			    glVertex3f(smallBoxCoords[x][y][z][BOX_COORD_X_LOW], smallBoxCoords[x][y][z][BOX_COORD_Y_LOW], smallBoxCoords[x][y][z][BOX_COORD_Z_LOW]);
			    glVertex3f(smallBoxCoords[x][y][z][BOX_COORD_X_LOW], smallBoxCoords[x][y][z][BOX_COORD_Y_HIGH], smallBoxCoords[x][y][z][BOX_COORD_Z_HIGH]);
			    glVertex3f(smallBoxCoords[x][y][z][BOX_COORD_X_LOW], smallBoxCoords[x][y][z][BOX_COORD_Y_HIGH], smallBoxCoords[x][y][z][BOX_COORD_Z_LOW]);
			    
			    glVertex3f(smallBoxCoords[x][y][z][BOX_COORD_X_LOW], smallBoxCoords[x][y][z][BOX_COORD_Y_LOW], smallBoxCoords[x][y][z][BOX_COORD_Z_LOW]);
			    glVertex3f(smallBoxCoords[x][y][z][BOX_COORD_X_LOW], smallBoxCoords[x][y][z][BOX_COORD_Y_LOW], smallBoxCoords[x][y][z][BOX_COORD_Z_HIGH]);
			    glVertex3f(smallBoxCoords[x][y][z][BOX_COORD_X_LOW], smallBoxCoords[x][y][z][BOX_COORD_Y_HIGH], smallBoxCoords[x][y][z][BOX_COORD_Z_HIGH]);
			    
			    // right side
			    glVertex3f(smallBoxCoords[x][y][z][BOX_COORD_X_HIGH], smallBoxCoords[x][y][z][BOX_COORD_Y_LOW], smallBoxCoords[x][y][z][BOX_COORD_Z_LOW]);
			    glVertex3f(smallBoxCoords[x][y][z][BOX_COORD_X_HIGH], smallBoxCoords[x][y][z][BOX_COORD_Y_HIGH], smallBoxCoords[x][y][z][BOX_COORD_Z_LOW]);
			    glVertex3f(smallBoxCoords[x][y][z][BOX_COORD_X_HIGH], smallBoxCoords[x][y][z][BOX_COORD_Y_HIGH], smallBoxCoords[x][y][z][BOX_COORD_Z_HIGH]);
			    
			    glVertex3f(smallBoxCoords[x][y][z][BOX_COORD_X_HIGH], smallBoxCoords[x][y][z][BOX_COORD_Y_LOW], smallBoxCoords[x][y][z][BOX_COORD_Z_LOW]);
			    glVertex3f(smallBoxCoords[x][y][z][BOX_COORD_X_HIGH], smallBoxCoords[x][y][z][BOX_COORD_Y_HIGH], smallBoxCoords[x][y][z][BOX_COORD_Z_HIGH]);
			    glVertex3f(smallBoxCoords[x][y][z][BOX_COORD_X_HIGH], smallBoxCoords[x][y][z][BOX_COORD_Y_LOW], smallBoxCoords[x][y][z][BOX_COORD_Z_HIGH]);
			    
			    // front
			    glVertex3f(smallBoxCoords[x][y][z][BOX_COORD_X_LOW], smallBoxCoords[x][y][z][BOX_COORD_Y_LOW], smallBoxCoords[x][y][z][BOX_COORD_Z_HIGH]);
			    glVertex3f(smallBoxCoords[x][y][z][BOX_COORD_X_HIGH], smallBoxCoords[x][y][z][BOX_COORD_Y_HIGH], smallBoxCoords[x][y][z][BOX_COORD_Z_HIGH]);
			    glVertex3f(smallBoxCoords[x][y][z][BOX_COORD_X_LOW], smallBoxCoords[x][y][z][BOX_COORD_Y_HIGH], smallBoxCoords[x][y][z][BOX_COORD_Z_HIGH]);
			    
			    glVertex3f(smallBoxCoords[x][y][z][BOX_COORD_X_LOW], smallBoxCoords[x][y][z][BOX_COORD_Y_LOW], smallBoxCoords[x][y][z][BOX_COORD_Z_HIGH]);
			    glVertex3f(smallBoxCoords[x][y][z][BOX_COORD_X_HIGH], smallBoxCoords[x][y][z][BOX_COORD_Y_LOW], smallBoxCoords[x][y][z][BOX_COORD_Z_HIGH]);
			    glVertex3f(smallBoxCoords[x][y][z][BOX_COORD_X_HIGH], smallBoxCoords[x][y][z][BOX_COORD_Y_HIGH], smallBoxCoords[x][y][z][BOX_COORD_Z_HIGH]);
			    
			    // back
			    glVertex3f(smallBoxCoords[x][y][z][BOX_COORD_X_LOW], smallBoxCoords[x][y][z][BOX_COORD_Y_LOW], smallBoxCoords[x][y][z][BOX_COORD_Z_LOW]);
			    glVertex3f(smallBoxCoords[x][y][z][BOX_COORD_X_LOW], smallBoxCoords[x][y][z][BOX_COORD_Y_HIGH], smallBoxCoords[x][y][z][BOX_COORD_Z_LOW]);
			    glVertex3f(smallBoxCoords[x][y][z][BOX_COORD_X_HIGH], smallBoxCoords[x][y][z][BOX_COORD_Y_HIGH], smallBoxCoords[x][y][z][BOX_COORD_Z_LOW]);
			    
			    glVertex3f(smallBoxCoords[x][y][z][BOX_COORD_X_LOW], smallBoxCoords[x][y][z][BOX_COORD_Y_LOW], smallBoxCoords[x][y][z][BOX_COORD_Z_LOW]);
			    glVertex3f(smallBoxCoords[x][y][z][BOX_COORD_X_HIGH], smallBoxCoords[x][y][z][BOX_COORD_Y_HIGH], smallBoxCoords[x][y][z][BOX_COORD_Z_LOW]);
			    glVertex3f(smallBoxCoords[x][y][z][BOX_COORD_X_HIGH], smallBoxCoords[x][y][z][BOX_COORD_Y_LOW], smallBoxCoords[x][y][z][BOX_COORD_Z_LOW]);
			    
			    // top
			    glVertex3f(smallBoxCoords[x][y][z][BOX_COORD_X_LOW], smallBoxCoords[x][y][z][BOX_COORD_Y_HIGH], smallBoxCoords[x][y][z][BOX_COORD_Z_LOW]);
			    glVertex3f(smallBoxCoords[x][y][z][BOX_COORD_X_LOW], smallBoxCoords[x][y][z][BOX_COORD_Y_HIGH], smallBoxCoords[x][y][z][BOX_COORD_Z_HIGH]);
			    glVertex3f(smallBoxCoords[x][y][z][BOX_COORD_X_HIGH], smallBoxCoords[x][y][z][BOX_COORD_Y_HIGH], smallBoxCoords[x][y][z][BOX_COORD_Z_HIGH]);
			    
			    glVertex3f(smallBoxCoords[x][y][z][BOX_COORD_X_LOW], smallBoxCoords[x][y][z][BOX_COORD_Y_HIGH], smallBoxCoords[x][y][z][BOX_COORD_Z_LOW]);
			    glVertex3f(smallBoxCoords[x][y][z][BOX_COORD_X_HIGH], smallBoxCoords[x][y][z][BOX_COORD_Y_HIGH], smallBoxCoords[x][y][z][BOX_COORD_Z_HIGH]);
			    glVertex3f(smallBoxCoords[x][y][z][BOX_COORD_X_HIGH], smallBoxCoords[x][y][z][BOX_COORD_Y_HIGH], smallBoxCoords[x][y][z][BOX_COORD_Z_LOW]);
			    
			    // down
			    glVertex3f(smallBoxCoords[x][y][z][BOX_COORD_X_LOW], smallBoxCoords[x][y][z][BOX_COORD_Y_LOW], smallBoxCoords[x][y][z][BOX_COORD_Z_LOW]);
			    glVertex3f(smallBoxCoords[x][y][z][BOX_COORD_X_HIGH], smallBoxCoords[x][y][z][BOX_COORD_Y_LOW], smallBoxCoords[x][y][z][BOX_COORD_Z_HIGH]);
			    glVertex3f(smallBoxCoords[x][y][z][BOX_COORD_X_LOW], smallBoxCoords[x][y][z][BOX_COORD_Y_LOW], smallBoxCoords[x][y][z][BOX_COORD_Z_HIGH]);
			    
			    glVertex3f(smallBoxCoords[x][y][z][BOX_COORD_X_LOW], smallBoxCoords[x][y][z][BOX_COORD_Y_LOW], smallBoxCoords[x][y][z][BOX_COORD_Z_LOW]);
			    glVertex3f(smallBoxCoords[x][y][z][BOX_COORD_X_HIGH], smallBoxCoords[x][y][z][BOX_COORD_Y_LOW], smallBoxCoords[x][y][z][BOX_COORD_Z_LOW]);
			    glVertex3f(smallBoxCoords[x][y][z][BOX_COORD_X_HIGH], smallBoxCoords[x][y][z][BOX_COORD_Y_LOW], smallBoxCoords[x][y][z][BOX_COORD_Z_HIGH]);
			    glEnd();
			}
			// ========================================
			// ========================================
			// ========================================
			
			glEndQueryARB(GL_SAMPLES_PASSED_ARB);
			
			count++;
		    }
		}
		
		glFlush();
		
		glColorMask(true, true, true, true);
		glDepthMask(true);
		
		count = 0;
		GLuint samples;
		for (int y = 0; y < (int)smallBoxes.size(); y++) {
		    for (int z = 0; z < (int)smallBoxes.size(); z++) {
			glGetQueryObjectuivARB(queries[count], GL_QUERY_RESULT_ARB, &samples);
			
			if ((int)samples < renderSet_minimumPixelsToDraw) {
			    // DONT DRAW!
			    smallBoxState[x][y][z] = !currentState;
			    countBlockedBoxes++;
			} else {
			    countNonBlockedBoxes++;
			}
			
			count++;
		    }
		}
		
		glDeleteQueriesARB(querLen, queries);
	    }
	    
	    if(drawAsSlice) {
		for (int y = 0; y < (int)smallBoxes.size(); y++) {
		    for (int z = 0; z < (int)smallBoxes.size(); z++) {
			if (smallBoxState[x][y][z] == currentState) {
			    smallBoxState[x][y][z] = !smallBoxState[x][y][z];
			    for (int actI = 0; actI < (int)smallBoxes[x][y][z].size(); actI++) {
				int index = smallBoxes[x][y][z][actI];
				
				if(sliceXLow <= middle->at(index)->at(0) && sliceXHigh >= middle->at(index)->at(0)
				    && sliceYLow <= middle->at(index)->at(1) && sliceYHigh >= middle->at(index)->at(1)
				    && sliceZLow <= middle->at(index)->at(2) && sliceZHigh >= middle->at(index)->at(2)) {
				    glTranslateRotateCallList(index, modelListIndex.at(modelInd->at(index)) + layerLOD[layerCounter]);
				}
				
			    }
			}
		    }
		}
	    } else {
		for (int y = 0; y < (int)smallBoxes.size(); y++) {
		    for (int z = 0; z < (int)smallBoxes.size(); z++) {
			if (smallBoxState[x][y][z] == currentState) {
			    smallBoxState[x][y][z] = !smallBoxState[x][y][z];
			    for (int actI = 0; actI < (int)smallBoxes[x][y][z].size(); actI++) {
				int index = smallBoxes[x][y][z][actI];
				glTranslateRotateCallList(index,modelListIndex.at(modelInd->at(index)) + layerLOD[layerCounter]);
			    }
			}
		    }
		}
	    }
	    layerCounter++;
	}
	
	// ================================================================================
	// ================================================================================
	
    } else if (renderSet_RenderSide == RENDERSIDE_Y) {
	//cout << "-------- hallo y" << endl;
	if (yDir[0] > 0) {
	    delta = -1;
	    start = smallBoxes.size() - 1;
	}
	for (int y = start; y < (int)smallBoxes.size() && y >= 0; y += delta) {
	    
	    if (renderSet_UseOcclusionTest && renderSet_occlusionExtensionSupported && layerCounter >= (int)smallBoxes.size() / renderSet_occlusionLayerStartFactor) {
		GLuint queries[smallBoxes.size() * smallBoxes.size()];
		
		glGenQueriesARB(smallBoxes.size() * smallBoxes.size(), queries);
		int querLen = smallBoxes.size() * smallBoxes.size();
		
		glColorMask(false, false, false, false);
		glDepthMask(false);
		
		int count = 0;
		for (int x = 0; x < (int)smallBoxes.size(); x++) {
		    for (int z = 0; z < (int)smallBoxes.size(); z++) {
			glBeginQueryARB(GL_SAMPLES_PASSED_ARB, queries[count]);
			// ========================================
			// ========================================
			// ========================================
			if (smallBoxes[x][y][z].size() > 0) {
			    glBegin(GL_TRIANGLES);
			    // left side
			    glVertex3f(smallBoxCoords[x][y][z][BOX_COORD_X_LOW], smallBoxCoords[x][y][z][BOX_COORD_Y_LOW], smallBoxCoords[x][y][z][BOX_COORD_Z_LOW]);
			    glVertex3f(smallBoxCoords[x][y][z][BOX_COORD_X_LOW], smallBoxCoords[x][y][z][BOX_COORD_Y_HIGH], smallBoxCoords[x][y][z][BOX_COORD_Z_HIGH]);
			    glVertex3f(smallBoxCoords[x][y][z][BOX_COORD_X_LOW], smallBoxCoords[x][y][z][BOX_COORD_Y_HIGH], smallBoxCoords[x][y][z][BOX_COORD_Z_LOW]);
			    
			    glVertex3f(smallBoxCoords[x][y][z][BOX_COORD_X_LOW], smallBoxCoords[x][y][z][BOX_COORD_Y_LOW], smallBoxCoords[x][y][z][BOX_COORD_Z_LOW]);
			    glVertex3f(smallBoxCoords[x][y][z][BOX_COORD_X_LOW], smallBoxCoords[x][y][z][BOX_COORD_Y_LOW], smallBoxCoords[x][y][z][BOX_COORD_Z_HIGH]);
			    glVertex3f(smallBoxCoords[x][y][z][BOX_COORD_X_LOW], smallBoxCoords[x][y][z][BOX_COORD_Y_HIGH], smallBoxCoords[x][y][z][BOX_COORD_Z_HIGH]);
			    
			    // right side
			    glVertex3f(smallBoxCoords[x][y][z][BOX_COORD_X_HIGH], smallBoxCoords[x][y][z][BOX_COORD_Y_LOW], smallBoxCoords[x][y][z][BOX_COORD_Z_LOW]);
			    glVertex3f(smallBoxCoords[x][y][z][BOX_COORD_X_HIGH], smallBoxCoords[x][y][z][BOX_COORD_Y_HIGH], smallBoxCoords[x][y][z][BOX_COORD_Z_LOW]);
			    glVertex3f(smallBoxCoords[x][y][z][BOX_COORD_X_HIGH], smallBoxCoords[x][y][z][BOX_COORD_Y_HIGH], smallBoxCoords[x][y][z][BOX_COORD_Z_HIGH]);
			    
			    glVertex3f(smallBoxCoords[x][y][z][BOX_COORD_X_HIGH], smallBoxCoords[x][y][z][BOX_COORD_Y_LOW], smallBoxCoords[x][y][z][BOX_COORD_Z_LOW]);
			    glVertex3f(smallBoxCoords[x][y][z][BOX_COORD_X_HIGH], smallBoxCoords[x][y][z][BOX_COORD_Y_HIGH], smallBoxCoords[x][y][z][BOX_COORD_Z_HIGH]);
			    glVertex3f(smallBoxCoords[x][y][z][BOX_COORD_X_HIGH], smallBoxCoords[x][y][z][BOX_COORD_Y_LOW], smallBoxCoords[x][y][z][BOX_COORD_Z_HIGH]);
			    
			    // front
			    glVertex3f(smallBoxCoords[x][y][z][BOX_COORD_X_LOW], smallBoxCoords[x][y][z][BOX_COORD_Y_LOW], smallBoxCoords[x][y][z][BOX_COORD_Z_HIGH]);
			    glVertex3f(smallBoxCoords[x][y][z][BOX_COORD_X_HIGH], smallBoxCoords[x][y][z][BOX_COORD_Y_HIGH], smallBoxCoords[x][y][z][BOX_COORD_Z_HIGH]);
			    glVertex3f(smallBoxCoords[x][y][z][BOX_COORD_X_LOW], smallBoxCoords[x][y][z][BOX_COORD_Y_HIGH], smallBoxCoords[x][y][z][BOX_COORD_Z_HIGH]);
			    
			    glVertex3f(smallBoxCoords[x][y][z][BOX_COORD_X_LOW], smallBoxCoords[x][y][z][BOX_COORD_Y_LOW], smallBoxCoords[x][y][z][BOX_COORD_Z_HIGH]);
			    glVertex3f(smallBoxCoords[x][y][z][BOX_COORD_X_HIGH], smallBoxCoords[x][y][z][BOX_COORD_Y_LOW], smallBoxCoords[x][y][z][BOX_COORD_Z_HIGH]);
			    glVertex3f(smallBoxCoords[x][y][z][BOX_COORD_X_HIGH], smallBoxCoords[x][y][z][BOX_COORD_Y_HIGH], smallBoxCoords[x][y][z][BOX_COORD_Z_HIGH]);
			    
			    // back
			    glVertex3f(smallBoxCoords[x][y][z][BOX_COORD_X_LOW], smallBoxCoords[x][y][z][BOX_COORD_Y_LOW], smallBoxCoords[x][y][z][BOX_COORD_Z_LOW]);
			    glVertex3f(smallBoxCoords[x][y][z][BOX_COORD_X_LOW], smallBoxCoords[x][y][z][BOX_COORD_Y_HIGH], smallBoxCoords[x][y][z][BOX_COORD_Z_LOW]);
			    glVertex3f(smallBoxCoords[x][y][z][BOX_COORD_X_HIGH], smallBoxCoords[x][y][z][BOX_COORD_Y_HIGH], smallBoxCoords[x][y][z][BOX_COORD_Z_LOW]);
			    
			    glVertex3f(smallBoxCoords[x][y][z][BOX_COORD_X_LOW], smallBoxCoords[x][y][z][BOX_COORD_Y_LOW], smallBoxCoords[x][y][z][BOX_COORD_Z_LOW]);
			    glVertex3f(smallBoxCoords[x][y][z][BOX_COORD_X_HIGH], smallBoxCoords[x][y][z][BOX_COORD_Y_HIGH], smallBoxCoords[x][y][z][BOX_COORD_Z_LOW]);
			    glVertex3f(smallBoxCoords[x][y][z][BOX_COORD_X_HIGH], smallBoxCoords[x][y][z][BOX_COORD_Y_LOW], smallBoxCoords[x][y][z][BOX_COORD_Z_LOW]);
			    
			    // top
			    glVertex3f(smallBoxCoords[x][y][z][BOX_COORD_X_LOW], smallBoxCoords[x][y][z][BOX_COORD_Y_HIGH], smallBoxCoords[x][y][z][BOX_COORD_Z_LOW]);
			    glVertex3f(smallBoxCoords[x][y][z][BOX_COORD_X_LOW], smallBoxCoords[x][y][z][BOX_COORD_Y_HIGH], smallBoxCoords[x][y][z][BOX_COORD_Z_HIGH]);
			    glVertex3f(smallBoxCoords[x][y][z][BOX_COORD_X_HIGH], smallBoxCoords[x][y][z][BOX_COORD_Y_HIGH], smallBoxCoords[x][y][z][BOX_COORD_Z_HIGH]);
			    
			    glVertex3f(smallBoxCoords[x][y][z][BOX_COORD_X_LOW], smallBoxCoords[x][y][z][BOX_COORD_Y_HIGH], smallBoxCoords[x][y][z][BOX_COORD_Z_LOW]);
			    glVertex3f(smallBoxCoords[x][y][z][BOX_COORD_X_HIGH], smallBoxCoords[x][y][z][BOX_COORD_Y_HIGH], smallBoxCoords[x][y][z][BOX_COORD_Z_HIGH]);
			    glVertex3f(smallBoxCoords[x][y][z][BOX_COORD_X_HIGH], smallBoxCoords[x][y][z][BOX_COORD_Y_HIGH], smallBoxCoords[x][y][z][BOX_COORD_Z_LOW]);
			    
			    // down
			    glVertex3f(smallBoxCoords[x][y][z][BOX_COORD_X_LOW], smallBoxCoords[x][y][z][BOX_COORD_Y_LOW], smallBoxCoords[x][y][z][BOX_COORD_Z_LOW]);
			    glVertex3f(smallBoxCoords[x][y][z][BOX_COORD_X_HIGH], smallBoxCoords[x][y][z][BOX_COORD_Y_LOW], smallBoxCoords[x][y][z][BOX_COORD_Z_HIGH]);
			    glVertex3f(smallBoxCoords[x][y][z][BOX_COORD_X_LOW], smallBoxCoords[x][y][z][BOX_COORD_Y_LOW], smallBoxCoords[x][y][z][BOX_COORD_Z_HIGH]);
			    
			    glVertex3f(smallBoxCoords[x][y][z][BOX_COORD_X_LOW], smallBoxCoords[x][y][z][BOX_COORD_Y_LOW], smallBoxCoords[x][y][z][BOX_COORD_Z_LOW]);
			    glVertex3f(smallBoxCoords[x][y][z][BOX_COORD_X_HIGH], smallBoxCoords[x][y][z][BOX_COORD_Y_LOW], smallBoxCoords[x][y][z][BOX_COORD_Z_LOW]);
			    glVertex3f(smallBoxCoords[x][y][z][BOX_COORD_X_HIGH], smallBoxCoords[x][y][z][BOX_COORD_Y_LOW], smallBoxCoords[x][y][z][BOX_COORD_Z_HIGH]);
			    glEnd();
			}
			// ========================================
			// ========================================
			// ========================================
			
			glEndQueryARB(GL_SAMPLES_PASSED_ARB);
			
			count++;
		    }
		}
		
		glFlush();
		
		glColorMask(true, true, true, true);
		glDepthMask(true);
		
		count = 0;
		GLuint samples;
		for (int x = 0; x < (int)smallBoxes.size(); x++) {
		    for (int z = 0; z < (int)smallBoxes.size(); z++) {
			glGetQueryObjectuivARB(queries[count], GL_QUERY_RESULT_ARB, &samples);
			
			if ((int)samples < renderSet_minimumPixelsToDraw) {
			    // DONT DRAW!
			    smallBoxState[x][y][z] = !currentState;
			    countBlockedBoxes++;
			} else {
			    countNonBlockedBoxes++;
			}
			
			count++;
		    }
		}
		
		glDeleteQueriesARB(querLen, queries);
	    }
	    
	    if(drawAsSlice) {
		for (int x = 0; x < (int)smallBoxes.size(); ++x) {
		    for (int z = 0; z < (int)smallBoxes.size(); ++z) {
			if (smallBoxState[x][y][z] == currentState) {
			    smallBoxState[x][y][z] = !smallBoxState[x][y][z];
			    for (int actI = 0; actI < (int)smallBoxes[x][y][z].size(); actI++) {
				int index = smallBoxes[x][y][z][actI];
				
				if(sliceXLow <= middle->at(index)->at(0) && sliceXHigh >= middle->at(index)->at(0)
				    && sliceYLow <= middle->at(index)->at(1) && sliceYHigh >= middle->at(index)->at(1)
				    && sliceZLow <= middle->at(index)->at(2) && sliceZHigh >= middle->at(index)->at(2)) {
				    glTranslateRotateCallList(index, modelListIndex.at(modelInd->at(index)) + layerLOD[layerCounter]);
				}
				
			    }
			}
		    }
		}
	    } else {
		for (int x = 0; x < (int)smallBoxes.size(); ++x)
		{
		    for (int z = 0; z < (int)smallBoxes.size(); ++z)
		    {
			if( smallBoxState[x][y][z] == currentState )
			{
			    //cout << " ---------------- hallo" << endl;
			    smallBoxState[x][y][z] = !smallBoxState[x][y][z];
			    for (int actI = 0; actI < (int)smallBoxes[x][y][z].size(); actI++)
			    {
				int index = smallBoxes[x][y][z][actI];
				
				glTranslateRotateCallList(index, modelListIndex.at(modelInd->at(index)) + layerLOD[layerCounter]);
				
			    }
			}
		    }
		}
	    }
	    layerCounter++;
	}
	
	// ================================================================================
	// ================================================================================
    } else { // Z!
	//cout << "-------- hallo z" << endl;
	if (zDir[0] > 0) {
	    delta = -1;
	    start = smallBoxes.size() - 1;
	}
	for (int z = start; z < (int)smallBoxes.size() && z >= 0; z += delta) {
	    if (renderSet_UseOcclusionTest && renderSet_occlusionExtensionSupported && layerCounter >= (int)smallBoxes.size() / renderSet_occlusionLayerStartFactor) {
		GLuint queries[smallBoxes.size() * smallBoxes.size()];
		
		glGenQueriesARB(smallBoxes.size() * smallBoxes.size(), queries);
		int querLen = smallBoxes.size()*smallBoxes.size();
		glColorMask(false, false, false, false);
		glDepthMask(false);
		
		
		int count = 0;
		for (int x = 0; x < (int)smallBoxes.size(); x++) {
		    for (int y = 0; y < (int)smallBoxes.size(); y++) {
			glBeginQueryARB(GL_SAMPLES_PASSED_ARB, queries[count]);
			// ========================================
			// ========================================
			// ========================================
			if (smallBoxes[x][y][z].size() > 0) {
			    glBegin(GL_TRIANGLES);
			    // left side
			    glVertex3f(smallBoxCoords[x][y][z][BOX_COORD_X_LOW], smallBoxCoords[x][y][z][BOX_COORD_Y_LOW], smallBoxCoords[x][y][z][BOX_COORD_Z_LOW]);
			    glVertex3f(smallBoxCoords[x][y][z][BOX_COORD_X_LOW], smallBoxCoords[x][y][z][BOX_COORD_Y_HIGH], smallBoxCoords[x][y][z][BOX_COORD_Z_HIGH]);
			    glVertex3f(smallBoxCoords[x][y][z][BOX_COORD_X_LOW], smallBoxCoords[x][y][z][BOX_COORD_Y_HIGH], smallBoxCoords[x][y][z][BOX_COORD_Z_LOW]);
			    
			    glVertex3f(smallBoxCoords[x][y][z][BOX_COORD_X_LOW], smallBoxCoords[x][y][z][BOX_COORD_Y_LOW], smallBoxCoords[x][y][z][BOX_COORD_Z_LOW]);
			    glVertex3f(smallBoxCoords[x][y][z][BOX_COORD_X_LOW], smallBoxCoords[x][y][z][BOX_COORD_Y_LOW], smallBoxCoords[x][y][z][BOX_COORD_Z_HIGH]);
			    glVertex3f(smallBoxCoords[x][y][z][BOX_COORD_X_LOW], smallBoxCoords[x][y][z][BOX_COORD_Y_HIGH], smallBoxCoords[x][y][z][BOX_COORD_Z_HIGH]);
			    
			    // right side
			    glVertex3f(smallBoxCoords[x][y][z][BOX_COORD_X_HIGH], smallBoxCoords[x][y][z][BOX_COORD_Y_LOW], smallBoxCoords[x][y][z][BOX_COORD_Z_LOW]);
			    glVertex3f(smallBoxCoords[x][y][z][BOX_COORD_X_HIGH], smallBoxCoords[x][y][z][BOX_COORD_Y_HIGH], smallBoxCoords[x][y][z][BOX_COORD_Z_LOW]);
			    glVertex3f(smallBoxCoords[x][y][z][BOX_COORD_X_HIGH], smallBoxCoords[x][y][z][BOX_COORD_Y_HIGH], smallBoxCoords[x][y][z][BOX_COORD_Z_HIGH]);
			    
			    glVertex3f(smallBoxCoords[x][y][z][BOX_COORD_X_HIGH], smallBoxCoords[x][y][z][BOX_COORD_Y_LOW], smallBoxCoords[x][y][z][BOX_COORD_Z_LOW]);
			    glVertex3f(smallBoxCoords[x][y][z][BOX_COORD_X_HIGH], smallBoxCoords[x][y][z][BOX_COORD_Y_HIGH], smallBoxCoords[x][y][z][BOX_COORD_Z_HIGH]);
			    glVertex3f(smallBoxCoords[x][y][z][BOX_COORD_X_HIGH], smallBoxCoords[x][y][z][BOX_COORD_Y_LOW], smallBoxCoords[x][y][z][BOX_COORD_Z_HIGH]);
			    
			    // front
			    glVertex3f(smallBoxCoords[x][y][z][BOX_COORD_X_LOW], smallBoxCoords[x][y][z][BOX_COORD_Y_LOW], smallBoxCoords[x][y][z][BOX_COORD_Z_HIGH]);
			    glVertex3f(smallBoxCoords[x][y][z][BOX_COORD_X_HIGH], smallBoxCoords[x][y][z][BOX_COORD_Y_HIGH], smallBoxCoords[x][y][z][BOX_COORD_Z_HIGH]);
			    glVertex3f(smallBoxCoords[x][y][z][BOX_COORD_X_LOW], smallBoxCoords[x][y][z][BOX_COORD_Y_HIGH], smallBoxCoords[x][y][z][BOX_COORD_Z_HIGH]);
			    
			    glVertex3f(smallBoxCoords[x][y][z][BOX_COORD_X_LOW], smallBoxCoords[x][y][z][BOX_COORD_Y_LOW], smallBoxCoords[x][y][z][BOX_COORD_Z_HIGH]);
			    glVertex3f(smallBoxCoords[x][y][z][BOX_COORD_X_HIGH], smallBoxCoords[x][y][z][BOX_COORD_Y_LOW], smallBoxCoords[x][y][z][BOX_COORD_Z_HIGH]);
			    glVertex3f(smallBoxCoords[x][y][z][BOX_COORD_X_HIGH], smallBoxCoords[x][y][z][BOX_COORD_Y_HIGH], smallBoxCoords[x][y][z][BOX_COORD_Z_HIGH]);
			    
			    // back
			    glVertex3f(smallBoxCoords[x][y][z][BOX_COORD_X_LOW], smallBoxCoords[x][y][z][BOX_COORD_Y_LOW], smallBoxCoords[x][y][z][BOX_COORD_Z_LOW]);
			    glVertex3f(smallBoxCoords[x][y][z][BOX_COORD_X_LOW], smallBoxCoords[x][y][z][BOX_COORD_Y_HIGH], smallBoxCoords[x][y][z][BOX_COORD_Z_LOW]);
			    glVertex3f(smallBoxCoords[x][y][z][BOX_COORD_X_HIGH], smallBoxCoords[x][y][z][BOX_COORD_Y_HIGH], smallBoxCoords[x][y][z][BOX_COORD_Z_LOW]);
			    
			    glVertex3f(smallBoxCoords[x][y][z][BOX_COORD_X_LOW], smallBoxCoords[x][y][z][BOX_COORD_Y_LOW], smallBoxCoords[x][y][z][BOX_COORD_Z_LOW]);
			    glVertex3f(smallBoxCoords[x][y][z][BOX_COORD_X_HIGH], smallBoxCoords[x][y][z][BOX_COORD_Y_HIGH], smallBoxCoords[x][y][z][BOX_COORD_Z_LOW]);
			    glVertex3f(smallBoxCoords[x][y][z][BOX_COORD_X_HIGH], smallBoxCoords[x][y][z][BOX_COORD_Y_LOW], smallBoxCoords[x][y][z][BOX_COORD_Z_LOW]);
			    
			    // top
			    glVertex3f(smallBoxCoords[x][y][z][BOX_COORD_X_LOW], smallBoxCoords[x][y][z][BOX_COORD_Y_HIGH], smallBoxCoords[x][y][z][BOX_COORD_Z_LOW]);
			    glVertex3f(smallBoxCoords[x][y][z][BOX_COORD_X_LOW], smallBoxCoords[x][y][z][BOX_COORD_Y_HIGH], smallBoxCoords[x][y][z][BOX_COORD_Z_HIGH]);
			    glVertex3f(smallBoxCoords[x][y][z][BOX_COORD_X_HIGH], smallBoxCoords[x][y][z][BOX_COORD_Y_HIGH], smallBoxCoords[x][y][z][BOX_COORD_Z_HIGH]);
			    
			    glVertex3f(smallBoxCoords[x][y][z][BOX_COORD_X_LOW], smallBoxCoords[x][y][z][BOX_COORD_Y_HIGH], smallBoxCoords[x][y][z][BOX_COORD_Z_LOW]);
			    glVertex3f(smallBoxCoords[x][y][z][BOX_COORD_X_HIGH], smallBoxCoords[x][y][z][BOX_COORD_Y_HIGH], smallBoxCoords[x][y][z][BOX_COORD_Z_HIGH]);
			    glVertex3f(smallBoxCoords[x][y][z][BOX_COORD_X_HIGH], smallBoxCoords[x][y][z][BOX_COORD_Y_HIGH], smallBoxCoords[x][y][z][BOX_COORD_Z_LOW]);
			    
			    // down
			    glVertex3f(smallBoxCoords[x][y][z][BOX_COORD_X_LOW], smallBoxCoords[x][y][z][BOX_COORD_Y_LOW], smallBoxCoords[x][y][z][BOX_COORD_Z_LOW]);
			    glVertex3f(smallBoxCoords[x][y][z][BOX_COORD_X_HIGH], smallBoxCoords[x][y][z][BOX_COORD_Y_LOW], smallBoxCoords[x][y][z][BOX_COORD_Z_HIGH]);
			    glVertex3f(smallBoxCoords[x][y][z][BOX_COORD_X_LOW], smallBoxCoords[x][y][z][BOX_COORD_Y_LOW], smallBoxCoords[x][y][z][BOX_COORD_Z_HIGH]);
			    
			    glVertex3f(smallBoxCoords[x][y][z][BOX_COORD_X_LOW], smallBoxCoords[x][y][z][BOX_COORD_Y_LOW], smallBoxCoords[x][y][z][BOX_COORD_Z_LOW]);
			    glVertex3f(smallBoxCoords[x][y][z][BOX_COORD_X_HIGH], smallBoxCoords[x][y][z][BOX_COORD_Y_LOW], smallBoxCoords[x][y][z][BOX_COORD_Z_LOW]);
			    glVertex3f(smallBoxCoords[x][y][z][BOX_COORD_X_HIGH], smallBoxCoords[x][y][z][BOX_COORD_Y_LOW], smallBoxCoords[x][y][z][BOX_COORD_Z_HIGH]);
			    glEnd();
			}
			// ========================================
			// ========================================
			// ========================================
			
			glEndQueryARB(GL_SAMPLES_PASSED_ARB);
			
			count++;
		    }
		}
		
		glFlush();
		
		glColorMask(true, true, true, true);
		glDepthMask(true);
		
		count = 0;
		GLuint samples;
		for (int x = 0; x < (int)smallBoxes.size(); x++) {
		    for (int y = 0; y < (int)smallBoxes.size(); y++) {
			glGetQueryObjectuivARB(queries[count], GL_QUERY_RESULT_ARB, &samples);
			
			if ((int)samples < renderSet_minimumPixelsToDraw) {
			    // DONT DRAW!
			    smallBoxState[x][y][z] = !currentState;
			    countBlockedBoxes++;
			} else {
			    countNonBlockedBoxes++;
			}
			
			count++;
		    }
		}
		
		glDeleteQueriesARB(querLen, queries);
	    }
	    if(drawAsSlice) {
		for (int x = 0; x < (int)smallBoxes.size(); x++) {
		    for (int y = 0; y < (int)smallBoxes.size(); y++) {
			if (smallBoxState[x][y][z] == currentState) {
			    smallBoxState[x][y][z] = !smallBoxState[x][y][z];
			    for (int actI = 0; actI < (int)smallBoxes[x][y][z].size(); actI++) {
				int index = smallBoxes[x][y][z][actI];
				
				if(sliceXLow <= middle->at(index)->at(0) && sliceXHigh >= middle->at(index)->at(0)
				    && sliceYLow <= middle->at(index)->at(1) && sliceYHigh >= middle->at(index)->at(1)
				    && sliceZLow <= middle->at(index)->at(2) && sliceZHigh >= middle->at(index)->at(2)) {
				    glTranslateRotateCallList(index,modelListIndex.at(modelInd->at(index)) + layerLOD[layerCounter]);
				}
				
			    }
			}
		    }
		}
	    } else {
		for (int x = 0; x < (int)smallBoxes.size(); x++) {
		    for (int y = 0; y < (int)smallBoxes.size(); y++) {
			if (smallBoxState[x][y][z] == currentState) {
			    smallBoxState[x][y][z] = !smallBoxState[x][y][z];
			    for (int actI = 0; actI < (int)smallBoxes[x][y][z].size(); actI++) {
				int index = smallBoxes[x][y][z][actI];
				glTranslateRotateCallList(index, modelListIndex.at(modelInd->at(index)) + layerLOD[layerCounter]);
				
				
			    }
			}
		    }
		}
	    }
	    layerCounter++;
	}
	
    }
    
    //cout << "Renderer::renderFromSide() end" << endl;    
}

/*
 * 
 * 
 * 
 * 
 * 
 * 
 * 
 * 
 * 
 * 
 * 
 * 
 * 
 */

/*!
 *
 *
 *  \param 
 *  \return 
 *  \author Timm Meyer
 */
void Renderer::renderFromCorner() {
    //cout << "Renderer::renderFromCorner() beg" << endl;    
    int currentBoxX = startBox_X;
    int currentBoxY = startBox_Y;
    int currentBoxZ = startBox_Z;
    int layerCounter = 0;
    int countBlockedBoxes = 0;
    int countNonBlockedBoxes = 0;
    
    while (currentBoxX >= 0 && currentBoxX < (int)smallBoxes.size() && currentBoxY >= 0 && currentBoxY < (int)smallBoxes.size() && currentBoxZ >= 0 && currentBoxZ < (int)smallBoxes.size()) {
	
	if (renderSet_UseOcclusionTest && renderSet_occlusionExtensionSupported && layerCounter >= (int)smallBoxes.size() / renderSet_occlusionLayerStartFactor) {
	    GLuint queries[smallBoxes.size() * smallBoxes.size()];
	    
	    glGenQueriesARB((smallBoxes.size() - layerCounter) * (smallBoxes.size() - layerCounter), queries);
	    int querLen = (smallBoxes.size() - layerCounter) * (smallBoxes.size() - layerCounter);
	    glColorMask(false, false, false, false);
	    glDepthMask(false);
	    
	    
	    int count = 0;
	    int z = currentBoxZ;
	    for (int x = currentBoxX; x >= 0 && x < (int)smallBoxes.size(); x += startBox_DeltaX) {
		for (int y = currentBoxY; y >= 0 && y < (int)smallBoxes.size(); y += startBox_DeltaY) {
		    glBeginQueryARB(GL_SAMPLES_PASSED_ARB, queries[count]);
		    // ========================================
		    // ========================================
		    // ========================================
		    if (smallBoxes[x][y][z].size() > 0) {
			glColor4f(1,0,0,1);
			glBegin(GL_TRIANGLES);
			// left side
			glVertex3f(smallBoxCoords[x][y][z][BOX_COORD_X_LOW], smallBoxCoords[x][y][z][BOX_COORD_Y_LOW], smallBoxCoords[x][y][z][BOX_COORD_Z_LOW]);
			glVertex3f(smallBoxCoords[x][y][z][BOX_COORD_X_LOW], smallBoxCoords[x][y][z][BOX_COORD_Y_HIGH], smallBoxCoords[x][y][z][BOX_COORD_Z_HIGH]);
			glVertex3f(smallBoxCoords[x][y][z][BOX_COORD_X_LOW], smallBoxCoords[x][y][z][BOX_COORD_Y_HIGH], smallBoxCoords[x][y][z][BOX_COORD_Z_LOW]);
			
			glVertex3f(smallBoxCoords[x][y][z][BOX_COORD_X_LOW], smallBoxCoords[x][y][z][BOX_COORD_Y_LOW], smallBoxCoords[x][y][z][BOX_COORD_Z_LOW]);
			glVertex3f(smallBoxCoords[x][y][z][BOX_COORD_X_LOW], smallBoxCoords[x][y][z][BOX_COORD_Y_LOW], smallBoxCoords[x][y][z][BOX_COORD_Z_HIGH]);
			glVertex3f(smallBoxCoords[x][y][z][BOX_COORD_X_LOW], smallBoxCoords[x][y][z][BOX_COORD_Y_HIGH], smallBoxCoords[x][y][z][BOX_COORD_Z_HIGH]);
			
			// right side
			glVertex3f(smallBoxCoords[x][y][z][BOX_COORD_X_HIGH], smallBoxCoords[x][y][z][BOX_COORD_Y_LOW], smallBoxCoords[x][y][z][BOX_COORD_Z_LOW]);
			glVertex3f(smallBoxCoords[x][y][z][BOX_COORD_X_HIGH], smallBoxCoords[x][y][z][BOX_COORD_Y_HIGH], smallBoxCoords[x][y][z][BOX_COORD_Z_LOW]);
			glVertex3f(smallBoxCoords[x][y][z][BOX_COORD_X_HIGH], smallBoxCoords[x][y][z][BOX_COORD_Y_HIGH], smallBoxCoords[x][y][z][BOX_COORD_Z_HIGH]);
			
			glVertex3f(smallBoxCoords[x][y][z][BOX_COORD_X_HIGH], smallBoxCoords[x][y][z][BOX_COORD_Y_LOW], smallBoxCoords[x][y][z][BOX_COORD_Z_LOW]);
			glVertex3f(smallBoxCoords[x][y][z][BOX_COORD_X_HIGH], smallBoxCoords[x][y][z][BOX_COORD_Y_HIGH], smallBoxCoords[x][y][z][BOX_COORD_Z_HIGH]);
			glVertex3f(smallBoxCoords[x][y][z][BOX_COORD_X_HIGH], smallBoxCoords[x][y][z][BOX_COORD_Y_LOW], smallBoxCoords[x][y][z][BOX_COORD_Z_HIGH]);
			
			// front
			glVertex3f(smallBoxCoords[x][y][z][BOX_COORD_X_LOW], smallBoxCoords[x][y][z][BOX_COORD_Y_LOW], smallBoxCoords[x][y][z][BOX_COORD_Z_HIGH]);
			glVertex3f(smallBoxCoords[x][y][z][BOX_COORD_X_HIGH], smallBoxCoords[x][y][z][BOX_COORD_Y_HIGH], smallBoxCoords[x][y][z][BOX_COORD_Z_HIGH]);
			glVertex3f(smallBoxCoords[x][y][z][BOX_COORD_X_LOW], smallBoxCoords[x][y][z][BOX_COORD_Y_HIGH], smallBoxCoords[x][y][z][BOX_COORD_Z_HIGH]);
			
			glVertex3f(smallBoxCoords[x][y][z][BOX_COORD_X_LOW], smallBoxCoords[x][y][z][BOX_COORD_Y_LOW], smallBoxCoords[x][y][z][BOX_COORD_Z_HIGH]);
			glVertex3f(smallBoxCoords[x][y][z][BOX_COORD_X_HIGH], smallBoxCoords[x][y][z][BOX_COORD_Y_LOW], smallBoxCoords[x][y][z][BOX_COORD_Z_HIGH]);
			glVertex3f(smallBoxCoords[x][y][z][BOX_COORD_X_HIGH], smallBoxCoords[x][y][z][BOX_COORD_Y_HIGH], smallBoxCoords[x][y][z][BOX_COORD_Z_HIGH]);
			
			// back
			glVertex3f(smallBoxCoords[x][y][z][BOX_COORD_X_LOW], smallBoxCoords[x][y][z][BOX_COORD_Y_LOW], smallBoxCoords[x][y][z][BOX_COORD_Z_LOW]);
			glVertex3f(smallBoxCoords[x][y][z][BOX_COORD_X_LOW], smallBoxCoords[x][y][z][BOX_COORD_Y_HIGH], smallBoxCoords[x][y][z][BOX_COORD_Z_LOW]);
			glVertex3f(smallBoxCoords[x][y][z][BOX_COORD_X_HIGH], smallBoxCoords[x][y][z][BOX_COORD_Y_HIGH], smallBoxCoords[x][y][z][BOX_COORD_Z_LOW]);
			
			glVertex3f(smallBoxCoords[x][y][z][BOX_COORD_X_LOW], smallBoxCoords[x][y][z][BOX_COORD_Y_LOW], smallBoxCoords[x][y][z][BOX_COORD_Z_LOW]);
			glVertex3f(smallBoxCoords[x][y][z][BOX_COORD_X_HIGH], smallBoxCoords[x][y][z][BOX_COORD_Y_HIGH], smallBoxCoords[x][y][z][BOX_COORD_Z_LOW]);
			glVertex3f(smallBoxCoords[x][y][z][BOX_COORD_X_HIGH], smallBoxCoords[x][y][z][BOX_COORD_Y_LOW], smallBoxCoords[x][y][z][BOX_COORD_Z_LOW]);
			
			// top
			glVertex3f(smallBoxCoords[x][y][z][BOX_COORD_X_LOW], smallBoxCoords[x][y][z][BOX_COORD_Y_HIGH], smallBoxCoords[x][y][z][BOX_COORD_Z_LOW]);
			glVertex3f(smallBoxCoords[x][y][z][BOX_COORD_X_LOW], smallBoxCoords[x][y][z][BOX_COORD_Y_HIGH], smallBoxCoords[x][y][z][BOX_COORD_Z_HIGH]);
			glVertex3f(smallBoxCoords[x][y][z][BOX_COORD_X_HIGH], smallBoxCoords[x][y][z][BOX_COORD_Y_HIGH], smallBoxCoords[x][y][z][BOX_COORD_Z_HIGH]);
			
			glVertex3f(smallBoxCoords[x][y][z][BOX_COORD_X_LOW], smallBoxCoords[x][y][z][BOX_COORD_Y_HIGH], smallBoxCoords[x][y][z][BOX_COORD_Z_LOW]);
			glVertex3f(smallBoxCoords[x][y][z][BOX_COORD_X_HIGH], smallBoxCoords[x][y][z][BOX_COORD_Y_HIGH], smallBoxCoords[x][y][z][BOX_COORD_Z_HIGH]);
			glVertex3f(smallBoxCoords[x][y][z][BOX_COORD_X_HIGH], smallBoxCoords[x][y][z][BOX_COORD_Y_HIGH], smallBoxCoords[x][y][z][BOX_COORD_Z_LOW]);
			
			// down
			glVertex3f(smallBoxCoords[x][y][z][BOX_COORD_X_LOW], smallBoxCoords[x][y][z][BOX_COORD_Y_LOW], smallBoxCoords[x][y][z][BOX_COORD_Z_LOW]);
			glVertex3f(smallBoxCoords[x][y][z][BOX_COORD_X_HIGH], smallBoxCoords[x][y][z][BOX_COORD_Y_LOW], smallBoxCoords[x][y][z][BOX_COORD_Z_HIGH]);
			glVertex3f(smallBoxCoords[x][y][z][BOX_COORD_X_LOW], smallBoxCoords[x][y][z][BOX_COORD_Y_LOW], smallBoxCoords[x][y][z][BOX_COORD_Z_HIGH]);
			
			glVertex3f(smallBoxCoords[x][y][z][BOX_COORD_X_LOW], smallBoxCoords[x][y][z][BOX_COORD_Y_LOW], smallBoxCoords[x][y][z][BOX_COORD_Z_LOW]);
			glVertex3f(smallBoxCoords[x][y][z][BOX_COORD_X_HIGH], smallBoxCoords[x][y][z][BOX_COORD_Y_LOW], smallBoxCoords[x][y][z][BOX_COORD_Z_LOW]);
			glVertex3f(smallBoxCoords[x][y][z][BOX_COORD_X_HIGH], smallBoxCoords[x][y][z][BOX_COORD_Y_LOW], smallBoxCoords[x][y][z][BOX_COORD_Z_HIGH]);
			glEnd();
			
		    }
		    // ========================================
		    // ========================================
		    // ========================================
		    
		    glEndQueryARB(GL_SAMPLES_PASSED_ARB);
		    
		    count++;
		}
	    }
	    
	    glFlush();
	    
	    glColorMask(true, true, true, true);
	    glDepthMask(true);
	    
	    count = 0;
	    GLuint samples;
	    for (int x = currentBoxX; x >= 0 && x < (int)smallBoxes.size(); x += startBox_DeltaX) {
		for (int y = currentBoxY; y >= 0 && y < (int)smallBoxes.size(); y += startBox_DeltaY) {
		    glGetQueryObjectuivARB(queries[count], GL_QUERY_RESULT_ARB, &samples);
		    
		    if ((int)samples < renderSet_minimumPixelsToDraw) {
			// DONT DRAW!
			smallBoxState[x][y][z] = !currentState;
			countBlockedBoxes++;
		    } else {
			countNonBlockedBoxes++;
		    }
		    
		    count++;
		}
	    }
	    
	    glDeleteQueriesARB(querLen, queries);
	}
	
	if(drawAsSlice) {
	    // Draw Code
	    // Oberste Ebene... bis rum Rand
	    for (int x = currentBoxX; x >= 0 && x < (int)smallBoxes.size(); x += startBox_DeltaX) {
		for (int y = currentBoxY; y >= 0 && y < (int)smallBoxes.size(); y += startBox_DeltaY) {
		    if (smallBoxState[x][y][currentBoxZ] == currentState) {
			smallBoxState[x][y][currentBoxZ] = !smallBoxState[x][y][currentBoxZ];
			for (int actI = 0; actI < (int)smallBoxes[x][y][currentBoxZ].size(); actI++) {
			    int index = smallBoxes[x][y][currentBoxZ][actI];
			    if(sliceXLow <= middle->at(index)->at(0) && sliceXHigh >= middle->at(index)->at(0)
				&& sliceYLow <= middle->at(index)->at(1) && sliceYHigh >= middle->at(index)->at(1)
				&& sliceZLow <= middle->at(index)->at(2) && sliceZHigh >= middle->at(index)->at(2)) {
				glTranslateRotateCallList(index, modelListIndex.at(modelInd->at(index)) + layerLOD[layerCounter]);
			    }
			    
			}
		    }
		}
	    }
	} else {
	    // Draw Code
	    // Oberste Ebene... bis rum Rand
	    for (int x = currentBoxX; x >= 0 && x < (int)smallBoxes.size(); x += startBox_DeltaX) {
		for (int y = currentBoxY; y >= 0 && y < (int)smallBoxes.size(); y += startBox_DeltaY) {
		    if (smallBoxState[x][y][currentBoxZ] == currentState) {
			smallBoxState[x][y][currentBoxZ] = !smallBoxState[x][y][currentBoxZ];
			for (int actI = 0; actI < (int)smallBoxes[x][y][currentBoxZ].size(); actI++) {
			    int index = smallBoxes[x][y][currentBoxZ][actI];
			    glTranslateRotateCallList(index, modelListIndex.at(modelInd->at(index)) + layerLOD[layerCounter]);
			}
		    }
		}
	    }
	}
	
	if (renderSet_UseOcclusionTest && renderSet_occlusionExtensionSupported && layerCounter >= (int)smallBoxes.size() / renderSet_occlusionLayerStartFactor) {
	    GLuint queries[smallBoxes.size() * smallBoxes.size()];
	    
	    glGenQueriesARB((smallBoxes.size() - layerCounter) * (smallBoxes.size() - layerCounter), queries);
	    int querLen = (smallBoxes.size() - layerCounter) * (smallBoxes.size() - layerCounter);
	    glColorMask(false, false, false, false);
	    glDepthMask(false);
	    
	    int count = 0;
	    int y = currentBoxY;
	    for (int x = currentBoxX; x >= 0 && x < (int)smallBoxes.size(); x += startBox_DeltaX) {
		for (int z = currentBoxZ + startBox_DeltaZ; z >= 0 && z < (int)smallBoxes.size(); z += startBox_DeltaZ) {
		    glBeginQueryARB(GL_SAMPLES_PASSED_ARB, queries[count]);
		    // ========================================
		    // ========================================
		    // ========================================
		    if (smallBoxes[x][y][z].size() > 0) {
			glBegin(GL_TRIANGLES);
			// left side
			glVertex3f(smallBoxCoords[x][y][z][BOX_COORD_X_LOW], smallBoxCoords[x][y][z][BOX_COORD_Y_LOW], smallBoxCoords[x][y][z][BOX_COORD_Z_LOW]);
			glVertex3f(smallBoxCoords[x][y][z][BOX_COORD_X_LOW], smallBoxCoords[x][y][z][BOX_COORD_Y_HIGH], smallBoxCoords[x][y][z][BOX_COORD_Z_HIGH]);
			glVertex3f(smallBoxCoords[x][y][z][BOX_COORD_X_LOW], smallBoxCoords[x][y][z][BOX_COORD_Y_HIGH], smallBoxCoords[x][y][z][BOX_COORD_Z_LOW]);
			
			glVertex3f(smallBoxCoords[x][y][z][BOX_COORD_X_LOW], smallBoxCoords[x][y][z][BOX_COORD_Y_LOW], smallBoxCoords[x][y][z][BOX_COORD_Z_LOW]);
			glVertex3f(smallBoxCoords[x][y][z][BOX_COORD_X_LOW], smallBoxCoords[x][y][z][BOX_COORD_Y_LOW], smallBoxCoords[x][y][z][BOX_COORD_Z_HIGH]);
			glVertex3f(smallBoxCoords[x][y][z][BOX_COORD_X_LOW], smallBoxCoords[x][y][z][BOX_COORD_Y_HIGH], smallBoxCoords[x][y][z][BOX_COORD_Z_HIGH]);
			
			// right side
			glVertex3f(smallBoxCoords[x][y][z][BOX_COORD_X_HIGH], smallBoxCoords[x][y][z][BOX_COORD_Y_LOW], smallBoxCoords[x][y][z][BOX_COORD_Z_LOW]);
			glVertex3f(smallBoxCoords[x][y][z][BOX_COORD_X_HIGH], smallBoxCoords[x][y][z][BOX_COORD_Y_HIGH], smallBoxCoords[x][y][z][BOX_COORD_Z_LOW]);
			glVertex3f(smallBoxCoords[x][y][z][BOX_COORD_X_HIGH], smallBoxCoords[x][y][z][BOX_COORD_Y_HIGH], smallBoxCoords[x][y][z][BOX_COORD_Z_HIGH]);
			
			glVertex3f(smallBoxCoords[x][y][z][BOX_COORD_X_HIGH], smallBoxCoords[x][y][z][BOX_COORD_Y_LOW], smallBoxCoords[x][y][z][BOX_COORD_Z_LOW]);
			glVertex3f(smallBoxCoords[x][y][z][BOX_COORD_X_HIGH], smallBoxCoords[x][y][z][BOX_COORD_Y_HIGH], smallBoxCoords[x][y][z][BOX_COORD_Z_HIGH]);
			glVertex3f(smallBoxCoords[x][y][z][BOX_COORD_X_HIGH], smallBoxCoords[x][y][z][BOX_COORD_Y_LOW], smallBoxCoords[x][y][z][BOX_COORD_Z_HIGH]);
			
			// front
			glVertex3f(smallBoxCoords[x][y][z][BOX_COORD_X_LOW], smallBoxCoords[x][y][z][BOX_COORD_Y_LOW], smallBoxCoords[x][y][z][BOX_COORD_Z_HIGH]);
			glVertex3f(smallBoxCoords[x][y][z][BOX_COORD_X_HIGH], smallBoxCoords[x][y][z][BOX_COORD_Y_HIGH], smallBoxCoords[x][y][z][BOX_COORD_Z_HIGH]);
			glVertex3f(smallBoxCoords[x][y][z][BOX_COORD_X_LOW], smallBoxCoords[x][y][z][BOX_COORD_Y_HIGH], smallBoxCoords[x][y][z][BOX_COORD_Z_HIGH]);
			
			glVertex3f(smallBoxCoords[x][y][z][BOX_COORD_X_LOW], smallBoxCoords[x][y][z][BOX_COORD_Y_LOW], smallBoxCoords[x][y][z][BOX_COORD_Z_HIGH]);
			glVertex3f(smallBoxCoords[x][y][z][BOX_COORD_X_HIGH], smallBoxCoords[x][y][z][BOX_COORD_Y_LOW], smallBoxCoords[x][y][z][BOX_COORD_Z_HIGH]);
			glVertex3f(smallBoxCoords[x][y][z][BOX_COORD_X_HIGH], smallBoxCoords[x][y][z][BOX_COORD_Y_HIGH], smallBoxCoords[x][y][z][BOX_COORD_Z_HIGH]);
			
			// back
			glVertex3f(smallBoxCoords[x][y][z][BOX_COORD_X_LOW], smallBoxCoords[x][y][z][BOX_COORD_Y_LOW], smallBoxCoords[x][y][z][BOX_COORD_Z_LOW]);
			glVertex3f(smallBoxCoords[x][y][z][BOX_COORD_X_LOW], smallBoxCoords[x][y][z][BOX_COORD_Y_HIGH], smallBoxCoords[x][y][z][BOX_COORD_Z_LOW]);
			glVertex3f(smallBoxCoords[x][y][z][BOX_COORD_X_HIGH], smallBoxCoords[x][y][z][BOX_COORD_Y_HIGH], smallBoxCoords[x][y][z][BOX_COORD_Z_LOW]);
			
			glVertex3f(smallBoxCoords[x][y][z][BOX_COORD_X_LOW], smallBoxCoords[x][y][z][BOX_COORD_Y_LOW], smallBoxCoords[x][y][z][BOX_COORD_Z_LOW]);
			glVertex3f(smallBoxCoords[x][y][z][BOX_COORD_X_HIGH], smallBoxCoords[x][y][z][BOX_COORD_Y_HIGH], smallBoxCoords[x][y][z][BOX_COORD_Z_LOW]);
			glVertex3f(smallBoxCoords[x][y][z][BOX_COORD_X_HIGH], smallBoxCoords[x][y][z][BOX_COORD_Y_LOW], smallBoxCoords[x][y][z][BOX_COORD_Z_LOW]);
			
			// top
			glVertex3f(smallBoxCoords[x][y][z][BOX_COORD_X_LOW], smallBoxCoords[x][y][z][BOX_COORD_Y_HIGH], smallBoxCoords[x][y][z][BOX_COORD_Z_LOW]);
			glVertex3f(smallBoxCoords[x][y][z][BOX_COORD_X_LOW], smallBoxCoords[x][y][z][BOX_COORD_Y_HIGH], smallBoxCoords[x][y][z][BOX_COORD_Z_HIGH]);
			glVertex3f(smallBoxCoords[x][y][z][BOX_COORD_X_HIGH], smallBoxCoords[x][y][z][BOX_COORD_Y_HIGH], smallBoxCoords[x][y][z][BOX_COORD_Z_HIGH]);
			
			glVertex3f(smallBoxCoords[x][y][z][BOX_COORD_X_LOW], smallBoxCoords[x][y][z][BOX_COORD_Y_HIGH], smallBoxCoords[x][y][z][BOX_COORD_Z_LOW]);
			glVertex3f(smallBoxCoords[x][y][z][BOX_COORD_X_HIGH], smallBoxCoords[x][y][z][BOX_COORD_Y_HIGH], smallBoxCoords[x][y][z][BOX_COORD_Z_HIGH]);
			glVertex3f(smallBoxCoords[x][y][z][BOX_COORD_X_HIGH], smallBoxCoords[x][y][z][BOX_COORD_Y_HIGH], smallBoxCoords[x][y][z][BOX_COORD_Z_LOW]);
			
			// down
			glVertex3f(smallBoxCoords[x][y][z][BOX_COORD_X_LOW], smallBoxCoords[x][y][z][BOX_COORD_Y_LOW], smallBoxCoords[x][y][z][BOX_COORD_Z_LOW]);
			glVertex3f(smallBoxCoords[x][y][z][BOX_COORD_X_HIGH], smallBoxCoords[x][y][z][BOX_COORD_Y_LOW], smallBoxCoords[x][y][z][BOX_COORD_Z_HIGH]);
			glVertex3f(smallBoxCoords[x][y][z][BOX_COORD_X_LOW], smallBoxCoords[x][y][z][BOX_COORD_Y_LOW], smallBoxCoords[x][y][z][BOX_COORD_Z_HIGH]);
			
			glVertex3f(smallBoxCoords[x][y][z][BOX_COORD_X_LOW], smallBoxCoords[x][y][z][BOX_COORD_Y_LOW], smallBoxCoords[x][y][z][BOX_COORD_Z_LOW]);
			glVertex3f(smallBoxCoords[x][y][z][BOX_COORD_X_HIGH], smallBoxCoords[x][y][z][BOX_COORD_Y_LOW], smallBoxCoords[x][y][z][BOX_COORD_Z_LOW]);
			glVertex3f(smallBoxCoords[x][y][z][BOX_COORD_X_HIGH], smallBoxCoords[x][y][z][BOX_COORD_Y_LOW], smallBoxCoords[x][y][z][BOX_COORD_Z_HIGH]);
			glEnd();
		    }
		    // ========================================
		    // ========================================
		    // ========================================
		    
		    glEndQueryARB(GL_SAMPLES_PASSED_ARB);
		    
		    count++;
		}
	    }
	    
	    glFlush();
	    
	    glColorMask(true, true, true, true);
	    glDepthMask(true);
	    
	    count = 0;
	    GLuint samples;
	    for (int x = currentBoxX; x >= 0 && x < (int)smallBoxes.size(); x += startBox_DeltaX) {
		for (int z = currentBoxZ + startBox_DeltaZ; z >= 0 && z < (int)smallBoxes.size(); z += startBox_DeltaZ) {
		    glGetQueryObjectuivARB(queries[count], GL_QUERY_RESULT_ARB, &samples);
		    
		    if ((int)samples < renderSet_minimumPixelsToDraw) {
			// DONT DRAW!
			smallBoxState[x][y][z] = !currentState;
			countBlockedBoxes++;
		    } else {
			countNonBlockedBoxes++;
		    }
		    
		    count++;
		}
	    }
	    
	    glDeleteQueriesARB(querLen, queries);
	}
	
	if(drawAsSlice) {
	    // Ein Streifen weniger
	    for (int x = currentBoxX; x >= 0 && x < (int)smallBoxes.size(); x += startBox_DeltaX) {
		for (int z = currentBoxZ + startBox_DeltaZ; z >= 0 && z < (int)smallBoxes.size(); z += startBox_DeltaZ) {
		    if (smallBoxState[x][currentBoxY][z] == currentState) {
			smallBoxState[x][currentBoxY][z] = !smallBoxState[x][currentBoxY][z];
			for (int actI = 0; actI < (int)smallBoxes[x][currentBoxY][z].size(); actI++) {
			    int index = smallBoxes[x][currentBoxY][z][actI];
			    if(sliceXLow <= middle->at(index)->at(0) && sliceXHigh >= middle->at(index)->at(0)
				&& sliceYLow <= middle->at(index)->at(1) && sliceYHigh >= middle->at(index)->at(1)
				&& sliceZLow <= middle->at(index)->at(2) && sliceZHigh >= middle->at(index)->at(2)) {
				
				glTranslateRotateCallList(index, modelListIndex.at(modelInd->at(index)) + layerLOD[layerCounter]);
			    }
			}
		    }
		}
	    }
	} else {
	    // Ein Streifen weniger
	    for (int x = currentBoxX; x >= 0 && x < (int)smallBoxes.size(); x += startBox_DeltaX) {
		for (int z = currentBoxZ + startBox_DeltaZ; z >= 0 && z < (int)smallBoxes.size(); z += startBox_DeltaZ) {
		    if (smallBoxState[x][currentBoxY][z] == currentState) {
			smallBoxState[x][currentBoxY][z] = !smallBoxState[x][currentBoxY][z];
			for (int actI = 0; actI < (int)smallBoxes[x][currentBoxY][z].size(); actI++) {
			    int index = smallBoxes[x][currentBoxY][z][actI];
			    
			    glTranslateRotateCallList(index, modelListIndex.at(modelInd->at(index)) + layerLOD[layerCounter]);
			}
		    }
		}
	    }
	}
	
	if (renderSet_UseOcclusionTest && renderSet_occlusionExtensionSupported && layerCounter >= (int)smallBoxes.size() / renderSet_occlusionLayerStartFactor) {
	    GLuint queries[smallBoxes.size() * smallBoxes.size()];
	    
	    glGenQueriesARB((smallBoxes.size() - layerCounter) * (smallBoxes.size() - layerCounter), queries);
	    int querLen = (smallBoxes.size() - layerCounter) * (smallBoxes.size() - layerCounter);
	    
	    glColorMask(false, false, false, false);
	    glDepthMask(false);
	    
	    int count = 0;
	    int x = currentBoxX;
	    for (int y = currentBoxY + startBox_DeltaY; y >= 0 && y < (int)smallBoxes.size(); y += startBox_DeltaY) {
		for (int z = currentBoxZ + startBox_DeltaZ; z >= 0 && z < (int)smallBoxes.size(); z += startBox_DeltaZ) {
		    glBeginQueryARB(GL_SAMPLES_PASSED_ARB, queries[count]);
		    // ========================================
		    // ========================================
		    // ========================================
		    if (smallBoxes[x][y][z].size() > 0) {
			glBegin(GL_TRIANGLES);
			// left side
			glVertex3f(smallBoxCoords[x][y][z][BOX_COORD_X_LOW], smallBoxCoords[x][y][z][BOX_COORD_Y_LOW], smallBoxCoords[x][y][z][BOX_COORD_Z_LOW]);
			glVertex3f(smallBoxCoords[x][y][z][BOX_COORD_X_LOW], smallBoxCoords[x][y][z][BOX_COORD_Y_HIGH], smallBoxCoords[x][y][z][BOX_COORD_Z_HIGH]);
			glVertex3f(smallBoxCoords[x][y][z][BOX_COORD_X_LOW], smallBoxCoords[x][y][z][BOX_COORD_Y_HIGH], smallBoxCoords[x][y][z][BOX_COORD_Z_LOW]);
			
			glVertex3f(smallBoxCoords[x][y][z][BOX_COORD_X_LOW], smallBoxCoords[x][y][z][BOX_COORD_Y_LOW], smallBoxCoords[x][y][z][BOX_COORD_Z_LOW]);
			glVertex3f(smallBoxCoords[x][y][z][BOX_COORD_X_LOW], smallBoxCoords[x][y][z][BOX_COORD_Y_LOW], smallBoxCoords[x][y][z][BOX_COORD_Z_HIGH]);
			glVertex3f(smallBoxCoords[x][y][z][BOX_COORD_X_LOW], smallBoxCoords[x][y][z][BOX_COORD_Y_HIGH], smallBoxCoords[x][y][z][BOX_COORD_Z_HIGH]);
			
			// right side
			glVertex3f(smallBoxCoords[x][y][z][BOX_COORD_X_HIGH], smallBoxCoords[x][y][z][BOX_COORD_Y_LOW], smallBoxCoords[x][y][z][BOX_COORD_Z_LOW]);
			glVertex3f(smallBoxCoords[x][y][z][BOX_COORD_X_HIGH], smallBoxCoords[x][y][z][BOX_COORD_Y_HIGH], smallBoxCoords[x][y][z][BOX_COORD_Z_LOW]);
			glVertex3f(smallBoxCoords[x][y][z][BOX_COORD_X_HIGH], smallBoxCoords[x][y][z][BOX_COORD_Y_HIGH], smallBoxCoords[x][y][z][BOX_COORD_Z_HIGH]);
			
			glVertex3f(smallBoxCoords[x][y][z][BOX_COORD_X_HIGH], smallBoxCoords[x][y][z][BOX_COORD_Y_LOW], smallBoxCoords[x][y][z][BOX_COORD_Z_LOW]);
			glVertex3f(smallBoxCoords[x][y][z][BOX_COORD_X_HIGH], smallBoxCoords[x][y][z][BOX_COORD_Y_HIGH], smallBoxCoords[x][y][z][BOX_COORD_Z_HIGH]);
			glVertex3f(smallBoxCoords[x][y][z][BOX_COORD_X_HIGH], smallBoxCoords[x][y][z][BOX_COORD_Y_LOW], smallBoxCoords[x][y][z][BOX_COORD_Z_HIGH]);
			
			// front
			glVertex3f(smallBoxCoords[x][y][z][BOX_COORD_X_LOW], smallBoxCoords[x][y][z][BOX_COORD_Y_LOW], smallBoxCoords[x][y][z][BOX_COORD_Z_HIGH]);
			glVertex3f(smallBoxCoords[x][y][z][BOX_COORD_X_HIGH], smallBoxCoords[x][y][z][BOX_COORD_Y_HIGH], smallBoxCoords[x][y][z][BOX_COORD_Z_HIGH]);
			glVertex3f(smallBoxCoords[x][y][z][BOX_COORD_X_LOW], smallBoxCoords[x][y][z][BOX_COORD_Y_HIGH], smallBoxCoords[x][y][z][BOX_COORD_Z_HIGH]);
			
			glVertex3f(smallBoxCoords[x][y][z][BOX_COORD_X_LOW], smallBoxCoords[x][y][z][BOX_COORD_Y_LOW], smallBoxCoords[x][y][z][BOX_COORD_Z_HIGH]);
			glVertex3f(smallBoxCoords[x][y][z][BOX_COORD_X_HIGH], smallBoxCoords[x][y][z][BOX_COORD_Y_LOW], smallBoxCoords[x][y][z][BOX_COORD_Z_HIGH]);
			glVertex3f(smallBoxCoords[x][y][z][BOX_COORD_X_HIGH], smallBoxCoords[x][y][z][BOX_COORD_Y_HIGH], smallBoxCoords[x][y][z][BOX_COORD_Z_HIGH]);
			
			// back
			glVertex3f(smallBoxCoords[x][y][z][BOX_COORD_X_LOW], smallBoxCoords[x][y][z][BOX_COORD_Y_LOW], smallBoxCoords[x][y][z][BOX_COORD_Z_LOW]);
			glVertex3f(smallBoxCoords[x][y][z][BOX_COORD_X_LOW], smallBoxCoords[x][y][z][BOX_COORD_Y_HIGH], smallBoxCoords[x][y][z][BOX_COORD_Z_LOW]);
			glVertex3f(smallBoxCoords[x][y][z][BOX_COORD_X_HIGH], smallBoxCoords[x][y][z][BOX_COORD_Y_HIGH], smallBoxCoords[x][y][z][BOX_COORD_Z_LOW]);
			
			glVertex3f(smallBoxCoords[x][y][z][BOX_COORD_X_LOW], smallBoxCoords[x][y][z][BOX_COORD_Y_LOW], smallBoxCoords[x][y][z][BOX_COORD_Z_LOW]);
			glVertex3f(smallBoxCoords[x][y][z][BOX_COORD_X_HIGH], smallBoxCoords[x][y][z][BOX_COORD_Y_HIGH], smallBoxCoords[x][y][z][BOX_COORD_Z_LOW]);
			glVertex3f(smallBoxCoords[x][y][z][BOX_COORD_X_HIGH], smallBoxCoords[x][y][z][BOX_COORD_Y_LOW], smallBoxCoords[x][y][z][BOX_COORD_Z_LOW]);
			
			// top
			glVertex3f(smallBoxCoords[x][y][z][BOX_COORD_X_LOW], smallBoxCoords[x][y][z][BOX_COORD_Y_HIGH], smallBoxCoords[x][y][z][BOX_COORD_Z_LOW]);
			glVertex3f(smallBoxCoords[x][y][z][BOX_COORD_X_LOW], smallBoxCoords[x][y][z][BOX_COORD_Y_HIGH], smallBoxCoords[x][y][z][BOX_COORD_Z_HIGH]);
			glVertex3f(smallBoxCoords[x][y][z][BOX_COORD_X_HIGH], smallBoxCoords[x][y][z][BOX_COORD_Y_HIGH], smallBoxCoords[x][y][z][BOX_COORD_Z_HIGH]);
			
			glVertex3f(smallBoxCoords[x][y][z][BOX_COORD_X_LOW], smallBoxCoords[x][y][z][BOX_COORD_Y_HIGH], smallBoxCoords[x][y][z][BOX_COORD_Z_LOW]);
			glVertex3f(smallBoxCoords[x][y][z][BOX_COORD_X_HIGH], smallBoxCoords[x][y][z][BOX_COORD_Y_HIGH], smallBoxCoords[x][y][z][BOX_COORD_Z_HIGH]);
			glVertex3f(smallBoxCoords[x][y][z][BOX_COORD_X_HIGH], smallBoxCoords[x][y][z][BOX_COORD_Y_HIGH], smallBoxCoords[x][y][z][BOX_COORD_Z_LOW]);
			
			// down
			glVertex3f(smallBoxCoords[x][y][z][BOX_COORD_X_LOW], smallBoxCoords[x][y][z][BOX_COORD_Y_LOW], smallBoxCoords[x][y][z][BOX_COORD_Z_LOW]);
			glVertex3f(smallBoxCoords[x][y][z][BOX_COORD_X_HIGH], smallBoxCoords[x][y][z][BOX_COORD_Y_LOW], smallBoxCoords[x][y][z][BOX_COORD_Z_HIGH]);
			glVertex3f(smallBoxCoords[x][y][z][BOX_COORD_X_LOW], smallBoxCoords[x][y][z][BOX_COORD_Y_LOW], smallBoxCoords[x][y][z][BOX_COORD_Z_HIGH]);
			
			glVertex3f(smallBoxCoords[x][y][z][BOX_COORD_X_LOW], smallBoxCoords[x][y][z][BOX_COORD_Y_LOW], smallBoxCoords[x][y][z][BOX_COORD_Z_LOW]);
			glVertex3f(smallBoxCoords[x][y][z][BOX_COORD_X_HIGH], smallBoxCoords[x][y][z][BOX_COORD_Y_LOW], smallBoxCoords[x][y][z][BOX_COORD_Z_LOW]);
			glVertex3f(smallBoxCoords[x][y][z][BOX_COORD_X_HIGH], smallBoxCoords[x][y][z][BOX_COORD_Y_LOW], smallBoxCoords[x][y][z][BOX_COORD_Z_HIGH]);
			glEnd();
		    }
		    // ========================================
		    // ========================================
		    // ========================================
		    
		    glEndQueryARB(GL_SAMPLES_PASSED_ARB);
		    
		    count++;
		}
	    }
	    
	    glFlush();
	    
	    glColorMask(true, true, true, true);
	    glDepthMask(true);
	    
	    count = 0;
	    GLuint samples;
	    for (int y = currentBoxY + startBox_DeltaY; y >= 0 && y < (int)smallBoxes.size(); y += startBox_DeltaY) {
		for (int z = currentBoxZ + startBox_DeltaZ; z >= 0 && z < (int)smallBoxes.size(); z += startBox_DeltaZ) {
		    glGetQueryObjectuivARB(queries[count], GL_QUERY_RESULT_ARB, &samples);
		    
		    if ((int)samples < renderSet_minimumPixelsToDraw) {
			// DONT DRAW!
			smallBoxState[x][y][z] = !currentState;
			countBlockedBoxes++;
		    } else {
			countNonBlockedBoxes++;
		    }
		    
		    count++;
		}
	    }
	    
	    glDeleteQueriesARB(querLen, queries);
	}
	
	if(drawAsSlice) {
	    // Zwei Streifen weniger
	    for (int y = currentBoxY + startBox_DeltaY; y >= 0 && y < (int)smallBoxes.size(); y += startBox_DeltaY) {
		for (int z = currentBoxZ + startBox_DeltaZ; z >= 0 && z < (int)smallBoxes.size(); z += startBox_DeltaZ) {
		    if (smallBoxState[currentBoxX][y][z] == currentState) {
			smallBoxState[currentBoxX][y][z] = !smallBoxState[currentBoxX][y][z];
			for (int actI = 0; actI < (int)smallBoxes[currentBoxX][y][z].size(); actI++) {
			    int index = smallBoxes[currentBoxX][y][z][actI];
			    if(sliceXLow <= middle->at(index)->at(0) && sliceXHigh >= middle->at(index)->at(0)
				&& sliceYLow <= middle->at(index)->at(1) && sliceYHigh >= middle->at(index)->at(1)
				&& sliceZLow <= middle->at(index)->at(2) && sliceZHigh >= middle->at(index)->at(2)) {
				
				glTranslateRotateCallList(index, modelListIndex.at(modelInd->at(index)) + layerLOD[layerCounter]);
			    }
			}
		    }
		}
	    }
	} else {
	    // Zwei Streifen weniger
	    for (int y = currentBoxY + startBox_DeltaY; y >= 0 && y < (int)smallBoxes.size(); y += startBox_DeltaY) {
		for (int z = currentBoxZ + startBox_DeltaZ; z >= 0 && z < (int)smallBoxes.size(); z += startBox_DeltaZ) {
		    if (smallBoxState[currentBoxX][y][z] == currentState) {
			smallBoxState[currentBoxX][y][z] = !smallBoxState[currentBoxX][y][z];
			for (int actI = 0; actI < (int)smallBoxes[currentBoxX][y][z].size(); actI++) {
			    int index = smallBoxes[currentBoxX][y][z][actI];
			    
			    glTranslateRotateCallList(index, modelListIndex.at(modelInd->at(index)) + layerLOD[layerCounter]);
			}
		    }
		}
	    }
	}
	
	currentBoxX += startBox_DeltaX;
	currentBoxY += startBox_DeltaY;
	currentBoxZ += startBox_DeltaZ;
	layerCounter++;
    }
    //cout << "Renderer::renderFromCorner() end" << endl;    
}

/*
 * 
 * 
 * 
 * 
 * 
 * 
 * 
 * 
 * 
 * 
 * 
 * 
 * 
 */

/*!
 *	Used for debugging purposes, to show the created sub-boxes.
 *
 *  \author Timm Meyer
 */
void Renderer::renderSubBoundingBoxes() {
    //cout << "Renderer::renderSubBoundingBoxes() beg" << endl;    
    glBegin(GL_TRIANGLES);
    for (int x = 0; x < (int)smallBoxes.size(); x++) {
	for (int y = 0; y < (int)smallBoxes.size(); y++) {
	    for (int z = 0; z < (int)smallBoxes.size(); z++) {
		if (smallBoxes[x][y][z].size() > 0) {
		    // left side
		    glVertex3f(smallBoxCoords[x][y][z][BOX_COORD_X_LOW], smallBoxCoords[x][y][z][BOX_COORD_Y_LOW], smallBoxCoords[x][y][z][BOX_COORD_Z_LOW]);
		    glVertex3f(smallBoxCoords[x][y][z][BOX_COORD_X_LOW], smallBoxCoords[x][y][z][BOX_COORD_Y_HIGH], smallBoxCoords[x][y][z][BOX_COORD_Z_HIGH]);
		    glVertex3f(smallBoxCoords[x][y][z][BOX_COORD_X_LOW], smallBoxCoords[x][y][z][BOX_COORD_Y_HIGH], smallBoxCoords[x][y][z][BOX_COORD_Z_LOW]);
		    
		    glVertex3f(smallBoxCoords[x][y][z][BOX_COORD_X_LOW], smallBoxCoords[x][y][z][BOX_COORD_Y_LOW], smallBoxCoords[x][y][z][BOX_COORD_Z_LOW]);
		    glVertex3f(smallBoxCoords[x][y][z][BOX_COORD_X_LOW], smallBoxCoords[x][y][z][BOX_COORD_Y_LOW], smallBoxCoords[x][y][z][BOX_COORD_Z_HIGH]);
		    glVertex3f(smallBoxCoords[x][y][z][BOX_COORD_X_LOW], smallBoxCoords[x][y][z][BOX_COORD_Y_HIGH], smallBoxCoords[x][y][z][BOX_COORD_Z_HIGH]);
		    
		    // right side
		    glVertex3f(smallBoxCoords[x][y][z][BOX_COORD_X_HIGH], smallBoxCoords[x][y][z][BOX_COORD_Y_LOW], smallBoxCoords[x][y][z][BOX_COORD_Z_LOW]);
		    glVertex3f(smallBoxCoords[x][y][z][BOX_COORD_X_HIGH], smallBoxCoords[x][y][z][BOX_COORD_Y_HIGH], smallBoxCoords[x][y][z][BOX_COORD_Z_LOW]);
		    glVertex3f(smallBoxCoords[x][y][z][BOX_COORD_X_HIGH], smallBoxCoords[x][y][z][BOX_COORD_Y_HIGH], smallBoxCoords[x][y][z][BOX_COORD_Z_HIGH]);
		    
		    glVertex3f(smallBoxCoords[x][y][z][BOX_COORD_X_HIGH], smallBoxCoords[x][y][z][BOX_COORD_Y_LOW], smallBoxCoords[x][y][z][BOX_COORD_Z_LOW]);
		    glVertex3f(smallBoxCoords[x][y][z][BOX_COORD_X_HIGH], smallBoxCoords[x][y][z][BOX_COORD_Y_HIGH], smallBoxCoords[x][y][z][BOX_COORD_Z_HIGH]);
		    glVertex3f(smallBoxCoords[x][y][z][BOX_COORD_X_HIGH], smallBoxCoords[x][y][z][BOX_COORD_Y_LOW], smallBoxCoords[x][y][z][BOX_COORD_Z_HIGH]);
		    
		    // front
		    glVertex3f(smallBoxCoords[x][y][z][BOX_COORD_X_LOW], smallBoxCoords[x][y][z][BOX_COORD_Y_LOW], smallBoxCoords[x][y][z][BOX_COORD_Z_HIGH]);
		    glVertex3f(smallBoxCoords[x][y][z][BOX_COORD_X_HIGH], smallBoxCoords[x][y][z][BOX_COORD_Y_HIGH], smallBoxCoords[x][y][z][BOX_COORD_Z_HIGH]);
		    glVertex3f(smallBoxCoords[x][y][z][BOX_COORD_X_LOW], smallBoxCoords[x][y][z][BOX_COORD_Y_HIGH], smallBoxCoords[x][y][z][BOX_COORD_Z_HIGH]);
		    
		    glVertex3f(smallBoxCoords[x][y][z][BOX_COORD_X_LOW], smallBoxCoords[x][y][z][BOX_COORD_Y_LOW], smallBoxCoords[x][y][z][BOX_COORD_Z_HIGH]);
		    glVertex3f(smallBoxCoords[x][y][z][BOX_COORD_X_HIGH], smallBoxCoords[x][y][z][BOX_COORD_Y_LOW], smallBoxCoords[x][y][z][BOX_COORD_Z_HIGH]);
		    glVertex3f(smallBoxCoords[x][y][z][BOX_COORD_X_HIGH], smallBoxCoords[x][y][z][BOX_COORD_Y_HIGH], smallBoxCoords[x][y][z][BOX_COORD_Z_HIGH]);
		    
		    // back
		    glVertex3f(smallBoxCoords[x][y][z][BOX_COORD_X_LOW], smallBoxCoords[x][y][z][BOX_COORD_Y_LOW], smallBoxCoords[x][y][z][BOX_COORD_Z_LOW]);
		    glVertex3f(smallBoxCoords[x][y][z][BOX_COORD_X_LOW], smallBoxCoords[x][y][z][BOX_COORD_Y_HIGH], smallBoxCoords[x][y][z][BOX_COORD_Z_LOW]);
		    glVertex3f(smallBoxCoords[x][y][z][BOX_COORD_X_HIGH], smallBoxCoords[x][y][z][BOX_COORD_Y_HIGH], smallBoxCoords[x][y][z][BOX_COORD_Z_LOW]);
		    
		    glVertex3f(smallBoxCoords[x][y][z][BOX_COORD_X_LOW], smallBoxCoords[x][y][z][BOX_COORD_Y_LOW], smallBoxCoords[x][y][z][BOX_COORD_Z_LOW]);
		    glVertex3f(smallBoxCoords[x][y][z][BOX_COORD_X_HIGH], smallBoxCoords[x][y][z][BOX_COORD_Y_HIGH], smallBoxCoords[x][y][z][BOX_COORD_Z_LOW]);
		    glVertex3f(smallBoxCoords[x][y][z][BOX_COORD_X_HIGH], smallBoxCoords[x][y][z][BOX_COORD_Y_LOW], smallBoxCoords[x][y][z][BOX_COORD_Z_LOW]);
		    
		    // top
		    glVertex3f(smallBoxCoords[x][y][z][BOX_COORD_X_LOW], smallBoxCoords[x][y][z][BOX_COORD_Y_HIGH], smallBoxCoords[x][y][z][BOX_COORD_Z_LOW]);
		    glVertex3f(smallBoxCoords[x][y][z][BOX_COORD_X_LOW], smallBoxCoords[x][y][z][BOX_COORD_Y_HIGH], smallBoxCoords[x][y][z][BOX_COORD_Z_HIGH]);
		    glVertex3f(smallBoxCoords[x][y][z][BOX_COORD_X_HIGH], smallBoxCoords[x][y][z][BOX_COORD_Y_HIGH], smallBoxCoords[x][y][z][BOX_COORD_Z_HIGH]);
		    
		    glVertex3f(smallBoxCoords[x][y][z][BOX_COORD_X_LOW], smallBoxCoords[x][y][z][BOX_COORD_Y_HIGH], smallBoxCoords[x][y][z][BOX_COORD_Z_LOW]);
		    glVertex3f(smallBoxCoords[x][y][z][BOX_COORD_X_HIGH], smallBoxCoords[x][y][z][BOX_COORD_Y_HIGH], smallBoxCoords[x][y][z][BOX_COORD_Z_HIGH]);
		    glVertex3f(smallBoxCoords[x][y][z][BOX_COORD_X_HIGH], smallBoxCoords[x][y][z][BOX_COORD_Y_HIGH], smallBoxCoords[x][y][z][BOX_COORD_Z_LOW]);
		    
		    // down
		    glVertex3f(smallBoxCoords[x][y][z][BOX_COORD_X_LOW], smallBoxCoords[x][y][z][BOX_COORD_Y_LOW], smallBoxCoords[x][y][z][BOX_COORD_Z_LOW]);
		    glVertex3f(smallBoxCoords[x][y][z][BOX_COORD_X_HIGH], smallBoxCoords[x][y][z][BOX_COORD_Y_LOW], smallBoxCoords[x][y][z][BOX_COORD_Z_HIGH]);
		    glVertex3f(smallBoxCoords[x][y][z][BOX_COORD_X_LOW], smallBoxCoords[x][y][z][BOX_COORD_Y_LOW], smallBoxCoords[x][y][z][BOX_COORD_Z_HIGH]);
		    
		    glVertex3f(smallBoxCoords[x][y][z][BOX_COORD_X_LOW], smallBoxCoords[x][y][z][BOX_COORD_Y_LOW], smallBoxCoords[x][y][z][BOX_COORD_Z_LOW]);
		    glVertex3f(smallBoxCoords[x][y][z][BOX_COORD_X_HIGH], smallBoxCoords[x][y][z][BOX_COORD_Y_LOW], smallBoxCoords[x][y][z][BOX_COORD_Z_LOW]);
		    glVertex3f(smallBoxCoords[x][y][z][BOX_COORD_X_HIGH], smallBoxCoords[x][y][z][BOX_COORD_Y_LOW], smallBoxCoords[x][y][z][BOX_COORD_Z_HIGH]);
		}
	    }
	}
    }
    glEnd();
    //cout << "Renderer::renderSubBoundingBoxes() end" << endl;    
}

void Renderer::setModels(vector<vector<float>*> *mittelpunkte, vector<vector<float>*> *rotation, vector<vector<float>*> *col, vector<int> *modelNr, float sizeX, float sizeY, float sizeZ) {
    //cout << "Renderer::setModels() beg" << endl;    
    middle = mittelpunkte;
    rot = rotation;
    color = col;
    modelInd = modelNr;
    //cout << "modelInd->size() " << modelInd->size() << endl;
    calculateBoundingBox(sizeX,sizeY,sizeZ);
    createGridBoxes(renderSet_BoxCount);
    //cout << "Renderer::setModels() end" << endl;    
}

void Renderer::calculateBoundingBox(float sizeX, float sizeY, float sizeZ) {
    //cout << "Renderer::calculateBoundingBox() beg" << endl;    
    if (middle->size() > 0) {
	float x_low, x_high, y_low, y_high, z_low, z_high;
	x_low = x_high = middle->at(0)->at(0);
	y_low = y_high = middle->at(0)->at(1);
	z_low = z_high = middle->at(0)->at(2);
	
	for (int index = 0; index < (int)middle->size(); index++) {
	    if (middle->at(index)->at(0) < x_low) {
		x_low = middle->at(index)->at(0);
	    } else if (middle->at(index)->at(0) > x_high) {
		x_high = middle->at(index)->at(0);
	    }
	    if (middle->at(index)->at(1) < y_low) {
		y_low = middle->at(index)->at(1);
	    } else if (middle->at(index)->at(1) > y_high) {
		y_high = middle->at(index)->at(1);
	    }
	    if (middle->at(index)->at(2) < z_low) {
		z_low = middle->at(index)->at(2);
	    } else if (middle->at(index)->at(2) > z_high) {
		z_high = middle->at(index)->at(2);
	    }
	}
	
	if(x_low == x_high) {
	    x_high += 1;
	}
	if(y_low == y_high) {
	    y_high += 1;
	}
	if(z_low == z_high) {
	    z_high += 1;
	}
	boundingBoxCoords[BOX_COORD_X_LOW] = min(x_low,-sizeX/2);
	boundingBoxCoords[BOX_COORD_X_HIGH] = max(x_high,sizeX/2);
	boundingBoxCoords[BOX_COORD_Y_LOW] = min(y_low,-sizeY/2);
	boundingBoxCoords[BOX_COORD_Y_HIGH] = max(y_high,sizeY/2);
	boundingBoxCoords[BOX_COORD_Z_LOW] = min(z_low,-sizeZ/2);
	boundingBoxCoords[BOX_COORD_Z_HIGH] = max(z_high,sizeZ/2);
	
    }
    maxBoundingDiff = max(max((boundingBoxCoords[BOX_COORD_X_HIGH]-boundingBoxCoords[BOX_COORD_X_LOW]),(boundingBoxCoords[BOX_COORD_Y_HIGH]-boundingBoxCoords[BOX_COORD_Y_LOW])),(boundingBoxCoords[BOX_COORD_Z_HIGH]-boundingBoxCoords[BOX_COORD_Z_LOW]));
    //cout << "Renderer::calculateBoundingBox() end" << endl;    
}

/*!
 *  Handles mouse press events for the connected widget.
 *
 *  \param 
 *  \return 
 *  \author Timm Meyer
 */
void Renderer::mousePressEvent( QMouseEvent * e )
{
    //cout << "Renderer::mousePressEvent() beg" << endl;
    if( dragLeft || dragRight || dragMid )
    {
	e->ignore();
	return;
    }
    
    if( e->button() == Qt::LeftButton )
    {
	//cout << "left press: " << endl;
	dragLeft = TRUE;
    }
    else if( e->button() == Qt::RightButton )
    {
	//cout << "right press: " << endl;
	dragRight = TRUE;
    }
    else if( e->button() == Qt::MidButton )
    {
	//cout << "mid press: " << endl;
	dragMid = TRUE;
    }
    
    lastDragPoint = e->pos();
    e->accept();
    //cout << "Renderer::mousePressEvent() end" << endl;    
}

/*!
 *  Handles mouse release events for the connect widget.
 *
 *  \param 
 *  \return 
 *  \author Timm Meyer
 */
void Renderer::mouseReleaseEvent( QMouseEvent * e ) {
    //cout << "Renderer::mouseReleaseEvent() beg" << endl;    
    if( e->button() == Qt::LeftButton && !dragRight && !dragMid )
    {
	//cout << "left release: " << endl;
	dragLeft = FALSE;
	e->accept();
    }
    else if( e->button() == Qt::RightButton && !dragLeft && !dragMid )
    {
	//cout << "right release: " << endl;
	dragRight = FALSE;
	e->accept();
    }
    else if( e->button() == Qt::MidButton && !dragLeft && !dragRight )
    {
	//cout << "mid release: " << endl;
	dragMid = FALSE;
	e->accept();
    }
    else
    {
	e->ignore();	
    }
    //cout << "Renderer::mouseReleaseEvent() end" << endl;    
}

/*!
 *  Handles mouse move events for the connect widget.
 *
 *  \param 
 *  \return 
 *  \author Timm Meyer
 */
void Renderer::mouseMoveEvent( QMouseEvent * e) {
    //cout << "Renderer::mouseMoveEvent() beg" << endl;    
    //if ( dragging )
    if( dragLeft || dragRight || dragMid )
    {
	QPoint point = e->pos();
	int deltax = point.x() - lastDragPoint.x();
	int deltay = point.y() - lastDragPoint.y();
	
	if ( dragLeft )
	{
	    //cout << "left move: " << endl;
	    // Das ist nicht vertauscht, das ist korrekt!
	    YMousedelta += deltax;
	    XMousedelta += deltay;
	    
	    //	    GLfloat matrix[16];
	    //	    // get openGL's current modelview matrix
	    //	    glGetFloatv(GL_MODELVIEW_MATRIX, matrix);
	    //	    cout    << "x1: " << matrix[0] << " x2: " << matrix[4] << " x3: " << matrix[8]  << " i: " << matrix[12] << "\n"
	    //		    << "y1: " << matrix[1] << " y2: " << matrix[5] << " y3: " << matrix[9]  << " j: " << matrix[13] << "\n"
	    //		    << "z1: " << matrix[2] << " z2: " << matrix[6] << " z3: " << matrix[10] << " k: " << matrix[14] << "\n"
	    //		    << "m1: " << matrix[3] << " n2: " << matrix[7] << " o3: " << matrix[11] << " l: " << matrix[15] << endl;
	    
	    
	    repaint();
	}
	else if ( dragRight )
	{
	    //cout << "right move: " << endl;
	    GLfloat matrix[16];
	    // get openGL's current modelview matrix
	    glGetFloatv(GL_MODELVIEW_MATRIX, matrix);
	    
	    /*
	    cout    << "x1: " << matrix[0] << " x2: " << matrix[4] << " x3: " << matrix[8]  << "\n"
		    << "y1: " << matrix[1] << " y2: " << matrix[5] << " y3: " << matrix[9]  << "\n"
		    << "z1: " << matrix[2] << " z2: " << matrix[6] << " z3: " << matrix[10] << endl;
	    cout << "|ex|: " << matrix[0]*matrix[0] + matrix[4]*matrix[4] + matrix[8]*matrix[8] << endl;
	    cout << "|ey|: " << matrix[0]*matrix[0] + matrix[4]*matrix[4] + matrix[8]*matrix[8] << endl;
	    cout << "|ez|: " << matrix[0]*matrix[0] + matrix[4]*matrix[4] + matrix[8]*matrix[8] << endl;
	  */	    
	    
	    vector<float> vecTranslation( 3, 0.0 );
	    
	    vecTranslation.at(0) = deltax * matrix[0] + deltay * matrix[4];
	    vecTranslation.at(1) = deltax * matrix[1] + deltay * matrix[5];
	    vecTranslation.at(2) = deltax * matrix[2] + deltay * matrix[6];
	    
	    emit translate( vecTranslation );
	}
	else if ( dragMid )
	{
	    //cout << "mid move: " << endl;
	}
	
	lastDragPoint = point;
	e->accept();
    }
    else
    {
	e->ignore();	
    }
    //cout << "Renderer::mouseMoveEvent() end" << endl;    
}

/*!
 *  Handles mouse wheel events for the connect widget.
 *
 *  \param 
 *  \return 
 *  \author Timm Meyer
 */
void Renderer::wheelEvent( QWheelEvent * e ) {
    //cout << "Renderer::wheelEvent() beg" << endl;    
    
    distanceUpdate += e->delta()/120;
    repaint();
    //cout << "Renderer::wheelEvent() end" << endl;    
}

/*!
 *
 *
 *  \param 
 *  \return 
 *  \author Timm Meyer
 */
void Renderer::setAngles(int a, int b, int g)
{
    //cout << "Renderer::setAngles() beg" << endl;    
    //if(!dragging) {
    if( !dragLeft )
    {
	XRealRot = a;
	YRealRot = b;
	ZRealRot = g;
	resetAngles = true;
	repaint();
    }
    //cout << "Renderer::setAngles() end" << endl;    
}

/*!
 *
 *
 *  \param 
 *  \return 
 *  \author Timm Meyer
 */
void Renderer::setFullRender(bool x) {
    //cout << "Renderer::setFullRender() beg" << endl;    
    renderSet_RenderAsLines = x;
    repaint();
    //cout << "Renderer::setFullRender() end" << endl;    
}

/*!
 *
 *
 *  \param 
 *  \return 
 *  \author Timm Meyer
 */
void Renderer::changeZoomRelative(float x) {
    //cout << "Renderer::changeZoomRelative() beg" << endl;    
    distanceUpdate += x;
    repaint();
    //cout << "Renderer::changeZoomRelative() end" << endl;    
}

/*!
 *
 *
 *  \param 
 *  \return 
 *  \author Timm Meyer
 */
void Renderer::changeZoomAbsolute(float x) {
    //cout << "Renderer::changeZoomAbsolute() beg" << endl;    
    distance = x;
    distAbsolut = true;
    distanceUpdate = 0;
    repaint();
    //cout << "Renderer::changeZoomAbsolute() end" << endl;    
}

/*!
 *
 *
 *  \param 
 *  \return 
 *  \author Timm Meyer
 */
void Renderer::setDrawColorMap(bool t) {
    //cout << "Renderer::setDrawColorMap() beg" << endl;    
    drawColorMap = t;
    repaint();
    //cout << "Renderer::setDrawColorMap() end" << endl;    
}

/*!
 *
 *
 *  \param 
 *  \return 
 *  \author Timm Meyer
 */
void Renderer::setDrawAxis(bool t) {
    //cout << "Renderer::setDrawAxis() beg" << endl;    
    drawAxis = t;
    repaint();
    //cout << "Renderer::setDrawAxis() end" << endl;    
}

/*!
 *
 *
 *  \param 
 *  \return 
 *  \author Timm Meyer // adrian
 */
void Renderer::setDrawBoundingBox(bool t) {
    //cout << "Renderer::setDrawBoundingBox() beg" << endl;    
    //cout << "- set draw bbox" << endl;
    drawBoundingBox = t;
    repaint();
    //cout << "Renderer::setDrawBoundingBox() end" << endl;    
}

/*!
 *
 *
 *  \param 
 *  \return 
 *  \author Timm Meyer
 */
void Renderer::setBoundingBox(float x, float y, float z, bool resetDistance)
{
    //cout << "Renderer::setBoundingBox(float,float,float,bool) beg" << endl;
    x = x/2;
    y = y/2;
    z = z/2;
    
    vector<float> plus_plus_plus;
    vector<float> plus_plus_min;
    vector<float> plus_min_plus;
    vector<float> plus_min_min;
    vector<float> min_plus_plus;
    vector<float> min_plus_min;
    vector<float> min_min_plus;
    vector<float> min_min_min;
    
    plus_plus_plus.push_back(  x );
    plus_plus_plus.push_back(  y );
    plus_plus_plus.push_back(  z );
    plus_plus_min.push_back (  x );
    plus_plus_min.push_back (  y );
    plus_plus_min.push_back ( -z );
    plus_min_plus.push_back (  x );
    plus_min_plus.push_back ( -y );
    plus_min_plus.push_back (  z );
    plus_min_min.push_back  (  x );
    plus_min_min.push_back  ( -y );
    plus_min_min.push_back  ( -z );
    min_plus_plus.push_back ( -x );
    min_plus_plus.push_back (  y );
    min_plus_plus.push_back (  z );
    min_plus_min.push_back  ( -x );
    min_plus_min.push_back  (  y );
    min_plus_min.push_back  ( -z );
    min_min_plus.push_back  ( -x );
    min_min_plus.push_back  ( -y );
    min_min_plus.push_back  (  z );
    min_min_min.push_back   ( -x );
    min_min_min.push_back   ( -y );
    min_min_min.push_back   ( -z );
    
    boundingBoxCoordinates.resize(24);
    boundingBoxCoordinates.at(0)  = plus_plus_plus;
    boundingBoxCoordinates.at(1)  = plus_plus_min;
    boundingBoxCoordinates.at(2)  = min_plus_plus;
    boundingBoxCoordinates.at(3)  = min_plus_min;
    boundingBoxCoordinates.at(4)  = min_min_plus;
    boundingBoxCoordinates.at(5)  = min_min_min;
    boundingBoxCoordinates.at(6)  = plus_min_plus;
    boundingBoxCoordinates.at(7)  = plus_min_min;
    boundingBoxCoordinates.at(8)  = plus_plus_plus;
    boundingBoxCoordinates.at(9)  = plus_min_plus;
    boundingBoxCoordinates.at(10) = min_plus_plus;
    boundingBoxCoordinates.at(11) = min_min_plus;
    boundingBoxCoordinates.at(12) = min_plus_min;
    boundingBoxCoordinates.at(13) = min_min_min;
    boundingBoxCoordinates.at(14) = plus_plus_min;
    boundingBoxCoordinates.at(15) = plus_min_min;
    boundingBoxCoordinates.at(16) = plus_plus_plus;
    boundingBoxCoordinates.at(17) = min_plus_plus;
    boundingBoxCoordinates.at(18) = plus_min_plus;
    boundingBoxCoordinates.at(19) = min_min_plus;
    boundingBoxCoordinates.at(20) = plus_min_min;
    boundingBoxCoordinates.at(21) = min_min_min;
    boundingBoxCoordinates.at(22) = plus_plus_min;
    boundingBoxCoordinates.at(23) = min_plus_min;
    
    if(resetDistance)
    {
	distanceUpdate = int(max(max(x,y),z)*1.5) - distance;
    }
    //cout << "Renderer::setBoundingBox(float,float,float,bool) end" << endl;
}

/*!
 *  \author Adrian Gabriel
 */
void Renderer::setBoundingBox( vector<vector<float> > bbox, bool resetDistance )
{
    //cout << "Renderer::setBoundingBox(vector<vector<flaot>>,bool) beg" << endl;
    //cout << "Renderer::setBoundingBox 2 beg" << endl;
    boundingBoxCoordinates = bbox;
    if(resetDistance)
    {
	distanceUpdate = int(maxBoundingDiff*1.5) - distance;
    }
    //cout << "Renderer::setBoundingBox(vector<vector<flaot>>,bool) end" << endl;
}

/*!
 *
 *
 *  \param 
 *  \return 
 *  \author Timm Meyer
 */
void Renderer::setColorMap(vector <float *> *f) {
    //cout << "Renderer::setColorMap() beg" << endl;
    colorMap = f; 
    //cout << "Renderer::setColorMap() end" << endl;
}

/*!
 *
 *
 *  \param 
 *  \return 
 *  \author Timm Meyer
 */
void Renderer::setAxisColors(float x_r, float x_g, float x_b,float y_r, float y_g, float y_b,float z_r, float z_g, float z_b) {
    //cout << "Renderer::setAxisColors() beg" << endl;
    axisColors[0][0] = x_r;
    axisColors[0][1] = x_g;
    axisColors[0][2] = x_b;
    axisColors[1][0] = y_r;
    axisColors[1][1] = y_g;
    axisColors[1][2] = y_b;
    axisColors[2][0] = z_r;
    axisColors[2][1] = z_g;
    axisColors[2][2] = z_b;
    //cout << "Renderer::setAxisColors() end" << endl;
}

/*!
 *
 *
 *  \param 
 *  \return 
 *  \author Timm Meyer
 */
void Renderer::getAxisColors(float &x_r, float &x_g, float &x_b,
			     float &y_r, float &y_g, float &y_b,
			     float &z_r, float &z_g, float &z_b) {
    //cout << "Renderer::getAxisColors() beg" << endl;
    x_r = axisColors[0][0];
    x_g = axisColors[0][1];
    x_b = axisColors[0][2];
    y_r = axisColors[1][0];
    y_g = axisColors[1][1];
    y_b = axisColors[1][2];
    z_r = axisColors[2][0];
    z_g = axisColors[2][1];
    z_b = axisColors[2][2];
    //cout << "Renderer::getAxisColors() end" << endl;
}

/*!
 *
 *
 *  \param 
 *  \return 
 *  \author Timm Meyer
 */
void Renderer::setSliceBounds(float x_lo, float x_hi, float y_lo, float y_hi, float z_lo, float z_hi) {
    //cout << "Renderer::setSliceBounds() beg" << endl;
    sliceXLow = x_lo;
    sliceXHigh = x_hi;
    sliceYLow = y_lo;
    sliceYHigh = y_hi;
    sliceZLow = z_lo;
    sliceZHigh = z_hi;
    //cout << "Renderer::setSliceBounds() end" << endl;
}

/*!
 *
 *
 *  \param 
 *  \return 
 *  \author Timm Meyer
 */
void Renderer::setDrawSlices(bool on) {
    //cout << "Renderer::setDrawSlices() beg" << endl;
    drawAsSlice = on;
    repaint();
    //cout << "Renderer::setDrawSlices() end" << endl;
}

/*!
 *  
 * 
 *  \param 
 *  \return 
 *  \author Timm Meyer
 */
void Renderer::setOptimized(bool on) {
    //cout << "Renderer::setOptimized() beg" << endl;
    //cout << "OPtim " << on  << endl;
    drawOptimized = on;
    repaint();
    //cout << "Renderer::setOptimized() end" << endl;
}

void Renderer::setLOD(bool on) {
    //cout << "Renderer::setLOD() beg" << endl;
    renderSet_UseAutoLOD = on;
    if(!on) {
	LODdelta = 0;
	for(unsigned int i=0; i < layerLOD.size(); i++) {
	    layerLOD.at(i) = 0;
	}
    }
    
    repaint();
    //cout << "Renderer::setLOD() end" << endl;
}

/*!
 *
 *
 *  \param 
 *  \return 
 *  \author Timm Meyer
 */
void Renderer::rotateXAxis(float v) {
    //cout << "Renderer::rotateXAxis() beg" << endl;
    xAdditionalRotation = v;
    repaint();
    //cout << "Renderer::rotateXAxis() end" << endl;
}

/*!
 *
 *
 *  \param 
 *  \return 
 *  \author Timm Meyer
 */
void Renderer::rotateYAxis(float v) {
    //cout << "Renderer::rotateYAxis() beg" << endl;
    yAdditionalRotation = v;
    repaint();
    //cout << "Renderer::rotateYAxis() end" << endl;
}

/*!
 *
 *
 *  \param 
 *  \return 
 *  \author Timm Meyer
 */
void Renderer::rotateZAxis(float v) {
    //cout << "Renderer::rotateZAxis() beg" << endl;
    zAdditionalRotation = v;
    repaint();
    //cout << "Renderer::rotateZAxis() end" << endl;
}

/*!
 *
 *
 *  \param 
 *  \return 
 *  \author Timm Meyer
 */
void Renderer::waitForRepaint() {
    //cout << "Renderer::waitForRepaint() beg" << endl;
    rendering = true;
    repaint();
    while(rendering) {
	sleep(20);
    }
    //cout << "Renderer::waitForRepaint() end" << endl;
}

/*!
 *
 *
 *  \param 
 *  \return 
 *  \author Timm Meyer
 */
void Renderer::getSnapShot(int snapW, int snapH, QString pFileName, bool png, bool eps ) {
    //cout << "Renderer::getSnapShot() beg" << endl;
    if( !png && !eps ) {
	emit noFileFormatSet();
	cerr << "Warning: no file format chosen. Do nothing!" << endl;
	//cout << "Renderer::getSnapShot() end" << endl;
	return;
    }
    
    QStrList liste = QImage::outputFormats();
    
    if( !liste.contains("PNG") && png ) {
	cerr << "Warning: png support not included in your Qt installation!" << endl;
    }
    
    GLint OldReadBuffer;
    glGetIntegerv(GL_READ_BUFFER,&OldReadBuffer);
    
    GLint OldPackAlignment;
    glGetIntegerv(GL_PACK_ALIGNMENT,&OldPackAlignment);
    glPixelStorei(GL_PACK_ALIGNMENT,1);
    
    int iNumPixels = snapW * snapH;
    
    uchar* pPixels = new GLubyte[iNumPixels * 4];
    if( pPixels==NULL ) {
	qWarning("QGLVUWidget::SnapShot(): Unable to alloc pixel read array!\n");
	//cout << "Renderer::getSnapShot() end" << endl;
	return;
    }
    
    /* Setup tiled rendering */
    tiledRender = trNew();
    trOrtho(tiledRender, -sideRelation * (distance / 2), sideRelation * (distance / 2), -(distance / 2), (distance / 2), nearClip, farClip+abs(distance));
    trTileSize(tiledRender, width(),height(), 10);
    //    trTileSize(tiledRender, 128,128, 0);
    
    /* We don't call trTileBuffer() since we're not interested in getting
   * each individual tile's data.
   */
    trImageSize(tiledRender , snapW, snapH);
    trImageBuffer(tiledRender , GL_RGBA, GL_UNSIGNED_BYTE, pPixels);
    
    offScreen = true;
    
    int tileCount = 0;
    /* Draw tiles */
    do {
	trBeginTile( tiledRender );
	paintGL();
	tileCount++;
    } while( trEndTile(tiledRender ) );
    //printf("%d tiles drawn\n", tileCount);
    
    offScreen = false;
    trDelete( tiledRender );
    
    if( liste.contains("PNG") && png ) {
	// create a QImage;
	QImage pImage( pPixels, snapW, snapH, 32, 0, 0, QImage::IgnoreEndian );
	pImage = pImage.mirror(false,true);
	pImage = pImage.swapRGB();
	pImage.save(pFileName+".png", "PNG", 0);
    }
    
    if( eps ) {
	string fileName = pFileName+".eps";
	FILE *fp = fopen( fileName.c_str(), "w" );
	
	if( fp != NULL ) {
	    
	    float printsize[] = {8.5f, 11.0f};
	    short size[] = {(short)snapW, (short)snapH};
	    cout << writeToPostScript(fp,
				      printsize,
				      4,
				      144.0f,
				      pPixels,
				      size);
	} else {
	    cerr << "ERROR FILE NULL" << endl;
	}
    }
    delete[] pPixels;
    
    glPixelStorei(GL_PACK_ALIGNMENT,OldPackAlignment);
    
    // Reset view
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(-sideRelation * (distance / 2), sideRelation * (distance / 2), -(distance / 2), (distance / 2), nearClip, farClip+abs(distance));
    glMatrixMode(GL_MODELVIEW);
    //cout << "Renderer::getSnapShot() end" << endl;
}

void Renderer::setUseLOD(bool use) {
    //cout << "Renderer::setUseLOD() beg" << endl;
    renderSet_UseAutoLOD = use;
    if(!use) {
	for (int i = 0; i < (int)layerLOD.size(); i++) {
	    layerLOD.at(i) = 0;
	}
    }
    //cout << "Renderer::setUseLOD() end" << endl;
}

void Renderer::setBoxCount(int x) {
    //cout << "Renderer::setBoxCount() beg" << endl;
    renderSet_BoxCount = x;
    recreateModel = true;
    //cout << "Renderer::setBoxCount() end" << endl;
}

/*!
 *
 *
 *  \param 
 *  \return 
 *  \author Timm Meyer
 */
void Renderer::repaint() {
    //cout << "Renderer::repaint() beg" << endl;
    QGLWidget::repaint();
    //cout << "Renderer::repaint() end" << endl;
}

/*!
 *  Catches x, y, z and c keypress and sends a signal.
 *  \param 
 *  \return 
 *  \author Adrian Gabriel
 */
void Renderer::keyPressEvent( QKeyEvent *e )
{
    //cout << "Renderer::keyPressEvent() beg" << endl;
    QString tmp = e->text();
    if( tmp == "x" || tmp == "y" || tmp == "z" || tmp == "c" )
    {
	e->accept();
	if( tmp == "x" )
	{
	    emit keyPress(0);
	}
	else if( tmp == "y" )
	{
	    emit keyPress(1);
	}
	else if( tmp == "z" )
	{
	    emit keyPress(2);
	}
	else if( tmp == "c" )
	{
	    emit keyPress(3);
	}
    }
    else
    {
	e->ignore();
    }
    //cout << "Renderer::keyPressEvent() end" << endl;
}

//-------------------------------------------------------------------------
void Renderer::dropEvent( QDropEvent *event )
{
    //cout << "Renderer::dropEvent() beg" << endl;
    QStringList files;
    QString text="";
    
    if( QUriDrag::decodeLocalFiles(event, files) ) 
    {
	if( files.isEmpty() == false )
	{
	    if( files.first().startsWith("SSH/") == false )
	    {
		emit dropAccepted( files.first() );
	    }
	    else
	    {
		cerr << "Warning: Local root folder 'SSH/' is ambiguous. Ignoring drop!" << endl;
	    }
	}
    }
    else if( QTextDrag::decode(event, text) )
    {
	if( text.startsWith("SSH|") == true )
	{
	    emit dropAccepted( text );	
	}
    }
    //cout << "Renderer::dropEvent() end" << endl;
}

//-------------------------------------------------------------------------
void Renderer::dragEnterEvent( QDragEnterEvent *event )
{
    //cout << "Renderer::dragEnterEvent() beg" << endl;
    if( QUriDrag::canDecode(event) || QTextDrag::canDecode(event) )
    {
	event->accept( true );
    }
    //cout << "Renderer::dragEnterEvent() end" << endl;
}

/*!
 *  Function used to get lighting specifications
 *
 *  \author Adrian Gabriel
 */
void Renderer::getData( vector<float> &data )
{
    //cout << "Renderer::getData() beg" << endl;
    float tmp[4];
    //float tmp2[4];
    data.resize(0);
    
    glGetLightfv(GL_LIGHT1, GL_AMBIENT , tmp );
    data.push_back( tmp[0] );                     //  0: ambient r
    data.push_back( tmp[1] );                     //  1: ambient g
    data.push_back( tmp[2] );                     //  2: ambient b
    glGetLightfv(GL_LIGHT1, GL_DIFFUSE , tmp );
    data.push_back( tmp[0] );                     //  3: diffuse r
    data.push_back( tmp[1] );                     //  4: diffuse g
    data.push_back( tmp[2] );                     //  5: diffuse b
    glGetLightfv(GL_LIGHT1, GL_SPECULAR, tmp );
    data.push_back( tmp[0] );                     //  6: specular r
    data.push_back( tmp[1] );                     //  7: specular g
    data.push_back( tmp[2] );                     //  8: specular b
    data.push_back( shininess );                  //  9: shininess
    glGetLightfv(GL_LIGHT1, GL_POSITION, tmp );
    data.push_back( tmp[0] );                     // 10: light position x
    data.push_back( tmp[1] );                     // 11: light position y
    data.push_back( tmp[2] );                     // 12: light position z
    glGetFloatv( GL_COLOR_CLEAR_VALUE, tmp );	
    data.push_back( tmp[0] );                     // 13: background color r
    data.push_back( tmp[1] );                     // 14: background color g
    data.push_back( tmp[2] );                     // 15: background color b
    
    data.push_back( boundingBoxColor.at(0) );     // 16: bounding box color r
    data.push_back( boundingBoxColor.at(1) );     // 17: bounding box color g
    data.push_back( boundingBoxColor.at(2) );     // 18: bounding box color b
    
    data.push_back( maxBoundingDiff );            // 19: camera position z
    
    data.push_back( XRealRot );                   // 20: camera rotation x
    data.push_back( YRealRot );                   // 21: camera rotation y
    data.push_back( ZRealRot );                   // 22: camera rotation z
    
    data.push_back( distance );                   // 23: zoom value
    
    data.push_back( double(drawBoundingBox) );    // 24: bounding box on/off
    
    //    glGetFloatv( GL_COLOR_CLEAR_VALUE, tmp2 );
    //    data.push_back( tmp2[0] );                    // 24: bgcolor
    //    data.push_back( tmp2[1] );
    //    data.push_back( tmp2[2] );
    //    data.push_back( tmp2[3] );
    
    //data.push_back( boundingBoxXDraw );           // 24: 0.5*bounding box length x
    //data.push_back( boundingBoxYDraw );           // 25: 0.5*bounding box length y
    //data.push_back( boundingBoxZDraw );           // 26: 0.5*bounding box length z
    
    //cout << "Renderer::getData() end" << endl;
}



/*
void Renderer::paintEvent( QPaintEvent * e )
{
    cout << "Renderer::paintEvent()" << endl;
    QGLWidget::paintEvent(e);
}
*/






