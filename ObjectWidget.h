#ifndef	OBJECTWIDGET_H
#define OBJECTWIDGET_H

#include "ui_objectwidget.h"

#include <QKeyEvent>
#include <QTreeWidgetItem>

class MyObjectWidget : public QTreeWidget
{
	Q_OBJECT

public:
	MyObjectWidget(QWidget *parent = 0);
	~MyObjectWidget();

	void startDrag(Qt::DropActions supportedActions);

	void dragMoveEvent(QDragMoveEvent * event);

	void dropEvent(QDropEvent * event);

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
	void recomputeSignal();

};

#endif