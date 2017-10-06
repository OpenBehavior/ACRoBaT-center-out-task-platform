// PythonGUIInterfaceClass.h

#ifndef PythonGUIInterfaceClass_H
#define PythonGUIInterfaceClass_H

#include <windows.h>
#include <string>
#include <Python.h>
#include <msclr\marshal_cppstd.h>

using namespace std;
using namespace System;
using namespace System::Runtime::InteropServices;
using namespace msclr::interop;

public ref class PythonGUIInterfaceClass
{
private:
	PyObject *pModule, 
			 *pInitFunc,
			 *pConnectFunc, 
			 *pSendDataFunc, 
			 *pGetGUIVar, 
			 *pSetGUIVar, 
			 *pTest1, 
			 *pTest2;

	char* convertNetString2Char(String^ string);
	String^ convertChar2NetString(char* nativeString);

	String^ sendCommand2PyGUI(PyObject* pFunc, String^ arg1);
	String^ sendCommand2PyGUI(PyObject* pFunc, String^ arg1, String^ arg2);
	String^ sendCommand2PyGUI(PyObject* pFunc, String^ arg1, String^ arg2, String^ arg3);

public:
	bool VERBOSE;
	
	// Constructor
	PythonGUIInterfaceClass();

	bool mainLoop();
	bool initializePythonIntrpt();
	bool closePythonIntrpt();
		
	String^ add_device(String^ device_name, String^ field_names);
	String^ sendDataUpdate(String^ device_name, String^ data);
	String^ getGUIVar(String^ device_name, String^ var_name);
	String^ setGUIVar(String^ device_name, String^ var_name, String^ var_value);

	void test(String^ arg1, String^ arg2);	
};

#endif