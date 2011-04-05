/****************************************************************************
 ** $Id: qt/dirview.cpp   3.3.7   edited Aug 31 2005 $
 **
 ** Copyright (C) 1992-2005 Trolltech AS.  All rights reserved.
 **
 ** This file is part of an example program for Qt.  This example
 ** program may be used, distributed and modified without limitation.
 **
 ** And so I did.
 ** Cleansed the code of everything I do not need and wrote new parts.
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

#include "dirview.h"

#include <qmessagebox.h>
#include <qdragobject.h>
#include <qapplication.h>
#include <qheader.h>

#include <iostream>

using std::cout;
using std::endl;

static const char* folder_closed_xpm[]={
  "16 16 9 1",
  "g c #808080",
  "b c #c0c000",
  "e c #c0c0c0",
  "# c #000000",
  "c c #ffff00",
  ". c None",
  "a c #585858",
  "f c #a0a0a4",
  "d c #ffffff",
  "..###...........",
  ".#abc##.........",
  ".#daabc#####....",
  ".#ddeaabbccc#...",
  ".#dedeeabbbba...",
  ".#edeeeeaaaab#..",
  ".#deeeeeeefe#ba.",
  ".#eeeeeeefef#ba.",
  ".#eeeeeefeff#ba.",
  ".#eeeeefefff#ba.",
  ".##geefeffff#ba.",
  "...##gefffff#ba.",
  ".....##fffff#ba.",
  ".......##fff#b##",
  ".........##f#b##",
  "...........####."};

static const char* folder_open_xpm[]={
  "16 16 11 1",
  "# c #000000",
  "g c #c0c0c0",
  "e c #303030",
  "a c #ffa858",
  "b c #808080",
  "d c #a0a0a4",
  "f c #585858",
  "c c #ffdca8",
  "h c #dcdcdc",
  "i c #ffffff",
  ". c None",
  "....###.........",
  "....#ab##.......",
  "....#acab####...",
  "###.#acccccca#..",
  "#ddefaaaccccca#.",
  "#bdddbaaaacccab#",
  ".eddddbbaaaacab#",
  ".#bddggdbbaaaab#",
  "..edgdggggbbaab#",
  "..#bgggghghdaab#",
  "...ebhggghicfab#",
  "....#edhhiiidab#",
  "......#egiiicfb#",
  "........#egiibb#",
  "..........#egib#",
  "............#ee#"};

static const char * folder_locked[]={
  "16 16 10 1",
  "h c #808080",
  "b c #ffa858",
  "f c #c0c0c0",
  "e c #c05800",
  "# c #000000",
  "c c #ffdca8",
  ". c None",
  "a c #585858",
  "g c #a0a0a4",
  "d c #ffffff",
  "..#a#...........",
  ".#abc####.......",
  ".#daa#eee#......",
  ".#ddf#e##b#.....",
  ".#dfd#e#bcb##...",
  ".#fdccc#daaab#..",
  ".#dfbbbccgfg#ba.",
  ".#ffb#ebbfgg#ba.",
  ".#ffbbe#bggg#ba.",
  ".#fffbbebggg#ba.",
  ".##hf#ebbggg#ba.",
  "...###e#gggg#ba.",
  ".....#e#gggg#ba.",
  "......###ggg#b##",
  ".........##g#b##",
  "...........####."};

static const char * pix_file []={
  "16 16 7 1",
  "# c #000000",
  "b c #ffffff",
  "e c #000000",
  "d c #404000",
  "c c #c0c000",
  "a c #ffffc0",
  ". c None",
  "................",
  ".........#......",
  "......#.#a##....",
  ".....#b#bbba##..",
  "....#b#bbbabbb#.",
  "...#b#bba##bb#..",
  "..#b#abb#bb##...",
  ".#a#aab#bbbab##.",
  "#a#aaa#bcbbbbbb#",
  "#ccdc#bcbbcbbb#.",
  ".##c#bcbbcabb#..",
  "...#acbacbbbe...",
  "..#aaaacaba#....",
  "...##aaaaa#.....",
  ".....##aa#......",
  ".......##......."};

QPixmap *folderLocked = 0;
QPixmap *folderClosed = 0;
QPixmap *folderOpen = 0;
QPixmap *fileNormal = 0;

/*****************************************************************************
 *
 * Class FileItem
 *
 *****************************************************************************/

//------------------------------------------------------
//--------------- setPixmap
//------------------------------------------------------
void FileItem::setPixmap( QPixmap *p )
{
  pix = p;
  setup();
  widthChanged( 0 );
  invalidateHeight();
  repaint();
}

//------------------------------------------------------
//--------------- pixmap
//------------------------------------------------------
const QPixmap *FileItem::pixmap( int i ) const
{
  if( i )
    {
      return 0;
    }
  return pix;
}

/*****************************************************************************
 *
 * Class Directory
 *
 *****************************************************************************/

//------------------------------------------------------
//--------------- Directory
//------------------------------------------------------
Directory::Directory( Directory * parent, const QString& filename )
  : QListViewItem( parent ), file(filename),  pix( 0 )
{
  par = parent;
  readable = QDir( fullName() ).isReadable();

  if( readable == false )
    {
      setPixmap( folderLocked );
    }
  else
    {
      setPixmap( folderClosed );
    }
}

//------------------------------------------------------
//--------------- Directory
//------------------------------------------------------
Directory::Directory( QListView * parent, const QString& filename )
  : QListViewItem( parent ), file(filename), pix( 0 )
{
  par = 0;
  readable = QDir( fullName() ).isReadable();
}

//------------------------------------------------------
//--------------- setPixmap
//------------------------------------------------------
void Directory::setPixmap( QPixmap *px )
{
  pix = px;
  setup();
  widthChanged( 0 );
  invalidateHeight();
  repaint();
}

//------------------------------------------------------
//--------------- pixmap
//------------------------------------------------------
const QPixmap *Directory::pixmap( int i ) const
{
  if( i != 0 )
    {
      return 0;
    }
  return pix;
}

//------------------------------------------------------
//--------------- setOpen
//------------------------------------------------------
void Directory::setOpen( bool open )
{
  if( open == true )
    {
      setPixmap( folderOpen );
    }
  else
    {
      setPixmap( folderClosed );
    }

  if ( open == true && childCount() == 0 ) 
    {
      QString s( fullName() );
      QDir thisDir( s );
      if ( thisDir.isReadable() == false ) 
	{
	  readable = false;
	  setExpandable( false );
	  return;
	}

      listView() -> setUpdatesEnabled( false );
      const QFileInfoList * files = thisDir.entryInfoList();
      if( files ) 
	{
	  QFileInfoListIterator it( *files );
	  QFileInfo * fi;
	  while( (fi=it.current()) != 0 ) 
	    {
	      ++it;
	      if( fi->fileName() == "." || fi->fileName() == ".." )
		{
		  ; // nothing
		}
	      else if( fi->isSymLink() ) 
		{
		  FileItem *item = new FileItem( this, fi->fileName(), "Symbolic Link" );
		  item->setPixmap( fileNormal );
		}
	      else if( fi->isDir() )
		{
		  (void)new Directory( this, fi->fileName() );
		}
	      else 
		{
		  FileItem *item = new FileItem( this, fi->fileName(), fi->isFile()?"File":"Special" );
		  item->setPixmap( fileNormal );
		}
	    }
	}
      listView()->setUpdatesEnabled( true );
    }
  QListViewItem::setOpen( open );
}

//------------------------------------------------------
//--------------- setup
//------------------------------------------------------
void Directory::setup()
{
  setExpandable( true );
  QListViewItem::setup();
}

//------------------------------------------------------
//--------------- fullName
//------------------------------------------------------
QString Directory::fullName()
{
  QString s;
  if( par != 0 )
    {
      s = par->fullName();
      s.append( file.name() );
      s.append( "/" );
    } 
  else 
    {
      s = file.name();
    }

  return s;
}

//------------------------------------------------------
//--------------- text
//------------------------------------------------------
QString Directory::text( int column ) const
{
  if( column == 0 )
    {
      return file.name();
    }
  else if( readable )
    {
      return "Directory";
    }
  else
    {
      return "Unreadable Directory";
    }
}

//--------------------------------------------------------------------------------------

/*****************************************************************************
 *
 * Class SSHFileItem
 *
 *****************************************************************************/

//------------------------------------------------------
//--------------- setPixmap
//------------------------------------------------------
void SSHFileItem::setPixmap( QPixmap *p )
{
  pix = p;
  setup();
  widthChanged( 0 );
  invalidateHeight();
  repaint();
}

//------------------------------------------------------
//--------------- pixmap
//------------------------------------------------------
const QPixmap *SSHFileItem::pixmap( int i ) const
{
  if( i )
    {
      return 0;
    }
  return pix;
}

/*****************************************************************************
 *
 * Class SSHDirectory
 *
 *****************************************************************************/

//------------------------------------------------------
//--------------- SSHDirectory
//------------------------------------------------------
SSHDirectory::SSHDirectory( SSHDirectory * parent, const QString& nam )
  : QListViewItem( parent ), name(nam),  pix( 0 ), dirEntries( 0 )
{
  dirEntries = new QStringList();
  par = parent;
  readable = true;

  if( readable == false )
    {
      setPixmap( folderLocked );
    }
  else
    {
      setPixmap( folderClosed );
    }
}

//------------------------------------------------------
//--------------- SSHDirectory
//------------------------------------------------------
SSHDirectory::SSHDirectory( QListView * parent )
  : QListViewItem( parent ), fileLsLine(""), pix( 0 ), dirEntries( 0 )
{
  par = 0;
  readable = true;
  name = "/";
  dirEntries = new QStringList();
}

//------------------------------------------------------
//--------------- setPixmap
//------------------------------------------------------
void SSHDirectory::setPixmap( QPixmap *px )
{
  pix = px;
  setup();
  widthChanged( 0 );
  invalidateHeight();
  repaint();
}

//------------------------------------------------------
//--------------- pixmap
//------------------------------------------------------
const QPixmap *SSHDirectory::pixmap( int i ) const
{
  if( i != 0 )
    {
      return 0;
    }
  return pix;
}

//------------------------------------------------------
//--------------- refresh
//------------------------------------------------------
void SSHDirectory::refresh()
{
    if( childCount() != 0 )
    {
	while( firstChild() != 0 ) { delete(firstChild()); }
    }
    setOpen( true );
}

//------------------------------------------------------
//--------------- setOpen
//------------------------------------------------------
void SSHDirectory::setOpen( bool open )
{
    //cout << "setopen beg" << endl;
    if( open == true )
    {
	setPixmap( folderOpen );
    }
    else
    {
	setPixmap( folderClosed );
    }
    QListView *view = 0;
    DirectoryView *dirView = 0;
    view = listView();
    if( view != 0 )
    {
	dirView = dynamic_cast<DirectoryView *>( view );
	if( dirView != NULL ) 
	{
	    dirView -> emitOpenSSH( true );
	    dirView -> setCurrentDirectory( this );
	}
    }
    
    if( open == true && childCount() == 0 ) 
    {
	if( dirView != NULL ) { dirView -> emitOpenSSH( true ); }

	refreshDirEntries();
	if( isExecutableDir( fullName() ) == false )
	{
	    readable = false;
	    setExpandable( false );
	    if( dirView != NULL ) { dirView -> emitOpenSSH(false); }
	    return;
	}
	
	listView() -> setUpdatesEnabled( false );
	if( dirEntries -> size() >= 1 ) 
	{
	    QString line="";
	    
	    for( unsigned int i = 0; i < dirEntries -> size(); i++ )
	    {
		line = *dirEntries->at(i);
		
		if( line == "" )
		{
		    ;
		}
		else if( line.endsWith("/") ) // Directory
		{
		    (void)new SSHDirectory( this, line.remove(line.length()-1, 1) );
		}
		else if( line.endsWith("@") ) // Symbolic link
		{
		    ;
		}
		else if( line.endsWith("|") ) // FiFo
		{
		    ;
		}
		else if( line.endsWith("*") ) // Executable
		{
		    ;
		}
		else if( line.endsWith("=") ) // Socket
		{
		    ;
		}
		else // Normal file
		{
		    SSHFileItem *item = new SSHFileItem( this, line, "File" );
		    item -> setPixmap( fileNormal );
		}
	    }
	    
//	    QStringList line;
//	    for( unsigned int i = 0; i < dirEntries -> size(); i++ )
//	    {
//		line = QStringList::split( " ", *dirEntries->at(i) );
//		if( line.size() >= 8 )
//		{
//		    if( (*line.at(8)).startsWith(".") )
//		    {
//			; // nothing
//		    }
//		    else if( (*line.at(0)).startsWith("l") ) 
//		    {
//			SSHFileItem *item = new SSHFileItem( this, *line.at(8), "Symbolic Link" );
//			item -> setPixmap( fileNormal );
//		    }
//		    else if( (*line.at(0)).startsWith("d") )
//		    {
//			(void)new SSHDirectory( this, *line.at(8) );
//		    }
//		    else if( (*line.at(0)).startsWith("-") )
//		    {
//			SSHFileItem *item = new SSHFileItem( this, *line.at(8), "File" );
//			item -> setPixmap( fileNormal );
//		    }
//		    else
//		    {
//			; // nothing
//		    }
//		}
//		else
//		{
//		    cout << "Error: ls - output format not readable!" << endl; 
//		}
//	    }
	    
	}
	listView() -> setUpdatesEnabled( true );
    }
    QListViewItem::setOpen( open );
    
    if( dirView != NULL ) { dirView -> emitOpenSSH(false); }
    
    //cout << "setopen end" << endl;
}

//------------------------------------------------------
//--------------- refreshDirEntries
//------------------------------------------------------
bool SSHDirectory::refreshDirEntries()
{
    //cout << "refreshDirEntries beg" << endl;
    
    bool success = false;
    QListView *view = 0;
    DirectoryView *dirView = 0;
    view = listView();
    if( view != 0 )
    {
	dirView = dynamic_cast<DirectoryView *>( view );
	if( dirView != NULL ) 
	{
	    success = dirView -> refreshSSHDir( this );
	}
    }
       
    return ( success );
    //cout << "refreshDirEntries end" << endl;
}

//------------------------------------------------------
//--------------- setup
//------------------------------------------------------
void SSHDirectory::setup()
{
  setExpandable( true );
  QListViewItem::setup();
}

//------------------------------------------------------
//--------------- fullName
//------------------------------------------------------
QString SSHDirectory::fullName()
{
  QString s="";
  
  if( par != 0 )
    {
      s = par -> fullName();
      s.append( name );
      s.append( "/" );
    } 
  else 
    {
      s = name;
    }
  
  return s;
}

//------------------------------------------------------
//--------------- text
//------------------------------------------------------
QString SSHDirectory::text( int column ) const
{
    if( column == 0 )
    {
	if( name == "/" ) { return( QString("SSH/") ); }
	else              { return name; }
    }
    else if( readable )
    {
	return "SSH Directory";
    }
    else
    {
	return "Unreadable SSH Directory";
    }
}

//------------------------------------------------------
//--------------- isExecutableDir
//------------------------------------------------------
int SSHDirectory::isExecutableDir( const QString &dirPath )
{
    //cout << "isExecutableDir beg" << endl;
    
    bool success = false;
    QListView *view = 0;
    DirectoryView *dirView = 0;
    view = listView();
    if( view != 0 )
    {
	dirView = dynamic_cast<DirectoryView *>( view );
	if( dirView != NULL ) 
	{
	    success = dirView -> isSSHDirExecutable( dirPath );
	}
    }
    
    //cout << "isExecutableDir end" << endl;
    return( success );    
}

//--------------------------------------------------------------------------------------


/*****************************************************************************
 *
 * Class DirectoryView
 *
 *****************************************************************************/

//------------------------------------------------------
//--------------- DirectoryView
//------------------------------------------------------
DirectoryView::DirectoryView( QWidget *parent, const char *name )
  : QListView( parent, name ), oldCurrent( 0 ), dropItem( 0 ), mousePressed( false )
{
  //cout << "DirectoryView::DirectoryView beg" << endl;
  if( folderLocked == 0 ) 
    {
      folderLocked = new QPixmap( folder_locked );
      folderClosed = new QPixmap( folder_closed_xpm );
      folderOpen   = new QPixmap( folder_open_xpm );
      fileNormal   = new QPixmap( pix_file );
    }
  //cout << "DirectoryView::DirectoryView end" << endl;
}

//------------------------------------------------------
//--------------- fullPath
//------------------------------------------------------
QString DirectoryView::fullPath( QListViewItem* item )
{
  //cout << "DirectoryView::fullPath beg" << endl;
  QString fullpath = item -> text(0);
  
  while( (item = item -> parent()) ) 
    {
      if( item -> parent() )
	{
	  fullpath = item -> text(0) + "/" + fullpath;
	}
      else
	{
	  fullpath = item -> text(0) + fullpath;
	}
    }
#ifdef Q_WS_WIN
  if( fullpath.length() > 2 && fullpath.at(1) != ':' )
    {
      QDir dir(fullpath);
      fullpath = dir.currentDirPath().left(2) + fullpath;
    }
#endif
  //cout << "DirectoryView::fullPath end" << endl;
  return fullpath;
}

//------------------------------------------------------
//--------------- contentsMouseDoubleClickEvent
//------------------------------------------------------
void DirectoryView::contentsMouseDoubleClickEvent( QMouseEvent* e )
{
    //cout << "DirectoryView::contentsMouseDoubleClickEvent beg" << endl;
    
    QListViewItem *item = itemAt( contentsToViewport(e -> pos()) );
    if( item )
    {
	SSHFileItem *sshFile = dynamic_cast<SSHFileItem *>( item );
	if( sshFile != NULL )
	{
	    QString source = fullPath( sshFile );
	    QString file = QString("SSH|")+getUser()+"|"+getHost()+"|"+source.remove(0,3);

	    emit doubleClickedFile( file );
	}
	else
	{
	    FileItem *localFile = dynamic_cast<FileItem *>( item );
	    if( localFile != NULL )
	    {
		emit doubleClickedFile( fullPath( localFile ) );
	    }
	    
	}
    }
    
    QListView::contentsMouseDoubleClickEvent( e );
    
    //cout << "DirectoryView::contentsMouseDoubleClickEvent end" << endl;
}

//------------------------------------------------------
//--------------- checkAndRefresh
//------------------------------------------------------
void DirectoryView::checkAndRefresh( QListViewItem *p )
{
    //cout << "DirectoryView::checkAndRefresh beg" << endl;
    if( p )
    {
	SSHDirectory *SSHDir = dynamic_cast<SSHDirectory *>( p );
	if( SSHDir != NULL )
	{
	    SSHDir -> refresh();
	}
    }
    //cout << "DirectoryView::checkAndRefresh end" << endl;
}
	
//------------------------------------------------------
//--------------- contentsMousePressEvent
//------------------------------------------------------
void DirectoryView::contentsMousePressEvent( QMouseEvent* e )
{
    //cout << "DirectoryView::contentsMousePressEvent beg" << endl;
    
    QListView::contentsMousePressEvent( e );
    QPoint p( contentsToViewport( e -> pos() ) );
    QListViewItem *item = itemAt( p );
    
    if( item ) 
    {
	if( e -> button() !=  Qt::MidButton )
	{
	    // if the user clicked into the root decoration of the item, don't try to start a drag!
	    if( p.x() > header()->cellPos( header()->mapToActual( 0 ) ) +
		treeStepSize() * ( item->depth() + ( rootIsDecorated() ? 1 : 0) ) + itemMargin() ||
		p.x() < header()->cellPos( header()->mapToActual( 0 ) ) ) 
	    {
		presspos = e -> pos();
		mousePressed = true;
	    }
	}
    }
    
    //cout << "DirectoryView::contentsMousePressEvent end" << endl;
}

//------------------------------------------------------
//--------------- contentsMouseMoveEvent
//------------------------------------------------------
void DirectoryView::contentsMouseMoveEvent( QMouseEvent* e )
{
    //if(mousePressed) cout << "DirectoryView::contentsMouseMoveEvent beg" << endl;
    
    if( mousePressed && ( presspos - e->pos() ).manhattanLength() > QApplication::startDragDistance() ) 
    {
	mousePressed = false;
	QListViewItem *item = itemAt( contentsToViewport(presspos) );
	if( item )
	{
	    QString source = fullPath( item );
	    
	    if( QFile::exists(source) ) 
	    {
		QUriDrag* ud = new QUriDrag( viewport() );
		ud -> setFileNames( source );
		if( ud -> drag() )
		{
		    QMessageBox::information( this, "Drag source", QString("Delete ") + QDir::convertSeparators(source), "Not implemented" );
		}
	    }
	    else if( source.startsWith("SSH/") )
	    {
		QTextDrag* td = new QTextDrag( viewport() );
		td -> setText( QString("SSH|")+getUser()+"|"+getHost()+"|"+source.remove(0,3) );
		if( td -> drag() )
		{
		    QMessageBox::information( this, "Drag source", QString("Delete ") + QDir::convertSeparators(source), "Not implemented" );
		}
	    }
	}
    }
    
    //if(mousePressed) cout << "DirectoryView::contentsMouseMoveEvent end" << endl;
}

//------------------------------------------------------
//--------------- contentsMouseReleaseEvent
//------------------------------------------------------
void DirectoryView::contentsMouseReleaseEvent( QMouseEvent *e )
{
    //cout << "DirectoryView::contentsMouseReleaseEvent beg" << endl;
    
    mousePressed = false;
    
    if( e -> button() ==  Qt::MidButton )
    {
	QListViewItem *item = itemAt( contentsToViewport(e -> pos()) );
	checkAndRefresh( item );
    }
    
    //cout << "DirectoryView::contentsMouseReleaseEvent end" << endl;
}

//------------------------------------------------------
//--------------- emitOpenSSH
//------------------------------------------------------
void DirectoryView::emitOpenSSH( bool status )
{
    //cout << "DirectoryView::emitOpenSSH beg" << endl;
    emit openSSH( status );
    //cout << "DirectoryView::emitOpenSSH end" << endl;
}

//------------------------------------------------------
//--------------- refreshCurrentDirectory
//------------------------------------------------------
void DirectoryView::refreshCurrentDirectory()
{
    //cout << "DirectoryView::refreshCurrentDirectory beg" << endl;
     currentDirectory -> refresh();
    //cout << "DirectoryView::refreshCurrentDirectory end" << endl;
}

//------------------------------------------------------
//--------------- refreshSSHDir()
//------------------------------------------------------
bool DirectoryView::refreshSSHDir( SSHDirectory *dir )
{
    //cout << "DirectoryView::refreshSSHDir beg" << endl;
    
    if( ssh != NULL )
    {
	refreshDir = dir;

	connect( ssh, SIGNAL(readyReadStdout()), this, SLOT(readSSHStdOut()) );
	connect( ssh, SIGNAL(readyReadStderr()), this, SLOT(readSSHStdOut()) );
	connect( ssh, SIGNAL(processExited())  , this, SLOT(readSSHStdOut()) );
	
	ssh -> clearArguments();
	ssh -> addArgument("ssh");
	ssh -> addArgument( user + "@" + host );
	//ssh -> addArgument( "ls -l " + refreshDir -> fullName() );
	ssh -> addArgument( "ls -1F " + refreshDir -> fullName() );
	ssh -> start();
	
	//cout << "ssh " << user + "@" + host << " ls -l " + refreshDir -> fullName() << endl;
	//cout << "ssh " << user + "@" + host << " ls -1F " + refreshDir -> fullName() << endl;
	
	refreshDir -> getDirEntries() -> clear();

	int count = 0;
	while( ssh -> isRunning() == true )
	{
	    count++;
	    if(count >= 2 )
	    {
		// this strange line prevents a hang of the ssh process when there are 
		// more than about 435 files in a folder.
		// Do not remove it!
		ssh -> canReadLineStdout();
	    }
	    sleep(1);
	}
	//cout << "wait end count = " << count << endl;
    }
    
    //cout << "DirectoryView::refreshSSHDir end" << endl;
    return( ssh -> exitStatus() == 0 );
}

//------------------------------------------------------
//--------------- readSSHStdOut()
//------------------------------------------------------
void DirectoryView::readSSHStdOut()
{
    //cout << "DirectoryView::readSSHStdOut beg" << endl;    
    
    while( ssh -> canReadLineStdout() == true )
    {
	//QString test = ssh -> readLineStdout();
	//cout << "test: " << test << endl;
	//refreshDir -> getDirEntries() -> append( test );
	
	refreshDir -> getDirEntries() -> append( ssh -> readLineStdout() );
    }
    
    //cout << "DirectoryView::readSSHStdOut end" << endl;    
}

//------------------------------------------------------
//--------------- isSSHDirExecutable
//------------------------------------------------------
int DirectoryView::isSSHDirExecutable( const QString &dirPath )
{
    //cout << "DirectoryView::isSSHDirExecutable beg" << endl;
    
    if( ssh == 0 ) { return(-37); }
    
    ssh -> clearArguments();
    ssh -> addArgument( QString("ssh") );
    ssh -> addArgument( user + "@" + host );
    ssh -> addArgument( QString("cd ") + dirPath );
    ssh -> start();
    
    //cout << "TEST: " << QString("ssh ") + user + "@" + host + QString(" cd ") + dirPath << endl;
    
    //int count = 0;
    while( ssh -> isRunning() == true ) { /*count++;*/  sleep(10); }
    //cout << "wait end count = " << count << endl;
    
    //cout << "exitStatus: " << ssh -> exitStatus() << endl;
    
    //cout << "DirectoryView::isSSHDirExecutable end" << endl;
    return( ssh -> exitStatus() == 0 );
}
