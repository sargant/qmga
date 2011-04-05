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

#ifndef MAINFORM_INCLUDES_IMP_H
#define MAINFORM_INCLUDES_IMP_H

//--------- my includes
#include "mga_tools.h"
#include "renderer.h"
#include "colorform.h"
#include "modelsform.h"	
#include "sshform.h"
#include "aboutform.h"
#include "sliceform.h"
#include "PovForm.h"

//--------- STL includes
#include <iostream>
#include <iomanip>
#include <fstream>

//--------- QT includes
#include <qtimer.h>
#include <qcolordialog.h>
#include <qmessagebox.h>
#include <qapplication.h>
#include <qfiledialog.h>
#include <qstatusbar.h>
#include <qdockarea.h>
#include <qsettings.h>
#include <dirview.h>

//--------- usings
using mga::Colormap;
//using mga::Molecule;
using mga::MoleculeBiax;

#endif //MAINFORM_INCLUDES_IMP_H
