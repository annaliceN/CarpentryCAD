#include "OCCOpenGL.h"

#include <QMenu>
#include <QMouseEvent>
#include <QRubberBand>
#include <QStyleFactory>
#include <QMessageBox>

#include <OpenGl_GraphicDriver.hxx>
#include <V3d_View.hxx>
#include <Aspect_Handle.hxx>
#include <Aspect_DisplayConnection.hxx>

#include <BRepAlgoAPI_Fuse.hxx>
#include <BRepAlgoAPI_Cut.hxx>
#include <BRepBuilderAPI_Transform.hxx>
#include <BRepBuilderAPI_GTransform.hxx>
#include <BRepBuilderAPI_MakeWire.hxx>

#include <TopoDS.hxx>
#include <TopExp.hxx>
#include <TopExp_Explorer.hxx>
#include <GeomLProp_SLProps.hxx>
#include <Geom_BSplineSurface.hxx>
#include <Geom_BezierSurface.hxx>
#include <TColgp_Array2OfPnt.hxx>
#include <ShapeAnalysis_Curve.hxx>
#include <AIS_TextLabel.hxx>
#include <GeomLib_IsPlanarSurface.hxx>
#include <TopOpeBRepBuild_Tools.hxx>

#include "FeaturePartCut.h"
#include "FeaturePolyline.h"
#include "FeaturePolyCut.h"
#include "FeatureBezierCurve.h"
#include "FeatureDrill.h"
#include <BRepAdaptor_Surface.hxx>
#include <GeomAPI_ProjectPointOnSurf.hxx>

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

	// init filter
	QIcon filterIcon;
	filterIcon.addFile(":/Resources/filter.png");
	QTreeWidgetItem* filter = new QTreeWidgetItem(objectWidget, QStringList("-----------"));
	filter->setIcon(0, filterIcon);

	objectWidget->addTopLevelItem(filter);

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

	Handle(AIS_TextLabel) aTextPrs = new AIS_TextLabel();
	aTextPrs->SetFont("Courier");

	aTextPrs->SetText("hello world");
	aTextPrs->SetText("OpenCASCADE");

	myContext->Display(aTextPrs, Standard_True);
}

QList<QAction *> *OCCOpenGL::getDrawActions() {
	initDrawActions();
	return myDrawActions;
}

void OCCOpenGL::setSelectionOptions(bool isPreselectionEnabled, bool isSelectionEnabled)
{
	myPreselectionEnabled = isPreselectionEnabled;
	mySelectionEnabled = isSelectionEnabled;
	//clear current selection in the viewer

	if (!mySelectionEnabled) {
		myContext->ClearSelected(Standard_True);
	}
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
}

void OCCOpenGL::onInputLines() {
	mySketcher->ObjectAction(Line2P_Method);
}

void OCCOpenGL::onInputCircles() {
	mySketcher->ObjectAction(CircleCenterRadius_Method);
	//myCurrentMode = SketcherAction;
}

void OCCOpenGL::onInputCircles3P() {
	mySketcher->ObjectAction(Circle3P_Method);
	//myCurrentMode = SketcherAction;
}

void OCCOpenGL::onInputCircles2PTan() {
	mySketcher->ObjectAction(Circle2PTan_Method);
	//myCurrentMode = SketcherAction;
}

void OCCOpenGL::onInputCirclesP2Tan() {
	mySketcher->ObjectAction(CircleP2Tan_Method);
	//myCurrentMode = SketcherAction;
}

void OCCOpenGL::onInputCircles3Tan() {
	mySketcher->ObjectAction(Circle3Tan_Method);
	//myCurrentMode = SketcherAction;
}

void OCCOpenGL::onInputArc3P() {
	mySketcher->ObjectAction(Arc3P_Method);
	//myCurrentMode = SketcherAction;
}

void OCCOpenGL::onInputArcCenter2P() {
	mySketcher->ObjectAction(ArcCenter2P_Method);
	//myCurrentMode = SketcherAction;
}

void OCCOpenGL::onInputBezierCurve() {
	mySketcher->ObjectAction(BezierCurve_Method);
	//myCurrentMode = SketcherAction;
}

void OCCOpenGL::onTrimCurve() {
	mySketcher->ObjectAction(Trim_Method);
	//myCurrentMode = SketcherAction;
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
	case SketcherPolyline:
	case SketcherCurve:
	case SketcherDrill:
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

void OCCOpenGL::onLButtonUp(const int theFlags, const QPoint thePoint)
{
	// Hide the QRubberBand
	if (myRectBand)
	{
		myRectBand->hide();
	}

	switch (myCurrentMode)
	{
	case SketcherCurve:
	case SketcherPolyline:
	case SketcherDrill:
		return;
		break;
	}

	// Ctrl for multi selection.
	if (thePoint.x() == myXmin && thePoint.y() == myYmin)
	{
		std::cout << "kaishi xuanze" << std::endl;
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
					curSelectedGraphicShape = Handle(AIS_Shape)::DownCast(myContext->SelectedInteractive());
					Part::FeaturePrimitive* selectedPrim = primMapping[curSelectedGraphicShape];

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

void projectToSketchCoord(const gp_Ax3& myCoordinateSystem, const gp_Pnt& myTempPnt, gp_Pnt2d& myCurrentPnt2d)
{
	myCurrentPnt2d.SetX((myTempPnt.X() - myCoordinateSystem.Location().X())*myCoordinateSystem.XDirection().X() + (myTempPnt.Y() - myCoordinateSystem.Location().Y())*myCoordinateSystem.XDirection().Y() + (myTempPnt.Z() - myCoordinateSystem.Location().Z())*myCoordinateSystem.XDirection().Z());
	myCurrentPnt2d.SetY((myTempPnt.X() - myCoordinateSystem.Location().X())*myCoordinateSystem.YDirection().X() + (myTempPnt.Y() - myCoordinateSystem.Location().Y())*myCoordinateSystem.YDirection().Y() + (myTempPnt.Z() - myCoordinateSystem.Location().Z())*myCoordinateSystem.YDirection().Z());
}

void OCCOpenGL::actionStart2dSketch()
{
	if (curSelectedFace.IsNull()) return;

	const Part::TopoShape& tShape = primMapping[curSelectedGraphicShape]->Shape.getShape();
	int nFaces = tShape.countSubShapes("Face");
	
	for (int i = 1; i <= nFaces; ++i)
	{
		TopoDS_Shape sh = tShape.getSubShape( (QString("Face")+QString::number(i)).toStdString().c_str());
		if (sh == curSelectedFace)
		{
			std::cout << "Found: " << i << std::endl;
		}
	}
	
	// construct a edge-face map 
	TopTools_IndexedDataMapOfShapeListOfShape edgeFaceMap;
	TopExp::MapShapesAndAncestors(tShape.getShape(), TopAbs_EDGE, TopAbs_FACE, edgeFaceMap);

	BRepAdaptor_Surface adapt(curSelectedFace);
	if (adapt.GetType() != GeomAbs_Plane) {
		TopLoc_Location loc;
		Handle(Geom_Surface) surf = BRep_Tool::Surface(curSelectedFace, loc);
		if (surf.IsNull() || !GeomLib_IsPlanarSurface(surf).IsPlanar()) {
			QMessageBox::warning(this, QObject::tr("No planar support"),
				QObject::tr("You need a planar face as support for a sketch!"));
			return;
		}
	}

	bool Reverse = false;
	if (curSelectedFace.Orientation() == TopAbs_REVERSED)
		Reverse = true;
	/// compute mid point of the face
	myView->Convert(myXmin, myYmin, my_v3dX, my_v3dY, my_v3dZ);
	myView->Proj(projVx, projVy, projVz);
	gp_Pnt ObjOrg(projVx, projVy, projVz);
	
	/// compute gp_ax3
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

	// Snapping for the edges around the selected face
	for (TopExp_Explorer edgeExplorer(curSelectedFace, TopAbs_EDGE); edgeExplorer.More(); edgeExplorer.Next())
	{
		gp_Pnt2d vEdge[2];
		Handle(Geom_CartesianPoint) cpts[2];
		int cnt = 0;
		for (TopExp_Explorer vertexExplorer(edgeExplorer.Current(), TopAbs_VERTEX); vertexExplorer.More(); vertexExplorer.Next())
		{
			const TopoDS_Vertex& vertex = TopoDS::Vertex(vertexExplorer.Current());
			gp_Pnt pt = BRep_Tool::Pnt(vertex);
			projectToSketchCoord(SketchPos, pt, vEdge[cnt]);
			cpts[cnt] = new Geom_CartesianPoint(pt);
			++cnt;
		}
		Handle(Geom2d_Edge) newGeomEdge = new Geom2d_Edge();
		newGeomEdge->SetPoints(vEdge[0], vEdge[1]);
		Handle(AIS_Line) myAIS_Line = new AIS_Line(cpts[0], cpts[1]);
		myAIS_Line->SetColor(Quantity_NOC_RED);
		mySketcher->GetAnalyser()->AddObject(newGeomEdge, myAIS_Line, ExistingEdgeObject);
		
		/// flag the edge as visited
		isSketchDataVisited.insert(newGeomEdge);

		TopoDS_Edge curEdge = TopoDS::Edge(edgeExplorer.Current());
		TopoDS_Face targetFace;
		bool faceFound = TopOpeBRepBuild_Tools::GetAdjacentFace(curSelectedFace, curEdge,
			edgeFaceMap, targetFace);

		if (!faceFound)
		{
			// This happens, if the part boundary was reached.
			std::cout << "face not found" << std::endl;
		}
		else
		{
			std::cout << "face found" << std::endl;
		}
	}

	/// set coordinate system
	mySketcher->SetCoordinateSystem(SketchPos);
	myPreviousCam = new Graphic3d_Camera;
	myPreviousCam->Copy(myView->Camera());
	myView->Camera()->SetUp(SketchPos.YDirection());
	myView->Camera()->SetDirection(-dir);
	myView->Redraw();
}

gp_Dir OCCOpenGL::getDirection(const TopoDS_Face & face)
{
	Handle(Geom_Surface) SurfToProj = BRep_Tool::Surface(face);
	Standard_Real umin, umax, vmin, vmax;
	GeomLProp_SLProps props(SurfToProj, umin, vmin, 1, 0.01);
	gp_Dir Normal = props.Normal();
	Standard_Real Min_Curvature = props.MinCurvature();
	Standard_Real Max_Curvature = props.MaxCurvature();

	// In the case your manifold changes from convex to concave or viceversa
	// the normal could jump from "inner" to "outer" normal.
	// However, you should be able to change the normal sense preserving
	// the manifold orientation:
	if (face.Orientation() == TopAbs_REVERSED)
	{
		Normal = gp_Dir(-Normal.X(), -Normal.Y(), -Normal.Z());
	}

	return Normal;
}

void OCCOpenGL::actionApplyPolyCut()
{
	Part::FeaturePolyCut* fCut = new Part::FeaturePolyCut;
	fCut->BaseFeature.setValue(primMapping[curSelectedGraphicShape]);
	
	auto sData = mySketcher->GetData();
	const auto& curCoordinateSystem = mySketcher->GetCoordinateSystem();
	
	Part::FeaturePolyline* fPCut = new Part::FeaturePolyline;

	Handle(Geom_Surface) aSurface = BRep_Tool::Surface(curSelectedFace);
	Standard_Real umin, umax, vmin, vmax;
	GeomLProp_SLProps props(aSurface, umin, vmin, 1, 0.01);
	gp_Dir normal = props.Normal();
	fPCut->Dir.setValue(Base::Vector3d(normal.X(), normal.Y(), normal.Z()));
	
	std::vector< Base::Vector3d> vecLines;
	bool isStartPoint = true;
	for (Standard_Integer i = 1; i <= sData->Length(); i++)
	{
		auto myCurObject = Handle(Sketcher_Object)::DownCast(sData->Value(i));
		auto drawnEdge = Handle(Geom2d_Edge)::DownCast(myCurObject->GetGeometry());
		if (isSketchDataVisited.find(drawnEdge) != isSketchDataVisited.end())
		{
			continue;
		}
		auto ps = drawnEdge->GetStart_Pnt();
		auto pe = drawnEdge->GetEnd_Pnt();
		Handle(Geom_CartesianPoint) Geom_Point1 = new Geom_CartesianPoint(ElCLib::To3d(curCoordinateSystem.Ax2(), ps));
		Handle(Geom_CartesianPoint) Geom_Point2 = new Geom_CartesianPoint(ElCLib::To3d(curCoordinateSystem.Ax2(), pe));
		
		if (isStartPoint)
		{
			vecLines.push_back(Base::Vector3d(Geom_Point1->X(), Geom_Point1->Y(), Geom_Point1->Z()));
			isStartPoint = false;
		}
		vecLines.push_back(Base::Vector3d(Geom_Point2->X(), Geom_Point2->Y(), Geom_Point2->Z()));
	}

	fPCut->Nodes.setValues(vecLines);
	fPCut->execute();
	fCut->ToolFeature.setValue(fPCut);
	createShape(fPCut);

	// create shape
	fCut->execute();
	fCut->BuildGraphicShape();
	auto cutGraphicShape = fCut->getGraphicShape();
	createShape(fCut);
	
	// stop sketching
	actionStop2dSketch();
}

void OCCOpenGL::actionApplyCurveCut()
{
	Part::FeaturePolyCut* fCut = new Part::FeaturePolyCut;
	fCut->BaseFeature.setValue(primMapping[curSelectedGraphicShape]);
	
	Part::FeaturePolyline* fPCut = new Part::FeaturePolyline;
	Handle(Geom_Surface) aSurface = BRep_Tool::Surface(curSelectedFace);
	Standard_Real umin, umax, vmin, vmax;
	GeomLProp_SLProps props(aSurface, umin, vmin, 1, 0.01);
	gp_Dir normal = props.Normal();
	fPCut->Dir.setValue(Base::Vector3d(normal.X(), normal.Y(), normal.Z()));

	auto sData = mySketcher->GetData();
	const auto& curCoordinateSystem = mySketcher->GetCoordinateSystem();

	std::vector< Base::Vector3d> vecLines;
	auto curCommand = mySketcher->GetCurrentCommand();
	auto bezierCommand = Handle(Sketcher_CommandBezierCurve)::DownCast(curCommand);
	if (!bezierCommand.IsNull())
	{
		auto bCurve = bezierCommand->GetBezier();
		TColgp_SequenceOfPnt pnts;
		ShapeAnalysis_Curve::GetSamplePoints(bCurve, 0.0, 1.0, pnts);
		if (pnts.Length() == 0)
			printf_s(" !!!!!!!!!!!!! zero points in curve !!!!!!!!!!!!!!!!!\n");

		for (int i = 0; i < pnts.Length(); i++)
		{
			gp_Pnt pnt = pnts.Value(i + 1);
			vecLines.emplace_back(pnt.X(), pnt.Y(), pnt.Z());
		}
	}

	fPCut->Nodes.setValues(vecLines);
	fPCut->execute();
	fCut->ToolFeature.setValue(fPCut);

	// create shape
	fCut->execute();
	fCut->BuildGraphicShape();
	auto cutGraphicShape = fCut->getGraphicShape();
	createShape(fCut);

	// stop sketching
	actionStop2dSketch();
	return;
}

void OCCOpenGL::actionApplyDrill(void)
{
	Part::FeatureDrill* fDrill = new Part::FeatureDrill;
	fDrill->BaseFeature.setValue(primMapping[curSelectedGraphicShape]);

	auto sData = mySketcher->GetData();
	const auto& curCoordinateSystem = mySketcher->GetCoordinateSystem();
	
	gp_Dir normal = getDirection(curSelectedFace);
	fDrill->Dir.setValue(Base::Vector3d(-normal.X(), -normal.Y(), -normal.Z()));

	std::vector< Base::Vector3d> vecLines;
	bool isStartPoint = true;
	for (Standard_Integer i = 1; i <= sData->Length(); i++)
	{
		auto myCurObject = Handle(Sketcher_Object)::DownCast(sData->Value(i));
		if (myCurObject->GetGeometryType() == PointSketcherObject)
		{
			auto drawnPnt = Handle(Geom2d_CartesianPoint)::DownCast(myCurObject->GetGeometry());
			auto curPnt2d = gp_Pnt2d(drawnPnt->X(), drawnPnt->Y());
			Handle(Geom_CartesianPoint) Geom_Point = new Geom_CartesianPoint(ElCLib::To3d(curCoordinateSystem.Ax2(), curPnt2d));
			vecLines.push_back(Base::Vector3d(Geom_Point->X(), Geom_Point->Y(), Geom_Point->Z()));
		}
	}

	fDrill->Nodes.setValues(vecLines);
	fDrill->execute();

	// create shape
	fDrill->BuildGraphicShape();
	auto drillGraphicShape = fDrill->getGraphicShape();
	createShape(fDrill);
	
	// stop sketching
	actionStop2dSketch();
}

void OCCOpenGL::actionApply()
{
	switch (myCurrentMode)
	{
	case SketcherCurve:
		actionApplyCurveCut();
		break;
	case SketcherPolyline:
		actionApplyPolyCut();
		break;
	case SketcherDrill:
		actionApplyDrill();
		break;
	}
}

void OCCOpenGL::actionStop2dSketch()
{
	// Clear ref lines
	mySketcher->GetCurrentCommand()->ClearRefLines();

	myCurrentMode = CurAction3d_DynamicRotation;
	auto& sData= mySketcher->GetData();
	for (auto i = 1; i <= sData->Length(); ++i)
	{
		auto myCurObject = Handle(Sketcher_Object)::DownCast(sData->Value(i));
		auto myCurAISObj = myCurObject->GetAIS_Object();
		myContext->Remove(myCurAISObj, Standard_True);
		//myCurAISObj->Delete();
	}
	sData->Clear();
	mySketcher->ObjectAction(Nothing_Method);
	activateCursor(myCurrentMode);
	myView->SetCamera(myPreviousCam);
	myView->Redraw();
}

void OCCOpenGL::actionPolylineCutting()
{
	myCurrentMode = SketcherPolyline;
	mySketcher->SetPolyCut();
	actionStart2dSketch();
	onInputLines();
	mySketcher->SetPolylineMode(Standard_True);
	mySketcher->SetSnap(Sketcher_SnapType::SnapAnalyse);
	activateCursor(SketcherPolyline);
}

void OCCOpenGL::actionCurveCutting()
{
	myCurrentMode = SketcherCurve;
	mySketcher->SetPolyCut();
	actionStart2dSketch();
	onInputBezierCurve();
	activateCursor(SketcherCurve);
}

void OCCOpenGL::actionDrillHoles(void)
{
	myCurrentMode = SketcherDrill;
	actionStart2dSketch();
	onInputPoints();
	activateCursor(SketcherDrill);
}

void OCCOpenGL::RecomputeTree()
{
	for (auto feat : featureWorkSpace)
	{
		myContext->Remove(feat->getGraphicShape(), Standard_True);
	}

	// update filter
	bool foundFilter = false;
	for (int i = 0; i < objectWidget->topLevelItemCount(); ++i)
	{
		QTreeWidgetItem* curItem = objectWidget->topLevelItem(i);
		if (curItem->text(0) == "-----------")
		{
			foundFilter = true;
		}

		if (foundFilter)
		{
			curItem->setTextColor(0, QColor::fromRgb(100, 100, 100));
		}
		else
		{
			auto curBase = objMapping[curItem];
			if (curBase->isDerivedFrom(Part::FeaturePrimitive::getClassTypeId()))
			{
				auto curPrim = dynamic_cast<Part::FeaturePrimitive*>(curBase);
				if (curPrim != nullptr && curPrim->mustExecute())
				{
					curPrim->execute();
					std::vector< App::Property*> vecProp;
					curPrim->getPropertyList(vecProp);
					for (auto & p : vecProp) curPrim->onChanged(p);
				}
				RecomputeDisplay(curPrim);
			}
			curItem->setTextColor(0, QColor::fromRgb(0, 0, 0));
		}
	}
	slotRedraw();
	this->update();
}

void OCCOpenGL::objSelected()
{
	QList<QTreeWidgetItem*> iterList = objectWidget->selectedItems();
	bool noRealPrimitiveSelected = true;
	for (auto iter = iterList.begin(); iter != iterList.end(); ++iter)
	{
		if (!objMapping[*iter]) continue;
		noRealPrimitiveSelected = false;
		break;
	}
	if (noRealPrimitiveSelected) {
		//curselectedprimitive = NULL;
	}

	// set selected highLighted
	for (int i = 0; i < iterList.size(); ++i)
	{
		auto objOrPrimitive = objMapping[iterList[i]];
		if (objOrPrimitive == NULL) continue;
		propertyWidget->WritePropertiesToPropWidget(objOrPrimitive);
	}

	this->update();
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
		setCursor(*defCursor);
		break;
	case CurAction3d_GlobalPanning:
		setCursor(*globPanCursor);
		break;
	case CurAction3d_WindowZooming:
		setCursor(*handCursor);
		break;
	case SketcherPolyline:
	case SketcherCurve:
	case SketcherDrill:
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
	QAction* openAct = new QAction("Recompute", this);
	connect(openAct, &QAction::triggered, [=]() {
		for (auto f : featureWorkSpace)
		{
			std::cout << "mustExecute? " << f->mustExecute() << std::endl;
			if (f->mustExecute())
			{
				f->execute();
				std::vector< App::Property*> vecProp;
				f->getPropertyList(vecProp);
				for (auto & p : vecProp) f->onChanged(p);
			}
		}
		slotRedraw();
	});
	menu.addAction(openAct);
	menu.addSeparator();
	
	if (!curSelectedFace.IsNull())
	{
		openAct = new QAction("Polygon cut", this);
		connect(openAct, &QAction::triggered, [=]() {
			actionPolylineCutting();
		});
		menu.addAction(openAct);

		openAct = new QAction("Curve cut", this);
		connect(openAct, &QAction::triggered, [=]() {
			actionCurveCutting();
		});
		menu.addAction(openAct);

		openAct = new QAction("Drill holes", this);
		connect(openAct, &QAction::triggered, [=]() {
			actionDrillHoles();
		});
		menu.addAction(openAct);
		
		if (myCurrentMode == SketcherCurve ||
			myCurrentMode == SketcherPolyline ||
			myCurrentMode == SketcherDrill)
		{
			openAct = new QAction("Apply", this);
			connect(openAct, &QAction::triggered, [=]() {
				actionApply();
			});
			menu.addAction(openAct);

			menu.addAction(openAct);
			openAct = new QAction("Cancel", this);
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

	switch (myCurrentMode)
	{
	case OCCOpenGL::CurAction3d_Nothing:
		break;
	case OCCOpenGL::CurAction3d_DynamicZooming:
		break;
	case OCCOpenGL::CurAction3d_WindowZooming:
		break;
	case OCCOpenGL::CurAction3d_DynamicPanning:
		break;
	case OCCOpenGL::CurAction3d_GlobalPanning:
		break;
	case OCCOpenGL::CurAction3d_DynamicRotation:
		break;
	case OCCOpenGL::SketcherPolyline:
	case OCCOpenGL::SketcherCurve:
		if (mySketcher->DonePolyCut()) mySketcher->OnCancel();
	case OCCOpenGL::SketcherDrill:
		myView->Convert(thePoint.x(), thePoint.y(), my_v3dX, my_v3dY, my_v3dZ);
		myView->Proj(projVx, projVy, projVz);
		mySketcher->OnMouseMoveEvent(my_v3dX, my_v3dY, my_v3dZ, projVx, projVy, projVz);
		break;
	default:
		break;
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
	if (myCurrentMode == CurAction3d_DynamicRotation)
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

void OCCOpenGL::RecomputeDisplay(Part::FeaturePrimitive* prim)
{
	// First, display!
	if (prim->getTypeId() != Part::FeaturePolyline::getClassTypeId())
	{
		primMapping[prim->getGraphicShape()] = prim;
		myContext->Display(prim->getGraphicShape(), Standard_True);
	}

	// Second, disable previously displayed.
	if (prim->getTypeId() == Part::FeatureDrill::getClassTypeId())
	{
		auto pDrill = dynamic_cast<Part::FeatureDrill*>(prim);
		for (auto it = objMapping.begin(); it != objMapping.end(); ++it)
		{
			auto p = *it;
			if (p.second == pDrill->BaseFeature.getValue())
			{
				auto tbCut = dynamic_cast<Part::FeaturePrimitive*>(p.second);
				myContext->Remove(tbCut->getGraphicShape(), Standard_True);

			}
		}
	}
	else if (prim->getTypeId() == Part::FeaturePolyCut::getClassTypeId() ||
		prim->getTypeId() == Part::FeaturePartBoolean::getClassTypeId())
	{
		auto pCut = dynamic_cast<Part::FeaturePolyCut*>(prim);
		for (auto it = objMapping.begin(); it != objMapping.end(); ++it)
		{
			auto p = *it;
			if (p.second == pCut->BaseFeature.getValue() || p.second == pCut->ToolFeature.getValue())
			{
				auto tbCut = dynamic_cast<Part::FeaturePrimitive*>(p.second);
				myContext->Remove(tbCut->getGraphicShape(), Standard_True);
			}
		}
	}
}

void OCCOpenGL::createShape(Base::BaseClass* obj)
{
	if (obj == nullptr) return;
	/// allocate mapping

	QTreeWidgetItem *primitiveItem;

	if (obj->isDerivedFrom(Part::FeaturePrimitive::getClassTypeId()))
	{
		auto prim = dynamic_cast<Part::FeaturePrimitive*>(obj);
		if (prim == nullptr) return;
		
		QString primName;
		QIcon primitiveIcon;

		if (prim->getTypeId() == Part::FeatureBox::getClassTypeId())
		{
			try
			{
				TopoDS_Shape shape1 = prim->Shape.getShape().getSubShape("Edge20");
			}
			catch (...){
				std::cout << "error" << std::endl;
			}
			
			std::cout << prim->Shape.getShape().countSubShapes("Edge") << std::endl;
// 			TopoDS_Edge face1 = TopoDS::Edge(shape1);
// 			if (!face1.IsNull()) {
// 				std::cout << "Got face 1" << std::endl;
// 			}
// 			else
// 			{
// 				std::cout << "Cannot get face 1" << std::endl;
// 			}
			primName = QString("Box ") + helm->AssignCreatedShape(prim);
			primitiveIcon.addFile(":/Resources/cube_crop.png");
			primitiveItem = new QTreeWidgetItem(QStringList(primName));
		}
		else if (prim->getTypeId() == Part::FeatureCylinder::getClassTypeId())
		{
			primName = QString("Cylinder ") + helm->AssignCreatedShape(prim);
			primitiveIcon.addFile(":/Resources/cylinder_crop.png");
			primitiveItem = new QTreeWidgetItem(QStringList(primName));
		}
		// Cut operation
		//if (prim->isDerivedFrom(Part::FeaturePolycut::getClassTypeId()) ||
		//	prim->isDerivedFrom(Part::FeaturePartBoolean::getClassTypeId()))

		if (prim->getTypeId() == Part::FeatureDrill::getClassTypeId())
		{
			primName = QString("Drill ") + helm->AssignCreatedShape(prim);
			primitiveIcon.addFile(":/Resources/cube_crop.png");
			primitiveItem = new QTreeWidgetItem(QStringList(primName));
			
			// Add property to its child
			std::vector<App::Property*> pList;
			prim->getPropertyList(pList);
			for (auto prop : pList)
			{
				if (prop->isDerivedFrom(Part::PropertyShapeHistory::getClassTypeId())) continue;
				QTreeWidgetItem *tbWidget = new QTreeWidgetItem(QStringList(prop->getName()));
				primitiveItem->addChild(tbWidget);
				objMapping[tbWidget] = prop;
			}
		}
		else if (prim->getTypeId() == Part::FeaturePolyCut::getClassTypeId() ||
			prim->getTypeId() == Part::FeaturePartBoolean::getClassTypeId())
		{
			primName = QString("Cut ") + helm->AssignCreatedShape(prim);
			primitiveIcon.addFile(":/Resources/cube_crop.png");
			primitiveItem = new QTreeWidgetItem(QStringList(primName));
		
			// Add property to its child
			std::vector<App::Property*> pList;
			prim->getPropertyList(pList);
			for (auto prop : pList)
			{
				if (prop->isDerivedFrom(Part::PropertyShapeHistory::getClassTypeId())) continue;
				QTreeWidgetItem *tbWidget = new QTreeWidgetItem(QStringList(prop->getName()));
				primitiveItem->addChild(tbWidget);
				objMapping[tbWidget] = prop;
			}
		}
		else if (prim->getTypeId() == Part::FeaturePolyline::getClassTypeId())
		{
			primName = QString("Wire ") + helm->AssignCreatedShape(prim);
			primitiveIcon.addFile(":/Resources/cube_crop.png");
			primitiveItem = new QTreeWidgetItem(QStringList(primName));

			std::vector<App::Property*> pList;
			prim->getPropertyList(pList);
			for (auto prop : pList)
			{
				if (prop->isDerivedFrom(Part::PropertyShapeHistory::getClassTypeId())) continue;
				QTreeWidgetItem *tbWidget = new QTreeWidgetItem(QStringList(prop->getName()));
				primitiveItem->addChild(tbWidget);
				objMapping[tbWidget] = prop;
			}
		}

		QTreeWidget *treeWidget = objectWidget;
		
		if (treeWidget->topLevelItemCount() &&
			treeWidget->topLevelItem(treeWidget->topLevelItemCount() - 1)->text(0) == QString(("-----------")))
		{
			treeWidget->insertTopLevelItem(treeWidget->topLevelItemCount() - 1, primitiveItem);
		}
		else
		{
			treeWidget->addTopLevelItem(primitiveItem);
		}
		
		primitiveItem->setIcon(0, primitiveIcon);

		featureWorkSpace.push_back(prim);
		objMapping[primitiveItem] = prim;

		// recompute display
		RecomputeDisplay(prim);
	}

	// Refresh and generate helm code
	slotRedraw();
}

void OCCOpenGL::slotRedraw()
{
	// TODO
	//QTreeWidget *treeWidget = objectWidget->ui.treeWidget;


	Redraw();
	helm->CompileToHELM();
}

void OCCOpenGL::Redraw()
{
	myView->Redraw();
}