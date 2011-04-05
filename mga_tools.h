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

#ifndef MGA_TOOLS_H
#define MGA_TOOLS_H

#include <iostream>
#include <vector>
#include <algorithm>

#include "tnt/jama_eig.h"

using std::cout;
using std::cin;
using std::endl;
using std::string;
using std::vector;
using std::stringstream;

//! The namespace for mg by adrian gabriel.
/*!
 *  mga contains all classes and functions that were created to work
 *  out a c++ version of mg.
 *  \author Adrian Gabriel
 *  \date June 2005
 */
namespace mga
{
  //-------------------------------------------------------------------------
  //------------- Molecule
  //-------------------------------------------------------------------------
  //! A class to describe a molecule in terms of form, positioning and orientation.
  /*!
   *  This class was designed to read, calculate and contain all data neccessary to draw
   *  molecules found in .cnf files.
   *  \par Relevant data for drawing:
   *  \li Molecule::type
   *  \li Molecule::height
   *  \li Molecule::width
   *  \li Molecule::depth
   *  \li Molecule::red
   *  \li Molecule::green
   *  \li Molecule::blue
   *  \li Molecule::positionX
   *  \li Molecule::positionY
   *  \li Molecule::positionZ
   *  \li Molecule::orientationX
   *  \li Molecule::orientationY
   *  \li Molecule::orientationZ
   *
   *  \author Adrian Gabriel
   *  \date June 2005
   */
  class Molecule
  {
  public:
    Molecule( double positionXtmp      , double positionYtmp      , double positionZtmp      ,
	      double positionFoldedXtmp, double positionFoldedYtmp, double positionFoldedZtmp,
	      double orientationXtmp   , double orientationYtmp   , double orientationZtmp   ,
	      int    type      = 0     , unsigned int number = 0  ,
	      double heightTmp = 0.0   , double widthTmp = 0.0    , double depthTmp = 0.0    ,
	      double red       = 0.0   , double green    = 0.0    , double blue     = 0.0      ); //!< The constructor.
    virtual ~Molecule();                                                                        //!< The destructor.
    int    getType() const { return(type); }                                            //!< Returns integer that represents type of molecule (e.g. ellipsoid, spherical...).
    unsigned int getNumber() const { return(number); }                                  //!< Returns number of molecule
    double getHeight() const { return(height); }                                        //!< Returns height of the molecole.
    double getWidth () const { return(width);  }                                        //!< Returns width of the molecule.
    double getDepth () const { return(depth);  }                                        //!< Returns depth of the molecule.
    double getRed  () const { return(red);   }                                          //!< Returns red value.
    double getGreen() const { return(green); }                                          //!< Returns green value.
    double getBlue () const { return(blue);  }                                          //!< Returns blue value.
    void   getRGB  ( double& redTmp, double& greenTmp, double& blueTmp ) const;         //!< Sets three references to red, green and blue value.
    double getOrientationX  () const { return(orientationX); }                          //!< Returns x component of orientation vector.
    double getOrientationY  () const { return(orientationY); }                          //!< Returns y component of orientation vector.
    double getOrientationZ  () const { return(orientationZ); }                          //!< Returns z component of orientation vector.
    void   getOrientationXYZ( double& xTmp, double& yTmp, double& zTmp ) const;         //!< Sets three references to x,y,z position vector values.
    double getPositionX  () const { return(positionX); }                                //!< Returns x component of position.
    double getPositionY  () const { return(positionY); }                                //!< Returns y component of position.
    double getPositionZ  () const { return(positionZ); }                                //!< Returns z component of position.
    void   getPositionXYZ( double& xTmp, double& yTmp, double& zTmp ) const;            //!< Sets three references to x,y,z position values.    
    double getPositionFoldedX  () const { return(positionFoldedX); }                    //!< Returns x component of position after folding to bounding Box.
    double getPositionFoldedY  () const { return(positionFoldedY); }                    //!< Returns y component of position after folding to bounding Box.
    double getPositionFoldedZ  () const { return(positionFoldedZ); }                    //!< Returns z component of position after folding to bounding Box.
    void   getPositionFoldedXYZ( double& xTmp, double& yTmp, double& zTmp ) const;      //!< Sets three references to x,y,z position values.
    void   setType( int typeTmp ) { type   = typeTmp; }                                 //!< Sets type of molecule.
    void   setNumber( unsigned int numberTmp ) { number = numberTmp; }                  //!< Sets number of molecule.
    void   setHeight( double heightTmp ) { height = heightTmp; }                        //!< Sets height of molecule.
    void   setWidth ( double widthTmp  ) { width  = widthTmp;  }                        //!< Sets width of molecule.
    void   setDepth ( double depthTmp  ) { depth  = depthTmp;  }                        //!< Sets depth of molecule.
    void   setRed  ( double redTmp   ) { red   = redTmp;   }                            //!< Sets red value.
    void   setGreen( double greenTmp ) { green = greenTmp; }                            //!< Sets green value.
    void   setBlue ( double blueTmp  ) { blue  = blueTmp;  }                            //!< Sets blue value.
    void   setRGB  ( double redTmp, double  greenTmp, double  blueTmp );                //!< Sets all rgb values at once.
    void   setOrientationXYZ( double xTmp, double  yTmp, double  zTmp );                //!< Sets all orientation-vector components at once.
    void   setPositionX  ( double positionXtmp ){ positionX = positionXtmp; }           //!< Sets x component of position.
    void   setPositionY  ( double positionYtmp ){ positionY = positionYtmp; }           //!< Sets y component of position.
    void   setPositionZ  ( double positionZtmp ){ positionZ = positionZtmp; }           //!< Sets z component of position.
    void   setPositionXYZ( double xTmp, double  yTmp, double  zTmp );                   //!< Sets all position-values at once.
    void   setPositionFoldedX  ( double positionFoldedXtmp ){ positionFoldedX = positionFoldedXtmp; } //!< Sets x component of position after folding to bounding Box.
    void   setPositionFoldedY  ( double positionFoldedYtmp ){ positionFoldedY = positionFoldedYtmp; } //!< Sets y component of position after folding to bounding Box.
    void   setPositionFoldedZ  ( double positionFoldedZtmp ){ positionFoldedZ = positionFoldedZtmp; } //!< Sets z component of position after folding to bounding Box.
    void   setPositionFoldedXYZ( double xTmp, double  yTmp, double  zTmp );             //!< Sets all position-values at once.
    void   print() const;                                                               //!< Writes data to std-out.
    void   print( stringstream &stream ) const;                                         //!< Overloaded version writing output to a sringstream passed by reference.
    void   setColorIndex( int idx ) { colorIndex = idx;     }                           //!< Sets colorIndex of molecule.
    int    getColorIndex() const    { return( colorIndex ); }                           //!< Gets colorIndex of molecule.
  protected:
    void normalizeOrientationVector();                                                  //!< Normalizes orientation vector to 1.
    void initMembers( double positionXtmp       = 0.0, double positionYtmp       = 0.0, double positionZtmp       = 0.0,
		      double positionFoldedXtmp = 0.0, double positionFoldedYtmp = 0.0, double positionFoldedZtmp = 0.0,
		      double orientationXtmp    = 0.0, double orientationYtmp    = 0.0, double orientationZtmp    = 0.0,
		      int    typeTmp            = 0  , unsigned int number       = 0  ,                                                  
		      double heightTmp          = 0.0, double widthTmp           = 0.0, double depthTmp           = 0.0,
		      double redTmp             = 0.0, double greenTmp           = 0.0, double blueTmp            = 0.0  ); //!< Function to initialize all members of this class.
    int    type;                                                                        //!< Member that defines the type of the molecule (e.g. ellipsoid, spherical, etc.).
    unsigned int number;                                                                //!< number of molecule
    double height;                                                                      //!< Defines the height of the molecule.
    double width;                                                                       //!< Defines the width of the molecule.
    double depth;                                                                       //!< Defines the depth of the molecule.
    double red;                                                                         //!< RGB value for red.
    double green;                                                                       //!< RGB value for green.
    double blue;                                                                        //!< RGB value for blue.
    int    colorIndex;                                                                  //!< Line number of the colormap for the used color.
    double positionX;                                                                   //!< X component of the position of the centre of mass.
    double positionY;                                                                   //!< Y component of the position of the centre of mass.
    double positionZ;                                                                   //!< Z component of the position of the centre of mass.
    double positionFoldedX;                                                             //!< X component of the position of the centre of mass after folding to bounding box.
    double positionFoldedY;                                                             //!< Y component of the position of the centre of mass after folding to bounding box.
    double positionFoldedZ;                                                             //!< Z component of the position of the centre of mass after folding to bounding box.
    double orientationX;                                                                //!< X component of the orientation vector.
    double orientationY;                                                                //!< Y component of the orientation vector.
    double orientationZ;                                                                //!< Z component of the orientation vector.
  };
}

namespace mga
{
  //-------------------------------------------------------------------------
  //------------- MoleculeBiax
  //-------------------------------------------------------------------------
  //! A class to describe a molecule in terms of form, positioning and orientation.
  /*!
   *  This class was designed to read, calculate and contain all data neccessary to draw
   *  molecules found in .cnf files.
   *  \par Relevant data for drawing:
   *  \li Molecule::type
   *  \li Molecule::height
   *  \li Molecule::width
   *  \li Molecule::depth
   *  \li Molecule::red
   *  \li Molecule::green
   *  \li Molecule::blue
   *  \li Molecule::positionX
   *  \li Molecule::positionY
   *  \li Molecule::positionZ
   *  \li Molecule::orientationW
   *  \li Molecule::orientationX
   *  \li Molecule::orientationY
   *  \li Molecule::orientationZ
   *
   *  \author Matteo Ricci
   *  \date Mar 2008
   */
class MoleculeBiax : public Molecule
  {
  public:
    MoleculeBiax( double positionXtmp      , double positionYtmp      , double positionZtmp      ,
		  double positionFoldedXtmp, double positionFoldedYtmp, double positionFoldedZtmp,
		  double orientationWtmp   , double orientationXtmp   , double orientationYtmp   , double orientationZtmp   ,
		  int    type      = 0     , unsigned int number = 0  ,
		  double heightTmp = 0.0   , double widthTmp = 0.0    , double depthTmp = 0.0    ,
		  double red       = 0.0   , double green    = 0.0    , double blue     = 0.0 );    //!< The constructor (Quaternion).
    MoleculeBiax( double positionXtmp      , double positionYtmp      , double positionZtmp      ,
		  double positionFoldedXtmp, double positionFoldedYtmp, double positionFoldedZtmp,
		  double orientationXtmp   , double orientationYtmp   , double orientationZtmp   ,
		  int    type      = 0     , unsigned int number = 0  ,
		  double heightTmp = 0.0   , double widthTmp = 0.0    , double depthTmp = 0.0    ,
		  double red       = 0.0   , double green    = 0.0    , double blue     = 0.0 );    //!< The constructor (Vector).
    virtual ~MoleculeBiax();                                                                        //!< The destructor.

    double getOrientationW  () const { return(orientationW);  }                          //!< Returns x component of orientation vector.
    double getOrientationXQ () const { return(orientationXQ); }                          //!< Returns x component of orientation vector.
    double getOrientationYQ () const { return(orientationYQ); }                          //!< Returns y component of orientation vector.
    double getOrientationZQ () const { return(orientationZQ); }                          //!< Returns z component of orientation vector.
    void   getOrientationWXYZ( double& wTmp, double& xTmp, double& yTmp, double& zTmp ) const; //!< Sets three references to x,y,z position vector values.
    void   setOrientationWXYZ( double wTmp, double xTmp, double  yTmp, double  zTmp );         //!< Sets all orientation-quaternion components at once.
    void   print() const;                                                               //!< Writes data to std-out.
    void   print( stringstream &stream ) const;                                         //!< Overloaded version writing output to a sringstream passed by reference.
    void   setColorIndex( int idx ) { colorIndex = idx;     }                           //!< Sets colorIndex of molecule.
    int    getColorIndex() const    { return( colorIndex ); }                           //!< Gets colorIndex of molecule.

    std::vector<double> MakeQuat(double angle, double x, double y, double z) const;

    std::vector<double> QuatToVector(double w, double q1, double q2, double q3) const;
    std::vector<double> QuaternionToAxisAngle();
    static std::vector<double> QuaternionFromAxisAngle(double gamma, double axisX, double axisY, double axisZ);
    void NormalizeQuaternion();
    static double const RadToDeg = 57.29577951308232087679;
    void MultiplyQ(double q1, double q2, double q3, double q4);

    void generateQuaternionForUniaxialParticles();
    void setOrientationXYZ(double, double, double);
  private:

    //void normalizeOrientationVector();                                                  //!< Normalizes orientation vector to 1.
    void initMembers( double positionXtmp       = 0.0, double positionYtmp       = 0.0, double positionZtmp       = 0.0,
		      double positionFoldedXtmp = 0.0, double positionFoldedYtmp = 0.0, double positionFoldedZtmp = 0.0,
		      double orientationWtmp    = 0.0, double orientationXtmp    = 0.0, double orientationYtmp    = 0.0, double orientationZtmp    = 0.0,
		      int    typeTmp            = 0  , unsigned int number       = 0  ,                                                  
		      double heightTmp          = 0.0, double widthTmp           = 0.0, double depthTmp           = 0.0,
		      double redTmp             = 0.0, double greenTmp           = 0.0, double blueTmp            = 0.0  ); //!< Function to initialize all members of this class.
    double orientationW;                                                                //!< W component of the orientation quaternion.
    double orientationXQ;                                                               //!< X component of the orientation quaternion.
    double orientationYQ;                                                               //!< Y component of the orientation quaternion.
    double orientationZQ;                                                               //!< Z component of the orientation quaternion.
  };
}

namespace mga
{
  //-------------------------------------------------------------------------
  //------------- Colormap
  //-------------------------------------------------------------------------
  //! A class to contain a colormap file (R,G,B values)
  /*! The Colormap-class is designed to read and contain the contents of
   *  a colormap file.
   *  \par
   *  The (text-)file must have one to several lines of double values
   *  (3 values each line) ranging from 0 to 255.
   *  \par
   *  The double values are interpreted as R, G, B (in this order).
   *  \author Adrian Gabriel
   *  \date June 2005
   */
  class Colormap
  {
  public:
    Colormap( string colorFile );                                           //!< The constructor.
    ~Colormap();                                                            //!< The destructor.
    double    getRed  (int number) const { return redVector.at(number);   } //!< Returns red-value (0-255) from linenumber "number".
    double    getGreen(int number) const { return greenVector.at(number); } //!< Returns green-value (0-255) from linenumber "number".
    double    getBlue (int number) const { return blueVector.at(number);  } //!< Returns blue-value (0-255) from linenumber "number".
    void      print() const;                                                //!< Prints colormap to std-out.
    Molecule* setColor( Molecule* moleculeTmp,
			vector<double> &director, 
			vector<vector<float> > *models ) const;             //!< Takes pointer to a Molecule-object and sets the color of it.
    Molecule* setColor( Molecule* moleculeTmp,
			vector<vector<float> > *models ) const;             //!< Takes pointer to a Molecule-object and sets the color of it.
    int       getNumberOfColors()  const { return( numberOfLinesInFile ); } //!< Returns the number of color entries.
    
  private:
    vector<double> redVector;                                               //!< Vector contains all red-values of the colormap in order of the lines in file.
    vector<double> greenVector;                                             //!< Vector contains all green-values of the colormap in order of the lines in file.
    vector<double> blueVector;                                              //!< Vector contains all blue-values of the colormap in order of the lines in file.
    void loadColormap( string colorFile );                                  //!< Loads a given colormap file.
    int numberOfLinesInFile;                                                //!< Stores the number of lines of the colormap file.
  };
}


namespace mga
{
  //-------------------------------------------------------------------------
  //------------- CnfFile
  //-------------------------------------------------------------------------
  //! A class to contain a .cnf file.
  /*!
   *  This class is designed to read and contain a configuration file (cnf)
   *  that gives a certain set of molecules with corresponding
   *  location and orientation for each molecule. Such files are the output of
   *  certain molecular computer simulations.
   *  \author Adrian Gabriel
   *  \date June 2005
   */
  class CnfFile
  {
  public:
    CnfFile( string &cnffile, uint loadIndex, string colorscheme = "director", string colormap = "color-091.map" ); //!< The constructor.
    ~CnfFile();                                                                           //!< The destructor.
    int       getNumberOfMolecules() const { return(numMolCnt); }                         //!< Returns the number of Molecule-objects that are stored.
    double    getBoxX() const { return(boxX); }                                           //!< Returns the x-size of the bounding box.
    double    getBoxY() const { return(boxY); }                                           //!< Returns the y-size of the bounding box.
    double    getBoxZ() const { return(boxZ); }                                           //!< Returns the z-size of the bounding box.
    //Molecule* getMolecule( int number ) const { return( moleculeVector.at(number) ); }    //!< Returns the pointer to Molecule number "number"
    MoleculeBiax* getMolecule( int number ) const { return( moleculeVector.at(number) ); }    //!< Returns the pointer to Molecule number "number"
    void      printForVRML( string vrmlfile = "vrml.conf" );                              //!< Prints all info about molecules to a file.
    void      foldMoleculesToBoundingBox();                                               //!< Folds molecules position vector into bounding box.
    bool      getShowFolded() const { return(showFolded); }
    void      setShowFolded( bool fold ) { showFolded = fold; }
    bool      reloadCnfFile( string cnffile );                                            //!< Loads new cnffile. No need to "delete" current one.
    int       getNumberOfColorsInMap() const { return( colorMap->getNumberOfColors() ); } //!< Returns the number of colors in the current colormap.
    float     getRedAt  ( int pos ) const { return( colorMap -> getRed  (pos) ); }        //!< Returns pos'th redvalue of current colormap.
    float     getGreenAt( int pos ) const { return( colorMap -> getGreen(pos) ); }        //!< Returns pos'th greenvalue of current colormap.
    float     getBlueAt ( int pos ) const { return( colorMap -> getBlue (pos) ); }        //!< Returns pos'th bluevalue of current colormap.
    vector<vector<float> > getBoundingBoxCoordinates() { return(boundingBoxCoordinates); }
    vector<vector<float> > getBoundingBoxMatrix() { return(boundingBox); }
    void      setUserDefinedDirector( float x = 0.0, float y = 0.0, float z = 1.0 );      //!< Sets x,y,z value of the user defined director (used for molecule colorisation,default z-axis)
    void setColorScheme( string scheme );
    bool setBoundingBoxCoordinates( vector<vector<float> > v )
    {
	if( v.size() != 24 ) { return( false ); }
	for( int i = 0; i<24; i++ ) 
	{
	    if( v.at(i).size() != 3 ) { return( false ); }
	}
	
	boundingBoxCoordinates = v;
    }
    
    void setLoadCnfFileIndex( uint b ) { loadCnfFileIndex = b; }
    uint getLoadCnfFileIndex()         { return( loadCnfFileIndex ); }
    
    void      getDirector( vector<double> &tmpVec );
    double    getDirectorX() { return director.at(0); }
    double    getDirectorY() { return director.at(1); }
    double    getDirectorZ() { return director.at(2); }
    uint      getNumberOfTypes() { return numberOfTypes; }                                //!< Gets the number of different molecule types needed for this configuration.
    void      colorizeMolecules( vector<vector<float> > *models = 0 );                    //!< Sets color values of molecules based on calculated director.
    void      calculateBoundingBoxCoordinates();
    void      measureBox();
private:
    //vector<Molecule*> moleculeVector;                                                  //!< Stl-vector to store all molecules from file as Molecule-objects.
    vector<MoleculeBiax*> moleculeVector;                                                  //!< Stl-vector to store all molecules from file as Molecule-objects.
    void initMembers   ( string    colorscheme = ""  , int    numMolFile = 0  , int    numMolCnt = 0  ,                     
			 double    boxX        = 0.0 , double boxY       = 0.0, double boxZ = 0.0,
			 Colormap* colorMap    = 0                                                ); //!< Function to initialize all members of this class.
    bool loadCnfFile_gbmega( string cnffile, bool reload = false );                    //!< Opens given .cnf file and reads its contents.
    bool loadCnfFile_lammps1( string cnffile, bool reload = false );                   //!< Opens given .cnf file and reads its contents.
    bool loadCnfFile_lammps2( string cnffile, bool reload = false );                   //!< Opens given .cnf file and reads its contents.
    bool loadCnfFile_gbmegaBiax( string cnffile, bool reload = false );                //!< Opens given .cnf file and reads its contents.
    bool loadCnfFile_cinacchi( string cnffile, bool reload = false );                  //!< Opens given .cnf file and reads its contents.
   //bool loadCnfFile_foo-format( string cnffile, bool reload = false );                //!< Opens given .cnf file and reads its contents.
    void loadColorMap( string colorfile );                                             //!< Opens given colormap file and reads its contents.
    bool calculateDirector();                                                          //!< Calculates nematic director of the ensemble of Molecules.
    bool checkIntegrity() const;                                                       //!< Simple test to check file integrity of the .cnf file.
    bool createTmpDummy();
    int       numMolFile;                                                              //!< Number of molecules in .cnf file as provided by file itself.
    int       numMolCnt;                                                               //!< Number of molecules in .cnf file as counted while loading file.
    double    boxX;                                                                    //!< x-size of the bounding box as read out of .cnf file.
    double    boxY;                                                                    //!< y-size of the bounding box as read out of .cnf file.
    double    boxZ;                                                                    //!< z-size of the bounding box as read out of .cnf file.
    vector<vector<float> > boundingBox;
    vector<vector<float> > boundingBoxCoordinates;
    Colormap* colorMap;                                                                //!< Pointer to a Colormap-object.
    vector<double> director;                                                           //!< Vector containing eigenvector that belongs to biggest eigenvalue from eigenValuesVector.
    vector<double> userDefinedDirector;                                                //!< Vector containing user defined values to use for molecule color coding.
    bool useDirector;
    bool useUserDefinedDirector;                                                       //!< Boolean to decide, with which vector to colorize the molecules
    bool useColorByModel;
    bool showFolded;    
    bool alreadyFolded;
    uint colorScheme;    
    uint numberOfTypes;                                                                //!< The number of different molecule types found in curent file
    
    // "foo-format" set the correct number of loader functions here!
    bool (mga::CnfFile::*loadCnfFile[5])( string, bool );
    uint loadCnfFileIndex;
    
};
}

namespace mga
{
    //-------------------------------------------------------------------------
    //------------- purge
    //-------------------------------------------------------------------------
    template<class Seq>   void purge( Seq& c                 ); //! Handy to clean up stl containers filled with pointers to objects in the heap.
    template<class InpIt> void purge( InpIt begin, InpIt end ); //! Handy to clean up stl containers filled with pointers to objects in the heap.
    
    //-------------------------------------------------------------------------
    //------------- class Point
    //-------------------------------------------------------------------------
    class Point3D
    {
    public:
	Point3D() { coordinates.resize(3,0.0); }
	Point3D( double x, double y, double z )
	{
	    coordinates.resize(3,0.0);
	    setX(x);
	    setY(y);
	    setZ(z);
	}
	~Point3D() {}
	
	float        getX()            const { return coordinates.at(0); }
	float        getY()            const { return coordinates.at(1); }
	float        getZ()            const { return coordinates.at(2); }
	vector<float> getCoordinates() const { return( coordinates ); }
	
	void setX( float x )                   { coordinates.at(0) = x; }
	void setY( float y )                   { coordinates.at(1) = y; }
	void setZ( float z )                   { coordinates.at(2) = z; }
	bool setCoordinates( vector<float> v ) 
	{
	    if( v.size() != 3 ) { return( false ); }
	    coordinates = v;
	    return( true );
	}
	
	Point3D &operator+=( const Point3D &p );
	Point3D &operator-=( const Point3D &p );
	Point3D &operator*=( int s );
	Point3D &operator*=( double s );
 	Point3D &operator/=( int s );
	Point3D &operator/=( double s );
   
	friend inline const Point3D operator+( const Point3D &, const Point3D & );
	friend inline const Point3D operator-( const Point3D &, const Point3D & );
	friend inline const Point3D operator*( const double &, const Point3D & );
	friend inline const Point3D operator*( const Point3D &, const double & );
	friend inline const Point3D operator*( const int &, const Point3D & );
	friend inline const Point3D operator*( const Point3D &, const int & );
	friend inline const Point3D operator/( const double &, const Point3D & );
	friend inline const Point3D operator/( const Point3D &, const double & );
	friend inline const Point3D operator/( const int &, const Point3D & );
	friend inline const Point3D operator/( const Point3D &, const int & );
	
    private:
	vector<float> coordinates;
    };
    
    inline const Point3D operator+( const Point3D &lhs, const Point3D &rhs );
    inline const Point3D operator-( const Point3D &lhs, const Point3D &rhs );
    inline const Point3D operator*( const double &s, const Point3D &p );
    inline const Point3D operator*( const Point3D &p, const double &s );
    inline const Point3D operator*( const int &s, const Point3D &p );
    inline const Point3D operator*( const Point3D &p, const int &s );
    inline const Point3D operator/( const double &s, const Point3D &p );
    inline const Point3D operator/( const Point3D &p, const double &s );
    inline const Point3D operator/( const int &s, const Point3D &p );
    inline const Point3D operator/( const Point3D &p, const int &s );
}



#endif //MGA_TOOLS_H
