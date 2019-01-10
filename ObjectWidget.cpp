#include "objectwidget.h"

MyObjectWidget::MyObjectWidget(QWidget *parent /* = 0 */) :
	QTreeWidget(parent)
{
	setHeaderLabel("Tree");
	setSelectionMode(QAbstractItemView::SingleSelection);
	setDragEnabled(true);
	setAcceptDrops(true);
	setDropIndicatorShown(true);

	setDragDropMode(QAbstractItemView::InternalMove);

	connect(this, SIGNAL(itemPressed(QTreeWidgetItem*, int)), parent, SLOT(objSelected()));
	connect(this, SIGNAL(keyPressSignals(QKeyEvent*)), parent, SLOT(keyPressSlot(QKeyEvent*)));
	connect(this, SIGNAL(recomputeSignal()), parent, SLOT(RecomputeTree()));
	setFocusPolicy(Qt::ClickFocus);
}

MyObjectWidget::~MyObjectWidget()
{

}


void MyObjectWidget::startDrag(Qt::DropActions supportedActions)
{
	Q_UNUSED(supportedActions);
	QTreeWidgetItem *item = currentItem();
	if (item == NULL)
		return;
	if (item->text(0) == "-----------")
	{
		QTreeWidget::startDrag(supportedActions);
	}
}

void MyObjectWidget::dragMoveEvent(QDragMoveEvent *event)
{
	QTreeWidgetItem *hHitTest = itemAt(event->pos());
	if (hHitTest) event->ignore();
	else
	{
		QTreeWidget::dragMoveEvent(event);
	}
	event->accept();
	
}

void MyObjectWidget::dropEvent(QDropEvent *event)
{
	if (event->source() == this && (event->dropAction() == Qt::MoveAction ||
		dragDropMode() == QAbstractItemView::InternalMove)) 
	{
		QTreeWidgetItem* itemOver = itemAt(event->pos());
		QModelIndex indexOver = indexAt(event->pos());
		QTreeWidgetItem* itemCur = currentItem();
		QModelIndex curIndex = currentIndex();
		
		if (itemOver == nullptr || itemCur == nullptr) return;
		takeTopLevelItem(curIndex.row());
		insertTopLevelItem(indexOver.row(), itemCur);
		
		event->accept();
		// Don't want QAbstractItemView to delete it because it was "moved" we already did it
		event->setDropAction(Qt::CopyAction);

		emit recomputeSignal();
	}

	QTreeView::dropEvent(event);
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