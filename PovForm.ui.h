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
void PovrayForm::init()
{
    connect( pushButton_save, SIGNAL(clicked()), this, SLOT(save()) );
    connect( pushButton_generate, SIGNAL(clicked()), this, SLOT(generate()) );
    
    textEdit_povray->setText("- Click 'generate' to generate a povray-script.\n- The script might be edited for customization before saving.");
    
    radioButton_template_01->setChecked(true);
	    
    cnf = 0;
}

//-------------------------------------------------------------------------
//------------- save
//-------------------------------------------------------------------------
void PovrayForm::save()
{
    static QString newFile = "";
    newFile = QFileDialog::getSaveFileName( newFile=="" ? "./" : newFile, "Povray Files (*.pov);;All Files (*)", this, "save file dialog", "Choose File" );
   
    QFile file( newFile );
    if ( file.open( IO_WriteOnly ) )
    {
	QTextStream stream( &file );
	stream << textEdit_povray->text() << "\n";
	file.close();
    }
}

//-------------------------------------------------------------------------
//------------- generate
//-------------------------------------------------------------------------
void PovrayForm::generate()
{
    QString templateFileName = "";
    
    switch( buttonGroup_template -> selectedId() )
    {
    case 0: templateFileName = "povray.template.01";
	break;
    case 1: templateFileName = "povray.template.02";
	break;
    case 2: templateFileName = "povray.template.03";
	break;
    default: templateFileName = "povray.template.01";
    }
    
    
    QFile file( templateFileName );
    if ( file.open( IO_ReadOnly ) ) 
    {
	vector<float> data;
	vector<vector<float> > boxCoords;
	vector<vector<float> > boxMatrix;
	
	glWindow->getData( data );
	
	boxCoords = cnf->getBoundingBoxCoordinates();
	boxMatrix = cnf->getBoundingBoxMatrix();
	
	float boxLengthA = 0.0, boxLengthB = 0.0, boxLengthC = 0.0, boxLengthMax = 0.0, boxRadius = 1.0;
	
	boxLengthA = sqrt( boxMatrix.at(0).at(0)*boxMatrix.at(0).at(0) +
			   boxMatrix.at(0).at(1)*boxMatrix.at(0).at(1) +
			   boxMatrix.at(0).at(2)*boxMatrix.at(0).at(2)  );
	boxLengthB = sqrt( boxMatrix.at(1).at(0)*boxMatrix.at(1).at(0) +
			   boxMatrix.at(1).at(1)*boxMatrix.at(1).at(1) +
			   boxMatrix.at(1).at(2)*boxMatrix.at(1).at(2)  );
	boxLengthC = sqrt( boxMatrix.at(2).at(0)*boxMatrix.at(2).at(0) +
			   boxMatrix.at(2).at(1)*boxMatrix.at(2).at(1) +
			   boxMatrix.at(2).at(2)*boxMatrix.at(2).at(2)  );
	
	boxLengthMax = max( boxLengthA, max(boxLengthB,boxLengthC) ); // determine longest edge
	boxRadius = 0.0025 * boxLengthMax;
	
	textEdit_povray->setText( "" );
	QTextStream stream( &file );
	QString line = "";
	
	while( !(line = stream.readLine()).isNull() )
	{
	    if( line.startsWith("#declare") )
	    {
		if     ( line.contains("camera_location") ) { line.sprintf("#declare camera_location  = <%.2f,%.2f,%.2f>;",         0.0,         0.0, -data.at(19) ); }
		//else if( line.contains("camera_rotation") ) { line.sprintf("#declare camera_rotation  = <%.2f,%.2f,%.2f>;", data.at(20), data.at(21),  data.at(22) ); }
		else if( line.contains("camera_rotation_x") ) { line.sprintf("#declare camera_rotation_x  = <%.2f,%.2f,%.2f>;",  data.at(20),         0.0,           0.0 ); }
		else if( line.contains("camera_rotation_y") ) { line.sprintf("#declare camera_rotation_y  = <%.2f,%.2f,%.2f>;",          0.0, data.at(21),           0.0 ); }
		else if( line.contains("camera_rotation_z") ) { line.sprintf("#declare camera_rotation_z  = <%.2f,%.2f,%.2f>;",          0.0,         0.0,  -data.at(22) ); }
		else if( line.contains("light_position") )  { line.sprintf("#declare light_position   = <%.2f,%.2f,%.2f>;", data.at(10), data.at(11), -data.at(12) ); }
		else if( line.contains("backgroundcolor") ) { line.sprintf("#declare backgroundcolor = <%.2f,%.2f,%.2f>;", data.at(13), data.at(14),  data.at(15) ); }
		else if( line.contains("show_box") )        { line.sprintf("#declare show_box = %d;", int(data.at(24)) ); }
		else if( line.contains("boxcolor") )        { line.sprintf("#declare boxcolor = <%.2f,%.2f,%.2f>;",  data.at(16),  data.at(17), data.at(18) );  }
		else if( line.contains("boxradius") )       { line.sprintf("#declare boxradius = %.2f;",  boxRadius );  }
		else if( line.contains("bbox_01a") )        { line.sprintf("#declare bbox_01a = <%.2f,%.2f,%.2f>;",  boxCoords.at( 0).at(0), boxCoords.at( 0).at(1), boxCoords.at( 0).at(2)  );  }
		else if( line.contains("bbox_01b") )        { line.sprintf("#declare bbox_01b = <%.2f,%.2f,%.2f>;",  boxCoords.at( 1).at(0), boxCoords.at( 1).at(1), boxCoords.at( 1).at(2)  );  }
		else if( line.contains("bbox_02a") )        { line.sprintf("#declare bbox_02a = <%.2f,%.2f,%.2f>;",  boxCoords.at( 2).at(0), boxCoords.at( 2).at(1), boxCoords.at( 2).at(2)  );  }
		else if( line.contains("bbox_02b") )        { line.sprintf("#declare bbox_02b = <%.2f,%.2f,%.2f>;",  boxCoords.at( 3).at(0), boxCoords.at( 3).at(1), boxCoords.at( 3).at(2)  );  }
		else if( line.contains("bbox_03a") )        { line.sprintf("#declare bbox_03a = <%.2f,%.2f,%.2f>;",  boxCoords.at( 4).at(0), boxCoords.at( 4).at(1), boxCoords.at( 4).at(2)  );  }
		else if( line.contains("bbox_03b") )        { line.sprintf("#declare bbox_03b = <%.2f,%.2f,%.2f>;",  boxCoords.at( 5).at(0), boxCoords.at( 5).at(1), boxCoords.at( 5).at(2)  );  }
		else if( line.contains("bbox_04a") )        { line.sprintf("#declare bbox_04a = <%.2f,%.2f,%.2f>;",  boxCoords.at( 6).at(0), boxCoords.at( 6).at(1), boxCoords.at( 6).at(2)  );  }
		else if( line.contains("bbox_04b") )        { line.sprintf("#declare bbox_04b = <%.2f,%.2f,%.2f>;",  boxCoords.at( 7).at(0), boxCoords.at( 7).at(1), boxCoords.at( 7).at(2)  );  }
		else if( line.contains("bbox_05a") )        { line.sprintf("#declare bbox_05a = <%.2f,%.2f,%.2f>;",  boxCoords.at( 8).at(0), boxCoords.at( 8).at(1), boxCoords.at( 8).at(2)  );  }
		else if( line.contains("bbox_05b") )        { line.sprintf("#declare bbox_05b = <%.2f,%.2f,%.2f>;",  boxCoords.at( 9).at(0), boxCoords.at( 9).at(1), boxCoords.at( 9).at(2)  );  }
		else if( line.contains("bbox_06a") )        { line.sprintf("#declare bbox_06a = <%.2f,%.2f,%.2f>;",  boxCoords.at(10).at(0), boxCoords.at(10).at(1), boxCoords.at(10).at(2)  );  }
		else if( line.contains("bbox_06b") )        { line.sprintf("#declare bbox_06b = <%.2f,%.2f,%.2f>;",  boxCoords.at(11).at(0), boxCoords.at(11).at(1), boxCoords.at(11).at(2)  );  }
		else if( line.contains("bbox_07a") )        { line.sprintf("#declare bbox_07a = <%.2f,%.2f,%.2f>;",  boxCoords.at(12).at(0), boxCoords.at(12).at(1), boxCoords.at(12).at(2)  );  }
		else if( line.contains("bbox_07b") )        { line.sprintf("#declare bbox_07b = <%.2f,%.2f,%.2f>;",  boxCoords.at(13).at(0), boxCoords.at(13).at(1), boxCoords.at(13).at(2)  );  }
		else if( line.contains("bbox_08a") )        { line.sprintf("#declare bbox_08a = <%.2f,%.2f,%.2f>;",  boxCoords.at(14).at(0), boxCoords.at(14).at(1), boxCoords.at(14).at(2)  );  }
		else if( line.contains("bbox_08b") )        { line.sprintf("#declare bbox_08b = <%.2f,%.2f,%.2f>;",  boxCoords.at(15).at(0), boxCoords.at(15).at(1), boxCoords.at(15).at(2)  );  }
		else if( line.contains("bbox_09a") )        { line.sprintf("#declare bbox_09a = <%.2f,%.2f,%.2f>;",  boxCoords.at(16).at(0), boxCoords.at(16).at(1), boxCoords.at(16).at(2)  );  }
		else if( line.contains("bbox_09b") )        { line.sprintf("#declare bbox_09b = <%.2f,%.2f,%.2f>;",  boxCoords.at(17).at(0), boxCoords.at(17).at(1), boxCoords.at(17).at(2)  );  }
		else if( line.contains("bbox_10a") )        { line.sprintf("#declare bbox_10a = <%.2f,%.2f,%.2f>;",  boxCoords.at(18).at(0), boxCoords.at(18).at(1), boxCoords.at(18).at(2)  );  }
		else if( line.contains("bbox_10b") )        { line.sprintf("#declare bbox_10b = <%.2f,%.2f,%.2f>;",  boxCoords.at(19).at(0), boxCoords.at(19).at(1), boxCoords.at(19).at(2)  );  }
		else if( line.contains("bbox_11a") )        { line.sprintf("#declare bbox_11a = <%.2f,%.2f,%.2f>;",  boxCoords.at(20).at(0), boxCoords.at(20).at(1), boxCoords.at(20).at(2)  );  }
		else if( line.contains("bbox_11b") )        { line.sprintf("#declare bbox_11b = <%.2f,%.2f,%.2f>;",  boxCoords.at(21).at(0), boxCoords.at(21).at(1), boxCoords.at(21).at(2)  );  }
		else if( line.contains("bbox_12a") )        { line.sprintf("#declare bbox_12a = <%.2f,%.2f,%.2f>;",  boxCoords.at(22).at(0), boxCoords.at(22).at(1), boxCoords.at(22).at(2)  );  }
		else if( line.contains("bbox_12b") )        { line.sprintf("#declare bbox_12b = <%.2f,%.2f,%.2f>;",  boxCoords.at(23).at(0), boxCoords.at(23).at(1), boxCoords.at(23).at(2)  );  }
		else if( line.contains("zoom") )            { line.sprintf("#declare zoom = %.2f;",  data.at(23) );  }
		else if( line.contains("ambient_l") )       { line.sprintf("#declare ambient_l = <%.2f,%.2f,%.2f>;", data.at(0), data.at(1), data.at(2) ); }
		else if( line.contains("diffuse_l") )       { line.sprintf("#declare diffuse_l = %.2f;",  (data.at(3)+data.at(4)+data.at(5))/3.0 ); }
		else if( line.contains("specular_l") )      { line.sprintf("#declare specular_l = %.2f;", (data.at(6)+data.at(7)+data.at(8))/3.0 ); }
		else if( line.contains("shininess") )       { line.sprintf("#declare shininess = %.2f;",  1.0/data.at(9) ); }
	    }
	    
	    textEdit_povray->append( line );
	    if( line == "" ) { textEdit_povray->append( "\n" ); }
	    
	}
	
	float sliceXLow = 0.0, sliceXHigh = 0.0, sliceYLow = 0.0, sliceYHigh = 0.0, sliceZLow = 0.0, sliceZHigh = 0.0;
	float posX = 0.0, posY = 0.0, posZ = 0.0;
	Molecule *tmpMol = 0;
	vector<float> parameters;
	
	if( glWindow->getDrawSlices() == true )
	{
	    glWindow->getSliceBounds( sliceXLow, sliceXHigh, sliceYLow, sliceYHigh, sliceZLow, sliceZHigh );
	}
	
	QTextStream ts( &line, IO_WriteOnly );
	for( int i = 0; i<cnf->getNumberOfMolecules(); i++ )
	{
	    /*    line = "";
	    line.sprintf( "discotic(<%f,%f,%f>,<%f,%f,%f>, <%f,%f,%f>)",  cnf->getMolecule(i)->getPositionX(),
									  cnf->getMolecule(i)->getPositionY(),
									 -cnf->getMolecule(i)->getPositionZ(), 
									  cnf->getMolecule(i)->getOrientationX(),
									  cnf->getMolecule(i)->getOrientationY(),
									 -cnf->getMolecule(i)->getOrientationZ(),
									  cnf->getMolecule(i)->getRed()/255.0,
									  cnf->getMolecule(i)->getGreen()/255.0,
									  cnf->getMolecule(i)->getBlue()/255.0 );
	  */ // do not use this! It is extremely slow...
	    
	    tmpMol = cnf->getMolecule(i);
	    
	    posX = ( cnf->getShowFolded() ? tmpMol->getPositionFoldedX() : tmpMol->getPositionX() );
	    posY = ( cnf->getShowFolded() ? tmpMol->getPositionFoldedY() : tmpMol->getPositionY() );
	    posZ = ( cnf->getShowFolded() ? tmpMol->getPositionFoldedZ() : tmpMol->getPositionZ() );
	    
	    if( glWindow->getDrawSlices() == true )
	    {
		if( posX > sliceXHigh || posX < sliceXLow || 
		    posY > sliceYHigh || posY < sliceYLow ||
		    posZ > sliceZHigh || posZ < sliceZLow ) { continue; }
	    }
	    
	    parameters = glWindow->getObjectParams()->at(tmpMol->getType());
	    if( parameters.at(4) == 0 ) { ts << "ellipsoid(<"; }
	    else                        { ts << "sphearocylinder(<"; }
	    ts <<  posX << "," <<  posY << "," << -posZ << ">," 
		    << "<" 
		    << tmpMol->getOrientationX() << "," 
		    << tmpMol->getOrientationY() << ","
		    << -tmpMol->getOrientationZ() << ">," 
		    << "<"
		    << tmpMol->getRed()/255.0    << ","
		    << tmpMol->getGreen()/255.0  << ","
		    << tmpMol->getBlue()/255.0   << ">,";
	    
	    	    //<< "<1,1,0.2>)" << endl;
	    
	    if( parameters.at(4) == 0 ) 
	    {
		ts << "<"
			<< 2*parameters.at(0) << ","
			<< 2*parameters.at(2) << "," 
			<< 2*parameters.at(1) << ">)" << endl; 
	    }
	    else
	    {
		ts << parameters.at(3) << "," 
			<< parameters.at(4) << ")" << endl; 
	    }
	    
	    if( i%100 == 0 )
	    {
		textEdit_povray->append( line );
		line = "";
	    }
	}
	
	textEdit_povray->append( line );
	
	file.close();
    }
}

//-------------------------------------------------------------------------
//------------- setParameter
//-------------------------------------------------------------------------
void PovrayForm::setParameter( CnfFile *c, Renderer *glw )
{
    cnf      = c;
    glWindow = glw;
}









