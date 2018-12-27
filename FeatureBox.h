#pragma once
#include "FeaturePrimitive.h"
#include "PropertyStandard.h"

namespace Part
{
	class FeatureBox : public FeaturePrimitive
	{
		PROPERTY_HEADER(Part::FeatureBox);
	public:
		FeatureBox();
		~FeatureBox();

		App::PropertyFloat Length, Height, Width;

		bool execute(void);

		const char* getViewProviderName(void) const {
			return "Box";
		}

		void onChanged(const App::Property* prop);
	};
}
