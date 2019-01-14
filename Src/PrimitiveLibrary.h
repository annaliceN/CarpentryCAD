#pragma once

#include <Eigen/Dense>

#include <AIS_Shape.hxx>
#include <BRepPrimAPI_MakeBox.hxx>

enum PrimType{
	Lumber,
	Box,
	Cylinder
};

class MyPrimitive
{
public:
	MyPrimitive() {};
	MyPrimitive(const TopoDS_Shape& _shape);
	~MyPrimitive() {};

	virtual double getVolume() { return 0; };
	PrimType getPrimitiveType() { return pType; };
	Eigen::Vector3d getPosition() { return position; }
	Handle(AIS_Shape) getGraphicShape() { return graphicShape; }
	const TopoDS_Shape& getShape() const { return graphicShape->Shape(); }

	void BindGraphicShape(Handle(AIS_Shape)& s)
	{ 
		graphicShape = s;
		baseShape = s->Shape();
	}

	PrimType pType;
	
	Handle(AIS_Shape) graphicShape;
	TopoDS_Shape baseShape;
	Eigen::Vector3d position;
};

class MyCylinder : public MyPrimitive
{
public:
	MyCylinder(double _r, double _h) : radius(_r), height(_h) 
	{
		pType = PrimType::Cylinder;
	};
	double getVolume() { return M_PI * radius * radius * height; };
	double radius;
	double height;
};

class MyBox : public MyPrimitive 
{
public:
	MyBox(double _w, double _l, double _h) : width(_w), length(_l), height(_h) 
	{
		pType = PrimType::Box;
	};

	double getVolume() { return width * length * height; };
	double width;
	double length;
	double height;
};

class MyLumber : public MyPrimitive
{
public:
	enum StandardLumber {
		ONEBYTWO,
		TWOBYTWO,
		ONEBYFOUR,
		TWOBYFOUR,
		TWOBYSIX
	};

	MyLumber(StandardLumber sl, double l) {
		pType = PrimType::Lumber;
		AssignStandard(sl);
		length = l;
	};

	void AssignStandard(StandardLumber sl)
	{
		lumberType = sl;

		switch (sl)
		{
		case MyLumber::ONEBYTWO:
			width  = 1.0;
			height = 2.0;
			minLength = 0.2;
			maxLength = 10.0;
			break;
		case MyLumber::TWOBYTWO:
			width  = 2.0;
			height = 2.0;
			minLength = 0.2;
			maxLength = 10.0;
			break;
		case MyLumber::ONEBYFOUR:
			width  = 1.0;
			height = 4.0;
			minLength = 0.2;
			maxLength = 10.0;
			break;
		case MyLumber::TWOBYFOUR:
			width  = 2.0;
			height = 4.0;
			minLength = 0.2;
			maxLength = 10.0;
			break;
		case MyLumber::TWOBYSIX:
			width  = 2.0;
			height = 6.0;
			minLength = 0.2;
			maxLength = 10.0;
			break;
		default:
			break;
		}
	}

	void UpdateStandard()
	{
		baseShape = BRepPrimAPI_MakeBox(this->width, this->height, 1.0).Shape();
		auto newShape = BRepPrimAPI_MakeBox(this->width, this->height, this->length).Shape();
		graphicShape->Set(newShape);
		graphicShape->Redisplay(Standard_True);
	}

	double width, height;
	double length;
	double minLength;
	double maxLength;
	int lumberType;
};