#include "Arduino.h"
#include "RFID_command.h"
#include <Ds1302.h>
#include <SPI.h>
#include <SD.h>
#include <FS.h>
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
Ds1302 rtc(15, 22, 21);

//File myFile;

const static char *WeekDays[] = { "Monday", "Tuesday", "Wednesday", "Thursday",
		"Friday", "Saturday", "Sunday" };
// DS1302 RTC instance

typedef struct check_box {
	String tag_main;
	String tag_count[25];

} check_box_tags;

check_box_tags checkbox_set[30];
volatile int set_counts = 0;

const int button_read = 27;

void setup() {

	Serial.begin(115200);
	int check = 0;
	/*
	 Serial.print("Initializing SD card...");
	 if (!SD.begin(10)) {
	 Serial.println("initialization failed!");
	 while (1);
	 }
	 Serial.println("initialization done.");
	 // open the file. note that only one file can be open at a time,
	 // so you have to close this one before opening another.
	 myFile = SD.open("test.txt", FILE_WRITE);
	 // if the file opened okay, write to it:
	 if (myFile) {
	 Serial.print("Writing to test.txt...");
	 myFile.println("This is a test file :)");
	 myFile.println("testing 1, 2, 3.");
	 for (int i = 0; i < 20; i++) {
	 myFile.println(i);
	 }
	 // close the file:
	 myFile.close();
	 Serial.println("done.");
	 } else {
	 // if the file didn't open, print an error:
	 Serial.println("error opening test.txt");
	 }
	 */

	checkbox_set[0].tag_main = "3039606243961e80000928ff";
	checkbox_set[0].tag_count[4] = "e2003411b802011056264790";
	checkbox_set[0].tag_count[5] = "e2003411b802011056264792";

	checkbox_set[1].tag_main = "e2003411b802011056264768";

	Serial2.begin(115200, SERIAL_8N1, 16, 17); //16.17
	rtc.init();
	//attachInterrupt(button_read, func_read_all, FALLING);
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
	//func_read_all();
	cards = RFID.Multiple_polling_instructions(30);
	int check = 0;
	if (set_counts == sizeof(checkbox_set))
		set_counts = 1;

	for (size_t i = 0; i < cards.len; i++) {
		Serial.print("\n");
		if (cards.card[i]._EPC.length() == 24) {

			checkbox_set[set_counts].tag_count[i] = cards.card[i]._EPC;

			Serial.print("\t tag");
			Serial.print(i);
			Serial.print(":");
			//Serial.print(checkbox_set[0].tag_count[i]);    // 00-99
			Serial.print(cards.card[i]._EPC);    // 00-99
			for (int j = 0; j < set_counts; j++) {
				if (cards.card[i]._EPC == checkbox_set[j].tag_main) {

					check = 1;
					Serial.print("\n Tag main da caixa ");
					Serial.print(j);
					Serial.print(" detetada");
					/*for (int z = 0; z < 20; z++) {
					 if (checkbox_set[j].tag_count[z]
					 == checkbox_set[j].tag_count) {
					 Serial.print(
					 "\n-----------------------Tag pertencente à caixa");

					 }
					 }*/
				}

			}
			if (check == 0) {
				Serial.print("\n\nTag main não detetada");
				check = 1;
			}
			if (i == 0) {
				set_counts++;
			}
			if (set_counts == 20)
				set_counts = 1;
		}
	}
	Serial.print("\nNumber of boxes registered: ");
	Serial.print(set_counts);

	RFID.clean_data();
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
			Serial.print(i + 1);
			Serial.print(":");
			Serial.print(checkbox_set[0].tag_count[i]);    // 00-99

		}

	}

	RFID.clean_data();
}

