// ARTClass.h

#include "ARTClass.h"

// *******************************************
// Public Methods
// *******************************************

// Constructor
ARTClass::ARTClass(String^ n, SerialPort^ sp) 
{ 
	this->portName = n;
	this->subjectName = "SUBJECTX"; 
	this->_serialPort = sp;
}

// Constructor
ARTClass::ARTClass(String^ n, SerialPort^ sp, String^ sn) 
{ 
	this->portName = n;
	this->subjectName = sn; 
	this->_serialPort = sp;
}

// Main Polling Loop to Gather Data
void ARTClass::mainProcessingLoop()
{
	// Set the read timeout [ms]
	this->_serialPort->ReadTimeout = 25;

	if ( this->VERBOSE ) { Console::WriteLine(" Starting mainProcessingThread for: " + this->portName); }
	
	this->PROCESSING_LOOP_RUNNING = true;

	while ( this->PROCESSING_LOOP_RUNNING )
	{
		String^ tx_message = "all,1";
		String^ rx_message = "";

		try
		{
			// Request Data from ART
			this->_serialPort->WriteLine(tx_message);

			// Get Response
			rx_message = this->_serialPort->ReadLine();
			rx_message = rx_message->TrimEnd('\r', '\n');
		}
		catch (TimeoutException^ ex) { rx_message = ""; }

		if ( rx_message != "" )
		{
			array<String^>^ dataArray = splitDataFields(rx_message);
		
			if ( dataArray->Length == this->numFields )
			{
				this->logFileObject->writeToLogFile(rx_message);
				this->curTrialNum = Convert::ToInt32(dataArray[0]);
				this->curRewardNum = Convert::ToInt32(dataArray[2]);

				this->curDwellms = Convert::ToInt32(dataArray[4]);
				this->curTimeoutms = Convert::ToInt32(dataArray[5]);
				this->curTarget = Convert::ToInt32(dataArray[6]);

				this->curCorrectTarget = Convert::ToInt32(dataArray[10]);
			}
		}
	}

	if ( this->VERBOSE ) { Console::WriteLine(" Ending mainProcessingThread for: " + this->portName); }

	return;
}

// Close Main Polling Loop to Gather Data
void ARTClass::endProcessingLoop()
{
	this->PROCESSING_LOOP_RUNNING = false;

	Thread::Sleep(100);

	if (this->_serialPort)
		this->_serialPort->Close();

	if (this->logFileObject)
		this->logFileObject->closeLogFile();
}

// 
bool ARTClass::createLogFileObj()
{
	this->logFileObject = gcnew LogFileClass();
	return logFileObject->createLogFile(this->portName, 
										this->version,
										this->trialType,
										"" + this->numTargets,
										this->dataFields_string, 
										this->subjectName);
}

//
void ARTClass::writeToSerialOut(String^ tx_message)
{
	this->_serialPort->WriteLine(tx_message);
}

//
String^ ARTClass::readNextSerialLine()
{
	String^ rx_message = "";
	try 
	{
		rx_message = this->_serialPort->ReadLine();
	}
	catch (TimeoutException^ ex) {}

	return rx_message;
}

// Return the device Name
void ARTClass::setSubjectName(String^ sn)
{
	this->subjectName = sn;
}

// Return the device Name
String^ ARTClass::getDeviceName()
{
	return this->portName;
}

// Return the log file path
String^ ARTClass::getLogFileName()
{
	return this->logFileObject->getLogFileName();
}

//
int ARTClass::getCurrentTrialNumber()
{
	return this->curTrialNum;
}

//
int ARTClass::getCurrentRewardNumber()
{
	return this->curRewardNum;
}

int ARTClass::getCurrentTarget()
{
	return this->curTarget;
}

int ARTClass::getCurrentCorrectTarget()
{
	return this->curCorrectTarget;
}

int ARTClass::getCurrentDwellTime()
{
	return this->curDwellms;
}

int ARTClass::getCurrentTimeoutTime()
{
	return this->curTimeoutms;
}

// *******************************************
// Private Methods
// *******************************************

// Convert "asdf, asdfa, asdff, asdfsa" -> ["asdf", "asdfa", "asdff", "asdfsa"]
array<String^>^ ARTClass::splitDataFields(String^ rx_message)
{
	// Split String 
	String^ delimStr = ",";
	array<Char>^ delimiter = delimStr->ToCharArray( );
	array<String^>^ dataArray = rx_message->Split( delimiter );
	
	// Remove Trailing/Leading White Spaces
	for ( int i = 0; i < dataArray->Length; i++ )
		dataArray[i] = dataArray[i]->Trim();

	return dataArray;
}
