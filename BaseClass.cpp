
/// Here the FreeCAD includes sorted by Base,App,Gui......
#include <assert.h>
#include "BaseClass.h"

using namespace Base;

Type BaseClass::classTypeId = Type::badType();


//**************************************************************************
// Construction/Destruction

/**
* A constructor.
* A more elaborate description of the constructor.
*/
BaseClass::BaseClass()
{

}

/**
* A destructor.
* A more elaborate description of the destructor.
*/
BaseClass::~BaseClass()
{
}


//**************************************************************************
// separator for other implementation aspects

void BaseClass::init(void)
{
	assert(BaseClass::classTypeId == Type::badType() && "don't init() twice!");
	/* Make sure superclass gets initialized before subclass. */
	/*assert(strcmp(#_parentclass_), "inherited"));*/
	/*Type parentType(Type::fromName(#_parentclass_));*/
	/*assert(parentType != Type::badType() && "you forgot init() on parentclass!");*/

	/* Set up entry in the type system. */
	BaseClass::classTypeId =
		Type::createType(Type::badType(),
			"Base::BaseClass",
			BaseClass::create);
}

Type BaseClass::getClassTypeId(void)
{
	return BaseClass::classTypeId;
}

Type BaseClass::getTypeId(void) const
{
	return BaseClass::classTypeId;
}


void BaseClass::initSubclass(Type &toInit, const char* ClassName, const char *ParentName,
	Type::instantiationMethod method)
{
	// don't init twice!
	assert(toInit == Type::badType());
	// get the parent class
	Type parentType(Type::fromName(ParentName));
	// forgot init parent!
	assert(parentType != Type::badType());

	// create the new type
	toInit = Type::createType(parentType, ClassName, method);
}

/**
* This method returns the Python wrapper for a C++ object. It's in the responsibility of
* the programmer to do the correct reference counting. Basically there are two ways how
* to implement that: Either always return a new Python object then reference counting is
* not a matter or return always the same Python object then the reference counter must be
* incremented by one. However, it's absolutely forbidden to return always the same Python
* object without incrementing the reference counter.
*
* The default implementation returns 'None'.
*/
// PyObject *BaseClass::getPyObject(void)
// {
// 	assert(0);
// 	Py_Return;
// }
// 
// void BaseClass::setPyObject(PyObject *)
// {
// 	assert(0);
// }
