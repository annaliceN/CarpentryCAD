#pragma once
#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <stack>

#include <QObject>
#include <QString>

#include <AIS_Shape.hxx>

#include "PrimitiveLibrary.h"

class Object {
	std::string(*pp)(std::vector<std::string>&);
	std::string(*rr)();
	std::string value;
	std::string kind;
	MyPrimitive* primitive;
	std::vector<double> floatList;

public:
	Object() {};
	Object(std::string ss) { value = ss; kind = "variable"; pp = NULL; };
	Object(std::string(*p_)(std::vector<std::string>&)) { pp = p_; kind = "procedure"; value = ""; };
	Object(MyPrimitive* primitive_) { primitive = primitive_; kind = "primitive"; pp = NULL; }
	Object(std::vector<double> list_) { floatList = list_; kind = "float list"; pp = NULL; }

	std::string get_kind()
	{
		return kind;
	}
	
	std::string get_value()
	{
		return value;
	}
	
	MyPrimitive* get_primitive()
	{
		return primitive;
	}

	std::vector<double>& get_list()
	{
		return floatList;
	}
	
	std::string apply(std::vector<std::string>&V)
	{
		return pp(V);
	}
	
	std::string apply()
	{
		std::vector<std::string>V;
		return pp(V);
	}
};

typedef std::unordered_map<std::string, Object> Environment;

class PList {
	std::vector<std::string> store;

public:
	PList() {}
	PList(std::vector<std::string> vv) { store = vv; }
	void print() { for (std::size_t i = 0; i < store.size(); i++)std::cout << store[i] << " "; std::cout << std::endl; }
	std::string get_store() { std::string aux = ""; for (std::size_t i = 0; i < store.size(); i++) aux = aux + store[i] + " "; return aux; }
	void clear() { store.clear(); }

	std::size_t size() {
		std::size_t nn = 0, flag = 0;
		std::size_t left = 0, right = 0;
		if (store.size() == 0)return nn;
		if (store.size() == 1) { nn = 1; return nn; }

		for (size_t i = 1; i < store.size() - 1; i++) 
		{
			if (store[i] == "(")left++;
			if (store[i] == ")")right++;

			if (left == 0 && left == right)nn++;
			else if (left == 1 && flag == 0) { nn++; flag = 1; }	// content between parentness regards 1
			else if (left != 0 && left == right) { flag = 0; left = 0; right = 0; }
		}
		return nn;
	}

	// Get the no. 
	PList get(size_t pos) {
		PList pp;
		if (store.size() == 1) { pp = *this; }
		size_t nn = 0, flag = 0, flag_read = 0;
		size_t left = 0, right = 0;
		for (size_t i = 1; i < store.size() - 1; i++) {
			if (store[i] == "(")left++;
			if (store[i] == ")")right++;

			if (left == 0 && left == right) {
				nn++;
				if (pos == nn - 1) {
					pp.store.push_back(store[i]);
					break;
				}
			}
			else if (left == 1 && flag == 0) {
				nn++; flag = 1;
				if (pos == nn - 1) {
					flag_read = 1;
				}
			}
			else if (left != 0 && left == right) { flag = 0; left = 0; right = 0; flag_read = 0; }
			else if (flag_read == 1)pp.store.push_back(store[i]);
		}
		if (pp.store.size() == 1)  return pp;
		else {
			PList cc;
			cc.store.push_back("(");
			for (size_t ii = 0; ii < pp.full_size(); ii++)cc.store.push_back(pp.store[ii]);
			cc.store.push_back(")");
			return cc;
		}
	}

	size_t full_size() {
		return store.size();
	}

	std::string elem(size_t pos) {
		std::string inp = store[pos];
		//Clean all trailing empty spaces
		while (inp[inp.size() - 1] == ' ')inp.erase(inp.size() - 1); //clean some empty spaces
		return inp;
	}

	void puts(std::string ss) { store.push_back(ss); }

	~PList() { store.clear(); }
};


class OCCDomainLang : public QObject
{
	Q_OBJECT
public:
	OCCDomainLang();
	~OCCDomainLang();
	std::string eval(PList & pp, Environment & env);
	bool compile(std::string & sourceCode);

	std::unordered_set<MyPrimitive*>* carpentryPrimitives;

signals:
	void compiler_hints(QString line);

private:
	const double CHOPSAW_WIDTH_MIN = 0.2;
	const double CHOPSAW_WIDTH_MAX = 0.8;
	const double CHOPSAW_HEIGHT = 0.8;
};

