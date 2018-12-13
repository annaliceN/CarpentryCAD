#pragma once


struct Material {
	int materialType;

};

class LumberWood : public Material {
public:
	explicit LumberWood(double _l, double _w, double _h) : length(_l), width(_w), height(_h) {};

private:
	double length;
	double width;
	double height;
};

class PlyWood : public Material {
public:
	explicit PlyWood(double _l, double _w, double _t) : length(_l), width(_w), thickness(_t) {};

private:
	double length;
	double width;
	double thickness;
};

