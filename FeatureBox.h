#pragma once
#include "FeaturePrimitive.h"
#include "PropertyStandard.h"
#include "PropertyUnits.h"

namespace Part
{
	class FeatureBox : public FeaturePrimitive
	{
		PROPERTY_HEADER(Part::FeatureBox);
	public:
		FeatureBox();
		~FeatureBox();

		App::PropertyFloat Length, Height, Width;

		short mustExecute() const;

		bool execute(void);

		const char* getViewProviderName(void) const {
			return "Box";
		}

		void onChanged(const App::Property* prop);
	};


	class FeatureCylinder : public FeaturePrimitive
	{
		PROPERTY_HEADER(Part::Cylinder);

	public:
		FeatureCylinder();

		App::PropertyLength Radius;
		App::PropertyLength Height;
		App::PropertyAngle Angle;

		/** @name methods override feature */
		//@{
		/// recalculate the feature
		bool execute(void);
		short mustExecute() const;
		/// returns the type name of the ViewProvider
		const char* getViewProviderName(void) const {
			return "PartGui::ViewProviderCylinderParametric";
		}
		//@}
	};

}
