#ifndef OCCWINDOW_H
#define OCCWINDOW_H

#include "ui_MainWindow.h"


#include <TopoDS.hxx>
#include <AIS_Shape.hxx>
#include <AIS_InteractiveContext.hxx>

#include <memory>
#include <exception>

#include "Highlighter.h"
#include "CodeEditor.h"
#include "OCCDomainLang.h"
#include "PropertyWidget.h"


class OCCOpenGL;

class OCCWindow : public QMainWindow
{
    Q_OBJECT

public:
	OCCWindow(QWidget *parent = 0);
    ~OCCWindow();

	void setup_editor();

	void intialize_widget();

    void createActions(void);

    void createMenus(void);

    void createToolBars(void);
	
	void outputResult(const QString & res);

private slots:
    void about(void);

	void on_action_compile();

	void on_action_make_lumber();

	void on_action_make_box();

	void on_action_make_cone();

	void on_action_make_sphere();

	void on_action_make_cylinder();

	void on_action_fuse();

	void on_action_cut();
	
	void on_action_compiler_hints(QString line);

	void on_action_HELM_code(QString helm);

	void on_action_HELM_code(std::string helm);

private:
    Ui::occQtClass ui;

	OCCOpenGL* myOCCOpenGL;
	CodeEditor *editor;
	QTextEdit *outputEditor;
	Highlighter *highlighter;
	OCCDomainLang *DSL;

	// primitives
	TopoDS_Shape aTopoBox;
	std::vector<Handle(AIS_Shape)> vecShapes;
};

#endif
