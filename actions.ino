void openDoor()
{


	

	digitalWrite(DoorTriggerPin, LOW);
	delay(500);
	digitalWrite(DoorTriggerPin, HIGH);

#ifdef SERIAL_DEBUG
	SERIALPRINTLN("\tDOOR OPENED NOW");
#endif // SERIAL_DEBUG

	ledGreen.off();
	//delay(50);
	//ledGreen.on();
		
	//Add code to run if it works
	//digitalWrite(ledPin1, HIGH);
}
