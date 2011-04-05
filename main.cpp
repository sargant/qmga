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

#include <qapplication.h>
#include <GL/glut.h>
#include "mainform.h"
#include <iostream>

using std::cerr;
using std::endl;

void showHelp();
void openApp( int argc, char * argv[], QString format = "", QString cnfFile = "", QString colorMap = "color-090.map", string modelsFile = "", string  = "", int=0, int = 0, int = 1, int = 1 );

//-------------------------------------------------------------------------
//------------- main
//-------------------------------------------------------------------------
int main( int argc, char * argv[] )
{
    glutInit(&argc,argv);
    
    QString cnfFile   = "";
    QString colorMap  = "color-090.map";
    QString format    = "";
    string modelsFile = "";
    string videoFile = "";

    int videoStartValue = 0;
    int videoStopValue = 0;
    int videoStepValue = 0;
    int videoDigitsValue = 0;

    if     ( argc == 1 )
    {
	//showHelp();
	openApp( argc, argv );
    }
    else if( argc%2 != 0 )
    {
	for( int i = 1; i<argc; i+=2 )
	{
	    if     ( QString(argv[i]) == QString("-f") ) { format   = QString( argv[i+1] ); }
	    else if( QString(argv[i]) == QString("-i") ) { cnfFile  = QString( argv[i+1] ); }
	    else if( QString(argv[i]) == QString("-c") ) { colorMap = QString( argv[i+1] ); }
	    else if( QString(argv[i]) == QString("-m") ) { modelsFile = string( argv[i+1] ); }
	    else if( QString(argv[i]) == QString("-v") )
	    {
		videoFile            = string( argv[i+1] );
		videoStartValue  = atoi( argv[i+2] );
		videoStopValue  = atoi( argv[i+3] );
		videoStepValue  = atoi( argv[i+4] );
		videoDigitsValue = atoi( argv[i+5] );
	    }
	}
	openApp( argc, argv, format, cnfFile, colorMap, modelsFile, videoFile, videoStartValue, videoStopValue, videoStepValue, videoDigitsValue );
    }
    else
    {
	showHelp();
    }
}

//-------------------------------------------------------------------------
//------------- showHelp
//-------------------------------------------------------------------------
void showHelp()
{
    cerr << "Usage:" << endl;
    cerr << "\t./qmga [-f FILEFORMAT] [-i INPUTFILE] [-c COLORMAP] [-m MODELDEF] [-v VIDEOOPTIONS]" << endl;
    cerr << "eg:" << endl;
    cerr << "\t./qmga -f gbmega -i mga_dummy.cnf -c color-090.map -m modelsFile -v video parameters" << endl;
    cerr << "(NOTE: in modelsFile each model on a separate line with these attributes:\n [x] [y] [z] 0 0 [wireframe] [force model color] [r] [g] [b])" << endl;
    cerr << "(NOTE: video parameters are: videoStartFile, videoStartValue, int videoStopValue, int videoStepValue, int videoDigitsValue)" <<endl;
}


//-------------------------------------------------------------------------
//------------- openApp
//-------------------------------------------------------------------------
void openApp(  int argc, char * argv[], QString format, QString cnfFile, QString colorMap, 
	       string modelsFile, 
	       string videoFile, int videoStartVal, int videoStopVal, int videoStepVal, int videoDigitsVal )
{  
    //cout << "main::openApp beg" << endl;
    QApplication app(argc,argv);
    uint fileFormat = -1;
    
    MainForm w;
    if( cnfFile.isEmpty() )
    {
	w.setOnlyColormap( colorMap );
    }
    else
    {
	w.setInputFiles ( cnfFile, colorMap );
    }
    app.setMainWidget( &w );
    
    if     ( format == "gbmega" )     { fileFormat = 0; }
    else if( format == "lammps1" )    { fileFormat = 1; }
    else if( format == "lammps2" )    { fileFormat = 2; }
    else if( format == "gbmegaBiax" ) { fileFormat = 3; }
    else if( format == "cinacchi" )   { fileFormat = 4; }
    //else if( format == "foo-format" ) { fileFormat = 5; }
    // ...
    
    //cout << "main format:         >" << format << "<: " << fileFormat << endl;
    //cout << "main modelsFile:     >" << modelsFile<< "<" << endl;
    //cout << "main videoStartFile: >" << videoStartFile << "<" << endl;
    
    if( fileFormat != uint(-1) )
    {
	w.setFormat( fileFormat );
    }
    
    if( !modelsFile.empty() )
    {
	w.setModelsFile( modelsFile );      
	w.loadModels();
    }
    
    if( !videoFile.empty() )
    {
	w.setVideoParameters( videoFile, videoStartVal, videoStopVal, videoStepVal, videoDigitsVal );      
    }
    
    w.show();
    app.exec();
    //cout << "main::openApp end" << endl;
}
