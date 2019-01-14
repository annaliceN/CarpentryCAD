#ifndef PART_FEATUREPOLYLINE_H
#define PART_FEATUREPOLYLINE_H


#include "FeaturePrimitive.h"
#include "PropertyStandard.h"

namespace Part
{
	class FeaturePolyline : public Part::FeaturePrimitive
	{
		PROPERTY_HEADER(Part::FeaturePolyline);

	public:
		FeaturePolyline();
		virtual ~FeaturePolyline();

		App::PropertyVectorList Nodes;
		App::PropertyBool       Close;
		App::PropertyVector Dir;

		/** @name methods override Feature */
		//@{
		/// recalculate the Feature
		bool execute(void);
		short mustExecute() const;
		//@}
	};

} //namespace Part

#endif
