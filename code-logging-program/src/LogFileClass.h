// LogFileClass.h

#ifndef LogFileClass_H
#define LogFileClass_H

#include <windows.h>
#include <iostream>
#include <string>
#include <sstream>

using namespace std;
using namespace System;
using namespace System::IO;

public ref class LogFileClass
{
private:
	// *******************************************
	// Prviate Functions
	// *******************************************
	String^ logFileName;
	StreamWriter^ logFileObject;

public:
	// *******************************************
	// Public Variables
	// *******************************************
	bool VERBOSE;

	// *******************************************
	// Public Functions
	// *******************************************
	LogFileClass();
	
	bool createLogFile(String^ deviceName, String^ version, String^ trialType, String^ numTargets, String^ dataFields, String^ subjectName);
	bool writeToLogFile(String^ tx_message);
	bool closeLogFile();
	String^ getLogFileName();
};

#endif