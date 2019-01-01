#include "OCCWindow.h"
#include <QApplication>

void InitComponents();

int main(int argc, char *argv[])
{
	InitComponents();
    QApplication a(argc, argv);

    OCCWindow w;
    w.show();

    return a.exec();
}

// Initialize Components
#include "BaseClass.h"
#include "Persistence.h"
#include "TopoShape.h"
#include "PropertyContainer.h"
#include "PropertyFeature.h"
#include "PropertyUnits.h"
#include "FeaturePrimitive.h"
#include "FeatureBox.h"
#include "FeaturePartBoolean.h"
#include "FeaturePartCut.h"

void InitComponents()
{
	Base::Type::init();
	Base::BaseClass::init();
	Base::Exception::init();
	Base::Persistence::init();
	
	// Complex data classes
	Data::ComplexGeoData::init();
	Data::Segment::init();

	Part::TopoShape::init();
	App::Property::init();
	App::PropertyContainer::init();
	App::PropertyLists::init();
	App::PropertyInteger::init();
	App::PropertyIntegerConstraint::init();
	App::PropertyIntegerList::init();
	App::PropertyFloat::init();
	App::PropertyFloatConstraint::init();
	App::PropertyFloatList::init();
	App::PropertyQuantityConstraint::init();
	App::PropertyAngle::init();
	App::PropertyArea::init();
	App::PropertyLength::init();
	App::PropertyVolume::init();
	Part::PropertyPlacement::init();
	Part::PropertyPartShape::init();
	Part::PropertyFeature::init();
	Part::PropertyShapeHistory::init();
	Part::FeaturePrimitive::init();
	Part::FeatureBox::init();
	Part::FeatureCylinder::init();
	Part::FeaturePartBoolean::init();
	Part::FeatureCut::init();
}