#include "Sketcher_SnapExisitingEdge.h"

IMPLEMENT_STANDARD_RTTIEXT(Sketcher_SnapExistingEdge, Sketcher_Snap)

/**
* \fn Sketcher_SnapExistingEdge()
* \brief Constructs a Sketcher_SnapExistingEdge
*/
Sketcher_SnapExistingEdge::Sketcher_SnapExistingEdge()
{
}


/**
* \fn ~Sketcher_SnapExistingEdge()
* \brief Destructor
*/
Sketcher_SnapExistingEdge::~Sketcher_SnapExistingEdge()
{
}

/**
* \fn SelectEvent()
* \brief find nearest point on curve
* \return void
*/
void Sketcher_SnapExistingEdge::SelectEvent()
{
	findbestPnt2d = Standard_False;
	minDistance = DBL_MAX;

	for (Standard_Integer i = 1; i <= data->Length(); i++)
	{
		mySObject = Handle(Sketcher_Object)::DownCast(data->Value(i));
		myGeometryType = mySObject->GetGeometryType();
		switch (myGeometryType)
		{
		case PointSketcherObject:
		case LineSketcherObject:
		case CircleSketcherObject:
		case ArcSketcherObject: 
			break;
		case ExistingEdgeObject:	
		{
			curGeom2d_Curve = Handle(Geom2d_Curve)::DownCast(mySObject->GetGeometry());
			Handle(Geom2d_Edge) curGeom2d_Edge = Handle(Geom2d_Edge)::DownCast(mySObject->GetGeometry());
			ProjectOnCurve.Init(curPnt2d, curGeom2d_Curve, curGeom2d_Edge->StartParameter(), curGeom2d_Edge->EndParameter());
			if (countProject())
			{
				bestPnt2d = objectPnt2d;
				curHilightedObj = mySObject->GetAIS_Object();
			}
			break;
		}
		default:break;
		}
	}

	if (minDistance == minimumSnapDistance)
		bestPnt2d = curPnt2d;
	else   findbestPnt2d = Standard_True;
}


/**
* \fn GetSnapType()
* \brief get Snap type
* \return Sketcher_SnapType
*/
Sketcher_SnapType Sketcher_SnapExistingEdge::GetSnapType()
{
	return SnapExistingEdge;
}
