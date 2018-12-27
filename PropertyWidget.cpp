
#include "PropertyWidget.h"
#include <QColorDialog>
#include <QDoubleSpinBox>

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
	connect(this, SIGNAL(LumberTypeChangeSignal(MyPrimitive*)), parent, SLOT(primitiveMetalTypeChanged(MyPrimitive*)));
	connect(this, SIGNAL(sigRedraw()), parent, SLOT(slotRedraw()));
	connect(this, SIGNAL(sigLumberLengthChanged(MyLumber*, double)), parent, SLOT(onLumberLengthChanged(MyLumber*, double)));

	currentButton = NULL;
	currentCheckBox = NULL;
	curPrimitive = NULL;

	// Load lumber library
	lumberList.push_back("1 x 2");
	lumberList.push_back("2 x 2");
	lumberList.push_back("1 x 4");
	lumberList.push_back("2 x 4");
	lumberList.push_back("2 x 6");
}

MyPropertyWidget::~MyPropertyWidget()
{

}

void MyPropertyWidget::WritePropertiesToPropWidget(Part::FeaturePrimitive* primitive)
{
	Clear();
	curPrimitive = primitive;
	// set property widget
	const auto primName = primitive->getViewProviderName();
	
	if (primName == "Box")
	{
		WriteBoxProperties(static_cast<Part::FeatureBox*>(primitive));
	}

}


void MyPropertyWidget::WriteBoxProperties(Part::FeatureBox* box)
{
	QTableWidget *tableWidget = ui.tableWidget;
	tableWidget->setRowCount(13);

	QTableWidgetItem *tableWidgetItem = new QTableWidgetItem("Parameters");
	InsertMergeRow(tableWidgetItem, 0);

	int nRow = 1;

	if (box->getPlacement()->getTypeId() == Part::PropertyPlacement::getClassTypeId())
	{
		Part::PropertyPlacement* placement = box->getPlacement();
		Base::Placement pl = placement->getValue();

		Base::Vector3d pos = pl.getPosition();
		for (int i = 0; i < 3; ++i)
		{
			tableWidgetItem = new QTableWidgetItem(QString("Position ")+ ('X'+i));
			InsertItem(tableWidgetItem, nRow, 0);

			QDoubleSpinBox* spinBox = new QDoubleSpinBox;
			spinBox->setValue(pos[i]);
			tableWidget->setCellWidget(nRow, 1, spinBox);

			connect(spinBox, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),
				[=](double val) {
				auto _plc = placement->getValue();
				Base::Vector3d _pos = _plc.getPosition();
				_pos[i] = val;
				_plc.setPosition(_pos);
				placement->setValue(_plc);
				curPrimitive->onChanged(placement);
				emit sigRedraw();
			});

			++nRow;
		}
		
		Base::Rotation rot = pl.getRotation();
		const std::string rotTitle[3] = { "Yaw", "Pitch", "Roll" };
		double ypr[3] = { 0, 0, 0 };
		rot.getYawPitchRoll(ypr[0], ypr[1], ypr[2]);
		for (int i = 0; i < 3; ++i)
		{
			tableWidgetItem = new QTableWidgetItem(rotTitle[i].c_str());
			InsertItem(tableWidgetItem, nRow, 0);
			QDoubleSpinBox* spinBox = new QDoubleSpinBox;
			spinBox->setValue(ypr[i]);
			tableWidget->setCellWidget(nRow, 1, spinBox);
		
			connect(spinBox, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),
				[=](double val) {
				double _ypr[3] = { 0, 0, 0 };
				auto _plc = placement->getValue();
				Base::Rotation _rot = _plc.getRotation();
				_rot.getYawPitchRoll(_ypr[0], _ypr[1], _ypr[2]);
				_ypr[i] = val;
				_rot.setYawPitchRoll(_ypr[0], _ypr[1], _ypr[2]);
				_plc.setRotation(_rot);
				placement->setValue(_plc);
				curPrimitive->onChanged(placement);
				emit sigRedraw();
			});

			++nRow;
		}
	}

	std::vector< App::Property*> pList;
	box->getPropertyList(pList);

	for (auto & p : pList)
	{
		if (p->getTypeId() == App::PropertyFloat::getClassTypeId())
		{
			App::PropertyFloat* pf = static_cast<App::PropertyFloat*>(p);
			tableWidgetItem = new QTableWidgetItem(pf->getName());
			InsertItem(tableWidgetItem, nRow, 0);

			QDoubleSpinBox* spinBox = new QDoubleSpinBox;
			spinBox->setValue(pf->getValue());
			tableWidget->setCellWidget(nRow, 1, spinBox);

			connect(spinBox, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),
				[=](double val) {
				pf->setValue(val);
				std::cout << pf->getValue() << std::endl;
				curPrimitive->onChanged(pf);
				emit sigRedraw();
			});
			++nRow;
		}
	}
}


void MyPropertyWidget::ShowColorSelection()
{
	buttonColor = QColorDialog::getColor(buttonColor);
	if (!buttonColor.isValid()) return;
	QPalette palette(buttonColor);
	currentButton->setPalette(palette);
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

void MyPropertyWidget::LumberTypeChange(int idx)
{
	/*
	std::cout << "type change" << std::endl;
	auto curLumber = static_cast<MyLumber*>(curPrimitive);
	curLumber->AssignStandard(static_cast<MyLumber::StandardLumber>(idx));
	curLumber->UpdateStandard();
	emit sigRedraw();*/
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

void MyPropertyWidget::WriteLumberProperties(MyLumber *lumber)
{
	QTableWidget *tableWidget = ui.tableWidget;
	tableWidget->setRowCount(13);

	QTableWidgetItem *tableWidgetItem = new QTableWidgetItem("Parameters");
	InsertMergeRow(tableWidgetItem, 0);

	tableWidgetItem = new QTableWidgetItem("Center");
	InsertItem(tableWidgetItem, 1, 0);
	Eigen::Vector3d axis = lumber->getPosition();
	tableWidgetItem = new QTableWidgetItem(("(" + std::to_string(axis.x()) + ", " +
		std::to_string(axis.y()) + ", " + std::to_string(axis.z()) + ")").c_str());
	InsertItem(tableWidgetItem, 1, 1);

	tableWidgetItem = new QTableWidgetItem("Width");
	InsertItem(tableWidgetItem, 2, 0);
	tableWidgetItem = new QTableWidgetItem(std::to_string(lumber->width).c_str());
	InsertItem(tableWidgetItem, 2, 1);

	tableWidgetItem = new QTableWidgetItem("Height");
	InsertItem(tableWidgetItem, 3, 0);
	tableWidgetItem = new QTableWidgetItem(std::to_string(lumber->height).c_str());
	InsertItem(tableWidgetItem, 3, 1);

	tableWidgetItem = new QTableWidgetItem("Length");
	InsertItem(tableWidgetItem, 4, 0);
	tableWidgetItem = new QTableWidgetItem(std::to_string(lumber->length).c_str());
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

	slider->setMaximum(100);
	slider->setMinimum(1);
	slider->setValue(lumber->length / (lumber->maxLength - lumber->minLength) * 100.0);

	tableWidget->setCellWidget(5, 1, slider);
	connect(slider, SIGNAL(valueChanged(int)), this, SLOT(onLengthChanged(int)));

	tableWidgetItem = new QTableWidgetItem("Material");
	InsertMergeRow(tableWidgetItem, 9);
	tableWidgetItem = new QTableWidgetItem("Lumber Type");
	InsertItem(tableWidgetItem, 10, 0);
	auto comboBox = GenLumberComboBox(lumberList[lumber->lumberType]);
	tableWidget->setCellWidget(10, 1, comboBox);
	tableWidgetItem = new QTableWidgetItem("Volume");
	InsertItem(tableWidgetItem, 11, 0);
	tableWidgetItem = new QTableWidgetItem(std::to_string(lumber->getVolume()).c_str());
	InsertItem(tableWidgetItem, 11, 1);
	tableWidgetItem = new QTableWidgetItem("Mass");
	InsertItem(tableWidgetItem, 12, 0);
}

void MyPropertyWidget::onLengthChanged(int length)
{
	/*
	MyLumber *curLumber = static_cast<MyLumber*>(curPrimitive);
	double newLength = length / 100.0 * (curLumber->maxLength - curLumber->minLength);
	curLumber->length = newLength;
	ui.tableWidget->item(4, 1)->setText(std::to_string(curLumber->length).c_str());

	emit sigLumberLengthChanged(curLumber, newLength);*/
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

QComboBox* MyPropertyWidget::GenLumberComboBox(std::string metalType)
{
	currentComboBox = new QComboBox();
	int initialIndex = -1, iterCnt = 0;
	for (auto iter = lumberList.begin(); iter != lumberList.end(); ++iter)
	{
		currentComboBox->addItem(iter->c_str());
		if (*iter == metalType) initialIndex = iterCnt;
		++iterCnt;
	}
	currentComboBox->setCurrentIndex(initialIndex);

	connect(currentComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(LumberTypeChange(int)));
	return currentComboBox;
}