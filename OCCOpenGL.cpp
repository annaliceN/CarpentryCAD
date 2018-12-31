#include "OCCOpenGL.h"

#include <QMenu>
#include <QMouseEvent>
#include <QRubberBand>
#include <QStyleFactory>


#include <OpenGl_GraphicDriver.hxx>
#include <V3d_View.hxx>
#include <Aspect_Handle.hxx>
#include <Aspect_DisplayConnection.hxx>

#include <BRepAlgoAPI_Fuse.hxx>
#include <BRepAlgoAPI_Cut.hxx>
#include <BRepBuilderAPI_Transform.hxx>
#include <BRepBuilderAPI_GTransform.hxx>

#include <TopoDS.hxx>

#ifdef WNT
#include <WNT_Window.hxx>
#elif defined(__APPLE__) && !defined(MACOSX_USE_GLX)
#include <Cocoa_Window.hxx>
#else
#undef Bool
#undef CursorShape
#undef None
#undef KeyPress
#undef KeyRelease
#undef FocusIn
#undef FocusOut
#undef FontChange
#undef Expose
#include <Xw_Window.hxx>
#endif


static Handle(Graphic3d_GraphicDriver)& GetGraphicDriver()
{
	static Handle(Graphic3d_GraphicDriver) aGraphicDriver;
	return aGraphicDriver;
}

OCCOpenGL::OCCOpenGL(QWidget* parent)
	: QGLWidget(parent),
	myXmin(0),
	myYmin(0),
	myXmax(0),
	myYmax(0),
	myCurrentMode(CurAction3d_DynamicRotation),
	myDegenerateModeIsOn(Standard_True),
	myRectBand(nullptr),
	interactiveMode(InterMode::Viewing),
	myDrawActions(nullptr),
	GRIDCounter(true)
{
	propertyWidget = new MyPropertyWidget(this);
	objectWidget = new MyObjectWidget(this);
	helm = new PHELM(this);

	// Initialize cursors
	initCursors();

	// No Background
	setBackgroundRole(QPalette::NoRole);

	// Enable the mouse tracking, by default the mouse tracking is disabled.
	setMouseTracking(true);
}

void OCCOpenGL::init()
{
	// Create Aspect_DisplayConnection
	Handle(Aspect_DisplayConnection) aDisplayConnection =
		new Aspect_DisplayConnection();

	// Get graphic driver if it exists, otherwise initialize it
	if (GetGraphicDriver().IsNull())
	{
		GetGraphicDriver() = new OpenGl_GraphicDriver(aDisplayConnection);
	}

	// Get window handle. This returns something suitable for all platforms.
	auto window_handle = (WId)winId();

	// Create appropriate window for platform
#ifdef WNT
	Handle(WNT_Window) wind = new WNT_Window((Aspect_Handle)window_handle);
#elif defined(__APPLE__) && !defined(MACOSX_USE_GLX)
	Handle(Cocoa_Window) wind = new Cocoa_Window((NSView *)window_handle);
#else
	Handle(Xw_Window) wind = new Xw_Window(aDisplayConnection, (Window)window_handle);
#endif

	// Create V3dViewer and V3d_View
	myViewer = new V3d_Viewer(GetGraphicDriver(), Standard_ExtString("viewer3d"));

	myView = myViewer->CreateView();
	
	myView->SetBgGradientColors(Quantity_NOC_SKYBLUE, Quantity_NOC_GRAY, Aspect_GFM_VER);
	myView->SetWindow(wind);
	if (!wind->IsMapped()) wind->Map();

	// Create AISInteractiveContext
	myContext = new AIS_InteractiveContext(myViewer);

	auto * mySG = new Sketcher_QtGUI(parentWidget());
	mySketcher = new Sketcher(myContext, mySG);
	mySketcher->SetSnap(Sketcher_SnapType::SnapAnalyse);
	mySketcher->SetColor(Quantity_NameOfColor::Quantity_NOC_RED);

	// Set up lights etc
	myViewer->SetDefaultLights();
	myViewer->SetLightOn();

	myView->SetBackgroundColor(Quantity_NOC_WHITE);
	myView->MustBeResized();
	myView->SetShadingModel(Graphic3d_TypeOfShadingModel::V3d_PHONG);
	myView->SetZoom(6);
	myView->TriedronDisplay(Aspect_TOTP_LEFT_LOWER, Quantity_NOC_BLACK, 0.08, V3d_ZBUFFER);

	myContext->SetDisplayMode(AIS_Shaded, Standard_True);
	if (myContext->HasOpenedContext())
	{
		myContext->ClearLocalContext();
	}
	myContext->OpenLocalContext();
	myContext->ActivateStandardMode(TopAbs_FACE);

	myContext->DefaultDrawer()->SetFaceBoundaryDraw(Standard_True);
	myContext->DefaultDrawer()->FaceBoundaryAspect()->SetColor(Quantity_NOC_BLACK);
	myContext->UpdateCurrentViewer();

	onGrid();
	//myContext->MainSelector()->SetPickClosest(Standard_False);

	// face selection
	/*
	myContext->CloseAllContexts(Standard_True);
	myContext->OpenLocalContext();
	myContext->ActivateStandardMode(TopAbs_FACE);
	*/
}

QList<QAction *> *OCCOpenGL::getDrawActions() {
	initDrawActions();
	return myDrawActions;
}

void OCCOpenGL::initDrawActions() {
	if (myDrawActions) return;
	myDrawActions = new QList<QAction *>();
	QAction* a;
	a = new QAction(QIcon(":/icon_file/erase"), QString("erase all"), this);
	connect(a, SIGNAL(triggered()), this, SLOT(onErase()));
	myDrawActions->insert(MyEraseActionId, a);
	a->setCheckable(false);
	connect(a, SIGNAL(toggled(bool)), this, SLOT(updateToggled(bool)));

	a = new QAction(QIcon(":/icon_file/delete"), QString("delete selected"), this);
	connect(a, SIGNAL(triggered()), this, SLOT(onDeleteSelected()));
	myDrawActions->insert(MyDeleteActionId, a);
	a->setCheckable(false);
	connect(a, SIGNAL(toggled(bool)), this, SLOT(updateToggled(bool)));

	a = new QAction(QIcon(":/icon_file/property"), QString("view objects property"), this);
	connect(a, SIGNAL(triggered()), this, SLOT(onProperty()));
	a->setCheckable(false);
	connect(a, SIGNAL(toggled(bool)), this, SLOT(updateToggled(bool)));
	myDrawActions->insert(MyPropertyActionId, a);

	a = new QAction(QIcon(":/icon_file/redraw"), QString("redraw objects"), this);
	connect(a, SIGNAL(triggered()), this, SLOT(onRedrawAll()));
	a->setCheckable(false);
	connect(a, SIGNAL(toggled(bool)), this, SLOT(updateToggled(bool)));
	myDrawActions->insert(MyRedrawActionId, a);

	a = new QAction(QIcon(":/icon_file/plane"), QString("change plane"), this);
	connect(a, SIGNAL(triggered()), this, SLOT(onChangePlane()));
	a->setCheckable(false);
	connect(a, SIGNAL(toggled(bool)), this, SLOT(updateToggled(bool)));
	myDrawActions->insert(MyChangePlaneAction, a);

	a = new QAction(QIcon(":/icon_file/grid"), QString("grid"), this);
	connect(a, SIGNAL(triggered()), this, SLOT(onGrid()));
	myDrawActions->insert(MyGridActionId, a);

	a = new QAction(QIcon(":/icon_file/inputPoint"), QString("input points"), this);
	connect(a, SIGNAL(triggered()), this, SLOT(onInputPoints()));
	a->setCheckable(true);
	connect(a, SIGNAL(toggled(bool)), this, SLOT(updateToggled(bool)));
	myDrawActions->insert(MyInputPointAction, a);

	a = new QAction(QIcon(":/icon_file/inputLine"), QString("input line"), this);
	connect(a, SIGNAL(triggered()), this, SLOT(onInputLines()));
	a->setCheckable(true);
	connect(a, SIGNAL(toggled(bool)), this, SLOT(updateToggled(bool)));
	myDrawActions->insert(MyInputLineAction, a);

	a = new QAction(QIcon(":/icon_file/inputCircle"), QString("input circles with radius"), this);
	connect(a, SIGNAL(triggered()), this, SLOT(onInputCircles()));
	a->setCheckable(true);
	connect(a, SIGNAL(toggled(bool)), this, SLOT(updateToggled(bool)));
	myDrawActions->insert(MyInputCircleAction, a);

	a = new QAction(QIcon(":/icon_file/inputCircle3p"), QString("input circle by three points"), this);
	connect(a, SIGNAL(triggered()), this, SLOT(onInputCircles3P()));
	a->setCheckable(true);
	connect(a, SIGNAL(toggled(bool)), this, SLOT(updateToggled(bool)));
	myDrawActions->insert(MyInputCircle3PAction, a);

	a = new QAction(QIcon(":/icon_file/inputCircle2PTan"), QString("input circle by 2p, tangential to 3 curve"), this);
	connect(a, SIGNAL(triggered()), this, SLOT(onInputCircles2PTan()));
	a->setCheckable(true);
	connect(a, SIGNAL(toggled(bool)), this, SLOT(updateToggled(bool)));
	myDrawActions->insert(MyInputCircle2PTanAction, a);

	a = new QAction(QIcon(":/icon_file/inputCircleP2Tan"), QString("input circle by p, 2 tangential to 3 curve"), this);
	connect(a, SIGNAL(triggered()), this, SLOT(onInputCirclesP2Tan()));
	a->setCheckable(true);
	connect(a, SIGNAL(toggled(bool)), this, SLOT(updateToggled(bool)));
	myDrawActions->insert(MyInputCircleP2TanAction, a);

	a = new QAction(QIcon(":/icon_file/inputCircle3tan"), QString("input circle by three tan"), this);
	connect(a, SIGNAL(triggered()), this, SLOT(onInputCircles3Tan()));
	a->setCheckable(true);
	connect(a, SIGNAL(toggled(bool)), this, SLOT(updateToggled(bool)));
	myDrawActions->insert(MyInputCircle3TanAction, a);

	a = new QAction(QIcon(":/icon_file/inputArc3p"), QString("input arc by 3 points"), this);
	connect(a, SIGNAL(triggered()), this, SLOT(onInputArc3P()));
	a->setCheckable(true);
	connect(a, SIGNAL(toggled(bool)), this, SLOT(updateToggled(bool)));
	myDrawActions->insert(MyInputArc3PAction, a);

	a = new QAction(QIcon(":/icon_file/inputArcCenter2p"), QString("input arc by center &2points"), this);
	connect(a, SIGNAL(triggered()), this, SLOT(onInputArcCenter2P()));
	a->setCheckable(true);
	connect(a, SIGNAL(toggled(bool)), this, SLOT(updateToggled(bool)));
	myDrawActions->insert(MyInputArcCenter2PAction, a);

	a = new QAction(QIcon(":/icon_file/inputBezierCurve"), QString("input BezierCurve"), this);
	connect(a, SIGNAL(triggered()), this, SLOT(onInputBezierCurve()));
	a->setCheckable(true);
	connect(a, SIGNAL(toggled(bool)), this, SLOT(updateToggled(bool)));
	myDrawActions->insert(MyInputBezierCurveAction, a);

	a = new QAction(QIcon(":/icon_file/trimcurve"), QString("trim curve"), this);
	connect(a, SIGNAL(triggered()), this, SLOT(onTrimCurve()));
	a->setCheckable(true);
	connect(a, SIGNAL(toggled(bool)), this, SLOT(updateToggled(bool)));
	myDrawActions->insert(MyTrimCurveAction, a);
}


void OCCOpenGL::onDeleteSelected() {
	mySketcher->DeleteSelectedObject();
}

void OCCOpenGL::onProperty() {
	mySketcher->ViewProperties();
}

void OCCOpenGL::onRedrawAll() {
	mySketcher->RedrawAll();
}

void OCCOpenGL::onChangePlane() {
	gp_Dir dir(2, 0, 1);

	gp_Ax3 newgp_Ax3(gp::Origin(), dir);

	mySketcher->SetCoordinateSystem(newgp_Ax3);
}


void OCCOpenGL::onGrid() {

	Handle(V3d_Viewer) aViewer = myView->Viewer();
	if (GRIDCounter)
	{
		aViewer->ActivateGrid(GRID1);
		GRIDCounter = false;
	}
	else
	{
		aViewer->DeactivateGrid();
		//	aViewer->ActivateGrid(GRID2);
		GRIDCounter = true;
	}

	myView->Update();
}

void OCCOpenGL::onInputPoints() {
	mySketcher->ObjectAction(Point_Method);
	myCurrentMode = SketcherAction;
}

void OCCOpenGL::onInputLines() {
	mySketcher->ObjectAction(Line2P_Method);
	myCurrentMode = SketcherAction;
}

void OCCOpenGL::onInputCircles() {
	mySketcher->ObjectAction(CircleCenterRadius_Method);
	myCurrentMode = SketcherAction;
}

void OCCOpenGL::onInputCircles3P() {
	mySketcher->ObjectAction(Circle3P_Method);
	myCurrentMode = SketcherAction;
}

void OCCOpenGL::onInputCircles2PTan() {
	mySketcher->ObjectAction(Circle2PTan_Method);
	myCurrentMode = SketcherAction;
}

void OCCOpenGL::onInputCirclesP2Tan() {
	mySketcher->ObjectAction(CircleP2Tan_Method);
	myCurrentMode = SketcherAction;
}

void OCCOpenGL::onInputCircles3Tan() {
	mySketcher->ObjectAction(Circle3Tan_Method);
	myCurrentMode = SketcherAction;
}

void OCCOpenGL::onInputArc3P() {
	mySketcher->ObjectAction(Arc3P_Method);
	myCurrentMode = SketcherAction;
}

void OCCOpenGL::onInputArcCenter2P() {
	mySketcher->ObjectAction(ArcCenter2P_Method);
	myCurrentMode = SketcherAction;
}

void OCCOpenGL::onInputBezierCurve() {
	mySketcher->ObjectAction(BezierCurve_Method);
	myCurrentMode = SketcherAction;
}

void OCCOpenGL::onTrimCurve() {
	mySketcher->ObjectAction(Trim_Method);
	myCurrentMode = SketcherAction;
}


void OCCOpenGL::ApplyConnections()
{
	//connect(helm, SIGNAL(CreateObject(std::string)), this, SLOT(display_cad_code(std::string)));

}

void OCCOpenGL::paintEvent(QPaintEvent* /*theEvent*/)
{
	if (myContext.IsNull())
	{
		init();
	}

	myView->Redraw();
}

void OCCOpenGL::resizeEvent(QResizeEvent* /*theEvent*/)
{
	if (!myView.IsNull())
	{
		myView->MustBeResized();
	}
}

void OCCOpenGL::fitAll()
{
	myView->FitAll();
	myView->ZFitAll();
	myView->Redraw();
}

void OCCOpenGL::reset()
{
	myView->Reset();
}

void OCCOpenGL::pan()
{
	myCurrentMode = CurAction3d_DynamicPanning;
}

void OCCOpenGL::zoom()
{
	myCurrentMode = CurAction3d_DynamicZooming;
}

void OCCOpenGL::rotate()
{
	myCurrentMode = CurAction3d_DynamicRotation;
}

void OCCOpenGL::mousePressEvent(QMouseEvent* theEvent)
{
	if (theEvent->button() == Qt::LeftButton)
	{
		onLButtonDown((theEvent->buttons() | theEvent->modifiers()), theEvent->pos());
	}
	else if (theEvent->button() == Qt::MidButton)
	{
		onMButtonDown((theEvent->buttons() | theEvent->modifiers()), theEvent->pos());
	}
	else if (theEvent->button() == Qt::RightButton)
	{
		onRButtonDown((theEvent->buttons() | theEvent->modifiers()), theEvent->pos());
	}
}

void OCCOpenGL::mouseReleaseEvent(QMouseEvent* theEvent) {
	if (theEvent->button() == Qt::LeftButton)
	{
		onLButtonUp(theEvent->buttons() | theEvent->modifiers(), theEvent->pos());
	}
	else if (theEvent->button() == Qt::MidButton)
	{
		onMButtonUp(theEvent->buttons() | theEvent->modifiers(), theEvent->pos());
	}
	else if (theEvent->button() == Qt::RightButton)
	{
		onRButtonUp(theEvent->buttons() | theEvent->modifiers(), theEvent->pos());
	}
}

void OCCOpenGL::mouseMoveEvent(QMouseEvent * theEvent)
{
	onMouseMove(theEvent->buttons(), theEvent->pos());
}

void OCCOpenGL::wheelEvent(QWheelEvent * theEvent)
{
	onMouseWheel(theEvent->buttons(), theEvent->delta(), theEvent->pos());
}

void OCCOpenGL::onLButtonDown(const int theFlags, const QPoint thePoint)
{
	// Save the current mouse coordinate in min.
	myXmin = thePoint.x();
	myYmin = thePoint.y();
	myXmax = thePoint.x();
	myYmax = thePoint.y();

	switch (myCurrentMode)
	{
	case CurAction3d_Nothing:
		if (theFlags & Qt::ShiftModifier)
			multiDragEvent(myXmax, myYmax);
		else
			dragEvent(myXmax, myYmax);
		break;
	case CurAction3d_DynamicZooming:
		break;
	case CurAction3d_WindowZooming:
		break;
	case CurAction3d_DynamicPanning:
		break;
	case CurAction3d_GlobalPanning:
		break;
	case SketcherAction:
		myView->Convert(myXmin, myYmin, my_v3dX, my_v3dY, my_v3dZ);
		myView->Proj(projVx, projVy, projVz);
		mySketcher->OnMouseInputEvent(my_v3dX, my_v3dY, my_v3dZ, projVx, projVy, projVz);
		break;
	case CurAction3d_DynamicRotation:
		if (!myDegenerateModeIsOn)
			myView->SetComputedMode(Standard_True);
		myView->StartRotation(thePoint.x(), thePoint.y());
		break;
	default:
		Standard_Failure::Raise("incompatible Current Mode");
		break;
	}
}

void OCCOpenGL::onMButtonDown(const int /*theFlags*/, const QPoint thePoint)
{
	myXmax = thePoint.x();
	myYmax = thePoint.y();
}

void OCCOpenGL::onRButtonDown(const int /*theFlags*/, const QPoint thePoint)
{
	// Save the current mouse coordinate in min.
	myXmin = thePoint.x();
	myYmin = thePoint.y();
	myXmax = thePoint.x();
	myYmax = thePoint.y();

	if (myCurrentMode == CurAction3d_DynamicRotation)
	{
		myView->StartRotation(thePoint.x(), thePoint.y());
	}
}
#include <BRepAdaptor_Surface.hxx>
#include <GeomAPI_ProjectPointOnSurf.hxx>
void OCCOpenGL::onLButtonUp(const int theFlags, const QPoint thePoint)
{
	// Hide the QRubberBand
	if (myRectBand)
	{
		myRectBand->hide();
	}

	if (myCurrentMode == SketcherAction) return;

	// Ctrl for multi selection.
	if (thePoint.x() == myXmin && thePoint.y() == myYmin)
	{
		if (theFlags & Qt::ControlModifier)
		{
			multiInputEvent(thePoint.x(), thePoint.y());
		}
		else
		{
			inputEvent(thePoint.x(), thePoint.y());
		}
	}
	myContext->InitSelected();
	auto numSelected = myContext->NbSelected();
	std::cout << "numSelected = " << numSelected << std::endl;

	if (numSelected == 0)
	{
		propertyWidget->Clear();
		if (!curSelectedFace.IsNull())
		{
			curSelectedFace.Nullify();
		}
	}
	else if (numSelected == 1)
	{
		if (interactiveMode == InterMode::Viewing)
		{
			while (myContext->MoreSelected())
			{
				if (myContext->HasSelectedShape())
				{
					auto selectedInteractiveObj = myContext->SelectedInteractive();
					if (myContext->SelectedShape().ShapeType() == TopAbs_ShapeEnum::TopAbs_FACE)
					{
						curSelectedFace = TopoDS::Face(myContext->SelectedShape());
					}

					/// Property widget
					Handle(AIS_Shape) selectedShape = Handle(AIS_Shape)::DownCast(myContext->SelectedInteractive());
					Part::FeaturePrimitive* selectedPrim = primMapping[selectedShape];

					/// Drawing property list
					if (selectedPrim != nullptr)
					{
						propertyWidget->WritePropertiesToPropWidget(selectedPrim);
						std::cout << selectedPrim->getViewProviderName() << std::endl;
					}
				}
				myContext->NextSelected();
			}
		}
	}
}


void OCCOpenGL::actionStart2dSketch()
{
	if (curSelectedFace.IsNull()) return;
	myCurrentMode = SketcherAction;

	bool Reverse = false;
	if (curSelectedFace.Orientation() == TopAbs_REVERSED)
		Reverse = true;
	/// compute mid point of the face
	myView->Convert(myXmin, myYmin, my_v3dX, my_v3dY, my_v3dZ);
	myView->Proj(projVx, projVy, projVz);
	gp_Pnt ObjOrg(projVx, projVy, projVz);
	/// compute gp_ax3
	BRepAdaptor_Surface adapt(curSelectedFace);
	gp_Pln plane = adapt.Plane();
	Standard_Boolean ok = plane.Direct();
	if (!ok) {
		// toggle if plane has a left-handed coordinate system
		plane.UReverse();
		Reverse = !Reverse;

	}

	gp_Ax1 Normal = plane.Axis();
	if (Reverse)
		Normal.Reverse();

	Handle(Geom_Plane) gPlane = new Geom_Plane(plane);
	GeomAPI_ProjectPointOnSurf projector(ObjOrg, gPlane);
	gp_Pnt SketchBasePoint = projector.NearestPoint();

	gp_Dir dir = Normal.Direction();
	gp_Ax3 SketchPos;

	double cosNX = dir.Dot(gp::DX());
	double cosNY = dir.Dot(gp::DY());
	double cosNZ = dir.Dot(gp::DZ());
	std::vector<double> cosXYZ;
	cosXYZ.push_back(fabs(cosNX));
	cosXYZ.push_back(fabs(cosNY));
	cosXYZ.push_back(fabs(cosNZ));

	int pos = std::max_element(cosXYZ.begin(), cosXYZ.end()) - cosXYZ.begin();

	// +X/-X
	if (pos == 0) {
		if (cosNX > 0)
			SketchPos = gp_Ax3(SketchBasePoint, dir, gp_Dir(0, 1, 0));
		else
			SketchPos = gp_Ax3(SketchBasePoint, dir, gp_Dir(0, -1, 0));

	}
	// +Y/-Y
	else if (pos == 1) {
		if (cosNY > 0)
			SketchPos = gp_Ax3(SketchBasePoint, dir, gp_Dir(-1, 0, 0));
		else
			SketchPos = gp_Ax3(SketchBasePoint, dir, gp_Dir(1, 0, 0));

	}
	// +Z/-Z
	else {
		SketchPos = gp_Ax3(SketchBasePoint, dir, gp_Dir(1, 0, 0));
	}

	/// set coordinate system
	mySketcher->SetCoordinateSystem(SketchPos);
	myPreviousCam = new Graphic3d_Camera;
	myPreviousCam->Copy(myView->Camera());
	myView->Camera()->SetUp(SketchPos.YDirection());
	myView->Camera()->SetDirection(-dir);
	myView->Redraw();

}

void OCCOpenGL::actionComplete2dSketch()
{

	/// stop sketching
	actionStop2dSketch();
}

void OCCOpenGL::actionStop2dSketch()
{
	myCurrentMode = CurAction3d_Nothing;
	activateCursor(myCurrentMode);
	myView->SetCamera(myPreviousCam);
	myView->Redraw();
}

void OCCOpenGL::actionLineCutting()
{
	actionStart2dSketch();
	onInputLines();
	activateCursor(SketcherAction);
}

void OCCOpenGL::actionCurveCutting()
{
	actionStart2dSketch();
	onInputBezierCurve();
	activateCursor(SketcherAction);
}

void OCCOpenGL::initCursors() {
	if (!defCursor)
		defCursor = new QCursor(Qt::ArrowCursor);
	if (!handCursor)
		handCursor = new QCursor(Qt::PointingHandCursor);
	if (!panCursor)
		panCursor = new QCursor(Qt::SizeAllCursor);
	if (!globPanCursor)
		globPanCursor = new QCursor(Qt::CrossCursor);
}

void OCCOpenGL::activateCursor(const CurrentAction3d mode) {
	switch (mode)
	{
	case CurAction3d_DynamicPanning:
		setCursor(*panCursor);
		break;
	case CurAction3d_DynamicZooming:
		setCursor(*zoomCursor);
		break;
	case CurAction3d_DynamicRotation:
		setCursor(*rotCursor);
		break;
	case CurAction3d_GlobalPanning:
		setCursor(*globPanCursor);
		break;
	case CurAction3d_WindowZooming:
		setCursor(*handCursor);
		break;
	case SketcherAction:
		setCursor(*globPanCursor);
		break;
	case CurAction3d_Nothing:
	default:
		setCursor(*defCursor);
		break;
	}
}

void OCCOpenGL::onMButtonUp(const int /*theFlags*/, const QPoint thePoint)
{

	if (thePoint.x() == myXmin && thePoint.y() == myYmin)
	{
		panByMiddleButton(thePoint);
	}

}

void OCCOpenGL::onRButtonUp(const int /*theFlags*/, const QPoint thePoint)
{
	QMenu menu;

	if (pairParaShape.second)
	{
		QAction* paramAct = new QAction("Set its parameters", this);
		menu.addAction(paramAct);
	}
	QAction* openAct = new QAction("Open...", this);
	menu.addAction(openAct);
	menu.addSeparator();

	if (!curSelectedFace.IsNull())
	{
		openAct = new QAction("Sketch lines on the face", this);
		connect(openAct, &QAction::triggered, [=]() {
			
			actionLineCutting();
		});
		menu.addAction(openAct);

		openAct = new QAction("Sketch curves on the face", this);
		connect(openAct, &QAction::triggered, [=]() {

			actionCurveCutting();
		});
		menu.addAction(openAct);

		if (myCurrentMode == SketcherAction)
		{
			openAct = new QAction("Complete sketching", this);
			connect(openAct, &QAction::triggered, [=]() {
				actionComplete2dSketch();
			});
			menu.addAction(openAct);

			openAct = new QAction("Stop sketching", this);
			connect(openAct, &QAction::triggered, [=]() {
				actionStop2dSketch();
			});
			menu.addAction(openAct);
		}
	}

	menu.addSeparator();
	menu.exec(mapToGlobal(thePoint));
}

void OCCOpenGL::onMouseMove(const int theFlags, const QPoint thePoint)
{
	// Draw the rubber band.
	if (theFlags & Qt::LeftButton)
	{
		if (interactiveMode == InterMode::Manipulating && aManipulator->HasActiveMode())
		{
			manipulatorTrsf = aManipulator->Transform(thePoint.x(), thePoint.y(), myView);
			myView->Redraw();
		}
		else
		{
			drawRubberBand(myXmin, myYmin, thePoint.x(), thePoint.y());

			dragEvent(thePoint.x(), thePoint.y());

		}
	}

	// Ctrl for multi selection.
	if (theFlags & Qt::ControlModifier)
	{
		multiMoveEvent(thePoint.x(), thePoint.y());
	}
	else
	{
		moveEvent(thePoint.x(), thePoint.y());
	}

	// Middle button.
	if (theFlags & Qt::RightButton)
	{
		switch (myCurrentMode)
		{
		case CurAction3d_DynamicRotation:
			myView->Rotation(thePoint.x(), thePoint.y());
			break;

		case CurAction3d_DynamicZooming:
			myView->Zoom(myXmin, myYmin, thePoint.x(), thePoint.y());
			break;

		case CurAction3d_DynamicPanning:
			myView->Pan(thePoint.x() - myXmax, myYmax - thePoint.y());
			myXmax = thePoint.x();
			myYmax = thePoint.y();
			break;

		default:
			break;
		}
	}

	if (theFlags & Qt::MiddleButton)
	{
		myView->Pan(thePoint.x() - myXmax, myYmax - thePoint.y());
		myXmax = thePoint.x();
		myYmax = thePoint.y();
	}

	if (myCurrentMode == SketcherAction)
	{
		myView->Convert(thePoint.x(), thePoint.y(), my_v3dX, my_v3dY, my_v3dZ);
		myView->Proj(projVx, projVy, projVz);
		mySketcher->OnMouseMoveEvent(my_v3dX, my_v3dY, my_v3dZ, projVx, projVy, projVz);
		//mySketcher->OnMouseMoveEvent(thePoint.x(), thePoint.y());
	}

}


void OCCOpenGL::onMouseWheel(const int /*theFlags*/, const int theDelta, const QPoint thePoint)
{
	Standard_Integer aFactor = 16;

	Standard_Integer aX = thePoint.x();
	Standard_Integer aY = thePoint.y();

	if (theDelta > 0)
	{
		aX += aFactor;
		aY += aFactor;
	}
	else
	{
		aX -= aFactor;
		aY -= aFactor;
	}

	myView->Zoom(thePoint.x(), thePoint.y(), aX, aY);
}

void OCCOpenGL::addItemInPopup(QMenu* /*theMenu*/)
{

}


void OCCOpenGL::dragEvent(const int x, const int y)
{
	myContext->Select(myXmin, myYmin, x, y, myView, Standard_True);

	emit selectionChanged();
}

void OCCOpenGL::multiDragEvent(const int x, const int y)
{
	myContext->ShiftSelect(myXmin, myYmin, x, y, myView, Standard_True);

	emit selectionChanged();

}

void OCCOpenGL::inputEvent(const int x, const int y)
{
	Q_UNUSED(x);
	Q_UNUSED(y);
	myContext->Select(Standard_True);


	emit selectionChanged();
}

void OCCOpenGL::multiInputEvent(const int x, const int y)
{
	Q_UNUSED(x);
	Q_UNUSED(y);

	myContext->ShiftSelect(Standard_True);

	emit selectionChanged();
}

void OCCOpenGL::moveEvent(const int x, const int y)
{

	myContext->MoveTo(x, y, myView, Standard_True);

}

void OCCOpenGL::multiMoveEvent(const int x, const int y)
{
	myContext->MoveTo(x, y, myView, Standard_True);
}

void OCCOpenGL::drawRubberBand(const int minX, const int minY, const int maxX, const int maxY)
{
	QRect aRect;

	// Set the rectangle correctly.
	(minX < maxX) ? (aRect.setX(minX)) : (aRect.setX(maxX));
	(minY < maxY) ? (aRect.setY(minY)) : (aRect.setY(maxY));

	aRect.setWidth(abs(maxX - minX));
	aRect.setHeight(abs(maxY - minY));

	if (!myRectBand)
	{
		myRectBand = new QRubberBand(QRubberBand::Rectangle, this);

		// setStyle is important, set to windows style will just draw
		// rectangle frame, otherwise will draw a solid rectangle.
		myRectBand->setStyle(QStyleFactory::create("windows"));
	}

	myRectBand->setGeometry(aRect);
	myRectBand->show();
}

void OCCOpenGL::panByMiddleButton(const QPoint& thePoint)
{
	fitAll();
	return;

	Standard_Integer aCenterX = 0;
	Standard_Integer aCenterY = 0;

	QSize aSize = size();

	aCenterX = aSize.width() / 2;
	aCenterY = aSize.height() / 2;

	myView->Pan(aCenterX - thePoint.x(), thePoint.y() - aCenterY);
}


void OCCOpenGL::FuseSelected()
{
	if (vecShapes.size() != 2) return;

	if (aManipulator->HasActiveMode()) aManipulator->Detach();

	if (vecShapes[0]->IsKind(STANDARD_TYPE(AIS_Shape)) && vecShapes[1]->IsKind(STANDARD_TYPE(AIS_Shape)))
	{
		Handle(AIS_Shape) shapeA = Handle(AIS_Shape)::DownCast(vecShapes[0]);
		Handle(AIS_Shape) shapeB = Handle(AIS_Shape)::DownCast(vecShapes[1]);

		TopoDS_Shape aFusedShape = BRepAlgoAPI_Fuse(shapeA->Shape(), shapeB->Shape());
		Handle(AIS_Shape) anAisCylinder = new AIS_Shape(aFusedShape);
		std::cout << "fused done" << std::endl;
		myContext->Remove(vecShapes[0], Standard_True);
		myContext->Remove(vecShapes[1], Standard_True);
		myContext->Display(anAisCylinder, Standard_True);
	}

}

void OCCOpenGL::IntersectSelected()
{
	if (vecShapes.size() != 2) return;

	if (aManipulator->HasActiveMode()) aManipulator->Detach();

	if (vecShapes[0]->IsKind(STANDARD_TYPE(AIS_Shape)) && vecShapes[1]->IsKind(STANDARD_TYPE(AIS_Shape)))
	{
		Handle(AIS_Shape) shapeA = Handle(AIS_Shape)::DownCast(vecShapes[0]);
		Handle(AIS_Shape) shapeB = Handle(AIS_Shape)::DownCast(vecShapes[1]);

		TopoDS_Shape aFusedShape = BRepAlgoAPI_Cut(shapeA->Shape(), shapeB->Shape());
		Handle(AIS_Shape) intersectedSahpe = new AIS_Shape(aFusedShape);
		std::cout << "cut done" << std::endl;
		myContext->Remove(vecShapes[0], Standard_True);
		myContext->Remove(vecShapes[1], Standard_True);
		myContext->Display(intersectedSahpe, Standard_True);

		helm->IntersectAwithB(shapeA, shapeB, intersectedSahpe);
	}
}

void OCCOpenGL::objSelected()
{
	QList<QTreeWidgetItem*> iterList = objectWidget->ui.treeWidget->selectedItems();
	bool noRealPrimitiveSelected = true;
	for (auto & iter : iterList)
	{
		if (!objMapping[iter]) continue;
		noRealPrimitiveSelected = false;
		break;
	}
}

void OCCOpenGL::CreateShape(Part::FeaturePrimitive* prim)
{
	/// allocate mapping
	primMapping[prim->getGraphicShape()] = prim;
	getContext()->Display(prim->getGraphicShape(), Standard_True);

	if (prim == nullptr) return;

	QString primName;
	QIcon primitiveIcon;

	if (prim->getTypeId() == Part::FeatureBox::getClassTypeId())
	{
		primName = QString("Box ") + helm->AssignCreatedShape(prim);
		primitiveIcon.addFile(":/Resources/cube_crop.png");
	}
	QTreeWidget *treeWidget = objectWidget->ui.treeWidget;
	QTreeWidgetItem *primitiveItem = new QTreeWidgetItem(treeWidget, QStringList(primName));
	primitiveItem->setIcon(0, primitiveIcon);

	// 	switch (prim->pType)
	// 	{
	// 	case PrimType::Lumber:
	// 		primName = "Create Lumber";
	// 		primitiveIcon.addFile(":/Resources/lumber.png");
	// 		break;
	// 	case PrimType::Box:
	// 		primName = "Box";
	// 		primitiveIcon.addFile(":/Resources/cube_crop.png");
	// 		break;
	// 	case PrimType::Cylinder:
	// 		primName = "Cylinder";
	// 		primitiveIcon.addFile(":/Resources/cylinder_crop.png");
	// 		break;
	// 	}
	// 

	// 
	// 	helm->AssignCreatedShape(prim);
}

void OCCOpenGL::slotRedraw()
{
	Redraw();
	helm->CompileToHELM();
}

void OCCOpenGL::onLumberLengthChanged(MyLumber* lumber, double length)
{
	AIS_ListOfInteractive listOfInteractive;
	myContext->DisplayedObjects(listOfInteractive);
	AIS_ListIteratorOfListOfInteractive iter(listOfInteractive);
	for (; iter.More(); iter.Next())
	{
		Handle(AIS_InteractiveObject) firstInterObj = iter.Value();
		if (lumber->getGraphicShape() == Handle(AIS_Shape)::DownCast(firstInterObj))
		{
			double scaleRatio = length;
			gp_GTrsf scaleTrsf = gp_GTrsf();
			gp_Mat rot(1.0, 0, 0, 0, 1.0, 0, 0, 0, length);
			scaleTrsf.SetVectorialPart(rot);
			//scaleTrsf.SetValue(3, 3, scaleRatio);

			BRepBuilderAPI_GTransform trsf(lumber->baseShape, scaleTrsf);
			auto updatedShape = trsf.Shape();

			lumber->getGraphicShape()->Set(updatedShape);
			lumber->getGraphicShape()->Redisplay(Standard_True);
			myView->Redraw();
		}
	}

}

void OCCOpenGL::Redraw()
{
	myView->Redraw();
}