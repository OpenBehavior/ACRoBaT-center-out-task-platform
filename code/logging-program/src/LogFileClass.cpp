
#include "LogFileClass.h"

// Constructor
LogFileClass::LogFileClass() {}

//
bool LogFileClass::createLogFile(String^ deviceName, String^ version, String^ trialType, String^ numTargets, String^ dataFields, String^ subjectName) 
{
	DateTime dt = DateTime::Now;
	String^ date = dt.ToString("yyyyMMdd_HHmmss");

	String^ path = Directory::GetCurrentDirectory();
	String^ parent_path = Directory::GetParent(path)->FullName;
	String^ parent_parent_path = Directory::GetParent(parent_path)->FullName;
	String^ data_directory = parent_parent_path + "/data";

	String^ prefix = data_directory + "/MLRT_DATA_" + date;
	String^ suffix = ".txt";

	String^ fileName = prefix + "_" + deviceName + "_" + subjectName + suffix;
	StreamWriter^ logFileObject;

	if ( this->VERBOSE ) { Console::WriteLine("DEBUG: Log File Path Name: " + fileName); }

	try { logFileObject = gcnew StreamWriter(fileName); }
	catch (IOException^ ex) { return false; }

	logFileObject->WriteLine("Header: Moritz Lab Remote Trainer, " + version + ", " + trialType + ", " + numTargets + ", " + date);
	logFileObject->WriteLine("Header: " + dataFields);	
	
	this->logFileName = fileName;
	this->logFileObject = logFileObject;

	return true;
}

//
bool LogFileClass::writeToLogFile(String^ tx_message)
{
	this->logFileObject->WriteLine(tx_message);
	return true;
}

//
bool LogFileClass::closeLogFile()
{
	this->logFileObject->Close();
	return true;
}

//
String^ LogFileClass::getLogFileName()
{
	return this->logFileName;
}