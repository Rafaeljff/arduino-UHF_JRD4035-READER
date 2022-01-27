#include "Arduino.h"
#include "RFID_command.h"
#include <stdio.h>
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
//Ds1302::DateTime now;
// DS1302 RTC instance
Ds1302 rtc(15, 22, 21);

//File myFile;
const int chipSelect = 5;
char filename[] = "00000000.CSV";
const static char *WeekDays[] = { "Monday", "Tuesday", "Wednesday", "Thursday",
		"Friday", "Saturday", "Sunday" };
// DS1302 RTC instance

typedef struct check_box {
	String tag_main;
	String tag_count[20];

} check_box_tags;

check_box_tags checkbox_set[20];
check_box_tags current_reading;

volatile int set_counts = 0;

const int button_read = 27;

void setup() {

	Serial.begin(115200);
	int check = 0;

	checkbox_set[0].tag_main = "3039606243961e80000928ff";
	checkbox_set[0].tag_count[0] = "3039606243961e80000928ff";

	checkbox_set[0].tag_count[5] = "e2003411b802011056264666";
	checkbox_set[1].tag_main = "e2003411b802011056264798";

	Serial2.begin(115200, SERIAL_8N1, 16, 17); //16.17
	rtc.init();
	//attachInterrupt(button_read, func_read_all, FALLING);
	RFID.Set_transmission_Power(2600);
	RFID.Set_the_Select_mode();
	RFID.Delay(100);
	RFID.Readcallback();
	RFID.clean_data();

}

void loop() {
	//card = RFID.A_single_poll_of_instructions();
	int pos;
	int check = func_read_all(&pos);
	if (set_counts == 20)
		set_counts = 1;

	if (check == 1)
		check = caixa_completa(check, pos);

	if (check == 0) {
		Serial.print("\n\nA caixa com a tag main ");
		Serial.print(pos);
		Serial.print("nao esta completa");
	} else if (check == 1) {
		Serial.print("\n\nA caixa com a tag main ");
		Serial.print(pos);
		Serial.print("esta completa");
	}

	Serial.print("\nNumber of boxes registered: ");
	Serial.print(set_counts);
	delay(5000);
	/*Serial.println("\t");
	 Serial.print(now.year);    // 00-99
	 Serial.print('-');

	 Serial.print(now.month);    // 01-12
	 Serial.print('-');
	 Serial.print(now.day);    // 01-31
	 Serial.print(' ');
	 Serial.print(WeekDays[now.dow - 1]);
	 Serial.print(' ');
	 Serial.print(now.hour);    // 00-23
	 Serial.print(':');
	 Serial.print(now.minute);    // 00-59
	 //RFID.clean_data();
	 delay(5000);
	 */

	/*
	 cards = RFID.Multiple_polling_instructions(20);
	 int check = 0;
	 int pos = 0;

	 for (size_t i = 0; i < cards.len; i++) {
	 Serial.print("\n");
	 if (i == 0) {
	 set_counts++;

	 if (cards.card[i]._EPC.length() == 24) {
	 current_reading[i] = cards.card[i]._EPC;
	 Serial.print("\t tag");
	 Serial.print(i);
	 Serial.print(":");

	 Serial.print(cards.card[i]._EPC);    // 00-99

	 for (int j = 0; j < set_counts; j++) {
	 if (cards.card[i]._EPC == checkbox_set[j].tag_main) {
	 pos = j;
	 check = 1;
	 Serial.print("\n Tag main da caixa ");
	 Serial.print(j);
	 Serial.print(" detetada");

	 } else if (cards.card[i]._EPC != checkbox_set[j].tag_main
	 && (j == set_counts - 1) && check == 0) {
	 check = 0;
	 Serial.print("\n Nao foram detetadas tags main ");
	 }

	 //check = 1;
	 }
	 }
	 }
	 }*/

}
int func_read_all(int *pos) {
	int check = 0;
	*pos = 0;
	cards = RFID.Multiple_polling_instructions(20);

	for (size_t i = 0; i < cards.len; i++) {
		current_reading.tag_count[i] = cards.card[i]._EPC;
		Serial.print("\n");

		if (cards.card[i]._EPC.length() == 24) {
			if (i == 0)
				set_counts++; // Apenas incrementa uma vez, basta haver tag

			Serial.print("\t tag");
			Serial.print(i);
			Serial.print(":");

			Serial.print(cards.card[i]._EPC); // 00-99

			for (int j = 0; j < set_counts; j++) {
				if (cards.card[i]._EPC == checkbox_set[j].tag_main) {
					*pos = j;
					check = 1;
					Serial.print("\n Tag main da caixa ");
					Serial.print(*pos);
					Serial.print(" detetada");

					/*	for (int i = 0; i < 20; i++) {
					 current_reading.tag_count[i]=cards.card[i]._EPC;
					 }*/

					RFID.clean_data();
					return check;
				} else if (cards.card[i]._EPC != checkbox_set[j].tag_main
						&& (j == set_counts - 1) && check == 0
						&& i == (cards.len - 1)) {
					check = 0;
					Serial.print("\n Nao foram detetadas tags main ");
					RFID.clean_data();
					return check;
				}

				//check = 1;
			}
		}

	}
	RFID.clean_data();
	return check;
}
int caixa_completa(int check, int pos) {
	int check2[20];
	int j = 0;

	for (int i = 0; i < 20; i++) {
		check2[i] = 0;

		for (j = 0; j < 20; j++) {
			if (checkbox_set[pos].tag_count[i]
					== current_reading.tag_count[j]) {
				check2[i] = 1;
			}
		}

		/*
		 } else if (checkbox_set[pos].tag_count[i]
		 != current_reading.tag_count[j] && (j == 20)
		 && check == 1) {
		 check = 0;
		 return check;
		 }
		 */
	}

	for (int y = 0; y < 20; y++) {
		if (check2[y] != 1)
			check = 0;
	}
	return check;

}
