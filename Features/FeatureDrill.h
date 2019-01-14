#pragma once
#include <FeaturePrimitive.h>
#include <PropertyTopoShape.h>
#include <PropertyFeature.h>
#include <BRepAlgoAPI_BooleanOperation.hxx>
#include <PropertyStandard.h>
namespace Part
{
	class FeatureDrill : public FeaturePrimitive
	{
		PROPERTY_HEADER(Part::FeatureDrill);
	public:
		FeatureDrill();
		~FeatureDrill();

		App::PropertyVectorList Nodes;
		App::PropertyVector Dir;
		App::PropertyFloat Radius;
		App::PropertyFloat Depth;
		PropertyFeature BaseFeature;
		PropertyShapeHistory History;

		bool execute(void);
		short mustExecute() const;
		//@}

		/// returns the type name of the ViewProvider
		const char* getViewProviderName(void) const {
			return "PartGui::ViewProviderBoolean";
		}

	};

}