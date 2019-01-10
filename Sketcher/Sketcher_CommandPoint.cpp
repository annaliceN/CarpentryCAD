#include "Sketcher_CommandPoint.h"

IMPLEMENT_STANDARD_RTTIEXT(Sketcher_CommandPoint, Sketcher_Command)

/**
* \fn Sketcher_CommandPoint()
* \brief Constructs a Sketcher_CommandPoint
*/
Sketcher_CommandPoint::Sketcher_CommandPoint()
: Sketcher_Command("Point.")
{
	myPointAction = Nothing;
}


/**
* \fn ~Sketcher_CommandPoint()
* \brief destructor
*/
Sketcher_CommandPoint::~Sketcher_CommandPoint()
{
}

/**
* \fn Action()
* \brief turn command to active state
*/
void Sketcher_CommandPoint::Action()
{
	myPointAction = Input_Point;
}

/**
* \fn MouseInputEvent(const gp_Pnt2d& thePnt2d )
* \brief input event handler
* \return Standard_Boolean
* \param thePnt2d const gp_Pnt2d&
*/
Standard_Boolean Sketcher_CommandPoint::MouseInputEvent(const gp_Pnt2d& thePnt2d)
{
	curPnt2d = myAnalyserSnap->MouseInput(thePnt2d);
	switch (myPointAction)
	{
	case Nothing:	break;

	case Input_Point:
	{
		// Find a reference system (consists of two non-parallel edges)
		std::vector<RefPnt> vecRefPnts;
		for (Standard_Integer i = 1; i <= data->Length(); i++)
		{
			auto mySObject = Handle(Sketcher_Object)::DownCast(data->Value(i));
			auto myGeometryType = mySObject->GetGeometryType();

			if (myGeometryType == ExistingEdgeObject)
			{
				auto curGeom2d_Edge = Handle(Geom2d_Edge)::DownCast(mySObject->GetGeometry());
				ProjectOnCurve.Init(curPnt2d, curGeom2d_Edge, curGeom2d_Edge->StartParameter(), curGeom2d_Edge->EndParameter());
				if (ProjectOnCurve.NbPoints() > 0)
				{
					vecRefPnts.emplace_back(ProjectOnCurve.NearestPoint(), ProjectOnCurve.LowerDistance(), curGeom2d_Edge);
				}
			}
		}
		
		std::pair<int, int> findBestRef;
		bool hasFindBest = false;
		double bestSumDist = DBL_MAX;

		for (int i = 0; i < vecRefPnts.size(); ++i)
		{
			if (vecRefPnts[i].edge->EndParameter() < 10.0) continue;
			const auto& dirEdgeA = vecRefPnts[i].edge->Direction();
			for (int j = i + 1; j < vecRefPnts.size(); ++j)
			{
				if (vecRefPnts[j].edge->EndParameter() < 10.0) continue;
				const auto& dirEdgeB = vecRefPnts[j].edge->Direction();
				if ( !dirEdgeA.IsParallel(dirEdgeB, Precision::Confusion()) )
				{
					const double sumDist = vecRefPnts[i].dist + vecRefPnts[j].dist;
					if ( sumDist < bestSumDist)
					{
						bestSumDist = sumDist;
						hasFindBest = true;
						findBestRef.first = i; 
						findBestRef.second = j;
					}
				}
			}
		}
		
		if (!hasFindBest) return Standard_False;

		Handle(Geom2d_CartesianPoint) myGeom2d_Point = new Geom2d_CartesianPoint(curPnt2d);
		Handle(Geom_CartesianPoint) myGeom_Point = new Geom_CartesianPoint(ElCLib::To3d(curCoordinateSystem.Ax2(), curPnt2d));
		Handle(AIS_Point) myAIS_Point = new AIS_Point(myGeom_Point);
		myAIS_Point->SetColor(Quantity_NOC_RED);
		myContext->Display(myAIS_Point, Standard_True);

		AddObject(myGeom2d_Point, myAIS_Point, PointSketcherObject);

		Handle(Prs3d_LineAspect) myDashLineAspect = new Prs3d_LineAspect(Quantity_NOC_GRAY, Aspect_TOL_DASH, 1.0);
		Handle(Geom_CartesianPoint) myGeom_RefPointA = new Geom_CartesianPoint(ElCLib::To3d(curCoordinateSystem.Ax2(), vecRefPnts[findBestRef.first].pnt));
		Handle(AIS_Line) myAIS_RefA = new AIS_Line(myGeom_Point, myGeom_RefPointA);
		myAIS_RefA->Attributes()->SetLineAspect(myDashLineAspect);
		myContext->Display(myAIS_RefA, Standard_True);

		Handle(Geom_CartesianPoint) myGeom_RefPointB = new Geom_CartesianPoint(ElCLib::To3d(curCoordinateSystem.Ax2(), vecRefPnts[findBestRef.second].pnt));
		Handle(AIS_Line) myAIS_RefB = new AIS_Line(myGeom_Point, myGeom_RefPointB);
		myAIS_RefB->Attributes()->SetLineAspect(myDashLineAspect);
		myContext->Display(myAIS_RefB, Standard_True);
	}
	break;
	default:break;
	}
	return Standard_False;
}


/**
* \fn MouseMoveEvent(const gp_Pnt2d& thePnt2d )
* \brief mouse move handler
* \return void
* \param thePnt2d const gp_Pnt2d&
*/
void Sketcher_CommandPoint::MouseMoveEvent(const gp_Pnt2d& thePnt2d)
{
	curPnt2d = myAnalyserSnap->MouseMove(thePnt2d);
}


/**
* \fn CancelEvent()
* \brief cancel event handler, stop entering object
* \return void
*/
void Sketcher_CommandPoint::CancelEvent()
{
	myPointAction = Nothing;
}


/**
* \fn GetTypeOfMethod()
* \brief get command Method
* \return Sketcher_ObjectTypeOfMethod
*/
Sketcher_ObjectTypeOfMethod Sketcher_CommandPoint::GetTypeOfMethod()
{
	return Point_Method;
}
