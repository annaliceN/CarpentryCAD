#include "Sketcher_CommandLine2P.h"

IMPLEMENT_STANDARD_RTTIEXT(Sketcher_CommandLine2P, Sketcher_Command)

/**
* \fn Sketcher_CommandLine2P()
* \brief Constructs a Sketcher_CommandLine2P
*/
Sketcher_CommandLine2P::Sketcher_CommandLine2P()
: Sketcher_Command("Line2P.")
{
	myLine2PAction = Nothing;
	//myContext->Display(myRubberLine, Standard_True);
}


/**
* \fn ~Sketcher_CommandLine2P()
* \brief destructor
*/
Sketcher_CommandLine2P::~Sketcher_CommandLine2P()
{
}

/**
* \fn Action()
* \brief turn command to active state
*/
void Sketcher_CommandLine2P::Action()
{
	myLine2PAction = Input_FirstPointLine;
}

/**
* \fn MouseInputEvent(const gp_Pnt2d& thePnt2d )
* \brief input event handler
* \return Standard_Boolean
* \param thePnt2d const gp_Pnt2d&
*/
Standard_Boolean Sketcher_CommandLine2P::MouseInputEvent(const gp_Pnt2d& thePnt2d)
{
	switch (myLine2PAction)
	{
	case Nothing:	break;

	case Input_FirstPointLine:
		curPnt2d = myAnalyserSnap->MouseInputException(thePnt2d, thePnt2d, Line_FirstPnt, Standard_True);
		myFirstgp_Pnt2d = curPnt2d;
		myFirstPoint->SetPnt(ElCLib::To3d(curCoordinateSystem.Ax2(), curPnt2d));
		myRubberLine->SetPoints(myFirstPoint, myFirstPoint);

		myContext->Display(myRubberLine, 0, -1, Standard_True);
		myLine2PAction = Input_SecondPointLine;
		break;

	case Input_SecondPointLine:
	{
		curPnt2d = myAnalyserSnap->MouseInputException(myFirstgp_Pnt2d, thePnt2d, Line_SecondPnt, Standard_False);

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
				if (!dirEdgeA.IsParallel(dirEdgeB, Precision::Confusion()))
				{
					const double sumDist = vecRefPnts[i].dist + vecRefPnts[j].dist;
					if (sumDist < bestSumDist)
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

		Handle(Geom2d_Edge) newGeom2d_Edge = new Geom2d_Edge();

		if (newGeom2d_Edge->SetPoints(myFirstgp_Pnt2d, curPnt2d))
		{
			Handle(Geom_CartesianPoint) Geom_Point1 = new Geom_CartesianPoint(ElCLib::To3d(curCoordinateSystem.Ax2(), myFirstgp_Pnt2d));
			Handle(Geom_CartesianPoint) Geom_Point2 = new Geom_CartesianPoint(ElCLib::To3d(curCoordinateSystem.Ax2(), curPnt2d));
			Handle(AIS_Line) myAIS_Line = new AIS_Line(Geom_Point1, Geom_Point2);
			myAIS_Line->SetColor(Quantity_NOC_RED);
			AddObject(newGeom2d_Edge, myAIS_Line, LineSketcherObject);

			myContext->Display(myAIS_Line, Standard_True);
			if (myPolylineMode)
			{
				myFirstgp_Pnt2d = curPnt2d;
				myFirstPoint->SetPnt(mySecondPoint->Pnt());
				myRubberLine->SetPoints(myFirstPoint, myFirstPoint);
				myContext->Redisplay(myRubberLine, Standard_True);
			}
			else
			{
				myContext->Remove(myRubberLine, Standard_True);
				myLine2PAction = Input_FirstPointLine;
			}


			// Draw reference lines
			Handle(Prs3d_LineAspect) myDashLineAspect = new Prs3d_LineAspect(Quantity_NOC_GRAY, Aspect_TOL_DASH, 1.0);
			Handle(Geom_CartesianPoint) myGeom_RefPointA = new Geom_CartesianPoint(ElCLib::To3d(curCoordinateSystem.Ax2(), vecRefPnts[findBestRef.first].pnt));
			Handle(AIS_Line) myAIS_RefA = new AIS_Line(Geom_Point2, myGeom_RefPointA);
			myAIS_RefA->Attributes()->SetLineAspect(myDashLineAspect);
			myContext->Display(myAIS_RefA, Standard_True);
			vecRefLines.push_back(myAIS_RefA);

			Handle(Geom_CartesianPoint) myGeom_RefPointB = new Geom_CartesianPoint(ElCLib::To3d(curCoordinateSystem.Ax2(), vecRefPnts[findBestRef.second].pnt));
			Handle(AIS_Line) myAIS_RefB = new AIS_Line(Geom_Point2, myGeom_RefPointB);
			myAIS_RefB->Attributes()->SetLineAspect(myDashLineAspect);
			myContext->Display(myAIS_RefB, Standard_True);
			vecRefLines.push_back(myAIS_RefB);
		}

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
void Sketcher_CommandLine2P::MouseMoveEvent(const gp_Pnt2d& thePnt2d)
{
	switch (myLine2PAction)
	{
	case Nothing:break;

	case Input_FirstPointLine:
		curPnt2d = myAnalyserSnap->MouseMoveException(thePnt2d, thePnt2d, Line_FirstPnt, Standard_True);
		break;
	case Input_SecondPointLine:
		mySecondPoint->SetPnt(ElCLib::To3d(curCoordinateSystem.Ax2(), curPnt2d));
		curPnt2d = myAnalyserSnap->MouseMoveException(myFirstgp_Pnt2d, thePnt2d, Line_SecondPnt, Standard_False);
		myRubberLine->SetPoints(myFirstPoint, mySecondPoint);
		myContext->Redisplay(myRubberLine, Standard_True);
		break;
	default:break;
	}
}


/**
* \fn CancelEvent()
* \brief cancel event handler, stop entering object
* \return void
*/
void Sketcher_CommandLine2P::CancelEvent()
{
	switch (myLine2PAction)
	{
	case Nothing:	break;

	case Input_FirstPointLine: 	break;

	case Input_SecondPointLine:	myContext->Remove(myRubberLine, Standard_True);
		break;
	default: break;
	}
	myLine2PAction = Nothing;
}

/**
* \fn GetTypeOfMethod()
* \brief get command Method
* \return Sketcher_ObjectTypeOfMethod
*/
Sketcher_ObjectTypeOfMethod Sketcher_CommandLine2P::GetTypeOfMethod()
{
	return Line2P_Method;
}


/**
* \fn SetPolylineFirstPnt(const gp_Pnt2d& p1)
* \brief set last entering gp_Pnt2d for polyline
* \return void
* \param p1 const gp_Pnt2d&
*/
void  Sketcher_CommandLine2P::SetPolylineFirstPnt(const gp_Pnt2d& p1)
{
	myFirstgp_Pnt2d = p1;
	myFirstPoint->SetPnt(ElCLib::To3d(curCoordinateSystem.Ax2(), p1));
	myRubberLine->SetPoints(myFirstPoint, myFirstPoint);
	myContext->Display(myRubberLine, 0, -1, Standard_True);
	myLine2PAction = Input_SecondPointLine;
}


/**
* \fn GetPolylineFirstPnt(gp_Pnt2d& p1)
* \brief get last entering gp_Pnt2d for polyline
* \return Standard_Boolean
* \param p1 gp_Pnt2d&
*/
Standard_Boolean  Sketcher_CommandLine2P::GetPolylineFirstPnt(gp_Pnt2d& p1)
{
	if (myLine2PAction == Input_SecondPointLine && myPolylineMode == Standard_True)
	{
		p1 = myFirstgp_Pnt2d;
		return Standard_True;
	}
	else
		return Standard_False;
}


/**
* \fn SetPolylineMode(Standard_Boolean mode)
* \brief set polyline mode
* \return void
* \param mode Standard_Boolean
*/
void Sketcher_CommandLine2P::SetPolylineMode(Standard_Boolean mode)
{
	myPolylineMode = mode;
}