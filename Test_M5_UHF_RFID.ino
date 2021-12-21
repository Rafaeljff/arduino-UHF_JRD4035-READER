#include "Arduino.h"
#include "RFID_command.h"

UHF_RFID RFID;

String comd = " ";
CardpropertiesInfo card;
ReadInfo card2;
ManyInfo cards;
SelectInfo Select;
CardInformationInfo Cardinformation;
QueryInfo Query;
ReadInfo Read;
TestInfo Test;

void setup() {
	Serial.begin(115200);
	RFID._debug = 0;
	Serial.begin(115200, SERIAL_8N1, 16, 17); //16.17
if (RFID._debug == 1){
	//Serial.begin(115200, SERIAL_8N1, 21, 22);
  Serial.println("Using Uart CHANNEL 0");
}
	RFID.Set_transmission_Power(2600);
	RFID.Set_the_Select_mode();
	RFID.Delay(100);
	RFID.Readcallback();
	RFID.clean_data();

	Serial.println("debuggggggg");

// Determined whether to connect to UHF_RFID 判断是否连接UHF_RFID
	/* String soft_version;
	soft_version = RFID.Query_software_version();
	 { while(soft_version.indexOf("V2.3.5") == -1)
	  {
	 RFID.clean_data();
	 RFID.Delay(150);
	 soft_version = RFID.Query_software_version();
	 }
	Serial.println("Please approach the RFID card you need to use");
*/}

void loop() {

	RFID.Delay(100);


	card = RFID.A_single_poll_of_instructions();
	card2=RFID.Read_working_area();
	Serial.println(card2.Region);
	Serial.println(card2.Mixer_G);
	Serial.println(card2.IF_G);
	Serial.println("Debug");
	if (card._ERROR.length() != 0) {
		Serial.println(card._ERROR);
	}
else{




	if(card._EPC.length() == 24){
     Serial.println("RSSI :" + card._RSSI);
     Serial.println("PC :" + card._PC);
     Serial.println("EPC :" + card._EPC);
     Serial.println("CRC :" + card._CRC);
     Serial.println(" ");

   }
 }


	RFID.clean_data();

	delay(1000);
}
