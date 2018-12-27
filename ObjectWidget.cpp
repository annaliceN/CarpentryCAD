#include "objectwidget.h"

MyObjectWidget::MyObjectWidget(QWidget *parent /* = 0 */)
{
	ui.setupUi(this);
	ui.treeWidget->setHeaderLabel("CAD Operations");

	connect(ui.treeWidget, SIGNAL(itemSelectionChanged()), parent, SLOT(objSelected()));
	connect(ui.treeWidget, SIGNAL(itemClicked(QTreeWidgetItem*, int)), parent, SLOT(objChecked(QTreeWidgetItem*, int)));
	/*connect(ui.treeWidget, SIGNAL(itemPressed(QTreeWidgetItem*, int)), this, SLOT(objSelected()));*/
	connect(this, SIGNAL(keyPressSignals(QKeyEvent*)), parent, SLOT(keyPressSlot(QKeyEvent*)));

	setFocusPolicy(Qt::ClickFocus);
	ui.treeWidget->setFocusPolicy(Qt::ClickFocus);
	setFeatures(QDockWidget::DockWidgetFloatable | QDockWidget::DockWidgetMovable);
}

MyObjectWidget::~MyObjectWidget()
{

}

void MyObjectWidget::removeQTreeItemWidget(QTreeWidgetItem* item)
{

}

// void MyObjectWidget::objSelected()
// {
// 	QList<QTreeWidgetItem*> iterList = ui.treeWidget->selectedItems();
// 	if (iterList.empty()) return;
// 
// 	int objIndex = ui.treeWidget->indexOfTopLevelItem(iterList[0]);
// 	emit clickItem(objIndex);
// 	char objIndexStr[255];
// 	sprintf_s(objIndexStr, "%d", objIndex);
// }

void MyObjectWidget::keyPressEvent(QKeyEvent *event)
{
	emit keyPressSignals(event);
	// 	switch (event->key())
	// 	{
	// 	case Qt::Key_Delete:
	// 
	// 		break;
	// 	}
}

void MyObjectWidget::keyReleaseEvent(QKeyEvent *event)
{
	emit keyPressSignals(event);
}