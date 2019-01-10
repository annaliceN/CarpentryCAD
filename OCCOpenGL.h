#ifndef _OCCOPENGL_H_
#define _OCCOPENGL_H_

#include <QGLWidget>

#include <AIS_InteractiveContext.hxx>
#include <AIS_Manipulator.hxx>
#include <AIS_ManipulatorMode.hxx>
#include <TopoDS_Face.hxx>
#include <AIS_Shape.hxx>

#include "PHELM.h"
#include "PropertyWidget.h"
#include "ObjectWidget.h"
#include "FeatureBox.h"
#include "Property.h"
#include "PropertyFeature.h"

#include <Sketcher.h>
#include <Sketcher_QtGUI.h>

class QMenu;
class QRubberBand;

#define GRID1 Aspect_GT_Rectangular, Aspect_GDM_Lines
#define GRID2 Aspect_GT_Circular, Aspect_GDM_Points

//! adapted a QGLWidget for OpenCASCADE viewer.
class OCCOpenGL : public QGLWidget
{
    Q_OBJECT

public:
    //! mouse actions.
    enum CurrentAction3d
    {
        CurAction3d_Nothing,
        CurAction3d_DynamicZooming,
        CurAction3d_WindowZooming,
        CurAction3d_DynamicPanning,
        CurAction3d_GlobalPanning,
        CurAction3d_DynamicRotation,
		SketcherPolyline,
		SketcherCurve,
		SketcherDrill
    };

	enum InterMode
	{
		Viewing,
		Manipulating	
	};

	enum DrawAction {
		MyEraseActionId,
		MyDeleteActionId,
		MyPropertyActionId,
		MyRedrawActionId,
		MyChangePlaneAction,
		MyGridActionId,
		MyInputPointAction,
		MyInputLineAction,
		MyInputCircleAction,
		MyInputCircle3PAction,
		MyInputCircle2PTanAction,
		MyInputCircleP2TanAction,
		MyInputCircle3TanAction,
		MyInputArc3PAction,
		MyInputArcCenter2PAction,
		MyInputBezierCurveAction,
		MyTrimCurveAction
	};

public:
    //! constructor.
	OCCOpenGL(QWidget* parent);

	// Get created objects
	const Handle(AIS_InteractiveContext)& getContext() const { return myContext; };
	PHELM* getHELM() { return helm; };
	MyPropertyWidget* getPropertyWidget() { return propertyWidget; };
	MyObjectWidget* getObjectWidget() { return objectWidget; };
	QList<QAction*>* getDrawActions();

	void setSelectionOptions(bool isPreselectionEnabled, bool isSelectionEnabled);
	
	void RecomputeDisplay(Part::FeaturePrimitive * prim);

signals:
    void selectionChanged(void);

public slots:
    void pan(void);
    void fitAll(void);
    void reset(void);
    void zoom(void);
	void rotate(void);
	void objSelected();
	void Redraw(void);
	void createShape(Base::BaseClass* obj);
	void slotRedraw(void);
	void onDeleteSelected();
	void onProperty();
	void onRedrawAll();
	void onChangePlane();
	void onGrid();
	void onInputPoints();
	void onInputLines();
	void onInputCircles();
	void onInputCircles3P();
	void onInputCircles2PTan();
	void onInputCirclesP2Tan();
	void onInputCircles3Tan();
	void onInputArc3P();
	void onInputArcCenter2P();
	void onInputBezierCurve();
	void onTrimCurve();
	void RecomputeTree();

protected:
    // Paint events.
    virtual void paintEvent(QPaintEvent* theEvent);
    virtual void resizeEvent(QResizeEvent* theEvent);

    // Mouse events.
    virtual void mousePressEvent(QMouseEvent* theEvent);
    virtual void mouseReleaseEvent(QMouseEvent* theEvent);
    virtual void mouseMoveEvent(QMouseEvent * theEvent);
    virtual void wheelEvent(QWheelEvent * theEvent);

    // Button events.
    virtual void onLButtonDown(const int theFlags, const QPoint thePoint);
    virtual void onMButtonDown(const int theFlags, const QPoint thePoint);
    virtual void onRButtonDown(const int theFlags, const QPoint thePoint);
    virtual void onMouseWheel(const int theFlags, const int theDelta, const QPoint thePoint);
    virtual void onLButtonUp(const int theFlags, const QPoint thePoint);
	void actionStart2dSketch(void);
	gp_Dir getDirection(const TopoDS_Face & face);
	void actionApplyPolyCut(void);
	void actionApplyCurveCut();
	void actionApplyDrill(void);
	void actionApply();
	void actionStop2dSketch(void);
	void actionPolylineCutting(void);
	void actionCurveCutting(void);
	void actionDrillHoles(void);
	void initCursors();
	void activateCursor(const CurrentAction3d mode);
    virtual void onMButtonUp(const int theFlags, const QPoint thePoint);
    virtual void onRButtonUp(const int theFlags, const QPoint thePoint);
    virtual void onMouseMove(const int theFlags, const QPoint thePoint);

    // Popup menu.
	virtual void addItemInPopup(QMenu* theMenu);

protected:
    void init(void);
	void initDrawActions();
	void ApplyConnections();
    void popup(const int x, const int y);
    void dragEvent(const int x, const int y);
    void inputEvent(const int x, const int y);
    void moveEvent(const int x, const int y);
    void multiMoveEvent(const int x, const int y);
    void multiDragEvent(const int x, const int y);
    void multiInputEvent(const int x, const int y);
    void drawRubberBand(const int minX, const int minY, const int maxX, const int maxY);
    void panByMiddleButton(const QPoint& thePoint);


private:

    //! the occ viewer.
    Handle(V3d_Viewer) myViewer;

    //! the occ view.
    Handle(V3d_View) myView;

    //! the occ context.
    Handle(AIS_InteractiveContext) myContext;

	Handle(AIS_Manipulator) aManipulator;

    //! save the mouse position.
    Standard_Integer myXmin;
    Standard_Integer myYmin;
    Standard_Integer myXmax;
    Standard_Integer myYmax;

    //! the mouse current mode.
    CurrentAction3d myCurrentMode;
	TopoDS_Face curSelectedFace;
	Handle(AIS_Shape) curSelectedGraphicShape;

    //! save the degenerate mode state.
    Standard_Boolean myDegenerateModeIsOn;

    //! rubber rectangle for the mouse selection.
    QRubberBand* myRectBand;
	gp_Trsf manipulatorTrsf;
	std::vector<Handle(AIS_InteractiveObject)> vecShapes;
	std::pair<Handle(AIS_InteractiveObject), bool> pairParaShape;

	int interactiveMode;
	bool GRIDCounter;

	// HELM 
	PHELM *helm;
	Sketcher* mySketcher;

	// PropertyWidget
	MyPropertyWidget *propertyWidget;
	MyObjectWidget *objectWidget;
	QList<QAction *> *myDrawActions;
	std::unordered_map< QTreeWidgetItem*, Base::BaseClass* > objMapping;
	std::map< Handle(AIS_Shape), Part::FeaturePrimitive*> primMapping;
	std::vector< Part::FeaturePrimitive*> featureWorkSpace;
	V3d_Coordinate my_v3dX, my_v3dY, my_v3dZ;
	Quantity_Parameter projVx, projVy, projVz;
	std::set< Handle(Geom2d_Edge) > isSketchDataVisited;

	QCursor* defCursor = NULL;
	QCursor* handCursor = NULL;
	QCursor* panCursor = NULL;
	QCursor* globPanCursor = NULL;
	QCursor* zoomCursor = NULL;
	QCursor* rotCursor = NULL;

	Handle(Graphic3d_Camera) myPreviousCam;

	bool myPreselectionEnabled;
	bool mySelectionEnabled;
};

#endif 
