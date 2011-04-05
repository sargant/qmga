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

#ifndef RENDERER_H
#define RENDERER_H

using namespace std;

#include <qgl.h>

#include <vector>
#include <sys/time.h>

#include "tr/tr.h"
#include "mga_tools.h"

class MainForm;

#define RENDERSIDE_X 1
#define RENDERSIDE_Y 2
#define RENDERSIDE_Z 3
#define RENDERSIDE_NONE 0
#define BOX_COORD_X_LOW 0
#define BOX_COORD_X_HIGH 1
#define BOX_COORD_Y_LOW 2
#define BOX_COORD_Y_HIGH 3
#define BOX_COORD_Z_LOW 4
#define BOX_COORD_Z_HIGH 5
#define CLK_MIL (CLOCKS_PER_SEC/1000)

class Renderer : public QGLWidget
{
    Q_OBJECT
    
public:
    Renderer( QWidget* parent, const char* name );
    ~Renderer();
    
    void setModels(vector<vector<float>*> *mittelpunkte, vector<vector<float>*> *rotation, vector<vector<float>*> *col, vector<int> *modelNr, float sizeX, float sizeY, float sizeZ);
    void setAngles(int a, int b, int g);
    void setFullRender(bool x);
    void setModelParams(int compxmax, int compymax, int compxmin, int compymin, int levels, vector<vector<float> > data);
    void changeZoomRelative(float x);
    void changeZoomAbsolute(float x);    
    void setDrawColorMap(bool t);
    void setDrawAxis(bool t);
    void setDrawBoundingBox(bool t);
    void setBoundingBox(float x, float y, float z, bool resetDistance);
    void setBoundingBox( vector<vector<float> > bbox, bool resetDistance );
    void setColorMap(vector <float *> *f);
    void setModelColors(vector< float* > *col);
    void setAxisColors(float x_r, float x_g, float x_b,float y_r, float y_g, float y_b,float z_r, float z_g, float z_b);
    void getAxisColors(float &x_r, float &x_g, float &x_b, float &y_r, float &y_g, float &y_b, float &z_r, float &z_g, float &z_b);
    void setSliceBounds(float x_lo, float x_hi, float y_lo, float y_hi, float z_lo, float z_hi);
    void getSliceBounds(float &x_lo, float &x_hi, float &y_lo, float &y_hi, float &z_lo, float &z_hi)
    {
	x_lo = sliceXLow;
	x_hi = sliceXHigh;
	y_lo = sliceYLow;
	y_hi = sliceYHigh;
	z_lo = sliceZLow;
	z_hi = sliceZHigh;
    }
    void setDrawSlices(bool on);
    bool getDrawSlices() { return(drawAsSlice); }
    void setOptimized(bool on);
    void setLOD(bool on);
    void setUseLOD(bool use);
    void getSnapShot(int w, int h, QString pFileName, bool png, bool eps );
    
    vector<vector<float> > *getObjectParams() { return( &objectParams ); }
    
    void rotateXAxis(float v);
    void rotateYAxis(float v);
    void rotateZAxis(float v);
    
    void waitForRepaint();
    void repaint();
    void setLight(vector<float> amb, vector<float> diff, vector<float> spec, vector<float> pos, float shini, vector<float> bgColor, vector<float> bbColor);
    void getData( vector<float> &data );
    void setLineSize(float f);
    void setBoxCount(int x);
    
protected:
    
    void mousePressEvent( QMouseEvent *);
    void mouseReleaseEvent( QMouseEvent *);
    void mouseMoveEvent( QMouseEvent *);
    void wheelEvent ( QWheelEvent * e );
    void keyPressEvent( QKeyEvent * e );
    void dropEvent( QDropEvent *event );
    void dragEnterEvent( QDragEnterEvent *event );
    void initializeGL();
    void paintGL();
    void resizeGL( int w, int h );
    
    //void paintEvent( QPaintEvent * e );

    
signals:
    void sizeChanged();
    void dropAccepted(QString);
    void noFileFormatSet();
    void zoomChanged(int);
    void angleXChanged(int);
    void angleYChanged(int);
    void angleZChanged(int);
    void keyPress(int);
    void fpsUpdate(double);
    void translate(vector<float>);
    
private:
	vector<int> howManyDraw;
	vector<int> howManyIndices;
	vector<int> modelNArray;
	vector<int> modelIArray;

    inline void glTranslateRotateCallList(int objectIndex, int glListIndex);
    void calculateBoundingBox(float sizeX, float sizeY, float sizeZ);
    void renderFromSide();
    void renderFromCorner();
    void renderSubBoundingBoxes();
    void processInput(bool v);
    void calculateRotationMatrix();
    void renderColormap();
    void translateAndRotateCamera();
    void updateGuiF();
    void displayModels();
    void displayBoundingBox();
    void processFPS(int x);
    bool increaseLOD(int x);
    bool decreaseLOD(int x);
    void updateGuiF(bool force);
    inline void swap(int i, int j);
    void createModels1(int x);
    void createModels1Wireframe(int index);
    void createModels2(int x);
    void createModels2Wireframe(int index);
    void createModels3(int x);
    void createModels3Wireframe(int index);
    void createModels4(int x);
    void createModels4Wireframe(int index);
    void createModels5(int x);
    void createModels5Wireframe(int index);
    inline float getLength(float* b);
    inline float getLength(float x, float y, float z);
    float* normalizeV(float x, float y, float z, float scaleY);
    float* normalizeV(float* b, float scaleX, float scaleY, float scaleZ);
    float* normalizeV(float* b, float scaleY);
    void createGridBoxes(int x);
    void resetRotation();
    time_t render_LastFrameStartTime;
    float render_RotationMatrix[16];
    
    bool renderSet_RenderAsLines;
    bool renderSet_occlusionExtensionSupported;
    bool renderSet_UseOcclusionTest;
    bool renderSet_UseAutoLOD;
    int renderSet_BoxCount;
    int renderSet_occlusionLayerStartFactor; // percent of layers which have to be drawn, before occlusion test is started
    int renderSet_minimumPixelsToDraw;
    int renderSet_RenderSideStartingPercent;
    int renderSet_RenderSide;
    bool drawOptimized;
    bool drawColorMap;
    bool drawAxis;
    bool drawBoundingBox;
    bool drawAsSlice;
    
    bool rendering;
    bool initialized;
    bool updateGui;
    
    // used for calculating the nearest corner to the viewpoint
    float xCoord[8];
    float yCoord[8];
    float zCoord[8];
    int xDir[8];
    int yDir[8];
    int zDir[8];
    float sqDist[8];
    
    // colors of the colorMap
    vector< float* > *colorMap;
    
    // clipping distances
    float nearClip;
    float farClip;
    
    // line-sizes used by OpenGl   
    float lineSizes[2];
    // factor [0,1] 0 = minimum size, 1 = maximum size
    float lineSizeRelative;    
    
    float boundingBoxCoords[6];
    float maxBoundingDiff;
    
    // Drawing BB
    float boundingBoxXDraw;
    float boundingBoxYDraw;
    float boundingBoxZDraw;
    bool freeBoundingBox;
    vector<vector<float> > boundingBoxCoordinates;

    // Used for marking gridboxes, which have to be drawn this frame  
    bool currentState;
    
    // data about the models
    vector<vector<float>*> *middle; // center
    vector<vector<float>*> *rot; // rotation
    vector<vector<float>*> *color; // color
    vector<int> *modelInd; // model type
    
    // lighting, axis, color stuff
    float axisColors[3][3];
    GLuint textures[1];
    float colorMapLeftDelta;
    float shininess;
    vector<float> boundingBoxColor;
    
    // normal model  
    vector<int> modelListIndex;
    // "line" model
    vector<int> modelListSimpleIndex;
    // used to signal that we have to recreate the model, either because of a restart, or
    // because of a parameter change caused by the user
    bool recreateModel;
    // complexity used for calculating the different LOD models
    int modelXComplexity_max;
    int modelYComplexity_max;
    int modelXComplexity_min;
    int modelYComplexity_min;
    int modelLevels; // how many LOD levels we have
    vector<vector<float> > objectParams;
    vector<int> layerLOD; // different LOD layers
    // maximum Model Size
    float globalMaxModelSize;
    
    // shared array for different calculations  
    float speedArr[3];
    
    // used if we have LOD enabled, but not in layers   
    int LODdelta;
    // used to show the distribution of the grid, not available via the UserInterface
    bool drawBoxes;
    
    // Grid Boxes  
    vector< vector< vector< vector<int> > > > smallBoxes; // contains the indexes of the models belonging to each grid box
    vector< vector< vector<int> > > smallBoxCounter; // counts the elements inside the grid boxes
    vector< vector< vector< float* > > > smallBoxCoords; // coordinates of the grid box
    vector< vector< vector<bool> > > smallBoxState; // drawing state for the grid box for the actual frame
    
    // used for rendering from the side
    int startBox_X;
    int startBox_DeltaX;
    int startBox_Y;
    int startBox_DeltaY;
    int startBox_Z;
    int startBox_DeltaZ;
    
    int viewWidth;
    int viewHeigth;
    float sideRelation;
    
    float distance;
    float distanceUpdate;
    bool distAbsolut; // Used to distinguish between absolute and relative distance changes
    
    // rotation + mouse stuff
    float XMousedelta;
    float YMousedelta;
    float XRealRot;
    float YRealRot;
    float ZRealRot;
    float xAdditionalRotation;
    float yAdditionalRotation;
    float zAdditionalRotation;
    bool resetAngles;
    
    time_t lastUpdate;
    
    // drag stuff    
    int pressedButton;
    //bool dragging;
    bool dragRight;
    bool dragLeft;
    bool dragMid;
    QPoint lastDragPoint;
    
    // slice stuff  
    float sliceXLow;
    float sliceXHigh;
    float sliceYLow;
    float sliceYHigh;
    float sliceZLow;
    float sliceZHigh;
    
    // Screenshot stuff  
    bool offScreen; // User for screenshot taking
    TRcontext *tiledRender;
    
    void ttrRasterPos3f(float x, float y, float z) {
	trRasterPos3f(tiledRender, (GLfloat)x, (GLfloat)y, (GLfloat)z);
    }
    
    void tglRasterPos3f(float x, float y, float z) {
	glRasterPos3f((GLfloat)x, (GLfloat)y, (GLfloat)z);
    }
};

#endif // RENDERER_H
