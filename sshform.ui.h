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
void SSHForm::init()
{
    connect( lineEdit_user, SIGNAL(returnPressed()), this, SLOT(emitSSHChanged()) );
    connect( lineEdit_host, SIGNAL(returnPressed()), this, SLOT(emitSSHChanged()) );
    connect( lineEdit_user, SIGNAL(textChanged(const QString &)), this, SLOT(changedUserOrHost()) );
    connect( lineEdit_host, SIGNAL(textChanged(const QString &)), this, SLOT(changedUserOrHost()) );
    connect( pushButton_emptyTmp, SIGNAL(clicked()), this, SLOT(emptyTmp()) );
    connect( pushButton_current, SIGNAL(clicked()), this, SLOT(emitSSHRefreshCurrentDir()) );
    calculateTmpSize();
}

//-------------------------------------------------------------------------
//------------- emitSSHChanged
//-------------------------------------------------------------------------
void SSHForm::emitSSHChanged()
{
    QString user = lineEdit_user -> text().stripWhiteSpace();
    QString host = lineEdit_host -> text().stripWhiteSpace();
    
    if( user != "" && host != "" )
    {
	QStringList list;
	list.append( user );
	list.append( host );
	
	emit sshChanged( list );
    }
}

//-------------------------------------------------------------------------
//------------- setUser
//-------------------------------------------------------------------------
void SSHForm::setUser( QString newUser )
{
    lineEdit_user -> setText( newUser );
}

//-------------------------------------------------------------------------
//------------- setHost
//-------------------------------------------------------------------------
void SSHForm::setHost( QString newHost )
{
    lineEdit_host -> setText( newHost );
}

//-------------------------------------------------------------------------
//------------- setCurrent
//-------------------------------------------------------------------------
void SSHForm::setCurrent( QString newCurrent )
{
    lineEdit_current -> setText( newCurrent );
}

//-------------------------------------------------------------------------
//------------- emitSSHRefreshCurrentDir
//-------------------------------------------------------------------------
void SSHForm::emitSSHRefreshCurrentDir()
{
    emit sshRefreshCurrentDir();
}

//-------------------------------------------------------------------------
//------------- emptyTmp
//-------------------------------------------------------------------------
void SSHForm::emptyTmp()
{
    QDir tmpDir( "./scp-tmp" ); 
    
    if( tmpDir.exists() != false )
    {
	if( QMessageBox::question( this,
				   "QMGA - empty temporary files",
				   QString("Delete all files in directory\n./scp-tmp ?"),
				   QMessageBox::Yes, QMessageBox::No ) == QMessageBox::Yes )
	{
	    const QFileInfoList *list = tmpDir.entryInfoList();
	    QFileInfoListIterator it( *list );
	    QFileInfo *fi;
	    
	    while ( (fi = it.current()) != 0 )
	    {
		if( fi -> fileName().startsWith(".") == false )
		{
		    tmpDir.remove( fi -> fileName() );
		}
		++it;
	    }
	    calculateTmpSize();
	}
    }
    else
    {
	cerr << "Warning: ./scp-tmp does not exist" << endl;       
    }
}

//-------------------------------------------------------------------------
//------------- calculateTmpSize
//-------------------------------------------------------------------------
void SSHForm::calculateTmpSize()
{
    QDir tmpDir( "./scp-tmp" ); 
    if( tmpDir.exists() != false )
    {
	double sumSize = 0.0;
	const QFileInfoList *list = tmpDir.entryInfoList();
	QFileInfoListIterator it( *list );
	QFileInfo *fi;
	
	while ( (fi = it.current()) != 0 )
	{
	    if( fi -> fileName().startsWith(".") == false )
	    {
		sumSize += fi -> size();
	    }
	    ++it;
	}

	QString sum;
	if( sumSize < 1024.0 )
	{
	    sum = sum.setNum( sumSize, 'f', 3 ) + " Byte";
	}
	else if( sumSize < 1024.0*1024.0 )
	{
	    sum = sum.setNum( sumSize / 1024.0, 'f', 3 ) + " kB";
	}
	else if( sumSize < 1024.0*1024.0*1024.0 )
	{
	    sum = sum.setNum( sumSize / (1024.0*1024.0), 'f', 3 ) + " MB";
	}
	else if( sumSize < 1024.0*1024.0*1024.0*1024.0 )
	{
	    sum = sum.setNum( sumSize / (1024.0*1024.0*1024.0), 'f', 3 ) + " GB";
	}
	else if( sumSize < 1024.0*1024.0*1024.0*1024.0*1024.0 )
	{
	    sum = sum.setNum( sumSize / (1024.0*1024.0*1024.0*1024.0), 'f', 3 ) + " TB (too much disk space?)";
	}
	lineEdit_emptyTmp -> setText( sum );	
    }
    else
    {
	cerr << "Warning: ./scp-tmp does not exist" << endl;       
    }
}

//-------------------------------------------------------------------------
//------------- showUsedUserAndHost
//-------------------------------------------------------------------------
void SSHForm::showUsedUserAndHost( QString userAtHost )
{
    textLabel_userAtHost -> setText( QString("user@host: ")+userAtHost );
    textLabel_userAtHost->setPaletteForegroundColor( QColor(0,0,0) );
    
}

//-------------------------------------------------------------------------
//------------- changedUserOrHost
//-------------------------------------------------------------------------
void SSHForm::changedUserOrHost()
{
    textLabel_userAtHost->setPaletteForegroundColor( QColor(255,0,0) );
}



