/***************************************************************************
*   Copyright (c) Jürgen Riegel          (juergen.riegel@web.de) 2008     *
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


#ifndef PART_PROPERTYTOPOSHAPE_H
#define PART_PROPERTYTOPOSHAPE_H

#include <TopAbs_ShapeEnum.hxx>
#include <map>
#include <vector>

#include "TopoShape.h"
#include "BaseClass.h"
#include "Property.h"

namespace Part
{
	/** The part shape property class.
	* @author Werner Mayer
	*/
	class PropertyPartShape : public App::Property
	{
		TYPESYSTEM_HEADER();

	public:
		PropertyPartShape();
		~PropertyPartShape();

		/** @name Getter/setter */
		//@{
		/// set the part shape
		void setValue(const TopoShape&);
		/// set the part shape
		void setValue(const TopoDS_Shape&);
		/// get the part shape
		const TopoDS_Shape& getValue(void) const;
		const TopoShape& getShape() const;
		const Data::ComplexGeoData* getComplexData() const;
		//@}

		/** @name Modification */
		//@{
		/// Transform the real shape data
		void transformGeometry(const Base::Matrix4D &rclMat);
		//@}

		/** @name Getting basic geometric entities */
		//@{
		/** Returns the bounding box around the underlying mesh kernel */
		Base::BoundBox3d getBoundingBox() const;
		//@}


		/** @name Save/restore */
		//@{
		void Save(Base::Writer &writer) const;
		void Restore(Base::XMLReader &reader);

		void SaveDocFile(Base::Writer &writer) const;
		void RestoreDocFile(Base::Reader &reader);

		App::Property *Copy(void) const;
		void Paste(const App::Property &from);
		unsigned int getMemSize(void) const;
		//@}


	private:
		TopoShape _Shape;
	};

	struct ShapeHistory {
		/**
		* @brief MapList: key is index of subshape (of type 'type') in source
		* shape. Value is list of indexes of subshapes in result shape.
		*/
		typedef std::map<int, std::vector<int> > MapList;
		typedef std::vector<int> List;

		TopAbs_ShapeEnum type;
		MapList shapeMap;
	};


	class PropertyShapeHistory : public App::PropertyLists
	{
		TYPESYSTEM_HEADER();

	public:
		PropertyShapeHistory();
		~PropertyShapeHistory();

		virtual void setSize(int newSize) {
			_lValueList.resize(newSize);
		}
		virtual int getSize(void) const {
			return _lValueList.size();
		}

		/** Sets the property
		*/
		void setValue(const ShapeHistory&);

		void setValues(const std::vector<ShapeHistory>& values);

		const std::vector<ShapeHistory> &getValues(void) const {
			return _lValueList;
		}


		virtual void Save(Base::Writer &writer) const;
		virtual void Restore(Base::XMLReader &reader);

		virtual void SaveDocFile(Base::Writer &writer) const;
		virtual void RestoreDocFile(Base::Reader &reader);

		virtual Property *Copy(void) const;
		virtual void Paste(const Property &from);

		virtual unsigned int getMemSize(void) const {
			return _lValueList.size() * sizeof(ShapeHistory);
		}

	private:
		std::vector<ShapeHistory> _lValueList;
	};

	class PropertyPlacement : public App::Property
	{
		TYPESYSTEM_HEADER();

	public:
		/**
		* A constructor.
		* A more elaborate description of the constructor.
		*/
		PropertyPlacement();

		/**
		* A destructor.
		* A more elaborate description of the destructor.
		*/
		virtual ~PropertyPlacement();

		/** Sets the property
		*/
		void setValue(const Base::Placement &pos);

		/** This method returns a string representation of the property
		*/
		const Base::Placement &getValue(void) const;

		/// Get valid paths for this property; used by auto completer
		const char* getEditorName(void) const {
			return "Gui::PropertyEditor::PropertyPlacementItem";
		}

		virtual void Save(Base::Writer &writer) const;
		virtual void Restore(Base::XMLReader &reader);

		virtual Property *Copy(void) const;
		virtual void Paste(const Property &from);

		virtual unsigned int getMemSize(void) const {
			return sizeof(Base::Placement);
		}

	private:
		Base::Placement _cPos;
	};

} //namespace Part


#endif // PART_PROPERTYTOPOSHAPE_H
