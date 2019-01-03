# include "FeaturePolycut.h"
# include "PropertyContainer.h"

# include <BRepAlgoAPI_BooleanOperation.hxx>
# include <BRepCheck_Analyzer.hxx>
# include <GEOMAlgo_Splitter.hxx>
# include <Standard_Failure.hxx>
# include <memory>

using namespace Part;

PROPERTY_SOURCE_ABSTRACT(Part::FeaturePolyCut, Part::FeaturePrimitive);

FeaturePolyCut::FeaturePolyCut()
{
	ADD_PROPERTY(ToolFeature, (0));
	ADD_PROPERTY(BaseFeature, (0));
	ADD_PROPERTY_TYPE(History, (ShapeHistory()), "Boolean", (App::PropertyType)
		(App::Prop_Output | App::Prop_Transient | App::Prop_Hidden), "Shape history");
	ADD_PROPERTY(ReserveDir, (false));
	History.setSize(0);
}


FeaturePolyCut::~FeaturePolyCut()
{

}

short FeaturePolyCut::mustExecute() const
{
	if (ToolFeature.getValue() && ToolFeature.getValue()) {
		if (BaseFeature.isTouched())
			return 1;
		if (ToolFeature.isTouched())
			return 1;
	}
	return 0;
}

bool FeaturePolyCut::execute(void)
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

		GEOMAlgo_Splitter splitter;
		splitter.AddArgument(BaseShape);
		splitter.AddTool(ToolShape);
		splitter.Perform();
		//creatingPrimitive->shape = splitter.Shape();
		TopTools_ListIteratorOfListOfShape iter(splitter.Modified(BaseShape));
		
		int cnt = 0;
		for (; iter.More(); iter.Next(), ++cnt);
		
		iter = TopTools_ListIteratorOfListOfShape(splitter.Modified(BaseShape));
		if (cnt == 2)
		{
			std::cout << "recomputing" << std::endl;
			for (int i = 0; iter.More(); iter.Next(), ++i)
			{
				if (ReserveDir.getValue() == i)
				{
					std::cout << i << std::endl;
					TopoDS_Shape shape = iter.Value();
					this->Shape.setValue(shape);
				}
			}
		}
		
		if (GraphicShape.IsNull()) BuildGraphicShape();

		return true;
	}
	catch (...) {
		return false;
	}
}
