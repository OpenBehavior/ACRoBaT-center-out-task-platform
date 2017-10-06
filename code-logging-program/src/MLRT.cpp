
//---------------------------------------------------------------------------
// INCLUDES
//---------------------------------------------------------------------------
#include "stdafx.h"
#include <windows.h>
#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <algorithm>

//#include "PythonGUIInterfaceClass.h"
#include "ARTClass.h"

#using <System.dll>

//---------------------------------------------------------------------------
// DECLARED NAMESPACES
//---------------------------------------------------------------------------
using namespace std;
using namespace System;
using namespace System::IO;
using namespace System::IO::Ports;
using namespace System::ComponentModel;
using namespace System::Threading;
using namespace System::Collections::Generic;

//---------------------------------------------------------------------------
// DECLARED CONSTANTS
//---------------------------------------------------------------------------
const int MAX_NUM_ARTRAINERS = 3;

//---------------------------------------------------------------------------
// DECLARED VARIBLES
//---------------------------------------------------------------------------
bool VERBOSE = true;
bool DEBUG = false;

bool MLRT_RUNNING = false;


//---------------------------------------------------------------------------
// CLASS DECLARATIONS
//---------------------------------------------------------------------------

public ref class CommandLineClass
{
public:
	void updateCommandLineOutput()
	{
		Console::WriteLine("");
		Console::WriteLine(" ----------------------------");

		while ( MLRT_RUNNING )
		{
			Console::Write("\r");
			Console::Write(" [ ");
	
			for ( int i = 0; i < List_ART->Count; i++ )
			{
				int trial_num = List_ART[i]->getCurrentTrialNumber();
				int rewrd_num = List_ART[i]->getCurrentRewardNumber();
				double prcnt = double(rewrd_num) / double(trial_num-1.0);
				
				Console::Write(List_ART[i]->getDeviceName() + ": " + trial_num + ", {0:P} ", prcnt);
				Console::Write("] ");

				Console::Write(" [ ");
				cout << List_ART[i]->getCurrentDwellTime();
				Console::Write(" / ");
				cout << List_ART[i]->getCurrentTimeoutTime();
				Console::Write(" ] ");


				String ^ crct       = "- X -";
				String ^ slct       = "-[o]-";
				String ^ slct_crct  = "-[X]-";
				String ^ home       = "-(o)-";
				String ^ home_crct  = "-(X)-";
				String ^ empty      = "- o -";				
				List<String ^>^ trgt = gcnew List<String ^>(5);					
				for (int j = 0; j < 5; j++)
				{
					trgt->Add(empty);
					if (List_ART[i]->getCurrentTarget() == j && List_ART[i]->getCurrentCorrectTarget() == j)
						trgt[j] = slct_crct;
					else if (List_ART[i]->getCurrentTarget() == j)
						trgt[j] = slct;
					else if (List_ART[i]->getCurrentCorrectTarget() == j)
						trgt[j] = crct;
				}

				if (List_ART[i]->getCurrentTarget() == -1)
					if (List_ART[i]->getCurrentTarget() == 2)
						trgt[2] = home_crct;
					else
						trgt[2] = home;
				
				Console::Write(String::Format( " [ {4} {1} {2} {0} {3} ]  ", trgt[0], trgt[1], trgt[2], trgt[3], trgt[4]));
			}
			//Console::Write("]   ");
			
			Thread::Sleep(100);
		}
		return;
	}

	CommandLineClass(List<ARTClass^>^ &obj) : List_ART(obj) { }

private:
	List<ARTClass^>^& List_ART;
};

//---------------------------------------------------------------------------
// FUNCTION DECLARATIONS
//---------------------------------------------------------------------------

bool bootMessage(void);

array<String^>^ getConnectedSerialPorts(void);

List<String^>^ findARTrainers(array<String^>^ serialPortNames);

bool connectToSerialPort(String^ portName, bool reboot32u4, bool saveSerialPortObj, List<ARTClass^>^ %List_ART);

bool selectARTrainer(List<String^>^ %ARTrainerNames);

String^ getSubjectName(void);

List<ARTClass^>^ connectARTrainers(List<String^>^ ARTrainerNames);

List<String^>^ getARTrainerParams(ARTClass^ %ART_Obj, List<String^>^ tx_messages);

bool setTrialType(ARTClass^ %ART_Obj);

bool setTargetNum(ARTClass^ %ART_Obj);

bool setStimParams(ARTClass^ %ART_Obj);

bool initART_ObjParams(ARTClass^ %ART_Obj, List<String^>^ rx_messages);

bool createLogFile(ARTClass^ %ART_Obj);

array<String^>^ splitDataFields(String^ rx_message); 

bool exit(List<ARTClass^>^ %List_ART);


//---------------------------------------------------------------------------
// MAIN FUNCTION
//---------------------------------------------------------------------------

int main(array<System::String ^> ^args)
{
	// Boot up Message
	bootMessage();

	// Find Available Serial Ports
	array<String^>^ connectedSerialPorts = getConnectedSerialPorts();

	// Find Connected ARTrainers
	List<String^>^ ARTrainerNames = findARTrainers(connectedSerialPorts); 

	// Select ARTrainer from List
	selectARTrainer(ARTrainerNames);

	// Connect to ARTrainer(s)
	List<ARTClass^>^ List_ART = connectARTrainers(ARTrainerNames);
	
	// List of Messages to Send
	List<String^>^ tx_messages = gcnew List<String^>();
	tx_messages->Add("version,1");		// Get Arduino Code Version
	tx_messages->Add("data_list,0");	// Get Data Fields 
	tx_messages->Add("data_list,1");	// Get Data Fields
	tx_messages->Add("data_list,2");	// Get Data Fields
	tx_messages->Add("mirror,0");		// Set Serial Comm Mirroring Off

	// Get/Set Training Parameters
	for ( int i = 0; i < List_ART->Count; i++ )
	{	
		List_ART[i]->setSubjectName(getSubjectName());

		if ( VERBOSE | DEBUG )
		{
			Console::WriteLine("");
			Console::WriteLine(" Get/Set Training Parameters From: " + List_ART[i]->getDeviceName());
		}
		
		List<String^>^ rx_messages = getARTrainerParams(List_ART[i], tx_messages);
		
		if (setTrialType(List_ART[i]) == false)
			return 0;

		if (setTargetNum(List_ART[i]) == false)
			return 0;

		if (setStimParams(List_ART[i]) == false)
			return 0;

		initART_ObjParams(List_ART[i], rx_messages);

		if ( !List_ART[i]->createLogFileObj() )
		{
			// End of Program
			Console::WriteLine(" Error!! Could Not Create Log File. ");
			Console::WriteLine(" Press Enter to Exit. ");
			Console::ReadLine();	
			return 0;
		}
	}

	// Start Training
	Console::WriteLine("");
	Console::WriteLine(" Starting Training... ");
	Console::WriteLine("");
	MLRT_RUNNING = true;	

	// Create Threads
	List<Thread^>^ List_Threads = gcnew List<Thread^>();
	for ( int i = 0; i < List_ART->Count; i++ )
	{
		Thread^ newThread = gcnew Thread ( gcnew ThreadStart ( List_ART[i], &ARTClass::mainProcessingLoop ) );
		List_Threads->Add( newThread );
	}

	// Start Threads
	for ( int i = 0; i < List_Threads->Count; i++ )
		List_Threads[i]->Start();
	
	// Wait for Threads to Start
	Thread::Sleep(1000);

	// Stop Training Message	
	Console::WriteLine("");
	Console::WriteLine(" Type ""s"" To Stop Training: ");

	// System Output
	CommandLineClass^ cmdLineObj = gcnew CommandLineClass(List_ART);
	Thread^ cmdLineThread = gcnew Thread ( gcnew ThreadStart ( cmdLineObj, &CommandLineClass::updateCommandLineOutput ) );
	cmdLineThread->Start();
	
	// Wait to Stop Training
	while(Console::ReadLine() != "s") { Thread::Sleep(1000); }
	Console::WriteLine("");
	exit(List_ART);
	
	// End of Program

	Console::WriteLine("");
	Console::WriteLine(" Press Enter to Exit. ");
	Console::ReadLine();	
	return 0;
}


//---------------------------------------------------------------------------
// HELPER FUNCTIONS
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
bool bootMessage(void)
{
	Console::WriteLine(" ");
	Console::WriteLine(" *****************************");
	Console::WriteLine(" Moritz Lab Remote Trainer 1");
	Console::WriteLine(" Created by David Bjanes");
	Console::WriteLine(" Email: dbjanes@uw.edu");
	Console::WriteLine(" Copyright 2015");
	Console::WriteLine(" *****************************");
	Console::WriteLine(" ");
	Console::WriteLine(" Booting...");
	Console::WriteLine(" ");

	return true;
}

//---------------------------------------------------------------------------
array<String^>^ getConnectedSerialPorts(void)
{
	// Get a list of serial port names.
    array<String^>^ serialPorts = nullptr;
    try 
	{
		serialPorts = SerialPort::GetPortNames();
	}
	catch (Win32Exception^ ex)
	{
		Console::WriteLine(ex->Message);
	}

	// Print out to Command Line if Verbose
    if ( VERBOSE | DEBUG )
	{
		Console::WriteLine(" Serial Ports Currently Connected: ");

		// Display each port name to the console. 
		for each(String^ port in serialPorts)
			Console::WriteLine("  -" + port);
	}

	return serialPorts;
}

//---------------------------------------------------------------------------
List<String^>^ findARTrainers(array<String^>^ serialPortNames)
{
	List<String^>^ ARTrainerNames = gcnew List<String^>();

	//if ( VERBOSE | DEBUG )
	//{
	//	Console::WriteLine("");
	//	Console::WriteLine(" ARTrainers: ");
	//}

	for ( int i = 0; i < serialPortNames->Length; i++ )
	{	
		String^ portName = serialPortNames[i];
					
		//List<ARTClass^>^ temp = gcnew List<ARTClass^>();

		// Check if serial port is a valid ARTrainer
		//if  ( connectToSerialPort(portName, true, false, temp) )
			ARTrainerNames->Add(portName);
	}

	return ARTrainerNames;
}

//---------------------------------------------------------------------------
bool selectARTrainer(List<String^>^ %ARTrainerNames)
{
	if (ARTrainerNames->Count > 1)
	{
		if ( VERBOSE | DEBUG ) { Console::WriteLine(""); }

		String^ user_input = ""; 
		bool validComPortSelected = false;
		while (!validComPortSelected)
		{
			Console::WriteLine(" Enter COM Port of ARTrainer to Select [Ex. 'COM3']:");

			user_input = Console::ReadLine();
			for ( int i = 0; i < ARTrainerNames->Count; i++ )
			{
				validComPortSelected = ARTrainerNames[i] == user_input;
				if (validComPortSelected)
					break;			
			}
		}	

		// Remove all the other connected Trainers
		int i = 0;
		while( ARTrainerNames->Count > 1 )
		{
			if (ARTrainerNames[i] != user_input)
			{
				ARTrainerNames->Remove(ARTrainerNames[i]);
				i = 0;
			}
			else
			{
				i++;
			}
		}
	}

	return true;
}

//---------------------------------------------------------------------------
String^ getSubjectName()
{
	if ( VERBOSE | DEBUG ) { Console::WriteLine(""); }

	String^ user_input = ""; 
	
	Console::WriteLine(" Enter Subject ID [Ex. 171]:");

	user_input = Console::ReadLine();
		
	return "SUBJECT" + user_input;
}

//---------------------------------------------------------------------------
bool connectToSerialPort(String^ portName, bool reboot32u4, bool saveSerialPortObj, List<ARTClass^>^ %List_ART)
{
	try
	{
		// If flag to reboot ATmega32u4 
		if ( reboot32u4 )
		{
			// Create SerialPort Object
			SerialPort^ _serialPort_reboot;
			_serialPort_reboot = gcnew SerialPort(portName, 1200);

			// **** Debug Statements
			if ( VERBOSE ) { Console::Write(    "\r  -" + portName + "  [ Opened at 1200 baud ]   "); }
			if ( DEBUG )   { Console::Write("DEBUG:  -" + portName + "  [ Opened at 1200 baud ] \n"); }

			// Open Port
			_serialPort_reboot->Open();
			Thread::Sleep(500);		
			
			// **** Debug Statements
			if ( VERBOSE ) { Console::Write(    "\r  -" + portName + "  [ Closed port to reboot ATmega32u4 ]      "); }
			if ( DEBUG )   { Console::Write("DEBUG:  -" + portName + "  [ Closed port to reboot ATmega32u4 ]    \n"); }

			// Close Port (if available)		
			try { _serialPort_reboot->Close(); }
			catch (Exception^ ex) { }

			// Wait for ATmega32u4 to wake up again
			for (int i = 0; i < 0; i++)
			{
				// **** Debug Statements
				if ( VERBOSE ) { for (int j = 0; j < 7; j++) { Console::Write("\b \b"); } Console::Write(". ]     "); }
				if ( DEBUG )   { Console::Write("DEBUG:  -" + portName + "  [ Waiting... ] \n"); }
				Thread::Sleep(1000);
			}
		}

		// Create SerialPort Object
		SerialPort^ _serialPort;

		// **** Debug Statements
		if ( VERBOSE ) { Console::Write(    "\r  -" + portName + "                                                 "); }
		if ( VERBOSE ) { Console::Write(    "\r  -" + portName + "  [ Opened at 115200 baud ]      "); }
		if ( DEBUG )   { Console::Write("DEBUG:  -" + portName + "  [ Opened at 115200 baud ]    \n"); }

		// Attempt 10 Connections before aborting
		for (int i = 0; i < 10; i++)
		{
			try
			{
				// Initialize Port Object
				_serialPort = gcnew SerialPort(portName, 115200);

				// Open Port
				_serialPort->Open();

				break;
			}
			catch (Exception^ ex) { }

			// **** Debug Statements
			if ( VERBOSE ) { for (int j = 0; j < 7; j++) { Console::Write("\b \b"); } Console::Write(". ]     "); }
			if ( DEBUG )   { Console::Write("DEBUG:  -" + portName + "  [ Waiting... ] \n"); }
			Thread::Sleep(1000);
		}	

		// If connection did not succeed
		if ( _serialPort == nullptr )
			return false;

		// **** Debug Statements
		if ( VERBOSE ) { Console::Write(    "\r  -" + portName + "  [ Toggle DTR Lines to Enable Comms ]        "); }
		if ( DEBUG )   { Console::Write("DEBUG:  -" + portName + "  [ Toggle DTR Lines to Enable Comms ]      \n"); }

		// Enable Commuications
		_serialPort->DtrEnable = false;
		Thread::Sleep(100);
		_serialPort->DtrEnable = true;
		Thread::Sleep(2000);
						
		// Set the read/write timeouts
		_serialPort->ReadTimeout  = 500;
		_serialPort->WriteTimeout = 500;

		// **** Debug Statements
		if ( VERBOSE ) { Console::Write(    "\r  -" + portName + "  [ Clear Serial Buffer ]                "); }
		if ( DEBUG )   { Console::Write("DEBUG:  -" + portName + "  [ Clear Serial Buffer ]              \n"); }

		// Clear Serial Buffer
		while (1)
		{
			try { String^ rx_message = _serialPort->ReadLine(); }
			catch(TimeoutException^ ex) { break; }
		}

		// Initialize Serial Commands
		String^ tx_message = "identity,1";
		String^ rx_message1 = "";
		String^ rx_message2 = "";
		String^ rx_expected_respd = "Arduino Remote Trainer";

		// **** Debug Statements
		if ( VERBOSE ) { Console::Write(    "\r  -" + portName + "  [ Ping: \"" + tx_message + "\" ]      "); }
		if ( DEBUG )   { Console::Write("DEBUG:  -" + portName + "  [ Ping: \"" + tx_message + "\" ]    \n"); }
		
		// Ping Serial Line
		try
		{			
			_serialPort->WriteLine(tx_message);
			Thread::Sleep(100);			
			rx_message1 = _serialPort->ReadLine();
			rx_message1 = rx_message1->Replace("\r", "");

			rx_message2 = _serialPort->ReadLine();
			rx_message2 = rx_message2->Replace("\r", "");
		}
		catch(Exception^ ex) { }		

		// Get Correct Message
		String^ rx_message = (rx_message1->Contains(tx_message)) ? rx_message2 : rx_message1;
		String^ isValidART = (rx_message->Contains(rx_expected_respd)) ? "ART" : "No Connection";

		// **** Debug Statements
		if ( VERBOSE ) { Console::WriteLine(    "\r  -" + portName + "  [ " + isValidART + ", Response: \"" + rx_message + "\" ]  "); }
		if ( DEBUG )   { Console::WriteLine("DEBUG:  -" + portName + "  [ " + isValidART + ", Response: \"" + rx_message + "\" ]  "); }

		// If ARTrainers
		if ( rx_message->Contains(rx_expected_respd) )
		{
			// If we want to add an ARTrainer to List
			if ( saveSerialPortObj )
			{
				ARTClass^ newART = gcnew ARTClass(portName, _serialPort);
				List_ART->Add(newART);
			}
			return true;
		}		
		else
		{
			// Close Serial Port
			_serialPort->Close();

			return false;
		}		
	}
	catch(Exception^ ex)
	{
		if ( VERBOSE ) { Console::WriteLine(    "\r  -" + portName + "  [ ERROR! Connection Refused ] "); }
		if ( DEBUG )   { Console::WriteLine("DEBUG:  -" + portName + "  [ ERROR! Connection Refused ] "); }

		return false;
	}
}

//---------------------------------------------------------------------------
List<ARTClass^>^ connectARTrainers(List<String^>^ ARTrainerNames)
{
	List<ARTClass^>^ List_ART = gcnew List<ARTClass^>();

	if ( VERBOSE | DEBUG )
	{
		Console::WriteLine("");
		Console::WriteLine(" Connecting to ARTrainer(s): ");
	}

	for ( int i = 0; i < ARTrainerNames->Count; i++ )
	{	
		String^ portName = ARTrainerNames[i];

		// Check if serial port is a valid ARTrainer
		connectToSerialPort(portName, true, true, List_ART);
	}

	return List_ART;
}

//---------------------------------------------------------------------------
List<String^>^ getARTrainerParams(ARTClass^ %ART_Obj, List<String^>^ tx_messages)
{
	if ( DEBUG )
	{
		Console::WriteLine("");
		Console::WriteLine("DEBUG: Getting Training Parameters From: " + ART_Obj->getDeviceName());
	}

	// List of Received Messages
	List<String^>^ rx_messages = gcnew List<String^>(); 

	// Send Messages
	for each( String^ tx_message in tx_messages )
	{
		String^ rx_message1 = "";
		String^ rx_message2 = "";
		try 
		{
			ART_Obj->writeToSerialOut(tx_message);
			Thread::Sleep(100);
			rx_message1 = ART_Obj->readNextSerialLine();
			rx_message2 = ART_Obj->readNextSerialLine();
			rx_message2 = rx_message2->TrimEnd('\r', '\n');
		}
		catch (TimeoutException^ ex) {}

		// Check Comms
		if  ( rx_message1->Contains(tx_message) )
			  rx_messages->Add(rx_message2);

		if ( DEBUG )
		{
			Console::WriteLine("DEBUG: Pinging " + ART_Obj->getDeviceName() + ": \'" + tx_message + "\'");
			Console::WriteLine("DEBUG: Received: " + "\'" + rx_message2 + "\'");
		}
	}

	return rx_messages;
}

//---------------------------------------------------------------------------
bool setTrialType(ARTClass^ %ART_Obj)
{
	List<String^>^ trialnames_list = gcnew List<String^>();

	if ( VERBOSE | DEBUG )
	{
		Console::WriteLine("");
		Console::WriteLine(" Retrieve All Trial Types from Trainer: " + ART_Obj->getDeviceName() );
	}

	String^ tx_message = "trial_list,1";
	String^ rx_header_message = "";
	int rx_header_numLines = 0;
	try 
	{
		// Ask Trainer for Trial Type List
		ART_Obj->writeToSerialOut(tx_message);
		Thread::Sleep(100);
		rx_header_message = ART_Obj->readNextSerialLine();
		rx_header_message = rx_header_message->TrimEnd('\r', '\n');

		// Parse Header Message		
		array<String^>^ header = splitDataFields(rx_header_message);
		rx_header_numLines = Convert::ToInt32(header[1], 10);

		// Record Each Trial Type and Print it for User
		String^ rx_message = "";
		for ( int i = 0; i < rx_header_numLines; i++ )
		{
			rx_message = ART_Obj->readNextSerialLine();
			rx_message = rx_message->TrimEnd('\r', '\n');
			trialnames_list->Add(rx_message);
			Thread::Sleep(100);
			
			Console::WriteLine(" - [" + (i+1) + "] " + rx_message);
		}

		// Get VALID User Input
		int user_input_int = 0;
		while ( user_input_int < 1 || user_input_int > rx_header_numLines )
		{
			Console::Write(" Please Enter VALID Trial Type Number: ");
			String^ user_input_str = Console::ReadLine();
			user_input_int = Convert::ToInt32(user_input_str, 10);
		}

		// Set Trial Type
		if ( DEBUG) { Console::WriteLine(" Setting Trial Type... "); }

		String^ tx_message_set = "set_trial_type," + user_input_int;
		ART_Obj->writeToSerialOut(tx_message_set);
		Thread::Sleep(100);
		
		// Confirm Trial Type Set
		String^ tx_message_get = "get_trial_type,1";
		ART_Obj->writeToSerialOut(tx_message_get);
		Thread::Sleep(100);
		rx_message = ART_Obj->readNextSerialLine();
		rx_message = rx_message->TrimEnd('\r', '\n');
		int rx_message_int = Convert::ToInt32(rx_message, 10);

		// Trial Type Handshake Confirmed
		if ( rx_message_int == user_input_int )
		{
			Console::WriteLine(" Trial Type Set: True");
			Console::WriteLine("");
		}
		else
		{
			Console::WriteLine("");
			Console::WriteLine("ERROR!!!! TrialType NOT SET!!");
			Console::WriteLine("Press Enter To Exit... ");
			Console::ReadLine();
			return 0;
		}		

		// Set Trial Type
		ART_Obj->trialType = trialnames_list[rx_message_int-1];
	}
	catch (Exception^ ex) 
	{
		return 0;
	}

	return 1;
}

//---------------------------------------------------------------------------
bool setTargetNum(ARTClass^ %ART_Obj)
{
	if ( VERBOSE | DEBUG )
	{
		Console::WriteLine("");
		Console::WriteLine(" Set/Get Target Number: " + ART_Obj->getDeviceName() );
	}

	try 
	{
		int user_input_int = 0;
		
		while (user_input_int < 2 || user_input_int > 5)
		{
			Console::Write(" Please Enter VALID Number of Targets [2-5]: ");
			String^ user_input_str = Console::ReadLine();
			user_input_int = Convert::ToInt32(user_input_str, 10);
		}

		String^ tx_message = "set_num_targets," + user_input_int;
		String^ rx_message = "";

		// Set Number of Targets
		ART_Obj->writeToSerialOut(tx_message);
		Thread::Sleep(100);

		// Confirm 
		tx_message = "get_num_targets,1";
		ART_Obj->writeToSerialOut(tx_message);
		Thread::Sleep(100);
		rx_message = ART_Obj->readNextSerialLine();
		rx_message = rx_message->TrimEnd('\r', '\n');

		int curr_target_num = Convert::ToInt32(rx_message, 10);

		// Trial Type Handshake Confirmed
		if ( curr_target_num == user_input_int )
		{
			Console::WriteLine(" Number of Targets: " + curr_target_num);
			Console::WriteLine("");
		}
		else
		{
			Console::WriteLine("");
			Console::WriteLine("ERROR!!!! Number of Targets NOT SET!!");
			Console::WriteLine("Press Enter To Exit... ");
			Console::ReadLine();
			return 0;
		}		

		// Set Trial Type
		ART_Obj->numTargets = curr_target_num;
	}
	catch (Exception^ ex) 
	{
		Console::WriteLine("");
		Console::WriteLine("ERROR!!!! Number of Targets NOT SET!!");
		Console::WriteLine("Press Enter To Exit... ");
		Console::ReadLine();
		return 0;
	}

	return 1;
}

//---------------------------------------------------------------------------
bool setStimParams(ARTClass^ %ART_Obj)
{
	if ( VERBOSE | DEBUG )
	{
		Console::WriteLine("");
		Console::WriteLine(" Set Stimulation Parameters: " + ART_Obj->getDeviceName() );
	}

	try 
	{
		// Set Correct Amplitudes list
		String^ user_input_str = "";
		array<String^>^ dataFields;

		int list_size = 11;
		while (list_size > 10)
		{
			Console::WriteLine(" Please Enter CSV List of CORRECT Amplitudes [i.e. ""10, 20, 30, 40""], ""skip"" to skip: ");
			user_input_str = Console::ReadLine();
			dataFields = splitDataFields(user_input_str);
			list_size = dataFields.Length;
		}

		if (user_input_str == "skip")
		{
			Console::WriteLine(" SKIPPING Stimulation Parameters ... " );
			Console::WriteLine(" " );
			return true;
		}

		String^ tx_message = "set_stim_amp_cr," + user_input_str;
		ART_Obj->writeToSerialOut(tx_message);
		Thread::Sleep(100);


		List<String^>^ cmdLineRequests_list = gcnew List<String^>();
		cmdLineRequests_list->Add(" Please Enter CSV List of INcorrect Amplitudes ""10, 20, 30, 40"": ");
		cmdLineRequests_list->Add(" Please Enter CSV List of CORRECT Frequencies ""100, 200, 300, 400"": ");
		cmdLineRequests_list->Add(" Please Enter CSV List of INcorrect Frequencies ""100, 200, 300, 400"": ");
		cmdLineRequests_list->Add(" Please Enter CSV List of CORRECT Pulse Width(s) ""100, 200, 300, 400"": ");
		cmdLineRequests_list->Add(" Please Enter CSV List of INcorrect Pulse Width(s) ""100, 200, 300, 400"": ");
		cmdLineRequests_list->Add(" Please Enter CSV List of CORRECT Number of Pulse(s) ""5, 7, 10, 12, 15"": ");
		cmdLineRequests_list->Add(" Please Enter CSV List of INcorrect Number of Pulse(s) ""5, 7, 10, 12, 15"": ");
		cmdLineRequests_list->Add(" Please Enter CSV List of CORRECT Inter-Train Interval ""50, 60, 70, 80, 90, 100"": ");
		cmdLineRequests_list->Add(" Please Enter CSV List of INcorrect Inter-Train Interval ""50, 60, 70, 80, 90, 100"": ");

		List<String^>^ art_tx_cmds_list = gcnew List<String^>();
		art_tx_cmds_list->Add("set_stim_amp_incr,");
		art_tx_cmds_list->Add("set_stim_freq_cr,");
		art_tx_cmds_list->Add("set_stim_freq_incr,");
		art_tx_cmds_list->Add("set_stim_pulsewidth_cr,");
		art_tx_cmds_list->Add("set_stim_pulsewidth_incr,");
		art_tx_cmds_list->Add("set_stim_pulsenum_cr,");
		art_tx_cmds_list->Add("set_stim_pulsenum_incr,");
		art_tx_cmds_list->Add("set_stim_carfreq_cr,");
		art_tx_cmds_list->Add("set_stim_carfreq_incr,");

		for (int i = 0; i < cmdLineRequests_list->Count; i++)
		{
			list_size = 11;
			while (list_size > 10)
			{
				Console::WriteLine(cmdLineRequests_list[i]);
				user_input_str = Console::ReadLine();
				dataFields = splitDataFields(user_input_str);
				list_size = dataFields.Length;
			}

			tx_message = art_tx_cmds_list[i] + user_input_str;
			ART_Obj->writeToSerialOut(tx_message);
			Thread::Sleep(100);
		}
	
		Console::WriteLine("");	
	}

	catch (Exception^ ex) 
	{
		Console::WriteLine("");
		Console::WriteLine("ERROR!!!! Stimulation Parameters NOT SET!!");
		Console::WriteLine("Press Enter To Exit... ");
		Console::ReadLine();
		return 0;
	}

	return 1;
}

//---------------------------------------------------------------------------
bool initART_ObjParams(ARTClass^ %ART_Obj, List<String^>^ rx_messages) 
{
	ART_Obj->version			= rx_messages[0];
	ART_Obj->dataFields_string  = rx_messages[1] + rx_messages[2] + rx_messages[3];
	ART_Obj->dataFields			= splitDataFields(ART_Obj->dataFields_string);
	ART_Obj->numFields			= ART_Obj->dataFields->Length;
	ART_Obj->mirroring			= rx_messages[4] != "";
	
	if ( VERBOSE | DEBUG )
	{
		Console::WriteLine("  VERSION: " + ART_Obj->version);
		Console::WriteLine("  MIRRORING: " + (ART_Obj->mirroring ? "True" : "False")); 
		Console::WriteLine("  TRIAL TYPE: " + ART_Obj->trialType); 
		Console::WriteLine("  DATA FIELDS: ");
		for ( int i = 0; i < ART_Obj->dataFields->Length; i++ )
			Console::WriteLine("   - " + ART_Obj->dataFields[i]); 
	}

	return true;
}

//---------------------------------------------------------------------------
static array<String^>^ splitDataFields(String^ rx_message) 
{
	// Split String 
	String^ delimStr = ",";
	array<Char>^ delimiter = delimStr->ToCharArray( );
	array<String^>^ dataFields = rx_message->Split( delimiter );
	
	// Remove Trailing/Leading White Spaces
	for ( int i = 0; i < dataFields->Length; i++ )
		dataFields[i] = dataFields[i]->Trim();

	return dataFields;
}

//---------------------------------------------------------------------------
bool exit(List<ARTClass^>^ %List_ART) 
{
	MLRT_RUNNING = false;

	Thread::Sleep(1000);

	for ( int i = 0; i < List_ART->Count; i++ )
	{
		List_ART[i]->endProcessingLoop();

		if ( VERBOSE | DEBUG )
		{
			Console::WriteLine("");
			Console::WriteLine(" Closed Serial Port: " + List_ART[i]->getDeviceName());
			Console::WriteLine(" Closed: " + List_ART[i]->getLogFileName());
		}
	}

	return 0;
}
