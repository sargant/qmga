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

#ifndef MAINFORM_INCLUDES_DEC_H
#define MAINFORM_INCLUDES_DEC_H

//--------- my includes
#include "mga_tools.h"

//--------- STL includes
#include <vector>

//--------- QT includes

//--------- usings
class SSHDirectory;
class SSHForm;
class ModelsForm;
class Renderer;
class AboutForm;
class SliceForm;
class ColorForm;
class DirectoryView;
class PovrayForm;
class QProcess;

namespace mga
{
  class CnfFile;  
}

using std::vector;
using mga::CnfFile;

#endif //MAINFORM_INCLUDES_DEC_H
