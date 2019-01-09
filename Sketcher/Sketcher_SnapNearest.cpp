#include "Sketcher_SnapNearest.h"

IMPLEMENT_STANDARD_RTTIEXT(Sketcher_SnapNearest, Sketcher_Snap)

/**
* \fn Sketcher_SnapNearest()
* \brief Constructs a Sketcher_SnapNearest
*/
Sketcher_SnapNearest::Sketcher_SnapNearest()
{
}


/**
* \fn ~Sketcher_SnapNearest()
* \brief Destructor
*/
Sketcher_SnapNearest::~Sketcher_SnapNearest()
{
}

/**
* \fn SelectEvent()
* \brief find nearest point on curve
* \return void
*/
void Sketcher_SnapNearest::SelectEvent()
{
	Sketcher_ObjectGeometryType bestGeometricType;
	findbestPnt2d = Standard_False;
	minDistance = minimumSnapDistance;

	for (Standard_Integer i = 1; i <= data->Length(); i++)
	{
		mySObject = Handle(Sketcher_Object)::DownCast(data->Value(i));
		myGeometryType = mySObject->GetGeometryType();
		switch (myGeometryType)
		{
		case PointSketcherObject:	break;
		case LineSketcherObject:
		case CircleSketcherObject:
		case ArcSketcherObject: 	
		case ExistingEdgeObject:
			curGeom2d_Curve = Handle(Geom2d_Curve)::DownCast(mySObject->GetGeometry());
			
			if (myGeometryType == LineSketcherObject)
			{
				Handle(Geom2d_Edge) curGeom2d_Edge = Handle(Geom2d_Edge)::DownCast(mySObject->GetGeometry());
				ProjectOnCurve.Init(curPnt2d, curGeom2d_Curve, curGeom2d_Edge->StartParameter(), curGeom2d_Edge->EndParameter());
			}
			else
			{
				ProjectOnCurve.Init(curPnt2d, curGeom2d_Curve);
			}
			if (countProject())
			{
				bestGeometricType = myGeometryType;
				bestPnt2d = objectPnt2d;
				curHilightedObj = mySObject->GetAIS_Object();
			}
			break;
		default:break;
		}
	}

	if (minDistance == minimumSnapDistance)
	{
		bestPnt2d = curPnt2d;
	}
	else
	{
		findbestGeometryType = bestGeometricType;
		findbestPnt2d = Standard_True;
	}
}


/**
* \fn GetSnapType()
* \brief get Snap type
* \return Sketcher_SnapType
*/
Sketcher_SnapType Sketcher_SnapNearest::GetSnapType()
{
	return SnapNearest;
}
