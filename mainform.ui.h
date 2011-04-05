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

/****************************************************************************
** ui.h extension file, included from the uic-generated form implementation.
**
** If you want to add, delete, or rename functions or slots, use
** Qt Designer to update this file, preserving your code.
**
** You should not define a constructor or destructor in this file.
** Instead, write your code in functions called init() and destroy().
** These will automatically be called by the form's constructor and
** destructor.
*****************************************************************************/

#include "mga_tools.h"
#include <sstream>
//-------------------------------------------------------------------------
const QString WINDOWS_REGISTRY = "/qmga";
const QString APP_KEY = "/qmga/";

enum ReloadSameName { DONTRELOADSAME, RELOADSAME };

//-------------------------------------------------------------------------
//------------- init
//-------------------------------------------------------------------------
/*!
 *  This function is automatically called by the objects constructor. (default qt behaviour)
 *  \author Adrian Gabriel
 *  \date Dec 2005
 */
void MainForm::init()
{
    //cout << "MainForm::init beg" << endl;
    glWindow  = new Renderer   ( glFrame, "rend" );
    glLayout  = new QGridLayout( glFrame        );
    glLayout -> addWidget      ( glWindow, 0, 0 );
    
    qtTimer   = new QTimer( this );
    saveTimer = new QTimer( this );
    
    middle    = new vector<vector<float>*> ();
    rot       = new vector<vector<float>*> ();
    color     = new vector<vector<float>*> ();
    modelType = new vector<int> ();  
    
    saveAsActivcated = false;
    
    videoSliderActive       = false;
    videoStartPressed       = false;
    blockCnfChanged         = false;
    blockUpdate             = false;
    forceChangeColorization = false;
    showCalled              = false; // set this before the first call of rescale scene() !
    
    cnf        = NULL;
    oldCnf     = NULL;
    aboutForm  = NULL;
    colorForm  = NULL;
    povform    = NULL;
    sliceForm  = NULL;
    sshRoot    = NULL;
    modelsForm = new ModelsForm( this );
    models     = new vector<vector<float> >;
    
    modelNames << "ellipsoid" << "spherocylinder" << "spheroplatelet" << "cut sphere"<<"eyelens";
    modelsForm -> setModelNames( modelNames );
    ((QComboBox*)(toolBar_model1 -> child("comboBox_model1"))) -> insertStringList( modelNames );
    ((QComboBox*)(toolBar_model2 -> child("comboBox_model2"))) -> insertStringList( modelNames );
    
    
    sshUser = sshHost = "";
    
    openHist_1 = openHist_2 = openHist_3 = openHist_4 = ".";
    
    setDefaultModels();
    modelsForm -> setModelsVector( models );
    
    gnuplot = new QProcess( QString("gnuplot"), this, "gnuplot" );
    gnuplot -> addArgument( "-" );
    
    QSettings settings;
    settings.insertSearchPath( QSettings::Windows, WINDOWS_REGISTRY );
    action_useDirector            -> setOn( settings.readBoolEntry( APP_KEY + "UseDirector", true ) );
    action_useUserDefined         -> setOn( settings.readBoolEntry( APP_KEY + "UseUserDefined", false ) );
    action_useColorByModel        -> setOn( settings.readBoolEntry( APP_KEY + "UseColorByModel", false ) );    
    action_toggleObjectsChangable -> setOn( settings.readBoolEntry( APP_KEY + "ObjectsChangable", false ) );
    action_toggleObjects          -> setOn( settings.readBoolEntry( APP_KEY + "Objects", true ) );
    if( action_toggleObjectsChangable -> isOn() ) { action_toggleObjects -> setEnabled( true ); }
    
    setConnections();
    loadSettings();
    initTreeWindow();
    updateModelsVector();
    
    toggleOptimized( action_toggleOptimized -> isOn() );
    toggleLOD      ( action_toggleLOD -> isOn()       );
    
    numberMeanFps = 1;
    
    //cout << "MainForm::init end" << endl;
}

//-------------------------------------------------------------------------
//------------- setDefaultModels
//-------------------------------------------------------------------------
void MainForm::setDefaultModels()
{
    //cout << "MainForm::setDefaultModels beg " << number << endl;
    
    vector<float> ellipse;
    ellipse.push_back( 0    ); // type
    ellipse.push_back( 0.5  ); // 0.5 * x
    ellipse.push_back( 0.5  ); // 0.5 * y
    ellipse.push_back( 0.1  ); // 0.5 * z
    ellipse.push_back( 0.25 ); // d
    ellipse.push_back( 1    ); // l
    ellipse.push_back( 0.5  ); // rad s1
    ellipse.push_back( 0.5  ); // rad circ
    ellipse.push_back( 0.5  ); // rad s2
    ellipse.push_back( 0.25 ); // cut
    ellipse.push_back( 0.5  ); // rad underlying sphere
    ellipse.push_back( 3    ); // opening angle
    ellipse.push_back( 0    ); // wireframe
    ellipse.push_back( 0    ); // force model color
    ellipse.push_back( 255  );
    ellipse.push_back( 50   );
    ellipse.push_back( 50   );
    
    vector<float> sphaero;
    sphaero.push_back( 1    ); // type
    sphaero.push_back( 0.5  ); // 0.5 * x
    sphaero.push_back( 0.5  ); // 0.5 * y
    sphaero.push_back( 1    ); // 0.5 * z
    sphaero.push_back( 0.25 ); // d
    sphaero.push_back( 1    ); // l
    sphaero.push_back( 0.5  ); // rad s1
    sphaero.push_back( 0.5  ); // rad circ
    sphaero.push_back( 0.5  ); // rad s2
    sphaero.push_back( 0.25 ); // cut
    sphaero.push_back( 0.5  ); // rad underlying sphere
    sphaero.push_back( 3    ); // opening angle
    sphaero.push_back( 0    ); // wireframe
    sphaero.push_back( 0    ); // force model color
    sphaero.push_back( 50   ); // red
    sphaero.push_back( 50   ); // green
    sphaero.push_back( 255  ); // blue
    
    models -> push_back(ellipse);
    models -> push_back(sphaero);
    //cout << "MainForm::setDefaultModels end " << number << endl;
}

//-------------------------------------------------------------------------
//------------- addDummyModels
//-------------------------------------------------------------------------
void MainForm::addDummyModels( uint number )
{
    //cout << "MainForm::addDummyModels beg " << number << endl;
    
    vector<float> dummy;
    dummy.push_back( 0    ); // type
    dummy.push_back( 0.5  ); // 0.5 * x
    dummy.push_back( 0.5  ); // 0.5 * y
    dummy.push_back( 0.5  ); // 0.5 * z
    dummy.push_back( 0.25 ); // d
    dummy.push_back( 1    ); // l
    dummy.push_back( 0.5  ); // rad s1
    dummy.push_back( 0.5  ); // rad circ
    dummy.push_back( 0.5  ); // rad s2
    dummy.push_back( 0.25 ); // cut
    dummy.push_back( 0.5  ); // rad underlying sphere
    dummy.push_back( 3    ); // opening angle
    dummy.push_back( 0    ); // Wireframe
    dummy.push_back( 0    ); // force model color
    dummy.push_back( 150  ); // red
    dummy.push_back( 150  ); // green
    dummy.push_back( 150  ); // blue
    
    for( uint i = 0; i<number; ++i )
    {
	models -> push_back(dummy);   
    }
    
    //cout << "MainForm::addDummyModels end" << endl;
}

//-------------------------------------------------------------------------
//------------- setInputFiles
//-------------------------------------------------------------------------
/*!
 *  Slot to set input file and colormap before the object is initially drawn
 *  \author Adrian Gabriel
 *  \date Dec 2005
 */
void MainForm::setInputFiles( QString file, QString color )
{
    //cout << "MainForm::setInputFiles beg" << endl;
    cnfFile  = file;
    colorMap = color;
    //cout << "MainForm::setInputFiles end" << endl;
}

//-------------------------------------------------------------------------
//------------- setOnlyColormap
//-------------------------------------------------------------------------
/*!
 *  Slot to set colormap before the object is initially drawn
 *  used, when cnf input file is to be read from saved settings.
 *  \author Adrian Gabriel
 *  \date Dec 2005
 */
void MainForm::setOnlyColormap( QString color )
{
    //cout << "MainForm::setOnlyColormap beg" << endl;
    colorMap = color;
    //cout << "MainForm::setOnlyColormap end" << endl;
}

//-------------------------------------------------------------------------
//------------- setFormat
//-------------------------------------------------------------------------
/*!
 *  \author Adrian Gabriel
 *  \date Jan 2008
 */
void MainForm::setFormat( uint f )
{
    //cout << "MainForm::setFormat beg" << endl;
    comboBox_fileType->setCurrentItem( f );
    //cout << "MainForm::setFormat end" << endl;
}

//-------------------------------------------------------------------------
//------------- formatChanged
//-------------------------------------------------------------------------
/*!
 *  \author Adrian Gabriel
 *  \date Jan 2008
 */
void MainForm::formatChanged( int f )
{
    //cout << "MainForm::formatChanged beg" << endl;
    if( f >= 0 )
    {
	setFormat( f );
	if( cnf != 0 ) { cnf -> setLoadCnfFileIndex( f ); }
    }
    //cout << "MainForm::formatChanged end" << endl;
}

//-------------------------------------------------------------------------
//------------- show
//-------------------------------------------------------------------------
/*!
 *  Overridden function to construct the scenegraph before the window is drawn
 *  \author Adrian Gabriel
 *  \date Dec 2005
 */
void MainForm::show()
{
    //cout << "MainForm::show mainform beg" << endl;
    initializeScene();
    initSliceWindow();
    
    updateModelsLineEdits( false );
    
    topDock()    -> setAcceptDockWindow ( sliceWindow, false );
    topDock()    -> setAcceptDockWindow ( treeWindow , false );
    rightDock()  -> setAcceptDockWindow ( sliceWindow, false );
    rightDock()  -> setAcceptDockWindow ( treeWindow , false );
    bottomDock() -> setAcceptDockWindow ( treeWindow , false );
    
    QWidget::show();
    
    if( !colorForm )
    {
	colorForm = new ColorForm( this );
	colorForm -> setGlWindow( glWindow );
	
	QSettings settings;
	settings.insertSearchPath( QSettings::Windows, WINDOWS_REGISTRY );
	float red = 0.0, green = 0.0, blue = 0.0;
	int shiny = 0;
	
	red   = settings.readDoubleEntry( APP_KEY + "AmbientRed"  , 0.05 );
	green = settings.readDoubleEntry( APP_KEY + "AmbientGreen", 0.05 );
	blue  = settings.readDoubleEntry( APP_KEY + "AmbientBlue" , 0.05 );
	colorForm -> setSliderAmbientValues( red, green, blue );
	
	red   = settings.readDoubleEntry( APP_KEY + "DiffuseRed"  , 0.70 );
	green = settings.readDoubleEntry( APP_KEY + "DiffuseGreen", 0.70 );
	blue  = settings.readDoubleEntry( APP_KEY + "DiffuseBlue" , 0.70 );
	colorForm -> setSliderDiffuseValues( red, green, blue );
	
	red   = settings.readDoubleEntry( APP_KEY + "SpecularRed"  , 0.25 );
	green = settings.readDoubleEntry( APP_KEY + "SpecularGreen", 0.25 );
	blue  = settings.readDoubleEntry( APP_KEY + "SpecularBlue" , 0.25 );
	shiny = settings.readNumEntry( APP_KEY + "SpecularShiny", 128 );
	colorForm -> setSliderSpecularValues( red, green, blue, shiny );
	
	red   = settings.readDoubleEntry( APP_KEY + "BackgroundRed"  , 1.0 );
	green = settings.readDoubleEntry( APP_KEY + "BackgroundGreen", 1.0 );
	blue  = settings.readDoubleEntry( APP_KEY + "BackgroundBlue" , 1.0 );
	colorForm -> setSliderBackgroundValues( red, green, blue );
	
	red   = settings.readDoubleEntry( APP_KEY + "BoundingBoxRed"  , 0.0 );
	green = settings.readDoubleEntry( APP_KEY + "BoundingBoxGreen", 0.0 );
	blue  = settings.readDoubleEntry( APP_KEY + "BoundingBoxBlue" , 0.0 );
	colorForm -> setSliderBoundingBoxValues( red, green, blue );
	
	QString x = "", y = "", z = "";
	bool d = false;
	x = settings.readEntry( APP_KEY + "LightPosX"  , "0.0" );
	y = settings.readEntry( APP_KEY + "LightPosY"  , "0.0" );
	z = settings.readEntry( APP_KEY + "LightPosZ"  , "1.0" );
	d = settings.readBoolEntry( APP_KEY + "Directional", true );
	colorForm -> setLightPosition( x, y, z, d );
    }
    
    colorForm -> connectSlots();
    colorForm -> sliderValueChanged();
    
    QSettings settings;
    settings.insertSearchPath( QSettings::Windows, WINDOWS_REGISTRY );
    
    spinBox_zoom     -> setValue( settings.readNumEntry( APP_KEY + "Zoom" , 16 ) );
    spinBox_lineSize -> setValue( settings.readNumEntry( APP_KEY + "LineSize" , 0 ) );
    
    showCalled = true;
    
    //modelsForm -> commitChanges();
    
    statusBar() -> message( "Welcome to QMGA.", 3000 );
    //cout << "MainForm::show mainform end" << endl;
}

//-------------------------------------------------------------------------
//------------- newInputFile
//-------------------------------------------------------------------------
/*!
 *  Function that can always be called, when the configuration file changes.
 *  It sets the class member that holds actual configuration file
 *  Reloads new config file.
 *  Rebuilds the scenegraph to make changes visible.
 *  \param newFile String with the name of the new file (including path information).
 *  \author Adrian Gabriel
 *  \date Dec 2005
 */
bool MainForm::newInputFile( QString newFile, bool reloadSameName )
{    
    //cout << "MainForm::newInputFile beg" << endl;
    if( (cnfFile != newFile || reloadSameName) && newFile != "" )
    {
	//cnf -> setColorScheme( "director" );
	if( (cnf -> reloadCnfFile( newFile )) == true )
	{
	    //toggleFold();
	    cnf -> setShowFolded( action_toggleFold->isOn() );
	    
	    cnfFile = newFile;
	    if( !blockCnfChanged ) 
	    {
		emit cnfChanged(cnf);
		updateHistory( newFile );
	    }
	    
	    buildScene( true, true );
	    
	    //cout << "----------------------------------" << endl;
	    forceChangeColorization = true;
	    changeColorisation();
	    forceChangeColorization = false;
	    
	    glWindow -> repaint();
	    lineEditFileOpen -> setText( newFile );
	    statusBar() -> message( "loaded file: " + newFile, 3000 );	    
	    //cout << "MainForm::newInputFile end 0" << endl;
	    return( true );
	}
	else
	{
	    statusBar()->message( "beware! : error while reloading cnf file: " + newFile, 3000 );
	}
    }
    //cout << "MainForm::newInputFile end final" << endl;
    return( false );  
}

//-------------------------------------------------------------------------
//------------- rescaleScene
//-------------------------------------------------------------------------
/*!
 *  Changes the scale of the axes.
 *  Rerenderes the gl window.
 *  \author Adrian Gabriel
 *  \date Dec 2005
 */
void MainForm::rescaleScene()
{
    //cout << "MainForm::rescaleScene beg" << endl;
    updateModelsVector();
    
    if( forceChangeColorization == true && showCalled == true )
    {
	//forceChangeColorization = true;
	changeColorisation();
	//forceChangeColorization = false;
    }
    
    int selectedQual = comboBox_quality -> currentItem();
    // Warning, TODO: scaleX,Y,Z values have to be divided by two before beeing passed to setModelParams
    
    switch( selectedQual )
    {
    case 0:
	glWindow -> setModelParams( 6, 1, 4, 0, 5, *models );
	break;
    case 1:
	glWindow -> setModelParams( 8, 2, 5, 1, 5, *models );
	break;
    case 2:
	glWindow -> setModelParams( 12, 4, 7, 2, 5, *models );
	break;
    case 3:
	glWindow -> setModelParams( 24, 8, 14, 4, 5, *models );
	break;
    case 4:
	glWindow -> setModelParams( 48, 12, 23, 6, 5, *models );
	break;
    }
    //cout << "MainForm::rescaleScene end" << endl;
}

//-------------------------------------------------------------------------
//------------- initializeScene
//-------------------------------------------------------------------------
/*!
 *  Sets up the scene for the first time.
 *  \author Adrian Gabriel
 *  \date Dec 2005
 */
void MainForm::initializeScene()
{
    //cout << "MainForm::initializeScene beg" << endl;
    
    string colorschemeTmp;
    if     ( action_useUserDefined  -> isOn() ) {  colorschemeTmp = "userDefined"; }
    else if( action_useDirector     -> isOn() ) {  colorschemeTmp = "director";    }
    else if( action_useColorByModel -> isOn() ) {  colorschemeTmp = "byModel";     }
    
    string tmpFile = cnfFile;
    cnf = new CnfFile( tmpFile, comboBox_fileType->currentItem(), colorschemeTmp, colorMap );
    cnfFile = tmpFile;
    lineEditFileOpen -> setText( cnfFile );
    updateHistory( cnfFile );
    //cnf -> setUseUserDefinedDirector( action_useUserDefined -> isOn() );
    
    setColorMap();
    
    connect( action_toggleFold, SIGNAL(toggled(bool)), this, SLOT(toggleFold()) );
    cnf -> setShowFolded( action_toggleFold->isOn() );
    //toggleFold();
    
    
    changeColorisation();
    buildScene( true, false );
    
    //cout << "MainForm::initializeScene end" << endl;
}

//-------------------------------------------------------------------------
//------------- buildScene
//-------------------------------------------------------------------------
/*!
 *  This function builds the subscenegraph that holds the actual molecule data.
 *  \param resetDistance Bool to tell the render engin to recalculate the zoom value or not.
 *  \author Adrian Gabriel 
 *  \date Dec 2005
 */
void MainForm::buildScene( bool resetDistance, bool blockRepaint )
{
    //cout << "MainForm::buildScene beg" << endl;
    //cout << "MainForm::buildScene blockRepaint: " << blockRepaint << endl;
    
    if( cnf == 0 ) { return; }
    QString tmp = "";
    tmp = tmp.setNum ( cnf -> getDirectorX(), 'f', 2 );
    lineEdit_directorX -> setText( tmp );
    tmp = tmp.setNum ( cnf -> getDirectorY(), 'f', 2 );
    lineEdit_directorY->setText( tmp );
    tmp = tmp.setNum ( cnf -> getDirectorZ(), 'f', 2 );
    lineEdit_directorZ -> setText( tmp );
    
    //    cout << "director:\n" 
    //	    << "x: "  << cnf -> getDirectorX() 
    //	    << " y: " << cnf -> getDirectorY()
    //	    << " z: " << cnf -> getDirectorZ() 
    //	    << " norm: " << sqrt( pow(cnf->getDirectorX(),2) + pow(cnf->getDirectorY(),2) + pow(cnf->getDirectorZ(),2) ) << endl;
    
    numMolecules = cnf -> getNumberOfMolecules();
    if( numMolecules > 0 )
    {
	uint numTypesCnf = cnf -> getNumberOfTypes();
	uint numTypesVec = models -> size();
	if( numTypesCnf > numTypesVec )
	{
	    addDummyModels( numTypesCnf - numTypesVec );
	    
	    numTypesVec = models -> size();
	    if( numTypesCnf != numTypesVec ) { cerr << "ERROR: Wrong number of dummy models added! Check code in MainForm::buildScene()" << endl; exit(0); }
	    
	    modelsForm -> updateModelsVector();
	    rescaleScene();
	}
	paintScene( resetDistance, blockRepaint );
    }
    
    //cout << "MainForm::buildScene end" << endl;
}

//-------------------------------------------------------------------------
//------------- paintScene
//-------------------------------------------------------------------------
/*!
 *  This function sends the actual object data to the render area.
 *  Be sure that all data vectors are filled.
 *  \param resetDistance Bool to tell the render engin to recalculate the zoom value or not.
 *  \author Adrian Gabriel 
 *  \date Dec 2005
 */
void MainForm::paintScene( bool resetDistance, bool blockRepaint )
{
    //cout << "MainForm::paintScene beg" << endl;
    //cout << "MainForm::paintScene blockRepaint: " << blockRepaint << endl;
    
    fillModelVectors();
    glWindow -> setModels( middle, rot, color, modelType, cnf->getBoxX(), cnf->getBoxY(), cnf->getBoxZ() );
    
    vector<vector<float> > tmpBox = cnf->getBoundingBoxCoordinates();
    if( action_translate -> isOn() )
    {
	for( uint i = 0; i<tmpBox.size(); ++i )
	{
	    tmpBox.at(i).at(0) += float(spinBox_translateX->value()) / 10.0; 
	    tmpBox.at(i).at(1) += float(spinBox_translateY->value()) / 10.0; 
	    tmpBox.at(i).at(2) += float(spinBox_translateZ->value()) / 10.0; 
	    
	}
    }
    
    
    glWindow -> setBoundingBox( tmpBox, resetDistance );
    //glWindow -> setBoundingBox( cnf->getBoundingBoxCoordinates(), resetDistance );
    
    if( blockRepaint == false ) 
    {
	glWindow -> repaint(); 
    }
    //cout << "MainForm::paintScene end" << endl;
}

//-------------------------------------------------------------------------
//------------- setColorMap
//-------------------------------------------------------------------------
/*!
 *  This function fills a vector with RBG values and sends it to the render engine.
 *  \author Adrian Gabriel 
 *  \date Dec 2005
 */
void MainForm::setColorMap()
{
    //cout << "MainForm::setColorMap beg" << endl;
    vector<float *> *map = new vector <float *>();
    for( int i = 0; i < cnf->getNumberOfColorsInMap(); i++ ) 
    {
	float *tmp = new float[3];
	tmp[0] = float( cnf -> getRedAt(i)  /255.0 );
	tmp[1] = float( cnf -> getGreenAt(i)/255.0 );
	tmp[2] = float( cnf -> getBlueAt(i) /255.0 );
	map -> push_back(tmp);
    }
    glWindow -> setColorMap( map );
    //cout << "MainForm::setColorMap end" << endl;
}

//-------------------------------------------------------------------------
//------------- fillModelVectors
//-------------------------------------------------------------------------
/*!
 *  This function processes position, orientation and color information and sends it to the render engine.
 *  \author Adrian Gabriel 
 *  \date Dec 2005
 */
void MainForm::fillModelVectors()
{
    //cout << "MainForm::fillModelVectors beg" << endl;
    int old = middle -> size();
    
    if( numMolecules != old )
    {
	modelType -> resize(numMolecules);
	if( numMolecules < old )
	{
	    for( int i = numMolecules; i<old; i++ )
	    {
		delete middle -> at(i); middle -> at(i) = 0;
		delete rot    -> at(i); rot    -> at(i) = 0;
		delete color  -> at(i); color  -> at(i) = 0;
	    }
	    middle -> resize(numMolecules);
	    rot    -> resize(numMolecules);
	    color  -> resize(numMolecules);
	}
	else
	{
	    for( int i = 0; i < numMolecules-old; i++ ) 
	    {
		middle -> push_back( new vector<float>(3) );
		rot    -> push_back( new vector<float>(4) );
		color  -> push_back( new vector<float>(3) );
	    }
	}
    }
    
    bool fold = cnf -> getShowFolded();
    if( fold ) { cnf -> foldMoleculesToBoundingBox(); } 
    
    MoleculeBiax *mc = NULL;
    vector<double> r;
    for( int i = 0; i < numMolecules; i++ ) 
    {
	mc = cnf -> getMolecule(i);
	
	if( fold )
	{
	    middle->at(i)->at(0) = (float) ( mc -> getPositionFoldedX() );
	    middle->at(i)->at(1) = (float) ( mc -> getPositionFoldedY() );
	    middle->at(i)->at(2) = (float) ( mc -> getPositionFoldedZ() );	    
	}
	else
	{
	    middle->at(i)->at(0) = (float) ( mc -> getPositionX() );
	    middle->at(i)->at(1) = (float) ( mc -> getPositionY() );
	    middle->at(i)->at(2) = (float) ( mc -> getPositionZ() );
	}
	
	if( action_translate -> isOn() )
	{
	    middle->at(i)->at(0) += float(spinBox_translateX->value()) / 10.0; 
	    middle->at(i)->at(1) += float(spinBox_translateY->value()) / 10.0; 
	    middle->at(i)->at(2) += float(spinBox_translateZ->value()) / 10.0; 
	}
	
	
	color->at(i)->at(0) = (float) ( mc -> getRed()/255.0f   );
	color->at(i)->at(1) = (float) ( mc -> getGreen()/255.0f );
	color->at(i)->at(2) = (float) ( mc -> getBlue()/255.0f  );
	
	// INITIAL OPENGL AXES: camera looks down -Z with Y upwards 
	r = mc->QuaternionToAxisAngle();
	
	rot->at(i)->at(0) = r[0]; 
	rot->at(i)->at(1) = r[1]; 
	rot->at(i)->at(2) = r[2];
	rot->at(i)->at(3) = r[3]; 
	
	if( !action_toggleObjectsChangable -> isOn() ) { modelType -> at(i) = mc -> getType(); }
	else if( !action_toggleObjects -> isOn() )     { modelType -> at(i) = 0;               }
	else                                           { modelType -> at(i) = 1;               }
	
	// Warning: be sure to set enough modelTypes with setModelParams to handle the different types!
	// e.g.: by calling this->rescaleScene()
    }
    //cout << "MainForm::fillModelVectors end" << endl;
}


//-------------------------------------------------------------------------
//------------- translate
//-------------------------------------------------------------------------
void MainForm::translate()
{
    //cout << "MainForm::translate beg" << endl;
    paintScene( false, false );
    //cout << "MainForm::translate end" << endl;
}

//-------------------------------------------------------------------------
//------------- toggleObjectsChangable
//-------------------------------------------------------------------------
/*!
 *  This function enables/disables the option to impose the model type via the gui.
 *  \author Adrian Gabriel 
 *  \date Dec 2005
 */
void MainForm::toggleObjectsChangable()
{
    //cout << "MainForm::toggleObjectsChangable beg" << endl;
    switch( action_toggleObjectsChangable->isOn() )
    {
    case true: statusBar()->message( "representation change enabled.", 3000 );
	action_toggleObjects->setEnabled(true);
	break;
    case false: statusBar()->message( "representation change disabled..", 3000 );
	action_toggleObjects->setEnabled(false);
	break;
    }
    paintScene( false, false );    
    //cout << "MainForm::toggleObjectsChangable end" << endl;
}

//-------------------------------------------------------------------------
//------------- toggleObjects
//-------------------------------------------------------------------------
/*!
 *  This function imposes the use of model 1 or model 2 on all objects.
 *  \author Adrian Gabriel 
 *  \date Dec 2005
 */
void MainForm::toggleObjects()
{
    //cout << "MainForm::toggleObjects beg" << endl;
    switch( action_toggleObjects->isOn() )
    {
    case false: statusBar()->message( "switch to Model-2 representation.", 3000 );
	break;
    case true:  statusBar()->message( "switch to Model-1 representation.", 3000 );
	break;
    }
    paintScene( false, false );
    //cout << "MainForm::toggleObjects end" << endl;
}

//-------------------------------------------------------------------------
//------------- toggleFold
//-------------------------------------------------------------------------
/*!
 *  Enables folding for configurations that are unfolded.
 *  Rerenders the scene.
 *  \author Adrian Gabriel 
 *  \date Dec 2005
 */
void MainForm::toggleFold()
{
    //cout << "MainForm::toggleFold beg" << endl;
    switch( action_toggleFold->isOn() )
    {
    case true:  statusBar()->message( "switch to folded mode.", 3000 );
	break;
    case false: statusBar()->message( "switch to unfolded mode.", 3000 );
	break;
    }
    cnf -> setShowFolded( action_toggleFold->isOn() );
    buildScene( false, false );
    //cout << "MainForm::toggleFold end" << endl;
}

//-------------------------------------------------------------------------
//------------- changeColorisation
//-------------------------------------------------------------------------
/*!
 *  Changes the color coding between user defined vector and calculated director.
 *  Rerenders the scene.
 *  \author Adrian Gabriel 
 *  \date Dec 2005
 */
void MainForm::changeColorisation()
{
    //cout << "MainForm::changeColorisation beg" << endl;
    
    //    if( !showCalled ) // this function must not be called before Mainform::show() was executed !!!
    //    {
    //	cout << "MainForm::changeColorisation end 2" << endl;
    //	return;
    //    }
    
    static float xOld=0,yOld=0,zOld=0;
    static float x=0,y=0,z=0;
    
    if( action_useUserDefined -> isOn() )
    {
	statusBar()->message( "switch to user-defined color axis.", 3000 );
	cnf -> setColorScheme( "userDefined" );
	x = lineEdit_userX->text().toFloat();
	y = lineEdit_userY->text().toFloat();
	z = lineEdit_userZ->text().toFloat();
	cnf -> setUserDefinedDirector( x, y, z );
    }
    else if( action_useDirector -> isOn() )
    {
	statusBar()->message( "switch to director as color axis.", 3000 );
	cnf -> setColorScheme( "director" );
	x = cnf->getDirectorX();
	y = cnf->getDirectorY();
	z = cnf->getDirectorZ();
    }
    else if( action_useColorByModel -> isOn() )
    {
	statusBar()->message( "switch to model type for colorization.", 3000 );
	cnf -> setColorScheme( "byModel" );
	x=0;
	y=0;
	z=0;
    }
    
    if( x != xOld || y != yOld || z != zOld || action_useColorByModel -> isOn() || forceChangeColorization )
    {
	xOld = x;
	yOld = y;
	zOld = z;
	cnf -> colorizeMolecules( models );
	
	//cout << "color vector size: " << color -> size() << endl;
	Molecule *mc = NULL;
	for( unsigned int i = 0; i < color -> size(); i++ )
	{
	    mc = cnf->getMolecule(i);
	    color->at(i)->at(0) = float( (mc -> getRed()  /255.0f) );
	    color->at(i)->at(1) = float( (mc -> getGreen()/255.0f) );
	    color->at(i)->at(2) = float( (mc -> getBlue() /255.0f) );
	}	
	glWindow->repaint();
    }
    
    //cout << "MainForm::changeColorisation end 1" << endl;
}

//-------------------------------------------------------------------------
//------------- lineEditFileOpenReturnPressed
//-------------------------------------------------------------------------
/*!
 *  Slot to update the scene, when a new configuration file was entered by hand into the line edit.
 *  It is called, when enter is pressed (from inside the line edit, of course)
 *  \author Adrian Gabriel 
 *  \date Dec 2005
 */
void MainForm::lineEditFileOpenReturnPressed()
{
    //cout << "MainForm::lineEditFileOpenReturnPressed beg" << endl;
    QString newFile = parseFilename(lineEditFileOpen -> text() );
    if( newInputFile( newFile, DONTRELOADSAME ) == false )
    {
	statusBar() -> message( "ERROR: Could not load file: " + newFile, 8000 );
    }
    //cout << "MainForm::lineEditFileOpenReturnPressed end" << endl;
}

//-------------------------------------------------------------------------
//------------- updateAngleBoxes
//-------------------------------------------------------------------------
/*!
 *  Sets the values of the spin boxes that hold the angle information of the rotation.
 *  \param x Angle of the rotation around x-Axis.
 *  \param y Angle of the rotation around y-Axis.
 *  \param z Angle of the rotation around z-axis.
 *  \author Adrian Gabriel 
 *  \date Dec 2005
 */
void MainForm::updateAngleBoxes( int x, int y, int z )
{
    //cout << "MainForm::updateAngleBoxes beg" << endl;
    blockUpdate = true;
    spinBox_rotx -> setValue(x);
    spinBox_roty -> setValue(y);
    spinBox_rotz -> setValue(z);
    blockUpdate = false;
    //cout << "MainForm::updateAngleBoxes end" << endl;
}

//-------------------------------------------------------------------------
//------------- updateAlpha
//-------------------------------------------------------------------------
/*!
 *  Sets the value of the spin box that hold the angle information of the rotation around the x-axis.
 *  \param val Angle of the rotation around x-Axis.
 *  \author Adrian Gabriel 
 *  \date Dec 2005
 */
void MainForm::updateAlpha( int val )
{
    //cout << "MainForm::updateAlpha beg" << endl;
    blockUpdate = true;
    spinBox_rotx -> setValue(val);
    blockUpdate = false;
    //cout << "MainForm::updateAlpha end" << endl;
}

//-------------------------------------------------------------------------
//------------- updateBeta
//-------------------------------------------------------------------------
/*!
 *  Sets the value of the spin box that hold the angle information of the rotation around the y-axis.
 *  \param val Angle of the rotation around y-Axis.
 *  \author Adrian Gabriel 
 *  \date Dec 2005
 */
void MainForm::updateBeta( int val )
{
    //cout << "MainForm::updateBeta beg" << endl;
    blockUpdate = true;
    spinBox_roty -> setValue(val);
    blockUpdate = false;
    //cout << "MainForm::updateBeta end" << endl;
}

//-------------------------------------------------------------------------
//------------- updateGamma
//-------------------------------------------------------------------------
/*!
 *  Sets the value of the spin box that hold the angle information of the rotation around the z-axis.
 *  \param val Angle of the rotation around z-Axis.
 *  \author Adrian Gabriel 
 *  \date Dec 2005
 */
void MainForm::updateGamma( int val )
{
    //cout << "MainForm::updateGamma beg" << endl;
    blockUpdate = true;
    spinBox_rotz -> setValue(val);
    blockUpdate = false;
    //cout << "MainForm::updateGamma end" << endl;
}

//-------------------------------------------------------------------------
//------------- updateAngleBoxesRenderer
//-------------------------------------------------------------------------
/*!
 *  Sends updated render information to the render area.
 *  \author Adrian Gabriel 
 *  \date Dec 2005
 */
void MainForm::updateAngleBoxesRenderer()
{
    //cout << "MainForm::updateAngleBoxesRenderer beg" << endl;
    if(!blockUpdate)
    {
	glWindow->setAngles( spinBox_rotx -> value(), spinBox_roty -> value(), spinBox_rotz -> value() );
    }
    //cout << "MainForm::updateAngleBoxesRenderer end" << endl;
}

//-------------------------------------------------------------------------
//------------- toggleFullRender
//-------------------------------------------------------------------------
/*!
 *  Toggles between full and stick render mode.
 *  \param state true for full render mode.
 *  \author Adrian Gabriel 
 *  \date Dec 2005
 */
void MainForm::toggleFullRender( bool state )
{
    //cout << "MainForm::toggleFullRender beg" << endl;
    switch(state)
    {
    case true: statusBar()->message( "switch to full render mode.", 3000 );
	break;
    case false: statusBar()->message( "switch to stick render mode.", 3000 );
	break;
    }
    glWindow->setFullRender( !state );
    //cout << "MainForm::toggleFullRender end" << endl;
}

//-------------------------------------------------------------------------
//------------- changeZoom
//-------------------------------------------------------------------------
/*!
 *  Sends updated zoom information to the render area.
 *  \author Adrian Gabriel 
 *  \date Dec 2005
 */
void MainForm::changeZoom()
{
    //cout << "MainForm::changeZoom beg" << endl;
    if(!blockUpdate)
    {
	glWindow -> changeZoomAbsolute( spinBox_zoom -> value() );
    }
    //cout << "MainForm::changeZoom end" << endl;
}

//-------------------------------------------------------------------------
//------------- updateZoom
//-------------------------------------------------------------------------
/*!
 *  Sets the value of the spin box holding zoom information.
 *  \param val The new value.
 *  \author Adrian Gabriel 
 *  \date Dec 2005
 */
void MainForm::updateZoom( int val )
{
    //cout << "MainForm::updateZoom beg" << endl;
    blockUpdate = true;
    spinBox_zoom -> setValue( val );
    blockUpdate = false;
    //cout << "MainForm::updateZoom end" << endl;
}

//-------------------------------------------------------------------------
//------------- toggleColormap
//-------------------------------------------------------------------------
/*!
 *  Switches the colormap representation on and off.
 *  \param state true to show the colormap.
 *  \author Adrian Gabriel 
 *  \date Dec 2005
 */
void MainForm::toggleColormap( bool state )
{
    //cout << "MainForm::toggleColormap beg" << endl;
    switch( state )
    {
    case true: statusBar()->message( "switch colormap on.", 3000 );
	break;
    case false: statusBar()->message( "switch colormap off.", 3000 );
	break;
    }
    glWindow -> setDrawColorMap( state );
    //cout << "MainForm::toggleColormap end" << endl;
}

//-------------------------------------------------------------------------
//------------- toggleAxes
//-------------------------------------------------------------------------
/*!
 *  Switches the coordinate system representation on and off.
 *  \param state true to show the coordinate system.
 *  \author Adrian Gabriel 
 *  \date Dec 2005
 */
void MainForm::toggleAxes( bool state )
{
    //cout << "MainForm::toggleAxes beg" << endl;
    switch( state )
    {
    case true: statusBar()->message( "switch axes on.", 3000 );
	break;
    case false: statusBar()->message( "switch axes off.", 3000 );
	break;
    }
    glWindow -> setDrawAxis( state );
    //cout << "MainForm::toggleAxes end" << endl;
}

//-------------------------------------------------------------------------
//------------- toggleBoundingBox
//-------------------------------------------------------------------------
/*!
 *  Switches the bounding box representation on and off.
 *  \param state true to show the bounding box.
 *  \author Adrian Gabriel 
 *  \date Apr 2007
 */
void MainForm::toggleBoundingBox( bool state )
{
    //cout << "MainForm::toggleBoundingBox beg" << endl;
    switch( state )
    {
    case true: statusBar()->message( "switch bounding box on.", 3000 );
	break;
    case false: statusBar()->message( "switch bounding box off.", 3000 );
	break;
    }
    glWindow -> setDrawBoundingBox( state );
    //cout << "MainForm::toggleBoundingBox end" << endl;
}

//-------------------------------------------------------------------------
//------------- parseFilename
//-------------------------------------------------------------------------
/*!
 *  
 *  \author Adrian Gabriel 
 *  \date Apr 2007
 */
QString MainForm::parseFilename( QString filename )
{
    //cout << "MainForm::parseFilename beg" << endl;
    if( filename.startsWith("SSH|") == true || filename.startsWith("SCP|") == true )
    {
	QString user = filename.section( '|', 1, 1 );
	QString host = filename.section( '|', 2, 2 );
	QString path = filename.section( '|', 3, 3 );
	QString name = path;
	while( name.endsWith("/") ) { name.truncate(name.length()-1); }
	name = name.section( '/', -1, -1 );
	
	//cout << "test: " << user << " " << host << " " << path << " " << name << endl;
	
	QProcess scp( QString("scp") );
	scp.addArgument( user + "@" + host + ":" + path );
	scp.addArgument( "./scp-tmp/" );
	
	//cout << "try" << endl;
	scp.start();
	//cout << "wait" << endl;
	int count=0;
	while( scp.isRunning() == true ) { count++;  sleep(1); }
	//cout << "wait end count = " << count << endl;
	
	if( scp.normalExit() == true )
	{
	    //cout << "exitStatus: " << scp.exitStatus() << endl;
	    if( scp.exitStatus() == 0 )
	    {
		filename = "./scp-tmp/" + name;
		emit newTmpFile();
	    }
	    else { cerr << "scp copy failed!" << endl; }
	}
	else { cerr << "scp copy failed!" << endl; }
	//cout << "done" << endl;
	//cout <<  "./scp-tmp/" + name << endl;
    }
    
    //cout << "MainForm::parseFilename end" << endl;
    return(filename);
}

//-------------------------------------------------------------------------
//------------- setButtonColor
//-------------------------------------------------------------------------
/*!
 *  Openes a color pick dialoge and assignes the chosen color to a button, a line edit and a dial.
 *  These three objects have to be passed as parameters.
 *  \param button true to show the coordinate system.
 *  \param lineEdit true to show the coordinate system.
 *  \param dial true to show the coordinate system.
 *  \author Adrian Gabriel 
 *  \date Dec 2005
 */
void MainForm::setButtonColor( QPushButton *button, QLineEdit *lineEdit, QDial *dial )
{
    //cout << "MainForm::setButtonColor beg" << endl;
    QColor c, x, y, z;
    c = QColorDialog::getColor( c, this );
    if ( c.isValid() ) 
    {
	button   -> setPaletteBackgroundColor ( c );
	lineEdit -> setPaletteBackgroundColor ( c );
	dial     -> setPaletteForegroundColor ( c );
	x = pushButton_x -> paletteBackgroundColor();
	y = pushButton_y -> paletteBackgroundColor();
	z = pushButton_z -> paletteBackgroundColor();
	glWindow->setAxisColors( float(x.red())/255.0, float(x.green())/255.0, float(x.blue())/255.0,
				 float(y.red())/255.0, float(y.green())/255.0, float(y.blue())/255.0,
				 float(z.red())/255.0, float(z.green())/255.0, float(z.blue())/255.0 );
    }    
    //cout << "MainForm::setButtonColor end" << endl;
}

//-------------------------------------------------------------------------
//------------- colorButtonX
//-------------------------------------------------------------------------
/*!
 *  Calles setButtonColor() to set the color of x-axis related objects.
 *  \author Adrian Gabriel 
 *  \date Dec 2005
 */
void MainForm::colorButtonX()
{
    //cout << "MainForm::colorButtonX beg" << endl;
    setButtonColor( pushButton_x, lineEdit_rotX, dial_rotX );
    //cout << "MainForm::colorButtonX end" << endl;
}

//-------------------------------------------------------------------------
//------------- colorButtonY
//-------------------------------------------------------------------------
/*!
 *  Calles setButtonColor() to set the color of y-axis related objects.
 *  \author Adrian Gabriel 
 *  \date Dec 2005
 */
void MainForm::colorButtonY()
{
    //cout << "MainForm::colorButtonY beg" << endl;
    setButtonColor( pushButton_y, lineEdit_rotY, dial_rotY );
    //cout << "MainForm::colorButtonY end" << endl;
}

//-------------------------------------------------------------------------
//------------- colorButtonZ
//-------------------------------------------------------------------------
/*!
 *  Calles setButtonColor() to set the color of z-axis related objects.
 *  \author Adrian Gabriel 
 *  \date Dec 2005
 */
void MainForm::colorButtonZ()
{
    //cout << "MainForm::colorButtonZ beg" << endl;
    setButtonColor( pushButton_z, lineEdit_rotZ, dial_rotZ );
    //cout << "MainForm::colorButtonZ end" << endl;
}

//-------------------------------------------------------------------------
//------------- toggleOptimized
//-------------------------------------------------------------------------
/*!
 *  Switches the optimization option on and off.
 *  \param state true to use optimization (see manual).
 *  \author Adrian Gabriel 
 *  \date Dec 2005
 */
void MainForm::toggleOptimized( bool state )
{
    //cout << "MainForm::toggleOptimized beg" << endl;
    switch( state )
    {
    case true: statusBar()->message( "switch optimization for many objects on.", 3000 );
	break;
    case false: statusBar()->message( "switch optimization for many objects off.", 3000 );
	break;
    }
    glWindow->setOptimized(state);
    //cout << "MainForm::toggleOptimized end" << endl;
}

//-------------------------------------------------------------------------
//------------- toggleLOD
//-------------------------------------------------------------------------
/*!
 *  Switches the auto level of detail option on and off.
 *  \param state true to use auto lod (see manual).
 *  \author Adrian Gabriel 
 *  \date Apr 2007
 */
void MainForm::toggleLOD( bool state )
{
    //cout << "MainForm::toggleLOD beg" << endl;
    switch( state )
    {
    case true: statusBar()->message( "switch auto level of detail for many objects on.", 3000 );
	break;
    case false: statusBar()->message( "switch auto level of detail for many objects off.", 3000 );
	break;
    }
    glWindow->setLOD(state);
    //cout << "MainForm::toggleLOD end" << endl;
}

//-------------------------------------------------------------------------
//------------- toggleSlice
//-------------------------------------------------------------------------
/*!
 *  Switches sliced view on and off.
 *  \param state true to use sliced view.
 *  \author Adrian Gabriel 
 *  \date Dec 2005
 */
void MainForm::toggleSlice( bool state )
{
    //cout << "MainForm::toggleSlice beg" << endl;
    switch( state )
    {
    case true: statusBar() -> message( "switch sliced view on.", 3000 );
	break;
    case false: statusBar() -> message( "switch sliced view off.", 3000 );
	break;
    }
    glWindow -> setDrawSlices( state );
    //cout << "MainForm::toggleSlice end" << endl;
}

//-------------------------------------------------------------------------
//------------- toggleSliceAction
//-------------------------------------------------------------------------
/*!
 *  Toggles slice action on and off. In effect turns sliced view on and off.
 *  \author Adrian Gabriel 
 *  \date Dec 2005
 */
void MainForm::toggleSliceAction()
{
    //cout << "MainForm::toggleSliceAction beg" << endl;
    action_toggleSlice -> toggle();
    //cout << "MainForm::toggleSliceAction end" << endl;
}

//-------------------------------------------------------------------------
//------------- rotX
//-------------------------------------------------------------------------
/*!
 *  Rotation around the x-axis with a relative angle.
 *  \param newVal Relative rotation angle.
 *  \author Adrian Gabriel 
 *  \date Dec 2005
 */
void MainForm::rotX( int newVal )
{
    //cout << "MainForm::rotX beg" << endl;
    static int oldValue = 0;
    glWindow->rotateXAxis( oldValue - newVal );
    oldValue = newVal;    
    //cout << "MainForm::rotX end" << endl;
}

//-------------------------------------------------------------------------
//------------- rotY
//-------------------------------------------------------------------------
/*!
 *  Rotation around the y-axis with a relative angle.
 *  \param newVal Relative rotation angle.
 *  \author Adrian Gabriel 
 *  \date Dec 2005
 */
void MainForm::rotY( int newVal )
{
    //cout << "MainForm::rotY beg" << endl;
    static int oldValue = 0;
    glWindow->rotateYAxis( oldValue - newVal );
    oldValue = newVal;
    //cout << "MainForm::rotY end" << endl;
}

//-------------------------------------------------------------------------
//------------- rotZ
//-------------------------------------------------------------------------
/*!
 *  Rotation around the z-axis with a relative angle.
 *  \param newVal Relative rotation angle.
 *  \author Adrian Gabriel 
 *  \date Dec 2005
 */
void MainForm::rotZ( int newVal )
{
    //cout << "MainForm::rotZ beg" << endl;
    static int oldValue = 0;
    glWindow->rotateZAxis( oldValue - newVal );
    oldValue = newVal;
    //cout << "MainForm::rotZ end" << endl;
}

//-------------------------------------------------------------------------
//------------- rotXEnter
//-------------------------------------------------------------------------
/*!
 *  Slot to rotate around x-axis by a certain angle.
 *  \author Adrian Gabriel 
 *  \date Dec 2005
 */
void MainForm::rotXEnter()
{
    //cout << "MainForm::rotXEnter beg" << endl;
    statusBar()->message( "rotate around x-axis.", 3000 );
    
    glWindow->rotateXAxis( lineEdit_rotX->text().stripWhiteSpace().toFloat() );
    //cout << "MainForm::rotXEnter end" << endl;
}

//-------------------------------------------------------------------------
//------------- rotYEnter
//-------------------------------------------------------------------------
/*!
 *  Slot to rotate around y-axis by a certain angle.
 *  \author Adrian Gabriel 
 *  \date Dec 2005
 */
void MainForm::rotYEnter()
{
    //cout << "MainForm::rotYEnter beg" << endl;
    statusBar()->message( "rotate around y-axis.", 3000 );
    
    glWindow->rotateYAxis( lineEdit_rotY->text().stripWhiteSpace().toFloat() );
    //cout << "MainForm::rotYEnter end" << endl;
}

//-------------------------------------------------------------------------
//------------- rotZEnter
//-------------------------------------------------------------------------
/*!
 *  Slot to rotate around z-axis by a certain angle.
 *  \author Adrian Gabriel 
 *  \date Dec 2005
 */
void MainForm::rotZEnter()
{
    //cout << "MainForm::rotZEnter beg" << endl;
    statusBar()->message( "rotate around z-axis.", 3000 );
    
    glWindow->rotateZAxis( lineEdit_rotZ->text().stripWhiteSpace().toFloat() );
    //cout << "MainForm::rotZEnter end" << endl;
}

//-------------------------------------------------------------------------
//------------- rotAngleBoxesChanged
//-------------------------------------------------------------------------
/*!
 *  Updates new values in the angle spin boxes to the render area.
 *  \author Adrian Gabriel 
 *  \date Dec 2005
 */
void MainForm::rotAngleBoxesChanged()
{
    //cout << "MainForm::rotAngleBoxesChanged beg" << endl;
    if(!blockUpdate) 
    {
	unsigned int rx, ry, rz;
	rx = spinBox_rotx -> value();
	ry = spinBox_roty -> value();
	rz = spinBox_rotz -> value();
	glWindow->setAngles( rx, ry, rz );
    }
    //cout << "MainForm::rotAngleBoxesChanged end" << endl;
}

//-------------------------------------------------------------------------
//------------- togglePixel
//-------------------------------------------------------------------------
/*!
 *  Toggles which of the two pixel line edits is readonly.
 *  \author Adrian Gabriel 
 *  \date Dec 2005
 */
void MainForm::togglePixel()
{
    //cout << "MainForm::togglePixel beg" << endl;
    if( lineEditPix2->isReadOnly() )
    {
	lineEditPix1->setReadOnly(true);
	lineEditPix2->setReadOnly(false);
	lineEditPix1->setFrameStyle( QFrame::Box | QFrame::Sunken );
	lineEditPix2->setFrameStyle( QFrame::Box | QFrame::Raised );
	lineEditPix1->setPaletteBackgroundColor( QColor(220,220,220) );
	lineEditPix2->setPaletteBackgroundColor( QColor(255,255,255) );
    }
    else
    {
	lineEditPix1->setReadOnly(false);
	lineEditPix2->setReadOnly(true);
	lineEditPix1->setFrameStyle( QFrame::Box | QFrame::Raised );
	lineEditPix2->setFrameStyle( QFrame::Box | QFrame::Sunken );	
	lineEditPix1->setPaletteBackgroundColor( QColor(255,255,255) );
	lineEditPix2->setPaletteBackgroundColor( QColor(220,220,220) );
    }
    keepAspectRatio();
    //cout << "MainForm::togglePixel end" << endl;
}

//-------------------------------------------------------------------------
//------------- keepAspectRatio
//-------------------------------------------------------------------------
/*!
 *  Calculates the needed value for the read only line edit to keep aspect ratio.
 *  \author Adrian Gabriel 
 *  \date Dec 2005
 */
void MainForm::keepAspectRatio()
{
    //cout << "MainForm::keepAspectRatio beg" << endl;
    float aspectRatio = float(glWindow->height())/float(glWindow->width());
    QString tmpNum;
    
    if( lineEditPix2->isReadOnly()  )
    {
	tmpNum.setNum( (int) ( (float) (lineEditPix1->text().toInt()*aspectRatio) ) );    
	lineEditPix2->setText(tmpNum);
    }
    else
    {
	tmpNum.setNum( (int) ( (float) (lineEditPix2->text().toInt()/aspectRatio) ) );    
	lineEditPix1->setText(tmpNum);
    }
    //cout << "MainForm::keepAspectRatio end" << endl;
}

//-------------------------------------------------------------------------
//------------- fileOpen
//-------------------------------------------------------------------------
/*!
 *  Openes a file dialog.
 *  \author Adrian Gabriel 
 *  \date Dec 2005
 */
void MainForm::fileOpen()
{
    //cout << "MainForm::fileOpen beg" << endl;
    static QString newFile = "";
    newFile = QFileDialog::getOpenFileName( newFile=="" ? "./" : newFile, "All Files (*)", this, "open file", "Choose configuration file" );   
    if( newFile != "" )
    {
	if( newInputFile( newFile, DONTRELOADSAME ) == true )
	{
	    lineEditFileOpen -> setText( newFile );
	}
	else
	{
	    statusBar() -> message( "ERROR: Could not load file: " + newFile, 3000 );
	}	
    }
    //cout << "MainForm::fileOpen end" << endl;
}

//-------------------------------------------------------------------------
//------------- fileSave
//-------------------------------------------------------------------------
/*!
 *  Function that handles screen shots.
 *  \author Adrian Gabriel 
 *  \date Dec 2005
 */
void MainForm::fileSave()
{
    //cout << "MainForm::fileSave beg" << endl;
    if( saveAsActivcated || lineEdit_snapShot -> text() == "" )
    {
	if( getSaveFilename() == false ) { return; }
	saveAsActivcated = false;
    }
    
    string basename = lineEdit_snapShot->text();
    string filenames = "";
    if( action_togglePng->isOn() )
    {
	filenames += basename + ".png\n";
    }
    if( action_toggleEps->isOn() )
    {
	filenames += basename + ".eps\n";
    }
    
    bool saveTest = true;
    if( filenames.size() == 0 || lineEditPix1->text().toInt() == 0 || lineEditPix2->text().toInt() == 0 )
    {
	saveTest = false;
    }
    
    switch( QMessageBox::information( this, "QMGA -- Information",
				      saveTest ? "Saving file(s).\n"+filenames : 
				      "Cannot save file.\n Either no format specified or size is 0.",
				      QMessageBox::Cancel|QMessageBox::Default,
				      saveTest ? QMessageBox::Ok : QMessageBox::NoButton,
				      QMessageBox::NoButton) )
    {
    case QMessageBox::Ok :
	repaint();
	this->setEnabled( false );
	saveTimer->start( 2000, TRUE ); // 2 seconds single-shot timer
	break;
    case QMessageBox::Cancel : 
	statusBar()->message( "save aborted!", 3000 ); 
	break;
    }
    //cout << "MainForm::fileSave end" << endl;
}

//-------------------------------------------------------------------------
//------------- startSnapshot
//-------------------------------------------------------------------------
void MainForm::startSnapshot()
{
    glWindow->getSnapShot( lineEditPix1->text().toInt(),
			   lineEditPix2->text().toInt(), 
			   lineEdit_snapShot->text(),
			   action_togglePng->isOn(),
			   action_toggleEps->isOn() );
    statusBar()->message( "saved "+lineEdit_snapShot->text(), 3000 );
    this->setEnabled( true );
}	

//-------------------------------------------------------------------------
//------------- getSaveFilename
//-------------------------------------------------------------------------
/*!
 *  Opens a save file dialoge.
 *  \author Adrian Gabriel 
 *  \date Dec 2005
 */
bool MainForm::getSaveFilename()
{
    //cout << "MainForm::getSaveFilename beg" << endl;
    if( !QFile::exists(lastSaveFolder) ) { lastSaveFolder = "./"; }
    static QString file = lastSaveFolder;
    file = QFileDialog::getSaveFileName( file=="" ? "./" : file,
					 "PNG (*.png *.PNG *.eps *.EPS)",
					 this,
					 "save file",
					 "Coose Filename - the extension (.png .PNG .eps and .EPS) will be truncated" );
    
    if( file.isEmpty() )
    {
	file = lastSaveFolder;
	return( false );
    }
    
    if( file.endsWith(".png") || file.endsWith(".PNG") || 
	file.endsWith(".eps") || file.endsWith(".EPS") )
    {
	file.truncate( file.length()-4 );
    }
    
    lineEdit_snapShot -> setText( file );
    
    lastSaveFolder = QFileInfo( file ).dirPath();
    
    //cout << "MainForm::getSaveFilename end" << endl;
    return( true );
}

//-------------------------------------------------------------------------
//------------- fileSaveAs
//-------------------------------------------------------------------------
/*!
 *  Deletes name of the save filename dialoge to open save file dialoge.
 *  \author Adrian Gabriel 
 *  \date Dec 2005
 */
void MainForm::fileSaveAs()
{
    //cout << "MainForm::fileSaveAs beg" << endl;
    //lineEdit_snapShot -> setText( "" );
    saveAsActivcated = true;
    fileSave();
    //cout << "MainForm::fileSaveAs end" << endl;
}

//-------------------------------------------------------------------------
//------------- closeEvent
//-------------------------------------------------------------------------
/*!
 *  Catches close event, to call a function that saves settings.
 *  \author Adrian Gabriel 
 *  \date Dec 2005
 */
void MainForm::closeEvent( QCloseEvent * )
{
    //cout << "MainForm::closeEvent beg" << endl;
    fileExit();
    //cout << "MainForm::closeEvent end" << endl;
}

//-------------------------------------------------------------------------
//------------- fileExit
//-------------------------------------------------------------------------
/*!
 *  Saves settings and then exits the application.
 *  \author Adrian Gabriel 
 *  \date Dec 2005
 */
void MainForm::fileExit()
{
    //cout << "MainForm::fileExit beg" << endl;
    saveSettings();
    QApplication::exit( 0 );
    //cout << "MainForm::fileExit end" << endl;
}

//-------------------------------------------------------------------------
//------------- helpAbout
//-------------------------------------------------------------------------
/*!
 *  Openes the about dialoge.
 *  \author Adrian Gabriel 
 *  \date Dec 2005
 */
void MainForm::helpAbout()
{
    //cout << "MainForm::helpAbout beg" << endl;
    if( !aboutForm )
    {
	aboutForm = new AboutForm( this );
    }
    aboutForm -> show();
    //cout << "MainForm::helpAbout end" << endl;
}

//-------------------------------------------------------------------------
//------------- openPovrayForm
//-------------------------------------------------------------------------
/*!
 *  Openes the about dialoge.
 *  \author Adrian Gabriel 
 *  \date Dec 2005
 */
void MainForm::openPovrayForm()
{
    //cout << "MainForm::openPovrayForm beg" << endl;
    
    if( !povform )
    {
	povform = new PovrayForm( this );
    }
    povform -> setParameter( cnf, glWindow );//, data );
    povform -> show();
    //cout << "MainForm::openPovrayForm end" << endl;
}

//-------------------------------------------------------------------------
//------------- noFileFormat
//-------------------------------------------------------------------------
/*!
 *  Status bar message when no file format was chosen for screenshot.
 *  \author Adrian Gabriel 
 *  \date Dec 2005
 */
void MainForm::noFileFormat()
{
    //cout << "MainForm::noFileFormat beg" << endl;
    statusBar() -> message( "Can not save File! No file format chosen!", 3000 );
    //cout << "MainForm::noFileFormat end" << endl;
}

//-------------------------------------------------------------------------
//------------- toggleFps
//-------------------------------------------------------------------------
/*!
 *  Show/hide fps toolbar.
 *  \author Adrian Gabriel 
 *  \date Jul 2007
 */
void MainForm::toggleFps()
{
    //cout << "MainForm::toggleFps beg" << endl;
    if( action_toggleFps -> isOn() ){ toolBar_fps -> show(); }
    else                            { toolBar_fps -> hide(); }
    //cout << "MainForm::toggleFps end" << endl;
}

//-------------------------------------------------------------------------
//------------- toggleModel1
//-------------------------------------------------------------------------
/*!
 *  Show/hide model 1 toolbar.
 *  \author Adrian Gabriel 
 *  \date Dec 2005
 */
void MainForm::toggleModel1()
{
    //cout << "MainForm::toggleModel1 beg" << endl;
    if( action_toggleModel1 -> isOn() ){ toolBar_model1 -> show(); }
    else                               { toolBar_model1 -> hide(); }
    //cout << "MainForm::toggleModel1 end" << endl;
}

//-------------------------------------------------------------------------
//------------- toggleModel2
//-------------------------------------------------------------------------
/*!
 *  Show/hide model 2 toolbar.
 *  \author Adrian Gabriel 
 *  \date Dec 2005
 */
void MainForm::toggleModel2()
{
    //cout << "MainForm::toggleModel2 beg" << endl;
    if( action_toggleModel2 -> isOn() ){ toolBar_model2 -> show(); }
    else                               { toolBar_model2 -> hide(); }
    //cout << "MainForm::toggleModel2 end" << endl;
}

//-------------------------------------------------------------------------
//------------- toggleTreeView
//-------------------------------------------------------------------------
void MainForm::toggleTreeView()
{
    //cout << "MainForm::toggleTreeView beg" << endl;
    if( action_toggleTreeView -> isOn() && treeView != 0 ){ treeWindow -> show(); }
    else                                                  { treeWindow -> hide(); }    
    //cout << "MainForm::toggleTreeView end" << endl;
}

//-------------------------------------------------------------------------
//------------- toggleSliceWindow
//-------------------------------------------------------------------------
void MainForm::toggleSliceWindow()
{
    //cout << "MainForm::toggleSliceWindow beg" << endl;
    if( action_toggleSliceWindow -> isOn() && sliceWindow != 0 ){ sliceWindow -> show(); }
    else                                                        { sliceWindow -> hide(); }    
    //cout << "MainForm::toggleSliceWindow end" << endl;
}

//-------------------------------------------------------------------------
//------------- toggleColor
//-------------------------------------------------------------------------
/*!
 *  Show/hide color toolbar.
 *  \author Adrian Gabriel 
 *  \date Dec 2005
 */
void MainForm::toggleColor()
{
    //cout << "MainForm::toggleColor beg" << endl;
    if( action_toggleColor -> isOn() ){ toolBar_color -> show(); }
    else                              { toolBar_color -> hide(); }
    //cout << "MainForm::toggleColor end" << endl;
}

//-------------------------------------------------------------------------
//------------- toggleView
//-------------------------------------------------------------------------
/*!
 *  Show/hide view toolbar.
 *  \author Adrian Gabriel 
 *  \date Dec 2005
 */
void MainForm::toggleView()
{
    //cout << "MainForm::toggleView beg" << endl;
    if( action_toggleView -> isOn() ){ toolBar_view -> show(); }
    else                             { toolBar_view -> hide(); }
    //cout << "MainForm::toggleView end" << endl;
}

//-------------------------------------------------------------------------
//------------- togglePosition
//-------------------------------------------------------------------------
/*!
 *  Show/hide position toolbar.
 *  \author Adrian Gabriel 
 *  \date Dec 2005
 */
void MainForm::togglePosition()
{
    //cout << "MainForm::togglePosition beg" << endl;
    if( action_togglePosition -> isOn() ){ toolBar_position -> show(); }
    else                                 { toolBar_position -> hide(); }
    //cout << "MainForm::togglePosition end" << endl;
}

//-------------------------------------------------------------------------
//------------- toggleOpenSave
//-------------------------------------------------------------------------
/*!
 *  Show/hide open/save toolbar.
 *  \author Adrian Gabriel 
 *  \date Dec 2005
 */
void MainForm::toggleOpenSave()
{
    //cout << "MainForm::toggleOpenSave beg" << endl;
    if( action_toggleOpenSave -> isOn() ){ toolBar_opensave -> show(); }
    else                                 { toolBar_opensave -> hide(); }
    //cout << "MainForm::toggleOpenSave end" << endl;
}

//-------------------------------------------------------------------------
//------------- toggleVideoControls
//-------------------------------------------------------------------------
/*!
 *  Show/hide video controls toolbar.
 *  \author Adrian Gabriel 
 *  \date Dec 2005
 */
void MainForm::toggleVideoControls()
{
    //cout << "MainForm::toggleVideoControls beg" << endl;
    if( action_toggleVideoControls -> isOn() ){ toolBar_videoControls -> show(); }
    else                                      { toolBar_videoControls -> hide(); }
    //cout << "MainForm::toggleVideoControls end" << endl;
}

//-------------------------------------------------------------------------
//------------- toggleVideoSettings
//-------------------------------------------------------------------------
/*!
 *  Show/hide video settings toolbar.
 *  \author Adrian Gabriel 
 *  \date Dec 2005
 */
void MainForm::toggleVideoSettings()
{
    //cout << "MainForm::toggleVideoSettings beg" << endl;
    if( action_toggleVideoSettings -> isOn() ){ toolBar_videoSettings -> show(); }
    else                                      { toolBar_videoSettings -> hide(); }
    //cout << "MainForm::toggleVideoSettings end" << endl;
}

//-------------------------------------------------------------------------
//------------- openColorForm
//-------------------------------------------------------------------------
/*!
 *  Openes color form.
 *  \author Adrian Gabriel 
 *  \date Dec 2005
 */
void MainForm::openColorForm()
{
    //cout << "MainForm::openColorForm beg" << endl;
    colorForm -> show();
    
    //cout << "MainForm::openColorForm end" << endl;
}

//-------------------------------------------------------------------------
//------------- videoFileOpen
//-------------------------------------------------------------------------
/*!
 *  Slot to open a system native file browser.
 *  Selected file will be set as the new configuration file to start an animation from.
 *  \author Adrian Gabriel 
 *  \date Dec 2005
 */
void MainForm::videoFileOpen()
{
    //cout << "MainForm::videoFileOpen beg" << endl;
    static QString newFile = "";
    newFile = QFileDialog::getOpenFileName( newFile=="" ? "./" : newFile, "All Files (*)", this, "open", "Choose configuration file" );   
    if( newFile != "" )
    {
	lineEdit_videoFile -> setText( newFile );
	if( newInputFile( newFile, DONTRELOADSAME ) == false )
	{
	    statusBar() -> message( "ERROR: Could not load file: " + newFile, 3000 );
	}
    }
    //cout << "MainForm::videoFileOpen end" << endl;
}

//-------------------------------------------------------------------------
//------------- videoStart
//-------------------------------------------------------------------------
/*!
 *  Slot that is called, when animation is to be started.
 *  Several buttons and line edits are enabled or disabled by this function.
 *  \author Adrian Gabriel 
 *  \date Dec 2005
 */
void MainForm::videoStart()
{
    //cout << "MainForm::videoStart beg" << endl;
    if( lineEdit_videoFile -> text() != "" && lineEdit_videoStart -> text() != "" && 
	lineEdit_videoStop -> text() != "" && lineEdit_videoStep  -> text() != ""    )
    {
	loadFirst = true;
	videoStartPressed = true;
	blockCnfChanged = true;
	lineEditVideoFileReturnPressed();
	blockCnfChanged = false;
	
	action_videoPause -> setEnabled( true );
	action_videoStop  -> setEnabled( true );
	qtTimer           -> start     ( 0    );
	
	if( !action_videoCapture -> isOn() ) { statusBar() -> message( "animation on." ); }
	
	enableVideoLineEdits( false );
	
	videoStartVal  = lineEdit_videoStart     -> text().toUInt();
	videoStopVal   = lineEdit_videoStop      -> text().toUInt();
	videoStepVal   = lineEdit_videoStep      -> text().toUInt();
	videoNumDigits = lineEdit_videoNumDigits -> text().toUInt();
	videoCount     = videoStartVal;
	captureCount   = 0;
	
	slider_videoProgress -> setMaxValue( (videoStopVal-videoStartVal) / videoStepVal );
	slider_videoProgress -> setMinValue( videoStartVal );
	slider_videoProgress -> setValue(0);
	
	if( action_videoCapture -> isOn() && videoNumDigits > 0 )
	{
	    QString zeros="";
	    QString count="";
	    
	    if( ( lineEdit_snapShot->text() ).stripWhiteSpace().isEmpty() )
	    {
		videoPause( true );
		if( getSaveFilename() == false )
		{
		    videoStop();
		    statusBar()->message( "Warning: No name given to save files to.", 3000 );
		    QMessageBox::warning( this, "QMGA -- Warning",
					  "Warning: No filename given.\n"
					  "Either set save filename or disable capture.\n\n",
					  "OK" );
		    return;
		}
		videoPause( false );
	    }
	    
	    count = count.setNum( captureCount );
	    zeros.setLength( videoNumDigits - count.length() );
	    zeros.fill( '0' );
	    videoCapture( lineEdit_snapShot -> text() + "." + zeros + count );
	}	
    }
    //cout << "MainForm::videoStart end" << endl;
}

//-------------------------------------------------------------------------
//------------- enableVideoLineEdits
//-------------------------------------------------------------------------
/*!
 *  Enable or disable line edits in the video section as well as the file-browser button.
 *  \param enabled True enables and false disables line edits.
 *  \author Adrian Gabriel 
 *  \date Dec 2005
 */
void MainForm::enableVideoLineEdits( bool enabled )
{
    //cout << "MainForm::enableVideoLineEdits beg" << endl;
    static const QSize sizeMin = minimumSize();
    static const QSize sizeMax = maximumSize();
    
    if( !enabled )
    {
	setFixedSize( size() );
    }
    else
    {
	setMinimumSize( sizeMin );
	setMaximumSize( sizeMax );
    }
    
    lineEdit_snapShot -> setEnabled( enabled );
    
    lineEditFileOpen -> setEnabled( enabled );
    
    lineEditPix1 -> setEnabled( enabled );
    lineEditPix2 -> setEnabled( enabled );
    
    action_fileOpen      -> setEnabled( enabled );
    action_togglePixel   -> setEnabled( enabled );
    action_fileSave      -> setEnabled( enabled );    
    action_videoFileOpen -> setEnabled( enabled );
    action_toggleEps     -> setEnabled( enabled );
    action_togglePng     -> setEnabled( enabled );
    
    lineEdit_videoFile      -> setEnabled( enabled );
    lineEdit_videoStart     -> setEnabled( enabled );
    lineEdit_videoStop      -> setEnabled( enabled );
    lineEdit_videoStep      -> setEnabled( enabled );
    lineEdit_videoNumDigits -> setEnabled( enabled );
    //cout << "MainForm::enableVideoLineEdits end" << endl;
}

//-------------------------------------------------------------------------
//------------- videoNextScene
//-------------------------------------------------------------------------
/*!
 *  Called by a timer (set up in videoStart()). With every call a new cnf file is loaded and displayed.
 *  \author Adrian Gabriel 
 *  \date Dec 2005
 */
void MainForm::videoNextScene()
{
    //cout << "MainForm::videoNextScene beg" << endl;
    if( lineEdit_videoFile -> text() == "" ) { return; }
    QString fileName = "";
    QString tmpNum   = "";
    QString strMask  = "";
    statusBar() -> message( "animation running." );
    
    if( videoCount >= videoStartVal && videoCount <= videoStopVal )
    {
	fileName  = lineEdit_videoFile -> text();
	fileName  . truncate( fileName.findRev(".") );
	tmpNum    . sprintf( "%u", videoCount );
	strMask   = lineEdit_videoFile -> text() . section('.', -1, -1 );
	strMask   . fill('0');
	strMask   . truncate( strMask.length() - tmpNum.length() );
	fileName += '.' + strMask + tmpNum;
	
	
	cnf -> setColorScheme( "director" );
	if( (cnf -> reloadCnfFile( fileName )) == true )
	{  
	    QString tmp = "";
	    cnfFile = fileName;
	    
	    buildScene( false, true );
	    forceChangeColorization = true;
	    changeColorisation();
	    forceChangeColorization = false;
	    glWindow -> waitForRepaint();
	    
	    lCDNumber_currentVideoFile -> display( tmp.sprintf("%d", videoCount) );
	    slider_videoProgress -> blockSignals(true);
	    slider_videoProgress -> setValue( (videoCount/videoStepVal) - (videoStartVal/videoStepVal) );
	    slider_videoProgress -> blockSignals(false);
	    if( action_videoCapture -> isOn() && videoNumDigits > 0 )
	    {
		QString zeros="";
		QString count="";
		captureCount++;
		
		count = count.setNum(captureCount);
		if( count.length() > videoNumDigits )
		{
		    videoStop();
		    statusBar()->message( "Warning: #digits set too low.", 3000 );
		    QMessageBox::warning( this, "QMGA -- Warning",
					  "Warning: #digits set too low.\n"
					  "Correct value and run again.\n\n",
					  "OK" );
		    return;
		}
		zeros.setLength( videoNumDigits - count.length() );
		zeros.fill('0');
		videoCapture( lineEdit_snapShot->text() + "." + zeros + count );
	    }
	}
	else { cerr << "Beware! : error while reloading cnf file!" << endl; }
    }
    
    if( !videoSliderActive )
    {
	if     ( action_videoForward -> isOn()  )
	{
	    if( videoCount <= videoStopVal - videoStepVal  ) { videoCount += videoStepVal;     }
	    else                                             { loadFirst = false; videoStop(); }
	}
	else if( action_videoBackward -> isOn() )
	{
	    if( videoCount >= videoStartVal + videoStepVal ) { videoCount -= videoStepVal;                                      }	
	    else                                             { loadFirst = false; videoStop(); action_videoForward -> toggle(); }
	}
    }
    //cout << "MainForm::videoNextScene end" << endl;
}

//-------------------------------------------------------------------------
//------------- videoCapture
//-------------------------------------------------------------------------
/*!
 *  Called when the capture every frame option is enabled.
 *  Every displayed frame is then saved as a screenahot.
 *  \author Adrian Gabriel 
 *  \date Dec 2005
 */
void MainForm::videoCapture( QString captureFilename )
{
    //cout << "MainForm::videoCapture beg" << endl;
    glWindow->getSnapShot( lineEditPix1->text().toInt(),
			   lineEditPix2->text().toInt(),
			   captureFilename,
			   action_togglePng->isOn(),
			   action_toggleEps->isOn() );
    statusBar()->message( "captured " + captureFilename );    
    //cout << "MainForm::videoCapture end" << endl;
}

//-------------------------------------------------------------------------
//------------- videoStop
//-------------------------------------------------------------------------
/*!
 *  Stops the animation and resets the counter, progress bar and lcd display.
 *  Some buttons are also enabled/disabled.
 *  \author Adrian Gabriel 
 *  \date Dec 2005
 */
void MainForm::videoStop()
{
    //cout << "MainForm::videoStop beg" << endl;
    qtTimer -> stop();
    videoStartPressed = false;
    statusBar()-> message( "animation off.", 3000 );
    
    forceChangeColorization = true;
    changeColorisation();
    forceChangeColorization = false;
    
    enableVideoLineEdits( true );
    action_videoPause          -> setEnabled( false );
    action_videoStop           -> setEnabled( false );
    slider_videoProgress       -> setValue( 0 );
    lCDNumber_currentVideoFile -> display( 0 );
    action_videoStart          -> setEnabled( true );
    action_videoStart          -> setOn( false );
    
    if( loadFirst )
    {
	blockCnfChanged = true;
	lineEditVideoFileReturnPressed(); 
	blockCnfChanged = false;
    }
    //cout << "MainForm::videoStop end" << endl;
}

//-------------------------------------------------------------------------
//------------- lineEditVideoFileReturnPressed
//-------------------------------------------------------------------------
/*!
 *  Loads file named in the line edit for the video file.
 *  \author Adrian Gabriel 
 *  \date Dec 2005
 */
void MainForm::lineEditVideoFileReturnPressed()
{
    //cout << "MainForm::lineEditVideoFileReturnPressed beg" << endl;
    QString newFile = lineEdit_videoFile -> text();
    newInputFile( newFile, DONTRELOADSAME );
    //cout << "MainForm::lineEditVideoFileReturnPressed end" << endl;
}

//-------------------------------------------------------------------------
//------------- videoPause
//-------------------------------------------------------------------------
/*!
 *  Starts and stops the timer that is used to get the animation going.
 *  Also some buttons are enabled or disabeld from here.
 *  \author Adrian Gabriel 
 *  \date Dec 2005
 */
void MainForm::videoPause( bool isOn )
{
    //cout << "MainForm::videoPause beg" << endl;
    if( isOn )
    {
	qtTimer           -> stop();
	action_videoStart -> setEnabled( false );
	action_videoStop  -> setEnabled( false );
	statusBar()->message( "Paused." );
    }
    else
    {
	qtTimer           -> start( 0 );
	action_videoStart -> setEnabled( true );
	action_videoStop  -> setEnabled( true );
    }
    //cout << "MainForm::videoPause end" << endl;
}

//-------------------------------------------------------------------------
//------------- videoSliderPressed
//-------------------------------------------------------------------------
/*!
 *  Controls behavior of the video slider.
 *  \author Adrian Gabriel 
 *  \date Dec 2005
 */
void MainForm::videoSliderPressed()
{
    //cout << "MainForm::videoSliderPressed beg" << endl;
    if( lineEdit_videoFile -> text() != "" && lineEdit_videoStart -> text() != "" && 
	lineEdit_videoStop -> text() != "" && lineEdit_videoStep  -> text() != ""    )
    {
	if( !action_videoPause->isOn() && videoStartPressed ) { videoPause( true ); }
	else if( !videoStartPressed ) { videoStart(); action_videoPause -> setOn( true ); }
    }
    videoSliderActive = true;
    //cout << "MainForm::videoSliderPressed end" << endl;
}

//-------------------------------------------------------------------------
//------------- videoSliderReleased
//-------------------------------------------------------------------------
/*!
 *  Controls behavior of the video slider.
 *  \author Adrian Gabriel 
 *  \date Dec 2005
 */
void MainForm::videoSliderReleased()
{
    //cout << "MainForm::videoSliderReleased beg" << endl;
    if( lineEdit_videoFile -> text() != "" && lineEdit_videoStart -> text() != "" && 
	lineEdit_videoStop -> text() != "" && lineEdit_videoStep  -> text() != ""    )
    {
	if( !action_videoPause -> isOn() && videoStartPressed ) { videoPause( false ); }
    }
    videoSliderActive = false;
    //cout << "MainForm::videoSliderReleased end" << endl;
}

//-------------------------------------------------------------------------
//------------- videoSliderMoved
//-------------------------------------------------------------------------
/*!
 *  Controls behavior of the video slider.
 *  \author Adrian Gabriel 
 *  \date Dec 2005
 */
void MainForm::videoSliderMoved( int newVal )
{
    //cout << "MainForm::videoSliderMoved beg" << endl;
    //if( videoSliderActive == false ) { return; }
    if( lineEdit_videoFile -> text() != "" && lineEdit_videoStart -> text() != "" && 
	lineEdit_videoStop -> text() != "" && lineEdit_videoStep  -> text() != ""    )
    {
	videoCount = newVal*videoStepVal + videoStartVal;
	//cout << "moved: " << videoCount << endl;
	videoNextScene();
    }
    //cout << "MainForm::videoSliderMoved end" << endl;
}

//-------------------------------------------------------------------------
//------------- saveSettings
//-------------------------------------------------------------------------
/*!
 *  Saves some settings to a configuration file.
 *  \author Adrian Gabriel 
 *  \date Dec 2005
 */
void MainForm::saveSettings()
{
    //cout << "MainForm::saveSettings beg" << endl;
    QSettings settings;
    settings.insertSearchPath( QSettings::Windows, WINDOWS_REGISTRY );
    
    settings.writeEntry( APP_KEY + "WindowWidth" , width()  );
    settings.writeEntry( APP_KEY + "WindowHeight", height() );
    
    settings.writeEntry( APP_KEY + "WindowX", x() );
    settings.writeEntry( APP_KEY + "WindowY", y() );
    // FIXME -- Gnome issue: when saving window position without
    //                       moving window by mouse before, -> x=y=0. Why ever...
    
    settings.writeEntry( APP_KEY + "VideoControls", toolBar_videoControls -> isShown() );
    settings.writeEntry( APP_KEY + "VideoSettings", toolBar_videoSettings -> isShown() );
    settings.writeEntry( APP_KEY + "Model1"       , toolBar_model1        -> isShown() );
    settings.writeEntry( APP_KEY + "Model2"       , toolBar_model2        -> isShown() );
    settings.writeEntry( APP_KEY + "Color"        , toolBar_color         -> isShown() );
    settings.writeEntry( APP_KEY + "Position"     , toolBar_position      -> isShown() );
    settings.writeEntry( APP_KEY + "OpenSave"     , toolBar_opensave      -> isShown() );
    settings.writeEntry( APP_KEY + "View"         , toolBar_view          -> isShown() );
    settings.writeEntry( APP_KEY + "Fps"          , toolBar_fps           -> isShown() );
    
    settings.writeEntry( APP_KEY + "FullRender"      , action_toggleFullRender       -> isOn() );
    settings.writeEntry( APP_KEY + "Axes"            , action_toggleAxes             -> isOn() );
    settings.writeEntry( APP_KEY + "BoundingBox"     , action_toggleBoundingBox      -> isOn() );
    settings.writeEntry( APP_KEY + "Colormap"        , action_toggleColormap         -> isOn() );
    settings.writeEntry( APP_KEY + "Fold"            , action_toggleFold             -> isOn() );
    settings.writeEntry( APP_KEY + "Optimized"       , action_toggleOptimized        -> isOn() );
    settings.writeEntry( APP_KEY + "Lod"             , action_toggleLOD              -> isOn() );
    settings.writeEntry( APP_KEY + "Png"             , action_togglePng              -> isOn() );
    settings.writeEntry( APP_KEY + "Eps"             , action_toggleEps              -> isOn() );
    settings.writeEntry( APP_KEY + "VideoCapture"    , action_videoCapture           -> isOn() );
    
    settings.writeEntry( APP_KEY + "UseDirector"     , action_useDirector            -> isOn() );
    settings.writeEntry( APP_KEY + "UseUserDefined"  , action_useUserDefined         -> isOn() );
    settings.writeEntry( APP_KEY + "UseColorByModel" , action_useColorByModel        -> isOn() );
    
    settings.writeEntry( APP_KEY + "ObjectsChangable", action_toggleObjectsChangable -> isOn() );
    settings.writeEntry( APP_KEY + "Objects"         , action_toggleObjects          -> isOn() );
    
    settings.writeEntry( APP_KEY + "LastFile"     , lineEditFileOpen  -> text() );    
    settings.writeEntry( APP_KEY + "LastSavedFile", lineEdit_snapShot -> text() );
    
    settings.writeEntry( APP_KEY + "LastVideoFile" , lineEdit_videoFile      -> text() );
    settings.writeEntry( APP_KEY + "VideoStart"    , lineEdit_videoStart     -> text() );
    settings.writeEntry( APP_KEY + "VideoStop"     , lineEdit_videoStop      -> text() );
    settings.writeEntry( APP_KEY + "VideoStep"     , lineEdit_videoStep      -> text() );
    settings.writeEntry( APP_KEY + "VideoNumDigits", lineEdit_videoNumDigits -> text() );
    
    settings.writeEntry( APP_KEY + "UserX", lineEdit_userX -> text() );
    settings.writeEntry( APP_KEY + "UserY", lineEdit_userY -> text() );
    settings.writeEntry( APP_KEY + "UserZ", lineEdit_userZ -> text() );
    
    settings.writeEntry( APP_KEY + "Model1_type" ,  comboBox_model1       -> currentItem() );
    settings.writeEntry( APP_KEY + "Model1_x"    ,  lineEdit_model1_x     -> text() );
    settings.writeEntry( APP_KEY + "Model1_y"    ,  lineEdit_model1_y     -> text() );
    settings.writeEntry( APP_KEY + "Model1_z"    ,  lineEdit_model1_z     -> text() );
    settings.writeEntry( APP_KEY + "Model1_dia"  ,  lineEdit_model1_dia   -> text() );
    settings.writeEntry( APP_KEY + "Model1_len"  ,  lineEdit_model1_len   -> text() );
    settings.writeEntry( APP_KEY + "Model1_radS1",  lineEdit_model1_radS1 -> text() );
    settings.writeEntry( APP_KEY + "Model1_radC" ,  lineEdit_model1_radC  -> text() );
    settings.writeEntry( APP_KEY + "Model1_radS2",  lineEdit_model1_radS2 -> text() );
    settings.writeEntry( APP_KEY + "Model1_cut"  ,  lineEdit_model1_cut   -> text() );
    settings.writeEntry( APP_KEY + "Model1_radS3",  lineEdit_model1_radS3 -> text() );
    settings.writeEntry( APP_KEY + "Model1_ang"  ,  lineEdit_model1_ang   -> text() );
    //settings.writeEntry( APP_KEY + "Model1_wire",  lineEdit_model1_wire -> text() );
    
    settings.writeEntry( APP_KEY + "Model2_type" ,  comboBox_model2       -> currentItem() );
    settings.writeEntry( APP_KEY + "Model2_x"    ,  lineEdit_model2_x     -> text() );
    settings.writeEntry( APP_KEY + "Model2_y"    ,  lineEdit_model2_y     -> text() );
    settings.writeEntry( APP_KEY + "Model2_z"    ,  lineEdit_model2_z     -> text() );
    settings.writeEntry( APP_KEY + "Model2_dia"  ,  lineEdit_model2_dia   -> text() );
    settings.writeEntry( APP_KEY + "Model2_len"  ,  lineEdit_model2_len   -> text() );
    settings.writeEntry( APP_KEY + "Model2_radS1",  lineEdit_model2_radS1 -> text() );
    settings.writeEntry( APP_KEY + "Model2_radC" ,  lineEdit_model2_radC  -> text() );
    settings.writeEntry( APP_KEY + "Model2_radS2",  lineEdit_model2_radS2 -> text() );
    settings.writeEntry( APP_KEY + "Model2_cut"  ,  lineEdit_model2_cut   -> text() );
    settings.writeEntry( APP_KEY + "Model2_radS3",  lineEdit_model2_radS3 -> text() );
    settings.writeEntry( APP_KEY + "Model2_ang"  ,  lineEdit_model2_ang   -> text() );
    //settings.writeEntry( APP_KEY + "Model2_wire",  lineEdit_model2_wire -> text() );
    
    settings.writeEntry( APP_KEY + "Pix1", lineEditPix1 -> isReadOnly() );
    settings.writeEntry( APP_KEY + "Pix2", lineEditPix2 -> isReadOnly() );
    
    settings.writeEntry( APP_KEY + "Pix1val", lineEditPix1 -> text() );
    settings.writeEntry( APP_KEY + "Pix2val", lineEditPix2 -> text() );
    
    settings.writeEntry( APP_KEY + "Quality" , comboBox_quality  -> currentItem() );
    settings.writeEntry( APP_KEY + "FileType", comboBox_fileType -> currentItem() );
    
    settings.writeEntry( APP_KEY + "LastSaveFolder", lastSaveFolder );
    settings.writeEntry( APP_KEY + "LastModelSaveFolder", lastModelSaveFolder );
    
    settings.writeEntry( APP_KEY + "RotX", spinBox_rotx -> value() );
    settings.writeEntry( APP_KEY + "RotY", spinBox_roty -> value() );
    settings.writeEntry( APP_KEY + "RotZ", spinBox_rotz -> value() );
    
    float  x_r, x_g, x_b, y_r, y_g, y_b, z_r, z_g, z_b;
    glWindow -> getAxisColors( x_r, x_g, x_b, y_r, y_g, y_b, z_r, z_g, z_b );
    
    settings.writeEntry( APP_KEY + "XRed"  , x_r );
    settings.writeEntry( APP_KEY + "XGreen", x_g );
    settings.writeEntry( APP_KEY + "XBlue" , x_b );
    
    settings.writeEntry( APP_KEY + "YRed"  , y_r );
    settings.writeEntry( APP_KEY + "YGreen", y_g );
    settings.writeEntry( APP_KEY + "YBlue" , y_b );
    
    settings.writeEntry( APP_KEY + "ZRed"  , z_r );
    settings.writeEntry( APP_KEY + "ZGreen", z_g );
    settings.writeEntry( APP_KEY + "ZBlue" , z_b );
    
    settings.writeEntry( APP_KEY + "Zoom" ,spinBox_zoom -> value() );
    
    settings.writeEntry( APP_KEY + "LineSize" ,spinBox_lineSize -> value() );
    
    if( colorForm != 0 )
    {
	float red = 0.0, green = 0.0, blue = 0.0;
	colorForm->getSliderAmbientValues( red, green, blue );
	settings.writeEntry( APP_KEY + "AmbientRed"  , red );
	settings.writeEntry( APP_KEY + "AmbientGreen", green );
	settings.writeEntry( APP_KEY + "AmbientBlue" , blue );
	
	colorForm->getSliderDiffuseValues( red, green, blue );
	settings.writeEntry( APP_KEY + "DiffuseRed"  , red );
	settings.writeEntry( APP_KEY + "DiffuseGreen", green );
	settings.writeEntry( APP_KEY + "DiffuseBlue" , blue );
	
	int shiny = 100;
	colorForm->getSliderSpecularValues( red, green, blue, shiny );
	settings.writeEntry( APP_KEY + "SpecularRed"  , red );
	settings.writeEntry( APP_KEY + "SpecularGreen", green );
	settings.writeEntry( APP_KEY + "SpecularBlue" , blue );
	settings.writeEntry( APP_KEY + "SpecularShiny", shiny );
	
	colorForm->getSliderBackgroundValues( red, green, blue );
	settings.writeEntry( APP_KEY + "BackgroundRed"  , red );
	settings.writeEntry( APP_KEY + "BackgroundGreen", green );
	settings.writeEntry( APP_KEY + "BackgroundBlue" , blue );
	
	colorForm->getSliderBoundingBoxValues( red, green, blue );
	settings.writeEntry( APP_KEY + "BoundingBoxRed"  , red );
	settings.writeEntry( APP_KEY + "BoundingBoxGreen", green );
	settings.writeEntry( APP_KEY + "BoundingBoxBlue" , blue );
	
	QString x = "", y = "", z = "";
	bool directional = false;
	colorForm->getLightPosition( x, y, z, directional );
	settings.writeEntry( APP_KEY + "LightPosX"  , x );
	settings.writeEntry( APP_KEY + "LightPosY"  , y );
	settings.writeEntry( APP_KEY + "LightPosZ"  , z );
	settings.writeEntry( APP_KEY + "Directional", directional );
    }
    
    settings.writeEntry( APP_KEY + "OpenHist_1" , openHist_1 );
    settings.writeEntry( APP_KEY + "OpenHist_2" , openHist_2 );
    settings.writeEntry( APP_KEY + "OpenHist_3" , openHist_3 );
    settings.writeEntry( APP_KEY + "OpenHist_4" , openHist_4 );
    
    settings.writeEntry( APP_KEY + "SSHUser" , sshUser );
    settings.writeEntry( APP_KEY + "SSHHost" , sshHost );
    
    settings.writeEntry( APP_KEY + "TreeSideBar", treeWindow -> isShown() );
    settings.writeEntry( APP_KEY + "SliceSideBar", sliceWindow -> isShown() );
    //cout << "MainForm::saveSettings end" << endl;
}

//-------------------------------------------------------------------------
//------------- loadSettings
//-------------------------------------------------------------------------
/*!
 *  Loads most settings from a configuration file.
 *  \author Adrian Gabriel 
 *  \date Dec 2005
 */
void MainForm::loadSettings()
{
    //cout << "MainForm::loadSettings beg" << endl;
    QSettings settings;
    settings.insertSearchPath( QSettings::Windows, WINDOWS_REGISTRY );
    int windowWidth = settings.readNumEntry( APP_KEY + "WindowWidth", 550 );
    int windowHeight = settings.readNumEntry( APP_KEY + "WindowHeight", 500 );
    int windowX = settings.readNumEntry( APP_KEY + "WindowX", 0 );
    int windowY = settings.readNumEntry( APP_KEY + "WindowY", 0 );
    
    setToolbar( settings.readBoolEntry( APP_KEY + "VideoControls", 1 ), toolBar_videoControls, action_toggleVideoControls );
    setToolbar( settings.readBoolEntry( APP_KEY + "VideoSettings", 1 ), toolBar_videoSettings, action_toggleVideoSettings );
    setToolbar( settings.readBoolEntry( APP_KEY + "Model1", 1 )       , toolBar_model1       , action_toggleModel1 );
    setToolbar( settings.readBoolEntry( APP_KEY + "Model2", 1 )       , toolBar_model2       , action_toggleModel2 );
    setToolbar( settings.readBoolEntry( APP_KEY + "Color", 1 )        , toolBar_color        , action_toggleColor );
    setToolbar( settings.readBoolEntry( APP_KEY + "Position", 1 )     , toolBar_position     , action_togglePosition );
    setToolbar( settings.readBoolEntry( APP_KEY + "OpenSave", 1 )     , toolBar_opensave     , action_toggleOpenSave );
    setToolbar( settings.readBoolEntry( APP_KEY + "View", 1 )         , toolBar_view         , action_toggleView );
    setToolbar( settings.readBoolEntry( APP_KEY + "Fps", 1 )          , toolBar_fps          , action_toggleFps );
    
    action_toggleFullRender  -> setOn( settings.readBoolEntry( APP_KEY + "FullRender"  , false ) );
    action_toggleAxes        -> setOn( settings.readBoolEntry( APP_KEY + "Axes"        , true  ) ); 
    action_toggleBoundingBox -> setOn( settings.readBoolEntry( APP_KEY + "BoundingBox" , true  ) ); 
    action_toggleColormap    -> setOn( settings.readBoolEntry( APP_KEY + "Colormap"    , true  ) );
    action_toggleFold        -> setOn( settings.readBoolEntry( APP_KEY + "Fold"        , false ) );
    action_toggleOptimized   -> setOn( settings.readBoolEntry( APP_KEY + "Optimized"   , false ) );
    action_toggleLOD         -> setOn( settings.readBoolEntry( APP_KEY + "Lod"         , true  ) );
    action_togglePng         -> setOn( settings.readBoolEntry( APP_KEY + "Png"         , true  ) );
    action_toggleEps         -> setOn( settings.readBoolEntry( APP_KEY + "Eps"         , false ) );
    action_videoCapture      -> setOn( settings.readBoolEntry( APP_KEY + "VideoCapture", false ) );
    
    QString lastFile = settings.readEntry( APP_KEY + "LastFile" , "mga_dummy.cnf" );
    if( QFile::exists(lastFile) )
    {
	cnfFile = lastFile;
	lineEditFileOpen -> setText( cnfFile );
    }
    else if( QFile::exists("mga_dummy.cnf") )
    {
	cnfFile = "mga_dummy.cnf";
	lineEditFileOpen -> setText( "mga_dummy.cnf" );
    }
    lineEdit_snapShot -> setText( settings.readEntry( APP_KEY + "LastSavedFile" , "snapshot" ) );
    
    QString lastVideoFile = settings.readEntry( APP_KEY + "LastVideoFile" , "" );
    if( QFile::exists(lastVideoFile) )
    {
	lineEdit_videoFile -> setText( lastVideoFile );
    }
    
    lineEdit_videoStart     -> setText( settings.readEntry( APP_KEY + "VideoStart"    , "0" ) );
    lineEdit_videoStop      -> setText( settings.readEntry( APP_KEY + "VideoStop"     , "100" ) );
    lineEdit_videoStep      -> setText( settings.readEntry( APP_KEY + "VideoStep"     , "1" ) );
    lineEdit_videoNumDigits -> setText( settings.readEntry( APP_KEY + "VideoNumDigits", "10" ) );
    
    lineEdit_userX -> setText( settings.readEntry( APP_KEY + "UserX" , "0.00"   ) );
    lineEdit_userY -> setText( settings.readEntry( APP_KEY + "UserY" , "0.00"   ) );
    lineEdit_userZ -> setText( settings.readEntry( APP_KEY + "UserZ" , "1.00"   ) );
    
    comboBox_model1       -> setCurrentItem( settings.readNumEntry( APP_KEY + "Model1_type", 0 ) );
    lineEdit_model1_x     -> setText( settings.readEntry( APP_KEY + "Model1_x"     , "1.0" ) );
    lineEdit_model1_y     -> setText( settings.readEntry( APP_KEY + "Model1_y"     , "1.0" ) );
    lineEdit_model1_z     -> setText( settings.readEntry( APP_KEY + "Model1_z"     , "0.2" ) );
    lineEdit_model1_len   -> setText( settings.readEntry( APP_KEY + "Model1_len"   , "0.0" ) );
    lineEdit_model1_dia   -> setText( settings.readEntry( APP_KEY + "Model1_dia"   , "0.0" ) );
    lineEdit_model1_radS1 -> setText( settings.readEntry( APP_KEY + "Model1_radS1" , "0.5" ) );
    lineEdit_model1_radC  -> setText( settings.readEntry( APP_KEY + "Model1_radC"  , "0.5" ) );
    lineEdit_model1_radS2 -> setText( settings.readEntry( APP_KEY + "Model1_radS2" , "0.5" ) );
    lineEdit_model1_cut   -> setText( settings.readEntry( APP_KEY + "Model1_cut"   , "0.1" ) );
    lineEdit_model1_radS3 -> setText( settings.readEntry( APP_KEY + "Model1_radS3" , "0.5" ) );
    lineEdit_model1_ang   -> setText( settings.readEntry( APP_KEY + "Model1_ang"   , "3.14" ) );
    //lineEdit_model1_wire -> setText( settings.readEntry( APP_KEY + "Model1_wire"  , "0.0" ) );
    
    comboBox_model2       -> setCurrentItem( settings.readNumEntry( APP_KEY + "Model2_type", 1 ) );
    lineEdit_model2_x     -> setText( settings.readEntry( APP_KEY + "Model2_x"     , "0.0" ) );
    lineEdit_model2_y     -> setText( settings.readEntry( APP_KEY + "Model2_y"     , "1.0" ) );
    lineEdit_model2_z     -> setText( settings.readEntry( APP_KEY + "Model2_z"     , "0.0" ) );
    lineEdit_model2_len   -> setText( settings.readEntry( APP_KEY + "Model2_len"   , "1.0" ) );
    lineEdit_model2_dia   -> setText( settings.readEntry( APP_KEY + "Model2_dia"   , "3.0" ) );
    lineEdit_model2_radS1 -> setText( settings.readEntry( APP_KEY + "Model2_radS1" , "0.5" ) );
    lineEdit_model2_radC  -> setText( settings.readEntry( APP_KEY + "Model2_radC"  , "0.5" ) );
    lineEdit_model2_radS2 -> setText( settings.readEntry( APP_KEY + "Model2_radS2" , "0.5" ) );
    lineEdit_model2_cut   -> setText( settings.readEntry( APP_KEY + "Model2_cut"   , "0.1" ) );
    lineEdit_model2_radS3 -> setText( settings.readEntry( APP_KEY + "Model2_radS3" , "0.5" ) );
    lineEdit_model2_ang   -> setText( settings.readEntry( APP_KEY + "Model2_ang"   , "3.14" ) );
    //lineEdit_model2_wire -> setText( settings.readEntry( APP_KEY + "Model2_wire"  , "0.0" ) );
    
    lineEditPix1 -> setReadOnly( !settings.readBoolEntry( APP_KEY + "Pix1", false ) );
    lineEditPix2 -> setReadOnly( !settings.readBoolEntry( APP_KEY + "Pix2", true  ) );
    togglePixel();
    
    QString pixVal = "";
    if( lineEditPix2->isReadOnly()  )
    {
	pixVal = settings.readEntry( APP_KEY + "Pix1val", "1000" );
	lineEditPix1->setText( pixVal );
    }
    else
    {
	pixVal = settings.readEntry( APP_KEY + "Pix2val", "1000" );
	lineEditPix2->setText( pixVal );
    }
    
    comboBox_quality  -> setCurrentItem( settings.readNumEntry( APP_KEY + "Quality", 2 ) );
    comboBox_fileType -> setCurrentItem( settings.readNumEntry( APP_KEY + "FileType", 0 ) );
    
    unsigned int rx = settings.readNumEntry( APP_KEY + "RotX", 0 );
    unsigned int ry = settings.readNumEntry( APP_KEY + "RotY", 0 );
    unsigned int rz = settings.readNumEntry( APP_KEY + "RotZ", 0 );
    spinBox_rotx -> setValue( rx );
    spinBox_roty -> setValue( ry );
    spinBox_rotz -> setValue( rz );
    glWindow->setAngles( rx, ry, rz );
    
    float  x_r, x_g, x_b, y_r, y_g, y_b, z_r, z_g, z_b;
    glWindow -> getAxisColors( x_r, x_g, x_b, y_r, y_g, y_b, z_r, z_g, z_b );
    
    x_r = settings.readDoubleEntry( APP_KEY + "XRed"  , x_r );
    x_g = settings.readDoubleEntry( APP_KEY + "XGreen", x_g );
    x_b = settings.readDoubleEntry( APP_KEY + "XBlue" , x_b );
    
    y_r = settings.readDoubleEntry( APP_KEY + "YRed"  , y_r );
    y_g = settings.readDoubleEntry( APP_KEY + "YGreen", y_g );
    y_b = settings.readDoubleEntry( APP_KEY + "YBlue" , y_b );
    
    z_r = settings.readDoubleEntry( APP_KEY + "ZRed"  , z_r );
    z_g = settings.readDoubleEntry( APP_KEY + "ZGreen", z_g );
    z_b = settings.readDoubleEntry( APP_KEY + "ZBlue" , z_b );
    glWindow->setAxisColors( x_r, x_g, x_b, y_r, y_g, y_b, z_r, z_g, z_b );
    
    int x_R, x_G, x_B, y_R, y_G, y_B, z_R, z_G, z_B;
    x_R = int(x_r*255);
    x_G = int(x_g*255);
    x_B = int(x_b*255);
    y_R = int(y_r*255);
    y_G = int(y_g*255);
    y_B = int(y_b*255);
    z_R = int(z_r*255);
    z_G = int(z_g*255);
    z_B = int(z_b*255);
    pushButton_x -> setPaletteBackgroundColor( QColor(x_R, x_G, x_B) );
    pushButton_y -> setPaletteBackgroundColor( QColor(y_R, y_G, y_B) );
    pushButton_z -> setPaletteBackgroundColor( QColor(z_R, z_G, z_B) );
    
    lineEdit_rotX -> setPaletteBackgroundColor( QColor(x_R, x_G, x_B) );
    lineEdit_rotY -> setPaletteBackgroundColor( QColor(y_R, y_G, y_B) );
    lineEdit_rotZ -> setPaletteBackgroundColor( QColor(z_R, z_G, z_B) );
    
    dial_rotX -> setPaletteForegroundColor( QColor(x_R, x_G, x_B) );
    dial_rotY -> setPaletteForegroundColor( QColor(y_R, y_G, y_B) );
    dial_rotZ -> setPaletteForegroundColor( QColor(z_R, z_G, z_B) );
    
    lastSaveFolder = settings.readEntry( APP_KEY + "LastSaveFolder" , "./" );
    lastModelSaveFolder = settings.readEntry( APP_KEY + "LastModelSaveFolder" , "./" );
    
    resize( windowWidth, windowHeight );
    move( windowX, windowY );
    
    openHist_1 = settings.readEntry( APP_KEY + "OpenHist_1", "." );
    openHist_2 = settings.readEntry( APP_KEY + "OpenHist_2", "." );
    openHist_3 = settings.readEntry( APP_KEY + "OpenHist_3", "." );
    openHist_4 = settings.readEntry( APP_KEY + "OpenHist_4", "." );
    updateOpenHist();
    
    sshUser = settings.readEntry( APP_KEY + "SSHUser", "noUser" );
    sshHost = settings.readEntry( APP_KEY + "SSHHost", "noHost" );
    
    //cout << "MainForm::loadSettings end" << endl;
}

//-------------------------------------------------------------------------
//------------- setToolbar
//-------------------------------------------------------------------------
/*!
 *  Shows/hides a toolbar and sets/unsets the connected action.
 *  \param set True to show toolbar and set the action to on.
 *  \param toolbar The toolbar to show/hide.
 *  \param action The action connected to the toolbar.
 *  \author Adrian Gabriel 
 *  \date Dec 2005
 */
void MainForm::setToolbar( bool set, QToolBar *toolbar, QAction *action )
{
    //cout << "MainForm::setToolbar beg" << endl;
    if( set )
    {
	toolbar -> show();
	action  -> setOn( true );
    }
    else
    {
	toolbar -> hide();
	action  -> setOn( false );
    }
    //cout << "MainForm::setToolbar end" << endl;
}

//-------------------------------------------------------------------------
//------------- lineSizeChanged
//-------------------------------------------------------------------------
/*!
 *  Slot to call, when the value of the line size is to be changed.
 *  \author Adrian Gabriel 
 *  \date April 2007
 */
void MainForm::lineSizeChanged( int lineSize )
{
    //cout << "MainForm::lineSizeChanged beg" << endl;
    if( lineSize >= 0 && lineSize <= 100 )
    {
	glWindow -> setLineSize( float(lineSize)/100.0 );
    }
    //cout << "MainForm::lineSizeChanged end" << endl;
}

/*
//-------------------------------------------------------------------------
//------------- moveEvent
//-------------------------------------------------------------------------
void MainForm::moveEvent( QMoveEvent * event )
{
//    static int count = 2;

//    if( count != 1 )
//    {
	cout << "move1: " << x() << " " << y() << endl;
	QWidget::moveEvent ( event );
	cout << "move2: " << x() << " " << y() << endl;
//    }
//    if( count != 0 )
//    {
//	count--;
//    }
}	
*/

//-------------------------------------------------------------------------
//------------- setConnections
//-------------------------------------------------------------------------
/*!
 *  Handles most of the connections in the main window.
 *  \author Adrian Gabriel 
 *  \date Dec 2005
 */
void MainForm::setConnections()
{
    //cout << "MainForm::setConnections beg" << endl;
    connect( qtTimer  , SIGNAL(timeout()), this, SLOT(videoNextScene ()) );
    connect( saveTimer, SIGNAL(timeout()), this, SLOT(startSnapshot()) );
    
    connect( glWindow, SIGNAL(sizeChanged())           , this, SLOT(keepAspectRatio()) );
    connect( glWindow, SIGNAL(noFileFormatSet())       , this, SLOT(noFileFormat()) );
    connect( glWindow, SIGNAL(zoomChanged(int))        , this, SLOT(updateZoom(int)) );
    connect( glWindow, SIGNAL(angleXChanged(int))      , this, SLOT(updateAlpha(int)) );
    connect( glWindow, SIGNAL(angleYChanged(int))      , this, SLOT(updateBeta(int)) );
    connect( glWindow, SIGNAL(angleZChanged(int))      , this, SLOT(updateGamma(int)) );
    connect( glWindow, SIGNAL(keyPress(int))           , this, SLOT(slotKeyPress(int)) );
    connect( glWindow, SIGNAL(dropAccepted(QString))   , this, SLOT(dropInRenderArea(QString)) );
    connect( glWindow, SIGNAL(fpsUpdate(double))       , this, SLOT(updateFps(double)) );
    connect( glWindow, SIGNAL(translate(vector<float>)), this, SLOT(updateTranslationBoxes(vector<float>)) );
    
    connect( action_fileOpen              , SIGNAL(activated())  , this, SLOT(fileOpen()) );
    connect( action_fileExit              , SIGNAL(activated())  , this, SLOT(fileExit()) );
    connect( action_fileSave              , SIGNAL(activated())  , this, SLOT(fileSave()) );
    connect( action_fileSaveAs            , SIGNAL(activated())  , this, SLOT(fileSaveAs()) );
    connect( action_colorForm             , SIGNAL(activated())  , this, SLOT(openColorForm()) );
    connect( action_povrayForm            , SIGNAL(activated())  , this, SLOT(openPovrayForm()) );
    connect( action_helpAbout             , SIGNAL(activated())  , this, SLOT(helpAbout()) );
    connect( action_toggleAxes            , SIGNAL(toggled(bool)), this, SLOT(toggleAxes(bool)) );
    connect( action_toggleBoundingBox     , SIGNAL(toggled(bool)), this, SLOT(toggleBoundingBox(bool)) );
    connect( action_toggleColor           , SIGNAL(toggled(bool)), this, SLOT(toggleColor()) );
    connect( action_toggleColormap        , SIGNAL(toggled(bool)), this, SLOT(toggleColormap(bool)) );
    //connect( action_toggleFold            , SIGNAL(toggled(bool)), this, SLOT(toggleFold()) );
    connect( action_toggleFullRender      , SIGNAL(toggled(bool)), this, SLOT(toggleFullRender(bool)) );
    connect( action_toggleView            , SIGNAL(toggled(bool)), this, SLOT(toggleView()) );
    connect( action_toggleOpenSave        , SIGNAL(toggled(bool)), this, SLOT(toggleOpenSave()) );
    connect( action_toggleOptimized       , SIGNAL(toggled(bool)), this, SLOT(toggleOptimized(bool)) );
    connect( action_toggleLOD             , SIGNAL(toggled(bool)), this, SLOT(toggleLOD(bool)) );
    connect( action_togglePixel           , SIGNAL(activated())  , this, SLOT(togglePixel()) );
    connect( action_togglePosition        , SIGNAL(toggled(bool)), this, SLOT(togglePosition()) );
    connect( action_toggleFps             , SIGNAL(toggled(bool)), this, SLOT(toggleFps()) );
    connect( action_toggleModel1          , SIGNAL(toggled(bool)), this, SLOT(toggleModel1()) );
    connect( action_toggleModel2          , SIGNAL(toggled(bool)), this, SLOT(toggleModel2()) );
    connect( action_toggleTreeView        , SIGNAL(toggled(bool)), this, SLOT(toggleTreeView()) );
    connect( action_toggleSliceWindow     , SIGNAL(toggled(bool)), this, SLOT(toggleSliceWindow()) );
    connect( action_toggleSlice           , SIGNAL(toggled(bool)), this, SLOT(toggleSlice(bool)) );
    //connect( action_useUserDefined        , SIGNAL(toggled(bool)), this, SLOT(changeColorisation()) );
    connect( action_toggleVideoControls   , SIGNAL(toggled(bool)), this, SLOT(toggleVideoControls()) );
    connect( action_toggleVideoSettings   , SIGNAL(toggled(bool)), this, SLOT(toggleVideoSettings()) );
    connect( action_videoFileOpen         , SIGNAL(activated())  , this, SLOT(videoFileOpen()) );	
    connect( action_videoStart            , SIGNAL(activated())  , this, SLOT(videoStart()) );	
    connect( action_videoStop             , SIGNAL(activated())  , this, SLOT(videoStop()) );	
    connect( action_videoPause            , SIGNAL(toggled(bool)), this, SLOT(videoPause(bool)) );	
    connect( action_toggleObjectsChangable, SIGNAL(toggled(bool)), this, SLOT(toggleObjectsChangable()) );	
    connect( action_toggleObjects         , SIGNAL(toggled(bool)), this, SLOT(toggleObjects()) );	
    connect( action_objectParams          , SIGNAL(activated())  , this, SLOT(openModelsForm()) );	
    connect( action_openHist_1            , SIGNAL(activated())  , this, SLOT(loadOpenHist_1()) );
    connect( action_openHist_2            , SIGNAL(activated())  , this, SLOT(loadOpenHist_2()) );
    connect( action_openHist_3            , SIGNAL(activated())  , this, SLOT(loadOpenHist_3()) );
    connect( action_openHist_4            , SIGNAL(activated())  , this, SLOT(loadOpenHist_4()) );
    connect( action_saveHistogram         , SIGNAL(activated())  , this, SLOT(printHistogram()) );
    connect( action_loadModelsFile        , SIGNAL(activated())  , this, SLOT(openModelsFile()) );
    connect( action_saveModelsFile        , SIGNAL(activated())  , this, SLOT(saveModelsFile()) );
    connect( action_translate             , SIGNAL(toggled(bool)), this, SLOT(translate()) );
    
    connect( actionGroup_colorAxis, SIGNAL(selected(QAction *)), this, SLOT(changeColorisation()) );
    
    connect( lineEdit_userX    , SIGNAL(returnPressed()), this, SLOT(changeColorisation()) );
    connect( lineEdit_userY    , SIGNAL(returnPressed()), this, SLOT(changeColorisation()) );
    connect( lineEdit_userZ    , SIGNAL(returnPressed()), this, SLOT(changeColorisation()) );
    connect( lineEditFileOpen  , SIGNAL(returnPressed()), this, SLOT(lineEditFileOpenReturnPressed()) );
    connect( lineEdit_snapShot , SIGNAL(returnPressed()), this, SLOT(fileSave() ));
    connect( lineEdit_videoFile, SIGNAL(returnPressed()), this, SLOT(lineEditVideoFileReturnPressed()) );
    connect( lineEdit_rotX     , SIGNAL(returnPressed()), this, SLOT(rotXEnter()) );
    connect( lineEdit_rotY     , SIGNAL(returnPressed()), this, SLOT(rotYEnter()) );
    connect( lineEdit_rotZ     , SIGNAL(returnPressed()), this, SLOT(rotZEnter()) );
    
    connect( comboBox_model1      , SIGNAL(activated(int)) , this, SLOT(rescaleScene()) );
    connect( lineEdit_model1_x    , SIGNAL(returnPressed()), this, SLOT(rescaleScene()) );
    connect( lineEdit_model1_y    , SIGNAL(returnPressed()), this, SLOT(rescaleScene()) );
    connect( lineEdit_model1_z    , SIGNAL(returnPressed()), this, SLOT(rescaleScene()) );
    connect( lineEdit_model1_dia  , SIGNAL(returnPressed()), this, SLOT(rescaleScene()) );
    connect( lineEdit_model1_len  , SIGNAL(returnPressed()), this, SLOT(rescaleScene()) );
    connect( lineEdit_model1_radS1, SIGNAL(returnPressed()), this, SLOT(rescaleScene()) );
    connect( lineEdit_model1_radC , SIGNAL(returnPressed()), this, SLOT(rescaleScene()) );
    connect( lineEdit_model1_radS2, SIGNAL(returnPressed()), this, SLOT(rescaleScene()) );
    connect( lineEdit_model1_cut  , SIGNAL(returnPressed()), this, SLOT(rescaleScene()) );
    connect( lineEdit_model1_radS3, SIGNAL(returnPressed()), this, SLOT(rescaleScene()) );
    connect( lineEdit_model1_ang  , SIGNAL(returnPressed()), this, SLOT(rescaleScene()) );
    //connect( lineEdit_model1_wire, SIGNAL(returnPressed()), this, SLOT(rescaleScene()) );
    
    connect( comboBox_model2      , SIGNAL(activated(int)) , this, SLOT(rescaleScene()) );
    connect( lineEdit_model2_x    , SIGNAL(returnPressed()), this, SLOT(rescaleScene()) );
    connect( lineEdit_model2_y    , SIGNAL(returnPressed()), this, SLOT(rescaleScene()) );
    connect( lineEdit_model2_z    , SIGNAL(returnPressed()), this, SLOT(rescaleScene()) );
    connect( lineEdit_model2_dia  , SIGNAL(returnPressed()), this, SLOT(rescaleScene()) );
    connect( lineEdit_model2_len  , SIGNAL(returnPressed()), this, SLOT(rescaleScene()) );
    connect( lineEdit_model2_radS1, SIGNAL(returnPressed()), this, SLOT(rescaleScene()) );
    connect( lineEdit_model2_radC , SIGNAL(returnPressed()), this, SLOT(rescaleScene()) );
    connect( lineEdit_model2_radS2, SIGNAL(returnPressed()), this, SLOT(rescaleScene()) );
    connect( lineEdit_model2_cut  , SIGNAL(returnPressed()), this, SLOT(rescaleScene()) );
    connect( lineEdit_model2_radS3, SIGNAL(returnPressed()), this, SLOT(rescaleScene()) );
    connect( lineEdit_model2_ang  , SIGNAL(returnPressed()), this, SLOT(rescaleScene()) );
    //connect( lineEdit_model2_wire, SIGNAL(returnPressed()), this, SLOT(rescaleScene()) );
    
    connect( comboBox_model1      , SIGNAL(activated(int)) , modelsForm, SLOT(updateModelsVector()) );
    connect( lineEdit_model1_x    , SIGNAL(returnPressed()), modelsForm, SLOT(updateModelsVector()) );
    connect( lineEdit_model1_y    , SIGNAL(returnPressed()), modelsForm, SLOT(updateModelsVector()) );
    connect( lineEdit_model1_z    , SIGNAL(returnPressed()), modelsForm, SLOT(updateModelsVector()) );
    connect( lineEdit_model1_dia  , SIGNAL(returnPressed()), modelsForm, SLOT(updateModelsVector()) );
    connect( lineEdit_model1_len  , SIGNAL(returnPressed()), modelsForm, SLOT(updateModelsVector()) );
    connect( lineEdit_model1_radS1, SIGNAL(returnPressed()), modelsForm, SLOT(updateModelsVector()) );
    connect( lineEdit_model1_radC , SIGNAL(returnPressed()), modelsForm, SLOT(updateModelsVector()) );
    connect( lineEdit_model1_radS2, SIGNAL(returnPressed()), modelsForm, SLOT(updateModelsVector()) );
    connect( lineEdit_model1_cut  , SIGNAL(returnPressed()), modelsForm, SLOT(updateModelsVector()) );
    connect( lineEdit_model1_radS3, SIGNAL(returnPressed()), modelsForm, SLOT(updateModelsVector()) );
    connect( lineEdit_model1_ang  , SIGNAL(returnPressed()), modelsForm, SLOT(updateModelsVector()) );
    //connect( lineEdit_model1_wire, SIGNAL(returnPressed()), modelsForm, SLOT(updateModelsVector()) );
    
    connect( comboBox_model2      , SIGNAL(activated(int)) , modelsForm, SLOT(updateModelsVector()) );
    connect( lineEdit_model2_x    , SIGNAL(returnPressed()), modelsForm, SLOT(updateModelsVector()) );
    connect( lineEdit_model2_y    , SIGNAL(returnPressed()), modelsForm, SLOT(updateModelsVector()) );
    connect( lineEdit_model2_z    , SIGNAL(returnPressed()), modelsForm, SLOT(updateModelsVector()) );
    connect( lineEdit_model2_dia  , SIGNAL(returnPressed()), modelsForm, SLOT(updateModelsVector()) );
    connect( lineEdit_model2_len  , SIGNAL(returnPressed()), modelsForm, SLOT(updateModelsVector()) );
    connect( lineEdit_model2_radS1, SIGNAL(returnPressed()), modelsForm, SLOT(updateModelsVector()) );
    connect( lineEdit_model2_radC , SIGNAL(returnPressed()), modelsForm, SLOT(updateModelsVector()) );
    connect( lineEdit_model2_radS2, SIGNAL(returnPressed()), modelsForm, SLOT(updateModelsVector()) );
    connect( lineEdit_model2_cut  , SIGNAL(returnPressed()), modelsForm, SLOT(updateModelsVector()) );
    connect( lineEdit_model2_radS3, SIGNAL(returnPressed()), modelsForm, SLOT(updateModelsVector()) );
    connect( lineEdit_model2_ang  , SIGNAL(returnPressed()), modelsForm, SLOT(updateModelsVector()) );
    //connect( lineEdit_model2_wire, SIGNAL(returnPressed()), modelsForm, SLOT(updateModelsVector()) );
    
    connect( lineEdit_boxCount, SIGNAL(returnPressed()), this, SLOT(updateBoxCount()) );
    
    connect( lineEditPix1, SIGNAL(textChanged(const QString &)), this, SLOT(keepAspectRatio()) );
    connect( lineEditPix2, SIGNAL(textChanged(const QString &)), this, SLOT(keepAspectRatio()) );
    
    connect( dial_rotX  , SIGNAL(valueChanged(int)), this, SLOT(rotX(int)) );
    connect( dial_rotY  , SIGNAL(valueChanged(int)), this, SLOT(rotY(int)) );
    connect( dial_rotZ  , SIGNAL(valueChanged(int)), this, SLOT(rotZ(int)) );
    
    
    connect( pushButton_x, SIGNAL(clicked()), this, SLOT(colorButtonX()) );
    connect( pushButton_y, SIGNAL(clicked()), this, SLOT(colorButtonY()) );
    connect( pushButton_z, SIGNAL(clicked()), this, SLOT(colorButtonZ()) );
    
    connect( pushButton_meanFps, SIGNAL(clicked()), this, SLOT(resetMeanFps()) );
    
    connect( comboBox_quality , SIGNAL(activated(int)), this, SLOT(rescaleScene()) );
    connect( comboBox_fileType, SIGNAL(activated(int)), this, SLOT(formatChanged(int)) );
    
    connect( spinBox_zoom      , SIGNAL(valueChanged(int)), this, SLOT(changeZoom()) );
    connect( spinBox_rotx      , SIGNAL(valueChanged(int)), this, SLOT(rotAngleBoxesChanged()) );
    connect( spinBox_roty      , SIGNAL(valueChanged(int)), this, SLOT(rotAngleBoxesChanged()) );
    connect( spinBox_rotz      , SIGNAL(valueChanged(int)), this, SLOT(rotAngleBoxesChanged()) );
    connect( spinBox_translateX, SIGNAL(valueChanged(int)), this, SLOT(translate()) );
    connect( spinBox_translateY, SIGNAL(valueChanged(int)), this, SLOT(translate()) );
    connect( spinBox_translateZ, SIGNAL(valueChanged(int)), this, SLOT(translate()) );
    
    
    connect( slider_videoProgress, SIGNAL(sliderPressed()) , this, SLOT(videoSliderPressed()) );
    connect( slider_videoProgress, SIGNAL(sliderReleased()), this, SLOT(videoSliderReleased()) );
    connect( slider_videoProgress, SIGNAL(valueChanged(int)), this, SLOT(videoSliderMoved(int)) );
    //connect( slider_videoProgress, SIGNAL(sliderMoved(int)), this, SLOT(videoSliderMoved(int)) );
    
    connect( toolBar_videoControls, SIGNAL(placeChanged(QDockWindow::Place)), this, SLOT(toolBarUnDocked(QDockWindow::Place)) ); 
    
    connect( comboBox_model1, SIGNAL(activated(int)), this, SLOT(adjustModel1Toolbar(int)) ); 
    connect( comboBox_model2, SIGNAL(activated(int)), this, SLOT(adjustModel2Toolbar(int)) ); 
    
    connect( modelsForm, SIGNAL(modelDataChanged(bool)), this, SLOT(updateModelsLineEdits(bool)) );
    
    connect( spinBox_lineSize, SIGNAL(valueChanged(int)), this, SLOT(lineSizeChanged(int)) );
    
    //cout << "MainForm::setConnections end" << endl;
}

//-------------------------------------------------------------------------
//------------- showModel1Ellipsoid
//-------------------------------------------------------------------------
void MainForm::showModel1Ellipsoid( bool show )
{
    static QWidget* labelX  = (QWidget*)(toolBar_model1 -> child("textLabel_model1_x"));
    static QWidget* labelY  = (QWidget*)(toolBar_model1 -> child("textLabel_model1_y"));
    static QWidget* labelZ  = (QWidget*)(toolBar_model1 -> child("textLabel_model1_z"));
    static QWidget* lineX   = (QWidget*)(toolBar_model1 -> child("lineEdit_model1_x"));
    static QWidget* lineY   = (QWidget*)(toolBar_model1 -> child("lineEdit_model1_y"));
    static QWidget* lineZ   = (QWidget*)(toolBar_model1 -> child("lineEdit_model1_z"));
    
    if( show )
    {
	labelX -> show();
	lineX  -> show();
	labelY -> show();
	lineY  -> show();
	labelZ -> show();
	lineZ  -> show();
    }
    else
    {
	labelX -> hide();
	lineX  -> hide();
	labelY -> hide();
	lineY  -> hide();
	labelZ -> hide();
	lineZ  -> hide();	
    }
}

//-------------------------------------------------------------------------
//------------- showModel2Ellipsoid
//-------------------------------------------------------------------------
void MainForm::showModel2Ellipsoid( bool show )
{
    static QWidget* labelX  = (QWidget*)(toolBar_model2 -> child("textLabel_model2_x"));
    static QWidget* labelY  = (QWidget*)(toolBar_model2 -> child("textLabel_model2_y"));
    static QWidget* labelZ  = (QWidget*)(toolBar_model2 -> child("textLabel_model2_z"));
    static QWidget* lineX   = (QWidget*)(toolBar_model2 -> child("lineEdit_model2_x"));
    static QWidget* lineY   = (QWidget*)(toolBar_model2 -> child("lineEdit_model2_y"));
    static QWidget* lineZ   = (QWidget*)(toolBar_model2 -> child("lineEdit_model2_z"));
    
    if( show )
    {
	labelX -> show();
	lineX  -> show();
	labelY -> show();
	lineY  -> show();
	labelZ -> show();
	lineZ  -> show();
    }
    else
    {
	labelX -> hide();
	lineX  -> hide();
	labelY -> hide();
	lineY  -> hide();
	labelZ -> hide();
	lineZ  -> hide();	
    }    
}

//-------------------------------------------------------------------------
//------------- showModel1Spherocylinder
//-------------------------------------------------------------------------
void MainForm::showModel1Spherocylinder( bool show )
{
    static QWidget* labelD  = (QWidget*)(toolBar_model1 -> child("textLabel_model1_dia"));
    static QWidget* labelL  = (QWidget*)(toolBar_model1 -> child("textLabel_model1_len"));
    static QWidget* lineD   = (QWidget*)(toolBar_model1 -> child("lineEdit_model1_dia"));
    static QWidget* lineL   = (QWidget*)(toolBar_model1 -> child("lineEdit_model1_len"));
    
    if( show )
    {
	labelD -> show();
	lineD  -> show();
	labelL -> show();
	lineL  -> show();
    }
    else
    {
	labelD -> hide();
	lineD  -> hide();
	labelL -> hide();
	lineL  -> hide();	
    }
}

//-------------------------------------------------------------------------
//------------- showModel2Spherocylinder
//-------------------------------------------------------------------------
void MainForm::showModel2Spherocylinder( bool show )
{
    static QWidget* labelD  = (QWidget*)(toolBar_model2 -> child("textLabel_model2_dia"));
    static QWidget* labelL  = (QWidget*)(toolBar_model2 -> child("textLabel_model2_len"));
    static QWidget* lineD   = (QWidget*)(toolBar_model2 -> child("lineEdit_model2_dia"));
    static QWidget* lineL   = (QWidget*)(toolBar_model2 -> child("lineEdit_model2_len"));
    
    if( show )
    {
	labelD -> show();
	lineD  -> show();
	labelL -> show();
	lineL  -> show();
    }
    else
    {
	labelD -> hide();
	lineD  -> hide();
	labelL -> hide();
	lineL  -> hide();	
    }
}

//-------------------------------------------------------------------------
//------------- showModel1Spheroplatelet
//-------------------------------------------------------------------------
void MainForm::showModel1Spheroplatelet( bool show )
{
    static QWidget* labelS1  = (QWidget*)(toolBar_model1 -> child("textLabel_model1_radS1"));
    static QWidget* labelRC  = (QWidget*)(toolBar_model1 -> child("textLabel_model1_radC"));
    static QWidget* lineS1   = (QWidget*)(toolBar_model1 -> child("lineEdit_model1_radS1"));
    static QWidget* lineRC   = (QWidget*)(toolBar_model1 -> child("lineEdit_model1_radC"));
    
    if( show )
    {
	labelS1 -> show();
	lineS1  -> show();
	labelRC -> show();
	lineRC  -> show();
    }
    else
    {
	labelS1 -> hide();
	lineS1  -> hide();
	labelRC -> hide();
	lineRC  -> hide();	
    }
}

//-------------------------------------------------------------------------
//------------- showModel2Spheroplatelet
//-------------------------------------------------------------------------
void MainForm::showModel2Spheroplatelet( bool show )
{
    static QWidget* labelS1  = (QWidget*)(toolBar_model2 -> child("textLabel_model2_radS1"));
    static QWidget* labelRC  = (QWidget*)(toolBar_model2 -> child("textLabel_model2_radC"));
    static QWidget* lineS1   = (QWidget*)(toolBar_model2 -> child("lineEdit_model2_radS1"));
    static QWidget* lineRC   = (QWidget*)(toolBar_model2 -> child("lineEdit_model2_radC"));
    
    if( show )
    {
	labelS1 -> show();
	lineS1  -> show();
	labelRC -> show();
	lineRC  -> show();
    }
    else
    {
	labelS1 -> hide();
	lineS1  -> hide();
	labelRC -> hide();
	lineRC  -> hide();	
    }
}
//-------------------------------------------------------------------------
//------------- showModel1CutSphere
//-------------------------------------------------------------------------
void MainForm::showModel1CutSphere( bool show )
{
    static QWidget* labelS = (QWidget*)(toolBar_model1 -> child("textLabel_model1_radS2"));
    static QWidget* labelC = (QWidget*)(toolBar_model1 -> child("textLabel_model1_cut"));
    static QWidget* lineS  = (QWidget*)(toolBar_model1 -> child("lineEdit_model1_radS2"));
    static QWidget* lineC  = (QWidget*)(toolBar_model1 -> child("lineEdit_model1_cut"));
    
    if( show )
    {
	labelS -> show();
	lineS  -> show();
	labelC -> show();
	lineC  -> show();
    }
    else
    {
	labelS -> hide();
	lineS  -> hide();
	labelC -> hide();
	lineC  -> hide();	
    }
}

//-------------------------------------------------------------------------
//------------- showModel2CutSphere
//-------------------------------------------------------------------------
void MainForm::showModel2CutSphere( bool show )
{
    static QWidget* labelS = (QWidget*)(toolBar_model2 -> child("textLabel_model2_radS2"));
    static QWidget* labelC = (QWidget*)(toolBar_model2 -> child("textLabel_model2_cut"));
    static QWidget* lineS  = (QWidget*)(toolBar_model2 -> child("lineEdit_model2_radS2"));
    static QWidget* lineC  = (QWidget*)(toolBar_model2 -> child("lineEdit_model2_cut"));
    
    if( show )
    {
	labelS -> show();
	lineS  -> show();
	labelC -> show();
	lineC  -> show();
    }
    else
    {
	labelS -> hide();
	lineS  -> hide();
	labelC -> hide();
	lineC  -> hide();	
    }
}

//-------------------------------------------------------------------------
//------------- showModel1Eyelens
//-------------------------------------------------------------------------
void MainForm::showModel1Eyelens( bool show )
{
    static QWidget* labelS3  = (QWidget*)(toolBar_model1 -> child("textLabel_model1_radS3"));
    static QWidget* labelAng = (QWidget*)(toolBar_model1 -> child("textLabel_model1_ang"));
    static QWidget* lineS3   = (QWidget*)(toolBar_model1 -> child("lineEdit_model1_radS3"));
    static QWidget* lineAng  = (QWidget*)(toolBar_model1 -> child("lineEdit_model1_ang"));
    
    if( show )
    {
	labelS3  -> show();
	lineS3   -> show();
	labelAng -> show();
	lineAng  -> show();
    }
    else
    {
	labelS3  -> hide();
	lineS3   -> hide();
	labelAng -> hide();
	lineAng  -> hide();	
    }
}

//-------------------------------------------------------------------------
//------------- showModel2Eyelens
//-------------------------------------------------------------------------
void MainForm::showModel2Eyelens( bool show )
{
    static QWidget* labelS3  = (QWidget*)(toolBar_model2 -> child("textLabel_model2_radS3"));
    static QWidget* labelAng = (QWidget*)(toolBar_model2 -> child("textLabel_model2_ang"));
    static QWidget* lineS3   = (QWidget*)(toolBar_model2 -> child("lineEdit_model2_radS3"));
    static QWidget* lineAng  = (QWidget*)(toolBar_model2 -> child("lineEdit_model2_ang"));
    
    if( show )
    {
	labelS3  -> show();
	lineS3   -> show();
	labelAng -> show();
	lineAng  -> show();
    }
    else
    {
	labelS3  -> hide();
	lineS3   -> hide();
	labelAng -> hide();
	lineAng  -> hide();	
    }
}

//-------------------------------------------------------------------------
//------------- adjustModel1Toolbar
//-------------------------------------------------------------------------
void MainForm::adjustModel1Toolbar(int index)
{
    //cout << "MainForm::adjustModel1Toolbar beg " << index << endl;
    switch( index )
    {
    case 0:
	showModel1Ellipsoid     ( true  );
	showModel1Spherocylinder( false );
	showModel1Spheroplatelet( false );
	showModel1CutSphere     ( false );
	showModel1Eyelens       ( false );	
	break;
    case 1:
	showModel1Ellipsoid     ( false );
	showModel1Spherocylinder( true  );
	showModel1Spheroplatelet( false );
	showModel1CutSphere     ( false );
	showModel1Eyelens       ( false );	
	break;
    case 2:
	showModel1Ellipsoid     ( false );
	showModel1Spherocylinder( false );
	showModel1Spheroplatelet( true  );
	showModel1CutSphere     ( false );
	showModel1Eyelens       ( false );	
	break;
    case 3:
	showModel1Ellipsoid     ( false );
	showModel1Spherocylinder( false );
	showModel1Spheroplatelet( false );
	showModel1CutSphere     ( true  );
	showModel1Eyelens       ( false );	
	break;
    case 4:
	showModel1Ellipsoid     ( false );
	showModel1Spherocylinder( false );
	showModel1Spheroplatelet( false );
	showModel1CutSphere     ( false );
	showModel1Eyelens       ( true  );
	break;
    }
    //cout << "MainForm::adjustModel1Toolbar end" << endl;
}

//-------------------------------------------------------------------------
//------------- adjustModel2Toolbar
//-------------------------------------------------------------------------
void MainForm::adjustModel2Toolbar(int index)
{
    //cout << "MainForm::adjustModel2Toolbar beg " << index << endl;
    switch( index )
    {
    case 0:
	showModel2Ellipsoid     ( true  );
	showModel2Spherocylinder( false );
	showModel2Spheroplatelet( false );
	showModel2CutSphere     ( false );
	showModel2Eyelens       ( false );	
	break;
    case 1:
	showModel2Ellipsoid     ( false );
	showModel2Spherocylinder( true  );
	showModel2Spheroplatelet( false );
	showModel2CutSphere     ( false );
	showModel2Eyelens       ( false );	
	break;
    case 2:
	showModel2Ellipsoid     ( false );
	showModel2Spherocylinder( false );
	showModel2Spheroplatelet( true  );
	showModel2CutSphere     ( false );
	showModel2Eyelens       ( false );	
	break;
    case 3:
	showModel2Ellipsoid     ( false );
	showModel2Spherocylinder( false );
	showModel2Spheroplatelet( false );
	showModel2CutSphere     ( true  );
	showModel2Eyelens       ( false );	
	break;
    case 4:
	showModel2Ellipsoid     ( false );
	showModel2Spherocylinder( false );
	showModel2Spheroplatelet( false );
	showModel2CutSphere     ( false );
	showModel2Eyelens       ( true  );
	break;
    }
    //cout << "MainForm::adjustModel2Toolbar end" << endl;
}

//-------------------------------------------------------------------------
//------------- updateBoxCount
//-------------------------------------------------------------------------
void MainForm::updateBoxCount()
{
    //cout << "MainForm::updateBoxCount beg" << endl;
    glWindow->setBoxCount( lineEdit_boxCount->text().toInt() );
    glWindow->repaint();
    //cout << "MainForm::updateBoxCount end" << endl;
}

//-------------------------------------------------------------------------
//------------- updateTreeView
//-------------------------------------------------------------------------
void MainForm::updateTreeView( QStringList list )
{
    //cout << "MainForm::updateTreeView beg" << endl;
    //cout << "updateTreeView: " << *list.at(0) << " : " << *list.at(1) << endl;
    sshUser = *list.at(0);
    sshHost = *list.at(1);
    setSSHValues();
    //cout << "MainForm::updateTreeView end" << endl;
}

//-------------------------------------------------------------------------
//------------- waitSSH
//-------------------------------------------------------------------------
void MainForm::waitSSH( bool status )
{
    //cout << "MainForm::waitSSH beg" << endl;
    if( status == true )
    {
	statusBar() -> message( "Opening remote directories. Please wait..." );
    }
    else
    {
	statusBar() -> message( "Finished!", 3000 );
    }
    //cout << "MainForm::waitSSH end" << endl;
}

//-------------------------------------------------------------------------
//------------- toolBarUnDocked
//-------------------------------------------------------------------------
/*!
 *  Makes toolbars resizeable or not when docked undocked.
 *  \author Adrian Gabriel 
 *  \date Dec 2005
 */
void MainForm::toolBarUnDocked( QDockWindow::Place p )
{
    //cout << "MainForm::toolBarUnDocked beg" << endl;
    if( p == QDockWindow::InDock )
    {
	toolBar_videoControls -> setResizeEnabled(false);
    }
    else
    {
	toolBar_videoControls -> setResizeEnabled(true);	
    }
    //cout << "MainForm::toolBarUnDocked end" << endl;
}

//-------------------------------------------------------------------------
//------------- openModelsForm
//-------------------------------------------------------------------------
/*!
 *  Openes the models form.
 *  \author Adrian Gabriel 
 *  \date Dec 2005
 */
void MainForm::openModelsForm()
{
    //cout << "MainForm::openModelsForm beg" << endl;
    if( !modelsForm->isShown() )
    {
	modelsForm->show();
	modelsForm->updateModelsVector();
    }
    else
    {
	modelsForm->hide();
    }
    //cout << "MainForm::openModelsForm end" << endl;
}

//-------------------------------------------------------------------------
//------------- updateModelsLineEdits
//-------------------------------------------------------------------------
/*!
 *  Sets values in the model toolbars based on the models vector.
 *  (Needed when the vector is changed by use of the model form for instance...)
 *  \author Adrian Gabriel 
 *  \date Dec 2005
 */
void MainForm::updateModelsLineEdits( bool blockRescale )
{
    //cout << "MainForm::updateModelsLineEdits beg" << endl;
    QString tmp;
    
    comboBox_model1       -> setCurrentItem(     int(models->at(0).at(0 )) );
    lineEdit_model1_x     -> setText( tmp.setNum ( 2*models->at(0).at(1 ), 'f', 2 ) );
    lineEdit_model1_y     -> setText( tmp.setNum ( 2*models->at(0).at(2 ), 'f', 2 ) );
    lineEdit_model1_z     -> setText( tmp.setNum ( 2*models->at(0).at(3 ), 'f', 2 ) );
    lineEdit_model1_dia   -> setText( tmp.setNum ( 2*models->at(0).at(4 ), 'f', 2 ) );
    lineEdit_model1_len   -> setText( tmp.setNum (   models->at(0).at(5 ), 'f', 2 ) );
    lineEdit_model1_radS1 -> setText( tmp.setNum (   models->at(0).at(6 ), 'f', 2 ) );
    lineEdit_model1_radC  -> setText( tmp.setNum (   models->at(0).at(7 ), 'f', 2 ) );
    lineEdit_model1_radS2 -> setText( tmp.setNum (   models->at(0).at(8 ), 'f', 2 ) );
    lineEdit_model1_cut   -> setText( tmp.setNum (   models->at(0).at(9 ), 'f', 2 ) );
    lineEdit_model1_radS3 -> setText( tmp.setNum (   models->at(0).at(10), 'f', 2 ) );
    lineEdit_model1_ang   -> setText( tmp.setNum (   models->at(0).at(11), 'f', 2 ) );
    
    comboBox_model2       -> setCurrentItem(     int(models->at(1).at(0 )) );
    lineEdit_model2_x     -> setText( tmp.setNum ( 2*models->at(1).at(1 ), 'f', 2 ) );
    lineEdit_model2_y     -> setText( tmp.setNum ( 2*models->at(1).at(2 ), 'f', 2 ) );
    lineEdit_model2_z     -> setText( tmp.setNum ( 2*models->at(1).at(3 ), 'f', 2 ) );
    lineEdit_model2_dia   -> setText( tmp.setNum ( 2*models->at(1).at(4 ), 'f', 2 ) );
    lineEdit_model2_len   -> setText( tmp.setNum (   models->at(1).at(5 ), 'f', 2 ) );
    lineEdit_model2_radS1 -> setText( tmp.setNum (   models->at(1).at(6 ), 'f', 2 ) );
    lineEdit_model2_radC  -> setText( tmp.setNum (   models->at(1).at(7 ), 'f', 2 ) );
    lineEdit_model2_radS2 -> setText( tmp.setNum (   models->at(1).at(8 ), 'f', 2 ) );
    lineEdit_model2_cut   -> setText( tmp.setNum (   models->at(1).at(9 ), 'f', 2 ) );
    lineEdit_model2_radS3 -> setText( tmp.setNum (   models->at(1).at(10), 'f', 2 ) );
    lineEdit_model2_ang   -> setText( tmp.setNum (   models->at(1).at(11), 'f', 2 ) );
    
    adjustModel1Toolbar( comboBox_model1->currentItem() );
    adjustModel2Toolbar( comboBox_model2->currentItem() );
    
    if( blockRescale == false )
    {
	forceChangeColorization = true;
	rescaleScene();
	forceChangeColorization = false;
    }
    //cout << "MainForm::updateModelsLineEdits end" << endl;
}

//-------------------------------------------------------------------------
//------------- updateModelsVector
//-------------------------------------------------------------------------
/*!
 *  Sets values in the model vector based on the models toolbars.
 *  \author Adrian Gabriel 
 *  \date Dec 2005
 */
void MainForm::updateModelsVector()
{
    //cout << "MainForm::updateModelsVector beg" << endl;
    
    models->at(0).at(0 ) = comboBox_model1       -> currentItem();
    models->at(0).at(1 ) = lineEdit_model1_x     -> text().toFloat()/2;
    models->at(0).at(2 ) = lineEdit_model1_y     -> text().toFloat()/2;
    models->at(0).at(3 ) = lineEdit_model1_z     -> text().toFloat()/2;
    models->at(0).at(4 ) = lineEdit_model1_dia   -> text().toFloat()/2;
    models->at(0).at(5 ) = lineEdit_model1_len   -> text().toFloat();
    models->at(0).at(6 ) = lineEdit_model1_radS1 -> text().toFloat();
    models->at(0).at(7 ) = lineEdit_model1_radC  -> text().toFloat();
    models->at(0).at(8 ) = lineEdit_model1_radS2 -> text().toFloat();
    models->at(0).at(9 ) = lineEdit_model1_cut   -> text().toFloat();
    models->at(0).at(10) = lineEdit_model1_radS3 -> text().toFloat();
    models->at(0).at(11) = lineEdit_model1_ang   -> text().toFloat();
    
    models->at(1).at(0 ) = comboBox_model2       -> currentItem();
    models->at(1).at(1 ) = lineEdit_model2_x     -> text().toFloat()/2;
    models->at(1).at(2 ) = lineEdit_model2_y     -> text().toFloat()/2;
    models->at(1).at(3 ) = lineEdit_model2_z     -> text().toFloat()/2;
    models->at(1).at(4 ) = lineEdit_model2_dia   -> text().toFloat()/2;
    models->at(1).at(5 ) = lineEdit_model2_len   -> text().toFloat();
    models->at(1).at(6 ) = lineEdit_model2_radS1 -> text().toFloat();
    models->at(1).at(7 ) = lineEdit_model2_radC  -> text().toFloat();
    models->at(1).at(8 ) = lineEdit_model2_radS2 -> text().toFloat();
    models->at(1).at(9 ) = lineEdit_model2_cut   -> text().toFloat();
    models->at(1).at(10) = lineEdit_model2_radS3 -> text().toFloat();
    models->at(1).at(11) = lineEdit_model2_ang   -> text().toFloat();
    
    //cout << "MainForm::updateModelsVector end" << endl;
}

//-------------------------------------------------------------------------
//------------- slotKeyPress
//-------------------------------------------------------------------------
/*!
 *  Rotates the system to a certain position when called.
 *  \param val Determines orientation.
 *  \author Adrian Gabriel 
 *  \date Jan 2007
 */
void MainForm::slotKeyPress( int val )
{
    //cout << "MainForm::slotKeyPress beg" << endl;
    if( val == 0 )
    {
	updateAngleBoxes( 90, 0, 0 ); // x-axis horizontal ->, z-axis up
	rotAngleBoxesChanged();
    }
    else if( val == 1 )
    {
	updateAngleBoxes( 0, 90, 90 ); // y-axis horizontal ->, z-axis up
	rotAngleBoxesChanged();
    }
    else if( val == 2 )
    {
	updateAngleBoxes( 270, 270, 0 ); // z-axis horizontal ->, x-axis up
	rotAngleBoxesChanged();
    }
    else if( val == 3 )
    {
	updateAngleBoxes( 0, 0, 0 ); // x-axis horizontal ->, y-axis up
	rotAngleBoxesChanged();
    }
    //cout << "MainForm::slotKeyPress end" << endl;
}

//-------------------------------------------------------------------------
//------------- dropInRenderArea
//-------------------------------------------------------------------------
/*!
 *  Slot that is called, when a drop occurs in the render area.
 *  \param file path and name of the dropped file
 *  \author Adrian Gabriel 
 *  \date Apr 2007
 */
void MainForm::dropInRenderArea( QString file )
{
    //cout << "MainForm::dropInRenderArea beg" << endl;
    //cout << "dropped: " << file << endl;
    file = parseFilename(file);
    bool reloadSameName = false;
    if( file == cnfFile )
    {
	if( QMessageBox::question( this,
				   "QMGA - Duplicate File",
				   QString("Open file with same name?\n") + file,
				   QMessageBox::Yes, QMessageBox::No ) == QMessageBox::Yes ) 
	{
	    reloadSameName = RELOADSAME;
	}
    }
    
    newInputFile( file, reloadSameName );
    //cout << "MainForm::dropInRenderArea end" << endl;
}

//-------------------------------------------------------------------------
//------------- openFileFromTreeView
//-------------------------------------------------------------------------
/*!
 *  Slot that is called, when a file was double clicked in the tree view (ssh only at the time).
 *  \param file path and name of the double-clicked file
 *  \author Adrian Gabriel 
 *  \date Apr 2007
 */
void MainForm::openFileFromTreeView( QString file )
{
    //cout << "MainForm::openFileFromTreeView beg" << endl;
    dropInRenderArea( file );
    //cout << "MainForm::openFileFromTreeView end" << endl;
}

//-------------------------------------------------------------------------
//------------- updateHistory
//-------------------------------------------------------------------------
/*!
 *  Slot to keep track of the files opened.
 *  \param file path and name of the new file
 *  \author Adrian Gabriel 
 *  \date Apr 2007
 */
void MainForm::updateHistory( QString file )
{
    //cout << "MainForm::updateHistory beg" << endl;
    if( file != openHist_1 && file != openHist_2 && 
	file != openHist_3 && file != openHist_4 )
    {
	openHist_4 = openHist_3;
	openHist_3 = openHist_2;
	openHist_2 = openHist_1;
	openHist_1 = file;
	
	updateOpenHist();
    }
    //cout << "MainForm::updateHistory end" << endl;
}

//-------------------------------------------------------------------------
//------------- setOpenHistText
//-------------------------------------------------------------------------
/*!
 *  Sets menue text of the actions_openHist_x.
 *  \author Adrian Gabriel 
 *  \date Apr 2007
 */
void MainForm::setOpenHistText()
{
    //cout << "MainForm::setOpenHistText beg" << endl;
    action_openHist_1 -> setMenuText( openHist_1 );
    action_openHist_2 -> setMenuText( openHist_2 );
    action_openHist_3 -> setMenuText( openHist_3 );
    action_openHist_4 -> setMenuText( openHist_4 );
    //cout << "MainForm::setOpenHistText end" << endl;
}

//-------------------------------------------------------------------------
//------------- setOpenHistEnable
//-------------------------------------------------------------------------
/*!
 *  Enables/disables the actions_openHist_x.
 *  \author Adrian Gabriel 
 *  \date Apr 2007
 */
void MainForm::setOpenHistEnable()
{
    //cout << "MainForm::setOpenHistEnable beg" << endl;
    if( action_openHist_1 -> menuText() != "." ) { action_openHist_1 -> setEnabled( true  ); }
    else                                         { action_openHist_1 -> setEnabled( false ); }
    
    if( action_openHist_2 -> menuText() != "." ) { action_openHist_2 -> setEnabled( true  ); }
    else                                         { action_openHist_2 -> setEnabled( false ); }
    
    if( action_openHist_3 -> menuText() != "." ) { action_openHist_3 -> setEnabled( true  ); }
    else                                         { action_openHist_3 -> setEnabled( false ); }
    
    if( action_openHist_4 -> menuText() != "." ) { action_openHist_4 -> setEnabled( true  ); }
    else                                         { action_openHist_4 -> setEnabled( false ); }
    //cout << "MainForm::setOpenHistEnable end" << endl;
}

//-------------------------------------------------------------------------
//------------- updateOpenHist
//-------------------------------------------------------------------------
/*!
 *  Helper function to keep action_openHist_x up to date.
 *  \author Adrian Gabriel 
 *  \date Apr 2007
 */
void MainForm::updateOpenHist()
{
    //cout << "MainForm::updateOpenHist beg" << endl;
    setOpenHistText();
    setOpenHistEnable();
    //cout << "MainForm::updateOpenHist end" << endl;
}

//-------------------------------------------------------------------------
//------------- loadOpenHist_1
//-------------------------------------------------------------------------
/*!
 *  Called, when action_openHist_1 was activated. Tries to load the corresponding file.
 *  \author Adrian Gabriel 
 *  \date Apr 2007
 */
void MainForm::loadOpenHist_1()
{
    //cout << "MainForm::loadOpenHist_1 beg" << endl;
    if( newInputFile( action_openHist_1->menuText(), DONTRELOADSAME ) == true )
    {
	lineEditFileOpen -> setText( action_openHist_1 -> menuText() );
    }
    //cout << "MainForm::loadOpenHist_1 end" << endl;    
}

//-------------------------------------------------------------------------
//------------- loadOpenHist_2
//-------------------------------------------------------------------------
/*!
 *  Called, when action_openHist_2 was activated. Tries to load the corresponding file.
 *  \author Adrian Gabriel 
 *  \date Apr 2007
 */
void MainForm::loadOpenHist_2()
{
    //cout << "MainForm::loadOpenHist_2 beg" << endl;
    if( newInputFile( action_openHist_2->menuText(), DONTRELOADSAME ) == true )
    {
	lineEditFileOpen -> setText( action_openHist_2 -> menuText() );
    }
    //cout << "MainForm::loadOpenHist_2 end" << endl;
}

//-------------------------------------------------------------------------
//------------- loadOpenHist_3
//-------------------------------------------------------------------------
/*!
 *  Called, when action_openHist_3 was activated. Tries to load the corresponding file.
 *  \author Adrian Gabriel 
 *  \date Apr 2007
 */
void MainForm::loadOpenHist_3()
{
    //cout << "MainForm::loadOpenHist_3 beg" << endl;
    if( newInputFile( action_openHist_3->menuText(), DONTRELOADSAME ) == true )
    {
	lineEditFileOpen -> setText( action_openHist_3 -> menuText() );
    }
    //cout << "MainForm::loadOpenHist_3 end" << endl;
}

//-------------------------------------------------------------------------
//------------- loadOpenHist_4
//-------------------------------------------------------------------------
/*!
 *  Called, when action_openHist_4 was activated. Tries to load the corresponding file.
 *  \author Adrian Gabriel 
 *  \date Apr 2007
 */
void MainForm::loadOpenHist_4()
{
    //cout << "MainForm::loadOpenHist_4 beg" << endl;
    if( newInputFile( action_openHist_4->menuText(), DONTRELOADSAME ) == true )
    {
	lineEditFileOpen -> setText( action_openHist_4 -> menuText() );
    }
    //cout << "MainForm::loadOpenHist_4 end" << endl;
}

//-------------------------------------------------------------------------
//------------- printHistogram()
//-------------------------------------------------------------------------
/*!
 *  Opens gnuplot with a histogram of the current colors.
 *  Also writes a file in the current working directory called like the file + ".hist"
 *  \author Adrian Gabriel 
 *  \date Apr 2007
 */
void MainForm::printHistogram()
{
    //cout << "MainForm::printHistogram beg" << endl;
    QString hist = QString("./") + cnfFile.section( '/', -1 ) + ".hist";
    ofstream out( hist );
    if( out.is_open() )
    {
	vector<int> colorHist( cnf -> getNumberOfColorsInMap(), 0 );
	Molecule *mc = NULL;
	for( int i = 0; i < cnf -> getNumberOfMolecules(); i++ ) 
	{
	    mc = cnf -> getMolecule(i);
	    
	    colorHist.at( mc -> getColorIndex() )++;
	}	
	float mean = 0.0, num  = 0.0;	
	out << "# file: " << cnfFile << endl;
	out << "#" << right << setw(5) << "Angle" << setw(10) << "Frequency" << endl;
	for( unsigned int i = 0; i<colorHist.size(); i++ )
	{
	    mean += i * colorHist.at(i);
	    num  += colorHist.at(i);
	    out << right << setw(5) << i + 0.5 << setw(3) << "" << left << setw(10) << setprecision(5) << colorHist.at(i)/double(cnf -> getNumberOfMolecules()) << endl;
	}
	out << "# mean = " << mean/num << endl << endl;
	out.close();
	statusBar() -> message( QString("Wrote: ") + hist );
	
	if( gnuplot -> isRunning() == false ) { gnuplot -> start(); }
	if( gnuplot -> isRunning() == true  )
	{
	    // gnuplot -> writeToStdin( "set terminal x11 enhanced\n" );	    
	    gnuplot -> writeToStdin( "set grid; set xlabel '|theta|'; set ylabel 'P(|theta|)'\n" );
	    gnuplot -> writeToStdin( "p '" + hist + "' w histeps\n" );
	    deleteHistogram( hist );
	}
    }
    else
    {
	cerr << "ERROR: Cannot write file: " << hist << endl;
	statusBar() -> message( QString("ERROR: Cannot write file: ") + hist );
    }
    //cout << "MainForm::printHistogram end" << endl;
}

//-------------------------------------------------------------------------
//------------- deleteHistogram
//-------------------------------------------------------------------------
/*!
 *  Opens a dialog and asks if given file is to be deleted.
 *  \param histFile Filename of the histogram file in question.
 *  \author Adrian Gabriel 
 *  \date Apr 2007
 */
void MainForm::deleteHistogram( QString histFile )
{
    //cout << "MainForm::deleteHistogram beg" << endl;
    if( QMessageBox::question( this,
			       "QMGA - show histogram",
			       QString("Delete histogram file:\n")+histFile,
			       QMessageBox::Yes, QMessageBox::No ) == QMessageBox::Yes ) 
    {
	if( QFile::remove(histFile) == false ) 
	{
	    cerr << "Warning: could not remove file: " << histFile << endl;
	}
    }
    //cout << "MainForm::deleteHistogram end" << endl;
}

//-------------------------------------------------------------------------
//------------- updateFps
//-------------------------------------------------------------------------
/*!
 *  Sets a new value to the fps lcdnumber.
 *  \param newVal New fps value to be displayed.
 *  \author Adrian Gabriel 
 *  \date Apr 2007
 */
void MainForm::updateFps( double newVal )
{
    //cout << "MainForm::updateFps beg" << endl;
    lCDNumber_fps -> display( newVal );
    meanFps( newVal, false );
    //cout << "MainForm::updateFps end" << endl;
}

//-------------------------------------------------------------------------
//------------- meanFps
//-------------------------------------------------------------------------
/*!
 *  Calculates the mean of the displayed fps, until true is given for reset.
 *  \param newVal New fps value to take into mean account.
 *  \param reset If reset is true, then the mean calculation gets reset.
 *  \author Adrian Gabriel 
 *  \date Apr 2007
 */
void MainForm::meanFps( double newVal, bool reset )
{
    //cout << "MainForm::meanFps beg" << endl;
    //static double num = 1;
    static double val = 0;
    
    if( reset == true )
    {
	numberMeanFps = 1;
	val = 0;
	lCDNumber_meanFps -> display( 0 );
	
	if( checkBox_benchmark-> isChecked() ) { benchmark(); }
    }
    else
    {
	val  += newVal;
	lCDNumber_meanFps -> display( val/numberMeanFps );
	//cout << num << endl;
	numberMeanFps++;
    }
    //cout << "MainForm::meanFps end" << endl;
}

//-------------------------------------------------------------------------
//------------- benchmark
//-------------------------------------------------------------------------
/*!
 *  randomly turns the system until a given number of refreshes are reached.
 *  \author Adrian Gabriel 
 *  \date Apr 2007
 */
void MainForm::benchmark()
{
    //cout << "MainForm::benchmark beg" << endl;
    static bool init = false;
    int die = 0;
    int angle = 3;
    
    if( init == true )
    {
	srand( static_cast<unsigned>(time(0)) ); 
	init = true;
    }
    
    //cout << endl;
    for( /**/; numberMeanFps<spinBox_benchmark->value()+1; /**/ )
    {
	die = int( 3.0*rand() / (RAND_MAX+1.0) );
	//angle = 1 + int( 3.0*rand() / (RAND_MAX+1.0) ); // angle from 1 to 3 deg
	
	if( die == 0 )
	{
	    glWindow->rotateXAxis( angle );
	    //cout << "X: " << angle << endl;
	}
	else if( die == 1 )
	{
	    glWindow->rotateYAxis( angle );
	    //cout << "Y: " << angle << endl;
	}
	else if( die == 2 )
	{
	    glWindow->rotateZAxis( angle );
	    //cout << "Z: " << angle << endl;
	}
	else
	{
	    cerr << "Beware! Expect 0-2 but got '" << die << "' that must not be!" << endl;
	}
    }
    //cout << "MainForm::benchmark end" << endl;
}

//-------------------------------------------------------------------------
//------------- resetMeanFps
//-------------------------------------------------------------------------
/*!
 *  Resets the mean fps calculation
 *  \author Adrian Gabriel 
 *  \date Apr 2007
 */
void MainForm::resetMeanFps()
{
    //cout << "MainForm::resetMeanFps beg" << endl;
    meanFps( 0, true );
    //cout << "MainForm::resetMeanFps end" << endl;
}

//-------------------------------------------------------------------------
//------------- setSSHValues
//-------------------------------------------------------------------------
/*!
 *  (re)sets ssh connection values
 *  \author Adrian Gabriel 
 *  \date Apr 2007
 */
void MainForm::setSSHValues()
{
    //cout << "MainForm::setSSHValues beg" << endl;
    
    if( treeView -> isSSHSet() == false )
    {
	treeView -> setSSH(); 
    }
    treeView -> setUser( sshUser );
    treeView -> setHost( sshHost );
    
    //cout << sshForm << " : " << sshUser+"@"+sshHost << endl;
    sshForm -> showUsedUserAndHost(sshUser+"@"+sshHost);
    
    if( sshRoot != 0 )
    {
	delete sshRoot;
	sshRoot = 0;
    }
    
    sshRoot = new SSHDirectory( treeView );
    
    if( sshRoot != 0 )
    {
	sshRoot -> setOpen( false );
    }
    
    //cout << "MainForm::setSSHValues end" << endl;
}

//-------------------------------------------------------------------------
//------------- initTreeWindow
//-------------------------------------------------------------------------
/*!
 *  initializes the Tree DockWindow. Be sure to call it after sshUser and sshHost are set!
 *  \author Adrian Gabriel 
 *  \date Apr 2007
 */
void MainForm::initTreeWindow()
{
    //cout << "MainForm::initTreeWindow beg" << endl;
    treeWindow = new QDockWindow( QDockWindow::InDock, this );
    treeWindow -> setCaption( "QMGA -- Tree Sidebar" );
    treeWindow -> setResizeEnabled( true );
    treeWindow -> setAcceptDrops( false );
    treeWindow -> setCloseMode( QDockWindow::Always );
    moveDockWindow( treeWindow, Qt::DockLeft );
    
    QSettings settings;
    settings.insertSearchPath( QSettings::Windows, WINDOWS_REGISTRY );
    if( settings.readBoolEntry( APP_KEY + "TreeSideBar", false ) == true )
    {
	treeWindow -> show();
	action_toggleTreeView  -> setOn( true );
    }
    else
    {
	treeWindow -> hide();
	action_toggleTreeView -> setOn( false );
    }
    
    treeView  = new DirectoryView( treeWindow, "dirview"   );
    treeView -> setAcceptDrops( false );
    
    sshForm = new SSHForm( treeWindow, "sshform" );
    sshForm -> setUser( sshUser );
    sshForm -> setHost( sshHost );
    treeWindow -> boxLayout() -> addWidget( sshForm  );
    treeWindow -> boxLayout() -> addWidget( treeView );
    
    treeView -> addColumn( "Name" );
    treeView -> addColumn( "Type" );
    treeView -> setTreeStepSize( 20 );
    treeView -> setResizeMode( DirectoryView::AllColumns );
    
    const QFileInfoList* roots = QDir::drives();
    QPtrListIterator<QFileInfo> i(*roots);
    QFileInfo* fi;
    while( (fi = *i) )
    {
	++i;
	Directory * root = new Directory( treeView, fi -> filePath() );
	if( roots -> count() <= 1 )
	{
	    root -> setOpen( false );
	}
    }
    setSSHValues();
    
    connect( treeWindow, SIGNAL(visibilityChanged(bool))         , action_toggleTreeView, SLOT(setOn(bool)) );
    connect( treeView  , SIGNAL(openSSH(bool))                   , this   , SLOT(waitSSH(bool)) );
    connect( treeView  , SIGNAL(currentDirectoryChanged(QString)), sshForm, SLOT(setCurrent(QString)) );
    connect( treeView  , SIGNAL(doubleClickedFile(QString))      , this   , SLOT(openFileFromTreeView(QString)) );
    connect( sshForm   , SIGNAL(sshChanged(QStringList))         , this   , SLOT(updateTreeView(QStringList)) );
    connect( sshForm   , SIGNAL(sshRefreshCurrentDir())          , this   , SLOT(refreshCurrentDir()) );
    connect( this      , SIGNAL(newTmpFile())                    , sshForm, SLOT(calculateTmpSize()) );
    //cout << "MainForm::initTreeWindow end" << endl;
}

//-------------------------------------------------------------------------
//------------- initSliceWindow
//-------------------------------------------------------------------------
/*!
 *  \author Adrian Gabriel 
 *  \date Apr 2007
 */
void MainForm::initSliceWindow()
{
    //cout << "MainForm::initSliceWindow beg" << endl;
    sliceWindow = new QDockWindow( QDockWindow::InDock, this );
    sliceWindow -> setCaption( "QMGA -- Slice" );
    sliceWindow -> setResizeEnabled( true );
    sliceWindow -> setAcceptDrops( false );
    sliceWindow -> setCloseMode( QDockWindow::Always );
    
    sliceWindow -> adjustSize();
    
    moveDockWindow( sliceWindow, Qt::DockLeft );
    
    QSettings settings;
    settings.insertSearchPath( QSettings::Windows, WINDOWS_REGISTRY );
    if( settings.readBoolEntry( APP_KEY + "SliceSideBar", false ) == true )
    {
	sliceWindow -> show();
	action_toggleSliceWindow -> setOn( true );
    }
    else
    {
	sliceWindow -> hide();
	action_toggleSliceWindow -> setOn( false );
    }
    
    sliceForm = new SliceForm( sliceWindow, "sliceform" );
    sliceForm -> setGlWindow( glWindow );
    connect( this, SIGNAL(cnfChanged(CnfFile*)), sliceForm, SLOT(setCnfFile(CnfFile*)) );
    connect( sliceForm, SIGNAL(sliceClosed()), this, SLOT(toggleSliceAction()) );
    emit cnfChanged( cnf );
    
    sliceWindow -> boxLayout() -> addWidget( sliceForm );
    
    connect( sliceWindow, SIGNAL(visibilityChanged(bool)), action_toggleSliceWindow, SLOT(setOn(bool)) );
    //cout << "MainForm::initSliceWindow end" << endl;
}

//-------------------------------------------------------------------------
//------------- refreshCurrentDir
//-------------------------------------------------------------------------
/*!
 *  initiates a reload of the contents of the current dir set in the treeview (ssh only just now)
 *  \author Adrian Gabriel 
 *  \date Apr 2007
 */
void MainForm::refreshCurrentDir()
{
    //cout << "MainForm::refreshCurrentDir beg" << endl;
    treeView -> refreshCurrentDirectory();
    //cout << "MainForm::refreshCurrentDir end" << endl;
}

//-------------------------------------------------------------------------
//------------- SetModelsFile
//-------------------------------------------------------------------------
void MainForm::setModelsFile( string file )
{
    modelsFile = file;
}

//-------------------------------------------------------------------------
//------------- LoadModels
//-------------------------------------------------------------------------
void MainForm::loadModels()
{
    //cout << "MainForm::LoadModels beg" << endl;
    if( modelsFile == "" ) { cerr << "Warning: Empty modelsFile name in loadModels() ...do nothing" << endl; return; }
    else
    {
	if( !QFile::exists(modelsFile) ) { cerr << "Warning: modelsFile >" << modelsFile << "< does not exist. ...do nothing" << endl; return; }
    }
    
    uint modelsNeeded = 2;
    
    if( cnf != NULL )
    {
	uint modelsInCnf = cnf -> getNumberOfTypes();
	modelsNeeded = ( modelsInCnf > 2 ? modelsInCnf : 2 );
    }
    
    cout << "Loading models file: " << modelsFile << "\n";
    // read from input
    std::ifstream in( modelsFile.c_str() );
    if( !in.is_open() ) { cerr << "Error! Cannot open file: " << modelsFile << "Using default\n" << flush; }
    else
    {
	models->clear();
	
	vector<float> model;
	stringstream m;
	string currentLine;
	float tmp;
	
	while( getline( in, currentLine ) )
	{
	    model.clear();
	    m.clear();
	    m.str(currentLine);
	    
	    while( m >> tmp ) { model.push_back(tmp); }
	    
	    if( model.size() == 17 )
	    {
		model.at(1) *= 0.5;
		model.at(2) *= 0.5;
		model.at(3) *= 0.5;
		model.at(4) *= 0.5;
		models -> push_back(model);
	    }
	    else
	    {
		if( model.size() != 0 ) // otherwise it is probably just an empty line at the end of the file, like some editors append automatically...
		{
		    cerr << "Warning: model definition file contains corrupt lines!" << endl;
		}
	    }
	    
	    // model.at(0 )   ->  model type (0: ellipsoid; 1: spherocylinder; 2: spheroplatelet; 3: cut sphere; 4:eyelens)
	    // model.at(1 )   ->  x/2 (sphere)
	    // model.at(2 )   ->  y/2 (sphere)
	    // model.at(3 )   ->  z/2 (sphere)
	    // model.at(4 )   ->  diameter (shpereocylinder)
	    // model.at(5 )   ->  length   (shpereocylinder cylinder part)
	    // model.at(6 )   ->  radius sphere (spheroplatelet)
	    // model.at(7 )   ->  radius circle (spheroplatelet)
	    // model.at(8 )   ->  radius sphere                 (cut sphere)
	    // model.at(9 )   ->  cut planes distance to center (cut sphere)
	    // model.at(10)  ->  radius underlying sphere (eyelens)
	    // model.at(11)  ->  opening angle (eyelens)
	    // model.at(12)  ->  wireframe
	    // model.at(13)  ->  force model color	      
	    // model.at(14)  ->  r
	    // model.at(15)  ->  g
	    // model.at(16)  ->  b
	}
	
	if( models -> size() < modelsNeeded )
	{
	    cerr << "Warning: Not enough model definitions in file (at least " << modelsNeeded << " are required)! .. adding dummies." << endl;
	    addDummyModels( modelsNeeded - models->size() );
	}
	
	// At this point the models vector is at least filled with two correct models.
	modelsForm -> setModelsVector( models );
	modelsForm -> commitChanges();
    }
    //cout << "MainForm::LoadModels end" << endl;
}

//-------------------------------------------------------------------------
//------------- SetVideoParameters
//-------------------------------------------------------------------------
void MainForm::setVideoParameters( string videoFile, int startValue, int stopValue, int stepValue, int digitsValue )
{
    
    lineEdit_videoFile -> setText(videoFile);
    //lineEdit_videoStart     -> setText( enabled );
    
    std::stringstream itoaStream;
    
    itoaStream.str("");
    itoaStream << startValue;
    lineEdit_videoStart      -> setText( itoaStream.str() );
    
    itoaStream.str("");
    itoaStream << stopValue;
    lineEdit_videoStop      -> setText( itoaStream.str() );
    
    itoaStream.str("");
    itoaStream << stepValue;
    lineEdit_videoStep      -> setText( itoaStream.str() );
    
    itoaStream.str("");
    itoaStream << digitsValue;
    lineEdit_videoNumDigits -> setText( itoaStream.str() );
}

//-------------------------------------------------------------------------
//------------- loadModelsFile
//-------------------------------------------------------------------------
void MainForm::openModelsFile()
{
    //cout << "MainForm::loadModelsFile beg" << endl;
    static QString newFile = "";
    newFile = QFileDialog::getOpenFileName( newFile=="" ? "./" : newFile, "All Files (*)", this, "open file", "Choose model definitions file" );   
    if( newFile != "" )
    {
	setModelsFile( newFile );
	loadModels();
    }
    //cout << "MainForm::loadModelsFile end" << endl;
}

//-------------------------------------------------------------------------
//------------- saveModelsFile
//-------------------------------------------------------------------------
void MainForm::saveModelsFile()
{
    //cout << "MainForm::saveModelsFile beg" << endl;
    if( !QFile::exists(lastModelSaveFolder) ) { lastModelSaveFolder = "./"; }
    static QString file = lastModelSaveFolder;
    file = QFileDialog::getSaveFileName( file=="" ? "./" : file,
					 "All Files (*)",
					 this,
					 "save file",
					 "Choose Save Filename" );
    
    if( file.isEmpty() )
    {
	file = lastModelSaveFolder;
	return;
    }
    
    lastModelSaveFolder = QFileInfo( file ).dirPath();
    
    bool saveTest = ( file == "" ? false : true );
    
    switch( QMessageBox::information( this, "QMGA -- Information",
				      saveTest ? "Save file?\n"+file : "Cannot save file, name empty.",
				      QMessageBox::Cancel|QMessageBox::Default,
				      saveTest ? QMessageBox::Ok : QMessageBox::NoButton,
				      QMessageBox::NoButton) )
    {
    case QMessageBox::Ok :
	modelsForm -> writeFile( file );
	break;
    case QMessageBox::Cancel : 
	statusBar()->message( "save aborted!", 3000 ); 
	break;
    }
    //cout << "MainForm::saveModelsFile end" << endl;
}


//-------------------------------------------------------------------------
//------------- mousePressEvent
//-------------------------------------------------------------------------
void MainForm::updateTranslationBoxes( vector<float> vecTranslation )
{
    //cout << "MainForm::updateTranslationBoxes beg" << endl;
    
    spinBox_translateX->setValue( spinBox_translateX->value() + int(vecTranslation.at(0)) );
    spinBox_translateY->setValue( spinBox_translateY->value() + int(vecTranslation.at(1)) );
    spinBox_translateZ->setValue( spinBox_translateZ->value() + int(vecTranslation.at(2)) );
    
    //cout << "MainForm::updateTranslationBoxes end" << endl;
}


/*
//-------------------------------------------------------------------------
//------------- mousePressEvent
//-------------------------------------------------------------------------
void MainForm::mousePressEvent( QMouseEvent * e ) {
    cout << "MainForm::mousePressEvent() beg" << endl;
    if( e->button() == Qt::RightButton )
    {
	e->accept();
	//dragging = TRUE;
	lastDragPoint = e->pos();		// add point
    }
    else
    {
	e->ignore();
    }
    cout << "MainForm::mousePressEvent() end" << endl;    
}

//-------------------------------------------------------------------------
//------------- mouseReleaseEvent
//-------------------------------------------------------------------------
void MainForm::mouseReleaseEvent( QMouseEvent * e ) {
    cout << "MainForm::mouseReleaseEvent() beg" << endl;    
    if( e->button() == Qt::RightButton )
    {
	e->accept();
	//dragging = FALSE;				// done recording points
    }
    else
    {
	e->ignore();
    }
    
    cout << "MainForm::mouseReleaseEvent() end" << endl;    
}


//-------------------------------------------------------------------------
//------------- mouseMoveEvent
//-------------------------------------------------------------------------
void MainForm::mouseMoveEvent( QMouseEvent * e) {
    cout << "MainForm::mouseMoveEvent() beg" << endl;    
	e->accept();
//    if ( dragging )
//    {
	QPoint point = e->pos();		// add point
//	
	int deltax = point.x() - lastDragPoint.x();
	int deltay = point.y() - lastDragPoint.y();
	
	cout << "deltax =" << deltax << endl;
	cout << "deltay =" << deltay << endl;
	
//	// Das ist nicht vertauscht, das ist korrekt!
//	YMousedelta += deltax;
//	XMousedelta += deltay;
//	repaint();
	lastDragPoint = point;
//    }
//    else
//    {
//	e->ignore();
//    }
    cout << "MainForm::mouseMoveEvent() end" << endl;    
}
*/
/*
void MainForm::paintEvent( QPaintEvent * e )
{
    cout << "MainForm::paintEvent()" << endl;
    QWidget::paintEvent(e);
}
*/



