#pragma once

enum MaterialType {
	LumberMaterial,
	PlyMaterial
};

struct Material {
	int materialType;

};

class LumberWood : public Material {
public:
	LumberWood(double _l, double _w, double _h) : length(_l), width(_w), height(_h) 
	{
		materialType = MaterialType::LumberMaterial;
	};
	
	double length;
	double width;
	double height;
};

class PlyWood : public Material {
public:
	PlyWood(double _l, double _w, double _t) : length(_l), width(_w), thickness(_t) 
	{
		materialType = MaterialType::PlyMaterial;
	};

	double length;
	double width;
	double thickness;
};

