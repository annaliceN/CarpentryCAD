#pragma once
#include <FeaturePrimitive.h>
#include <PropertyTopoShape.h>
#include <PropertyFeature.h>
#include <BRepAlgoAPI_BooleanOperation.hxx>
#include <PropertyStandard.h>
namespace Part
{
	class FeaturePolyCut : public FeaturePrimitive
	{
		PROPERTY_HEADER(Part::FeaturePolyCut);
	public:
		FeaturePolyCut();
		~FeaturePolyCut();

		PropertyFeature BaseFeature;
		PropertyFeature ToolFeature;
		App::PropertyBool ReserveDir;
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