#ifndef	OBJECTWIDGET_H
#define OBJECTWIDGET_H

#include "ui_objectwidget.h"

#include <QKeyEvent>
#include <QTreeWidgetItem>

class MyObjectWidget : public QDockWidget
{
	Q_OBJECT

public:
	MyObjectWidget(QWidget *parent = 0);
	~MyObjectWidget();

	void removeQTreeItemWidget(QTreeWidgetItem* item);

	// signals:
	// 	void clickItem(int);


	// public slots:
	// 	void objSelected();

protected:
	void keyPressEvent(QKeyEvent *event);
	void keyReleaseEvent(QKeyEvent *event);

signals:
	void keyPressSignals(QKeyEvent *event);

public:
	Ui::ObjectWidget ui;
};

#endif