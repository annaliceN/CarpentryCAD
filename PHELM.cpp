#include "PHELM.h"
#include <TopExp_Explorer.hxx>
#include <TopoDS.hxx>
#include <BRepAlgoAPI_Section.hxx>
#include <BRepTools.hxx>
#include <GeomLProp_SLProps.hxx>
#include <gp_Dir.hxx>
#include <BRepBuilderAPI_MakeFace.hxx>

#include "FeaturePolycut.h"
#include "FeatureBezierCurve.h"
#include "FeaturePolyline.h"
#include "FeatureDrill.h"
#include <BRepExtrema_ExtFF.hxx>
#include <BRepExtrema_DistShapeShape.hxx>

void PHELM::LoadChopSaw()
{

}

void PHELM::LoadJigSaw()
{

}

void PHELM::LoadDrillTools()
{
	vecDrillTools.emplace_back(2, 20);
	vecDrillTools.emplace_back(4, 10);
	vecDrillTools.emplace_back(5, 10);
}

void PHELM::LoadTools()
{
	LoadChopSaw();
	LoadJigSaw();
	LoadDrillTools();

	toolLoaded = true;
}

void PHELM::LoadMaterialLib()
{
	// Ply wood
	vecMaterial.push_back(new PlyWood(2.0, 2.0, 0.4));

	// Lumber wood
	vecMaterial.push_back(new LumberWood(20, 20, 160));
	vecMaterial.push_back(new LumberWood(20, 60, 160));
	vecMaterial.push_back(new LumberWood(20, 40, 160));
	vecMaterial.push_back(new LumberWood(10, 40, 160));
	
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
	const double MIN_JIGSAW = 2;
	const double MIN_CHOPSAW = 2;
	
	QString boxHELM;
	double side[3] = { box->Length.getValue(), box->Width.getValue(), box->Height.getValue() };
	std::sort(side, side+3);

	double maxSide = side[2];
	bool solutionFound = false;

	double objVal = -DBL_MAX;
	Material* bestMat;
	
	std::cout << "start compiling box" << std::endl;

	for (auto& mat : vecMaterial)
	{
		if (mat->materialType == MaterialType::LumberMaterial)
		{
			auto lumber = static_cast<LumberWood*>(mat);
			
			if (maxSide - TOOL_EPS < lumber->height)
			{
				if (side[0] - TOOL_EPS < lumber->length && side[1] - TOOL_EPS < lumber->width)
				{
					if (std::abs(side[2] - lumber->height) > TOOL_EPS)
					{
						// Manufacturable constraint (Chopsaw)
						if (side[2] < MIN_CHOPSAW || ((lumber->height - side[2]) < MIN_CHOPSAW && (lumber->height - side[2]) > 1e-2))
						{
							continue;
						}
					}

					// Manufacturable constraint (Jigsaw 1)
					if (std::abs(side[0] - lumber->length) > TOOL_EPS)
					{
						if (side[0] < MIN_JIGSAW ||
							((lumber->length - side[0]) < MIN_JIGSAW && (lumber->length - side[0]) > 1e-2))
						{
							continue;
						}
					}

					// Manufacturable constraint (Jigsaw 2)
					if (std::abs(side[1] - lumber->width) > TOOL_EPS)
					{
						if (side[1] < MIN_JIGSAW ||
							((lumber->width - side[1]) < MIN_JIGSAW && (lumber->width - side[1]) > 1e-2))
						{
							continue;
						}
					}
					
					// Objective value
					double tmpVal = (side[0] * side[1]) / (lumber->length * lumber->width) * (side[2]) / (lumber->height);
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
		boxHELM += "lumber (" + QString::number(lumber->length) + "by" + QString::number(lumber->width) + " @ " + QString::number(lumber->height) + ")\n";
		
		if (std::abs(side[2] - lumber->height) > TOOL_EPS)
			boxHELM += "chopsaw (Z " + QString::number(side[2]) + ")\n";
		
		if (std::abs(side[0] - lumber->length) > TOOL_EPS)
			boxHELM += "jigsaw (line (X " + QString::number(side[0]) + ")\n";
		
		if (std::abs(side[1] - lumber->width) > TOOL_EPS)
			boxHELM += "jigsaw (line (Y " + QString::number(side[1]) + ")\n";
	}
	else
	{
		boxHELM = "The object is too large to be fabricated.\n";
	}

	return boxHELM;
}

QString PHELM::CompileDrill(Part::FeatureDrill* drill)
{
	QString drillHELM;

	std::cout << "start compiling drill" << std::endl;

	bool solutionFound = false;
	auto solution = vecDrillTools.front();

	const double R = drill->Radius.getValue();
	const double D = drill->Depth.getValue();

	for (auto d : vecDrillTools)
	{
		if (std::abs(R - d.first) > TOOL_EPS)
		{
			continue;
		}

		if (std::abs(R - d.second) > TOOL_EPS)
		{
			if (R > d.second) continue;
		}

		solution.first = R;
		solution.second = D;
		solutionFound = true;
	}
	
	if (solutionFound)
	{
		for (auto& pos : drill->Nodes.getValues())
		{
			drillHELM += "drill (" + QString::number(pos.x) + ", " + 
				QString::number(pos.y) + ", " + QString::number(pos.z) + ", R = " +
				QString::number(solution.first) + ", D = " + 
				QString::number(solution.second) +")\n";
		}
	}
	else
	{
		drillHELM += "no matched drill tool to be used\n";
	}

	return drillHELM;
}

QString PHELM::CompilePolycut(Part::FeaturePolyCut* polycut)
{
	QString polyCutHELM;

	std::cout << "start compiling polycut" << std::endl;

	bool solutionFound = false;

	auto polyline = dynamic_cast<Part::FeaturePolyline*>(polycut->ToolFeature.getValue());

	auto& vertices = polyline->Nodes.getValues();

	if (vertices.size() == 2)
	{
		// use chopsaw tool
		auto baseShape = polycut->BaseFeature.getValue()->Shape.getValue();
		Base::Vector3d baseDir = polyline->Dir.getValue();

		gp_Pnt P(vertices[0].x, vertices[0].y, vertices[0].z);
		gp_Dir D1(baseDir.x, baseDir.y, baseDir.z);
		gp_Dir D2(P.X()-vertices[1].x, P.Y()-vertices[1].y, P.Z() - vertices[1].z);

		gp_Dir D = D1.Crossed(D2);
		gp_Pln Plan(P, D);
		TopoDS_Face F = BRepBuilderAPI_MakeFace(Plan);
		//TopoDS_Face F = TopoDS::Face(polyline->Shape.getValue());
		
		std::vector<std::pair<TopoDS_Face, gp_Dir>> candidateFaces;

		// Define the tool
		for (TopExp_Explorer faceExplorer(baseShape, TopAbs_FACE); faceExplorer.More(); faceExplorer.Next())
		{
			// get current face and convert to TopoDS_Face
			const TopoDS_Face& face = TopoDS::Face(faceExplorer.Current());

			BRepAlgoAPI_Section ffsect(face, F, Standard_True);
			ffsect.Approximation(Standard_True);
			ffsect.Build();
			if (ffsect.SectionEdges().Size() > 0)
			{
				Standard_Real umin, umax, vmin, vmax;
				BRepTools::UVBounds(face, umin, umax, vmin, vmax);
				Handle(Geom_Surface) aSurface = BRep_Tool::Surface(face);
				GeomLProp_SLProps props(aSurface, umin, vmin, 1, 0.01);
				gp_Dir normal = props.Normal();
				// std::cout << normal.X() << " " << normal.Y() << " " << normal.Z() << std::endl;
				// std::cout << "dot = " << normal.Dot(D) << std::endl;

				// Approximate
				if (std::fabs(normal.Dot(D)) < 1e-4)
				{
					candidateFaces.push_back(std::make_pair(face, normal));
				}
			}

			// 					GProp_GProps myProps;
			// 					BRepGProp::SurfaceProperties(face, myProps);
			// 					Standard_Real area = myProps.Mass();
			// 					std::cout << "area of the surface is " << area << std::endl;
		}

		std::vector<std::pair<double, double>> foundSolutions;
		for (int i = 0; i < candidateFaces.size(); ++i)
		{
			for (int j = i + 1; j < candidateFaces.size(); ++j)
			{
				for (int k = j + 1; k < candidateFaces.size(); ++k)
				{
					gp_Dir n1 = candidateFaces[i].second;
					gp_Dir n2 = candidateFaces[j].second;
					gp_Dir n3 = candidateFaces[k].second;
					if (n1.Dot(n2) < 1e-4 && n1.Dot(n3) < 1e-4)
					{
						BRepExtrema_ExtFF dss(candidateFaces[j].first, candidateFaces[k].first);
						if (dss.IsDone() && dss.NbExt())
						{
							double dist1 = std::sqrt(dss.SquareDistance(1));

							for (int m = 0; m < candidateFaces.size(); ++m)
							{
								if (m == i || m == j || m == k) continue;
								BRepExtrema_DistShapeShape extrema(candidateFaces[i].first, candidateFaces[m].first);
								if (extrema.IsDone())
								{
									double dist2 = extrema.Value();
									foundSolutions.push_back(std::make_pair(dist1, dist2));
								}
							}
						}
					}
				}
			}
		}

		// see if manufacturable
		bool isManufacturable = false;
		for (auto& s : foundSolutions)
		{
			if (s.first < CHOPSAW_WIDTH_MAX && s.first > CHOPSAW_WIDTH_MIN && s.second < CHOPSAW_HEIGHT)
			{
				isManufacturable = true;
				break;
			}
		}

		if (isManufacturable)
		{
			polyCutHELM += "chopsaw(Line( ["+
				QString::number(vertices[0].x) + "," + 
				QString::number(vertices[0].y) + ", " +
				QString::number(vertices[0].z) +"], [" + 
				QString::number(vertices[1].x) + "," +
				QString::number(vertices[1].y) + ", " +
				QString::number(vertices[1].z) + "])\n";
		}
		else
		{
			polyCutHELM += "chopsaw tool mismatched.\n";
		}
	}
	else
	{
		// Jigsaw
		polyCutHELM += "jigsaw(Line( ";
		for (auto& v : vertices)
		{
			polyCutHELM += "[" + QString::number(v.x) + "," +
				QString::number(v.y) + ", " +
				QString::number(v.z) + "],";
		}
		polyCutHELM +=")\n";
	}

	return polyCutHELM;
}

void PHELM::CompileToHELM()
{
	if (!materialLoaded) LoadMaterialLib();

	if (!toolLoaded) LoadTools();

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
			auto box = dynamic_cast<Part::FeatureBox*>(p);
			compiledHELM += CompileBox(box);
		}
		else if (p->getTypeId() == Part::FeatureDrill::getClassTypeId())
		{
			auto drill = dynamic_cast<Part::FeatureDrill*>(p);
			compiledHELM += CompileDrill(drill);
		}
		else if (p->getTypeId() == Part::FeaturePolyCut::getClassTypeId())
		{
			auto polycut = dynamic_cast<Part::FeaturePolyCut*>(p);
			compiledHELM += CompilePolycut(polycut);
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
