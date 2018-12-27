#ifndef _PHELM_H
#define _PHELM_H

#include <vector>
#include <map>
#include <unordered_map>

#include <QObject>
#include <QTreeWidgetItem>

#include <AIS_Shape.hxx>

#include "MaterialLibrary.h"
#include "PrimitiveLibrary.h"
#include "FeatureBox.h"

class PHELM : public QObject{
	Q_OBJECT
public:
	PHELM(QWidget *parent = 0) {
		curVarIndex = 0;
		materialLoaded = false;

	};

	void LoadMaterialLib();

	void CreateShape(const Handle(AIS_Shape)& shape);

	QString AssignCreatedShape(Part::FeaturePrimitive* prim);

	void TransformShape(const Handle(AIS_Shape)& shape, const gp_XYZ& transPart);

	void FuseTwoShapes(const Handle(AIS_Shape)& shapeA, const Handle(AIS_Shape)& shapeB);

	void IntersectAwithB(const Handle(AIS_Shape)& shapeA, const Handle(AIS_Shape)& shapeB, const Handle(AIS_Shape)& shapeInt);

	void CompileToHELM();

	MyPrimitive* getPrimitiveFromShape(const Handle(AIS_Shape)& shape) { return mapAisShapePrimtive[shape]; };

	std::string getVarName();

	QString CompileBox(Part::FeatureBox * box);

	void SendHELMCode(QString & helm);

signals:
	void sigReplaceHELMCode(QString lines);
	void sigAppendHELMCode(std::string line);

private:
	std::map<Handle(AIS_Shape), std::string> mapAisShapeStr;

	std::map<Handle(AIS_Shape), MyPrimitive*> mapAisShapePrimtive;

	std::vector<Material*> vecMaterial;

	std::vector<Part::FeaturePrimitive*> vecPrimitive;

	std::vector<std::string> CADCode;

	std::vector<std::string> HELMCode;
	
	int curVarIndex;

	bool materialLoaded;
};

#endif // _PHELM_H
