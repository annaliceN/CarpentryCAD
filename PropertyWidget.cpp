
#include "PropertyWidget.h"
#include <QColorDialog>

using namespace std;

MyPropertyWidget::MyPropertyWidget(QWidget *parent /* = 0 */)
{
	ui.setupUi(this);

	ui.tableWidget->verticalHeader()->setVisible(false);

	setFocusPolicy(Qt::ClickFocus);
	ui.tableWidget->setFocusPolicy(Qt::ClickFocus);
	setFeatures(QDockWidget::DockWidgetFloatable | QDockWidget::DockWidgetMovable);

	connect(this, SIGNAL(VisibilityChangeSignal(void*)), parent, SLOT(primitiveVisibilityChanged(void*)));
	connect(this, SIGNAL(NormalDirectionChangeSignal(MyPrimitive*)), parent, SLOT(primitiveNormalChanged(MyPrimitive*)));
	connect(this, SIGNAL(MetalTypeChangeSignal(MyPrimitive*)), parent, SLOT(primitiveMetalTypeChanged(MyPrimitive*)));

	currentButton = NULL;
	currentCheckBox = NULL;
	curPrimitive = NULL;

	metalList.push_back(make_pair("Steel", 7.85));
	metalList.push_back(make_pair("Aluminum", 2.70));
	metalList.push_back(make_pair("Cast Iron", 7.90));
}

MyPropertyWidget::~MyPropertyWidget()
{

}

void MyPropertyWidget::WritePropertiesToPropWidget(MyPrimitive* primitive)
{
	Clear();
	curPrimitive = primitive;
	// set property widget
	switch (primitive->getPrimitiveType())
	{
	case PrimType::Box:
		WriteBoxProperties((MyBox*)primitive);
		break;
	case PrimType::Cylinder:
		//WriteCylinderProperties((MyCylinder*)primitive);
		break;
// 	case MyPrimitive::PRIMITIVETYPE::PARTIALCYLINDER:
// 		WritePartialCylinderProperties((MyPartialCylinder*)primitive);
// 		break;
// 	case MyPrimitive::PRIMITIVETYPE::PLANE:
// 		WritePlaneProperties((MyPlane*)primitive);
// 		break;
// 	case MyPrimitive::PRIMITIVETYPE::SPHERE:
// 		WriteSphereProperties((MySphere*)primitive);
// 		break;
// 	case MyPrimitive::PRIMITIVETYPE::TORUS:
// 		WriteTorusProperties((MyTorus*)primitive);
// 		break;
// 	case MyPrimitive::PRIMITIVETYPE::PARTIALTORUS:
// 		WritePartialTorusProperties((MyPartialTorus*)primitive);
// 		break;
// 	case MyPrimitive::PRIMITIVETYPE::BOX:
// 		WriteBoxProperties((MyBox*)primitive);
// 		break;
	default:
		break;
	}
}

void MyPropertyWidget::ShowColorSelection()
{
	buttonColor = QColorDialog::getColor(buttonColor);
	if (!buttonColor.isValid()) return;
	QPalette palette(buttonColor);
	currentButton->setPalette(palette);
// 	if (curPrimitive)
// 		curPrimitive->SetColor(Vector3D(buttonColor.red() / 255.0, buttonColor.green() / 255.0, buttonColor.blue() / 255.0));
// 	else if (mesh)
// 		mesh->color = Vector3D(buttonColor.red() / 255.0, buttonColor.green() / 255.0, buttonColor.blue() / 255.0);
// 	else if (corkMesh)
// 	{
// 		corkMesh->r = buttonColor.red() / 255.0;
// 		corkMesh->g = buttonColor.green() / 255.0;
// 		corkMesh->b = buttonColor.blue() / 255.0;
// 	}
}

void MyPropertyWidget::VisibilityChange(int state)
{
// 	if (curPrimitive) {
// 		curPrimitive->isVisible = (state == Qt::Checked);
// 		emit VisibilityChangeSignal(curPrimitive);
// 	}
// 	else if (mesh) {
// 		mesh->isVisible = (state == Qt::Checked);
// 		emit VisibilityChangeSignal(mesh);
// 	}
// 	else if (corkMesh) {
// 		corkMesh->isVisible = (state == Qt::Checked);
// 		emit VisibilityChangeSignal(corkMesh);
// 	}
}

void MyPropertyWidget::NormalDirectionChange(int idx)
{
// 	curPrimitive->normalOutward = idx == 0;
// 	emit NormalDirectionChangeSignal(curPrimitive);
}

void MyPropertyWidget::MetalTypeChange(int idx)
{
// 	auto iter = metalList.begin();
// 	for (int i = 0; i < idx; ++i) ++iter;
// 	curPrimitive->metalType = *iter;
// 	emit MetalTypeChangeSignal(curPrimitive);
}

void MyPropertyWidget::Clear()
{
	// clear existing properties
	for (auto iter = propItems.begin(); iter != propItems.end(); ++iter)
		delete (*iter);
	propItems.clear();
	/*ui.tableWidget->clearContents();*/
	while (ui.tableWidget->rowCount() > 0) ui.tableWidget->removeRow(0);
}

void MyPropertyWidget::InsertItem(QTableWidgetItem* item, int rowIndex, int columnIndex)
{
	ui.tableWidget->setItem(rowIndex, columnIndex, item);
	propItems.push_back(item);
}

void MyPropertyWidget::InsertMergeRow(QTableWidgetItem* item, int rowIndex)
{
	item->setBackgroundColor(QColor(190, 190, 190));
	item->setTextColor(QColor(255, 255, 255));
	ui.tableWidget->setItem(rowIndex, 0, item);
	ui.tableWidget->setSpan(rowIndex, 0, 1, ui.tableWidget->columnCount());
	propItems.push_back(item);
}

void MyPropertyWidget::WriteCylinderProperties(MyCylinder* cylinder)
{
	QTableWidget *tableWidget = ui.tableWidget;
	tableWidget->setRowCount(13);

	QTableWidgetItem *tableWidgetItem = new QTableWidgetItem("Parameters");
	InsertMergeRow(tableWidgetItem, 0);

	tableWidgetItem = new QTableWidgetItem("Radius");
	InsertItem(tableWidgetItem, 1, 0);
	tableWidgetItem = new QTableWidgetItem(std::to_string(cylinder->radius).c_str());
	InsertItem(tableWidgetItem, 1, 1);

	tableWidgetItem = new QTableWidgetItem("Height");
	InsertItem(tableWidgetItem, 2, 0);
	tableWidgetItem = new QTableWidgetItem(std::to_string(cylinder->height).c_str());
	InsertItem(tableWidgetItem, 2, 1);

	tableWidgetItem = new QTableWidgetItem("Position");
	InsertItem(tableWidgetItem, 3, 0);
// 	Vector3D bC = cylinder->GetPosition();
// 	tableWidgetItem = new QTableWidgetItem(("(" + std::to_string(bC.x) + ", " +
// 		std::to_string(bC.y) + ", " + std::to_string(bC.z) + ")").c_str());
// 	InsertItem(tableWidgetItem, 3, 1);

	tableWidgetItem = new QTableWidgetItem("Axis");
	InsertItem(tableWidgetItem, 4, 0);
// 	Vector3D axis(0, 0, 1);
// 	double m[16];
// 	GenIdentityMatrix(m); MultiRotateMatrix(m, cylinder->GetRotateAngle(), cylinder->GetRotateAxis());
// 	MultiMatrix(m, cylinder->localRotateMatrix);
// 	TransformVec(m, axis); axis.normalize();
// 	tableWidgetItem = new QTableWidgetItem(("(" + std::to_string(axis.x) + ", " +
// 		std::to_string(axis.y) + ", " + std::to_string(axis.z) + ")").c_str());
// 	InsertItem(tableWidgetItem, 4, 1);

	tableWidgetItem = new QTableWidgetItem("Appearance");
	InsertMergeRow(tableWidgetItem, 5);
	tableWidgetItem = new QTableWidgetItem("Color");
	InsertItem(tableWidgetItem, 6, 0);
	/*
	QPushButton *colorPalet = GenColorButton(cylinder->GetColor());
	tableWidget->setCellWidget(6, 1, colorPalet);
	tableWidgetItem = new QTableWidgetItem("Visible");
	InsertItem(tableWidgetItem, 7, 0);
	QCheckBox *checkBox = GenCheckBox(cylinder->isVisible);
	tableWidget->setCellWidget(7, 1, checkBox);

	tableWidgetItem = new QTableWidgetItem("Normal Outside");
	InsertItem(tableWidgetItem, 8, 0);
	QComboBox *comboBox = GenComboBox(cylinder->normalOutward);
	tableWidget->setCellWidget(8, 1, comboBox);

	tableWidgetItem = new QTableWidgetItem("Material");
	InsertMergeRow(tableWidgetItem, 9);
	tableWidgetItem = new QTableWidgetItem("Metal Type");
	InsertItem(tableWidgetItem, 10, 0);
	comboBox = GenMetalComboBox(cylinder->metalType.first);
	tableWidget->setCellWidget(10, 1, comboBox);
	tableWidgetItem = new QTableWidgetItem("Volume");
	InsertItem(tableWidgetItem, 11, 0);
	tableWidgetItem = new QTableWidgetItem(std::to_string(cylinder->getVolumn()).c_str());
	InsertItem(tableWidgetItem, 11, 1);
	tableWidgetItem = new QTableWidgetItem("Mass");
	InsertItem(tableWidgetItem, 12, 0);
	tableWidgetItem = new QTableWidgetItem(std::to_string(cylinder->getVolumn() * cylinder->metalType.second).c_str());
	InsertItem(tableWidgetItem, 12, 1);
	*/
}

void MyPropertyWidget::UpdateBoxProperties(MyBox* box)
{
	QTableWidget *tableWidget = ui.tableWidget;
	QTableWidgetItem *tableWidgetItem = new QTableWidgetItem(std::to_string(box->width).c_str());
	InsertItem(tableWidgetItem, 2, 1);
	tableWidgetItem = new QTableWidgetItem(std::to_string(box->height).c_str());
	InsertItem(tableWidgetItem, 3, 1);
	tableWidgetItem = new QTableWidgetItem(std::to_string(box->length).c_str());
	InsertItem(tableWidgetItem, 4, 1);
}

void MyPropertyWidget::WriteBoxProperties(MyBox* box)
{
	QTableWidget *tableWidget = ui.tableWidget;
	tableWidget->setRowCount(13);

	QTableWidgetItem *tableWidgetItem = new QTableWidgetItem("Parameters");
	InsertMergeRow(tableWidgetItem, 0);

	tableWidgetItem = new QTableWidgetItem("Center");
	InsertItem(tableWidgetItem, 1, 0);
	Eigen::Vector3d axis = box->getPosition();
	tableWidgetItem = new QTableWidgetItem(("(" + std::to_string(axis.x()) + ", " +
		std::to_string(axis.y()) + ", " + std::to_string(axis.z()) + ")").c_str());
	InsertItem(tableWidgetItem, 1, 1);

	tableWidgetItem = new QTableWidgetItem("Width");
	InsertItem(tableWidgetItem, 2, 0);
	tableWidgetItem = new QTableWidgetItem(std::to_string(box->width).c_str());
	InsertItem(tableWidgetItem, 2, 1);

	tableWidgetItem = new QTableWidgetItem("Height");
	InsertItem(tableWidgetItem, 3, 0);
	tableWidgetItem = new QTableWidgetItem(std::to_string(box->height).c_str());
	InsertItem(tableWidgetItem, 3, 1);

	tableWidgetItem = new QTableWidgetItem("Length");
 	InsertItem(tableWidgetItem, 4, 0);
 	tableWidgetItem = new QTableWidgetItem(std::to_string(box->length).c_str());
 	InsertItem(tableWidgetItem, 4, 1);

// 	QPushButton *colorPalet = GenColorButton(box->GetColor());
// 	tableWidget->setCellWidget(6, 1, colorPalet);
// 	tableWidgetItem = new QTableWidgetItem("Visible");
// 	InsertItem(tableWidgetItem, 7, 0);
// 	QCheckBox *checkBox = GenCheckBox(box->isVisible);
// 	tableWidget->setCellWidget(7, 1, checkBox);

	tableWidgetItem = new QTableWidgetItem("Length");
	InsertItem(tableWidgetItem, 5, 0);
	QSlider *slider = new QSlider(Qt::Horizontal);
	tableWidget->setCellWidget(5, 1, slider);
	connect(slider, SIGNAL(valueChanged(int)), this, SLOT(onLengthChanged(int)));

	tableWidgetItem = new QTableWidgetItem("Material");
	InsertMergeRow(tableWidgetItem, 9);
	tableWidgetItem = new QTableWidgetItem("Metal Type");
	InsertItem(tableWidgetItem, 10, 0);
	tableWidgetItem = new QTableWidgetItem("Volume");
	InsertItem(tableWidgetItem, 11, 0);
	tableWidgetItem = new QTableWidgetItem(std::to_string(box->getVolume()).c_str());
	InsertItem(tableWidgetItem, 11, 1);
	tableWidgetItem = new QTableWidgetItem("Mass");
	InsertItem(tableWidgetItem, 12, 0);
}

void MyPropertyWidget::onLengthChanged(int length)
{
	std::cout << "hello" << std::endl;
}

QPushButton* MyPropertyWidget::GenColorButton(Eigen::Vector3d color)
{
	currentButton = new QPushButton();
	currentButton->setFlat(true);
	currentButton->setAutoFillBackground(true);
	buttonColor = QColor(color.x() * 255, color.y() * 255, color.z() * 255);
	currentButton->setPalette(QPalette(buttonColor));

	connect(currentButton, SIGNAL(clicked()), this, SLOT(ShowColorSelection()));
	return currentButton;
}

QCheckBox* MyPropertyWidget::GenCheckBox(bool checked)
{
	currentCheckBox = new QCheckBox();
	currentCheckBox->setCheckState(checked ? Qt::Checked : Qt::Unchecked);
	connect(currentCheckBox, SIGNAL(stateChanged(int)), this, SLOT(VisibilityChange(int)));
	return currentCheckBox;
}

QComboBox* MyPropertyWidget::GenComboBox(bool outer)
{
	currentComboBox = new QComboBox();
	currentComboBox->addItem("Outside");
	currentComboBox->addItem("Inside");
	currentComboBox->setCurrentIndex(outer ? 0 : 1);

	connect(currentComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(NormalDirectionChange(int)));
	return currentComboBox;
}

QComboBox* MyPropertyWidget::GenMetalComboBox(std::string metalType)
{
	currentComboBox = new QComboBox();
	int initialIndex = -1, iterCnt = 0;
	for (auto iter = metalList.begin(); iter != metalList.end(); ++iter)
	{
		currentComboBox->addItem(iter->first.c_str());
		if (iter->first == metalType) initialIndex = iterCnt;
		++iterCnt;
	}
	currentComboBox->setCurrentIndex(initialIndex);

	connect(currentComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(MetalTypeChange(int)));
	return currentComboBox;
}