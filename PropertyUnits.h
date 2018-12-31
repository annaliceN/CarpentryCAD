#pragma once

#include "PropertyStandard.h"

namespace Base {
	class Writer;
}


namespace App
{
	// Currently we use millimeter as unit.
	class PropertyQuantityConstraint : public PropertyFloat
	{
		TYPESYSTEM_HEADER();

	public:
		PropertyQuantityConstraint(void) :_ConstStruct(0) {}
		virtual ~PropertyQuantityConstraint() {}

		/// Constraint methods 
		//@{
		/// the boundary struct
		struct Constraints {
			double LowerBound, UpperBound, StepSize;
		};
		/** setting the boundaries
		* This sets the constraint struct. It can be dynamically
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

		virtual const char* getEditorName(void) const;


	protected:
		const Constraints* _ConstStruct;
	};

	class PropertyDistance : public PropertyFloat
	{
		TYPESYSTEM_HEADER();
	public:
		PropertyDistance(void);
		virtual ~PropertyDistance() {}
	};


	class PropertyLength : public PropertyQuantityConstraint
	{
		TYPESYSTEM_HEADER();
	public:
		PropertyLength(void);
		virtual ~PropertyLength() {}
	};


	class PropertyArea : public PropertyQuantityConstraint
	{
		TYPESYSTEM_HEADER();
	public:
		PropertyArea(void);
		virtual ~PropertyArea() {}
	};


	class PropertyVolume : public PropertyQuantityConstraint
	{
		TYPESYSTEM_HEADER();
	public:
		PropertyVolume(void);
		virtual ~PropertyVolume() {}
	};

	class PropertyAngle : public PropertyQuantityConstraint
	{
		TYPESYSTEM_HEADER();
	public:
		PropertyAngle(void);
		virtual ~PropertyAngle() {}
		virtual const char* getEditorName(void) const { return "Gui::PropertyEditor::PropertyAngleItem"; }
	};
}