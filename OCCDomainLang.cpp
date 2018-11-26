#include "OCCDomainLang.h"
#include "procedures.h"

#include <gp_Circ.hxx>
#include <gp_Elips.hxx>
#include <gp_Pln.hxx>

#include <gp_Lin2d.hxx>

#include <Geom_ConicalSurface.hxx>
#include <Geom_ToroidalSurface.hxx>
#include <Geom_CylindricalSurface.hxx>

#include <GCE2d_MakeSegment.hxx>

#include <TopExp.hxx>
#include <TopExp_Explorer.hxx>
#include <TColgp_Array1OfPnt2d.hxx>

#include <BRepLib.hxx>

#include <BRepBuilderAPI_MakeVertex.hxx>
#include <BRepBuilderAPI_MakeEdge.hxx>
#include <BRepBuilderAPI_MakeWire.hxx>
#include <BRepBuilderAPI_MakeFace.hxx>
#include <BRepBuilderAPI_Transform.hxx>
#include <BRepBuilderAPI_MakePolygon.hxx>

#include <BRepPrimAPI_MakeBox.hxx>
#include <BRepPrimAPI_MakeCone.hxx>
#include <BRepPrimAPI_MakeSphere.hxx>
#include <BRepPrimAPI_MakeCylinder.hxx>
#include <BRepPrimAPI_MakeTorus.hxx>
#include <BRepPrimAPI_MakePrism.hxx>
#include <BRepPrimAPI_MakeRevol.hxx>

#include <BRepFilletAPI_MakeFillet.hxx>
#include <BRepFilletAPI_MakeChamfer.hxx>

#include <BRepOffsetAPI_MakePipe.hxx>
#include <BRepOffsetAPI_ThruSections.hxx>

#include <BRepAlgoAPI_Cut.hxx>
#include <BRepAlgoAPI_Fuse.hxx>
#include <BRepAlgoAPI_Common.hxx>

#include <gp_Trsf.hxx>

bool show_err1_flag = true;
bool show_err2_flag = true;

MyPrimitive* make_box(double W, double H, double L)
{
	return new MyPrimitive(BRepPrimAPI_MakeBox(W, H, L).Shape());
}

MyPrimitive* make_cylinder(double R, double H)
{
	return new MyPrimitive(BRepPrimAPI_MakeCylinder(R, H).Shape());
}

void translate_prmt(MyPrimitive* p, double x, double y, double z)
{
	gp_Trsf aTrsf;
	aTrsf.SetTranslation(gp_Vec(x, y, z));
	BRepBuilderAPI_Transform aTransform(p->shape, aTrsf);
	p->shape = aTransform.Shape();
}

template < class T>
void to_number(T &value, const std::string &s) {
	std::stringstream ss(s);
	ss >> value;
}

bool number(std::string inp) {
	char *pend;
	double tst = strtod(inp.c_str(), &pend);
	if (inp[0] != (*pend))return true;
	else return false;
}

void replace_substr(std::string &inp, std::string target, std::string res) {
	size_t ind = 0;
	for (;;) {
		ind = inp.find(target, ind);
		if (ind == std::string::npos)break;
		inp.replace(ind, 1, res);
		ind += res.length();
	}
}

std::vector<std::string> clean_input(std::string &str) {
	replace_substr(str, "(", " ( ");
	replace_substr(str, ")", " ) ");

	std::stringstream ss(str);
	std::string aux;
	std::vector<std::string>init_pieces;
	while (ss >> aux) init_pieces.push_back(aux);	// convert the std::string into split ones

	if (str.find("\"") == std::string::npos) return init_pieces;

	std::vector<std::string>pieces;
	aux = "";
	int flag = 0;
	for (size_t i = 0; i < init_pieces.size(); i++) 
	{
		if (init_pieces[i].find("\"") != std::string::npos) 
		{
			if (init_pieces[i][0] == '\"' && init_pieces[i][init_pieces[i].size() - 1] == '\"') {
				pieces.push_back(init_pieces[i]);
				continue;
			}
			flag = 1;
			if (init_pieces[i][0] == '\"')aux = init_pieces[i] + " ";
			else if (init_pieces[i][init_pieces[i].size() - 1] == '\"') 
			{
				aux += init_pieces[i] + " ";
				pieces.push_back(aux);
				aux = "";
				flag = 0;
			}
			else
				aux += init_pieces[i] + " ";
		}
		else
			if (flag == 0) 
			{
				pieces.push_back(init_pieces[i]);
			}
			else
				aux += init_pieces[i] + " ";
	}
	return pieces;
}


std::string get_input(std::string& sourceCode)
{
	string inp;
	char tmp;
	int left = 0, right = 0, ll = 0;
	for (auto s : sourceCode)
	{
		tmp = s;
		
		if (tmp == ';')
		{
			while (tmp != '\n')cin.get(tmp);
		}
		if (tmp != '\n')inp += tmp;
		if (tmp == '(')left++;
		else if (tmp == ')')right++;
		else if (tmp == '\"')ll++;

		if (tmp == '\n') 
		{
			if (left == 0 && right == 0 && ll == 0)break;
			if (left != 0 && (left == right) && (ll == 0 || (ll & 1) == 0))break;
			if (left == 0 && right == 0 && (ll & 1) == 0)break;
			if ((ll & 1))inp += "\\n";
			else inp += " ";
		}
	}
	return inp;
}

std::vector<std::string> split_codes(std::string& sourceCode)
{
	std::stack<bool> parStack;
	std::vector<std::string> processedCode;
	for (int i = 0; i < sourceCode.size(); ++i)
	{
		std::string singleCode;
		if (sourceCode[i] == '(')
		{
			parStack.push(1);
			singleCode.push_back(sourceCode[i]);
			while (!parStack.empty())
			{
				++i;
				singleCode.push_back(sourceCode[i]);
				if (sourceCode[i] == '(') parStack.push(1);
				if (sourceCode[i] == ')') parStack.pop();
			}
			processedCode.push_back(singleCode);
		}
	}
	return processedCode;
}

OCCDomainLang::OCCDomainLang()
{
	carpentryPrimitives = new std::unordered_set<MyPrimitive*>();
}


OCCDomainLang::~OCCDomainLang()
{
}

std::string OCCDomainLang::eval(PList &pp, Environment &env)
{
	int N = pp.size();
	std::cout << "N = " << N << std::endl;
	if (N == 1) { //Check for symbol, constant literal, procedure with no argument
		if (pp.elem(0) == "(" &&  pp.elem(pp.full_size() - 1) == ")") {
			PList aux = pp.get(0); std::string inp = aux.elem(0);
			//Check for procedure with no argument, e.g. (quit)
			if (env.find(inp) != env.end()) {
				return env[inp].apply();
			}
			else {
				return(("Error! Unbound variable: " + inp));
			}
		}
		else {
			std::string inp = pp.elem(0);
			//Check if character
			if (inp[0] == '#' && inp[1] == '\\') return "character type not yet implemented";
			//Check if std::string
			if (inp[0] == '\"' && inp[inp.size() - 1] == '\"')return inp;
			//Check if number
			if (number(inp)) return inp;
			//Check if variable or procedure
			if (env.find(inp) != env.end()) 
			{
				if (env[inp].get_kind() == "variable") return env[inp].get_value();
				else 
				{
					if (show_err1_flag)cout << env[inp].get_kind() << " ";
					show_err1_flag = true;
					return inp;
				}
			}
			else 
			{
				std::string res;
				if (show_err2_flag)res = "Error! Unbound variable: " + inp;
				show_err2_flag = true;
				return res;
			}
		}
	}
	else {
		PList aux = pp.get(0); std::string proc = aux.elem(0);
		show_err1_flag = false;
		show_err2_flag = false;
		if (proc == "quote") { return ((proc + " - not yet implemented!")); }
		else if (proc == "if") { return ((proc + " - not yet implemented!")); }
		else if (proc == "cylinder") { 
			if (pp.size() != 4) return ("ill expression");
			else
			{
				std::string name = pp.get(1).elem(0);
				PList radius = pp.get(2);
				PList height = pp.get(3);
				std::string valueRadius = eval(radius, env);
				std::string valueHeight = eval(height, env);
				double R = QString(valueRadius.c_str()).toDouble();
				double H = QString(valueHeight.c_str()).toDouble();
 				MyPrimitive* creatingPrimitive = make_cylinder(R, H);
				
				carpentryPrimitives->insert(creatingPrimitive);
				env[name] = Object(creatingPrimitive);
				return "";
			}
		}
		else if (proc == "box") 
		{
			if (pp.size() != 5) return ("ill expression");
			else
			{
				std::string name = pp.get(1).elem(0);
				PList width = pp.get(2);
				PList height = pp.get(3);
				PList depth = pp.get(4);
				std::string valueWidth = eval(width, env);
				std::string valueHeight = eval(height, env);
				std::string valueDepth = eval(depth, env);
				double W = QString(valueWidth.c_str()).toDouble();
				double H = QString(valueHeight.c_str()).toDouble();
				double L = QString(valueDepth.c_str()).toDouble();
				MyPrimitive* creatingPrimitive = make_box(W, H, L);

				carpentryPrimitives->insert(creatingPrimitive);
				env[name] = Object(creatingPrimitive);

				return "";
			}
		}
		else if (proc == "translate")
		{
			if (pp.size() != 5) return ("ill expression");
			std::string name = pp.get(1).elem(0);
			if (env.find(name) != env.end())
			{
				auto pmt = env[name].get_primitive();
				PList x = pp.get(2);
				PList y = pp.get(3);
				PList z = pp.get(4);
				double valueX = QString::fromStdString(eval(x, env)).toDouble();
				double valueY = QString::fromStdString(eval(y, env)).toDouble();
				double valueZ = QString::fromStdString(eval(z, env)).toDouble();
				translate_prmt(pmt, valueX, valueY, valueZ);
				//pmt->SetTranslation(Vector3D(valueX, valueY, valueZ));
			}	
			return "";
		}
		else if (proc == "set!") { return ((proc + " - not yet implemented!")); }
		else if (proc == "if") { return ((proc + " - not yet implemented!")); }
		else if (proc == "lambda") { return ((proc + " - not yet implemented!")); }
		else if (proc == "begin") { return ((proc + " - not yet implemented!")); }
		else
		{
			PList exps; exps.puts("(");
			for (int i = 0; i < N; i++) {
				PList piece = pp.get(i);
				std::string aux = eval(piece, env);
				if (aux == "")aux = (piece.get(0)).elem(0);
				exps.puts(aux);
			}
			exps.puts(")");
			std::string pr = (exps.get(0)).elem(0);
			std::vector<std::string>args;
			for (int i = 1; i < exps.size(); i++)args.push_back(exps.get(i).elem(0));
			if (env.find(pr) != env.end())  return env[pr].apply(args);
			else 
			{
				return(("Error! Unbound variable: " + pr));
			}
		}
	}
}

bool OCCDomainLang::compile(std::string& sourceCode)
{
	Environment env;
	env["+"] = add;
	env["-"] = diff;
	env["*"] = prod;
	env["/"] = div;
	env["exit"] = scheme_quit;
	env["quit"] = scheme_quit;
	env["="] = equal;
	env["!="] = not_equal;
	env["<"] = less_than;
	env["<="] = less_or_equal;
	env[">"] = great_than;
	env[">="] = great_or_equal;
	
	std::vector<string> processedCodes = split_codes(sourceCode);

	for (auto & singleCode : processedCodes)
	{
		std::cout << singleCode << std::endl;
		
		std::vector<string> vecCode = clean_input(singleCode);

		PList pl = PList(vecCode);

		std::cout << eval(pl, env) << std::endl;
	}
	

	return true;
}
