// ARTClass.h

#ifndef ARTClass_H
#define ARTClass_H

#include <windows.h>
#include <string>
#include <iostream>
#include <sstream>

#include "LogFileClass.h"

using namespace std;
using namespace System;
using namespace System::IO;
using namespace System::IO::Ports;
using namespace System::Threading;

public ref class ARTClass
{
private:

	// *******************************************
	// Private Variables
	// *******************************************
	bool PROCESSING_LOOP_RUNNING;

	SerialPort^ _serialPort;
	String^ portName;
	String^ subjectName;

	int curTrialNum;
	int curRewardNum;

	int curDwellms;
	int curTimeoutms;

	int curCorrectTarget;
	int curTarget;

	LogFileClass^ logFileObject;

	// *******************************************
	// Private Functions
	// *******************************************
	static array<String^>^ splitDataFields(String^ rx_message);

public:

	// *******************************************
	// Public Variables
	// *******************************************
	bool VERBOSE; 
	
	String^ version;
	bool mirroring;
	String^ trialType;
	int numTargets;

	String^ dataFields_string;
	array<String^>^ dataFields;
	int numFields;


	// *******************************************
	// Public Functions
	// *******************************************

	// Constructor
	ARTClass(String^ n, SerialPort^ sp);
	ARTClass(String^ n, SerialPort^ sp, String^ sn);

	// Main Polling Loop to Gather Data
	void mainProcessingLoop();
	void endProcessingLoop(); 

	void setSubjectName(String^ sn); 
	String^ getDeviceName(); 
	String^ getLogFileName(); 
	int getCurrentTrialNumber();
	int getCurrentRewardNumber();
	int getCurrentTarget();
	int getCurrentCorrectTarget();
	int getCurrentDwellTime();
	int getCurrentTimeoutTime();

	bool createLogFileObj();
	String^ readNextSerialLine(); 
	void writeToSerialOut(String^ tx_message); 
	
};

#endif