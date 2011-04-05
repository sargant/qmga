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
 ** These will automatically be called by the forms constructor and
 ** destructor.
 *****************************************************************************/

//-------------------------------------------------------------------------
// this function is automatically called by the objects constructor. (default qt behaviour)
void mainWiget::init()
{
    glWindow  = new Renderer   ( &mainWiget::updateAngleBoxes, &mainWiget::updateZoom, this, glFrame, "sdfvsd" );
    glLayout  = new QGridLayout( glFrame        );
    glLayout -> addWidget      ( glWindow, 0, 0 );
    blockUpdate = false;
    connect(glWindow, SIGNAL(sizeChanged()), this, SLOT(keepAspectRatio()));
    
    qtTimer     = 0;
    oldValueDialY = 0;
    
    sliderUpdate = true;
    
    radioButtonForward -> setChecked( true );
    
    float  x_r, x_g, x_b, y_r, y_g, y_b, z_r, z_g, z_b;
    glWindow -> getAxisColors( x_r, x_g, x_b, y_r, y_g, y_b, z_r, z_g, z_b );
    
    middle = new vector<vector<float>*> ();//(numMolecules);
    rot    = new vector<vector<float>*> ();//(numMolecules);
    color  = new vector<vector<float>*> ();//(numMolecules);
    
    pushButtonX -> setPaletteBackgroundColor( QColor(int(x_r*255),int(x_g*255),int(x_b*255)) );
    pushButtonY -> setPaletteBackgroundColor( QColor(int(y_r*255),int(y_g*255),int(y_b*255)) );
    pushButtonZ -> setPaletteBackgroundColor( QColor(int(z_r*255),int(z_g*255),int(z_b*255)) );
    
    lineEditScaleX -> setText( "1.0" );
    lineEditScaleY -> setText( "1.0" );
    lineEditScaleZ -> setText( "0.2" );
    
    lineEditStart -> setText( "0"      );
    lineEditStop  -> setText( "100000" );
    lineEditStep  -> setText( "100"    );
    
    lineEditSnapShot -> setText( "snapshot" );
    lineEditPix1     -> setText( "1000"     );
    lineEditPix2     -> setText( "1000"     );
    
    lineEditColorAxisX_user->setText( "0.00" );
    lineEditColorAxisY_user->setText( "0.00" );
    lineEditColorAxisZ_user->setText( "1.00" );
    
    pushButtonVidPause -> setEnabled( false );
    rescaleScene();    
}

//-------------------------------------------------------------------------
// slot to set input file and colormap before the object is initially drawn
void mainWiget::setInputFiles( QString file, QString color )
{
    cnfFile  = file;
    colorMap = color;
    lineEditOpenFile -> setText( file );
}

//-------------------------------------------------------------------------
// overridden function to construct the scenegraph before the window is drawn
void mainWiget::show()
{
    initializeScene();
    QWidget::show();
}

//-------------------------------------------------------------------------
// slot to open a system native file browser. Selected file will be set as the new configuration file
void mainWiget::openFile()
{
    QString newFile = QFileDialog::getOpenFileName( "./", "All Files (*)", this, "open file", "Choose configuration file" );   
    if( newFile != "" )
    {
	lineEditOpenFile -> setText( newFile );      
	newInputFile( newFile );
    }
}

//-------------------------------------------------------------------------
// function that can always be called, when the configuration file changes.
// sets class member that holds actual configuration file
// reloads new config file
// rebuilds the scenegraph to make changes visible
void mainWiget::newInputFile( string newFile )
{
    cout << "new file" << endl;
    if( cnfFile != newFile && newFile != "" )
    {
	cnfFile = newFile;
	if( (cnf -> reloadCnfFile( newFile )) == true )
	{
	    buildScene(true);
	    glWindow -> repaint();
	}
	else { cerr << "Beware! : error while reloading cnf file!" << endl; }
    }
}

//-------------------------------------------------------------------------
// changes the scale of the axes
// rerenderes the gl window
void mainWiget::rescaleScene()
{
    
    int selectedQual = comboBox_quality->currentItem();
    switch(selectedQual) {
    case 0:
	glWindow->setModelParams(6,1,4,0, lineEditScaleX -> text().toFloat(), lineEditScaleZ -> text().toFloat(), lineEditScaleY -> text().toFloat(),5);
	break;
    case 1:
	glWindow->setModelParams(8,2,5,1, lineEditScaleX -> text().toFloat(), lineEditScaleZ -> text().toFloat(), lineEditScaleY -> text().toFloat(),5);
	break;
    case 2:
	glWindow->setModelParams(12,4,7,2, lineEditScaleX -> text().toFloat(), lineEditScaleZ -> text().toFloat(), lineEditScaleY -> text().toFloat(),5);
	break;
    case 3:
	glWindow->setModelParams(24,8,14,4, lineEditScaleX -> text().toFloat(), lineEditScaleZ -> text().toFloat(), lineEditScaleY -> text().toFloat(),5);
	break;
    case 4:
	glWindow->setModelParams(48,12,23,6, lineEditScaleX -> text().toFloat(), lineEditScaleZ -> text().toFloat(), lineEditScaleY -> text().toFloat(),5);
	break;
    }
}

//-------------------------------------------------------------------------
// sets up the scene for the first time
void mainWiget::initializeScene()
{
    cnf = new CnfFile( cnfFile, colorMap );
    cnf -> setUseUserDefinedDirector( radioButton_colorAxis_user -> isChecked() );
    qtTimer = new QTimer( this );
    connect( qtTimer ,SIGNAL( timeout() ), this, SLOT( videoNextScene () ) );
    
    setColorMap();
    buildScene(true);
}

//-------------------------------------------------------------------------
// this function builds the subscenegraph that holds the actual molecule data
void mainWiget::buildScene(bool resetDistance)
{
    QString tmp = "";
    tmp = tmp.setNum ( cnf->getDirectorX(), 'f', 2 );
    lineEditColorAxisX_director->setText( tmp );
    tmp = tmp.setNum ( cnf->getDirectorY(), 'f', 2 );
    lineEditColorAxisY_director->setText( tmp );
    tmp = tmp.setNum ( cnf->getDirectorZ(), 'f', 2 );
    lineEditColorAxisZ_director->setText( tmp );
    
    if( cnf->getBoxX() != 0.0 && cnf->getBoxY() != 0.0 && cnf->getBoxZ() != 0.0 && sliderUpdate == true )
    {
	adjustSliceSliders();
    }
    
    numMolecules = cnf -> getNumberOfMolecules();
    if( radioButtonOriginal -> isChecked() == false ) { cnf -> foldMoleculesToBoundingBox(); } 
    if( numMolecules > 0 )
    {
	fillModelVectors();
	glWindow->setModels(middle, rot, color);
	glWindow->setBoundingBox(cnf->getBoxX(),cnf->getBoxY(),cnf->getBoxZ(), resetDistance);
	glWindow->repaint();
    }
}

//-------------------------------------------------------------------------
void mainWiget::setColorMap()
{
    vector<float *> *farben = new vector <float *>();
    for (int i = 0; i < cnf->getNumberOfColorsInMap(); i++) 
    {
	float *tmp = new float[3];
	tmp[0] = (float) (cnf->getRedAt(i)/255.0f);
	tmp[1] = (float) (cnf->getGreenAt(i)/255.0f);
	tmp[2] = (float) (cnf->getBlueAt(i)/255.0f);
	farben->push_back(tmp);
    }
    glWindow->setColorMap(farben);
}

//-------------------------------------------------------------------------
void mainWiget::fillModelVectors()
{
    cout << "FillModelVectors -> start" << endl;
    int old = middle -> size();
    
    if( numMolecules != old )
    {
	if( numMolecules < old )
	{
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
    
    Molecule *mc = NULL;
    for( int i = 0; i < numMolecules; i++ ) 
    {
	mc = cnf->getMolecule(i);
	
	middle->at(i)->at(0) = (float) (mc -> getPositionX());
	middle->at(i)->at(1) = (float) (mc -> getPositionY());
	middle->at(i)->at(2) = (float) (mc -> getPositionZ());
	
	color->at(i)->at(0) = (float) (mc -> getRed()/255.0f);
	color->at(i)->at(1) = (float) (mc -> getGreen()/255.0f);
	color->at(i)->at(2) = (float) (mc -> getBlue()/255.0f);
	
	rot->at(i)->at(0) = (float) (mc -> getOrientationX());
	rot->at(i)->at(1) = (float) (mc -> getOrientationY());
	rot->at(i)->at(2) = (float) (mc -> getOrientationZ());
	// Optimiert wegen 0 1 0
	// Winkel
	rot->at(i)->at(3) = (float) ((acos(rot->at(i)->at(1) / sqrt(pow(rot->at(i)->at(0),2) + pow(rot->at(i)->at(1),2) + pow(rot->at(i)->at(2),2))))/ M_PI * 180.0);
	rot->at(i)->at(0) = rot->at(i)->at(2);
	rot->at(i)->at(1) = 0;
	rot->at(i)->at(2) = -(float) (mc -> getOrientationX());
    }
    cout << "FillModelVectors -> stop" << endl;
}

//-------------------------------------------------------------------------
void mainWiget::changeColorisationAxis()
{
    float x=0,y=0,z=0;
    cnf -> setUseUserDefinedDirector( radioButton_colorAxis_user -> isChecked() );
    x = lineEditColorAxisX_user->text().toFloat();
    y = lineEditColorAxisY_user->text().toFloat();
    z = lineEditColorAxisZ_user->text().toFloat();
    cnf -> setUserDefinedDirector( x, y, z );
    cnf -> colorizeMolecules();
    
    Molecule *mc = NULL;
    for( unsigned int i = 0; i < color -> size(); i++ )
    {
	mc = cnf->getMolecule(i);
	color->at(i)->at(0) = (float) (mc -> getRed()/255.0f);
	color->at(i)->at(1) = (float) (mc -> getGreen()/255.0f);
	color->at(i)->at(2) = (float) (mc -> getBlue()/255.0f);
    }
    glWindow->repaint();
}

//-------------------------------------------------------------------------
// slot to update the scene, when a new configuration file was entered by hand into the line edit.
// is called, when enter is pressed (from inside the line edit, of course)
void mainWiget::lineEditNewFileReturnPressed()
{
    QString newFile = lineEditOpenFile -> text();
    newInputFile( newFile );
}

//-------------------------------------------------------------------------
void mainWiget::updateAngleBoxes( int alpha, int beta, int gamma )
{
    blockUpdate = true;
    spinBoxAlpha -> setValue(alpha);
    spinBoxBeta  -> setValue(beta);
    spinBoxGamma -> setValue(gamma);
    blockUpdate = false;
}

//-------------------------------------------------------------------------
void mainWiget::updateAngleBoxesRenderer()
{
    if(!blockUpdate)
    {
	glWindow->setAngles( spinBoxAlpha -> value(), spinBoxBeta -> value(), spinBoxGamma -> value() );
    }
}

//-------------------------------------------------------------------------
void mainWiget::setRender( bool state )
{
    glWindow->setFullRender( !state );
}

//-------------------------------------------------------------------------
void mainWiget::changeZoom()
{
    glWindow -> changeZoomAbsolute( spinBoxZoom -> value() );
}

//-------------------------------------------------------------------------
void mainWiget::updateZoom( int val )
{
    spinBoxZoom -> setValue( val );
}

//-------------------------------------------------------------------------
void mainWiget::setColorMap( bool b )
{
    glWindow -> setDrawColorMap( b );
}

//-------------------------------------------------------------------------
void mainWiget::setAxes( bool b )
{
    glWindow -> setDrawAxis( b );
}

//-------------------------------------------------------------------------
void mainWiget::setFrameColor( QPushButton *button )
{
    QColor c, x, y, z;
    c = QColorDialog::getColor( c, this );
    if ( c.isValid() ) 
    {
	button -> setPaletteBackgroundColor ( c );
	x = pushButtonX -> paletteBackgroundColor();
	y = pushButtonY -> paletteBackgroundColor();
	z = pushButtonZ -> paletteBackgroundColor();
	glWindow->setAxisColors( float(x.red())/255.0, float(x.green())/255.0, float(x.blue())/255.0,
				 float(y.red())/255.0, float(y.green())/255.0, float(y.blue())/255.0,
				 float(z.red())/255.0, float(z.green())/255.0, float(z.blue())/255.0 );
    }    
}

//-------------------------------------------------------------------------
void mainWiget::colorFrameX()
{
    setFrameColor( pushButtonX );
}

//-------------------------------------------------------------------------
void mainWiget::colorFrameY()
{
    setFrameColor( pushButtonY );
}

//-------------------------------------------------------------------------
void mainWiget::colorFrameZ()
{
    setFrameColor( pushButtonZ );
}

//-------------------------------------------------------------------------
void mainWiget::setOptimized( bool on )
{
    glWindow->setOptimized(on);
}

//-------------------------------------------------------------------------
void mainWiget::setSliced( bool on )
{
    glWindow->setDrawSlices(on);
}

//-------------------------------------------------------------------------
void mainWiget::rotx()
{
    static int oldValue = 0;
    //glWindow->rotateXAxis(lineEdit_rotx->text().toFloat());
    glWindow->rotateXAxis( oldValue - dial_rotx->value() );
    oldValue = dial_rotx->value();    
}

//-------------------------------------------------------------------------
void mainWiget::roty()
{
    static int oldValue = 0;
    glWindow->rotateYAxis( oldValue - dial_roty->value() );
    oldValue = dial_roty->value();
}

//-------------------------------------------------------------------------
void mainWiget::rotz()
{
    static int oldValue = 0;
    glWindow->rotateZAxis( oldValue - dial_rotz->value() );
    oldValue = dial_rotz->value();
}

//-------------------------------------------------------------------------
void mainWiget::rotxEnter()
{
    glWindow->rotateXAxis( lineEdit_rotx->text().stripWhiteSpace().toFloat() );
}

//-------------------------------------------------------------------------
void mainWiget::rotyEnter()
{
    glWindow->rotateYAxis( lineEdit_roty->text().stripWhiteSpace().toFloat() );
}

//-------------------------------------------------------------------------
void mainWiget::rotzEnter()
{
    glWindow->rotateZAxis( lineEdit_rotz->text().stripWhiteSpace().toFloat() );
}

//-------------------------------------------------------------------------
void mainWiget::togglePixelLineEdits()
{
    if( lineEditPix2->isReadOnly() )
    {
	lineEditPix1->setReadOnly(true);
	lineEditPix2->setReadOnly(false);
	lineEditPix1->setFrameStyle( QFrame::Box | QFrame::Sunken );
	lineEditPix2->setFrameStyle( QFrame::Box | QFrame::Raised );
    }
    else
    {
	lineEditPix1->setReadOnly(false);
	lineEditPix2->setReadOnly(true);
	lineEditPix1->setFrameStyle( QFrame::Box | QFrame::Raised );
	lineEditPix2->setFrameStyle( QFrame::Box | QFrame::Sunken );	
    }
}

//-------------------------------------------------------------------------
void mainWiget::keepAspectRatio()
{
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
}

//-------------------------------------------------------------------------
void mainWiget::createScreenshot()
{
    if( lineEditSnapShot -> text() == "" )
    {
	QString file = QFileDialog::getSaveFileName( "./", 
						     "PNG (*.png *.PNG)",
						     this,
						     "save file",
						     "Coose Filename - extension (.png or .PNG) will be truncated"  );   
	if( file.endsWith(".png") || file.endsWith(".PNG") )
	{
	    file.truncate( file.length()-4 );
	}
	lineEditSnapShot -> setText( file );
    }
    glWindow->getSnapShot(lineEditPix1->text().toInt(),lineEditPix2->text().toInt(),lineEditSnapShot->text()+".png");
}

//-------------------------------------------------------------------------
void mainWiget::updateSliderLCDs()
{
    lCDNumber_slice_x_left ->display( slider_slice_x_left ->value() );
    lCDNumber_slice_x_right->display( slider_slice_x_right->value() );
    
    lCDNumber_slice_y_left ->display( slider_slice_y_left ->value() );
    lCDNumber_slice_y_right->display( slider_slice_y_right->value() );
    
    lCDNumber_slice_z_left ->display( slider_slice_z_left ->value() );
    lCDNumber_slice_z_right->display( slider_slice_z_right->value() );
    
    setSliceBounds();
}

//-------------------------------------------------------------------------
void mainWiget::adjustSliceSliders()
{
    slider_slice_x_left -> setRange( -int(cnf->getBoxX()*10), int(cnf->getBoxX()*10)-1 );
    slider_slice_x_left -> setValue( -int(cnf->getBoxX()*10) );
    slider_slice_x_right-> setRange( -int(cnf->getBoxX()*10)+1, int(cnf->getBoxX()*10) );
    slider_slice_x_right-> setValue(  int(cnf->getBoxX()*10) );
    
    slider_slice_y_left -> setRange( -int(cnf->getBoxY()*10), int(cnf->getBoxY()*10)-1 );
    slider_slice_y_left -> setValue( -int(cnf->getBoxY()*10) );
    slider_slice_y_right-> setRange( -int(cnf->getBoxY()*10)+1, int(cnf->getBoxY()*10) );
    slider_slice_y_right-> setValue(  int(cnf->getBoxY()*10) );
    
    slider_slice_z_left -> setRange( -int(cnf->getBoxZ()*10), int(cnf->getBoxZ()*10)-1 );
    slider_slice_z_left -> setValue( -int(cnf->getBoxZ()*10) );
    slider_slice_z_right-> setRange( -int(cnf->getBoxZ()*10)+1, int(cnf->getBoxZ()*10) );
    slider_slice_z_right-> setValue(  int(cnf->getBoxZ()*10) );
    
    updateSliderLCDs();
}

//-------------------------------------------------------------------------
void mainWiget::checkSliderConsistencyRight()
{
    if( slider_slice_x_right->value() <= slider_slice_x_left->value() )
    {
	slider_slice_x_left->setValue( slider_slice_x_right->value()-1 ); 
    }
    if( slider_slice_y_right->value() <= slider_slice_y_left->value() )
    {
	slider_slice_y_left->setValue( slider_slice_y_right->value()-1 ); 
    }
    if( slider_slice_z_right->value() <= slider_slice_z_left->value() )
    {
	slider_slice_z_left->setValue( slider_slice_z_right->value()-1 ); 
    }
}

//-------------------------------------------------------------------------
void mainWiget::checkSliderConsistencyLeft()
{
    if( slider_slice_x_left->value() >= slider_slice_x_right->value() )
    {
	slider_slice_x_right->setValue( slider_slice_x_left->value()+1 ); 
    }
    if( slider_slice_y_left->value() >= slider_slice_y_right->value() )
    {
	slider_slice_y_right->setValue( slider_slice_y_left->value()+1 ); 
    }
    if( slider_slice_z_left->value() >= slider_slice_z_right->value() )
    {
	slider_slice_z_right->setValue( slider_slice_z_left->value()+1 ); 
    } 
}

//-------------------------------------------------------------------------
void mainWiget::setSliceBounds()
{
    glWindow->setSliceBounds( float(slider_slice_x_left  -> value())/10.0,
			      float(slider_slice_x_right -> value())/10.0,
			      float(slider_slice_y_left  -> value())/10.0,
			      float(slider_slice_y_right -> value())/10.0,
			      float(slider_slice_z_left  -> value())/10.0,
			      float(slider_slice_z_right -> value())/10.0 );
    glWindow->repaint();
}

//-------------------------------------------------------------------------
// slot to open a system native file browser.
// Selected file will be set as the new configuration file to start an animation from
void mainWiget::openVideoFile()
{
    QString newFile = QFileDialog::getOpenFileName( "./", "All Files (*)", this, "open file", "Choose configuration file" );   
    if( newFile != "" )
    {
	lineEditVideoFile -> setText( newFile );
	newInputFile( newFile );
    }
}

//-------------------------------------------------------------------------
// slot that is called, when animation is to be started
// several buttons and line edits are enabled or disabled by this function
void mainWiget::videoStart()
{
    if( lineEditVideoFile -> text() != "" && lineEditStart     -> text() != "" && 
	lineEditStop      -> text() != "" && lineEditStep      -> text() != ""    )
    {
	pushButtonVidPause -> setEnabled( true );
	qtTimer            -> start     ( 0    );
	
	enableVideoLineEdits( false );
	
	videoStartVal = lineEditStart -> text().toUInt();
	videoStopVal  = lineEditStop  -> text().toUInt();
	videoStepVal  = lineEditStep  -> text().toUInt();
	videoCount    = videoStartVal;
	
	progressBarVideo -> reset();
	progressBarVideo -> setTotalSteps( (videoStopVal-videoStartVal) / videoStepVal );
	
	/*
	if( checkBoxVideoAllFrames -> isChecked() )
	  {
	  snapShot();
	}
	*/
    }
}

// is called by the timer. With every call a new cnf file is loaded and displayed
void mainWiget::videoNextScene()
{
    if( lineEditVideoFile -> text() == "" ) { return; }
    QString fileName = "";
    QString tmpNum   = "";
    QString strMask  = "";
    
    if( videoCount >= videoStartVal && videoCount <= videoStopVal )
    {
	
	fileName  = lineEditVideoFile -> text();
	fileName  . truncate( fileName.findRev(".") );
	tmpNum    . sprintf ( "%u", videoCount );
	strMask   = lineEditVideoFile -> text() . section('.', -1, -1 );
	strMask   . fill    ( '0' );
	strMask   . truncate( strMask.length() - tmpNum.length() );
	fileName += '.' + strMask + tmpNum;
	
	if( (cnf -> reloadCnfFile( fileName )) == true )
	{  
	    sliderUpdate = false;
	    buildScene(false);
	    sliderUpdate = true;
	    glWindow->waitForRepaint();
//	    glWindow->repaint();

	    QString tmp;
	    lcdCurrentFileNumber -> display    ( tmp.sprintf("%d",videoCount) );
	    progressBarVideo     -> setProgress( videoCount/videoStepVal      );
	    
	    /*if( checkBoxVideoAllFrames -> isChecked() )
	    {
	      QString tmp;
	      
	      tmp  = lineEditSnapShot -> text();
	      tmp  . truncate( tmp.findRev(".") );
	      
	      tmp += "." + tmpNum.rightJustify( lineEditStop->text().length(), '0' );
	      lineEditSnapShot -> setText( tmp );
	      
	      snapShot();
	    }*/
	}
	else { cerr << "Beware! : error while reloading cnf file!" << endl; }
    }
    
    if     ( radioButtonForward -> isChecked()  )
    {
	if( videoCount <= videoStopVal - videoStepVal  ) { videoCount += videoStepVal; }
	else                                             { videoStop();                }
    }
    else if( radioButtonBackward -> isChecked() )
    {
	if( videoCount >= videoStartVal + videoStepVal ) { videoCount -= videoStepVal;                  }	
	else                                             { videoStop(); radioButtonForward -> toggle(); }
    }
}

//-------------------------------------------------------------------------
// stops the animation and resets the counter, progress bar and lcd display
// some buttons are also enabled/disabled
void mainWiget::videoStop()
{
    qtTimer              -> stop();
    enableVideoLineEdits( true );
    pushButtonVidPause   -> setEnabled( false );
    lineEditVideoFileReturnPressed ();
    progressBarVideo     -> reset  ();
    lcdCurrentFileNumber -> display( 0 );
    
    QString fileName  = "";
    fileName          = lineEditVideoFile -> text();
    if( (cnf -> reloadCnfFile( fileName )) == true )
    {
	sliderUpdate = false;
	buildScene(false);
	sliderUpdate = true;
    }
    else { cerr << "Beware! : error while reloading cnf file!" << endl; }
}

//-------------------------------------------------------------------------
void mainWiget::lineEditVideoFileReturnPressed()
{
    QString newFile = lineEditVideoFile -> text();
    newInputFile( newFile );
}

//-------------------------------------------------------------------------
// enable or disable line edits in the video section as well as the file-browser button
void mainWiget::enableVideoLineEdits( bool enabled )
{
    pushButtonOpenFileVideo -> setEnabled( enabled );
    
    lineEditVideoFile -> setEnabled( enabled );
    lineEditStart     -> setEnabled( enabled );
    lineEditStop      -> setEnabled( enabled );
    lineEditStep      -> setEnabled( enabled );
}

//-------------------------------------------------------------------------
// starts and stops the timer that is used to get the animation going
// also some buttons are enabled or disabeld from here
void mainWiget::videoPause( bool isOn )
{
    if( isOn )
    {
	qtTimer            -> stop();
	pushButtonVidStart -> setEnabled( false );
	pushButtonVidStop  -> setEnabled( false );
    }
    else
    {
	qtTimer            -> start( 0 );
	pushButtonVidStart -> setEnabled( true );
	pushButtonVidStop  -> setEnabled( true );
    }
}




// -------------------------- old stuff

// ----------
void mainWiget::snapShot()
{
    //  SbString imageName;
    //  if( lineEditSnapShot -> text() == "" )
    //    {
    //      QString file = QFileDialog::getSaveFileName( "./", 
    //						   "Almost All Files (*.*)",
    //						   this,
    //						   "save file",
    //						   "Coose Filename - extension (*.xx) will be truncated"  );   
    //	
    //      file.truncate( file.findRev(".") );
    //      lineEditSnapShot -> setText( file );
    //    }
    //    
    //  SbViewportRegion vpr = myViewer -> getViewportRegion();
    //  if( lineEditPix1 -> text() == "" || lineEditPix2 -> text() == "" )
    //    {
    //      cerr << "Invalid pixel size requested... using 1000x1000 instead." << endl;
    //      vpr.setWindowSize( 1000, 1000 );
    //    }
    //  else
    //    {
    //      vpr.setWindowSize( lineEditPix1->text().toInt(), lineEditPix2->text().toInt() );
    //    }
    //    
    //  myOffScreenRenderer -> setViewportRegion( vpr );
    //  SoNode *tmpRoot      = myViewer -> getSceneManager() -> getSceneGraph();
    //  myOffScreenRenderer -> setBackgroundColor( myViewer -> getBackgroundColor() );
    //  SbBool ok            = myOffScreenRenderer -> render( tmpRoot );
    //  if (!ok) { cerr << "Error rendering to offscreen renderer. Something is seriously wrong! exiting..." << endl; exit(1); }
    //    
    //  if( comboBoxSnapShot -> isEnabled() )
    //    {
    //      imageName = (SbString)(lineEditSnapShot->text()+"."+comboBoxSnapShot->currentText());
    //      ok = myOffScreenRenderer -> writeToFile( imageName, (const char *)(comboBoxSnapShot -> currentText()) );
    //      if (!ok) { cerr << "Error: could not write image file: " << imageName.getString() << endl; }
    //      else     { cerr << "Image " << imageName.getString() << " written successfully." << endl;  }
    //      //ok = myOffScreenRenderer -> writeToFile( (SbString)(lineEditSnapShot->text()+"."+comboBoxSnapShot->currentText() ),
    //      //				       (const char *)(comboBoxSnapShot -> currentText()) );
    //      //if (!ok) { cerr << "Error: could not write image file." << endl; }
    //      //else     { cerr << "Image " << (SbString)(lineEditSnapShot->text()+"."+comboBoxSnapShot->currentText()) << " written successfully." << endl; }
    //    }
    //  else
    //    {    
    //      if( checkBoxSnapRgb -> isChecked() )
    //	{
    //	  imageName = (SbString)(lineEditSnapShot->text()+".rgb");
    //	  ok = myOffScreenRenderer -> writeToRGB( imageName.getString() );
    //	  //ok = myOffScreenRenderer -> writeToRGB( ((SbString)(lineEditSnapShot->text()+".rgb")).getString() );
    //	}
    //      if( checkBoxSnapPs -> isChecked() )
    //	{
    //	  imageName = (SbString)(lineEditSnapShot->text()+".ps");
    //	  ok *= myOffScreenRenderer -> writeToPostScript( imageName.getString() ); 
    //	  //ok *= myOffScreenRenderer -> writeToPostScript( ((SbString)(lineEditSnapShot->text()+".ps")).getString() ); 
    //	}
    //      
    //      if (!ok) { cerr << "Error: could not write image file: " << imageName.getString() << endl; }
    //      else     { cerr << "Image " << imageName.getString() << " written successfully." << endl;  }
    //    }   
}





//--------------------------------- bin


//-------------------------------------------------------------------------
// this function constructs the basic scene graph and initiates some signal connections.
//void mainWiget::constructScene()
//{
//  cnf = new CnfFile( cnfFile, colorMap );
//  cnf -> setUseUserDefinedDirector( radioButton_colorAxis_user -> isChecked() );

//qtTimer = new QTimer( this );

//  if( cnf->getBoxX() == 0.0 || cnf->getBoxY() == 0.0 || cnf->getBoxZ() == 0.0 ) {}
//  else
//    {
//      adjustSliceSliders();
//    }

//buildMoleculesTree();
//  setColorMap();
//  buildScene();  
//  connect( myViewer, SIGNAL( signalHeightChanged(float) ), this    , SLOT( slotHeight     (float) ) );
//  connect( spinBoxR, SIGNAL( valueChanged       (int)   ), myViewer, SLOT( spinBoxRChanged(int)   ) );
//    
//  connect( myViewer    , SIGNAL( signalPositionChanged(SoSFVec3f*) ), this    , SLOT( slotPosition       (SoSFVec3f*) ) );
//  connect( spinBoxTheta, SIGNAL( valueChanged         (int)        ), myViewer, SLOT( spinBoxThetaChanged(int)        ) );
//  connect( spinBoxPhi  , SIGNAL( valueChanged         (int)        ), myViewer, SLOT( spinBoxPhiChanged  (int)        ) );
//    
//  connect( qtTimer ,SIGNAL( timeout() ), this, SLOT( videoNextScene () ) );  
//}

//-------------------------------------------------------------------------
// changes the color vector that is (was) given to glWindow
// rerenderes the gl window
//void mainWiget::recolorScene()
//{
//    cout << "refresh" << endl;
//  Molecule *mc = NULL;
//  for( int i = 0; i < color -> size(); i++ )
//    {
//      mc = cnf->getMolecule(i);
//      color->at(i)->at(0) = (float) (mc -> getRed()/255.0f);
//      color->at(i)->at(1) = (float) (mc -> getGreen()/255.0f);
//      color->at(i)->at(2) = (float) (mc -> getBlue()/255.0f);
//    }
//  glWindow->repaint();
//}



//          vector<float *> *farben = new vector <float *> (cnf->getNumberOfMolecules(), new float[3]);
//          
//          for (int i = 0; i < farben->size(); i++) {
//            farben->at(i)[0] = (float) cm.getRed(i)/255.0f;
//            farben->at(i)[1] = (float) cm.getGreen(i)/255.0f;
//            farben->at(i)[2] = (float) cm.getBlue(i)/255.0f;
//        }    
//          tr.setColorMap(farben);



