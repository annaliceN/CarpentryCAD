# include <BRepAlgoAPI_Cut.hxx>

#include "FeaturePartCut.h"

#include <Exception.h>

using namespace Part;

PROPERTY_SOURCE(Part::FeatureCut, Part::FeaturePartBoolean)


FeatureCut::FeatureCut(void)
{

}

BRepAlgoAPI_BooleanOperation* FeatureCut::makeOperation(const TopoDS_Shape& base, const TopoDS_Shape& tool) const
{
    return new BRepAlgoAPI_Cut(base, tool);
}
