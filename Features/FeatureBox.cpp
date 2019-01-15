#include "FeatureBox.h"
#include "PropertyContainer.h"
#include "BRepPrimAPI_MakeBox.hxx"
#include "BRepPrimAPI_MakeCylinder.hxx"
#include <Precision.hxx>

namespace Part {
	const App::PropertyQuantityConstraint::Constraints apexRange = { 0.0,90.0,0.1 };
	const App::PropertyQuantityConstraint::Constraints torusRangeV = { -180.0,180.0,1.0 };
	const App::PropertyQuantityConstraint::Constraints angleRangeU = { 0.0,360.0,1.0 };
	const App::PropertyQuantityConstraint::Constraints angleRangeV = { -90.0,90.0,1.0 };
	const App::PropertyQuantityConstraint::Constraints quantityRange = { 0.0,FLT_MAX,0.1 };
}

using namespace Part;

PROPERTY_SOURCE(Part::FeatureBox, Part::FeaturePrimitive);

FeatureBox::FeatureBox()
{
	ADD_PROPERTY_TYPE(Length, (40.0f), "Box", App::Prop_None, "The length of the box");
	ADD_PROPERTY_TYPE(Width, (20.0f), "Box", App::Prop_None, "The width of the box");
	ADD_PROPERTY_TYPE(Height, (100.0f), "Box", App::Prop_None, "The height of the box");
	execute();
}


FeatureBox::~FeatureBox()
{

}


short FeatureBox::mustExecute() const
{
	if (Length.isTouched() ||
		Height.isTouched() ||
		Width.isTouched())
	{
		return 1;
	}
	return FeaturePrimitive::mustExecute();
}

bool FeatureBox::execute(void)
{
	double L = Length.getValue();
	double W = Width.getValue();
	double H = Height.getValue();

	if (L < Precision::Confusion())
	{
		printf_s("Length of box too small");
		return false;
	}

	if (W < Precision::Confusion())
	{
		printf_s("Width of box too small");
		return false;
	}

	if (H < Precision::Confusion())
	{
		printf_s("Height of box too small");
		return false;
	}

	// Build a box using the dimension attributes
	BRepPrimAPI_MakeBox mkBox(L, W, H);
	TopoDS_Shape ResultShape = mkBox.Shape();
	this->Shape.setValue(ResultShape);
	return true;
}


void FeatureBox::onChanged(const App::Property* prop)
{
	if (prop == &Length || prop == &Width || prop == &Height) 
	{
		execute();
	}

	FeaturePrimitive::onChanged(prop);
}


PROPERTY_SOURCE(Part::FeatureCylinder, Part::FeaturePrimitive)

FeatureCylinder::FeatureCylinder(void)
{
	ADD_PROPERTY_TYPE(Radius, (20.0), "Cylinder", App::Prop_None, "The radius of the cylinder");
	ADD_PROPERTY_TYPE(Height, (100.0f), "Cylinder", App::Prop_None, "The height of the cylinder");
	ADD_PROPERTY_TYPE(Angle, (360.0f), "Cylinder", App::Prop_None, "The angle of the cylinder");

	Angle.setConstraints(&angleRangeU);
}

short FeatureCylinder::mustExecute() const
{
	if (Radius.isTouched())
		return 1;
	if (Height.isTouched())
		return 1;
	if (Angle.isTouched())
		return 1;
	return FeaturePrimitive::mustExecute();
}

bool FeatureCylinder::execute(void)
{
	// Build a cylinder
	if (Radius.getValue() < Precision::Confusion())
		return false;
	if (Height.getValue() < Precision::Confusion())
		return false;
	try {
		BRepPrimAPI_MakeCylinder mkCylr(Radius.getValue(),
			Height.getValue(),
			Angle.getValue() / 180.0f*M_PI);
		TopoDS_Shape ResultShape = mkCylr.Shape();
		this->Shape.setValue(ResultShape);
	}
	catch (Standard_Failure& e) {
		return false;
	}

	return FeaturePrimitive::execute();
}
