#pragma once

#include "TopoShape.h"
#include <TopoDS_Face.hxx>
#include "PropertyContainer.h"
#include "PropertyTopoShape.h"
#include <AIS_Shape.hxx>

class BRepBuilderAPI_MakeShape;

namespace Part
{
	class FeaturePrimitive : public App::PropertyContainer
	{
		PROPERTY_HEADER(Part::FeaturePrimitive);
	public:
		FeaturePrimitive();
		~FeaturePrimitive();

		/** @name methods override feature */
		//@{
		virtual short mustExecute(void) const;
		//@}

		/// returns the type name of the ViewProvider
		virtual const char* getViewProviderName(void) const;
		virtual void onChanged(const App::Property* prop);
		TopLoc_Location getLocation() const;
		bool isRecomputing();

		Handle(AIS_Shape) BuildGraphicShape();

		Handle(AIS_Shape) getGraphicShape();

		PropertyPlacement* getPlacement() { return &Placement; }

		ShapeHistory buildHistory(BRepBuilderAPI_MakeShape&, TopAbs_ShapeEnum type,
			const TopoDS_Shape& newS, const TopoDS_Shape& oldS);
		ShapeHistory joinHistory(const ShapeHistory&, const ShapeHistory&);

		PropertyPartShape Shape;
		PropertyPlacement Placement;
		Handle(AIS_Shape) GraphicShape;
		bool Recomputing;
	};

}
