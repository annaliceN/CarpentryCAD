# include "FeaturePartBoolean.h"
# include "PropertyContainer.h"

# include <BRepAlgoAPI_BooleanOperation.hxx>
# include <BRepCheck_Analyzer.hxx>
# include <Standard_Failure.hxx>
# include <memory>

using namespace Part;

PROPERTY_SOURCE_ABSTRACT(Part::FeaturePartBoolean, Part::FeaturePrimitive);

FeaturePartBoolean::FeaturePartBoolean()
{
	ADD_PROPERTY(ToolFeature, (0));
	ADD_PROPERTY(BaseFeature, (0));
	ADD_PROPERTY_TYPE(History, (ShapeHistory()), "Boolean", (App::PropertyType)
		(App::Prop_Output | App::Prop_Transient | App::Prop_Hidden), "Shape history");
	History.setSize(0);
}


FeaturePartBoolean::~FeaturePartBoolean()
{

}


short FeaturePartBoolean::mustExecute() const
{
	if (ToolFeature.getValue() && ToolFeature.getValue()) {
		if (BaseFeature.isTouched())
			return 1;
		if (ToolFeature.isTouched())
			return 1;
	}
	return 0;
}


bool FeaturePartBoolean::execute(void)
{
	try {
#if defined(__GNUC__) && defined (FC_OS_LINUX)
		Base::SignalException se;
#endif
		Part::FeaturePrimitive *base = dynamic_cast<Part::FeaturePrimitive*>(BaseFeature.getValue());
		Part::FeaturePrimitive *tool = dynamic_cast<Part::FeaturePrimitive*>(ToolFeature.getValue());

		if (!base || !tool)
			return false;

		// Now, let's get the TopoDS_Shape
		TopoDS_Shape BaseShape = base->Shape.getValue();
		if (BaseShape.IsNull())
			throw NullShapeException("Base shape is null");
		TopoDS_Shape ToolShape = tool->Shape.getValue();
		if (ToolShape.IsNull())
			throw NullShapeException("Tool shape is null");

		std::unique_ptr<BRepAlgoAPI_BooleanOperation> mkBool(makeOperation(BaseShape, ToolShape));
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

		std::vector<ShapeHistory> history;
		history.push_back(buildHistory(*mkBool.get(), TopAbs_FACE, resShape, BaseShape));
		history.push_back(buildHistory(*mkBool.get(), TopAbs_FACE, resShape, ToolShape));

#ifdef REFINE
		if (this->Refine.getValue()) {
			try {
				TopoDS_Shape oldShape = resShape;
				BRepBuilderAPI_RefineModel mkRefine(oldShape);
				resShape = mkRefine.Shape();
				ShapeHistory hist = buildHistory(mkRefine, TopAbs_FACE, resShape, oldShape);
				history[0] = joinHistory(history[0], hist);
				history[1] = joinHistory(history[1], hist);
			}
			catch (Standard_Failure&) {
				// do nothing
			}
		}
#endif
		this->Shape.setValue(resShape);
		this->History.setValues(history);
		return true;
	}
	catch (...) {
		return false;
	}
}
