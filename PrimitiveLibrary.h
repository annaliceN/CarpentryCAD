#pragma once

#include <Eigen/Dense>

#include <AIS_Shape.hxx>

enum PrimType{
	Cylinder,
	Box
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
	
	void BindGraphicShape(Handle(AIS_Shape)& s) { graphicShape = s; }

	PrimType pType;
	
	Handle(AIS_Shape) graphicShape;
	TopoDS_Shape shape;
	Eigen::Vector3d position;
};

class MyCylinder : public MyPrimitive
{
public:
	MyCylinder(double _r, double _h) : radius(_r), height(_h) {
		pType = PrimType::Cylinder;
	};
	double getVolume() { return M_PI * radius * radius * height; };
	double radius;
	double height;
};

class MyBox : public MyPrimitive 
{
public:
	MyBox(double _w, double _l, double _h) : width(_w), length(_l), height(_h) {
		pType = PrimType::Box;
	};

	double getVolume() { return width * length * height; };
	double width;
	double length;
	double height;

};