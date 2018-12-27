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
#endif

#include <Exception.h>

#include "PropertyStandard.h"

using namespace App;
using namespace Base;
using namespace std;


//**************************************************************************
//**************************************************************************
// PropertyInteger
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

TYPESYSTEM_SOURCE(App::PropertyInteger , App::Property);

//**************************************************************************
// Construction/Destruction


PropertyInteger::PropertyInteger()
{
    _lValue = 0;
}


PropertyInteger::~PropertyInteger()
{

}

//**************************************************************************
// Base class implementer


void PropertyInteger::setValue(long lValue)
{
    aboutToSetValue();
    _lValue=lValue;
    hasSetValue();
}

long PropertyInteger::getValue(void) const
{
    return _lValue;
}

void PropertyInteger::Save (Base::Writer &writer) const
{
}

void PropertyInteger::Restore(Base::XMLReader &reader)
{
   
}

Property *PropertyInteger::Copy(void) const
{
    PropertyInteger *p= new PropertyInteger();
    p->_lValue = _lValue;
    return p;
}

void PropertyInteger::Paste(const Property &from)
{
    aboutToSetValue();
    _lValue = dynamic_cast<const PropertyInteger&>(from)._lValue;
    hasSetValue();
}

//**************************************************************************
//**************************************************************************
// PropertyIntegerConstraint
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

TYPESYSTEM_SOURCE(App::PropertyIntegerConstraint, App::PropertyInteger)

//**************************************************************************
// Construction/Destruction


PropertyIntegerConstraint::PropertyIntegerConstraint()
  : _ConstStruct(0)
{

}


PropertyIntegerConstraint::~PropertyIntegerConstraint()
{
    if (_ConstStruct && _ConstStruct->isDeletable())
        delete _ConstStruct;
}

void PropertyIntegerConstraint::setConstraints(const Constraints* sConstrain)
{
    if (_ConstStruct != sConstrain) {
        if (_ConstStruct && _ConstStruct->isDeletable())
            delete _ConstStruct;
    }

    _ConstStruct = sConstrain;
}

const PropertyIntegerConstraint::Constraints*  PropertyIntegerConstraint::getConstraints(void) const
{
    return _ConstStruct;
}

//**************************************************************************
//**************************************************************************
// PropertyPercent
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

TYPESYSTEM_SOURCE(App::PropertyPercent , App::PropertyIntegerConstraint);

const PropertyIntegerConstraint::Constraints percent = {0,100,1};

//**************************************************************************
// Construction/Destruction


PropertyPercent::PropertyPercent()
{
    _ConstStruct = &percent;
}

PropertyPercent::~PropertyPercent()
{
}

//**************************************************************************
//**************************************************************************
// PropertyIntegerList
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

TYPESYSTEM_SOURCE(App::PropertyIntegerList , App::PropertyLists);

//**************************************************************************
// Construction/Destruction


PropertyIntegerList::PropertyIntegerList()
{

}

PropertyIntegerList::~PropertyIntegerList()
{

}

void PropertyIntegerList::setSize(int newSize)
{
    _lValueList.resize(newSize);
}

int PropertyIntegerList::getSize(void) const
{
    return static_cast<int>(_lValueList.size());
}

//**************************************************************************
// Base class implementer

void PropertyIntegerList::setValue(long lValue)
{
    aboutToSetValue();
    _lValueList.resize(1);
    _lValueList[0]=lValue;
    hasSetValue();
}

void PropertyIntegerList::setValues(const std::vector<long>& values)
{
    aboutToSetValue();
    _lValueList = values;
    hasSetValue();
}

void PropertyIntegerList::Save (Base::Writer &writer) const
{
   
}

void PropertyIntegerList::Restore(Base::XMLReader &reader)
{
   
}

Property *PropertyIntegerList::Copy(void) const
{
    PropertyIntegerList *p= new PropertyIntegerList();
    p->_lValueList = _lValueList;
    return p;
}

void PropertyIntegerList::Paste(const Property &from)
{
    aboutToSetValue();
    _lValueList = dynamic_cast<const PropertyIntegerList&>(from)._lValueList;
    hasSetValue();
}

unsigned int PropertyIntegerList::getMemSize (void) const
{
    return static_cast<unsigned int>(_lValueList.size() * sizeof(long));
}




//**************************************************************************
//**************************************************************************
// PropertyIntegerSet
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

TYPESYSTEM_SOURCE(App::PropertyIntegerSet , App::Property);

//**************************************************************************
// Construction/Destruction


PropertyIntegerSet::PropertyIntegerSet()
{

}

PropertyIntegerSet::~PropertyIntegerSet()
{

}


//**************************************************************************
// Base class implementer

void PropertyIntegerSet::setValue(long lValue)
{
    aboutToSetValue();
    _lValueSet.clear();
    _lValueSet.insert(lValue);
    hasSetValue();
}

void PropertyIntegerSet::setValues(const std::set<long>& values)
{
    aboutToSetValue();
    _lValueSet = values;
    hasSetValue();
}

void PropertyIntegerSet::Save (Base::Writer &writer) const
{
   
}

void PropertyIntegerSet::Restore(Base::XMLReader &reader)
{
   
}

Property *PropertyIntegerSet::Copy(void) const
{
    PropertyIntegerSet *p= new PropertyIntegerSet();
    p->_lValueSet = _lValueSet;
    return p;
}

void PropertyIntegerSet::Paste(const Property &from)
{
    aboutToSetValue();
    _lValueSet = dynamic_cast<const PropertyIntegerSet&>(from)._lValueSet;
    hasSetValue();
}

unsigned int PropertyIntegerSet::getMemSize (void) const
{
    return static_cast<unsigned int>(_lValueSet.size() * sizeof(long));
}



//**************************************************************************
//**************************************************************************
// PropertyFloat
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

TYPESYSTEM_SOURCE(App::PropertyFloat , App::Property);

//**************************************************************************
// Construction/Destruction


PropertyFloat::PropertyFloat()
{
    _dValue = 0.0;
}

PropertyFloat::~PropertyFloat()
{

}

//**************************************************************************
// Base class implementer

void PropertyFloat::setValue(double lValue)
{
    aboutToSetValue();
    _dValue=lValue;
    hasSetValue();
}

double PropertyFloat::getValue(void) const
{
    return _dValue;
}

void PropertyFloat::Save (Base::Writer &writer) const
{
   
}

void PropertyFloat::Restore(Base::XMLReader &reader)
{
   
}

Property *PropertyFloat::Copy(void) const
{
    PropertyFloat *p= new PropertyFloat();
    p->_dValue = _dValue;
    return p;
}

void PropertyFloat::Paste(const Property &from)
{
    aboutToSetValue();
    _dValue = dynamic_cast<const PropertyFloat&>(from)._dValue;
    hasSetValue();
}

//**************************************************************************
//**************************************************************************
// PropertyFloatConstraint
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

TYPESYSTEM_SOURCE(App::PropertyFloatConstraint, App::PropertyFloat)

//**************************************************************************
// Construction/Destruction


PropertyFloatConstraint::PropertyFloatConstraint()
  : _ConstStruct(0)
{

}

PropertyFloatConstraint::~PropertyFloatConstraint()
{
    if (_ConstStruct && _ConstStruct->isDeletable())
        delete _ConstStruct;
}

void PropertyFloatConstraint::setConstraints(const Constraints* sConstrain)
{
    if (_ConstStruct != sConstrain) {
        if (_ConstStruct && _ConstStruct->isDeletable())
            delete _ConstStruct;
    }
    _ConstStruct = sConstrain;
}

const PropertyFloatConstraint::Constraints*  PropertyFloatConstraint::getConstraints(void) const
{
    return _ConstStruct;
}

//**************************************************************************
// PropertyPrecision
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

TYPESYSTEM_SOURCE(App::PropertyPrecision, App::PropertyFloatConstraint);

//**************************************************************************
// Construction/Destruction
//
const PropertyFloatConstraint::Constraints PrecisionStandard = {0.0,DBL_MAX,0.001};

PropertyPrecision::PropertyPrecision()
{
    setConstraints(&PrecisionStandard);
}

PropertyPrecision::~PropertyPrecision()
{

}


//**************************************************************************
// PropertyFloatList
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

TYPESYSTEM_SOURCE(App::PropertyFloatList , App::PropertyLists);

//**************************************************************************
// Construction/Destruction


PropertyFloatList::PropertyFloatList()
{

}

PropertyFloatList::~PropertyFloatList()
{

}

//**************************************************************************
// Base class implementer

void PropertyFloatList::setSize(int newSize)
{
    _lValueList.resize(newSize);
}

int PropertyFloatList::getSize(void) const
{
    return static_cast<int>(_lValueList.size());
}

void PropertyFloatList::setValue(double lValue)
{
    aboutToSetValue();
    _lValueList.resize(1);
    _lValueList[0]=lValue;
    hasSetValue();
}

void PropertyFloatList::setValues(const std::vector<double>& values)
{
    aboutToSetValue();
    _lValueList = values;
    hasSetValue();
}

void PropertyFloatList::Save (Base::Writer &writer) const
{
    
}

void PropertyFloatList::Restore(Base::XMLReader &reader)
{
    
}

void PropertyFloatList::SaveDocFile (Base::Writer &writer) const
{
    
}

void PropertyFloatList::RestoreDocFile(Base::Reader &reader)
{
   
}

Property *PropertyFloatList::Copy(void) const
{
    PropertyFloatList *p= new PropertyFloatList();
    p->_lValueList = _lValueList;
    return p;
}

void PropertyFloatList::Paste(const Property &from)
{
    aboutToSetValue();
    _lValueList = dynamic_cast<const PropertyFloatList&>(from)._lValueList;
    hasSetValue();
}

unsigned int PropertyFloatList::getMemSize (void) const
{
    return static_cast<unsigned int>(_lValueList.size() * sizeof(double));
}

//**************************************************************************
//**************************************************************************
// PropertyString
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

TYPESYSTEM_SOURCE(App::PropertyString , App::Property);

PropertyString::PropertyString()
{

}

PropertyString::~PropertyString()
{

}

void PropertyString::setValue(const char* sString)
{
    if (sString) {
        aboutToSetValue();
        _cValue = sString;
        hasSetValue();
    }
}

void PropertyString::setValue(const std::string &sString)
{
    aboutToSetValue();
    _cValue = sString;
    hasSetValue();
}

const char* PropertyString::getValue(void) const
{
    return _cValue.c_str();
}

void PropertyString::Save (Base::Writer &writer) const
{
}

void PropertyString::Restore(Base::XMLReader &reader)
{
}

Property *PropertyString::Copy(void) const
{
    PropertyString *p= new PropertyString();
    p->_cValue = _cValue;
    return p;
}

void PropertyString::Paste(const Property &from)
{
    aboutToSetValue();
    _cValue = dynamic_cast<const PropertyString&>(from)._cValue;
    hasSetValue();
}

unsigned int PropertyString::getMemSize (void) const
{
    return static_cast<unsigned int>(_cValue.size());
}