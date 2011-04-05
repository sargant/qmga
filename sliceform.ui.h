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

enum rightleft { LEFT = -1, RIGHT = 1 };

//-------------------------------------------------------------------------
//------------- init
//-------------------------------------------------------------------------
/*! 
 *  This function is automatically called by the objects constructor. (default qt behaviour)
 *  \author Adrian Gabriel
 *  \date June 2005
 */
void SliceForm::init()
{
    //cout << "SliceForm::init beg" << endl;
    cnf = 0;
    glWindow = 0;
    sliderUpdate = true;
    
    dx = dy = dz = 5;
    
    slider_slice_x_left  -> setRange( -500, 500 );
    slider_slice_x_left  -> setValue( -500 );
    slider_slice_x_right -> setRange( -500, 500 );
    slider_slice_x_right -> setValue(  500 );
    
    slider_slice_y_left  -> setRange( -500, 500 );
    slider_slice_y_left  -> setValue( -500 );
    slider_slice_y_right -> setRange( -500, 500 );
    slider_slice_y_right -> setValue(  500 );
    
    slider_slice_z_left  -> setRange( -500, 500 );
    slider_slice_z_left  -> setValue( -500 );
    slider_slice_z_right -> setRange( -500, 500 );
    slider_slice_z_right -> setValue(  500 );
    
    connect( slider_slice_x_left, SIGNAL(valueChanged(int)), SLOT(checkSliderConsistencyLeft()) );
    connect( slider_slice_y_left, SIGNAL(valueChanged(int)), SLOT(checkSliderConsistencyLeft()) );
    connect( slider_slice_z_left, SIGNAL(valueChanged(int)), SLOT(checkSliderConsistencyLeft()) );
    
    connect( slider_slice_x_right, SIGNAL(valueChanged(int)), SLOT(checkSliderConsistencyRight()) );
    connect( slider_slice_y_right, SIGNAL(valueChanged(int)), SLOT(checkSliderConsistencyRight()) );
    connect( slider_slice_z_right, SIGNAL(valueChanged(int)), SLOT(checkSliderConsistencyRight()) );
    
    connect( slider_slice_x_left, SIGNAL(valueChanged(int)), SLOT(updateSliderLCDs()) );
    connect( slider_slice_y_left, SIGNAL(valueChanged(int)), SLOT(updateSliderLCDs()) );
    connect( slider_slice_z_left, SIGNAL(valueChanged(int)), SLOT(updateSliderLCDs()) );
    
    connect( slider_slice_x_right, SIGNAL(valueChanged(int)), SLOT(updateSliderLCDs()) );
    connect( slider_slice_y_right, SIGNAL(valueChanged(int)), SLOT(updateSliderLCDs()) );
    connect( slider_slice_z_right, SIGNAL(valueChanged(int)), SLOT(updateSliderLCDs()) );
    
    connect( pushButton_linkX, SIGNAL(toggled(bool)), SLOT(setDx(bool)) );
    connect( pushButton_linkY, SIGNAL(toggled(bool)), SLOT(setDy(bool)) );
    connect( pushButton_linkZ, SIGNAL(toggled(bool)), SLOT(setDz(bool)) );
    
    connect( pushButton_resetX, SIGNAL(clicked()), SLOT(resetX()) );
    connect( pushButton_resetY, SIGNAL(clicked()), SLOT(resetY()) );
    connect( pushButton_resetZ, SIGNAL(clicked()), SLOT(resetZ()) );
    
    QIconSet *is = new QIconSet();
    is -> setPixmap ( "images/connect_open.png"  , QIconSet::Automatic, QIconSet::Normal, QIconSet::Off );
    is -> setPixmap ( "images/connect_closed.png", QIconSet::Automatic, QIconSet::Normal, QIconSet::On  );
    is -> clearGenerated(); 
    
    pushButton_linkX -> setIconSet( *is );
    pushButton_linkX -> setFlat( true );
    
    pushButton_linkY -> setIconSet( *is );
    pushButton_linkY -> setFlat( false );
    
    pushButton_linkZ -> setIconSet( *is );
    pushButton_linkZ -> setFlat( false );
    //cout << "SliceForm::init end" << endl;
}

//-------------------------------------------------------------------------
//------------- updateSliderLCDs
//-------------------------------------------------------------------------
/*! 
 *  Updates the LCD values connected to the sliders.
 *  \author Adrian Gabriel
 *  \date June 2005
 */
void SliceForm::updateSliderLCDs()
{
    //cout << "SliceForm::updateSliderLCDs beg" << endl;
    lCDNumber_slice_x_left  -> display( slider_slice_x_left  -> value() );
    lCDNumber_slice_x_right -> display( slider_slice_x_right -> value() );
    
    lCDNumber_slice_y_left  -> display( slider_slice_y_left  -> value() );
    lCDNumber_slice_y_right -> display( slider_slice_y_right -> value() );
    
    lCDNumber_slice_z_left  -> display( slider_slice_z_left  -> value() );
    lCDNumber_slice_z_right -> display( slider_slice_z_right -> value() );
    
    setSliceBounds();
    //cout << "SliceForm::updateSliderLCDs end" << endl;
}

//-------------------------------------------------------------------------
//------------- checkSliderConsistencyRight
//-------------------------------------------------------------------------
/*! 
 *  Sliders on one axis shall not overtake each other.
 *  \author Adrian Gabriel
 *  \date June 2005
 */
void SliceForm::checkSliderConsistencyRight()
{
    if( pushButton_linkX -> isOn() == false )
    {
	if( slider_slice_x_right -> value() <= slider_slice_x_left -> value() )
	{
	    checkSliderConsistencyHelper( slider_slice_x_right, slider_slice_x_left, RIGHT, 1 );
	}
    }
    else
    {
	checkSliderConsistencyHelper( slider_slice_x_right, slider_slice_x_left, RIGHT, dx );
    }
    
    if( pushButton_linkY -> isOn() == false )
    {
	if( slider_slice_y_right -> value() <= slider_slice_y_left -> value() )
	{
	    checkSliderConsistencyHelper( slider_slice_y_right, slider_slice_y_left, RIGHT, 1 );
	}
    }
    else
    {
	checkSliderConsistencyHelper( slider_slice_y_right, slider_slice_y_left, RIGHT, dy );
    }
    
    if( pushButton_linkZ -> isOn() == false )
    {
	if( slider_slice_z_right -> value() <= slider_slice_z_left -> value() )
	{
	    checkSliderConsistencyHelper( slider_slice_z_right, slider_slice_z_left, RIGHT, 1 );
	}
    }
    else
    {
	checkSliderConsistencyHelper( slider_slice_z_right, slider_slice_z_left, RIGHT, dz );
    }
}

//-------------------------------------------------------------------------
//------------- checkSliderConsistencyLeft
//-------------------------------------------------------------------------
/*! 
 *  Sliders on one axis shall not overtake each other.
 *  \author Adrian Gabriel
 *  \date June 2005
 */
void SliceForm::checkSliderConsistencyLeft()
{
    if( pushButton_linkX -> isOn() == false )
    {
	if( slider_slice_x_left -> value() >= slider_slice_x_right -> value() )
	{
	    checkSliderConsistencyHelper( slider_slice_x_left, slider_slice_x_right, LEFT, 1 );
	}
    }
    else
    {
	checkSliderConsistencyHelper( slider_slice_x_left, slider_slice_x_right, LEFT, dx );
    }
    
    if( pushButton_linkY -> isOn() == false )
    {
	if( slider_slice_y_left -> value() >= slider_slice_y_right -> value() )
	{
	    checkSliderConsistencyHelper( slider_slice_y_left, slider_slice_y_right, LEFT, 1 );
	}
    }
    else
    {
	checkSliderConsistencyHelper( slider_slice_y_left, slider_slice_y_right, LEFT, dy );
    }
    
    if( pushButton_linkZ -> isOn() == false )
    {
	if( slider_slice_z_left -> value() >= slider_slice_z_right -> value() )
	{
	    checkSliderConsistencyHelper( slider_slice_z_left, slider_slice_z_right, LEFT, 1 );
	}
    }
    else
    {
	checkSliderConsistencyHelper( slider_slice_z_left, slider_slice_z_right, LEFT, dz );
    }    
}

//-------------------------------------------------------------------------
//------------- checkSliderConsistencyHelper
//-------------------------------------------------------------------------
/*! 
 *  
 *  \author Adrian Gabriel
 *  \date June 2005
 */
void SliceForm::checkSliderConsistencyHelper( QSlider *first, QSlider *second, int rightleft, int delta )
{
    if( rightleft == LEFT )
    {
	if( first -> value() > second -> maxValue() - delta )
	{
	    first  -> setValue( second -> maxValue() - delta );
	    second -> setValue( second -> maxValue() );
	}
	else
	{
	    second -> setValue( first -> value() + delta ); 
	}
    }
    else
    {
	if( first -> value() < second -> minValue() + delta )
	{
	    first  -> setValue( second -> minValue() + delta );
	    second -> setValue( second -> minValue() );
	}
	else
	{
	    second -> setValue( first -> value() - delta ); 
	}
    }
}

//-------------------------------------------------------------------------
//------------- setSliceBounds
//-------------------------------------------------------------------------
/*! 
 *  Commits the slice bounds to the render area.
 *  \author Adrian Gabriel
 *  \date June 2005
 */
void SliceForm::setSliceBounds()
{
    //cout << "SliceForm::setSliceBounds beg" << endl;
    if( glWindow != 0 )
    {
	glWindow -> setSliceBounds( float( slider_slice_x_left  -> value() * cnf -> getBoxX() ) / 1001.0,
				    float( slider_slice_x_right -> value() * cnf -> getBoxX() ) / 1001.0,
				    float( slider_slice_y_left  -> value() * cnf -> getBoxY() ) / 1001.0,
				    float( slider_slice_y_right -> value() * cnf -> getBoxY() ) / 1001.0,
				    float( slider_slice_z_left  -> value() * cnf -> getBoxZ() ) / 1001.0,
				    float( slider_slice_z_right -> value() * cnf -> getBoxZ() ) / 1001.0 );
	glWindow -> repaint();
    }
    //cout << "SliceForm::setSliceBounds end" << endl;
}

//-------------------------------------------------------------------------
//------------- setCnfFile
//-------------------------------------------------------------------------
/*! 
 *  When the configuration changes slider settings are updated.
 *  \author Adrian Gabriel
 *  \date June 2005
 */
void SliceForm::setCnfFile( CnfFile *newCnf )
{
    //cout << "SliceForm::setCnfFile beg" << endl;
    if( newCnf != 0 )
    {
	if( cnf != newCnf ) { cnf = newCnf; }
	setSliceBounds();
    }
    //cout << "SliceForm::setCnfFile end" << endl;
}

//-------------------------------------------------------------------------
//------------- setGlWindow
//-------------------------------------------------------------------------
/*! 
 *  Sets the pointer to the used render area.
 *  \author Adrian Gabriel
 *  \date June 2005
 */
void SliceForm::setGlWindow( Renderer *newGlWindow )
{
    if( newGlWindow != 0 )
    {
	glWindow = newGlWindow;
    }
}

//-------------------------------------------------------------------------
//------------- closeEvent
//-------------------------------------------------------------------------
/*! 
 *  Catches close event, so a signal can be emitted before closing the dialoge.
 *  \author Adrian Gabriel
 *  \date June 2005
 */
void SliceForm::closeEvent(QCloseEvent*e)
{
    emit sliceClosed();
    QWidget::closeEvent(e);
}

//-------------------------------------------------------------------------
//------------- setDx
//-------------------------------------------------------------------------
/*! 
 *  
 *  \author Adrian Gabriel
 *  \date May 2007
 */
void SliceForm::setDx( bool on )
{
    if( on == true )
    {
	dx = slider_slice_x_right -> value() - slider_slice_x_left -> value();
    }
}

//-------------------------------------------------------------------------
//------------- setDy
//-------------------------------------------------------------------------
/*! 
 *  
 *  \author Adrian Gabriel
 *  \date May 2007
 */
void SliceForm::setDy( bool on )
{
    if( on == true )
    {
	dy = slider_slice_y_right -> value() - slider_slice_y_left -> value();
    } 
}

//-------------------------------------------------------------------------
//------------- setDz
//-------------------------------------------------------------------------
/*! 
 *  
 *  \author Adrian Gabriel
 *  \date May 2007
 */
void SliceForm::setDz( bool on )
{
    if( on == true )
    {
	dz = slider_slice_z_right -> value() - slider_slice_z_left -> value();
    } 
}

//-------------------------------------------------------------------------
//------------- resetX
//-------------------------------------------------------------------------
/*! 
 *  
 *  \author Adrian Gabriel
 *  \date May 2007
 */
void SliceForm::resetX()
{
    pushButton_linkX -> setOn( false );
    slider_slice_x_left  -> setValue( slider_slice_x_left  -> minValue() );
    slider_slice_x_right -> setValue( slider_slice_x_right -> maxValue() );
}

//-------------------------------------------------------------------------
//------------- resetY
//-------------------------------------------------------------------------
/*! 
 *  
 *  \author Adrian Gabriel
 *  \date May 2007
 */
void SliceForm::resetY()
{
    pushButton_linkY -> setOn( false );
    slider_slice_y_left  -> setValue( slider_slice_y_left  -> minValue() );
    slider_slice_y_right -> setValue( slider_slice_y_right -> maxValue() );    
}

//-------------------------------------------------------------------------
//------------- resetZ
//-------------------------------------------------------------------------
/*! 
 *  
 *  \author Adrian Gabriel
 *  \date May 2007
 */
void SliceForm::resetZ()
{
    pushButton_linkZ -> setOn( false );
    slider_slice_z_left  -> setValue( slider_slice_z_left  -> minValue() );
    slider_slice_z_right -> setValue( slider_slice_z_right -> maxValue() );   
}





