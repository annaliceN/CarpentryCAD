#include "FeaturePrimitive.h"

# include <gp_Trsf.hxx>
# include <gp_Ax1.hxx>
# include <BRepBuilderAPI_MakeShape.hxx>
# include <BRepAlgoAPI_Fuse.hxx>
# include <BRepAlgoAPI_Common.hxx>
# include <TopTools_ListIteratorOfListOfShape.hxx>
# include <TopExp.hxx>
# include <TopExp_Explorer.hxx>
# include <TopTools_IndexedMapOfShape.hxx>
# include <Standard_Failure.hxx>
# include <TopoDS_Face.hxx>
# include <gp_Dir.hxx>
# include <gp_Pln.hxx> // for Precision::Confusion()
# include <Bnd_Box.hxx>
# include <BRepBndLib.hxx>
# include <BRepExtrema_DistShapeShape.hxx>

using namespace Part;

PROPERTY_SOURCE(Part::FeaturePrimitive, App::PropertyContainer)

FeaturePrimitive::FeaturePrimitive()
{
	Recomputing = true;
}


FeaturePrimitive::~FeaturePrimitive()
{
}

short FeaturePrimitive::mustExecute(void) const
{
	return 0;
}

void Part::FeaturePrimitive::onChanged(const App::Property * prop)
{
	// if the placement has changed apply the change to the point data as well
	if (prop == &this->Placement) {
		TopoShape& shape = const_cast<TopoShape&>(this->Shape.getShape());
		shape.setTransform(this->Placement.getValue().toMatrix());
	}
	// if the point data has changed check and adjust the transformation as well
	else {
		if (this->isRecomputing()) {
			TopoShape& shape = const_cast<TopoShape&>(this->Shape.getShape());
			shape.setTransform(this->Placement.getValue().toMatrix());
		}
		else {
			Base::Placement p;
			// shape must not be null to override the placement
			if (!this->Shape.getValue().IsNull()) {
				p.fromMatrix(this->Shape.getShape().getTransform());
				if (p != this->Placement.getValue())
					this->Placement.setValue(p);
			}
		}
	}

	GraphicShape->SetShape(this->Shape.getValue());
	GraphicShape->Redisplay();
}

TopLoc_Location FeaturePrimitive::getLocation() const
{
	Base::Placement pl = this->Placement.getValue();
	Base::Rotation rot(pl.getRotation());
	Base::Vector3d axis;
	double angle;
	rot.getValue(axis, angle);
	gp_Trsf trf;
	trf.SetRotation(gp_Ax1(gp_Pnt(), gp_Dir(axis.x, axis.y, axis.z)), angle);
	trf.SetTranslationPart(gp_Vec(pl.getPosition().x, pl.getPosition().y, pl.getPosition().z));
	return TopLoc_Location(trf);
}

bool Part::FeaturePrimitive::isRecomputing()
{
	return Recomputing;
}

Handle(AIS_Shape) Part::FeaturePrimitive::BuildGraphicShape()
{
	GraphicShape = new AIS_Shape(Shape.getValue());
	return GraphicShape;
}

Handle(AIS_Shape) Part::FeaturePrimitive::getGraphicShape()
{
	return GraphicShape;
}

ShapeHistory FeaturePrimitive::buildHistory(BRepBuilderAPI_MakeShape& mkShape, TopAbs_ShapeEnum type,
	const TopoDS_Shape& newS, const TopoDS_Shape& oldS)
{
	ShapeHistory history;
	history.type = type;

	TopTools_IndexedMapOfShape newM, oldM;
	TopExp::MapShapes(newS, type, newM); // map containing all old objects of type "type"
	TopExp::MapShapes(oldS, type, oldM); // map containing all new objects of type "type"

										 // Look at all objects in the old shape and try to find the modified object in the new shape
	for (int i = 1; i <= oldM.Extent(); i++) {
		bool found = false;
		TopTools_ListIteratorOfListOfShape it;
		// Find all new objects that are a modification of the old object (e.g. a face was resized)
		for (it.Initialize(mkShape.Modified(oldM(i))); it.More(); it.Next()) {
			found = true;
			for (int j = 1; j <= newM.Extent(); j++) { // one old object might create several new ones!
				if (newM(j).IsPartner(it.Value())) {
					history.shapeMap[i - 1].push_back(j - 1); // adjust indices to start at zero
					break;
				}
			}
		}

		// Find all new objects that were generated from an old object (e.g. a face generated from an edge)
		for (it.Initialize(mkShape.Generated(oldM(i))); it.More(); it.Next()) {
			found = true;
			for (int j = 1; j <= newM.Extent(); j++) {
				if (newM(j).IsPartner(it.Value())) {
					history.shapeMap[i - 1].push_back(j - 1);
					break;
				}
			}
		}

		if (!found) {
			// Find all old objects that don't exist any more (e.g. a face was completely cut away)
			if (mkShape.IsDeleted(oldM(i))) {
				history.shapeMap[i - 1] = std::vector<int>();
			}
			else {
				// Mop up the rest (will this ever be reached?)
				for (int j = 1; j <= newM.Extent(); j++) {
					if (newM(j).IsPartner(oldM(i))) {
						history.shapeMap[i - 1].push_back(j - 1);
						break;
					}
				}
			}
		}
	}

	return history;
}


const char* FeaturePrimitive::getViewProviderName(void) const {
	return "PartGui::ViewProviderPart";
}

ShapeHistory FeaturePrimitive::joinHistory(const ShapeHistory& oldH, const ShapeHistory& newH)
{
	ShapeHistory join;
	join.type = oldH.type;

	for (ShapeHistory::MapList::const_iterator it = oldH.shapeMap.begin(); it != oldH.shapeMap.end(); ++it) {
		int old_shape_index = it->first;
		if (it->second.empty())
			join.shapeMap[old_shape_index] = ShapeHistory::List();
		for (ShapeHistory::List::const_iterator jt = it->second.begin(); jt != it->second.end(); ++jt) {
			ShapeHistory::MapList::const_iterator kt = newH.shapeMap.find(*jt);
			if (kt != newH.shapeMap.end()) {
				ShapeHistory::List& ary = join.shapeMap[old_shape_index];
				ary.insert(ary.end(), kt->second.begin(), kt->second.end());
			}
		}
	}

	return join;
}