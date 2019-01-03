# include "FeatureDrill.h"
# include "PropertyContainer.h"

# include <BRepAlgoAPI_BooleanOperation.hxx>
# include <BRepCheck_Analyzer.hxx>
# include <GEOMAlgo_Splitter.hxx>
# include <Standard_Failure.hxx>
# include <TopoDS_Compound.hxx>
# include <BRepPrimAPI_MakeCylinder.hxx>
# include <BRepBuilderAPI_MakeShape.hxx>
# include <BRepAlgoAPI_Cut.hxx>
# include <memory>

using namespace Part;

PROPERTY_SOURCE_ABSTRACT(Part::FeatureDrill, Part::FeaturePrimitive);

FeatureDrill::FeatureDrill()
{
	ADD_PROPERTY(Nodes, (Base::Vector3d()));
	ADD_PROPERTY(Dir, (Base::Vector3d()));
	ADD_PROPERTY(Radius, (2));
	ADD_PROPERTY(Depth, (10));
	ADD_PROPERTY_TYPE(History, (ShapeHistory()), "Boolean", (App::PropertyType)
		(App::Prop_Output | App::Prop_Transient | App::Prop_Hidden), "Shape history");
	History.setSize(0);
}


FeatureDrill::~FeatureDrill()
{

}

short FeatureDrill::mustExecute() const
{
	if (BaseFeature.getValue()) {
		if (BaseFeature.isTouched() || Depth.isTouched() || Radius.isTouched() || Dir.isTouched())
			return 1;
	}
	return 0;
}

bool FeatureDrill::execute(void)
{
	try {
		Part::FeaturePrimitive *base = dynamic_cast<Part::FeaturePrimitive*>(BaseFeature.getValue());
		TopoDS_Shape BaseShape = base->Shape.getValue();
		if (BaseShape.IsNull())
			throw NullShapeException("Base shape is null");

		const std::vector<Base::Vector3d> nodes = Nodes.getValues();
		gp_Dir dir(Dir.getValue().x, Dir.getValue().y, Dir.getValue().z);

		TopoDS_Compound aCompound;
		BRep_Builder aBuilder;
		aBuilder.MakeCompound(aCompound);

		for (std::vector<Base::Vector3d>::const_iterator it = nodes.begin(); it != nodes.end(); ++it) {
			gp_Pnt pnt(it->x, it->y, it->z);
			gp_Ax2 ax(pnt, dir);
			BRepPrimAPI_MakeCylinder mkCylinder(ax, Radius.getValue(), Depth.getValue());
			aBuilder.Add(aCompound, mkCylinder.Shape());
		}
		
		std::unique_ptr<BRepAlgoAPI_BooleanOperation> mkBool(new BRepAlgoAPI_Cut(BaseShape, aCompound));
		if (!mkBool->IsDone()) {
			return false;
		}
		TopoDS_Shape resShape = mkBool->Shape();
		if (resShape.IsNull()) {
			return false;
		}

		BRepCheck_Analyzer aChecker(resShape);
		if (!aChecker.IsValid()) {
			return false;
		}

		this->Shape.setValue(resShape);
		
		return true;
	}
	catch (...) {
		return false;
	}
}
