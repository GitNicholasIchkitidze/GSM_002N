
/*
 * Circuits4you.com
 * Reading and Writing String to EEPROM Example Code
 * Oct 2018
 */


 /*
void setupi() {
	// put your setup code here, to run once:
	Serial.begin(9600);
	EEPROM.begin();
	String data = "Hello World";

	SERIALPRINT("Writing Data:");
	SERIALPRINTLN(data);

	writeString(10, data);  //Address 10 and String type data
	delay(10);
}

void loopi() {
	// put your main code here, to run repeatedly:
	String recivedData;
	recivedData = read_String(10);
	SERIALPRINT("Read Data:");
	SERIALPRINTLN(recivedData);
	delay(1000);
}

*/

/*
void eeWriteString(char add, String data)
{
	int _size = data.length();
	int i;
	for (i = 0; i < _size; i++)
	{
		EEPROM.write(add + i, data[i]);
	}
	EEPROM.write(add + _size, '\0');   //Add termination null character for String Data
	//EEPROM.commit();
}



String eeRead_String(char add)
{
	int i;
	char data[100]; //Max 100 Bytes
	int len = 0;
	unsigned char k;
	k = EEPROM.read(add);
	while (k != '\0' && len < 500)   //Read until null character
	{
		k = EEPROM.read(add + len);
		data[len] = k;
		len++;
	}
	data[len] = '\0';
	return String(data);
}
*/




// Returns true if the address is between the
// minimum and maximum allowed values, false otherwise.
//
// This function is used by the other, higher-level functions
// to prevent bugs and runtime errors due to invalid addresses.
boolean eeprom_is_addr_ok(int addr)
{
	return ((addr >= EEPROM_MIN_ADDR) && (addr <= EEPROM_MAX_ADDR));
}


// Writes a sequence of bytes to eeprom starting at the specified address.
// Returns true if the whole array is successfully written.
// Returns false if the start or end addresses aren't between
// the minimum and maximum allowed values.
// When returning false, nothing gets written to eeprom.
boolean eeprom_write_bytes(int startAddr, const byte* array, int numBytes)
{

	int i;  // counter

	// both first byte and last byte addresses must fall within
	// the allowed range 
	if (!eeprom_is_addr_ok(startAddr) || !eeprom_is_addr_ok(startAddr + numBytes))
	{
		return false;
	}

	for (i = 0; i < numBytes; i++)
	{
		EEPROM.write(startAddr + i, array[i]);
	}
	return true;
}



// Writes a string starting at the specified address.
// Returns true if the whole string is successfully written.
// Returns false if the address of one or more bytes fall outside the allowed range.
// If false is returned, nothing gets written to the eeprom.
boolean eeprom_write_string(int addr, const char* string)
{

	int numBytes; // actual number of bytes to be written
	//write the string contents plus the string terminator byte (0x00)
	numBytes = strlen(string) + 1;
	return eeprom_write_bytes(addr, (const byte*)string, numBytes);
}



boolean eeprom_read_string(int addr, char* buffer, int bufSize) {
	byte ch; // byte read from eeprom
	int bytesRead; // number of bytes read so far

	if (!eeprom_is_addr_ok(addr)) { // check start address
		return false;
	}

	if (bufSize == 0) { // how can we store bytes in an empty buffer ?
		return false;
	}

	// is there is room for the string terminator only, no reason to go further
	if (bufSize == 1) {
		buffer[0] = 0;
		return true;
	}

	bytesRead = 0; // initialize byte counter
	ch = EEPROM.read(addr + bytesRead); // read next byte from eeprom
	buffer[bytesRead] = ch; // store it into the user buffer
	bytesRead++; // increment byte counter


  // stop conditions:
  // - the character just read is the string terminator one (0x00)
  // - we have filled the user buffer
  // - we have reached the last eeprom address
	while ((ch != 0x00) && (bytesRead < bufSize) && ((addr + bytesRead) <= EEPROM_MAX_ADDR))
	{
		// if no stop condition is met, read the next byte from eeprom
		ch = EEPROM.read(addr + bytesRead);
		buffer[bytesRead] = ch; // store it into the user buffer
		bytesRead++; // increment byte counter
	}

	// make sure the user buffer has a string terminator, (0x00) as its last byte
	if ((ch != 0x00) && (bytesRead >= 1))
	{
		buffer[bytesRead - 1] = 0;
	}


	return true;
}

String getMasterCode()
{
	if (eeprom_read_string(0, buf, BUFSIZE))
		return buf;
	else
		return "";

}

String getDoorCode()
{
	if (eeprom_read_string(10, buf, BUFSIZE))
		return buf;
	else
		return "";

}

String getPhoneNum()
{
	if (eeprom_read_string(15, buf, BUFSIZE))
		return buf;
	else
		return "";

}


void putMasterCode(String val)
{

	String myString;
	char myStringChar[BUFSIZE];

	myString = val;
	myString.toCharArray(myStringChar, BUFSIZE); //convert string to char array
	strcpy(buf, myStringChar);
	eeprom_write_string(0, buf);

}


void putDoorCode(String val)
{

	String myString;
	char myStringChar[BUFSIZE];

	myString = val;
	myString.toCharArray(myStringChar, BUFSIZE); //convert string to char array
	strcpy(buf, myStringChar);
	eeprom_write_string(10, buf);

}


void putPhoneNum(String val)
{

	String myString;
	char myStringChar[BUFSIZE];

	myString = val;
	myString.toCharArray(myStringChar, BUFSIZE); //convert string to char array
	strcpy(buf, myStringChar);
	eeprom_write_string(15, buf);

}