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


#ifndef APP_PROPERTYSTANDARD_H
#define APP_PROPERTYSTANDARD_H

// Std. configurations


#include <string>
#include <list>
#include <vector>

#include "Property.h"

namespace Base {
class Writer;
}


namespace App
{

/** Integer properties
 * This is the father of all properties handling Integers.
 */
class PropertyInteger: public Property
{
    TYPESYSTEM_HEADER();

public:
    PropertyInteger();
    virtual ~PropertyInteger();

    /** Sets the property 
     */
    void setValue(long);

    /** This method returns a string representation of the property
     */
    long getValue(void) const;
    virtual const char* getEditorName(void) const { return "Gui::PropertyEditor::PropertyIntegerItem"; }

    virtual void Save (Base::Writer &writer) const;
    virtual void Restore(Base::XMLReader &reader);

    virtual Property *Copy(void) const;
    virtual void Paste(const Property &from);

    virtual unsigned int getMemSize (void) const{return sizeof(long);}

protected:
    long _lValue;
};

/** Constraint integer properties
 * This property fulfills the need of a constraint integer. It holds basically a 
 * state (integer) and a struct of boundaries. If the boundaries
 * is not set it acts basically like an IntegerProperty and does no checking.
 * The constraints struct can be created on the heap or build in.
 */
class  PropertyIntegerConstraint: public PropertyInteger
{
    TYPESYSTEM_HEADER();

public:
    /// Standard constructor
    PropertyIntegerConstraint();
    
    /// destructor
    virtual ~PropertyIntegerConstraint();

    /// Constraint methods 
    //@{
    /// the boundary struct
    struct Constraints {
        long LowerBound, UpperBound, StepSize;
        Constraints()
            : LowerBound(0)
            , UpperBound(0)
            , StepSize(0)
            , candelete(false)
        {
        }
        Constraints(long l, long u, long s)
            : LowerBound(l)
            , UpperBound(u)
            , StepSize(s)
            , candelete(false)
        {
        }
        ~Constraints()
        {
        }
        void setDeletable(bool on)
        {
            candelete = on;
        }
        bool isDeletable() const
        {
            return candelete;
        }
    private:
        bool candelete;
    };
    /** setting the boundaries
     * This sets the constraint struct. It can be dynamically 
     * allocated or set as a static in the class the property
     * belongs to:
     * \code
     * const Constraints percent = {0,100,1}
     * \endcode
     */
    void setConstraints(const Constraints* sConstraint);
    /// get the constraint struct
    const Constraints*  getConstraints(void) const;
    //@}

    virtual const char* getEditorName(void) const { return "Gui::PropertyEditor::PropertyIntegerConstraintItem"; }

protected:
    const Constraints* _ConstStruct;
};

/** Percent property
 * This property is a special integer property and holds only
 * numbers between 0 and 100.
 */

class  PropertyPercent: public PropertyIntegerConstraint
{
    TYPESYSTEM_HEADER();

public:
    /// Standard constructor
    PropertyPercent();
    
    /// destructor
    virtual ~PropertyPercent();
};

/** Integer list properties
 * 
 */
class  PropertyIntegerList: public PropertyLists
{
    TYPESYSTEM_HEADER();

public:
    /**

     * A constructor.
     * A more elaborate description of the constructor.
     */
    PropertyIntegerList();

    /**
     * A destructor.
     * A more elaborate description of the destructor.
     */
    virtual ~PropertyIntegerList();

    virtual void setSize(int newSize);
    virtual int getSize(void) const;

    /** Sets the property 
     */
    void setValue(long);
  
    /// index operator
    long operator[] (const int idx) const {return _lValueList.operator[] (idx);} 
  
    void  set1Value (const int idx, long value){_lValueList.operator[] (idx) = value;}
    void setValues (const std::vector<long>& values);

    const std::vector<long> &getValues(void) const{return _lValueList;}
    virtual const char* getEditorName(void) const
    { return "Gui::PropertyEditor::PropertyIntegerListItem"; }
	    
    virtual void Save (Base::Writer &writer) const;
    virtual void Restore(Base::XMLReader &reader);
    
    virtual Property *Copy(void) const;
    virtual void Paste(const Property &from);
    virtual unsigned int getMemSize (void) const;

private:
    std::vector<long> _lValueList;
};

/** Integer list properties
 * 
 */
class  PropertyIntegerSet: public Property
{
    TYPESYSTEM_HEADER();

public:
    /**

     * A constructor.
     * A more elaborate description of the constructor.
     */
    PropertyIntegerSet();

    /**
     * A destructor.
     * A more elaborate description of the destructor.
     */
    virtual ~PropertyIntegerSet();

    /** Sets the property 
     */
    void setValue(long);
    void setValue(void){;}
  
    void addValue (long value){_lValueSet.insert(value);}
    void setValues (const std::set<long>& values);

    const std::set<long> &getValues(void) const{return _lValueSet;}

    
    virtual void Save (Base::Writer &writer) const;
    virtual void Restore(Base::XMLReader &reader);
    
    virtual Property *Copy(void) const;
    virtual void Paste(const Property &from);
    virtual unsigned int getMemSize (void) const;

private:
    std::set<long> _lValueSet;
};


/** implements a key/value list as property 
 *  The key ought to be ASCII the Value should be treated as UTF8 to be saved.
 */
class  PropertyMap: public Property
{
    TYPESYSTEM_HEADER();

public:
       
    /**
     * A constructor.
     * A more elaborate description of the constructor.
     */
    PropertyMap();
    
    /**
     * A destructor.
     * A more elaborate description of the destructor.
     */
    virtual ~PropertyMap();

    virtual int getSize(void) const;
    
    /** Sets the property 
     */
    void setValue(void){}
    void setValue(const std::string& key,const std::string& value);
    void setValues(const std::map<std::string,std::string>&);
    
    /// index operator
    const std::string& operator[] (const std::string& key) const ;
    
    void  set1Value (const std::string& key, const std::string& value){_lValueList.operator[] (key) = value;}
    
    const std::map<std::string,std::string> &getValues(void) const{return _lValueList;}
    
    //virtual const char* getEditorName(void) const { return "Gui::PropertyEditor::PropertyStringListItem"; }
        
    virtual void Save (Base::Writer &writer) const;
    virtual void Restore(Base::XMLReader &reader);
    
    virtual Property *Copy(void) const;
    virtual void Paste(const Property &from);
    
    virtual unsigned int getMemSize (void) const;
    

private:
    std::map<std::string,std::string> _lValueList;
};



/** Float properties
 * This is the father of all properties handling floats.
 * Use this type only in rare cases. Mostly you want to 
 * use the more specialized types like e.g. PropertyLength.
 * These properties also fulfill the needs of the unit system.
 * See PropertyUnits.h for all properties with units.
 */
class  PropertyFloat: public Property
{
    TYPESYSTEM_HEADER();

public:
    /** Value Constructor
     *  Construct with explicit Values
     */
    PropertyFloat(void);

    /**
     * A destructor.
     * A more elaborate description of the destructor.
     */
    virtual ~PropertyFloat();


    void setValue(double lValue);
    double getValue(void) const;
    
    virtual const char* getEditorName(void) const { return "Gui::PropertyEditor::PropertyFloatItem"; }
    
    
    virtual void Save (Base::Writer &writer) const;
    virtual void Restore(Base::XMLReader &reader);
    
    virtual Property *Copy(void) const;
    virtual void Paste(const Property &from);
    
    virtual unsigned int getMemSize (void) const{return sizeof(double);}

protected:
    double _dValue;
};

/** Constraint float properties
 * This property fulfills the need of a constraint float. It holds basically a
 * state (float) and a struct of boundaries. If the boundaries
 * is not set it acts basically like a PropertyFloat and does no checking
 * The constraints struct can be created on the heap or built-in.
 */
class  PropertyFloatConstraint: public PropertyFloat
{
    TYPESYSTEM_HEADER();

public:

    /** Value Constructor
     *  Construct with explicit Values
     */
    PropertyFloatConstraint(void);
    
    /**
     * A destructor.
     * A more elaborate description of the destructor.
     */
    virtual ~PropertyFloatConstraint();
    

    /// Constraint methods 
    //@{
    /// the boundary struct
    struct Constraints {
        double LowerBound, UpperBound, StepSize;
        Constraints()
            : LowerBound(0)
            , UpperBound(0)
            , StepSize(0)
            , candelete(false)
        {
        }
        Constraints(double l, double u, double s)
            : LowerBound(l)
            , UpperBound(u)
            , StepSize(s)
            , candelete(false)
        {
        }
        ~Constraints()
        {
        }
        void setDeletable(bool on)
        {
            candelete = on;
        }
        bool isDeletable() const
        {
            return candelete;
        }
    private:
        bool candelete;
    };
    /** setting the boundaries
     * This sets the constraint struct. It can be dynamcly 
     * allocated or set as an static in the class the property
     * blongs to:
     * \code
     * const Constraints percent = {0.0,100.0,1.0}
     * \endcode
     */
    void setConstraints(const Constraints* sConstrain);
    /// get the constraint struct
    const Constraints*  getConstraints(void) const;
    //@}

    virtual const char* getEditorName(void) const
    { return "Gui::PropertyEditor::PropertyFloatConstraintItem"; }


protected:
    const Constraints* _ConstStruct;
};


/** Precision properties
 * This property fulfills the need of a floating value with many decimal points,
 * e.g. for holding values like Precision::Confusion(). The value has a default
 * constraint for non-negative, but can be overridden
 */
class  PropertyPrecision: public PropertyFloatConstraint
{
    TYPESYSTEM_HEADER();
public:
    PropertyPrecision(void);
    virtual ~PropertyPrecision();
    virtual const char* getEditorName(void) const
    { return "Gui::PropertyEditor::PropertyPrecisionItem"; }
};


class  PropertyFloatList: public PropertyLists
{
    TYPESYSTEM_HEADER();

public:

    /**
     * A constructor.
     * A more elaborate description of the constructor.
     */
    PropertyFloatList();
    
    /**
     * A destructor.
     * A more elaborate description of the destructor.
     */
    virtual ~PropertyFloatList();
    
    virtual void setSize(int newSize);
    virtual int getSize(void) const;

    /** Sets the property 
     */
    void setValue(double);

    void setValue (void){}
    
    /// index operator
    double operator[] (const int idx) const {return _lValueList.operator[] (idx);} 
    
    
    void set1Value (const int idx, double value){_lValueList.operator[] (idx) = value;}
    void setValues (const std::vector<double>& values);
    
    const std::vector<double> &getValues(void) const{return _lValueList;}

    virtual const char* getEditorName(void) const
    { return "Gui::PropertyEditor::PropertyFloatListItem"; }

    virtual void Save (Base::Writer &writer) const;
    virtual void Restore(Base::XMLReader &reader);
    
    virtual void SaveDocFile (Base::Writer &writer) const;
    virtual void RestoreDocFile(Base::Reader &reader);
    
    virtual Property *Copy(void) const;
    virtual void Paste(const Property &from);
    virtual unsigned int getMemSize (void) const;

private:
    std::vector<double> _lValueList;
};


/** String properties
 * This is the father of all properties handling Strings.
 */
class  PropertyString: public Property
{
    TYPESYSTEM_HEADER();

public:

    /**
     * A constructor.
     * A more elaborate description of the constructor.
     */
    PropertyString(void);
    
    /**
     * A destructor.
     * A more elaborate description of the destructor.
     */
    virtual ~PropertyString();

    void setValue(const char* sString);
    void setValue(const std::string &sString);
    const char* getValue(void) const;
    const std::string& getStrValue(void) const
    { return _cValue; }
    bool isEmpty(void){return _cValue.empty();}
    
    virtual const char* getEditorName(void) const { return "Gui::PropertyEditor::PropertyStringItem"; }
    
    virtual void Save (Base::Writer &writer) const;
    virtual void Restore(Base::XMLReader &reader);

    virtual Property *Copy(void) const;
    virtual void Paste(const Property &from);
    virtual unsigned int getMemSize (void) const;

private:
    std::string _cValue;
};


} // namespace App

#endif // APP_PROPERTYSTANDARD_H
