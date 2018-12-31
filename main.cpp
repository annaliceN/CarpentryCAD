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
#include "FeaturePrimitive.h"
#include "FeatureBox.h"


void InitComponents()
{
	Base::BaseClass::init();
	Base::Persistence::init();
	Part::TopoShape::init();
	App::Property::init();
	App::PropertyContainer::init();
	App::PropertyInteger::init();
	App::PropertyIntegerConstraint::init();
	App::PropertyIntegerList::init();
	App::PropertyFloat::init();
	App::PropertyFloatConstraint::init();
	App::PropertyFloatList::init();
	Part::PropertyPlacement::init();
	Part::PropertyPartShape::init();
	Part::PropertyFeature::init();
	Part::PropertyShapeHistory::init();
	Part::FeaturePrimitive::init();
	Part::FeatureBox::init();
}