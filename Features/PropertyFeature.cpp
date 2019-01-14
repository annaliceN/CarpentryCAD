#include "PropertyFeature.h"

using namespace Part;

//**************************************************************************
//**************************************************************************
// PropertyFeature
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

TYPESYSTEM_SOURCE(Part::PropertyFeature, App::Property);

//**************************************************************************
// Construction/Destruction


PropertyFeature::PropertyFeature()
{

}


PropertyFeature::~PropertyFeature()
{

}

//**************************************************************************
// Base class implementer


void PropertyFeature::setValue(FeaturePrimitive* feature)
{
	aboutToSetValue();
	_feature = feature;
	hasSetValue();
}

FeaturePrimitive * PropertyFeature::getValue(void)const
{
	return _feature;
}

void PropertyFeature::Save(Base::Writer &writer) const
{

}

void PropertyFeature::Restore(Base::XMLReader &reader)
{

}

App::Property *PropertyFeature::Copy(void) const
{
	PropertyFeature *p = new PropertyFeature();
	p->_feature = _feature;
	return p;
}

void PropertyFeature::Paste(const App::Property &from)
{
	aboutToSetValue();
	_feature = dynamic_cast<const PropertyFeature&>(from)._feature;
	hasSetValue();
}