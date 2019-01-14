# include <BRep_Builder.hxx>
# include <BRepBuilderAPI_MakePolygon.hxx>
# include <gp_Pnt.hxx>
# include <TopoDS_Wire.hxx>
# include <BRepOffsetAPI_ThruSections.hxx>
# include <TopoDS.hxx>
# include <TopoDS_Wire.hxx>
# include "FeaturePolyline.h"
# include <Exception.h>

PROPERTY_SOURCE(Part::FeaturePolyline, Part::FeaturePrimitive)


Part::FeaturePolyline::FeaturePolyline()
{
	ADD_PROPERTY(Nodes, (Base::Vector3d()));
	ADD_PROPERTY(Close, (false));
	ADD_PROPERTY(Dir, (Base::Vector3d()));
}

Part::FeaturePolyline::~FeaturePolyline()
{

}

short Part::FeaturePolyline::mustExecute() const
{
	if (Nodes.isTouched() || Close.isTouched() || Dir.isTouched())
		return 1;
	return 0;
}

bool Part::FeaturePolyline::execute(void)
{
	BRepBuilderAPI_MakePolygon poly;
	const std::vector<Base::Vector3d> nodes = Nodes.getValues();

	for (std::vector<Base::Vector3d>::const_iterator it = nodes.begin(); it != nodes.end(); ++it) {
		gp_Pnt pnt(it->x, it->y, it->z);
		poly.Add(pnt);
	}

	if (Close.getValue())
		poly.Close();

	if (!poly.IsDone())
		throw Base::CADKernelError("Cannot create polygon because less than two vertices are given");

	TopoDS_Wire sourceWire = poly.Wire();
	gp_Vec translation(Dir.getValue().x, Dir.getValue().y, Dir.getValue().z);
	gp_Trsf mat;
	mat.SetTranslation(100 * translation);
	TopLoc_Location loc(mat);
	TopoDS_Wire movedSourceWire = TopoDS::Wire(sourceWire.Moved(loc));

	std::list<TopoDS_Wire> list_of_sections;

	// 1
	list_of_sections.push_back(movedSourceWire);
	
	// 2
	list_of_sections.push_back(sourceWire);
	
	// 3
	mat.SetTranslation(-100 * translation);
	loc = TopLoc_Location(mat);
	movedSourceWire = TopoDS::Wire(sourceWire.Moved(loc));
	list_of_sections.push_back(movedSourceWire);

	//make loft
	BRepOffsetAPI_ThruSections mkGenerator( Standard_False, /*ruled=*/Standard_True);
	for (std::list<TopoDS_Wire>::const_iterator it = list_of_sections.begin(); it != list_of_sections.end(); ++it) {
		const TopoDS_Wire &wire = *it;
		mkGenerator.AddWire(wire);
	}
	mkGenerator.Build();

	this->Shape.setValue(mkGenerator.Shape());
	return true;
}
