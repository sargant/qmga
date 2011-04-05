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

#include "mga_tools.h"
#include "tnt/jama_eig.h"

#include <cmath>
#include <iostream>
#include <fstream>
#include <string>
#include <iomanip>
#include <algorithm>
#include <sstream>
#include <map>

#define FUNC (*this.*func)

using std::ifstream;
using std::ofstream;
using std::cerr;
using std::flush;
using std::setw;
using std::pow;
using std::sqrt;
using std::left;
using std::setprecision;
using std::map;

using mga::Colormap;
using mga::CnfFile;
using mga::Molecule;
using mga::purge;
using mga::Point3D;

using JAMA::Eigenvalue;



//--------------------------------------------
//------------ ColorMap
//--------------------------------------------


//-------------------------------------------------------------------------
//------------- Colormap
//-------------------------------------------------------------------------
/*!
 *  Constructor of the Colormap class. The only parameter is the path to the coormap file.
 *  \param colorFile Path to the colormap that is to be loaded.
 *  \author Adrian Gabriel
 *  \date June 2005
 */
mga::Colormap::Colormap( string colorFile )
{
    loadColormap( colorFile );
}

//-------------------------------------------------------------------------
//------------- ~Colormap
//-------------------------------------------------------------------------
/*! 
 *  A standard destructor.
 *  \author Adrian Gabriel
 *  \date June 2005
 */
mga::Colormap::~Colormap()
{
    
}

//-------------------------------------------------------------------------
//------------- loadColormap
//-------------------------------------------------------------------------
/*! 
 *  Loads colormap (textfile) and stores colorvalues for 
 *  red, green and blue into stl vector objects.
 *  \param colorFile Path to the colormap that is to be loaded.
 *  \return void.
 *  \author Adrian Gabriel
 *  \date June 2005
 */
void mga::Colormap::loadColormap( string colorFile )
{
    ifstream in( colorFile.c_str() );
    if( in.is_open() )
    {
	int    redSize = 0, greenSize = 0, blueSize = 0;
	double redTmp  = 0, grnTmp    = 0, bluTmp   = 0;
	while( in >> redTmp >> grnTmp >> bluTmp ) 
	{
	    redVector  .push_back(redTmp);
	    greenVector.push_back(grnTmp);
	    blueVector .push_back(bluTmp);
	}
	
	if( !in.eof() )
	{
	    cerr << "Format error in file: " << colorFile << endl;
	    exit(1);
	}
	
	in.close();
	
	redSize   = redVector  .size();
	greenSize = greenVector.size();
	blueSize  = blueVector .size();
	if( !( (redSize == greenSize) && (redSize == blueSize) ) )
	{
	    cerr << "Error: Check colormap file. Different number of colors for r,g,b." << endl;
	    exit(1);
	}
	else
	{
	    numberOfLinesInFile = redSize;
	}
    }
    else
    {
	cerr << "Cannot open file: " << colorFile  << endl;
	exit(1);
    }
}

//-------------------------------------------------------------------------
//------------- print
//-------------------------------------------------------------------------
/*!
 *  The Values for red, green and blue are printed line by line
 *  to std-out.
 *  \return void.
 *  \author Adrian Gabriel
 *  \date June 2005
 */
void mga::Colormap::print() const
{
    for( int line = 0; line < numberOfLinesInFile; ++line )
    {
      cout << setw(10) << redVector  .at(line)
      << setw(10) << greenVector.at(line)
      << setw(10) << blueVector .at(line) << endl;
    }
}

//-------------------------------------------------------------------------
//------------- setColor
//-------------------------------------------------------------------------
/*! 
 *  This function sets the r,g,b values of a Molecule-object.
 *  The calculation is actually done as follows:
 *  Calculate scalar product of two unit vectors, namely moleculeorientation and director.
 *  The result is then squared giving a positive number from 0 to 1.
 *  Multiply this number with the number of colors set in the loaded colormap and cast to integer.
 *  The result is the linenumber which contains the corresponding RGB values in the colormap.
 *  \param moleculeTmp Pointer to the Molecule-object which color is to be set.
 *  \param director Reference to the previusly calculated director
 *  \return The pointer to the Molecule-object of which the color has been set.
 *  \author Adrian Gabriel
 *  \date June 2005
 */
Molecule* mga::Colormap::setColor( Molecule* moleculeTmp, vector<double> &director, vector<vector<float> > *models ) const
{
    //cout << "Colormap::setColor beg" << endl;
    if( (moleculeTmp != 0) && (director.size() == 3) )
    {
	double orientationX = moleculeTmp -> getOrientationX();
	double orientationY = moleculeTmp -> getOrientationY();
	double orientationZ = moleculeTmp -> getOrientationZ();
	
	int numOfMapLines = redVector.size();
	
	float scalarProductOD = fabs( orientationX*director.at(0) + 
				      orientationY*director.at(1) + 
				      orientationZ*director.at(2)   );
	if( scalarProductOD > 1 ) { scalarProductOD = 1; } // catches a bug that causes the product
	// to be slightly larger than 1 (16th after .)
	// and then acos() fails (returns "nan").
	
	int mapLineNr = int( acos( scalarProductOD )/M_PI*2*( numOfMapLines ) );
	
	if( mapLineNr == 90 ) { mapLineNr = 89; }
	
	//cout << mapLineNr << " {" << orientationX << "," << orientationY << "," << orientationZ << "} . "
	//     << " {" << director.at(0) << "," << director.at(1) << "," << director.at(2) << "}" << endl;
	
	moleculeTmp -> setRGB( getRed( mapLineNr ), getGreen( mapLineNr ), getBlue( mapLineNr ) );
	moleculeTmp -> setColorIndex( mapLineNr );
	
	if( models != 0 )
	{
	    int typeTmp = moleculeTmp->getType();
	    if( typeTmp >= 0 && typeTmp < int(models->size()) )
	    {
		if( models->at(typeTmp).at(13) != 0.0 )
		{
		    setColor( moleculeTmp, models );
		}
	    }
	}
    }
    else
    {
	cerr << "Error: no molecule given to setColor, or director corrupt." << endl;
    }
    //cout << "Colormap::setColor end" << endl;
    return( moleculeTmp );
} 

//-------------------------------------------------------------------------
//------------- setColor
//-------------------------------------------------------------------------
/*! 
 *  \param moleculeTmp Pointer to the Molecule-object which color is to be set.
 *  \return The pointer to the Molecule-object of which the color has been set.
 *  \author Adrian Gabriel
 *  \date Jul 2007
 */
Molecule* mga::Colormap::setColor( Molecule* moleculeTmp, vector<vector<float> > *models ) const
{
    //cout << "molecule " << moleculeTmp << " models: " << models << endl;
    if( moleculeTmp != 0 && models != 0 )
    {
	int typeTmp = moleculeTmp->getType();
	if( typeTmp >= 0 && typeTmp < int(models->size()) )
	{
	    if( models->at(typeTmp).size() >= 17 )
	    {
		moleculeTmp -> setRGB( int(models->at(typeTmp).at(14)), 
				       int(models->at(typeTmp).at(15)),  
				       int(models->at(typeTmp).at(16)) );
	    }
	    else
	    {
		cerr << "Error: vectors per model are too short.. no rgb values appended?" << endl;
	    }
	}  
	else
	{
	    cerr << "Error: model type out of range (<0 or >= number of models)." << endl;
	}
    }
    else
    {
	cerr << "Error: no molecule given to setColor, or models vector empty." << endl;
    }
    return( moleculeTmp );
}

//--------------------------------------------
//------------ Molecule
//--------------------------------------------

//-------------------------------------------------------------------------
//------------- Molecule
//-------------------------------------------------------------------------
/*!
 *  Constructor of the Molecule-class. All memmbers can be given as parameters.
 *  \param positionXtmp sets Molecule::positionX
 *  \param positionYtmp sets Molecule::positionY
 *  \param positionZtmp sets Molecule::positionZ
 *  \param orientationXtmp sets Molecule::orientationX
 *  \param orientationYtmp sets Molecule::orientationY
 *  \param orientationZtmp sets Molecule::orientationZ
 *  \param typeTmp sets Molecule::type DEFAULT = 0
 *  \param heightTmp sets Molecule::height DEFAULT = 0.0
 *  \param widthTmp sets Molecule::width DEFAULT = 0.0
 *  \param depthTmp sets Molecule::depth DEFAULT = 0.0
 *  \param redTmp sets Molecule::red DEFAULT = 0.0
 *  \param greenTmp sets Molecule::green DEFAULT = 0.0
 *  \param blueTmp sets Molecule::blue DEFAULT = 0.0
 *  \author Adrian Gabriel
 *  \date June 2005
 */
mga::Molecule::Molecule( double positionXtmp      , double positionYtmp      , double positionZtmp      ,
			 double positionFoldedXtmp, double positionFoldedYtmp, double positionFoldedZtmp,
			 double orientationXtmp   , double orientationYtmp   , double orientationZtmp   ,
			 int    typeTmp           , unsigned int numberTmp   ,                                            
			 double heightTmp         , double widthTmp          , double depthTmp          ,
			 double redTmp            , double greenTmp          , double blueTmp            )
{
    initMembers( positionXtmp      , positionYtmp      , positionZtmp      ,
		 positionFoldedXtmp, positionFoldedYtmp, positionFoldedZtmp,
		 orientationXtmp   , orientationYtmp   , orientationZtmp   ,
		 typeTmp           , numberTmp         ,                                         
		 heightTmp         , widthTmp          , depthTmp          ,
		 redTmp            , greenTmp          , blueTmp            );
    
    normalizeOrientationVector();
}

//-------------------------------------------------------------------------
//------------- ~Molecule
//-------------------------------------------------------------------------
/*!
 *  A standard destructor.
 *  \author Adrian Gabriel
 *  \date June 2005
 */
mga::Molecule::~Molecule()
{
    
}

//-------------------------------------------------------------------------
//------------- initMembers
//-------------------------------------------------------------------------
/*!
 *  By the use of this function all class members can easily be initialized
 *  (e.g. in constructor).
 *  \param positionXtmp sets Molecule::positionX
 *  \param positionYtmp sets Molecule::positionY
 *  \param positionZtmp sets Molecule::positionZ
 *  \param orientationXtmp sets Molecule::orientationX
 *  \param orientationYtmp sets Molecule::orientationY
 *  \param orientationZtmp sets Molecule::orientationZ
 *  \param typeTmp sets Molecule::type
 *  \param heightTmp sets Molecule::height
 *  \param widthTmp sets Molecule::width
 *  \param depthTmp sets Molecule::depth
 *  \param redTmp sets Molecule::red
 *  \param greenTmp sets Molecule::green
 *  \param blueTmp sets Molecule::blue
 *  \return void.
 *  \note All parameters come with "0" as default value.
 *  \author Adrian Gabriel
 *  \date June 2005
 */
void mga::Molecule::initMembers( double positionXtmp      , double positionYtmp      , double positionZtmp      ,
				 double positionFoldedXtmp, double positionFoldedYtmp, double positionFoldedZtmp,
				 double orientationXtmp   , double orientationYtmp   , double orientationZtmp   ,
				 int    typeTmp           , unsigned int numberTmp   ,
				 double heightTmp         , double widthTmp          , double depthTmp          ,
				 double redTmp            , double greenTmp          , double blueTmp             )
{
    positionX = positionXtmp;
    positionY = positionYtmp;
    positionZ = positionZtmp;
    positionFoldedX = positionFoldedXtmp;
    positionFoldedY = positionFoldedYtmp;
    positionFoldedZ = positionFoldedZtmp;
    orientationX = orientationXtmp;
    orientationY = orientationYtmp;
    orientationZ = orientationZtmp;
    type  = typeTmp;
    number = numberTmp;
    height = heightTmp;
    width  = widthTmp;
    depth  = depthTmp;
    red   = redTmp;
    green = greenTmp;
    blue  = blueTmp;
}

//-------------------------------------------------------------------------
//------------- getRGB
//-------------------------------------------------------------------------
/*!
 *  Takes three references to double values and sets them
 *  to the values of red, green and blue.
 *  \param redTmp Reference that is set to the value of Molecule::red.
 *  \param greenTmp Reference that is set to the value of Molecule::green.
 *  \param blueTmp Reference that is set to the value of Molecule::blue.
 *  \return void
 *  \author Adrian Gabriel
 *  \date June 2005
*/
void mga::Molecule::getRGB( double& redTmp, double& greenTmp, double& blueTmp ) const
{
    redTmp   = red;
    greenTmp = green;
    blueTmp  = blue;
}

//-------------------------------------------------------------------------
//------------- getOrientationXYZ
//-------------------------------------------------------------------------
/*!
 *  Takes three references to double values and sets them
 *  to the values of orientationX, ...Y, and ...Z.
 *  \param xTmp Reference that is set to the value of Molecule::orientationX.
 *  \param yTmp Reference that is set to the value of Molecule::orientationY.
 *  \param zTmp Reference that is set to the value of Molecule::orientationZ.
 *  \return void
 *  \author Adrian Gabriel
 *  \author Adrian Gabriel
 *  \date June 2005
 */
void mga::Molecule::getOrientationXYZ( double& xTmp, double& yTmp, double& zTmp ) const
{
    xTmp = orientationX; 
    yTmp = orientationY; 
    zTmp = orientationZ;
}

//-------------------------------------------------------------------------
//------------- getPositionXYZ
//-------------------------------------------------------------------------
/*!
 *  Takes three references to double values and sets them
 *  to the values of positionX, ...Y, and ...Z.
 *  \param xTmp Reference that is set to the value of Molecule::positionX.
 *  \param yTmp Reference that is set to the value of Molecule::positionY.
 *  \param zTmp Reference that is set to the value of Molecule::positionZ.
 *  \return void
 *  \author Adrian Gabriel
 *  \date June 2005
 */
void mga::Molecule::getPositionXYZ( double& xTmp, double& yTmp, double& zTmp ) const
{
  xTmp = positionX;
  yTmp = positionY;
  zTmp = positionZ;
}

//-------------------------------------------------------------------------
//------------- getPositionFoldedXYZ
//-------------------------------------------------------------------------
/*!
 *  Takes three references to double values and sets them
 *  to the values of positionFoldedX, ...Y, and ...Z.
 *  \param xTmp Reference that is set to the value of Molecule::positionFoldedX.
 *  \param yTmp Reference that is set to the value of Molecule::positionFoldedY.
 *  \param zTmp Reference that is set to the value of Molecule::positionFoldedZ.
 *  \return void
 *  \author Adrian Gabriel
 *  \date Sept 2005
 */
void mga::Molecule::getPositionFoldedXYZ( double& xTmp, double& yTmp, double& zTmp ) const
{
    xTmp = positionFoldedX;
    yTmp = positionFoldedY;
    zTmp = positionFoldedZ;
}

//-------------------------------------------------------------------------
//------------- setRGB
//-------------------------------------------------------------------------
/*!
 *  Sets R, G, B values of molecule in one go.
 *  \param redTmp Sets Molecule::red.
 *  \param greenTmp Sets Molecule::green.
 *  \param blueTmp Sets Molecule::blue.
 *  \return void
 *  \author Adrian Gabriel
 *  \date June 2005
 */
void  mga::Molecule::setRGB( double redTmp, double  greenTmp, double  blueTmp )
{
    red   = redTmp;
    green = greenTmp;
    blue  = blueTmp;
}

//-------------------------------------------------------------------------
//------------- setOrientationXYZ
//-------------------------------------------------------------------------
/*!
 *  When his function is called the vector is automatically normalized to 1.
 *  \param xTmp Sets Molecule::orientationX
 *  \param yTmp Sets Molecule::orientationY
 *  \param zTmp Sets Molecule::orientationZ
 *  \return void.
 *  \author Adrian Gabriel
 *  \date June 2005
 */
void mga::Molecule::setOrientationXYZ( double xTmp, double yTmp, double zTmp )
{
    orientationX = xTmp;
    orientationY = yTmp;
    orientationZ = zTmp;
    normalizeOrientationVector();
    
    //cout << "Molecule:: " << getOrientationX() << " " << getOrientationY() << " " << getOrientationZ() << endl;
}

//-------------------------------------------------------------------------
//------------- setPositionXYZ
//-------------------------------------------------------------------------
/*!
 *  Sets positionX, ...Y, and ...Z values in one go.
 *  \param xTmp Sets Molecule::positionX.
 *  \param yTmp Sets Molecule::positionY.
 *  \param zTmp Sets Molecule::positionZ.
 *  \return void
 *  \author Adrian Gabriel
 *  \date June 2005
*/
void mga::Molecule::setPositionXYZ( double xTmp, double  yTmp, double  zTmp )
{
    positionX = xTmp; 
    positionY = yTmp; 
    positionZ = zTmp;
}

//-------------------------------------------------------------------------
//------------- setPositionFoldedXYZ
//-------------------------------------------------------------------------
/*!
 *  Sets positionFoldedX, ...Y, and ...Z values in one go.
 *  \param xTmp Sets Molecule::positionFoldedX.
 *  \param yTmp Sets Molecule::positionFoldedY.
 *  \param zTmp Sets Molecule::positionFoldedZ.
 *  \return void
 *  \author Adrian Gabriel
 *  \date Sept 2005
*/
void mga::Molecule::setPositionFoldedXYZ( double xTmp, double  yTmp, double  zTmp )
{
    positionFoldedX = xTmp; 
    positionFoldedY = yTmp; 
    positionFoldedZ = zTmp;
}

//-------------------------------------------------------------------------
//------------- print
//-------------------------------------------------------------------------
/*!
 *  This function prints all members of a Molecule-object to std-out.
 *  \return void.
 *  \author Adrian Gabriel
 *  \date June 2005
 */
void mga::Molecule::print() const
{
    cout << setw(12) << positionX       << setw(12) << positionY       << setw(12) << positionZ
	    << setw(12) << positionFoldedX << setw(12) << positionFoldedY << setw(12) << positionFoldedZ
	    << setw(12) << orientationX    << setw(12) << orientationY    << setw(12) << orientationZ
	    << setw(12) << type
	    << setw(12) << height          << setw(12) << width           << setw(12) << depth
	    << setw(12) << red             << setw(12) << green           << setw(12) << blue
	    << endl;
}

//-------------------------------------------------------------------------
//------------- print
//-------------------------------------------------------------------------
/*!
 *  Overloaded version writing output to a sringstream passed by reference.
 *  \return void.
 *  \author Adrian Gabriel
 *  \date June 2005
 */
void mga::Molecule::print( stringstream &stream ) const
{
    stream << setw(12) << positionX       << setw(12) << positionY       << setw(12) << positionZ
	    << setw(12) << positionFoldedX << setw(12) << positionFoldedY << setw(12) << positionFoldedZ
	    << setw(12) << orientationX    << setw(12) << orientationY    << setw(12) << orientationZ
	    << setw(12) << type
	    << setw(12) << height          << setw(12) << width           << setw(12) << depth
	    << setw(12) << red             << setw(12) << green           << setw(12) << blue
	    << endl;
}

//-------------------------------------------------------------------------
//------------- normalizeOrientationVector
//-------------------------------------------------------------------------
/*!
 *  This function recalculates the orientation vector components, as to normalize
 *  the vector to 1.
 *  \return void.
 *  \author Adrian Gabriel
 *  \date June 2005
 */
void mga::Molecule::normalizeOrientationVector()
{
    double norm = sqrt( pow(orientationX,2) + pow(orientationY,2) + pow(orientationZ,2) );
    if( norm )
    {
	orientationX = orientationX / norm;
	orientationY = orientationY / norm;
	orientationZ = orientationZ / norm;
    }
    else
    {
	cerr << "Error: OrientationVector of molecule with norm = 0... setting to {1,0,0}" << endl;
	orientationX = 1.0;
	orientationY = 0.0;
	orientationZ = 0.0;      
    }
}


//--------------------------------------------
//------------ MoleculeBiax
//--------------------------------------------

//-------------------------------------------------------------------------
//------------- MoleculeBiax (quaternion)
//-------------------------------------------------------------------------
/*!
 *  Constructor of the MoleculeBiax-class.
 *  \author Matteo Ricci
 *  \date June 2008
 */
mga::MoleculeBiax::MoleculeBiax( double positionXtmp      , double positionYtmp      , double positionZtmp      ,
				 double positionFoldedXtmp, double positionFoldedYtmp, double positionFoldedZtmp,
				 double orientationWtmp   , double orientationXQtmp  , double orientationYQtmp  , double orientationZQtmp   ,
				 int    typeTmp           , unsigned int numberTmp   ,                                            
				 double heightTmp         , double widthTmp          , double depthTmp          ,
				 double redTmp            , double greenTmp          , double blueTmp         )
: Molecule(positionXtmp      , positionYtmp      , positionZtmp      ,
	   positionFoldedXtmp, positionFoldedYtmp, positionFoldedZtmp,
	   0.0, 0.0, 1.0, // set to correct values by setOrientationWXYZ()
	   typeTmp           , numberTmp         ,                                         
	   heightTmp         , widthTmp          , depthTmp          ,
	   redTmp            , greenTmp          , blueTmp            )
{
    //cout << setprecision(4);
    //cout << "Quaternion: w: " << orientationWtmp << " x: " << orientationXQtmp << " y: " << orientationYQtmp << " z: " << orientationZQtmp << endl;
    
    // that way the orientationX/Y/Z values are set too and normalization is also taken care of.
    setOrientationWXYZ( orientationWtmp, orientationXQtmp, orientationYQtmp, orientationZQtmp );
}

//-------------------------------------------------------------------------
//------------- MoleculeBiax (vector)
//-------------------------------------------------------------------------
/*!
 *  Constructor of the MoleculeBiax-class.
 *  \author Matteo Ricci
 *  \date June 2008
 */
mga::MoleculeBiax::MoleculeBiax( double positionXtmp      , double positionYtmp      , double positionZtmp      ,
				 double positionFoldedXtmp, double positionFoldedYtmp, double positionFoldedZtmp,
				 double orientationXtmp   , double orientationYtmp , double orientationZtmp  ,
				 int    typeTmp           , unsigned int numberTmp   ,                                            
				 double heightTmp         , double widthTmp          , double depthTmp          ,
				 double redTmp            , double greenTmp          , double blueTmp  )
: Molecule(positionXtmp      , positionYtmp      , positionZtmp      ,
	   positionFoldedXtmp, positionFoldedYtmp, positionFoldedZtmp,
	   orientationXtmp   , orientationYtmp   , orientationZtmp   ,
	   typeTmp           , numberTmp         ,                                         
	   heightTmp         , widthTmp          , depthTmp          ,
	   redTmp            , greenTmp          , blueTmp            )
{
    //cout << setprecision(4);
    //cout << "Vector: x: " << orientationXtmp << " y: " << orientationYtmp << " z: " << orientationZtmp << endl;
    
    generateQuaternionForUniaxialParticles();
}

//-------------------------------------------------------------------------
//------------- ~MoleculeBiax
//-------------------------------------------------------------------------
/*!
 *  A standard destructor.
 *  \author Matteo Ricci
 *  \date June 2008
 */
mga::MoleculeBiax::~MoleculeBiax()
{    
}

//-------------------------------------------------------------------------
//------------- MakeQuat
//-------------------------------------------------------------------------
std::vector<double> mga::MoleculeBiax::MakeQuat(double angle, double x, double y, double z) const
{
  std::vector<double> vec;

  vec.push_back(cos(angle*0.5));
  vec.push_back(sin(angle*0.5)*x);
  vec.push_back(sin(angle*0.5)*y);
  vec.push_back(sin(angle*0.5)*z);

  return vec;
}

//-------------------------------------------------------------------------
//------------- MultiplyQ
//-------------------------------------------------------------------------
void mga::MoleculeBiax::MultiplyQ(double q1, double q2, double q3, double q4)
{
  // quaternion product
  //
  // [a,A]*[b,B]=[ab-A.B,aB+bA+AxB]
  
  double tmp[4];
  tmp[0] = orientationW *q1 - orientationXQ*q2 -  orientationYQ*q3 - orientationZQ*q4 ;
  tmp[1] = orientationXQ*q1 + orientationW *q2 + (orientationZQ*q3 - orientationYQ*q4);
  tmp[2] = orientationYQ*q1 + orientationW *q3 + (orientationXQ*q4 - orientationZQ*q2);
  tmp[3] = orientationZQ*q1 + orientationW *q4 + (orientationYQ*q2 - orientationXQ*q3);

  orientationW  = tmp[0];
  orientationXQ = tmp[1];
  orientationYQ = tmp[2];
  orientationZQ = tmp[3];
  NormalizeQuaternion();
  
  vector<double> orientationTmp = QuatToVector( orientationW, orientationXQ, orientationYQ, orientationZQ );
  orientationX = orientationTmp.at(0);
  orientationY = orientationTmp.at(1);
  orientationZ = orientationTmp.at(2);
  normalizeOrientationVector();
}

//-------------------------------------------------------------------------
//------------- QuatToVector
//-------------------------------------------------------------------------
std::vector<double> mga::MoleculeBiax::QuatToVector(double w, double x, double y, double z) const
{                                                        //w         q1       q2        q3
    // The quaternion (normalized!) represented by a rotation matrix looks like:
    // 1 - 2 * (   y*y + z*z )         2 * ( - w*z + x*y )         2 * (   w*y + x*z )
    //     2 * (   w*z + x*y )     1 - 2 * (   x*x + z*z )         2 * ( - w*x + y*z )
    //     2 * ( - w*y + x*z )         2 * (   w*x + y*z )     1 - 2 * (   x*x + y*y )
        
    // rotating the vector (0,0,R) leaves the left column of the matrix
    double a =     2 * (   w*y + x*z );
    double b =     2 * ( - w*x + y*z );
    double c = 1 - 2 * (   x*x + y*y );
    
    vector<double> vec;
    vec.push_back(a);
    vec.push_back(b);
    vec.push_back(c);
    return vec;
}

//-------------------------------------------------------------------------
//------------- QuaternionToAxisAngle
//-------------------------------------------------------------------------
//
//!  Given a quaternion, compute an axis and angle.
//
vector<double> mga::MoleculeBiax::QuaternionToAxisAngle()
{
  vector<double> axisAngle;
  // if w>1 acos and sqrt will produce errors, this cant happen if quaternion is normalised
  //if (orientationW > 1.0) 
  //NormalizeQuaternion(); // no longer necessary. there should be no way of a non normalized quaternion to enter the class...

  // errors this cant happen if quaternion is normalised
  double axis[3] = {orientationXQ, orientationYQ, orientationZQ};
  
  // test to avoid divide by zero, s is always positive due to sqrt
  // if s close to zero then direction of axis not important
  // if it is important that axis is normalised then replace with 
  // x = 1; y = z =0
  double s = sqrt(1.0 - orientationW*orientationW); // assuming quaternion normalised then w is less than 1, so term always positive.
  if (s > 0.001)
    {
      axis[0] /= s;
      axis[1] /= s;
      axis[2] /= s;
    }
  
  axisAngle.push_back(axis[0]);
  axisAngle.push_back(axis[1]);
  axisAngle.push_back(axis[2]);
  axisAngle.push_back(RadToDeg*(2.0 * acos(orientationW)));

  return axisAngle;
}

//-------------------------------------------------------------------------
//------------- QuaternionFromAxisAngle
//-------------------------------------------------------------------------
std::vector<double> mga::MoleculeBiax::QuaternionFromAxisAngle(double gamma, double axisX, double axisY, double axisZ)
{
  //cout << "[QuaternionFromAxisAngle] q: " << gamma << " "  <<  axisX << " "  <<  axisY << " "  <<  axisZ <<endl;
  std::vector<double> q;
  gamma *= (M_PI/180.0);
  double sine = sin(0.5*gamma);
  
  q.push_back(cos(0.5*gamma));
  q.push_back(axisX*sine);
  q.push_back(axisY*sine);
  q.push_back(axisZ*sine);
  //q.push_back(cos(0.5*gamma)); // I think of a quaternion as ( w, x, y, z )
  //cout << "[QuaternionFromAxisAngle] q: " << q[0] << " " << q[1] << " " << q[2] << " " << q[3] << " " <<endl;
  return q;
}


//-------------------------------------------------------------------------
//------------- generateQuaternionForUniaxialParticles
//-------------------------------------------------------------------------
void mga::MoleculeBiax::generateQuaternionForUniaxialParticles()
{
    // here the a vector representing the initial orientation of a freshly created particle (startVec..)
    // gets rotated on a new vector (aimVec..)
    // this is done by calculating the angle between the vectors like
    // a.b = |a||b|cos(angle) <-> arccos(a.b/|a||b|) = angle
    // and the vector to rotate about (rotVec..) is calculated as the vector product of the two vectors.
    
    // the code here is obviously way too detailed. But I will leave it that way, until the
    // issue about object creation in the render code is solved (y,z-swapping)
    
    double angle, rotVecX, rotVecY, rotVecZ, startVecX, startVecY, startVecZ, aimVecX, aimVecY, aimVecZ;
    
    startVecX = 0.0;               startVecY = 0.0;               startVecZ = 1.0;
    aimVecX   = getOrientationX(); aimVecY   = getOrientationY(); aimVecZ = getOrientationZ();
     
    double norm = sqrt( pow(startVecX,2) + pow(startVecY,2) + pow(startVecZ,2) );
    if( norm )
    {
	startVecX /= norm;
	startVecY /= norm;
	startVecZ /= norm;
    }
    
    norm = sqrt( pow(aimVecX,2) + pow(aimVecY,2) + pow(aimVecZ,2) );
    if( norm )
    {
	aimVecX /= norm;
	aimVecY /= norm;
	aimVecZ /= norm;
    }	
       
    //cout << "startvec: " << startVecX << " " << startVecY << " " << startVecZ << endl;
    //cout << "aimvec:   " << aimVecX   << " " << aimVecY   << " " << aimVecZ   << endl;
    
    // vector product (i know, there are a lot of zeros in there, but at the moment i want it to be somewhat general
    // just in case the startVec changes)
    if( aimVecX == startVecX && aimVecY == startVecY && (aimVecZ == startVecZ || aimVecZ == -startVecZ) ) // If the start and aimvectors are parallel or antiparallel
    {                                                                                                     //  choose the x-axis as the rotation vector
	rotVecX = 1.0;
	rotVecY = 0.0;
	rotVecZ = 0.0;
    }
    else                                                                                                  // otherwise carry out the vector product of the former two.
    {
	rotVecX =   startVecY*aimVecZ - startVecZ*aimVecY;
	rotVecY = -(startVecX*aimVecZ - startVecZ*aimVecX);
	rotVecZ =   startVecX*aimVecY - startVecY*aimVecX;
    }
    norm = sqrt( pow(rotVecX,2) + pow(rotVecY,2) + pow(rotVecZ,2) );
    if( norm )
    {
	rotVecX /= norm;
	rotVecY /= norm;
	rotVecZ /= norm;
    }	
    
    // angle in degrees
    angle = acos(startVecX*aimVecX + startVecY*aimVecY + startVecZ*aimVecZ) / M_PI * 180.0;
    
    cout << setprecision(5);
    //cout << "MoleculeBiax::generateQuaternionForUniaxialParticles() axis/angle:  " << rotVecX   << " " << rotVecY   << " " << rotVecZ << " angle: " << angle << endl;
    
    vector<double> quat = QuaternionFromAxisAngle( angle, rotVecX, rotVecY, rotVecZ );
    //cout << "quat:  " << quat.at(0) << " " << quat.at(1) << " " << quat.at(2) << " " << quat.at(3) << endl;
    setOrientationWXYZ( quat.at(0), quat.at(1), quat.at(2), quat.at(3) );
}

//-------------------------------------------------------------------------
//------------- setOrientationXYZ (OVERWRITE BASE CLASS)
//-------------------------------------------------------------------------
/*!
 *  When his function is called the vector is automatically normalized to 1.
 *  \param xTmp Sets MoleculeBiax::orientationX
 *  \param yTmp Sets MoleculeBiax::orientationY
 *  \param zTmp Sets MoleculeBiax::orientationZ
 *  \return void.
 *  \author Matteo Ricci
 *  \date June 2008
 */
void mga::MoleculeBiax::setOrientationXYZ( double xTmp, double yTmp, double zTmp )
{
    //cout << "MoleculeBiax::setOrientationXYZ() beg" << endl;
    orientationX = xTmp;
    orientationY = yTmp;
    orientationZ = zTmp;
    normalizeOrientationVector();
    
    generateQuaternionForUniaxialParticles();
    //cout << "MoleculeBiax::setOrientationXYZ() x: " << getOrientationX() << " y: " << getOrientationY() << " z: " << getOrientationZ() << endl;
    //cout << "MoleculeBiax::setOrientationXYZ() end" << endl;    
}


//-------------------------------------------------------------------------
//------------- NormalizeQuaternion
//-------------------------------------------------------------------------
void mga::MoleculeBiax::NormalizeQuaternion()
{
  double norm = sqrt( orientationW*orientationW + orientationXQ*orientationXQ + orientationYQ*orientationYQ + orientationZQ*orientationZQ);
  orientationW  /= norm;
  orientationXQ /= norm;
  orientationYQ /= norm;
  orientationZQ /= norm;
}

//-------------------------------------------------------------------------
//------------- getOrientationWXYZ
//-------------------------------------------------------------------------
/*!
 *  Takes three references to double values and sets them
 *  to the values of orientationX, ...Y, and ...Z.
 *  \param xTmp Reference that is set to the value of MoleculeBiax::orientationX.
 *  \param yTmp Reference that is set to the value of MoleculeBiax::orientationY.
 *  \param zTmp Reference that is set to the value of MoleculeBiax::orientationZ.
 *  \return void
 *  \author Matteo Ricci
 *  \date June 2008
 */
void mga::MoleculeBiax::getOrientationWXYZ( double& wTmp, double& xTmp, double& yTmp, double& zTmp ) const
{
    wTmp = orientationW; 
    xTmp = orientationXQ; 
    yTmp = orientationYQ; 
    zTmp = orientationZQ;
}

//-------------------------------------------------------------------------
//------------- setOrientationWXYZ
//-------------------------------------------------------------------------
/*!
 *  When his function is called the vector is automatically normalized to 1.
 *  \param xTmp Sets MoleculeBiax::orientationX
 *  \param yTmp Sets MoleculeBiax::orientationY
 *  \param zTmp Sets MoleculeBiax::orientationZ
 *  \return void.
 *  \author Matteo Ricci
 *  \date June 2008
 */
void mga::MoleculeBiax::setOrientationWXYZ( double wTmp, double xTmp, double yTmp, double zTmp )
{
    orientationW  = wTmp;
    orientationXQ = xTmp;
    orientationYQ = yTmp;
    orientationZQ = zTmp;
    NormalizeQuaternion();
    
    //cout << number << " " << orientationW << " " << orientationXQ << " " << orientationYQ << " " << orientationZQ << endl;
       
    vector<double> oTmp = QuatToVector( orientationW, orientationXQ, orientationYQ, orientationZQ );
    orientationX = oTmp.at(0);
    orientationY = oTmp.at(1);
    orientationZ = oTmp.at(2);
    normalizeOrientationVector();
}

//-------------------------------------------------------------------------
//------------- print
//-------------------------------------------------------------------------
/*!
 *  This function prints all members of a MoleculeBiax-object to std-out.
 *  \return void.
 *  \author Adrian Gabriel
 *  \date June 2005
 */
void mga::MoleculeBiax::print() const
{
    cout << setw(12) << positionX       << setw(12) << positionY       << setw(12) << positionZ
	 << setw(12) << positionFoldedX << setw(12) << positionFoldedY << setw(12) << positionFoldedZ
	 << setw(12) << orientationW    << setw(12) << orientationX    << setw(12) << orientationY    << setw(12) << orientationZ
	 << setw(12) << type
	 << setw(12) << height          << setw(12) << width           << setw(12) << depth
	 << setw(12) << red             << setw(12) << green           << setw(12) << blue
	 << endl;
}

//-------------------------------------------------------------------------
//------------- print
//-------------------------------------------------------------------------
/*!
 *  Overloaded version writing output to a sringstream passed by reference.
 *  \return void.
 *  \author Adrian Gabriel
 *  \date June 2005
 */
void mga::MoleculeBiax::print( stringstream &stream ) const
{
    stream << setw(12) << positionX       << setw(12) << positionY       << setw(12) << positionZ
	   << setw(12) << positionFoldedX << setw(12) << positionFoldedY << setw(12) << positionFoldedZ
	   << setw(12) << orientationX    << setw(12) << orientationY    << setw(12) << orientationZ
	   << setw(12) << type
	   << setw(12) << height          << setw(12) << width           << setw(12) << depth
	   << setw(12) << red             << setw(12) << green           << setw(12) << blue
	   << endl;
}


//--------------------------------------------
//------------ CnfFile
//--------------------------------------------

//-------------------------------------------------------------------------
//------------- CnfFile
//-------------------------------------------------------------------------
/*!
 *  \param cnffile Path to the .cnf file which is to be loaded.
 *  \param colormap Path to the colormap which is to be loaded.
 *  \note colormap comes with "color.map" as a default value.
 *  \author Adrian Gabriel
 *  \date Apr 2007
 */
mga::CnfFile::CnfFile( string &cnffile, uint loadIndex, string colorscheme, string colormap )
{                                 // order of following function calls is not arbitrary!
    initMembers( colorscheme );   // always initialize members first
    loadColorMap( colormap );     // colormap can be read before or after cnffile
    loadCnfFileIndex = loadIndex;
    
    if( (*this.*loadCnfFile[loadCnfFileIndex])( cnffile, false ) == false )             // if given file could not be loaded
    {
	cerr << "- I try loading the dummy file." << endl;
	cnffile = "mga_dummy.cnf";                                                      // try to load the standard dummy
	if( (*this.*loadCnfFile[loadCnfFileIndex])( cnffile, false ) == false )         // if this also failes
	{
	    cerr << "- failed! So I try to create one." << endl;
	    if( createTmpDummy() == true )                                              // create temporary dummy (is not automatically deleted)
	    {
		if( (*this.*loadCnfFile[loadCnfFileIndex])( cnffile, false ) == false ) // and try to load this one
		{
		    cerr << "ERROR: something is seriously wrong. Check file formats and program code" << endl;
		    exit(1);
		}
	    }
	    else                                                                       // if simple dummy could not be created
	    {
		cerr << "ERROR: cannot open last opened file." << endl
			<< " - no dummy file present." << endl
			<< " - cannot create dummy." << endl
			<< " - You should try running qmga giving it a file to open via command line." << endl;
		exit(1);
	    }
	}
    }
    if( !checkIntegrity()  ) { cerr << cnffile  << " - Beware! : cnf file integrity corrupt" << endl; } //exit(1); }
}

//-------------------------------------------------------------------------
//------------- createTmpDummy
//-------------------------------------------------------------------------
/*!
 *  Tries to create a simple dummy configuration file
 *  \author Adrian Gabriel
 *  \date Sept 2005
 */
bool mga::CnfFile::createTmpDummy()
{
    ofstream out("mga_dummy.cnf");
    if( out.is_open() )
    {
	out << "17" << endl
		<< "9.0" << endl
		<< "9.0" << endl
		<< "9.0" << endl
		<< "0.000000000000     0.00000000" << endl
		<< "-4.0    0.0    0.0       0 0 0       0.0    1.0    0.0       0 0 0 1 " << endl
		<< "-3.0    0.0    0.0       0 0 0       0.0    2.0    1.0       0 0 0 2 " << endl
		<< "-2.0    0.0    0.0       0 0 0       0.0    1.0    1.0       0 0 0 3 " << endl
		<< "-1.0    0.0    0.0       0 0 0       0.0    1.0    2.0       0 0 0 4 " << endl
		<< " 0.0    0.0    0.0       0 0 0       0.0    0.0    1.0       0 0 0 5 " << endl
		<< " 1.0    0.0    0.0       0 0 0       0.0   -1.0    2.0       0 0 0 6 " << endl
		<< " 2.0    0.0    0.0       0 0 0       0.0   -1.0    1.0       0 0 0 7 " << endl
		<< " 3.0    0.0    0.0       0 0 0       0.0   -2.0    1.0       0 0 0 8 " << endl
		<< " 4.0    0.0    0.0       0 0 0       0.0   -1.0    0.0       0 0 0 9 " << endl
		<< " 0.0   -4.0    0.0       0 0 0       1.0    0.0    0.0       0 0 0 10" << endl
		<< " 0.0   -3.0    0.0       0 0 0       2.0    0.0    1.0       0 0 0 11" << endl
		<< " 0.0   -2.0    0.0       0 0 0       1.0    0.0    1.0       0 0 0 12" << endl
		<< " 0.0   -1.0    0.0       0 0 0       1.0    0.0    2.0       0 0 0 13" << endl
		<< " 0.0    1.0    0.0       0 0 0      -1.0    0.0    2.0       0 0 0 14" << endl
		<< " 0.0    2.0    0.0       0 0 0      -1.0    0.0    1.0       0 0 0 15" << endl
		<< " 0.0    3.0    0.0       0 0 0      -2.0    0.0    1.0       0 0 0 16" << endl
		<< " 0.0    4.0    0.0       0 0 0      -1.0    0.0    0.0       0 0 0 17" << endl;
	out.close();
	return( true );
    }
    return( false );
}

//-------------------------------------------------------------------------
//------------- ~CnfFile
//-------------------------------------------------------------------------
/*!
 *  The destructor has to clean up a little bit.
 *  \li purge() is a external function which deletes all pointers in a stl container.
 *  \li The pointer to the colormap has also to be deleted, of course.
 *
 *  \author Adrian Gabriel
 *  \date June 2005
 */
mga::CnfFile::~CnfFile()
{
    purge( moleculeVector );                                // calls delete and =0 for every pointer in vector
    if( colorMap != 0 ) { delete colorMap; colorMap = 0; }
}


//-------------------------------------------------------------------------
//------------- initMembers
//-------------------------------------------------------------------------
/*!
 *  By the use of this function all class members can easily be initialized
 *  (e.g. in constructor).
 *  \param numMolFileTmp sets CnfFile::numMolFile
 *  \param numMolCntTmp sets CnfFile::numMolCnt
 *  \param boxXtmp sets CnfFile::boxX
 *  \param boxYtmp sets CnfFile::boxY
 *  \param boxZtmp sets CnfFile::boxZ
 *  \param colorMapTmp sets CnfFile::colorMap
 *  \return void.
 *  \note All parameters come with "0" as default value.
 *  \author Adrian Gabriel
 *  \date June 2005
 */
void mga::CnfFile::initMembers( string    colorscheme, int    numMolFileTmp, int    numMolCntTmp,
				double    boxXtmp    , double boxYtmp      , double boxZtmp     ,
				Colormap* colorMapTmp )
{
    //cout << "CnfFile::initMembers beg" << endl;
    numMolFile = numMolFileTmp;
    numMolCnt  = numMolCntTmp ;
    boxX = boxXtmp;
    boxY = boxYtmp;
    boxZ = boxZtmp;
    colorMap = colorMapTmp;
    userDefinedDirector.resize( 3, 0.0 );
    setUserDefinedDirector();
    
    setColorScheme( colorscheme );
    
    boundingBox.resize(3,vector<float>(3,0.0));
    boundingBoxCoordinates.resize(24,vector<float>(3,0.0));
    showFolded    = false;
    alreadyFolded = false;
    
    loadCnfFileIndex = 0;
    loadCnfFile[0] = &mga::CnfFile::loadCnfFile_gbmega;
    loadCnfFile[1] = &mga::CnfFile::loadCnfFile_lammps1;
    loadCnfFile[2] = &mga::CnfFile::loadCnfFile_lammps2;
    loadCnfFile[3] = &mga::CnfFile::loadCnfFile_gbmegaBiax;
    loadCnfFile[4] = &mga::CnfFile::loadCnfFile_cinacchi;
    //loadCnfFile[5] = &mga::CnfFile::loadCnfFile_foo-format;

    //cout << "CnfFile::initMembers end" << endl;
}


//-------------------------------------------------------------------------
//------------- foldMoleculesToBoundingBox
//-------------------------------------------------------------------------
/*!
 *  By the use of this function all molecules position values are folded back into the bounding box.
 *  \note Note that doing so will not loose any information, because the folded positions are stored separately.
 *  \author Adrian Gabriel
 *  \date Sept 2005
 */
void mga::CnfFile::foldMoleculesToBoundingBox()
{
    if( alreadyFolded == true ) { /*cout << "-- alreadyFolded is true" << endl;*/ return; }
    else                        { /*cout << "-- set alreadyFolded to true" << endl;*/ alreadyFolded = true; }
    
    double posX = 0, posY = 0, posZ = 0;
    
    if( !(boundingBox.at(0).at(1) == 0.0 && boundingBox.at(0).at(2) == 0.0 &&
	  boundingBox.at(1).at(0) == 0.0 && boundingBox.at(1).at(2) == 0.0 &&
	  boundingBox.at(2).at(0) == 0.0 && boundingBox.at(2).at(1) == 0.0)   )
    {
	cout << "Warning: Folding of non rectangular bounding boxes is not yet implemented. sorry." << endl;
	return;
    }
    
    for( unsigned int i = 0; i < moleculeVector.size(); ++i ) // loop over all molecules
    {
	moleculeVector.at(i) -> getPositionXYZ      ( posX, posY, posZ );
	moleculeVector.at(i) -> setPositionFoldedXYZ( posX -= rint( posX / boundingBox.at(0).at(0) ) * boundingBox.at(0).at(0),
						      posY -= rint( posY / boundingBox.at(1).at(1) ) * boundingBox.at(1).at(1),
						      posZ -= rint( posZ / boundingBox.at(2).at(2) ) * boundingBox.at(2).at(2) );
    }
}

//-------------------------------------------------------------------------
//------------- calculateBoundingBoxCoordinates()
//-------------------------------------------------------------------------
//
//
//           point_011 /--------------/ point_111
//                   / |            / | 
//                 /   |          /   |
//      point_001 ----------------    | point_101
//                |    |         |    | 
//                |    |         |    | 
//                |    |         |    |
//     point_010  |    /---------|----/ point_110
//                |  /           |  /
//                |/             |/
//      point_000 ---------------- point_100 
//
/*!
 *  Calculates the coordinates of the bounding box and groups them pairwise together as edges.
 *  \author Adrian Gabriel
 *  \date Jan 2008
 */
void mga::CnfFile::calculateBoundingBoxCoordinates()
{
    Point3D vector_a, vector_b, vector_c;    
    Point3D point_000, point_001, point_010, point_100, point_011, point_101, point_110, point_111;
    
    vector_a.setCoordinates( boundingBox.at(0) );
    vector_b.setCoordinates( boundingBox.at(1) );
    vector_c.setCoordinates( boundingBox.at(2) );
    
    point_000 = -0.5*( vector_a + vector_b + vector_c );
    point_100 = point_000 + vector_a;
    point_010 = point_000 + vector_b;
    point_001 = point_000 + vector_c;
    point_110 = point_100 + vector_b;
    point_101 = point_100 + vector_c;
    point_011 = point_010 + vector_c;
    point_111 = point_110 + vector_c;
    
    boundingBoxCoordinates.at( 0) = point_000.getCoordinates();  // edge from point
    boundingBoxCoordinates.at( 1) = point_100.getCoordinates();  //      to point
    
    boundingBoxCoordinates.at( 2) = point_000.getCoordinates();
    boundingBoxCoordinates.at( 3) = point_010.getCoordinates();
    
    boundingBoxCoordinates.at( 4) = point_000.getCoordinates();
    boundingBoxCoordinates.at( 5) = point_001.getCoordinates();
    
    boundingBoxCoordinates.at( 6) = point_100.getCoordinates();
    boundingBoxCoordinates.at( 7) = point_110.getCoordinates();
    
    boundingBoxCoordinates.at( 8) = point_010.getCoordinates();
    boundingBoxCoordinates.at( 9) = point_110.getCoordinates();
    
    boundingBoxCoordinates.at(10) = point_100.getCoordinates();
    boundingBoxCoordinates.at(11) = point_101.getCoordinates();
    
    boundingBoxCoordinates.at(12) = point_110.getCoordinates();
    boundingBoxCoordinates.at(13) = point_111.getCoordinates();
    
    boundingBoxCoordinates.at(14) = point_010.getCoordinates();
    boundingBoxCoordinates.at(15) = point_011.getCoordinates();
    
    boundingBoxCoordinates.at(16) = point_001.getCoordinates();
    boundingBoxCoordinates.at(17) = point_011.getCoordinates();
    
    boundingBoxCoordinates.at(18) = point_001.getCoordinates();
    boundingBoxCoordinates.at(19) = point_101.getCoordinates();
    
    boundingBoxCoordinates.at(20) = point_011.getCoordinates();
    boundingBoxCoordinates.at(21) = point_111.getCoordinates();
   
    boundingBoxCoordinates.at(22) = point_101.getCoordinates();
    boundingBoxCoordinates.at(23) = point_111.getCoordinates();
}

//-------------------------------------------------------------------------
//------------- measureBox
//-------------------------------------------------------------------------
/*!
 *  Determines the edges of the smallest rectangular box into which all molecules will fit.
 *  \author Adrian Gabriel
 *  \date Jan 2008
 */
void mga::CnfFile::measureBox()
{
    double tmpX = 0.0, tmpY = 0.0, tmpZ = 0.0;
    double xMin = 0.0, yMin = 0.0, zMin = 0.0;
    double xMax = 0.0, yMax = 0.0, zMax = 0.0;
    
    for( uint i = 0; i<moleculeVector.size(); ++i )
    {
	//cout << "[mga::CnfFile::measureBox()] " << i << endl;
	moleculeVector.at(i)->getPositionXYZ( tmpX, tmpY, tmpZ );
	if( tmpX > xMax ) { xMax = tmpX; }
	if( tmpY > yMax ) { yMax = tmpY; }
	if( tmpZ > zMax ) { zMax = tmpZ; }
	
	if( tmpX < xMin ) { xMin = tmpX; }
	if( tmpY < yMin ) { yMin = tmpY; }
	if( tmpZ < zMin ) { zMin = tmpZ; }
    }
    
    boxX = 2.0 * max( fabs(xMax), fabs(xMin) );
    boxY = 2.0 * max( fabs(yMax), fabs(yMin) );
    boxZ = 2.0 * max( fabs(zMax), fabs(zMin) );
    
    boxX *= 1.01;
    boxY *= 1.01;
    boxZ *= 1.01;
    
    if( boxX < 0.1 ) { boxX = 0.1; }
    if( boxY < 0.1 ) { boxY = 0.1; }
    if( boxZ < 0.1 ) { boxZ = 0.1; }
}

//-------------------------------------------------------------------------
//------------- loadCnfFile_lammps1
//-------------------------------------------------------------------------
/*!
 *  This function loads the contents of a cnf file. All relevant information is stored in 
 *  member variables of the CnfFile-object.
 *  \par As relevant data is considered:
 *  \li number of molecules inf file (as given by the file itself).
 *  \li x,y,z size of the bounding box
 *  \li x,y,z-Position of each molecule
 *  \li x,y,z-Orientation of each molecule
 *
 *  \param cnffile Path to the cnf file which is to be loaded.
 *  \param reload boolean which decides wether to initially load a cnf file or reload one.
 *  \return void.
 *  \author Adrian Gabriel
 *  \date Jan 2008
 */
bool mga::CnfFile::loadCnfFile_lammps1( string cnffile, bool reload )
{
    //cout << "CnfFile::loadCnfFile_lammps1 beg" << endl;
    bool reloadOriginal = reload;
    
    double         tmp=0, tmp1, offsetX = 0, offsetY = 0, offsetZ = 0;
    vector<double> tmpVec;      // Vector to read in a line
    unsigned int   count  = 0;  // Count how many molecules are read.
    //double a = 0.0, b = 0.0, c = 0.0;
    string         currentLine; // current line as string value
    string         model = "";
    string         tmpStr = "";
    stringstream   strstr;
    numberOfTypes = 1;
    MoleculeBiax *tmpMol = 0;
    
    map<string, double> models;
    
    ifstream in( cnffile.c_str() );
    if( in.is_open() )
    {
	getline( in, currentLine );
	//cout << "1: " << currentLine << endl;
	getline( in, currentLine );
	//cout << "2: " << currentLine << endl;
	getline( in, currentLine );
	//cout << "3: " << currentLine << endl;
	
	if( in.eof() == false ) { in >> numMolFile; }            // Read how many molecules should be in file.
	getline( in, currentLine );                              // get rid of "end of line" character
	//cout << "4: " << currentLine << endl;
	
	getline( in, currentLine );                              
	//cout << "5: " << currentLine << endl;
	getline( in, currentLine );                              
	//cout << "6: " << currentLine << endl;
	tmpVec.clear();
	strstr.clear();
	strstr.str(currentLine);
	strstr >> tmp >> tmp1;
	offsetX = 0.5*(tmp+tmp1);
	
	getline( in, currentLine );                              
	//cout << "7: " << currentLine << endl;
	tmpVec.clear();
	strstr.clear();
	strstr.str(currentLine);
	strstr >> tmp >> tmp1;
	offsetY = 0.5*(tmp+tmp1);
	
	getline( in, currentLine );                              
	//cout << "8: " << currentLine << endl;
	tmpVec.clear();
	strstr.clear();
	strstr.str(currentLine);
	strstr >> tmp >> tmp1;
	offsetZ = 0.5*(tmp+tmp1);
	
	getline( in, currentLine );

      	boundingBox.clear();
	boundingBox.resize(3,vector<float>(3,0.0));
        boundingBox.at(0).at(0) = 2*offsetX;
        boundingBox.at(1).at(1) = 2*offsetY;
        boundingBox.at(2).at(2) = 2*offsetZ;
	calculateBoundingBoxCoordinates();
	
	if( moleculeVector.size() == 0 ) { reload = false; }
	//cout << "test ->" << currentLine << "<-" << endl; // has to be empty
	while( getline( in, currentLine ) )
	{
	    tmpVec.clear();
	    strstr.clear();
	    strstr.str(currentLine);
	    
	    strstr >> tmp;
	    tmpVec.push_back(tmp);
	    
	    strstr >> tmpStr;
	    if( models.find( tmpStr ) == models.end() )
	    {
		tmp = models.size();
		models.insert( std::pair<string,double>( tmpStr, tmp ) );
		tmpVec.push_back(tmp);
	    }
	    else
	    {
		tmp = (*(models.find( tmpStr ))).second;
		tmpVec.push_back(tmp);
	    }
	    
	    while( strstr >> tmp )
	    {
		tmpVec.push_back(tmp);
	    }
	    
	    //cout << "size: " << tmpVec.size() << endl;
	    if( tmpVec.size() != 9 )
	    {
		cerr << "Beware! Check file format: There have to be 9 entries per molecule!" << endl;
		//cout << "CnfFile::loadCnfFile_lammps1 end 0" << endl;
		return( false ); 
	    }
	    
	    // quaternion w, x, y, z
	    // w = tmpVec.at(5)
	    // x = tmpVec.at(6)
	    // y = tmpVec.at(7)
	    // z = tmpVec.at(8)
	    
	    if( uint(tmpVec.at(1))+1 >= numberOfTypes ) { numberOfTypes = uint(tmpVec.at(1))+1; }
	    
	    // Calculate orientation vector from quaternions
	    //a = 1 - 2 * ( tmpVec.at(7)*tmpVec.at(7) + tmpVec.at(8)*tmpVec.at(8) );
	    //b =     2 * ( tmpVec.at(6)*tmpVec.at(7) + tmpVec.at(5)*tmpVec.at(8) );
	    //c =     2 * ( tmpVec.at(6)*tmpVec.at(8) - tmpVec.at(5)*tmpVec.at(7) );

	    //a =     2 * (  tmpVec.at(5)*tmpVec.at(7) + tmpVec.at(6)*tmpVec.at(8) );
	    //b =     2 * ( -tmpVec.at(5)*tmpVec.at(6) + tmpVec.at(7)*tmpVec.at(8) );
	    //c = 1 - 2 * (  tmpVec.at(6)*tmpVec.at(6) - tmpVec.at(7)*tmpVec.at(7) );

	    if( reload == true )
	    {
		if( count == moleculeVector.size()-1 ) { reload = false; }
		tmpMol  = moleculeVector.at   ( count            );
		tmpMol -> setPositionXYZ      ( tmpVec.at(2) - offsetX, tmpVec.at(3) - offsetY, tmpVec.at(4) - offsetZ );
		tmpMol -> setPositionFoldedXYZ( tmpVec.at(2) - offsetX, tmpVec.at(3) - offsetY, tmpVec.at(4) - offsetZ );
		//tmpMol -> setOrientationXYZ   ( a, b, c );
		tmpMol -> setOrientationWXYZ  ( tmpVec.at(5), tmpVec.at(6), tmpVec.at(7), tmpVec.at(8) );
		tmpMol -> setType             ( int(tmpVec.at(1)) );
		tmpMol -> setNumber           ( uint(tmpVec.at(0)) );
	    }
	    else
	    {
//		moleculeVector.push_back( new Molecule( tmpVec.at(2) - offsetX, tmpVec.at(3) - offsetY, tmpVec.at(4) - offsetZ,
//							tmpVec.at(2) - offsetX, tmpVec.at(3) - offsetY, tmpVec.at(4) - offsetZ,
//                                                        a, b, c,
//							int(tmpVec.at(1)),
//							int(tmpVec.at(0)) ) );
		
		moleculeVector.push_back( new MoleculeBiax( tmpVec.at(2) - offsetX, tmpVec.at(3) - offsetY, tmpVec.at(4) - offsetZ,
							    tmpVec.at(2) - offsetX, tmpVec.at(3) - offsetY, tmpVec.at(4) - offsetZ,
							    tmpVec.at(5), tmpVec.at(6), tmpVec.at(7), tmpVec.at(8),
							    int(tmpVec.at(1)),
							    uint(tmpVec.at(0)) ) );
		
		//dynamic_cast<MoleculeBiax*>(moleculeVector.at(moleculeVector.size()-1)) -> setOrientationXYZ   ( a, b, c ); 
	    }
	    ++count;
	}
	in.close();
	
	measureBox();
	
	numMolCnt = count;
	
	if( reloadOriginal == true )
	{
	    for( unsigned int i = count; i < moleculeVector.size(); ++i )
	    {
		delete moleculeVector.at(i);
		moleculeVector.at(i) = 0;
	    }
	    moleculeVector.resize( count );
	}
	if( count == 0 )
	{
	    //cout << "CnfFile::loadCnfFile_lammps1 end 1" << endl;
	    return( false );
	}
	if( calculateDirector() == false ) { return( false ); } // director can only be calculated after reading cnffile
	
	// the colorization of the molecules is now solely started from mainform.ui.h !
	//colorizeMolecules();                                    // to colorize molecules the director has to be calculated
	//cout << "CnfFile::loadCnfFile_lammps1 end 2" << endl;
	return( true );
    }
    else
    {
	cerr << "Error! Cannot open file: " << cnffile << endl;
	//cout << "CnfFile::loadCnfFile_lammps1 end 3" << endl;
	return( false );
    }
    //cout << "CnfFile::loadCnfFile_lammps1 end 4" << endl;   
}

//-------------------------------------------------------------------------
//------------- loadCnfFile_lammps2
//-------------------------------------------------------------------------
/*!
 *  This function loads the contents of a cnf file. All relevant information is stored in 
 *  member variables of the CnfFile-object.
 *  \par As relevant data is considered:
 *  \li number of molecules inf file (as given by the file itself).
 *  \li x,y,z size of the bounding box
 *  \li x,y,z-Position of each molecule
 *  \li x,y,z-Orientation of each molecule
 *
 *  \param cnffile Path to the cnf file which is to be loaded.
 *  \param reload boolean which decides wether to initially load a cnf file or reload one.
 *  \return void.
 *  \author Adrian Gabriel
 *  \date Jan 2008
 */
bool mga::CnfFile::loadCnfFile_lammps2( string cnffile, bool reload )
{
  //cout << "CnfFile::loadCnfFile_lammps2 beg" << endl;
  bool reloadOriginal = reload;
    
  double         tmp=0, tmp1, offsetX = 0, offsetY = 0, offsetZ = 0;
  vector<double> tmpVec;      // Vector to read in a line
  unsigned int   count  = 0;  // Count how many molecules are read.
  string         currentLine; // current line as string value
  string         model = "";
  string         tmpStr = "";
  stringstream   strstr;
  numberOfTypes = 1;
  MoleculeBiax *tmpMol = 0;
    
  ifstream in( cnffile.c_str() );
  if( in.is_open() )
    {
      do
	{
	  getline( in, currentLine );
	}
      while( !in.eof() && currentLine.find("atoms") == string::npos );
      stringstream m;
      m << currentLine;
      m >> numMolFile;

      do
	{
	  getline( in, currentLine );
	}
      while (!in.eof() && currentLine.find("xlo") == string::npos);
      stringstream x;

      x << currentLine;
      x >> tmp >> tmp1;

      offsetX = 0.5*(-tmp+tmp1);
      //cout << "x: " << offsetX << endl;
      
      getline(in, currentLine);
      stringstream y;
      y << currentLine;
      y >> tmp >> tmp1;

      offsetY = 0.5*(-tmp+tmp1);      
      //      cout << "y: " << offsetY << endl;

      getline(in, currentLine);
      stringstream z;
      z << currentLine;
      z >> tmp >> tmp1;

      offsetZ = 0.5*(-tmp+tmp1);
      //      cout << "z: " << offsetZ << endl;

      boundingBox.clear();
      boundingBox.resize(3,vector<float>(3,0.0));
      boundingBox.at(0).at(0) = 2*offsetX;
      boundingBox.at(1).at(1) = 2*offsetY;
      boundingBox.at(2).at(2) = 2*offsetZ;
      calculateBoundingBoxCoordinates();

      do
	{
	  getline( in, currentLine );
	}
      while (!in.eof() && currentLine.find("Atoms") == string::npos);
      getline(in, currentLine);

      tmpVec.clear();

      if( moleculeVector.size() == 0 ) { reload = false; }
      //cout << "test ->" << currentLine << "<-" << endl; // has to be empty

      //uint linecount(0); // equivalent to count variable
      do
	{
	  getline( in, currentLine );	    
	  tmpVec.clear();
	  strstr.clear();
	  strstr.str(currentLine);
	    
	  strstr >> tmp;
	  tmpVec.push_back(tmp);
	    
	  int type;
	  strstr >> type;

	  tmpVec.push_back(type);

	  while( strstr >> tmp )
	    {
	      tmpVec.push_back(tmp);
	    }
	    
	  if( tmpVec.size() != 10 )
	    {
	      cerr << "Beware! Check file format: There have to be 10 entries per molecule!" << endl;
	      cerr << "line: >" << currentLine << "<" << endl;	

	      //cout << "CnfFile::loadCnfFile_lammps2 end 0" << endl;
	      return( false ); 
	    }
	    
	  if( uint(tmpVec.at(1)) >= numberOfTypes ) { numberOfTypes = uint(tmpVec.at(1)); }
	    
	  if( reload == true )
	  {
	      if( count == moleculeVector.size()-1 ) { reload = false; }
	      tmpMol  = moleculeVector.at   ( count            );
	      tmpMol -> setPositionXYZ      ( tmpVec.at(2), tmpVec.at(3), tmpVec.at(4) );
	      tmpMol -> setPositionFoldedXYZ( tmpVec.at(2), tmpVec.at(3), tmpVec.at(4) );
	      tmpMol -> setOrientationWXYZ  ( tmpVec.at(5), tmpVec.at(6), tmpVec.at(7), tmpVec.at(8));
	      tmpMol -> setType             ( int(tmpVec.at(1)-1) );
	      tmpMol -> setNumber           ( int(tmpVec.at(0)) );
	  }
	  else
	  {
	      moleculeVector.push_back( new MoleculeBiax( tmpVec.at(2), tmpVec.at(3), tmpVec.at(4),
							  tmpVec.at(2), tmpVec.at(3), tmpVec.at(4),
							  tmpVec.at(5), tmpVec.at(6), tmpVec.at(7), tmpVec.at(8), 
							  int(tmpVec.at(1)-1),
							  int(tmpVec.at(0)) ) );
	  }
	  ++count;
	}
      while (!in.eof() && count < uint(numMolFile) );
      
      in.close();
	
      measureBox();
	
      numMolCnt = count;
	
      if( reloadOriginal == true )
	{
	  for( unsigned int i = count; i < moleculeVector.size(); ++i )
	    {
	      delete moleculeVector.at(i);
	      moleculeVector.at(i) = 0;
	    }
	  moleculeVector.resize( count );
	}
      if( count == 0 )
	{
	  //cout << "CnfFile::loadCnfFile_lammps2 end 1" << endl;
	  return( false );
	}
      if( calculateDirector() == false ) { return( false ); } // director can only be calculated after reading cnffile
      
      //cout << "CnfFile::loadCnfFile_lammps2 end 2" << endl;
      return( true );
    }
  else
    {
      cerr << "Error! Cannot open file: " << cnffile << endl;
      //cout << "CnfFile::loadCnfFile_lammps2 end 3" << endl;
      return( false );
    }
  //cout << "CnfFile::loadCnfFile_lammps2 end 4" << endl;   
}

//-------------------------------------------------------------------------
//------------- loadCnfFile_gbmega
//-------------------------------------------------------------------------
/*!
 *  This function loads the contents of a cnf file. All relevant information is stored in 
 *  member variables of the CnfFile-object.
 *  \par As relevant data is considered:
 *  \li number of molecules inf file (as given by the file itself).
 *  \li x,y,z size of the bounding box
 *  \li x,y,z-Position of each molecule
 *  \li x,y,z-Orientation of each molecule
 *
 *  \param cnffile Path to the cnf file which is to be loaded.
 *  \param reload boolean which decides wether to initially load a cnf file or reload one.
 *  \return void.
 *  \author Adrian Gabriel
 *  \date Jan 2008
 */
bool mga::CnfFile::loadCnfFile_gbmega( string cnffile, bool reload )
{
    //cout << endl << "CnfFile::loadCnfFile beg" << endl;
    bool reloadOriginal = reload;
    
    double         tmp=0;
    vector<double> tmpVec;      // Vector to read in a line
    unsigned int   count  = 0;  // Count how many molecules are read.
    string         currentLine; // current line as string value
    stringstream   strstr;
    numberOfTypes = 1;
    
    MoleculeBiax *tmpMol = 0;
    
    ifstream in( cnffile.c_str() );
    if( in.is_open() )
    {
	if( in.eof() == false ) { in >> numMolFile; }            // Read how many molecules should be in file.
	getline( in, currentLine ); // get rid of "end of line" character
	
	// read bounding box values
	boundingBox.clear();
	boundingBox.resize(3,vector<float>(3,0.0));
	float num_matrix_entries = 0;
	for( int i = 0; i<3; ++i )
	{
	    if( in.eof() == false )
	    {
		getline( in, currentLine );
		strstr.clear();
		strstr.str(currentLine);
		int cnt = 0;
		while( (strstr >> tmp) && (cnt < 3) )
		{
		    boundingBox.at(i).at(cnt) = tmp;
		    ++cnt;
		    ++num_matrix_entries;
		}
		if( cnt != 1 && cnt != 3 ) 
		{
		    cerr << "Beware! Check file format: There have to be either 1 or 3 entries per boundingbox line!" << endl;
		    //cout << "CnfFile::loadCnfFile end -1" << endl;
		    return( false ); 
		}
	    }
	}
	
	if( num_matrix_entries/3.0 == 1 )
	{
	    boundingBox.at(1).at(1) = boundingBox.at(1).at(0);
	    boundingBox.at(1).at(0) = 0.0;
	    boundingBox.at(2).at(2) = boundingBox.at(2).at(0);
	    boundingBox.at(2).at(0) = 0.0;
	}
	
	calculateBoundingBoxCoordinates();
	
	if( in.eof() == false ) { in >> tmp; }
	if( in.eof() == false ) { in >> tmp; }  // Values for moving boundary conditions, not used.
	
	if( moleculeVector.size() == 0 ) { reload = false; }
	getline( in, currentLine ); // get rid of "end of line" character
	//cout << "test ->" << currentLine << "<-" << endl; // has to be empty
	while( getline( in, currentLine ) )
	{
	    tmpVec.clear();
	    strstr.clear();
	    strstr.str(currentLine);
	    while( strstr >> tmp )
	    {
		tmpVec.push_back(tmp);
	    }
	    
	    if( tmpVec.size() == 13 ) { tmpVec.push_back(0.0); }
	    else if( tmpVec.size() != 14 )
	    {
		cerr << "Beware! Check file format: There have to be either 13 or 14 entries per molecule!" << endl;
		//cout << "CnfFile::loadCnfFile end 0" << endl;
		return( false ); 
	    }
	    
	    if( uint(tmpVec.at(13))+1 >= numberOfTypes ) { numberOfTypes = uint(tmpVec.at(13))+1; }

	    if( reload == true )
	    {
		if( count == moleculeVector.size()-1 ) { reload = false; }
		
		tmpMol  = moleculeVector.at   ( count            );
		tmpMol -> setPositionXYZ      ( tmpVec.at(0), tmpVec.at(1), tmpVec.at(2) );
		tmpMol -> setPositionFoldedXYZ( tmpVec.at(0), tmpVec.at(1), tmpVec.at(2) );
		tmpMol -> setOrientationXYZ   ( tmpVec.at(6), tmpVec.at(7), tmpVec.at(8) ); 
		tmpMol -> setType             ( int(tmpVec.at(13)) );
		tmpMol -> setNumber           ( uint(tmpVec.at(12)) );
	    }
	    else
	    {
		moleculeVector.push_back( new MoleculeBiax( tmpVec.at(0), tmpVec.at(1), tmpVec.at(2),
							    tmpVec.at(0), tmpVec.at(1), tmpVec.at(2),
							    tmpVec.at(6), tmpVec.at(7), tmpVec.at(8), // overloaded constructor
							    int(tmpVec.at(13)),
							    uint(tmpVec.at(12)) ) );
	    }	
	    ++count;
	}
	in.close();
	numMolCnt = count;
	
	measureBox();
	
	if( reloadOriginal == true )
	{
	    for( unsigned int i = count; i < moleculeVector.size(); ++i )
	    {
		delete moleculeVector.at(i);
		moleculeVector.at(i) = 0;
	    }
	    moleculeVector.resize( count ); 
	}
	if( count == 0 ) 
	{
	    //cout << "CnfFile::loadCnfFile end 1" << endl;
	    return( false ); 
	}
	if( calculateDirector() == false ) { return( false ); } // director can only be calculated after reading cnffile
	
	//cout << "CnfFile::loadCnfFile end 2" << endl;
	return( true );
    }
    else
    {
	cerr << "Error! Cannot open file: " << cnffile << endl;
	//cout << "CnfFile::loadCnfFile end 3" << endl;
	return( false );
    }
    //cout << "CnfFile::loadCnfFile end 4" << endl;
}

//-------------------------------------------------------------------------
//------------- loadCnfFile_gbmegaBiax
//-------------------------------------------------------------------------
/*!
 *  This function loads the contents of a cnf file. All relevant information is stored in 
 *  member variables of the CnfFile-object.
 *  \par As relevant data is considered:
 *  \li number of molecules inf file (as given by the file itself).
 *  \li x,y,z size of the bounding box
 *  \li x,y,z-Position of each molecule
 *  \li x,y,z-Orientation of each molecule
 *
 *  \param cnffile Path to the cnf file which is to be loaded.
 *  \param reload boolean which decides wether to initially load a cnf file or reload one.
 *  \return void.
 *  \author Matteo Ricci
 *  \date Mar 2008
 */
bool mga::CnfFile::loadCnfFile_gbmegaBiax( string cnffile, bool reload )
{
    //cout << "CnfFile::loadCnfFile_gbmegaBiax beg" << endl;
    bool reloadOriginal = reload;
    
    double         tmp=0;
    vector<double> tmpVec;      // Vector to read in a line
    unsigned int   count  = 0;  // Count how many molecules are read.
    string         currentLine; // current line as string value
    stringstream   strstr;
    numberOfTypes = 1;
    
    MoleculeBiax *tmpMol = 0;
    
    ifstream in( cnffile.c_str() );
    if( in.is_open() )
    {
	if( in.eof() == false ) { in >> numMolFile; }            // Read how many molecules should be in file.
	getline( in, currentLine ); // get rid of "end of line" character
	
	// read bounding box values
	boundingBox.clear();
	boundingBox.resize(3,vector<float>(3,0.0));
	float num_matrix_entries = 0;
	for( int i = 0; i<3; ++i )
	{
	    if( in.eof() == false )
	    {
		getline( in, currentLine );
		strstr.clear();
		strstr.str(currentLine);
		int cnt = 0;
		while( (strstr >> tmp) && (cnt < 4) )
		{
		    boundingBox.at(i).at(cnt) = tmp;
		    ++cnt;
		    ++num_matrix_entries;
		}
		if( cnt != 1 && cnt != 3 ) 
		{
		    cerr << "Beware! Check file format: There have to be either 1 or 3 entries per boundingbox line!" << endl;
		    //cout << "CnfFile::loadCnfFile_gbmegaBiax end -1" << endl;
		    return( false ); 
		}
	    }
	}
	
	if( num_matrix_entries/3.0 == 1 )
	{
	    boundingBox.at(1).at(1) = boundingBox.at(1).at(0);
	    boundingBox.at(1).at(0) = 0.0;
	    boundingBox.at(2).at(2) = boundingBox.at(2).at(0);
	    boundingBox.at(2).at(0) = 0.0;
	}
	
	calculateBoundingBoxCoordinates();
	
	if( in.eof() == false ) { in >> tmp; }
	if( in.eof() == false ) { in >> tmp; }  // Values for moving boundary conditions, not used.
	
	if( moleculeVector.size() == 0 ) { reload = false; }
	getline( in, currentLine ); // get rid of "end of line" character

	//cout << "test ->" << currentLine << "<-" << endl; // has to be empty
	while( getline( in, currentLine ) )
	{
	    tmpVec.clear();
	    strstr.clear();
	    strstr.str(currentLine);
	    while( strstr >> tmp )
	    {
		tmpVec.push_back(tmp);
	    }
	    
	    if( tmpVec.size() == 14 ) { tmpVec.push_back(0.0); }
	    if( tmpVec.size() != 15 )
	    {
		cerr << "Beware! Check file format: There have to be either 14 or 15 entries per molecule!" << endl;
		//cout << "CnfFile::loadCnfFile_gbmegaBiax end 0" << endl;
		return( false ); 
	    }

	    if( uint(tmpVec.at(14))+1 >= numberOfTypes ) { numberOfTypes = uint(tmpVec.at(14))+1; }

	    if( reload == true )
	    {
		if( count == moleculeVector.size()-1 ) { reload = false; }
		tmpMol  = moleculeVector.at   ( count );
		tmpMol -> setPositionXYZ      ( tmpVec.at(0), tmpVec.at(1), tmpVec.at(2) );
		tmpMol -> setPositionFoldedXYZ( tmpVec.at(0), tmpVec.at(1), tmpVec.at(2) );
		tmpMol -> setOrientationWXYZ  ( tmpVec.at(6), tmpVec.at(7), tmpVec.at(8), tmpVec.at(9));
		tmpMol -> setType             ( int(tmpVec.at(14)) );
		tmpMol -> setNumber           ( uint(tmpVec.at(13)) );
	    }
	    else
	    {
		moleculeVector.push_back( new MoleculeBiax( tmpVec.at(0), tmpVec.at(1), tmpVec.at(2),
							    tmpVec.at(0), tmpVec.at(1), tmpVec.at(2),
							    tmpVec.at(6), tmpVec.at(7), tmpVec.at(8), tmpVec.at(9), 
							    int(tmpVec.at(14)),
							    uint(tmpVec.at(13)) ) );
	    }
	    ++count;
	}
	in.close();
	numMolCnt = count;
	
	measureBox();
	
	if( reloadOriginal == true )
	{
	    for( unsigned int i = count; i < moleculeVector.size(); ++i )
	    {
		delete moleculeVector.at(i);
		moleculeVector.at(i) = 0;
	    }
	    moleculeVector.resize( count ); 
	}
	if( count == 0 ) 
	{
	  //cout << "CnfFile::loadCnfFile_gbmegaBiax end 1" << endl;
	    return( false ); 
	}
	if( calculateDirector() == false ) { return( false ); } // director can only be calculated after reading cnffile
	
	//cout << "CnfFile::loadCnfFile_gbmegaBiax end 2" << endl;
	return( true );
    }
    else
    {
	cerr << "Error! Cannot open file: " << cnffile << endl;
	//cout << "CnfFile::loadCnfFile_gbmegaBiax end 3" << endl;
	return( false );
    }
    //cout << "CnfFile::loadCnfFile_gbmegaBiax end 4" << endl;
}

//-------------------------------------------------------------------------
//------------- loadCnfFile_cinacchi
//-------------------------------------------------------------------------
/*!
 *  This function loads the contents of a cnf file. All relevant information is stored in 
 *  member variables of the CnfFile-object.
 *  \par As relevant data is considered:
 *  \li number of molecules inf file (as given by the file itself).
 *  \li x,y,z size of the bounding box
 *  \li x,y,z-Position of each molecule
 *  \li x,y,z-Orientation of each molecule
 *
 *  \param cnffile Path to the cnf file which is to be loaded.
 *  \param reload boolean which decides wether to initially load a cnf file or reload one.
 *  \return void.
 *  \author Adrian Gabriel
 *  \date Aug 2009
 */
bool mga::CnfFile::loadCnfFile_cinacchi( string cnffile, bool reload )
{
    //cout << "CnfFile::loadCnfFile_cinacchi beg" << endl;
    bool reloadOriginal = reload;
    
    double         tmp=0;
    vector<double> tmpVec;      // Vector to read in a line
    unsigned int   count  = 0;  // Count how many molecules are read.
    string         currentLine; // current line as string value
    stringstream   strstr;
    numberOfTypes = 1;
    double box_x = 0.0;
    double box_y = 0.0;
    double box_z = 0.0;
    numberOfTypes = 1;
    
    MoleculeBiax *tmpMol = 0;
    
    ifstream in( cnffile.c_str() );
    if( in.is_open() )
    {
	// read bounding box values
	boundingBox.clear();
	boundingBox.resize(3,vector<float>(3,0.0));
	
	if( in.eof() == false )
	{
	    getline( in, currentLine );
	    strstr.clear();
	    strstr.str(currentLine);
	    int cnt = 0;
	    while( (strstr >> tmp) && (cnt < 4) )
	    {
		boundingBox.at(cnt).at(cnt) = 2*tmp;
		++cnt;
	    }
	}
	
	box_x = 0.5 * boundingBox.at(0).at(0);
	box_y = 0.5 * boundingBox.at(1).at(1);
	box_z = 0.5 * boundingBox.at(2).at(2);
	
	calculateBoundingBoxCoordinates();
	
	if( moleculeVector.size() == 0 ) { reload = false; }

	while( getline( in, currentLine ) )
	{
	    tmpVec.clear();
	    strstr.clear();
	    strstr.str(currentLine);
	    while( strstr >> tmp )
	    {
		tmpVec.push_back(tmp);
	    }
	    
		int vecSize = tmpVec.size();
	    if( vecSize == 7 )
		{
	       tmpVec.push_back(0.0);
		}
		else if( vecSize != 8 )
	    {
		cerr << "Beware! Check file format: There have to be either 7 or 8 entries per molecule!" << endl;
		//cout << "CnfFile::loadCnfFile_cinacchi end 0" << endl;
		return( false ); 
	    }
		
        if( uint(tmpVec.at(7))+1 >= numberOfTypes ) { numberOfTypes = uint(tmpVec.at(7))+1; }

	    if( reload == true )
	    {
		if( count == moleculeVector.size()-1 ) { reload = false; }
		tmpMol  = moleculeVector.at   ( count );
		tmpMol -> setPositionXYZ      ( tmpVec.at(1)*box_x, tmpVec.at(2)*box_y, tmpVec.at(3)*box_z );
		tmpMol -> setPositionFoldedXYZ( tmpVec.at(1)*box_x, tmpVec.at(2)*box_y, tmpVec.at(3)*box_z );
		tmpMol -> setOrientationXYZ   ( tmpVec.at(4)      , tmpVec.at(5)      , tmpVec.at(6) );
		tmpMol -> setType             ( int(vecSize == 7 ? 0 : tmpVec.at(7)) );
		tmpMol -> setNumber           ( count );
	    }
	    else
	    {
		moleculeVector.push_back( new MoleculeBiax( tmpVec.at(1)*box_x, tmpVec.at(2)*box_y, tmpVec.at(3)*box_z,
							    tmpVec.at(1)*box_x, tmpVec.at(2)*box_y, tmpVec.at(3)*box_z,
							    tmpVec.at(4)      , tmpVec.at(5)      , tmpVec.at(6), 
							    int(vecSize == 7 ? 0 : tmpVec.at(7)), count ) );
	    }
		++count;
	}
	in.close();
	numMolCnt = numMolFile = count; // Number of particles is not specified in the file
	
	measureBox();
	
	if( reloadOriginal == true )
	{
	    for( unsigned int i = count; i < moleculeVector.size(); ++i )
	    {
		delete moleculeVector.at(i);
		moleculeVector.at(i) = 0;
	    }
	    moleculeVector.resize( count ); 
	}
	if( count == 0 ) 
	{
	    //cout << "CnfFile::loadCnfFile_cinacchi end 1" << endl;
	    return( false ); 
	}
	if( calculateDirector() == false ) { return( false ); } // director can only be calculated after reading cnffile
	
	//cout << "CnfFile::loadCnfFile_cinacchi end 2" << endl;
	return( true );
    }
    else
    {
	cerr << "Error! Cannot open file: " << cnffile << endl;
	//cout << "CnfFile::loadCnfFile_cinacchi end 3" << endl;
	return( false );
    }
    //cout << "CnfFile::loadCnfFile_cinacchi end 4" << endl;
}

//-------------------------------------------------------------------------
//------------- reloadCnfFile
//-------------------------------------------------------------------------
/*!
 *  This function calls loadCnfFile with reload set to true.
 *  \par As relevant data is considered:
 *
 *  \param cnffile Path to the .cnf file which is to be loaded.
 *  \param reload boolean which decides wether to initially load a cnf file or reload one.
 *  \note Note: When reloading a cnf-file it should be made sure that the number of molecules does not differ!
 *  \return void.
 *  \author Adrian Gabriel
 *  \date Sept 2005
 */
bool mga::CnfFile::reloadCnfFile( string cnffile )
{
    //cout << "CnfFile::reloadCnfFile beg" << endl;
    alreadyFolded = false;
    return( (*this.*loadCnfFile[loadCnfFileIndex])( cnffile, true ) );
}

//-------------------------------------------------------------------------
//------------- setUserDefinedDirector
//-------------------------------------------------------------------------
/*!
 *  \param x
 *  \param y
 *  \param z
 *  \return void.
 *  \author Adrian Gabriel
 *  \date March 2006
 */
void  mga::CnfFile::setUserDefinedDirector( float x, float y, float z )
{
    if( userDefinedDirector.size() != 3 )
    {
	cerr << "userDefinedDirector corrupted! Code needs to be checked... exiting..." << endl;
	exit(1);
    }
    
    double norm = sqrt( pow(x,2) + pow(y,2) + pow(z,2) );
    if( norm )
    {
	x = x / norm;
	y = y / norm;
	z = z / norm;
	
	userDefinedDirector.at(0) = x;
	userDefinedDirector.at(1) = y;
	userDefinedDirector.at(2) = z;
    }
    else
    {
	cerr << "user defined director with norm = 0" << endl;
	//exit(1);
    }
}


//-------------------------------------------------------------------------
//------------- calculateDirector
//-------------------------------------------------------------------------
/*!
 *  This function calculates th nematic director of the ensemble of molecules.
 *  \return bool indicates successful operation (if true)
 *  \author Adrian Gabriel
 *  \date June 2005
 */
bool mga::CnfFile::calculateDirector()
{
    //cout << "CnfFile::calculateDirector beg" << endl;
    Array2D<double> orderTensor( 3, 3, 0.0 );
    Array2D<double> eigenVectorsInColumns( 3, 3, 0.0 );
    
    double    orientationXtmp = 0, orientationYtmp = 0, orientationZtmp = 0;
    int       numberOfMolecules = moleculeVector.size();
    
    double    factor   = 3.0 / ( 2.0 * double(numberOfMolecules) );
    double    subtract = 1.0 / 2.0;
    
    if( numberOfMolecules < 1 ) { return(false); }
    for( int i = 0; i < numberOfMolecules; ++i )                          // loop over all molecules and calculate order tensor
    {
	moleculeVector.at(i)->getOrientationXYZ( orientationXtmp, orientationYtmp, orientationZtmp );
	orderTensor[0][0] += pow( orientationXtmp, 2 );
	orderTensor[0][1] += orientationXtmp * orientationYtmp;
	orderTensor[0][2] += orientationXtmp * orientationZtmp;
	orderTensor[1][1] += pow( orientationYtmp, 2 );
	orderTensor[1][2] += orientationYtmp * orientationZtmp;
	orderTensor[2][2] += pow( orientationZtmp, 2 );
    }
    
    orderTensor[0][0] *= factor; orderTensor[0][0] -= subtract;           // multiply each tensor value with "factor" and afterwards subtract "subtract" from diagonal elements
    orderTensor[0][1] *= factor;
    orderTensor[0][2] *= factor;
    orderTensor[1][1] *= factor; orderTensor[1][1] -= subtract;
    orderTensor[1][2] *= factor;
    orderTensor[2][2] *= factor; orderTensor[2][2] -= subtract;
    
    orderTensor[1][0] = orderTensor[0][1];                                // mirror matrix to make it symmetric
    orderTensor[2][0] = orderTensor[0][2];
    orderTensor[2][1] = orderTensor[1][2];
    
    Eigenvalue<double> eig(orderTensor);
    eig.getV(eigenVectorsInColumns);
    
    //cout << "Director: ";
    director.clear();
    for( int i = 0; i < 3; ++i )
    {
	director.push_back( eigenVectorsInColumns[i][2] );//eigenVectorsInColumnsTmp[i][0] );             // after the previous sorting our eigenvector is in the first column of matrix
	//cout << left << setw(20) << director.at(i);
    }
    
    double norm = sqrt( pow(getDirectorX(),2) + pow(getDirectorY(),2) + pow(getDirectorZ(),2) );
    if( norm != 0 )
    {
	director.at(0) = getDirectorX() / norm;
	director.at(1) = getDirectorY() / norm;
	director.at(2) = getDirectorZ() / norm;
    }
    else
    {
	cerr << "Error: Director with norm = 0... setting to {0,0,1}" << endl;
	director.at(0) =  0.0;
	director.at(1) =  0.0;
	director.at(2) =  1.0;
    }
    //cout << endl << "norm:     " << left << setw(20) << director.at(0)
    //                             << left << setw(20) << director.at(1)
    //                             << left << setw(20) << director.at(2) << endl;
    
    //cout << "CnfFile::calculateDirector end" << endl;
    return(true);
}


//-------------------------------------------------------------------------
//------------- getDirector
//-------------------------------------------------------------------------
/*!
 *  
 *  \return void
 *  \author Adrian Gabriel
 *  \date Sept 2005
 */
void mga::CnfFile::getDirector( vector<double> &tmpVec )
{
    tmpVec.resize(3);
    tmpVec.at(0) = director.at(0);
    tmpVec.at(1) = director.at(1);
    tmpVec.at(2) = director.at(2);
}


//-------------------------------------------------------------------------
//------------- colorizeMolecules
//-------------------------------------------------------------------------
/*!
 *  This function sends all read molecules to read colormap to colorize them.
 *  \return void
 *  \note It is obviously crucial that the colormap has already been loaded.
 *  \note The director needs to be calculated beforehand as well.
 *  \author Adrian Gabriel
 *  \date Sept 2005
 */
void mga::CnfFile::colorizeMolecules( vector<vector<float> > *models )
{
    //cout << "CnfFile::colorizeMolecules beg" << endl;
    if( (colorMap == 0) || (director.size() != 3) )
    {
	cerr << "Error! Either no colormap loaded for colorizing or director not calculated." << endl;
	exit(1);
    }
    
    //cout << "d: " << useDirector << " u: " << useUserDefinedDirector << " m: " << useColorByModel << endl;
    //cout << "moleculeVector.size() " << moleculeVector.size() << endl;
    
    if( useDirector == true )
    {
	//cout << "useDirector" << endl;
	for( unsigned int i = 0; i < moleculeVector.size(); ++i )              // loop over all molecules
	{
	    colorMap -> setColor( moleculeVector.at(i), director, models );    // send each molecule to the colormap for colorization
	}
    }
    else if( useUserDefinedDirector == true )
    {
	//cout << "useUserDefinedDirector" << endl;
	for( unsigned int i = 0; i < moleculeVector.size(); ++i )                      // loop over all molecules
	{
	    colorMap -> setColor( moleculeVector.at(i), userDefinedDirector, models ); // send each molecule to the colormap for colorization
	}      
    }
    else if( useColorByModel == true )
    {
	//cout << "useColorByModel" << endl;
	for( unsigned int i = 0; i < moleculeVector.size(); ++i )              // loop over all molecules
	{
	    colorMap -> setColor( moleculeVector.at(i), models );              // send each molecule to the colormap for colorization
	}      
    }
    else
    {
	cerr << "Warning! No valid color scheme set. Using director..." << endl;
	setColorScheme( "director" );
	colorizeMolecules();
    }
    //cout << "CnfFile::colorizeMolecules end" << endl;
} 


//-------------------------------------------------------------------------
//------------- setColorScheme
//-------------------------------------------------------------------------
/*!
 *  
 *  \return void
 *  \note 
 *  \author Adrian Gabriel
 *  \date Jul 2007
 */
void mga::CnfFile::setColorScheme( string scheme )
{
    //cout << "CnfFile::setColorScheme beg" << endl;
    if( scheme == "director" )
    {
	useDirector = true;
	useUserDefinedDirector = useColorByModel = false;
    }
    else if( scheme == "userDefined" )
    {
	useUserDefinedDirector = true;
	useDirector = useColorByModel = false;
    }
    else if( scheme == "byModel" )
    {
	useColorByModel = true;
	useUserDefinedDirector = useDirector = false;
    }
    else
    {
	useUserDefinedDirector = useDirector = useColorByModel = false;
    }
    //cout << "CnfFile::setColorScheme end" << endl;
}


//-------------------------------------------------------------------------
//------------- checkIntegrity
//-------------------------------------------------------------------------
/*!
 *  When called this function tests if the number of molecules as
 *  given in the .cnf file is the same as the number counted.
 *  \return true when numbers equal false otherwise.
 *  \author Adrian Gabriel
 *  \date June 2005
 */
bool mga::CnfFile::checkIntegrity() const
{
    if( numMolFile !=  numMolCnt || numMolCnt == 0 ) 
    {
	cerr << "Number of molecules as given in file: " << numMolFile << " , actually found molecules: " << numMolCnt << endl; 
	return(false); 
    }
    else
    {
	return(true);
    }    
}

//-------------------------------------------------------------------------
//------------- loadColorMap
//-------------------------------------------------------------------------
/*!
 *  This Function loads a given colormap file and assigns a pointer to the
 *  appropriate class member.
 *  \param colorfile Path to the colormap file.
 *  \return void.
 *  \author Adrian Gabriel
 *  \date June 2005
 */
void mga::CnfFile::loadColorMap( string colorfile )
{
    colorMap = new Colormap( colorfile );
    if( !colorMap ) { exit(1); }
}

//-------------------------------------------------------------------------
//------------- printForVRML
//-------------------------------------------------------------------------
/*!
 *  Writes out a file for convenient parsing named vrml.conf.
 *  \param "vrml.conf" Hardcoded filename.
 *  \return void.
 *  \author Adrian Gabriel
 *  \date June 2005
 */
void mga::CnfFile::printForVRML( string vrmlfile )
{
    ofstream file( vrmlfile.c_str() );
    stringstream strstr;
    if( file.is_open() )
    {
	for( unsigned int i = 0; i < moleculeVector.size(); ++i)
	{
	    moleculeVector.at(i)->print( strstr );
	    file << strstr.str();
	    strstr.str("");
	}
	file.close();
    }
}



//--------------------------------------------
//------------ purge
//--------------------------------------------

//-------------------------------------------------------------------------
//------------- purge
//-------------------------------------------------------------------------
/*!
 *  From "Thinking in C++, 2nd Edition, Volume 2"
 *  by Bruce Eckel & Chuck Allison, (c) 2001 MindView, Inc.
 *  Available at www.BruceEckel.com.
 *  Delete pointers in an STL sequence container
 *  \param c Some stl container (stl::list, stl::vector, etc.)
 *  \return void
 *  \author Adrian Gabriel
 *  \date June 2005
 */
template<class Seq> void mga::purge( Seq& c )
{
    typename Seq::iterator i;
    for( i = c.begin(); i != c.end(); ++i )
    {  
	delete *i;
	*i = 0;
    }
}

//-------------------------------------------------------------------------
//------------- purge
//-------------------------------------------------------------------------
/*!
 *  From "Thinking in C++, 2nd Edition, Volume 2"
 *  by Bruce Eckel & Chuck Allison, (c) 2001 MindView, Inc.
 *  Available at www.BruceEckel.com.
 *  Delete pointers in an STL sequence container
 *  \note Overloaded version for use with iterators.
 *  \param begin Iterator pointing to begin of container.
 *  \param end Iterator pointing to end of container.
 *  \return void
 *  \author Adrian Gabriel
 *  \date June 2005
 */
template<class InpIt> void mga::purge( InpIt begin, InpIt end )
{
    if( end < begin )
    {
	cerr << "End-Iterator smaller than Start." << endl;
	exit(1);
    }
    while( begin != end )
    {
	delete *begin;
	*begin = 0;
	++begin;
    }
}


//-------------------------------------------------------------------------
//------------- Point3D operators
//-------------------------------------------------------------------------
inline Point3D &mga::Point3D::operator+=( const Point3D &p )
{
    setX( getX() + p.getX() );
    setY( getY() + p.getY() );
    return( *this ); 
}

inline Point3D &mga::Point3D::operator-=( const Point3D &p )
{
    setX( getX() - p.getX() );
    setY( getY() - p.getY() );
    return( *this ); 
}

inline Point3D &mga::Point3D::operator*=( int s )
{
    setX( getX() * s );
    setY( getY() * s );
    return( *this ); 
}

inline Point3D &mga::Point3D::operator*=( double s )
{
    setX( getX() * s );
    setY( getY() * s );
    return( *this ); 
}

inline Point3D &mga::Point3D::operator/=( int s )
{
    setX( getX() / double(s) );
    setY( getY() / double(s) );
    return( *this ); 
}

inline Point3D &mga::Point3D::operator/=( double s )
{
    setX( getX() / s );
    setY( getY() / s );
    return( *this ); 
}

inline const Point3D mga::operator+( const Point3D &lhs, const Point3D &rhs )
{
    return( Point3D( lhs.getX()+rhs.getX(), lhs.getY()+rhs.getY(), lhs.getZ()+rhs.getZ() ) );
}

inline const Point3D mga::operator-( const Point3D &lhs, const Point3D &rhs )
{
    return( Point3D( lhs.getX()-rhs.getX(), lhs.getY()-rhs.getY(), lhs.getZ()-rhs.getZ() ) );
}

inline const Point3D mga::operator*( const double &s, const Point3D &p )
{
    return( Point3D( p.getX()*s, p.getY()*s, p.getZ()*s ) );
}

inline const Point3D mga::operator*( const Point3D &p, const double &s )
{
    return( Point3D( p.getX()*s, p.getY()*s, p.getZ()*s ) );
}

inline const Point3D mga::operator*( const int &s, const Point3D &p )
{
    return( Point3D( p.getX()*s, p.getY()*s, p.getZ()*s ) );
}

inline const Point3D mga::operator*( const Point3D &p, const int &s )
{
    return( Point3D( p.getX()*s, p.getY()*s, p.getZ()*s ) );
}

inline const Point3D mga::operator/( const double &s, const Point3D &p )
{
    return( Point3D( p.getX()/s, p.getY()/s, p.getZ()/s ) );
}

inline const Point3D mga::operator/( const Point3D &p, const double &s )
{
    return( Point3D( p.getX()/s, p.getY()/s, p.getZ()/s ) );
}

inline const Point3D mga::operator/( const int &s, const Point3D &p )
{
    return( Point3D( p.getX()/double(s), p.getY()/double(s), p.getZ()/double(s) ) );
}

inline const Point3D mga::operator/( const Point3D &p, const int &s )
{
    return( Point3D( p.getX()/double(s), p.getY()/double(s), p.getZ()/double(s) ) );
}
