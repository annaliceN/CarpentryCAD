#pragma once
#include <FeaturePrimitive.h>
#include <PropertyTopoShape.h>
#include <PropertyFeature.h>
#include "BRepAlgoAPI_BooleanOperation.hxx"

namespace Part
{
	class FeaturePartBoolean : public FeaturePrimitive
	{
		PROPERTY_HEADER(Part::FeaturePartBoolean);
	public:
		FeaturePartBoolean();
		~FeaturePartBoolean();
 
 		PropertyFeature BaseFeature;
 		PropertyFeature ToolFeature;
		PropertyShapeHistory History;

		bool execute(void);
		short mustExecute() const;
		//@}

		/// returns the type name of the ViewProvider
		const char* getViewProviderName(void) const {
			return "PartGui::ViewProviderBoolean";
		}

	protected:
		virtual BRepAlgoAPI_BooleanOperation* makeOperation(const TopoDS_Shape&, const TopoDS_Shape&) const = 0;
	};

}