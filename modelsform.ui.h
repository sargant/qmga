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
void ModelsForm::init()
{
    models = 0;
    
    modelNames = "";
    
    connect( pushButton_commit, SIGNAL(clicked()), this, SLOT(commitChanges()) );
}

//-------------------------------------------------------------------------
//------------- addModel
//-------------------------------------------------------------------------
//void ModelsForm::addModel()
//{
//    //coming soon (or not...)
//}

//-------------------------------------------------------------------------
//------------- setModelNames
//-------------------------------------------------------------------------
void ModelsForm::setModelNames( const QStringList names )
{
    //cout << "ModelsForm::setModelNames beg" << endl;
    
    modelNames = names;
    
    //cout << "ModelsForm::setModelNames end" << endl;
}

//-------------------------------------------------------------------------
//------------- setModelsVector
//-------------------------------------------------------------------------
/*! 
 *  Sets pointer to the used models vector.
 *  \param modelsVec Pointer to the used models vector.
 *  \author Adrian Gabriel
 *  \date June 2005
 */
void ModelsForm::setModelsVector( vector<vector<float> > *modelsVec )
{
    //cout << "ModelsForm::setModelsVector beg" << endl;
    
    if( modelsVec != 0 )
    {
	models = modelsVec;
	setModelsTable();
    }
    
    //cout << "ModelsForm::setModelsVector end" << endl;
}

//-------------------------------------------------------------------------
//------------- updateModelsVector
//-------------------------------------------------------------------------
/*! 
 *  Updates table in case the models vector was changed.
 *  \author Adrian Gabriel
 *  \date June 2005
 */
void ModelsForm::updateModelsVector()
{
    //cout << "ModelsForm::updateModelsVector beg" << endl;
    
    if( models != 0 )
    {
	setModelsTable(); 
    }
    
    //cout << "ModelsForm::updateModelsVector end" << endl;
}

//-------------------------------------------------------------------------
//------------- setModelsTable
//-------------------------------------------------------------------------
/*! 
 *  If more than two models are in use, more lines are created in the table with default values.
 *  \author Adrian Gabriel
 *  \date June 2005
 */
void ModelsForm::setModelsTable()
{
    //cout << "ModelsForm::setModelsTable beg" << endl;
    
/*    cout << "--Model1:\n" 
	    << models->at(0).at(0) << "\n"
	    << models->at(0).at(1) << "\n"
	    << models->at(0).at(2) << "\n"
	    << models->at(0).at(3) << "\n"
	    << models->at(0).at(4) << "\n"
	    << models->at(0).at(5) << "\n"
	    << models->at(0).at(6) << "\n"
	    << models->at(0).at(7) << "\n"
	    << models->at(0).at(8) << "\n"
	    << models->at(0).at(9) << "\n"
	    << models->at(0).at(10) << "\n"
	    << models->at(0).at(11) << "\n"
	    << models->at(0).at(12) << "\n"
	    << models->at(0).at(13) << "\n"
	    << models->at(0).at(14) << "\n"
	    << models->at(0).at(15) << "\n"
	    << models->at(0).at(16) << endl;
    cout << "--Model2:\n" 
	    << models->at(1).at(0) << "\n"
	    << models->at(1).at(1) << "\n"
	    << models->at(1).at(2) << "\n"
	    << models->at(1).at(3) << "\n"
	    << models->at(1).at(4) << "\n"
	    << models->at(1).at(5) << "\n"
	    << models->at(1).at(6) << "\n"
	    << models->at(1).at(7) << "\n"
	    << models->at(1).at(8) << "\n"
	    << models->at(1).at(9) << "\n"
	    << models->at(1).at(10) << "\n"
	    << models->at(1).at(11) << "\n"
	    << models->at(1).at(12) << "\n"
	    << models->at(1).at(13) << "\n"
	    << models->at(1).at(14) << "\n"
	    << models->at(1).at(15) << "\n"
	    << models->at(1).at(16) << endl;
  */  
    if( models != 0 )
    {    
        if( models->size()>2 ) { table_models->setNumRows( models->size() ); }
	QString tmp;
	for(  unsigned int i = 0; i<models->size(); i++ )
	{
	    table_models->setItem( i, 0, new QComboTableItem( table_models, modelNames ) ); // model type
	    if( table_models->item( i, 0 )->rtti() == 1 )
	    {
		static_cast<QComboTableItem*>(table_models->item( i, 0 ))
			-> setCurrentItem( int(models->at(i).at(0)) );
	    }
	    
	    table_models->setText( i,  1, tmp.setNum( 2.0*models->at(i).at(1 ), 'f', 2 ) ); // x
	    table_models->setText( i,  2, tmp.setNum( 2.0*models->at(i).at(2 ), 'f', 2 ) ); // y
	    table_models->setText( i,  3, tmp.setNum( 2.0*models->at(i).at(3 ), 'f', 2 ) ); // z
	    table_models->setText( i,  4, tmp.setNum( 2.0*models->at(i).at(4 ), 'f', 2 ) ); // d
	    table_models->setText( i,  5, tmp.setNum(     models->at(i).at(5 ), 'f', 2 ) ); // l
	    table_models->setText( i,  6, tmp.setNum(     models->at(i).at(6 ), 'f', 2 ) ); // rad sphere spheroplatelet
	    table_models->setText( i,  7, tmp.setNum(     models->at(i).at(7 ), 'f', 2 ) ); // rad circle spheroplatelet
	    table_models->setText( i,  8, tmp.setNum(     models->at(i).at(8 ), 'f', 2 ) ); // rad sphere cut sphere
	    table_models->setText( i,  9, tmp.setNum(     models->at(i).at(9 ), 'f', 2 ) ); // cut distance from center cut sphere
	    table_models->setText( i, 10, tmp.setNum(     models->at(i).at(10), 'f', 2 ) ); // rad sphere eyelens
	    table_models->setText( i, 11, tmp.setNum(     models->at(i).at(11), 'f', 2 ) ); // opening alngle eyelens
	    
	    table_models->setItem( i, 12, new QCheckTableItem( table_models, QString("") ) ); // wireframe
	    if( table_models->item( i, 12 )->rtti() == 2 )
	    {
		static_cast<QCheckTableItem*>(table_models->item( i, 12 ))
			-> setChecked( models->at(i).at(12)!=0 ? true : false );
	    }
	    
	    table_models->setItem( i, 13, new QCheckTableItem( table_models, QString("") ) ); // force color
	    if( table_models->item( i, 13 )->rtti() == 2 )
	    {
		static_cast<QCheckTableItem*>(table_models->item( i, 13 ))
			-> setChecked( models->at(i).at(13)!=0 ? true : false );
	    }
	    
	    table_models->setText( i, 14, tmp.setNum(     models->at(i).at(14), 'f', 0 ) ); // red
	    table_models->setText( i, 15, tmp.setNum(     models->at(i).at(15), 'f', 0 ) ); // green
	    table_models->setText( i, 16, tmp.setNum(     models->at(i).at(16), 'f', 0 ) ); // blue
	}
	
	table_models -> adjustColumn( 0 );
	
	for( int i = 1; i<table_models->numCols(); i++ )
	{
	    table_models -> setColumnStretchable ( i, true );
	}
    }
    
    //cout << "ModelsForm::setModelsTable end" << endl;
}

//-------------------------------------------------------------------------
//------------- commitChanges
//-------------------------------------------------------------------------
/*! 
 *  Commits changes made to the model parameters to the main window.
 *  \author Adrian Gabriel
 *  \date June 2005
 */
void ModelsForm::commitChanges()
{ 
    //cout << "ModelsForm::commitChanges beg" << endl;
    
    if( models != 0 )
    {    
	for( unsigned int i = 0; i<models->size(); i++ )
	{
	    models->at(i).at(0 )  = float( static_cast<QComboTableItem*>(table_models->item(i,0))->currentItem() );
	    models->at(i).at(1 )  = table_models->text(i,1).toFloat()/2;
	    models->at(i).at(2 )  = table_models->text(i,2).toFloat()/2;
	    models->at(i).at(3 )  = table_models->text(i,3).toFloat()/2;
	    models->at(i).at(4 )  = table_models->text(i,4).toFloat()/2;
	    models->at(i).at(5 )  = table_models->text(i,5).toFloat();	    
	    models->at(i).at(6 )  = table_models->text(i,6).toFloat();
	    models->at(i).at(7 )  = table_models->text(i,7).toFloat();
	    models->at(i).at(8 )  = table_models->text(i,8).toFloat();
	    models->at(i).at(9 )  = table_models->text(i,9).toFloat();
	    models->at(i).at(10)  = table_models->text(i,10).toFloat();
	    models->at(i).at(11)  = table_models->text(i,11).toFloat();
	    models->at(i).at(12) = float( static_cast<QCheckTableItem*>(table_models->item(i,12))->isChecked() );
	    models->at(i).at(13) = float( static_cast<QCheckTableItem*>(table_models->item(i,13))->isChecked() );
	    models->at(i).at(14) = table_models->text(i,14).toFloat();
	    models->at(i).at(15) = table_models->text(i,15).toFloat();
	    models->at(i).at(16) = table_models->text(i,16).toFloat();
	}	
	emit modelDataChanged( false );
    }
    
    //cout << "ModelsForm::commitChanges end" << endl;
}

//-------------------------------------------------------------------------
//------------- writeFile
//-------------------------------------------------------------------------
/*! 
 *  Writes content of model table to file.
 *  \author Adrian Gabriel
 *  \date June 2008
 */
void ModelsForm::writeFile( QString file )
{
    ofstream out( file );
    if( out.is_open() )
    {
	for( uint i = 0; i<models->size(); ++i )
	{
	    out << setw(8) << models->at(i).at(0)
		    << setw(8) << models->at(i).at(1) * 2.0
		    << setw(8) << models->at(i).at(2) * 2.0
		    << setw(8) << models->at(i).at(3) * 2.0
		    << setw(8) << models->at(i).at(4) * 2.0
		    << setw(8) << models->at(i).at(5)
		    << setw(8) << models->at(i).at(6)
		    << setw(8) << models->at(i).at(7)
		    << setw(8) << models->at(i).at(8)
		    << setw(8) << models->at(i).at(9)
		    << setw(8) << models->at(i).at(10)
		    << setw(8) << models->at(i).at(11)
		    << setw(8) << models->at(i).at(12)
		    << setw(8) << models->at(i).at(13)
		    << setw(8) << models->at(i).at(14)
		    << setw(8) << models->at(i).at(15)
		    << setw(8) << models->at(i).at(16) << endl;
	}
	out.close();
    }
    else
    {
	cerr << "ERROR: Cannot open file >" << file << "< for writing. Nothing saved!" << endl;
    }
}



