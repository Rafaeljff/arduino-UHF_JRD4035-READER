#include "Arduino.h"
#include "RFID_command.h"
#include <Ds1302.h>
#define OFFSETOF(TYPE, ELEMENT) ((size_t)&(((TYPE *)0)->ELEMENT))

//UHF_JRD3045
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

// DS1302 RTC instance
Ds1302 rtc(5, 22, 21);
const static char *WeekDays[] = { "Monday", "Tuesday", "Wednesday", "Thursday",
		"Friday", "Saturday", "Sunday" };
// DS1302 RTC instance

typedef struct check_box {
	String tag_count[10];

} check_box_tags;
check_box_tags checkbox_set[30];
const int button_read = 27;
void setup() {

	Serial.begin(115200);

	Serial2.begin(115200, SERIAL_8N1, 16, 17); //16.17
	rtc.init();
	attachInterrupt(button_read, func_read_all, FALLING);
	RFID.Set_transmission_Power(2600);
	RFID.Set_the_Select_mode();
	RFID.Delay(100);
	RFID.Readcallback();
	RFID.clean_data();

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

	//card = RFID.A_single_poll_of_instructions();
	Ds1302::DateTime now;
	rtc.getDateTime(&now);

	//Cardinformation = RFID.NXP_Change_EAS(0x00000000);
	//Serial.println("RSSI :" + cards.card[1]._RSSI);
	// Serial.println("PC :" + cards.card[1]._PC);
	//Serial.println("EPC :" + cards.card[1]._EPC);
	///kSerial.println(" ");
	 func_read_all();
	rtc.getDateTime(&now);
	 Serial.println("\t");
	 Serial.print(now.year);    // 00-99
	 Serial.print('-');

	 Serial.print(now.month);   // 01-12
	 Serial.print('-');
	 Serial.print(now.day);     // 01-31
	 Serial.print(' ');
	 Serial.print(WeekDays[now.dow - 1]);
	 Serial.print(' ');
	 Serial.print(now.hour);    // 00-23
	 Serial.print(':');
	 Serial.print(now.minute);  // 00-59




	 delay(5000);

	 //delay(1000);
}
void func_read_all(void) {

	cards = RFID.Multiple_polling_instructions(10);
	for (size_t i = 0; i < cards.len; i++) {
		Serial.print("\n");
		if (cards.card[i]._EPC.length() == 24) {

			checkbox_set[0].tag_count[i] = cards.card[i]._EPC;
			Serial.print("\t tag");
			Serial.print(i);
			Serial.print(":");
			Serial.print(checkbox_set[0].tag_count[i]);    // 00-99

		}

	}

RFID.clean_data();
}


