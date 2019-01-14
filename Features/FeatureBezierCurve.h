#ifndef PART_FEATUREBEZIERCURVE_H
#define PART_FEATUREBEZIERCURVE_H


#include "FeaturePrimitive.h"
#include "PropertyStandard.h"
#include "Geom_BezierCurve.hxx"

namespace Part
{
	class FeatureBezierCurve : public Part::FeaturePrimitive
	{
		PROPERTY_HEADER(Part::FeaturePolyline);

	public:
		FeatureBezierCurve();
		virtual ~FeatureBezierCurve();

		Handle(Geom_BezierCurve) BezierCurve;
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
