// PythonGUIInterfaceClass.h

#include "PythonGUIInterfaceClass.h"

// *******************************************
// Example Code
// *******************************************

	//PythonGUIInterfaceClass^ PyGUI = gcnew PythonGUIInterfaceClass();
	
	//PyGUI->add_device("COM5", "Header: GBL_trial_number, time_tstamp_msec");

	//PyGUI->sendDataUpdate("COM5", "1, 1000");
	//PyGUI->sendDataUpdate("COM5", "1, 1500");
	//PyGUI->sendDataUpdate("COM5", "1, 1999");

	//PyGUI->sendDataUpdate("COM5", "2, 2000");
	//PyGUI->sendDataUpdate("COM5", "2, 2500");
	//PyGUI->sendDataUpdate("COM5", "2, 2999");

	//PyGUI->sendDataUpdate("COM5", "3, 3000");
	//PyGUI->sendDataUpdate("COM5", "3, 3500");
	//PyGUI->sendDataUpdate("COM5", "3, 3999");
	
	//PyGUI->sendDataUpdate("COM5", "4, 4000");
	
	//PyGUI->closePythonIntrpt();

// IN DEVELOPMENT:
	//Console::WriteLine("");
	//Console::WriteLine(" Creating GUI Thread... ");	

	//// Start GUI Object
	//PythonGUIInterfaceClass^ PyGUI = gcnew PythonGUIInterfaceClass();
	//
	//// Create/Start GUI Thread
	//Thread^ guiThread = gcnew Thread ( gcnew ThreadStart ( PyGUI, &PythonGUIInterfaceClass::initializePythonIntrpt ) );
	//guiThread->Start();

	//// Wait for Thread to Start
	//Thread::Sleep(1000);

	//// Connect Available Trainers to GUI 
	//for ( int i = 0; i < List_ART->Count; i++ )
	//{
	//	//PyGUI->add_device("COM5", "Header: GBL_trial_number, time_tstamp_msec");
	//	PyGUI->add_device(List_ART[i]->getDeviceName(), (List_ART[i]->dataFields_string);
	//}


// *******************************************
// Private Methods
// *******************************************

char* PythonGUIInterfaceClass::convertNetString2Char(String^ string)
{
	IntPtr ptrToNativeString = Marshal::StringToHGlobalAnsi(string);
	char* nativeString = static_cast<char*>(ptrToNativeString.ToPointer());
	return nativeString;
}

String^ PythonGUIInterfaceClass::convertChar2NetString(char* nativeString)
{
	String^ netString = marshal_as<String^>( nativeString );
	return netString;
}

String^ PythonGUIInterfaceClass::sendCommand2PyGUI(PyObject* pFunc, String^ arg1)
{
	PyObject *pArgs, *pRet;

	// Preparing data to pass as python parameter
	pArgs = Py_BuildValue("(s)", convertNetString2Char(arg1));

	// Call Function
	pRet = PyEval_CallObject(pFunc, pArgs);
 
	String^ return_val = "";

	if (pRet != NULL && PyString_Check(pRet))
	{
		return_val = convertChar2NetString( PyString_AsString(pRet) );
			
		if (this->VERBOSE)
			Console::WriteLine("Return From Python: \"" + return_val + "\"");
	}
			 
	// Printing error ( if any )
	PyErr_Print();
 
	Py_DECREF(pRet);
	Py_DECREF(pArgs);

	return return_val;
}

String^ PythonGUIInterfaceClass::sendCommand2PyGUI(PyObject* pFunc, String^ arg1, String^ arg2)
{
	PyObject *pArgs, *pRet;

	// Preparing data to pass as python parameter
	pArgs = Py_BuildValue("(s,s)", convertNetString2Char(arg1), convertNetString2Char(arg2));

	// Call Function
	pRet = PyEval_CallObject(pFunc, pArgs);
 
	String^ return_val = "";
		 
	if (pRet != NULL)
	{
		if (PyString_Check(pRet))
			return_val = convertChar2NetString( PyString_AsString(pRet) );
		else if (PyInt_Check(pRet))
			return_val = "" + PyInt_AsLong(pRet) + "";
	}

	if (this->VERBOSE)
			Console::WriteLine("C++ -> Return From Python: \"" + return_val + "\"");
			 
	// Printing error ( if any )
	PyErr_Print();
 
	Py_DECREF(pRet);
	Py_DECREF(pArgs);

	return return_val;
}

String^ PythonGUIInterfaceClass::sendCommand2PyGUI(PyObject* pFunc, String^ arg1, String^ arg2, String^ arg3)
{
	PyObject *pArgs, *pRet;

	// Preparing data to pass as python parameter
	pArgs = Py_BuildValue("(s,s,s)", convertNetString2Char(arg1), 
										convertNetString2Char(arg2), 
										convertNetString2Char(arg2));
	// Call Function
	pRet = PyEval_CallObject(pFunc, pArgs);
 
	String^ return_val = "";

	if (pRet != NULL && PyString_Check(pRet))
	{
		return_val = convertChar2NetString( PyString_AsString(pRet) );
			
		if (this->VERBOSE)
			Console::WriteLine("Return From Python: \"" + return_val + "\"");
	}
			 
	// Printing error ( if any )
	PyErr_Print();
 
	Py_DECREF(pRet);
	Py_DECREF(pArgs);

	return return_val;
}

	
// *******************************************
// Public Methods
// *******************************************

// Constructor
PythonGUIInterfaceClass::PythonGUIInterfaceClass()
{
	this->VERBOSE = false;
	this->initializePythonIntrpt();
}

bool PythonGUIInterfaceClass::mainLoop()
{
	if ( this->VERBOSE ) { Console::WriteLine(" Starting Python GUI Thread... "); }

	// TODO: Something here

	if ( this->VERBOSE ) { Console::WriteLine(" CLosing Python GUI Thread... "); }

	return true;
}

bool PythonGUIInterfaceClass::initializePythonIntrpt()
{
	// Initialize the Python Interpreter
	Py_Initialize();
 
	const char *scriptDirectoryName = "..\\..\\python_code\\src_cplusplus";

	// Get the current path (this is a list)
	PyObject *sysPath = PySys_GetObject("path");
	// Create a value to add to the list 
	PyObject *newPath = PyString_FromString(scriptDirectoryName);
	// And add it 
	int result = PyList_Insert(sysPath, 0, newPath);
	// We don't need that string value anymore, so deref it 
	Py_DECREF(newPath);

	// importing python module
	this->pModule = PyImport_ImportModule("MLRT_GUI_CPP_Interface");
 
	if (this->pModule != NULL)
	{
		// getting function of that module
		this->pInitFunc = PyObject_GetAttrString( this->pModule, "main" );

		// getting function of that module
		this->pConnectFunc = PyObject_GetAttrString( this->pModule, "connect_device" );

		// getting function of that module
		this->pSendDataFunc = PyObject_GetAttrString( this->pModule, "send_data_string" );

		// getting function of that module
		this->pGetGUIVar = PyObject_GetAttrString( this->pModule, "get_GUI_variable" );

		// getting function of that module
		this->pSetGUIVar = PyObject_GetAttrString( this->pModule, "set_GUI_variable" );

		// Initalize GUI
		this->sendCommand2PyGUI(this->pInitFunc, "");

		return true;
	}
	else
	{
		PyErr_Print();
		return false;	
	}

	//// importing python test module
	//this->pModule = PyImport_ImportModule("MLRT_DummyClass");
	//// getting function of that test module
	//this->pTest1 = PyObject_GetAttrString( this->pModule, "main" );
	//// getting function of that test module
	//this->pTest2 = PyObject_GetAttrString( this->pModule, "modifyGUIObj" );	
}

void PythonGUIInterfaceClass::test(String^ arg1, String^ arg2)
{
	this->sendCommand2PyGUI(pTest1, arg1);
	this->sendCommand2PyGUI(pTest2, arg1, arg2);
}

String^ PythonGUIInterfaceClass::add_device(String^ device_name, String^ field_names)
{
	return this->sendCommand2PyGUI(pConnectFunc, device_name, field_names);
}

String^ PythonGUIInterfaceClass::sendDataUpdate(String^ device_name, String^ data)
{
	return this-> sendCommand2PyGUI(pSendDataFunc, device_name, data);
}

String^ PythonGUIInterfaceClass::getGUIVar(String^ device_name, String^ var_name)
{
	return this-> sendCommand2PyGUI(pGetGUIVar, var_name, "");
}

String^ PythonGUIInterfaceClass::setGUIVar(String^ device_name, String^ var_name, String^ var_value)
{
	return this-> sendCommand2PyGUI(pSetGUIVar, var_name, var_value);
} 

bool PythonGUIInterfaceClass::closePythonIntrpt()
{	
	Py_DECREF(pModule);
	Py_DECREF(pInitFunc);
	Py_DECREF(pConnectFunc);
	Py_DECREF(pSendDataFunc);
	Py_DECREF(pGetGUIVar);
	Py_DECREF(pSetGUIVar);

	//Py_DECREF(pTest1);
	//Py_DECREF(pTest2);		
		
	// Finish the Python Interpreter
	Py_Finalize();

	return true;
}


// *******************************************
// Example Code
// *******************************************
//PyObject *pName, *pModule, *pArgs, *pFunc,*pRet;
//
//char s[100]={"Hello World !!!! Nice to meet you."};
// 
//// Initialize the Python Interpreter
//Py_Initialize();
// 
//// importing python module
//pModule = PyImport_ImportModule("MLRT_DummyClass");
// 
////preparing data to pass as python parameter
//pArgs = Py_BuildValue("(s)", s);
// 
//// Calling function of that module
//pFunc = PyObject_GetAttrString( pModule, "main" );
//pRet = PyEval_CallObject(pFunc, pArgs);
// 
//if ( PyString_Check(pRet) ){
//    cout << "Return From python :: " << PyString_AsString(pRet) << endl;
//}
//
//pFunc = PyObject_GetAttrString( pModule, "modifyGUIObj" );
//pRet = PyEval_CallObject(pFunc, NULL);
// 
//// Printing error ( if any )
//PyErr_Print();
// 
//Py_DECREF(pFunc);
//Py_DECREF(pModule);
//Py_DECREF(pRet);
//Py_DECREF(pArgs);
// 
//// Finish the Python Interpreter
//Py_Finalize();

