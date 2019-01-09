#ifndef Sketcher_SnapEXISTINGEDGE_H
#define Sketcher_SnapEXISTINGEDGE_H

#include "Sketcher_Snap.h"
#include <Geom2d_Curve.hxx>
#include "Geom2d_Edge.h"

class Geom2d_Curve;

class Sketcher_SnapExistingEdge;
DEFINE_STANDARD_HANDLE(Sketcher_SnapExistingEdge, Sketcher_Snap)

//snap searching nearest point at line/circle/arc
class Sketcher_SnapExistingEdge : public Sketcher_Snap
{
public:

	/**
	* \fn Sketcher_SnapNearest()
	* \brief Constructs a Sketcher_SnapNearest
	*/
	Sketcher_SnapExistingEdge();

	/**
	* \fn ~Sketcher_SnapExistingEdge()
	* \brief Destructor
	*/
	~Sketcher_SnapExistingEdge();

	/**
	* \fn SelectEvent()
	* \brief find new point
	* \return void
	*/
	void SelectEvent();

	/**
	* \fn GetSnapType()
	* \brief get Snap type
	* \return Sketcher_SnapType
	*/
	Sketcher_SnapType GetSnapType();

	// Type management
	DEFINE_STANDARD_RTTIEXT(Sketcher_SnapExistingEdge, Sketcher_Snap)
private:

	Handle(Geom2d_Curve)	curGeom2d_Curve;

};

#endif