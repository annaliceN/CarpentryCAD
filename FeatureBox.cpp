#include "FeatureBox.h"
#include "PropertyContainer.h"
#include "BRepPrimAPI_MakeBox.hxx"
#include <Precision.hxx>
using namespace Part;

PROPERTY_SOURCE(Part::FeatureBox, Part::FeaturePrimitive);

FeatureBox::FeatureBox()
{
	ADD_PROPERTY_TYPE(Length, (10.0f), "Box", App::Prop_None, "The length of the box");
	ADD_PROPERTY_TYPE(Width, (10.0f), "Box", App::Prop_None, "The width of the box");
	ADD_PROPERTY_TYPE(Height, (10.0f), "Box", App::Prop_None, "The height of the box");
	execute();
}


FeatureBox::~FeatureBox()
{
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
