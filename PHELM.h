#ifndef _PHELM_H
#define _PHELM_H

#include <vector>
#include <map>
#include <unordered_map>

#include <QObject>

#include <AIS_Shape.hxx>

#include "MaterialLibrary.h"
#include "PrimitiveLibrary.h"

class PHELM : public QObject{
	Q_OBJECT
public:
	PHELM() {
		curVarIndex = 0;
	};

	void loadMaterialLib();

	void createShape(const Handle(AIS_Shape)& shape);

	void createShape(MyPrimitive* prim);

	void transformShape(const Handle(AIS_Shape)& shape, const gp_XYZ& transPart);

	void fuseTwoShapes(const Handle(AIS_Shape)& shapeA, const Handle(AIS_Shape)& shapeB);

	void intersectAwithB(const Handle(AIS_Shape)& shapeA, const Handle(AIS_Shape)& shapeB, const Handle(AIS_Shape)& shapeInt);

	void compileToHelm();

	MyPrimitive* getPrimitiveFromShape(const Handle(AIS_Shape)& shape) { return mapAisShapePrimtive[shape]; };

	std::string getVarName();

signals:
	void add_cad_code(std::string line);

private:
	std::map<Handle(AIS_Shape), std::string> mapAisShapeStr;

	std::map<Handle(AIS_Shape), MyPrimitive*> mapAisShapePrimtive;

	std::vector<Material> vecMaterial;

	std::vector<MyPrimitive> vecPrimitive;

	std::vector<std::string> CADCode;

	std::vector<std::string> HELMCode;
	
	int curVarIndex;
};

#endif // _PHELM_H
