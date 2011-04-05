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

//-------------------------------------------------------------------------
//------------- init
//-------------------------------------------------------------------------
/*! 
 *  This function is automatically called by the objects constructor. (default qt behaviour)
 *  \author Adrian Gabriel
 *  \date June 2005
 */
void ColorForm::init()
{
    glWindow = 0;
    
    pos.resize(4,0.0);
    spec.resize(4,0.0);
    diff.resize(4,0.0);
    amb.resize(4,0.0);
    bgColor.resize(3,0.0);
    boundingBoxColor.resize(3,1.0);
    
    lineEdit_posX -> setText("0.0");
    lineEdit_posY -> setText("0.0");
    lineEdit_posZ -> setText("1.0");
}

//-------------------------------------------------------------------------
//------------- connectSlots
//-------------------------------------------------------------------------
/*! 
 *  Takes care of needed signal/slot connections
 *  \author Adrian Gabriel
 *  \date June 2005
 */
void ColorForm::connectSlots() 
{
    connect( slider_ambientRed  , SIGNAL(valueChanged(int)), this, SLOT(sliderValueChanged()) );
    connect( slider_ambientGreen, SIGNAL(valueChanged(int)), this, SLOT(sliderValueChanged()) );
    connect( slider_ambientBlue , SIGNAL(valueChanged(int)), this, SLOT(sliderValueChanged()) );

    connect( slider_diffuseRed  , SIGNAL(valueChanged(int)), this, SLOT(sliderValueChanged()) );
    connect( slider_diffuseGreen, SIGNAL(valueChanged(int)), this, SLOT(sliderValueChanged()) );
    connect( slider_diffuseBlue , SIGNAL(valueChanged(int)), this, SLOT(sliderValueChanged()) );
    
    connect( slider_specularRed  , SIGNAL(valueChanged(int)), this, SLOT(sliderValueChanged()) );
    connect( slider_specularGreen, SIGNAL(valueChanged(int)), this, SLOT(sliderValueChanged()) );
    connect( slider_specularBlue , SIGNAL(valueChanged(int)), this, SLOT(sliderValueChanged()) );
    connect( slider_specularShiny, SIGNAL(valueChanged(int)), this, SLOT(sliderValueChanged()) );
    
    connect( slider_backgroundRed  , SIGNAL(valueChanged(int)), this, SLOT(sliderValueChanged()) );
    connect( slider_backgroundGreen, SIGNAL(valueChanged(int)), this, SLOT(sliderValueChanged()) );
    connect( slider_backgroundBlue , SIGNAL(valueChanged(int)), this, SLOT(sliderValueChanged()) );
    
    connect( slider_boundingBoxRed  , SIGNAL(valueChanged(int)), this, SLOT(sliderValueChanged()) );
    connect( slider_boundingBoxGreen, SIGNAL(valueChanged(int)), this, SLOT(sliderValueChanged()) );
    connect( slider_boundingBoxBlue , SIGNAL(valueChanged(int)), this, SLOT(sliderValueChanged()) );
    
    connect( lineEdit_posX, SIGNAL(returnPressed()), this,  SLOT(sliderValueChanged()) );
    connect( lineEdit_posY, SIGNAL(returnPressed()), this,  SLOT(sliderValueChanged()) );
    connect( lineEdit_posZ, SIGNAL(returnPressed()), this,  SLOT(sliderValueChanged()) );
    
    connect( checkBox_directionalLight, SIGNAL(toggled(bool)), this,  SLOT(sliderValueChanged()) );
    
    connect( pushButton_loadDefaults, SIGNAL(clicked()), this, SLOT(loadDefaults()) );
}

//-------------------------------------------------------------------------
//------------- sliderValueChanged
//-------------------------------------------------------------------------
/*! 
 *  Called when a slider is moved.
 *  \author Adrian Gabriel
 *  \date June 2005
 */
void ColorForm::sliderValueChanged()
{
    lCDNumber_ambientRed   -> display( slider_ambientRed   -> value()/100.0 );
    lCDNumber_ambientGreen -> display( slider_ambientGreen -> value()/100.0 );
    lCDNumber_ambientBlue  -> display( slider_ambientBlue  -> value()/100.0 );
    
    lCDNumber_diffuseRed   -> display( slider_diffuseRed   -> value()/100.0 );
    lCDNumber_diffuseGreen -> display( slider_diffuseGreen -> value()/100.0 );
    lCDNumber_diffuseBlue  -> display( slider_diffuseBlue  -> value()/100.0 );
    
    lCDNumber_specularRed   -> display( slider_specularRed   -> value()/100.0 );
    lCDNumber_specularGreen -> display( slider_specularGreen -> value()/100.0 );
    lCDNumber_specularBlue  -> display( slider_specularBlue  -> value()/100.0 );
    lCDNumber_specularShiny -> display( slider_specularShiny -> value() );
    
    lCDNumber_backgroundRed   -> display( slider_backgroundRed   -> value()/100.0 );
    lCDNumber_backgroundGreen -> display( slider_backgroundGreen -> value()/100.0 );
    lCDNumber_backgroundBlue  -> display( slider_backgroundBlue  -> value()/100.0 );
    
    lCDNumber_boundingBoxRed   -> display( slider_boundingBoxRed   -> value()/100.0 );
    lCDNumber_boundingBoxGreen -> display( slider_boundingBoxGreen -> value()/100.0 );
    lCDNumber_boundingBoxBlue  -> display( slider_boundingBoxBlue  -> value()/100.0 );
    
    pos.at(0) = lineEdit_posX->text().toFloat();
    pos.at(1) = lineEdit_posY->text().toFloat();
    pos.at(2) = lineEdit_posZ->text().toFloat();
    pos.at(3) = checkBox_directionalLight->isChecked() ? 0.0 : 1.0;
    
    amb.at(0) = slider_ambientRed  -> value()/100.0;
    amb.at(1) = slider_ambientGreen-> value()/100.0;
    amb.at(2) = slider_ambientBlue -> value()/100.0;

    diff.at(0) = slider_diffuseRed  -> value()/100.0;
    diff.at(1) = slider_diffuseGreen-> value()/100.0;
    diff.at(2) = slider_diffuseBlue -> value()/100.0;

    spec.at(0) = slider_specularRed  -> value()/100.0;
    spec.at(1) = slider_specularGreen-> value()/100.0;
    spec.at(2) = slider_specularBlue -> value()/100.0;

    bgColor.at(0) = slider_backgroundRed   -> value()/100.0;
    bgColor.at(1) = slider_backgroundGreen -> value()/100.0;
    bgColor.at(2) = slider_backgroundBlue  -> value()/100.0;
    
    boundingBoxColor.at(0) = slider_boundingBoxRed   -> value()/100.0;
    boundingBoxColor.at(1) = slider_boundingBoxGreen -> value()/100.0;
    boundingBoxColor.at(2) = slider_boundingBoxBlue  -> value()/100.0;
    
    if(glWindow!=0) { glWindow->setLight(amb,diff,spec,pos,slider_specularShiny->value(), bgColor, boundingBoxColor); }
}

//-------------------------------------------------------------------------
//------------- setGlWindow
//-------------------------------------------------------------------------
/*! 
 *  Sets the pointer to the used render area.
 *  \author Adrian Gabriel
 *  \date June 2005
 */
void ColorForm::setGlWindow( Renderer *newGlWindow )
{
    if( newGlWindow != 0 )
    {
	glWindow = newGlWindow;
    }
}

//-------------------------------------------------------------------------
//------------- setSliderAmbientValues
//-------------------------------------------------------------------------
/*! 
 *  Sets values of the ambient sliders.
 *  \param red Value for the red slider.
 *  \param green Value for the green slider.
 *  \param blue Value for the blue slider.
 *  \author Adrian Gabriel
 *  \date June 2005
 */
void ColorForm::setSliderAmbientValues( float red, float green, float blue )
{
    slider_ambientRed  ->setValue(int(rint(red  *100.0)));
    slider_ambientGreen->setValue(int(rint(green*100.0)));
    slider_ambientBlue ->setValue(int(rint(blue *100.0)));
}

//-------------------------------------------------------------------------
//------------- setSliderDiffuseValues
//-------------------------------------------------------------------------
/*! 
 *  Sets values of the diffuse sliders.
 *  \param red Value for the red slider.
 *  \param green Value for the green slider.
 *  \param blue Value for the blue slider.
 *  \author Adrian Gabriel
 *  \date June 2005
 */
void ColorForm::setSliderDiffuseValues( float red, float green, float blue )
{
    slider_diffuseRed   -> setValue(int(rint(red  *100.0)));
    slider_diffuseGreen -> setValue(int(rint(green*100.0)));
    slider_diffuseBlue  -> setValue(int(rint(blue *100.0)));
}

//-------------------------------------------------------------------------
//------------- setSliderSpecularValues
//-------------------------------------------------------------------------
/*! 
 *  Sets values of the specular sliders.
 *  \param red Value for the red slider.
 *  \param green Value for the green slider.
 *  \param blue Value for the blue slider.
 *  \param shiny Value for the shinyness slider.
 *  \author Adrian Gabriel
 *  \date June 2005
 */
void ColorForm::setSliderSpecularValues( float red, float green, float blue, int shiny )
{
    slider_specularRed   -> setValue(int(rint(red  *100.0)));
    slider_specularGreen -> setValue(int(rint(green*100.0)));
    slider_specularBlue  -> setValue(int(rint(blue *100.0)));
    slider_specularShiny -> setValue(shiny);
}

//-------------------------------------------------------------------------
//------------- setSliderBackgroundValues
//-------------------------------------------------------------------------
/*! 
 *  Sets values of the background sliders.
 *  \param red Value for the red slider.
 *  \param green Value for the green slider.
 *  \param blue Value for the blue slider.
 *  \author Adrian Gabriel
 *  \date April 2007
 */
void ColorForm::setSliderBackgroundValues( float red, float green, float blue )
{
    slider_backgroundRed   -> setValue(int(rint(red  *100.0)));
    slider_backgroundGreen -> setValue(int(rint(green*100.0)));
    slider_backgroundBlue  -> setValue(int(rint(blue *100.0)));
}

//-------------------------------------------------------------------------
//------------- setSliderBoundingBoxValues
//-------------------------------------------------------------------------
/*! 
 *  Sets values of the bounding box sliders.
 *  \param red Value for the red slider.
 *  \param green Value for the green slider.
 *  \param blue Value for the blue slider.
 *  \author Adrian Gabriel
 *  \date April 2007
 */
void ColorForm::setSliderBoundingBoxValues( float red, float green, float blue )
{
    slider_boundingBoxRed   -> setValue(int(rint(red  *100.0)));
    slider_boundingBoxGreen -> setValue(int(rint(green*100.0)));
    slider_boundingBoxBlue  -> setValue(int(rint(blue *100.0)));
}

//-------------------------------------------------------------------------
//------------- setLightPosition
//-------------------------------------------------------------------------
/*! 
 *  Sets kind (directional or not) and position of the light source.
 *  \param x x position of the light source.
 *  \param y y position of the light source.
 *  \param z z position of the light source.
 *  \param directional True sets light source to be directional.
 *  \author Adrian Gabriel
 *  \date June 2005
 */
void ColorForm::setLightPosition( QString x, QString y, QString z, bool directional )
{
    lineEdit_posX -> setText(x);
    lineEdit_posY -> setText(y);
    lineEdit_posZ -> setText(z);
    
    checkBox_directionalLight -> setChecked(directional);
}

//-------------------------------------------------------------------------
//------------- getSliderAmbientValues
//-------------------------------------------------------------------------
/*! 
 *  Gets values of the ambient sliders.
 *  \param red Value for the red slider.
 *  \param green Value for the green slider.
 *  \param blue Value for the blue slider.
 *  \author Adrian Gabriel
 *  \date June 2005
 */
void ColorForm::getSliderAmbientValues( float &red, float &green, float &blue )
{
    red   = slider_ambientRed   -> value()/100.0;
    green = slider_ambientGreen -> value()/100.0;
    blue  = slider_ambientBlue  -> value()/100.0;
}

//-------------------------------------------------------------------------
//------------- getSliderDiffuseValues
//-------------------------------------------------------------------------
/*! 
 *  Gets values of the diffuse sliders.
 *  \param red Value for the red slider.
 *  \param green Value for the green slider.
 *  \param blue Value for the blue slider.
 *  \author Adrian Gabriel
 *  \date June 2005
 */
void ColorForm::getSliderDiffuseValues( float &red, float &green, float &blue )
{
    red   = slider_diffuseRed   -> value()/100.0;
    green = slider_diffuseGreen -> value()/100.0;
    blue  = slider_diffuseBlue  -> value()/100.0;
}

//-------------------------------------------------------------------------
//------------- getSliderSpecularValues
//-------------------------------------------------------------------------
/*! 
 *  Gets values of the specular sliders.
 *  \param red Value for the red slider.
 *  \param green Value for the green slider.
 *  \param blue Value for the blue slider.
 *  \param shiny Value for the shinyness slider.
 *  \author Adrian Gabriel
 *  \date June 2005
 */
void ColorForm::getSliderSpecularValues( float &red, float &green, float &blue, int &shiny )
{
    red   = slider_specularRed   -> value()/100.0;
    green = slider_specularGreen -> value()/100.0;
    blue  = slider_specularBlue  -> value()/100.0;
    shiny = slider_specularShiny -> value();
}

//-------------------------------------------------------------------------
//------------- getSliderBackgroundValues
//-------------------------------------------------------------------------
/*! 
 *  Gets values of the background sliders.
 *  \param red Value for the red slider.
 *  \param green Value for the green slider.
 *  \param blue Value for the blue slider.
 *  \author Adrian Gabriel
 *  \date April 2007
 */
void ColorForm::getSliderBackgroundValues( float &red, float &green, float &blue )
{
    red   = slider_backgroundRed   -> value()/100.0;
    green = slider_backgroundGreen -> value()/100.0;
    blue  = slider_backgroundBlue  -> value()/100.0;
}

//-------------------------------------------------------------------------
//------------- getSliderBoundingBoxValues
//-------------------------------------------------------------------------
/*! 
 *  Gets values of the bounding box sliders.
 *  \param red Value for the red slider.
 *  \param green Value for the green slider.
 *  \param blue Value for the blue slider.
 *  \author Adrian Gabriel
 *  \date April 2007
 */
void ColorForm::getSliderBoundingBoxValues( float &red, float &green, float &blue )
{
    red   = slider_boundingBoxRed   -> value()/100.0;
    green = slider_boundingBoxGreen -> value()/100.0;
    blue  = slider_boundingBoxBlue  -> value()/100.0;
}

//-------------------------------------------------------------------------
//------------- getLightPosition
//-------------------------------------------------------------------------
/*! 
 *  Gets kind (directional or not) and position of the light source.
 *  \param x x position of the light source.
 *  \param y y position of the light source.
 *  \param z z position of the light source.
 *  \param directional True means light source is directional.
 *  \author Adrian Gabriel
 *  \date June 2005
 */
void ColorForm::getLightPosition( QString &x, QString &y, QString &z, bool &directional )
{
    x = lineEdit_posX -> text();
    y = lineEdit_posY -> text();
    z = lineEdit_posZ -> text();
    
    directional = checkBox_directionalLight -> isChecked();
}

//-------------------------------------------------------------------------
//------------- loadDefaults
//-------------------------------------------------------------------------
/*! 
 *  Sets a set of default values for this dialoge.
 *  \author Adrian Gabriel
 *  \date June 2005
 */
void ColorForm::loadDefaults()
{
    slider_ambientRed   -> setValue(5);
    slider_ambientGreen -> setValue(5);
    slider_ambientBlue  -> setValue(5);
    
    slider_diffuseRed   -> setValue(70);
    slider_diffuseGreen -> setValue(70);
    slider_diffuseBlue  -> setValue(70);
    
    slider_specularRed   -> setValue(25);
    slider_specularGreen -> setValue(25);
    slider_specularBlue  -> setValue(25);
    slider_specularShiny -> setValue(128);  
    
    slider_backgroundRed   -> setValue(100);
    slider_backgroundGreen -> setValue(100);
    slider_backgroundBlue  -> setValue(100);
    
    slider_boundingBoxRed   -> setValue(0);
    slider_boundingBoxGreen -> setValue(0);
    slider_boundingBoxBlue  -> setValue(0);
    
    lineEdit_posX -> setText("0.0");
    lineEdit_posY -> setText("0.0");
    lineEdit_posZ -> setText("1.0");
    
    checkBox_directionalLight -> setChecked(true);
}



