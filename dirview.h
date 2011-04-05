/****************************************************************************
 ** $Id: qt/dirview.h   3.3.7   edited Aug 31 2005 $
 **
 ** Copyright (C) 1992-2005 Trolltech AS.  All rights reserved.
 **
 ** This file is part of an example program for Qt.  This example
 ** program may be used, distributed and modified without limitation.
 **
 ** And so I did.
 ** Cleansed the code of everything I do not need and wrote some new parts.
 **
 ** Adrian Gabriel Apr 2007
 **
 *****************************************************************************/

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

#ifndef DIRVIEW_H
#define DIRVIEW_H

#include <qlistview.h>
#include <qprocess.h>

class QWidget;
class QDragEnterEvent;
class QDragMoveEvent;
class QDragLeaveEvent;
class QDropEvent;

//------------------------------------------------------
//--------------- class FileItem
//------------------------------------------------------
class FileItem : public QListViewItem
{
public:
    FileItem( QListViewItem *parent, const QString &s1, const QString &s2 ) : QListViewItem( parent, s1, s2 ), pix( 0 ) {}
    
    const QPixmap *pixmap( int i ) const;
    void setPixmap( QPixmap *px );
    
private:
    QPixmap *pix;
};

//------------------------------------------------------
//--------------- class Directory
//------------------------------------------------------
class Directory : public QListViewItem
{
public:
    Directory( QListView * parent, const QString& filename );
    Directory( Directory * parent, const QString& filename );
    
    QString text( int column ) const;
    
    QString fullName();
    
    void setOpen( bool );
    void setup();
    
    const QPixmap *pixmap( int i ) const;
    void setPixmap( QPixmap *px );
    
private:
    QFile file;
    Directory * par;
    bool readable;
    QPixmap *pix;
};

//------------------------------------------------------
//--------------- class SSHFileItem
//------------------------------------------------------
class SSHFileItem : public QListViewItem
{
public:
    SSHFileItem( QListViewItem *parent, const QString &s1, const QString &s2 ) : QListViewItem( parent, s1, s2 ), pix( 0 ) {}
    
    const QPixmap *pixmap( int i ) const;
    void setPixmap( QPixmap *px );
    
private:
    QPixmap *pix;
};

//------------------------------------------------------
//--------------- class SSHDirectory
//------------------------------------------------------
class SSHDirectory : public QListViewItem
{
public:
    SSHDirectory( QListView * parent );
    SSHDirectory( SSHDirectory * parent, const QString& nam );
    
    QString text( int column ) const;
    
    QString fullName();
    
    void setOpen( bool );
    void setup();
    
    const QPixmap *pixmap( int i ) const;
    void setPixmap( QPixmap *px );
    void refresh();
    
    int isExecutableDir( const QString &dirPath );
    QStringList *getDirEntries() { return( dirEntries ); }
    
private:
    const QString fileLsLine;
    QString name;
    QString rights;
    SSHDirectory * par;
    bool readable;
    QPixmap *pix;
    bool refreshDirEntries();
    QStringList *dirEntries;
    void readSshStdOut();
};

//------------------------------------------------------
//--------------- class DirectoryView
//------------------------------------------------------
class DirectoryView : public QListView
{
    Q_OBJECT
    
public:
    DirectoryView( QWidget *parent = 0, const char *name = 0 );
    void emitOpenSSH( bool status );
    void refreshCurrentDirectory();
    void setCurrentDirectory( SSHDirectory *dir ) 
    {
	currentDirectory = dir; 
	emit currentDirectoryChanged( currentDirectory -> text(0) );
    }
    
    int  isSSHDirExecutable( const QString &dirPath );
    bool refreshSSHDir( SSHDirectory *dir );
    void setUser( QString usr ) { user = usr; }
    void setHost( QString hst ) { host = hst; }
    QString getUser() { return(user); }
    QString getHost() { return(host); }
    void setSSH() { if( ssh == 0 ) { ssh = new QProcess(); } }
    bool isSSHSet()
    {
	if( ssh != 0 ) { return( true  ); }
	else           { return( false ); }
    }
    
protected:
    void contentsMouseDoubleClickEvent( QMouseEvent *e );
    void contentsMouseMoveEvent( QMouseEvent *e );
    void contentsMousePressEvent( QMouseEvent *e );
    void contentsMouseReleaseEvent( QMouseEvent *e );
    
private:
    QString fullPath( QListViewItem* item );
    void checkAndRefresh( QListViewItem *p );
    QListViewItem *oldCurrent;
    QListViewItem *dropItem;
    QPoint presspos;
    bool mousePressed;
    SSHDirectory *currentDirectory;
    QProcess * ssh;
    QString user;
    QString host;
    SSHDirectory *refreshDir;

private slots:
    void readSSHStdOut();
    
signals:
    void openSSH( bool );
    void currentDirectoryChanged( QString );
    void doubleClickedFile(QString);
};

#endif //DIRVIEW_H
