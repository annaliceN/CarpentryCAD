#include "OCCWindow.h"
#include "OCCOpenGL.h"
#include "OCCDomainLang.h"

#include <QToolBar>
#include <QTreeView>
#include <QMessageBox>
#include <QDockWidget>
#include <QDateTime>
#include <QDialogButtonBox>
#include <QVBoxLayout>
#include <QDoubleSpinBox>
#include <QLineEdit>

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

#include "FeatureBox.h"
#include "ToolsConfiguration.h"

template <typename T>
std::vector<Handle(AIS_Shape)> convert_to_AIS_shape(std::unordered_set<T>& shapes)
{
	std::vector<Handle(AIS_Shape)> vecShapes;

	for (const typename T& s : shapes)
	{
		Handle(AIS_Shape) anAisSphere = new AIS_Shape(s->getShape());

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

	DSL = new OCCDomainLang();

	setup_editor();
	intialize_widget();

	createActions();
	createMenus();
	createToolBars();
	
	outputResult("Carpentry CAD initialized.");

	ToolsConfiguration *dlg = new ToolsConfiguration(this);
	dlg->show();
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
	//editor->setPlainText("(cylinder a 10 1)\n(box b1 1 1 10)\n(translate b1 3.5 3.5 -10)\n(box b2 1 1 10)\n(translate b2 3.5 -3.5 -10)\n(box b3 1 1 10)\n(translate b3 -3.5 3.5 -10)\n(box b4 1 1 10)\n(translate b4 -3.5 -3.5 -10)\n(list pgn -14 0 0 -4 0 0 -7 2 0 -6 3 0 -4 3 0 -3 5 0 -3 15 0)\n(blade a b pgn)\n(display a b1 b2 b3 b4)");
	//highlighter = new Highlighter(editor->document());
}

void OCCWindow::intialize_widget()
{
	MyPropertyWidget *propertyWidget = myOCCOpenGL->getPropertyWidget();
	propertyWidget->setAllowedAreas(Qt::AllDockWidgetAreas);
	propertyWidget->setWindowTitle("Properties");
	QStringList columnNames;
	columnNames.append("Property");
	columnNames.append("Value");
	propertyWidget->ui.tableWidget->setRowCount(0);
	propertyWidget->ui.tableWidget->setColumnCount(2);
	propertyWidget->ui.tableWidget->setColumnWidth(0, 100);
	propertyWidget->ui.tableWidget->setColumnWidth(1, 100);
	//propertyWidget->ui.tableWidget->horizontalHeader()->setStretchLastSection(true);
	propertyWidget->ui.tableWidget->setHorizontalHeaderLabels(columnNames);
	propertyWidget->setMinimumWidth(200);
	propertyWidget->setMaximumWidth(400);
	addDockWidget(Qt::RightDockWidgetArea, propertyWidget);

	// CAD OPERATIONS
	//QDockWidget *cad = new QDockWidget(tr("CAD Operations"), this);
	QDockWidget *cad = new QDockWidget(tr("CAD Operations"), this);
	cad->setFeatures(QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetClosable);
	cad->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
	cad->setVisible(true);
	cad->setMinimumHeight(240);
	addDockWidget(Qt::LeftDockWidgetArea, cad);
	cad->setWidget(myOCCOpenGL->getObjectWidget());
// 
// 	MyObjectWidget *cad = myOCCOpenGL->getObjectWidget();
// 	cad->setMinimumHeight(240);
// 	cad->setAllowedAreas(Qt::LeftDockWidgetArea);
// 	addDockWidget(Qt::LeftDockWidgetArea, cad);

	QDockWidget *sr = new QDockWidget(tr("HELM Code"), this);
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

	outputEditor = new QTextEdit();
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

	//connect(ui.actionCylinder, SIGNAL(triggered()), this, SLOT(on_action_compile()));
	connect(ui.actionBox, SIGNAL(triggered()), this, SLOT(on_action_make_box()));
	connect(ui.actionCone, SIGNAL(triggered()), this, SLOT(on_action_make_cone()));
	connect(ui.actionSphere, SIGNAL(triggered()), this, SLOT(on_action_make_sphere()));
	connect(ui.actionCylinder, SIGNAL(triggered()), this, SLOT(on_action_make_cylinder()));
	connect(ui.actionLumber, SIGNAL(triggered()), this, SLOT(on_action_make_lumber()));

	// operations
	connect(ui.actionFuse, SIGNAL(triggered()), this, SLOT(on_action_fuse()));
	connect(ui.actionCut, SIGNAL(triggered()), this, SLOT(on_action_cut()));

	// Help
	connect(ui.actionAbout, SIGNAL(triggered()), this, SLOT(about()));

	connect(ui.actionHelix, SIGNAL(triggered()), this, SLOT(on_action_compile()));

	connect(myOCCOpenGL->getHELM(), SIGNAL(sigAppendHELMCode(std::string)), this, SLOT(on_action_HELM_code(std::string)));
	connect(myOCCOpenGL->getHELM(), SIGNAL(sigReplaceHELMCode(QString)), this, SLOT(on_action_HELM_code(QString)));

	connect(DSL, SIGNAL(compiler_hints(QString)), this, SLOT(on_action_compiler_hints(QString)));
}


void OCCWindow::on_action_compiler_hints(QString line)
{
	outputResult(line);
	outputResult("Failed!");
}

void OCCWindow::createMenus(void)
{
}

void OCCWindow::createToolBars(void)
{
	QToolBar* aToolBar = addToolBar(tr("&Navigate"));
	aToolBar->setStyleSheet("QToolBar {background: rgb(255, 255, 255)}");
	aToolBar->addAction(ui.actionZoom);
	aToolBar->addAction(ui.actionPan);
	aToolBar->addAction(ui.actionRotate);

	aToolBar = addToolBar(tr("&View"));
	aToolBar->setStyleSheet("QToolBar {background: rgb(255, 255, 255)}");
	aToolBar->addAction(ui.actionReset);
	aToolBar->addAction(ui.actionFitAll);

	aToolBar = addToolBar(tr("&Primitive"));
	aToolBar->setStyleSheet("QToolBar {background: rgb(255, 255, 255)}");
	aToolBar->addSeparator();
	aToolBar->addAction(ui.actionLumber);
	aToolBar->addAction(ui.actionBox);
	aToolBar->addAction(ui.actionCone);
	aToolBar->addAction(ui.actionSphere);
	aToolBar->addAction(ui.actionCylinder);

	aToolBar->addSeparator();
	aToolBar->addAction(ui.actionFuse);
	aToolBar->addAction(ui.actionCut);

	aToolBar->addSeparator();
	aToolBar->addAction(ui.actionHelix);

	aToolBar = addToolBar(QString("drawing tools"));
	auto myDrawActions = myOCCOpenGL->getDrawActions();
	std::cout << myDrawActions->size() << std::endl;
	for (auto iter = myDrawActions->begin(); iter != myDrawActions->end(); iter++)
		aToolBar->addAction(*iter);

	
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

void OCCWindow::on_action_make_lumber()
{
	MyLumber* anPrimLumber = new MyLumber(MyLumber::StandardLumber::TWOBYFOUR, 1.0);

	TopoDS_Shape aTopoBox = BRepPrimAPI_MakeBox(anPrimLumber->width, anPrimLumber->height, anPrimLumber->length).Shape();
	Handle(AIS_Shape) anAisBox = new AIS_Shape(aTopoBox);

	anPrimLumber->BindGraphicShape(anAisBox);

	//myOCCOpenGL->CreateShape(anPrimLumber);

	anAisBox->SetColor(Quantity_NOC_GOLD);

	myOCCOpenGL->getContext()->Display(anAisBox, Standard_True);
}

void OCCWindow::on_action_make_box()
{
	Part::FeatureBox* box = new Part::FeatureBox;
	Handle(AIS_Shape) boxShape = box->BuildGraphicShape();
	boxShape->SetColor(Quantity_NOC_GOLD);
	myOCCOpenGL->createShape(box);
}

void OCCWindow::on_action_make_cylinder()
{
	Part::FeatureCylinder* cylinder = new Part::FeatureCylinder;
	if (cylinder->mustExecute()) cylinder->execute();
	Handle(AIS_Shape) cylinderShape = cylinder->BuildGraphicShape(); 
	cylinderShape->SetColor(Quantity_NOC_GOLD);
	myOCCOpenGL->createShape(cylinder);
}

void OCCWindow::on_action_make_cone()
{
	gp_Ax2 anAxis;
	anAxis.SetLocation(gp_Pnt(0.0, 0.0, 0.0));
	TopoDS_Shape aTopoCone = BRepPrimAPI_MakeCone(anAxis, 3.0, 0.0, 5.0).Shape();
	Handle(AIS_Shape) anAisCone = new AIS_Shape(aTopoCone);
	anAisCone->SetColor(Quantity_NOC_GOLD);
	myOCCOpenGL->getContext()->Display(anAisCone, Standard_True);
}

void OCCWindow::on_action_make_sphere()
{
	gp_Ax2 anAxis;
	anAxis.SetLocation(gp_Pnt(0.0, 0.0, 0.0));
	auto parametricSphere = BRepPrimAPI_MakeSphere(anAxis, 3.0);
	TopoDS_Shape aTopoSphere = parametricSphere.Shape();
	Handle(AIS_Shape) anAisSphere = new AIS_Shape(aTopoSphere);

	anAisSphere->SetColor(Quantity_NOC_GOLD);

	myOCCOpenGL->getContext()->Display(anAisSphere, Standard_True);
}


void OCCWindow::on_action_fuse()
{
	
}

void OCCWindow::on_action_cut()
{
	
}

void OCCWindow::on_action_compile()
{
	std::string sourceCode = editor->toPlainText().toStdString();
	outputResult("Source code file size: " + QString::number(sourceCode.length()/1024.0) + "kb.");
	outputResult("Start compiling...");
	
	if (DSL != nullptr) delete DSL;
	DSL = new OCCDomainLang();
	connect(DSL, SIGNAL(compiler_hints(QString)), this, SLOT(on_action_compiler_hints(QString)));

	bool isSucces = DSL->compile(sourceCode);
	
	if (!isSucces)
	{
		outputResult("Errors occurred during compilation.");
		return;
	}

	//outputResult("Successfully compiled.");

	vecShapes = convert_to_AIS_shape(*(DSL->carpentryPrimitives));

	myOCCOpenGL->getContext()->RemoveAll(true);
	for (auto& s : vecShapes)
	{
		myOCCOpenGL->getContext()->Display(s, Standard_True);
	}
}

void OCCWindow::on_action_HELM_code(QString helm)
{
	editor->clear();
	editor->appendPlainText(helm);
}

void OCCWindow::on_action_HELM_code(std::string helm)
{
	editor->appendPlainText(QString(helm.c_str()));
}