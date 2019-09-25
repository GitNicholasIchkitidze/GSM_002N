
#include <Keypad.h>
//#include "c:\Program Files (x86)\Arduino\libraries\Keypad\Keypad.h"
#include <EEPROM.h>
#include <LED.h>
const int EEPROM_MIN_ADDR = 0;
const int EEPROM_MAX_ADDR = 511;


const int BUFSIZE = 13;
char buf[BUFSIZE];
String myString;
char myStringChar[BUFSIZE];

#define SERIAL_DEBUG 1
#define SERIALPRINT Serial.print
#define SERIALPRINTLN Serial.println

//uint8_t diezPos = 0;
//uint8_t keyPos = 0;

//void eeWriteString(char add, String data);
//String eeRead_String(char add);




uint8_t maxCodeLength = 4;

String _response = "";                                     // Переменная для хранения ответа модуля
long lastUpdate = millis();                                   // Время последнего обновления
long updatePeriod = 60000;                                  // Проверять каждую минуту

//String phones = "+995577110487, +995595253505, +995593940169";   // Белый список телефонов

String masterCode = "0000";
String doorCode = "1111";
String phoneNumber = "+995577110487";
String IMEI;
/*
uint8_t ledRed = 22;
uint8_t ledGreen = 24;
*/
uint8_t DoorTriggerPin = 52;



LED ledRed = LED(22);
LED ledGreen = LED(24);


String mMode = "IDLE";

// STATE MACHINE

//enum stateMODES {
//	idleMODE, editMODE, editMasterCodeMODE, editdoorCodeMODE, editPhoneNumberMODE
//};

//stateMODES stateMODE = idleMODE;

uint8_t stateMODE = 0;




const byte ROWS = 4; // Four rows
const byte COLS = 3; //  columns
// Define the Keymap
char keys[ROWS][COLS] = {
  {'1','2','3'},
  {'4','5','6'},
  {'7','8','9'},
  {'*','0','#'}
};
// Connect keypad ROW0, ROW1, ROW2 and ROW3 to these Arduino pins.
byte rowPins[ROWS] = { 5, 6, 7, 8 };// Connect keypad COL0, COL1 and COL2 to these Arduino pins.
byte colPins[COLS] = { 2, 3, 4 };


// Create the Keypad
Keypad kpd = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);


String inputString = "";         // a String to hold incoming data






String sendATCommand(String cmd, bool waiting, String Decr = "") {
	String _resp = "";                                              // Переменная для хранения результата

#ifdef SERIAL_DEBUG
	SERIALPRINTLN(Decr + cmd);                                            // Дублируем команду в монитор порта
#endif // SERIAL_DEBUG

	
	Serial3.println(cmd);                                            // Отправляем команду модулю
	if (waiting) {                                                  // Если необходимо дождаться ответа...
		_resp = waitResponse();                                       // ... ждем, когда будет передан ответ
		// Если Echo Mode выключен (ATE0), то эти 3 строки можно закомментировать
		if (_resp.startsWith(cmd)) {                                  // Убираем из ответа дублирующуюся команду
			_resp = _resp.substring(_resp.indexOf("\r", cmd.length()) + 2);
		}


#ifdef SERIAL_DEBUG
		//SERIALPRINTLN("[" + _resp +"]");                                        // Дублируем ответ в монитор порта
		SERIALPRINTLN(_resp);                                        // Дублируем ответ в монитор порта
#endif // SERIAL_DEBUG

	}
	return _resp;                                                   // Возвращаем результат. Пусто, если проблема
}



String waitResponse() {                                           // Функция ожидания ответа и возврата полученного результата
	String _resp = "";                                              // Переменная для хранения результата
	long _timeout = millis() + 10000;                               // Переменная для отслеживания таймаута (10 секунд)
	while (!Serial3.available() && millis() < _timeout) {};         // Ждем ответа 10 секунд, если пришел ответ или наступил таймаут, то...
	if (Serial3.available()) {                                       // Если есть, что считывать...
		_resp = Serial3.readString();                                  // ... считываем и запоминаем
	}
	else {                                                          // Если пришел таймаут, то...

#ifdef SERIAL_DEBUG
		SERIALPRINTLN("Timeout...");                                 // ... оповещаем об этом и...
#endif // SERIAL_DEBUG

	}
	return _resp;                                                   // ... возвращаем результат. Пусто, если проблема
}

bool hasmsg = false;                                              // Флаг наличия сообщений к удалению




void parseSMS(String msg) {                                   // Парсим SMS
	String msgheader = "";
	String msgbody = "";
	String msgphone = "";

	msg = msg.substring(msg.indexOf("+CMGR: "));
	msgheader = msg.substring(0, msg.indexOf("\r"));            // Выдергиваем телефон

	msgbody = msg.substring(msgheader.length() + 2);
	msgbody = msgbody.substring(0, msgbody.lastIndexOf("OK"));  // Выдергиваем текст SMS
	msgbody.trim();

	int firstIndex = msgheader.indexOf("\",\"") + 3;
	int secondIndex = msgheader.indexOf("\",\"", firstIndex);
	msgphone = msgheader.substring(firstIndex, secondIndex);

#ifdef SERIAL_DEBUG
	SERIALPRINTLN("Phone: " + msgphone);                       // Выводим номер телефона
	SERIALPRINTLN("Message: " + msgbody);                      // Выводим текст SMS
	SERIALPRINTLN("Registered Phone: " + getPhoneNum());                      // Выводим текст SMS

#endif // SERIAL_DEBUG

	
	//if (msgphone.length() > 6 && phoneNumber.indexOf(msgphone) > -1) { // Если телефон в белом списке, то...
	if (msgphone.length() > 6 && ("+" + getPhoneNum()).indexOf(msgphone) > -1) { // Если телефон в белом списке, то...
		//setLedState(msgbody, msgphone);                           // ...выполняем команду
		setCode(msgbody, msgphone);
#ifdef SERIAL_DEBUG
		SERIALPRINTLN("ACTION");
#endif // SERIAL_DEBUG
	}
	else {
#ifdef SERIAL_DEBUG
		SERIALPRINTLN("Unknown phonenumber");
#endif // SERIAL_DEBUG
	}
}
/*


void setLedState(String result, String phone) {
	bool correct = false;                                       // Для оптимизации кода, переменная корректности команды
	String msgToSend = "";
	if (result.length() == 2) {
		int ledIndex = ((String)result[0]).toInt();               // Получаем первую цифру команды - адрес устройства (1-3)
		int ledState = ((String)result[1]).toInt();               // Получаем вторую цифру команды - состояние (0 - выкл, 1 - вкл)
		if (ledIndex >= 1 && ledIndex <= 3 && (ledState == 0 or ledState == 1)) { // Если все нормально, исполняем команду
			msgToSend = "LED:" + (String)ledIndex + " set to " + (ledState == 0 ? "OFF" : "ON"); // Статус исполнения
			digitalWrite(pins[ledIndex - 1], ledState);             // Исполняем команду
			correct = true;                                         // Флаг корректности команды
		}
	}
	if (!correct) {
		msgToSend = "Incorrect command: " + result;               // Статус исполнения
	}
	SERIALPRINTLN(msgToSend);                                  // Выводим результат в терминал
}
*/
void setCode(String inStr, String phone) {
	//bool correct = false;                                       // Для оптимизации кода, переменная корректности команды
	String msgToSend = "";
	String cmdKind = "";
	if ((inStr.length() > 3) && (inStr.length() <14))
	{
		cmdKind = inStr.substring(0, 3);
		cmdKind.toLowerCase();
#ifdef SERIAL_DEBUG
		SERIALPRINTLN(cmdKind);
		SERIALPRINTLN(inStr.substring(3, 7));
#endif // SERIAL_DEBUG
		if (cmdKind == "set")
		{
			doorCode = inStr.substring(3, 7);
			msgToSend = "[" + cmdKind + "] [" + doorCode + "]";
			//eeWriteString(10, doorCode);

			if (doorCode != getMasterCode())
			{
				myString = doorCode;
				myString.toCharArray(myStringChar, BUFSIZE); //convert string to char array
				strcpy(buf, myStringChar);
				eeprom_write_string(10, buf);
				#ifdef SERIAL_DEBUG
					SERIALPRINTLN("Door CODE CHANGED"); 
				#endif // SERIAL_DEBUG

				sendSMS(phone, "Dasturi: karis axali kodi gaxda: \n*" + doorCode + "#");
			}
			else
			{
				sendSMS(phone, "akrdzaluli kodi, sxva kodi momecit");
			}




			
			//sendSMS(phone, "New Code set to " + eeRead_String(10));
		}
		else if (cmdKind == "cur") 
		{
			sendSMS(phone, "master kodi: " + getMasterCode() + ", da karis kodi: " + getDoorCode());
			//sendSMS(phone, "current code " + doorCode);
		}
		else if (cmdKind == "ope")
		{
			openDoor();
			sendSMS(phone, "kari gaigo " );
		}

	}
	else
	{
		msgToSend = "Incorrect command: " + inStr;               // Статус исполнения
	}


#ifdef SERIAL_DEBUG
	SERIALPRINTLN(msgToSend);                                  // Выводим результат в терминал
#endif // SERIAL_DEBUG

}


void sendSMS(String phone, String message)
{
	sendATCommand("AT+CMGS=\"" + phone + "\"", true);             // Переходим в режим ввода текстового сообщения
	sendATCommand(message + "\r\n" + (String)((char)26), true);   // После текста отправляем перенос строки и Ctrl+Z
}





void setup() {
  //for (int i = 0; i < 3; i++) {
  //  pinMode(pins[i], OUTPUT);                                 // Настраиваем пины в OUTPUT
  //}

  Serial.begin(9600);                                         // Скорость обмена данными с компьютером
  EEPROM.begin();

  Serial3.begin(9600);                                         // Скорость обмена данными с модемом
#ifdef SERIAL_DEBUG
  SERIALPRINTLN("GMS Module Start!....");
#endif // SERIAL_DEBUG



  sendATCommand("AT", true, "MODEM OK?\t");                                  // Отправили AT для настройки скорости обмена данными
  sendATCommand("AT+CMGDA=\"DEL ALL\"", true, "DEL ALL SMS\t");       // Удаляем все SMS, чтобы не забивать память
  sendATCommand("AT+CMEE=2", true);             // DEBUG MODE
  sendATCommand("ATI", true, "GET DEV INFO\t");                 // INFO About DEVICE
  sendATCommand("AT+CCALR?", true, "MODEM READY?\t");                 // DEVICE REAADY STATUS
  IMEI = sendATCommand("AT+GSN", true, "DEVICE SER NUM (IMEI)\t");                 //IMEI
  IMEI = "866192034487205";
  sendATCommand("AT+CCID", true, "SIM ID\t");                 //IMEI
  
  sendATCommand("AT+CNUM", true, "SIM NUM\t");                  //IMEI
  

  // Команды настройки модема при каждом запуске
  //_response = sendATCommand("AT+CLIP=1", true);             // Включаем АОН
  //_response = sendATCommand("AT+DDET=1", true);             // Включаем DTMF
  sendATCommand("AT+CMGF=1;&W", true);                        // Включаем текстовый режима SMS (Text mode) и сразу сохраняем значение (AT&W)!
  sendATCommand("AT+CNMI=?",true); // Decides how newly arrived SMS messages should be handled  
  //sendATCommand("AT+CNMI=1,2,0,0,0"); // Decides how newly arrived SMS messages should be handled
  lastUpdate = millis();                                      // Обнуляем таймер

  kpd.addEventListener(keypadEvent); //add an event listener for this keypad








  //pinMode(ledRed, OUTPUT);
  //pinMode(ledGreen, OUTPUT);
  pinMode(DoorTriggerPin, OUTPUT);
  //digitalWrite(ledRed, HIGH);
  //digitalWrite(ledGreen, HIGH);
  digitalWrite(DoorTriggerPin, HIGH);

  ledRed.on();
  ledGreen.on();


  //SERIALPRINTLN("phones:" + phones);   // Белый список телефонов


//doorCode = eeRead_String(10);
	doorCode = "1111";

  //mMode = "IDLE";

  //stateMODE = idleMODE;
  stateMODE = 0;

  /*
  SERIALPRINT("Reading string from address 0 for MasterCode: ");
  eeprom_read_string(0, buf, BUFSIZE);
  SERIALPRINTLN(buf);

  masterCode = String(buf);


  SERIALPRINT("Reading string from address 10 for doorCode: ");
  eeprom_read_string(10, buf, BUFSIZE);
  SERIALPRINTLN(buf);

  doorCode = String(buf);


  SERIALPRINT("Reading string from address 15 for phones: ");
  eeprom_read_string(15, buf, BUFSIZE);
  SERIALPRINTLN(buf);

  phoneNumber = "+" + String(buf);

  */

  masterCode = getMasterCode();
  doorCode = getDoorCode();
  phoneNumber = "+" + getPhoneNum();

#ifdef SERIAL_DEBUG
  SERIALPRINTLN("IMEI\t" + IMEI);
  SERIALPRINTLN("MasterCode:\t" + masterCode);
  SERIALPRINTLN("doorCode:\t" + doorCode);
  SERIALPRINTLN("phones:\t" + phoneNumber);   // Белый список телефонов

#endif // SERIAL_DEBUG

  
}

void loop() 
{

    kpd.getKey();

  if (lastUpdate + updatePeriod < millis()) {                    // Пора проверить наличие новых сообщений
    do {
      _response = sendATCommand("AT+CMGL=\"REC UNREAD\",1", true, "CHECK SMS\t");// Отправляем запрос чтения непрочитанных сообщений
      if (_response.indexOf("+CMGL: ") > -1) {                    // Если есть хоть одно, получаем его индекс
        int msgIndex = _response.substring(_response.indexOf("+CMGL: ") + 7, _response.indexOf("\"REC UNREAD\"", _response.indexOf("+CMGL: ")) - 1).toInt();
        char i = 0;                                               // Объявляем счетчик попыток
        do {
          i++;                                                    // Увеличиваем счетчик
          _response = sendATCommand("AT+CMGR=" + (String)msgIndex + ",1", true, "TRY READ SMS\t");  // Пробуем получить текст SMS по индексу
          _response.trim();                                       // Убираем пробелы в начале/конце
          if (_response.endsWith("OK")) {                         // Если ответ заканчивается на "ОК"
            if (!hasmsg) hasmsg = true;                           // Ставим флаг наличия сообщений для удаления
            sendATCommand("AT+CMGR=" + (String)msgIndex, true);   // Делаем сообщение прочитанным
            sendATCommand("\n", true);                            // Перестраховка - вывод новой строки
            parseSMS(_response);                                  // Отправляем текст сообщения на обработку
            sendATCommand("AT+CMGDA=\"DEL READ\"", true, "DELETE SMS\t");           // Удаляем все SMS
            break;                                                // Выход из do{}
          }
          else {                                                  // Если сообщение не заканчивается на OK


#ifdef SERIAL_DEBUG
			  SERIALPRINTLN("Error answer");                      // Какая-то ошибка
#endif // SERIAL_DEBUG

            sendATCommand("\n", true);                            // Отправляем новую строку и повторяем попытку
          }
        } while (i < 10);
        break;
      }
      else {
        lastUpdate = millis();                                    // Обнуляем таймер
        if (hasmsg) {
          sendATCommand("AT+CMGDA=\"DEL READ\"", true);           // Удаляем все прочитанные сообщения
          hasmsg = false;
        }
        break;
      }
    } while (1);
  }

  if (Serial3.available()) {                         // Если модем, что-то отправил...
    _response = waitResponse();                       // Получаем ответ от модема для анализа
    _response.trim();                                 // Убираем лишние пробелы в начале и конце
#ifdef SERIAL_DEBUG
	SERIALPRINTLN(_response);                        // Если нужно выводим в монитор порта
#endif // SERIAL_DEBUG


    if (_response.indexOf("+CMTI:") > -1) {             // Пришло сообщение об отправке SMS
      lastUpdate = millis() - updatePeriod;          // Теперь нет необходимости обрабатываеть SMS здесь, достаточно просто
                              // сбросить счетчик автопроверки и в следующем цикле все будет обработано
    }
  }
  if (Serial.available()) {                          // Ожидаем команды по Serial...
    Serial3.write(Serial.read());                      // ...и отправляем полученную команду модему
  };
}
