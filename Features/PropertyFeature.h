#pragma once
#include "Property.h"

namespace Part
{
	class FeaturePrimitive;
	class PropertyFeature : public App::Property
	{
		TYPESYSTEM_HEADER();

	public:
		/**
		* A constructor.
		* A more elaborate description of the constructor.
		*/
		PropertyFeature();

		/**
		* A destructor.
		* A more elaborate description of the destructor.
		*/
		virtual ~PropertyFeature();

		/** Sets the property
		*/
		void setValue(FeaturePrimitive* feature);

		/** This method returns a string representation of the property
		*/
		FeaturePrimitive *getValue(void) const;

		/// Get valid paths for this property; used by auto completer
		const char* getEditorName(void) const {
			return "Gui::PropertyEditor::PropertyPlacementItem";
		}

		virtual void Save(Base::Writer &writer) const;
		virtual void Restore(Base::XMLReader &reader);

		virtual Property *Copy(void) const;
		virtual void Paste(const Property &from);

		virtual unsigned int getMemSize(void) const {
			return 0;
		}

	private:
		FeaturePrimitive* _feature;
	};

}
