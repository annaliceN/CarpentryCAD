#include "PHELM.h"

void PHELM::loadMaterialLib()
{
	// Ply wood
	PlyWood pw1(2.0, 2.0, 0.1);
	vecMaterial.push_back(pw1);

	// Lumber wood
	LumberWood lw1(0.1, 0.1, 1.0);
	vecMaterial.push_back(lw1);

}

void PHELM::createShape(MyPrimitive* prim)
{
	vecPrimitive.push_back(*prim);
	
	// AIS_Shape -> Variable Name
	mapAisShapeStr[prim->getGraphicShape()] = getVarName();

	// AIS_Shape -> MyPrimitive*
	mapAisShapePrimtive[prim->getGraphicShape()] = prim;

	const auto graphicShapeName = mapAisShapeStr[prim->getGraphicShape()];
	std::string lineCodeForCAD;
	if (prim->pType == PrimType::Box)
	{
		MyBox* boxPrim = static_cast<MyBox*>(prim);
		lineCodeForCAD = "(box " + graphicShapeName + " " +
			std::to_string(boxPrim->length) + " " + 
			std::to_string(boxPrim->width) + " " +
			std::to_string(boxPrim->height) + ")";

		// push it to the cad code
		CADCode.push_back(lineCodeForCAD);
	}
	else if (prim->pType == PrimType::Cylinder)
	{
		MyCylinder* boxPrim = static_cast<MyCylinder*>(prim);
		lineCodeForCAD = "(cylinder " + graphicShapeName + " " +
			std::to_string(boxPrim->radius) + " " +
			std::to_string(boxPrim->height) + ")";

		// push it to the cad code
		CADCode.push_back(lineCodeForCAD);
	}

	// For debug
	std::cout << lineCodeForCAD;
	emit add_cad_code(lineCodeForCAD);
}

void PHELM::createShape(const Handle(AIS_Shape)& shape)
{

}

void PHELM::transformShape(const Handle(AIS_Shape)& shape, const gp_XYZ& transPart)
{
	std::string lineCodeForCAD;
	lineCodeForCAD = "(translate " + mapAisShapeStr[shape] + " " +
		std::to_string(transPart.X()) + " " +
		std::to_string(transPart.Y()) + " " +
		std::to_string(transPart.Z()) + ")";

	CADCode.push_back(lineCodeForCAD);
	
	// For debug
	std::cout << lineCodeForCAD;
	emit add_cad_code(lineCodeForCAD);
}

void PHELM::fuseTwoShapes(const Handle(AIS_Shape)& shapeA, const Handle(AIS_Shape)& shapeB)
{

}

void PHELM::intersectAwithB(const Handle(AIS_Shape)& shapeA, const Handle(AIS_Shape)& shapeB, const Handle(AIS_Shape)& shapeInt)
{
	std::string lineCodeForCAD;
	lineCodeForCAD = "(cut " + mapAisShapeStr[shapeA] + " " +
		mapAisShapeStr[shapeB] + " " + ")";

	mapAisShapeStr[shapeInt] = mapAisShapeStr[shapeA];

	CADCode.push_back(lineCodeForCAD);

	// For debug
	std::cout << lineCodeForCAD;
	emit add_cad_code(lineCodeForCAD);
		
}

void PHELM::compileToHelm()
{
	
}

std::string PHELM::getVarName()
{
	std::string c;
	c.push_back('a' + curVarIndex);
	++curVarIndex;
	return c;
}
