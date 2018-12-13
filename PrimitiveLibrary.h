#pragma once

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

	PrimType pType;
	TopoDS_Shape shape;
};

class MyClinder : public MyPrimitive
{
public:
	MyClinder(double _r, double _h) : radius(_r), height(_h) {
		pType = PrimType::Cylinder;
	};

	double radius;
	double height;
};

class MyBox : public MyPrimitive 
{
public:
	MyBox(double _w, double _l, double _h) : width(_w), length(_l), height(_h) {
		pType = PrimType::Box;
	};

	
	double width;
	double length;
	double height;

};