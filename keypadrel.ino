


void keypadEvent(KeypadEvent eKey) 
{
  switch (kpd.getState())
  {
  case IDLE:
  {
	  //SERIALPRINTLN("\nIDLE");
	  break;
  }
  case HOLD:
  {
	  SERIALPRINTLN("\nHOLD");
	  //char inChar = (char)eKey;
	  switch (eKey)
	  {
	  case '*':
			{
			 ledGreen.off();

			SERIALPRINTLN("\nZARI REKAVS!!!!!!");
			break;
			}
	  default:
			{
			break;
			}

	  }

	  break;
  }
  case RELEASED:
  {

	  switch (eKey)
	  {
	  case '*':
	  {
		  ledGreen.on();

		  //SERIALPRINTLN("\nZARI REKAVS!!!!!!");
		  break;
	  }
	  default:
	  {
		  break;
	  }

	  }

	  //SERIALPRINTLN("\nRELEASED");
	  break;
  }
  case PRESSED:
  {
    //SERIALPRINT("Pressed: ");
    SERIALPRINT(eKey);
	

    char inChar = (char)eKey;
    switch (eKey)
    {
    case '*': 
	{
		//digitalWrite(ledRed, LOW);
		ledGreen.on();
		ledRed.on();
		startPassword(); 
		break;
		
	}

    case '#': 
	{

		
		//digitalWrite(ledGreen, LOW);

		#ifdef SERIAL_DEBUG
				SERIALPRINT("\ninput = \t" + inputString + "\tMode before press # = " + stateMODE);
		#endif // SERIAL_DEBUG

		checkResult1(); 
		//delay(50);
		////////////////////////////////


		switch (stateMODE)
		{
		case 0:
		{
#ifdef SERIAL_DEBUG
			SERIALPRINTLN("\t Mode = " + String(stateMODE) + " - [IDLE MODE] enter MasterCode or DoorCode");
#endif // SERIAL_DEBUG


			break;
		}

		case 1:
		{
#ifdef SERIAL_DEBUG
			SERIALPRINTLN("\t Mode = " + String(stateMODE) + " - [EDIT MODE] enter 0,1,2 or * ");
#endif // SERIAL_DEBUG
			break;
		}
		case 11:
		{
#ifdef SERIAL_DEBUG
			SERIALPRINTLN("\t Mode = " + String(stateMODE) + " - [EDIT MASTER CODE] enter new MASTER code and then # enter or * to go root ");
#endif // SERIAL_DEBUG
			break;
		}
		case 12:
		{
#ifdef SERIAL_DEBUG
			SERIALPRINTLN("\t Mode = " + String(stateMODE) + " - [EDIT DOOR CODE] enter new  DOOR code and then # enter or * to go root ");
#endif // SERIAL_DEBUG
			break;
		}
		case 13:
		{
#ifdef SERIAL_DEBUG
			SERIALPRINTLN("\t Mode = " + String(stateMODE) + " - [EDIT PHONE NUM] enter PHONE NUMBER and then # enter or * to go root ");
#endif // SERIAL_DEBUG
			break;
		}

		}
		////////////////////////////////
		break;
	}
    default:
    {

		ledGreen.on();
		ledRed.on();

      inputString += inChar;
	  break;

    }
    }

	//SERIALPRINTLN("\t Mode = " + String(stateMODE));

	

	break;
    //SERIALPRINTLN(inputString);

  }


  }
}



void startPassword() 
{
  inputString = "";
  //stateMODE = idleMODE;
  stateMODE = 0;
  //keyPos = 0;
  //digitalWrite(ledPin1, LOW);
}

void checkResult1()
{
	
	switch (stateMODE)
	{




	case 0:
	//case idleMODE:
	{

		if ( (inputString == getMasterCode()) || (inputString == IMEI) )
		{

			//SERIALPRINT("\tmaster kdodi");			// editMODE
			stateMODE = 1;
			ledGreen.blink(1000, 3);
			//SERIALPRINTLN("\tstateMODE=" + stateMODE);			// editMODE
			//SERIALPRINTLN("\nEDIT MODE");

		}
		//else if (inputString == doorCode)
		else if (inputString == getDoorCode())
		{
			openDoor();
		}
		else if (inputString == "8881") 
		{

			#ifdef SERIAL_DEBUG
					SERIALPRINTLN("\nMasterCode:\t" + getMasterCode());
					SERIALPRINTLN("doorCode:\t" + getDoorCode());
					SERIALPRINTLN("phones:\t" + getPhoneNum());   // Белый список телефонов
			#endif // SERIAL_DEBUG

			ledGreen.blink(1000, 3);
		}

		else
		{
			#ifdef SERIAL_DEBUG
					SERIALPRINTLN("\tCODE FAILED");
			#endif // SERIAL_DEBUG
		

			ledRed.blink(500,3);
		}
		break;
	}

	case 1:
	{
		if (inputString == "0")
		{
			stateMODE = 11;
			//SERIALPRINTLN("stateMODE=" + stateMODE);			// editMasterCodeMODE
			//SERIALPRINTLN("\nEDIT MODE");
			ledGreen.blink(1000, 3);
		}
		else if (inputString == "1")
		{
			stateMODE = 12;
			//SERIALPRINTLN("stateMODE=" + stateMODE);			// editdoorCodeMODE
			ledGreen.blink(1000, 3);
		}
		else if (inputString == "2")
		{
			stateMODE = 13;
			//SERIALPRINTLN("stateMODE=" + stateMODE);			// editPhoneNumberMODE
			ledGreen.blink(1000, 3);
		}

		break;
	}

	case 11:
	{
		if ((inputString.length() == maxCodeLength) && (inputString != getDoorCode()))
		{
			masterCode = inputString;
			//stateMODE = 0;

			//putMasterCode(inputString);

			myString = inputString;
			myString.toCharArray(myStringChar, BUFSIZE); //convert string to char array
			strcpy(buf, myStringChar);
			eeprom_write_string(0, buf);
			#ifdef SERIAL_DEBUG
					SERIALPRINTLN("\tMaster CODE CHANGED");
			#endif // SERIAL_DEBUG

			stateMODE = 0;
			ledGreen.blink(300, 5);
		}
		else
		{
			#ifdef SERIAL_DEBUG
					SERIALPRINTLN("\tMaster CODE NOT CHANGED");

			#endif // SERIAL_DEBUG
			stateMODE = 0;
			ledRed.blink(300, 5);
		}
		break;
	}
	case 12:
	{
		if ((inputString.length() == maxCodeLength) && (inputString != getMasterCode()))
		{

			doorCode = inputString;
			
			//putDoorCode(inputString);
			//SERIALPRINTLN("Door CODE CHANGED & stateMODE=" + stateMODE);

			myString = inputString;
			myString.toCharArray(myStringChar, BUFSIZE); //convert string to char array
			strcpy(buf, myStringChar);
			eeprom_write_string(10, buf);
			
			#ifdef SERIAL_DEBUG
					SERIALPRINTLN("\tDoor CODE CHANGED");
			#endif // SERIAL_DEBUG

			sendSMS("+" + getPhoneNum(), "Dasturi: karebis axali kodi gaxda\n *" + inputString +"#");

			stateMODE = 0;
			ledGreen.blink(300, 5);
			
		}
		else
		{
			#ifdef SERIAL_DEBUG
					SERIALPRINTLN("\tDoor CODE NOT CHANGED");
			#endif // SERIAL_DEBUG

			ledRed.blink(300, 5);
			stateMODE = 0;
		}
		break;
	}
	case 13:
	{
		if (inputString.length() == 12)
		{

			phoneNumber = inputString;
			//stateMODE = 0;

			myString = inputString;
			myString.toCharArray(myStringChar, BUFSIZE); //convert string to char array
			strcpy(buf, myStringChar);
			eeprom_write_string(15, buf);
			#ifdef SERIAL_DEBUG
					SERIALPRINTLN("\tPhone NUMBER CHANGED");
			#endif // SERIAL_DEBUG

			sendSMS("+" + inputString, "nomeri: +" + inputString + ", daregistrirda rogorc admini ");

			stateMODE = 0;
			ledGreen.blink(300, 5);
		}
		else
		{
			#ifdef SERIAL_DEBUG
					SERIALPRINTLN("\tPhone NUMBER NOT CHANGED");
			#endif // SERIAL_DEBUG

			ledRed.blink(300, 5);
			stateMODE = 0;
		}
		break;
	}

	default:
	{
		break;
	}
	}


	
	
	
	inputString = "";

	
}






	/*
void checkResult() 
{
	if (mMode == "IDLE")
	{
		if (inputString == masterCode) {
			SERIALPRINTLN("\nEDIT MODE");
			mMode = "EDITMODE";

		}
		else if (inputString == doorCode) {
		//if (inputString == eeRead_String(10)) {
			SERIALPRINTLN("\tSuccess");
			//Add code to run if it works
			digitalWrite(ledPin1, HIGH);
		}
		else if (inputString == "000099")
		{
			SERIALPRINTLN("\nEDIT CODE MODE");
			mMode = "EDITCODE";
			//add code to run if it did not work
			digitalWrite(ledPin1, LOW);
		}
		else if (inputString == "000098")
		{
			SERIALPRINTLN("\nEDIT PHONE MODE");
			mMode = "EDITPHONE";
			//add code to run if it did not work
			digitalWrite(ledPin1, LOW);
		}

		else
		{
			SERIALPRINTLN("\tWrong");
			//Add code to run if it works
			digitalWrite(ledPin1, HIGH);

		}


		inputString = "";
	}	
	else if (mMode == "EDITCODE")
	{
		doorCode = inputString;
		
		SERIALPRINTLN("\nNew Code has been Set " + doorCode);
		//eeWriteString(10, doorCode);
		inputString = "";
		mMode = "IDLE";
	}
	else if (mMode == "EDITPHONE")
	{
		//doorCode = inputString;
		
		SERIALPRINTLN("\nNew Phone Number has been Set " + doorCode);
		//SERIALPRINTLN("\nNew Phone Number has been Set " + eeRead_String(10));
		inputString = "";
		mMode = "IDLE";
	}

}
*/