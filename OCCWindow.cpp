#include "OCCWindow.h"
#include "OCCOpenGL.h"
#include "OCCDomainLang.h"

#include <QToolBar>
#include <QTreeView>
#include <QMessageBox>
#include <QDockWidget>
#include <QDateTime>

#include <gp_Circ.hxx>
#include <gp_Elips.hxx>
#include <gp_Pln.hxx>

#include <gp_Lin2d.hxx>

#include <Geom_ConicalSurface.hxx>
#include <Geom_ToroidalSurface.hxx>
#include <Geom_CylindricalSurface.hxx>

#include <GCE2d_MakeSegment.hxx>

#include <TopExp.hxx>
#include <TopExp_Explorer.hxx>
#include <TColgp_Array1OfPnt2d.hxx>

#include <BRepLib.hxx>

#include <BRepBuilderAPI_MakeVertex.hxx>
#include <BRepBuilderAPI_MakeEdge.hxx>
#include <BRepBuilderAPI_MakeWire.hxx>
#include <BRepBuilderAPI_MakeFace.hxx>
#include <BRepBuilderAPI_Transform.hxx>
#include <BRepBuilderAPI_MakePolygon.hxx>

#include <BRepPrimAPI_MakeBox.hxx>
#include <BRepPrimAPI_MakeCone.hxx>
#include <BRepPrimAPI_MakeSphere.hxx>
#include <BRepPrimAPI_MakeCylinder.hxx>
#include <BRepPrimAPI_MakeTorus.hxx>
#include <BRepPrimAPI_MakePrism.hxx>
#include <BRepPrimAPI_MakeRevol.hxx>

#include <BRepFilletAPI_MakeFillet.hxx>
#include <BRepFilletAPI_MakeChamfer.hxx>

#include <BRepOffsetAPI_MakePipe.hxx>
#include <BRepOffsetAPI_ThruSections.hxx>

#include <BRepAlgoAPI_Cut.hxx>
#include <BRepAlgoAPI_Fuse.hxx>
#include <BRepAlgoAPI_Common.hxx>

#include <gp_Trsf.hxx>

template <typename T>
std::vector<Handle(AIS_Shape)> convert_to_AIS_shape(std::unordered_set<T>& shapes)
{
	std::vector<Handle(AIS_Shape)> vecShapes;

	for (const typename T& s : shapes)
	{
		Handle(AIS_Shape) anAisSphere = new AIS_Shape(s->shape);

		anAisSphere->SetColor(Quantity_NOC_GOLD);

		vecShapes.push_back(anAisSphere);
	}

	return vecShapes;
}

OCCWindow::OCCWindow(QWidget *parent)
	: QMainWindow(parent)
{
	ui.setupUi(this);

	myOCCOpenGL = new OCCOpenGL(this);

	setCentralWidget(myOCCOpenGL);

	setup_editor();
	intialize_widget();

	createActions();
	createMenus();
	createToolBars();
	
	outputResult("Carpentry CAD initialized.");
}

OCCWindow::~OCCWindow()
{

}

void OCCWindow::setup_editor()
{
	QFont font;
	font.setFamily("Courier");
	font.setFixedPitch(true);
	font.setPointSize(10);

	editor = new CodeEditor();
	editor->setFont(font);
	editor->setMinimumWidth(400);

	const int tabStop = 4;
	QFontMetrics metrics(font);
	editor->setTabStopWidth(tabStop * metrics.width(' '));
	editor->setPlainText("(cylinder a 10 1)\n(box b1 1 1 10)\n(translate b1 3.5 3.5 -10)\n(box b2 1 1 10)\n(translate b2 3.5 -3.5 -10)\n(box b3 1 1 10)\n(translate b3 -3.5 3.5 -10)\n(box b4 1 1 10)\n(translate b4 -3.5 -3.5 -10)\n(list pgn -14 0 0 -4 0 0 -7 2 0 -6 3 0 -4 3 0 -3 5 0 -3 15 0)\n(blade a b pgn)\n(display a b1 b2 b3 b4)");
	//highlighter = new Highlighter(editor->document());
}

void OCCWindow::intialize_widget()
{
	QDockWidget *sr = new QDockWidget(tr("Code"), this);
	sr->setFeatures(QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetClosable);
	sr->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
	sr->setVisible(true);
	addDockWidget(Qt::LeftDockWidgetArea, sr);

	sr->setWidget(editor);


	QDockWidget *ur = new QDockWidget(tr("File"), this);
	ur->setFeatures(QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetClosable);
	//ur->setVisible(false);
	ur->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
	//ur(Qt::LeftDockWidgetArea, ur);
	this->tabifyDockWidget(sr, ur);

	QDockWidget *as = new QDockWidget(tr("Setting"), this);
	as->setFeatures(QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetClosable);
	//as->setVisible(false);
	as->setAllowedAreas(Qt::RightDockWidgetArea | Qt::LeftDockWidgetArea);
	addDockWidget(Qt::LeftDockWidgetArea, as);
	this->tabifyDockWidget(sr, as);

	sr->raise();
	//this->tabifyDockWidget(as, ur);

	QDockWidget *res = new QDockWidget(tr("Output"), this);
	res->setAllowedAreas(Qt::BottomDockWidgetArea);
	addDockWidget(Qt::BottomDockWidgetArea, res);

	QFont font;
	font.setFamily("Courier");
	font.setFixedPitch(true);
	font.setPointSize(10);
	outputEditor = new QTextEdit();
	outputEditor->setFont(font);
	outputEditor->setMinimumHeight(200);
	res->setWidget(outputEditor);
}

void OCCWindow::createActions(void)
{
	// File
	connect(ui.actionExit, SIGNAL(triggered()), this, SLOT(close()));

	// View
	connect(ui.actionZoom, SIGNAL(triggered()), myOCCOpenGL, SLOT(zoom()));
	connect(ui.actionPan, SIGNAL(triggered()), myOCCOpenGL, SLOT(pan()));
	connect(ui.actionRotate, SIGNAL(triggered()), myOCCOpenGL, SLOT(rotate()));

	connect(ui.actionReset, SIGNAL(triggered()), myOCCOpenGL, SLOT(reset()));
	connect(ui.actionFitAll, SIGNAL(triggered()), myOCCOpenGL, SLOT(fitAll()));

	connect(ui.actionCylinder, SIGNAL(triggered()), this, SLOT(on_action_compile()));

	// Help
	connect(ui.actionAbout, SIGNAL(triggered()), this, SLOT(about()));
}

void OCCWindow::createMenus(void)
{
}

void OCCWindow::createToolBars(void)
{
	QToolBar* aToolBar = addToolBar(tr("&Navigate"));
	aToolBar->addAction(ui.actionZoom);
	aToolBar->addAction(ui.actionPan);
	aToolBar->addAction(ui.actionRotate);

	aToolBar = addToolBar(tr("&View"));
	aToolBar->addAction(ui.actionReset);
	aToolBar->addAction(ui.actionFitAll);

	aToolBar = addToolBar(tr("&Primitive"));
	aToolBar->addSeparator();
	aToolBar->addAction(ui.actionCylinder);

// 	aToolBar = addToolBar(tr("Help"));
// 	aToolBar->addAction(ui.actionAbout);
}

void OCCWindow::about()
{
	QMessageBox::about(this, tr("About"),
		tr("<h2>Qt 5.11</h2>"
			"<p>OpenCASCADE</p>"));
}

void OCCWindow::outputResult(const QString& res)
{
	QDateTime date = QDateTime::currentDateTime();
	QString authDate = date.toString("[yyyy-MM-d hh:mm:ss]: ");
	outputEditor->append(authDate + res);
}

void OCCWindow::on_action_compile()
{
	std::string sourceCode = editor->toPlainText().toStdString();
	outputResult("Source code file size: " + QString::number(sourceCode.length()/1024.0) + "kb.");
	outputResult("Start compiling...");
	
	OCCDomainLang DSL;

	bool isSucces = DSL.compile(sourceCode);
	
	if (!isSucces)
	{
		outputResult("Errors occurred during compilation.");
		return;
	}

	outputResult("Successfully compiled.");

	vecShapes = convert_to_AIS_shape(*DSL.carpentryPrimitives);

	myOCCOpenGL->getContext()->RemoveAll(true);
	for (auto& s : vecShapes)
	{
		myOCCOpenGL->getContext()->Display(s, Standard_True);
	}

}