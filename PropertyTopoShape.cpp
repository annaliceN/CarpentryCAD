/***************************************************************************
 *   Copyright (c) Jürgen Riegel          (juergen.riegel@web.de) 2002     *
 *                                                                         *
 *   This file is part of the FreeCAD CAx development system.              *
 *                                                                         *
 *   This library is free software; you can redistribute it and/or         *
 *   modify it under the terms of the GNU Library General Public           *
 *   License as published by the Free Software Foundation; either          *
 *   version 2 of the License, or (at your option) any later version.      *
 *                                                                         *
 *   This library  is distributed in the hope that it will be useful,      *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU Library General Public License for more details.                  *
 *                                                                         *
 *   You should have received a copy of the GNU Library General Public     *
 *   License along with this library; see the file COPYING.LIB. If not,    *
 *   write to the Free Software Foundation, Inc., 59 Temple Place,         *
 *   Suite 330, Boston, MA  02111-1307, USA                                *
 *                                                                         *
 ***************************************************************************/

#ifndef _PreComp_
# include <sstream>
# include <BRepAdaptor_Curve.hxx>
# include <BRepAdaptor_Surface.hxx>
# include <BRepBndLib.hxx>
# include <BRepBuilderAPI_GTransform.hxx>
# include <Bnd_Box.hxx>
# include <BRepTools.hxx>
# include <BRepTools_ShapeSet.hxx>
# include <BRepBuilderAPI_Copy.hxx>
# include <TopTools_HSequenceOfShape.hxx>
# include <TopTools_MapOfShape.hxx>
# include <TopoDS.hxx>
# include <TopoDS_Iterator.hxx>
# include <TopExp.hxx>
# include <Standard_Failure.hxx>
# include <Standard_Version.hxx>
# include <gp_GTrsf.hxx>
# include <gp_Trsf.hxx>
#endif

#if OCC_VERSION_HEX >= 0x060800
#include <OSD_OpenFile.hxx>
#endif

#include <Exception.h>
#include <FileInfo.h>

#include "PropertyTopoShape.h"

using namespace Part;

TYPESYSTEM_SOURCE(PropertyPartShape, App::Property);

PropertyPartShape::PropertyPartShape()
{
}

PropertyPartShape::~PropertyPartShape()
{
}

void PropertyPartShape::setValue(const TopoShape& sh)
{
    _Shape = sh;
}

void PropertyPartShape::setValue(const TopoDS_Shape& sh)
{
    _Shape.setShape(sh);
}

const TopoDS_Shape& PropertyPartShape::getValue(void)const 
{
    return _Shape.getShape();
}

const TopoShape& PropertyPartShape::getShape() const
{
    return this->_Shape;
}

const Data::ComplexGeoData* PropertyPartShape::getComplexData() const
{
    return &(this->_Shape);
}

Base::BoundBox3d PropertyPartShape::getBoundingBox() const
{
    Base::BoundBox3d box;
    if (_Shape.getShape().IsNull())
        return box;
    try {
        // If the shape is empty an exception may be thrown
        Bnd_Box bounds;
        BRepBndLib::Add(_Shape.getShape(), bounds);
        bounds.SetGap(0.0);
        Standard_Real xMin, yMin, zMin, xMax, yMax, zMax;
        bounds.Get(xMin, yMin, zMin, xMax, yMax, zMax);

        box.MinX = xMin;
        box.MaxX = xMax;
        box.MinY = yMin;
        box.MaxY = yMax;
        box.MinZ = zMin;
        box.MaxZ = zMax;
    }
    catch (Standard_Failure&) {
    }

    return box;
}

void PropertyPartShape::transformGeometry(const Base::Matrix4D &rclTrf)
{
    _Shape.transformGeometry(rclTrf);
}

App::Property *PropertyPartShape::Copy(void) const
{
    PropertyPartShape *prop = new PropertyPartShape();
    prop->_Shape = this->_Shape;
    if (!_Shape.getShape().IsNull()) {
        BRepBuilderAPI_Copy copy(_Shape.getShape());
        prop->_Shape.setShape(copy.Shape());
    }

    return prop;
}

void PropertyPartShape::Paste(const App::Property &from)
{
    aboutToSetValue();
    _Shape = dynamic_cast<const PropertyPartShape&>(from)._Shape;
    hasSetValue();
}

unsigned int PropertyPartShape::getMemSize (void) const
{
    return _Shape.getMemSize();
}

void PropertyPartShape::Save (Base::Writer &writer) const
{
   
}

void PropertyPartShape::Restore(Base::XMLReader &reader)
{
    
}

// The following two functions are copied from OCCT BRepTools.cxx and modified
// to disable saving of triangulation
//
static void BRepTools_Write(const TopoDS_Shape& Sh, Standard_OStream& S) {
  BRepTools_ShapeSet SS(Standard_False);
  // SS.SetProgress(PR);
  SS.Add(Sh);
  SS.Write(S);
  SS.Write(Sh,S);
}
static Standard_Boolean  BRepTools_Write(const TopoDS_Shape& Sh, 
                                   const Standard_CString File)
{
  ofstream os;
#if OCC_VERSION_HEX >= 0x060800
  OSD_OpenStream(os, File, ios::out);
#else
  os.open(File, ios::out);
#endif
  if (!os.rdbuf()->is_open()) return Standard_False;

  Standard_Boolean isGood = (os.good() && !os.eof());
  if(!isGood)
    return isGood;
  
  BRepTools_ShapeSet SS(Standard_False);
  // SS.SetProgress(PR);
  SS.Add(Sh);
  
  os << "DBRep_DrawableShape\n";  // for easy Draw read
  SS.Write(os);
  isGood = os.good();
  if(isGood )
    SS.Write(Sh,os);
  os.flush();
  isGood = os.good();

  errno = 0;
  os.close();
  isGood = os.good() && isGood && !errno;

  return isGood;
}

void PropertyPartShape::SaveDocFile (Base::Writer &writer) const
{
   
}

void PropertyPartShape::RestoreDocFile(Base::Reader &reader)
{
  
}

//**************************************************************************
//**************************************************************************
// PropertyPlacement
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

TYPESYSTEM_SOURCE(Part::PropertyPlacement, App::Property);

//**************************************************************************
// Construction/Destruction


PropertyPlacement::PropertyPlacement()
{

}


PropertyPlacement::~PropertyPlacement()
{

}

//**************************************************************************
// Base class implementer


void PropertyPlacement::setValue(const Base::Placement &pos)
{
	aboutToSetValue();
	_cPos = pos;
	hasSetValue();
}

const Base::Placement & PropertyPlacement::getValue(void)const
{
	return _cPos;
}

void PropertyPlacement::Save(Base::Writer &writer) const
{

}

void PropertyPlacement::Restore(Base::XMLReader &reader)
{
	
}


App::Property *PropertyPlacement::Copy(void) const
{
	PropertyPlacement *p = new PropertyPlacement();
	p->_cPos = _cPos;
	return p;
}

void PropertyPlacement::Paste(const App::Property &from)
{
	aboutToSetValue();
	_cPos = dynamic_cast<const PropertyPlacement&>(from)._cPos;
	hasSetValue();
}