#include "PropertyUnits.h"

using namespace App;
using namespace Base;
using namespace std;


const PropertyQuantityConstraint::Constraints LengthStandard = { 0.0, DBL_MAX, 1.0 };
const PropertyQuantityConstraint::Constraints AngleStandard = { -360.0, 360.0, 1.0 };


TYPESYSTEM_SOURCE(App::PropertyQuantityConstraint, App::PropertyFloat);

void PropertyQuantityConstraint::setConstraints(const Constraints* sConstrain)
{
	_ConstStruct = sConstrain;
}

const char* PropertyQuantityConstraint::getEditorName(void) const
{
	return "Gui::PropertyEditor::PropertyUnitConstraintItem";
}


const PropertyQuantityConstraint::Constraints*  PropertyQuantityConstraint::getConstraints(void) const
{
	return _ConstStruct;
}


//**************************************************************************
//**************************************************************************
// PropertyLength
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

TYPESYSTEM_SOURCE(App::PropertyLength, App::PropertyQuantityConstraint);

PropertyLength::PropertyLength()
{
	setConstraints(&LengthStandard);
}

//**************************************************************************
//**************************************************************************
// PropertyArea
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

TYPESYSTEM_SOURCE(App::PropertyArea, App::PropertyQuantityConstraint);

PropertyArea::PropertyArea()
{
	setConstraints(&LengthStandard);
}

//**************************************************************************
//**************************************************************************
// PropertyVolume
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

TYPESYSTEM_SOURCE(App::PropertyVolume, App::PropertyQuantityConstraint);

PropertyVolume::PropertyVolume()
{
	setConstraints(&LengthStandard);
}

//**************************************************************************
//**************************************************************************
// PropertyAngle
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

TYPESYSTEM_SOURCE(App::PropertyAngle, App::PropertyQuantityConstraint);

PropertyAngle::PropertyAngle()
{
	setConstraints(&AngleStandard);
}