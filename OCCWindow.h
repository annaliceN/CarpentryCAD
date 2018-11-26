#ifndef OCCWINDOW_H
#define OCCWINDOW_H

#include "ui_occQt.h"

#include "highlighter.h"
#include "codeeditor.h"

#include <TopoDS.hxx>
#include <AIS_Shape.hxx>
#include <AIS_InteractiveContext.hxx>

#include <memory>
#include <exception>

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

private:
    Ui::occQtClass ui;

	OCCOpenGL* myOCCOpenGL;
	CodeEditor *editor;
	QTextEdit *outputEditor;
	Highlighter *highlighter;

	// primitives
	TopoDS_Shape aTopoBox;

	std::vector<Handle(AIS_Shape)> vecShapes;
};

#endif
