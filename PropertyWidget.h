#ifndef PROPERTYWIDGET_H
#define PROPERTYWIDGET_H

#include <list>

#include <QDockWidget>
#include <QPushButton>
#include <QColor>
#include <QPalette>
#include <QCheckBox>
#include <QComboBox>
#include <QTableWidgetItem>

#include "ui_PropertyWidget.h"

#include "PrimitiveLibrary.h"

class MyPropertyWidget : public QDockWidget
{
	Q_OBJECT

public:
	MyPropertyWidget(QWidget *parent = 0);
	~MyPropertyWidget();

signals:
	void VisibilityChangeSignal(void*);
	void NormalDirectionChangeSignal(MyPrimitive*);
	void MetalTypeChangeSignal(MyPrimitive*);

public slots:
	void WritePropertiesToPropWidget(MyPrimitive* primitive);
	//void WritePropertiesToPropWidget(CMesh *mesh_);
	//void WritePropertiesToPropWidget(CorkTriMesh *corkMesh_);
	void ShowColorSelection();
	void VisibilityChange(int state);
	void NormalDirectionChange(int idx);
	void MetalTypeChange(int idx);
	void Clear();
	void onLengthChanged(int length);

private:
	void InsertItem(QTableWidgetItem* item, int rowIndex, int columnIndex);
	void InsertMergeRow(QTableWidgetItem* item, int rowIndex);

	void WriteCylinderProperties(MyCylinder* cylinder);
	void UpdateBoxProperties(MyBox * box);
	// 	void WritePartialCylinderProperties(MyPartialCylinder* cylinder);
	// 	void WriteConeProperties(MyCone* cone);
	// 	void WritePlaneProperties(MyPlane* plane);
	// 	void WriteSphereProperties(MySphere* sphere);
	// 	void WriteTorusProperties(MyTorus* torus);
	// 	void WritePartialTorusProperties(MyPartialTorus* torus);
	void WriteBoxProperties(MyBox* box);

	QPushButton* GenColorButton(Eigen::Vector3d color);
	QCheckBox* GenCheckBox(bool checked);
	QComboBox* GenComboBox(bool outer);
	QComboBox* GenMetalComboBox(std::string metalType);

public:
	Ui::PropertyWidget ui;
	std::list<QTableWidgetItem*> propItems;
	QPushButton *currentButton;
	QColor buttonColor;
	QCheckBox *currentCheckBox;
	QComboBox *currentComboBox;
	MyPrimitive *curPrimitive;
	std::list< std::pair<std::string, double> > metalList;
};
#endif
