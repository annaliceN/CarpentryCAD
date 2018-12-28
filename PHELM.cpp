#include "PHELM.h"

void PHELM::LoadMaterialLib()
{
	// Ply wood
	vecMaterial.push_back(new PlyWood(2.0, 2.0, 0.4));

	// Lumber wood
	vecMaterial.push_back(new LumberWood(2.0, 2.0, 16));
	vecMaterial.push_back(new LumberWood(2.0, 6.0, 16));
	vecMaterial.push_back(new LumberWood(2.0, 4.0, 16));
	vecMaterial.push_back(new LumberWood(1.0, 4.0, 16));
	
	materialLoaded = true;
}

QString PHELM::AssignCreatedShape(Part::FeaturePrimitive* prim)
{
	vecPrimitive.push_back(prim);
	// AIS_Shape -> Variable Name
	mapAisShapeStr[prim->getGraphicShape()] = getVarName();
	auto graphicShapeName = mapAisShapeStr[prim->getGraphicShape()];
	return QString(graphicShapeName.c_str());
}

QString PHELM::CompileBox(Part::FeatureBox* box)
{
	const double MIN_JIGSAW = 0.2;
	const double MIN_CHOPSAW = 0.2;
	QString boxHELM;
	double side[3] = { box->Length.getValue(), box->Width.getValue(), box->Height.getValue() };
	std::sort(side, side+3);

	double maxSide = side[2];
	bool solutionFound = false;

	double objVal = -DBL_MAX;
	Material* bestMat;

	for (auto& mat : vecMaterial)
	{
		if (mat->materialType == MaterialType::LumberMaterial)
		{
			auto lumber = static_cast<LumberWood*>(mat);
			
			if (maxSide < lumber->height)
			{
				if (side[0] < lumber->length && side[1] < lumber->width)
				{
					// Manufacturable constraint (Chopsaw)
					if (side[2] < MIN_CHOPSAW || ((lumber->height - side[2]) < MIN_CHOPSAW && (lumber->height - side[2]) > 1e-2))
					{
						continue;
					}

					// Manufacturable constraint (Jigsaw)
					if (side[0] < MIN_JIGSAW ||
						side[1] < MIN_JIGSAW ||
						((lumber->length - side[0]) < MIN_JIGSAW && (lumber->length - side[0]) > 1e-2) ||
						((lumber->width - side[1]) < MIN_JIGSAW && (lumber->width - side[1]) > 1e-2))
					{
						continue;
					}

					// Objective value
					double tmpVal = (side[0] * side[1]) / (lumber->length * lumber->height) * (side[2]) / (lumber->height);
					if (tmpVal > objVal)
					{
						solutionFound = true;
						objVal = tmpVal;
						bestMat = mat;
					}
				}
			}
			
		}
	}

	if (solutionFound)
	{
		auto lumber = static_cast<LumberWood*>(bestMat);
		boxHELM += "lumber (" + QString::number(lumber->length) + "by" + QString::number(lumber->width) + "@" + QString::number(lumber->height) + ")\n";
		boxHELM += "chopsaw (" + QString::number(side[2]) + ")\n";
		boxHELM += "jigsaw (line (X " + QString::number(side[0]) + ")\n";
		boxHELM += "jigsaw (line (Y " + QString::number(side[1]) + ")\n";
	}
	else
	{
		boxHELM = "The object is too large to be fabricate.";
	}

	return boxHELM;
}

void PHELM::CompileToHELM()
{
	if (!materialLoaded) LoadMaterialLib();

	QString compiledHELM;
	
	if (!materialLoaded)
	{
		compiledHELM = "Error! Please load materials first.";
		return SendHELMCode(compiledHELM);
	}

	for (auto p : vecPrimitive)
	{
		QString varName = QString(mapAisShapeStr[p->getGraphicShape()].c_str());
		
		// Simple case: box
		if (p->getTypeId() == Part::FeatureBox::getClassTypeId())
		{
			auto box = static_cast<Part::FeatureBox*>(p);
			compiledHELM += CompileBox(box);
		}
	}

	return SendHELMCode(compiledHELM);
}

void PHELM::SendHELMCode(QString& helm)
{
	emit sigReplaceHELMCode(helm);
	return;
}

void PHELM::CreateShape(const Handle(AIS_Shape)& shape)
{

}

void PHELM::TransformShape(const Handle(AIS_Shape)& shape, const gp_XYZ& transPart)
{
	std::string lineCodeForCAD;
	lineCodeForCAD = "(translate " + mapAisShapeStr[shape] + " " +
		std::to_string(transPart.X()) + " " +
		std::to_string(transPart.Y()) + " " +
		std::to_string(transPart.Z()) + ")";

	CADCode.push_back(lineCodeForCAD);
	
	// For debug
	std::cout << lineCodeForCAD;
}

void PHELM::FuseTwoShapes(const Handle(AIS_Shape)& shapeA, const Handle(AIS_Shape)& shapeB)
{

}

void PHELM::IntersectAwithB(const Handle(AIS_Shape)& shapeA, const Handle(AIS_Shape)& shapeB, const Handle(AIS_Shape)& shapeInt)
{
	std::string lineCodeForCAD;
	lineCodeForCAD = "(cut " + mapAisShapeStr[shapeA] + " " +
		mapAisShapeStr[shapeB] + " " + ")";

	mapAisShapeStr[shapeInt] = mapAisShapeStr[shapeA];

	CADCode.push_back(lineCodeForCAD);

	// For debug
	std::cout << lineCodeForCAD;
		
}


std::string PHELM::getVarName()
{
	std::string c;
	c.push_back('a' + curVarIndex);
	++curVarIndex;
	return c;
}
