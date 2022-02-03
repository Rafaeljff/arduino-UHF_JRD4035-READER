#include "Arduino.h"
#include "RFID_command.h"
#include <stdio.h>
#include <Wire.h>
#include <Ds1302.h>
#include <SPI.h>
#include <SD.h>
#include <FS.h>

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

Ds1302 rtc(15, 22, 21);

File myFile;

const int chipSelect = 5;
char filename[] = "/taglist.CSV";
char filename2[] = "/tags_completas.CSV";
String all_tags[200] = " ";
const static char *WeekDays[] = { "Monday", "Tuesday", "Wednesday", "Thursday",
		"Friday", "Saturday", "Sunday" };
// DS1302 RTC instance

typedef struct check_box {
	String tag_main;
	String tag_count[20];

} check_box_tags;

check_box_tags checkbox_set[20];
check_box_tags current_reading;

volatile int set_counts = 0; //numero da contagem feita pelo reader, dá reset de 20 em 20
volatile int number_of_tags = 0;
const int button_read = 27;

//List of functions pointers corresponding to each command

//List of command names
String commands_str = { "1-Read single tag"
		"\n2-Read Multiple tags"
		"\n3-Verificar caixa completa "
		"\n4-Ler do cartao SD"
		"\n5-Escrever para cartao SD" };

//List of LED sub command names

void setup() {
	Serial.begin(115200);

	Serial2.begin(115200, SERIAL_8N1, 16, 17); //16.17
	//attachInterrupt(button_read, func_read_all, FALLING);
	RFID.Set_transmission_Power(2600);
	RFID.Set_the_Select_mode();
	RFID.Delay(100);
	RFID.Readcallback();
	RFID.clean_data();

	pinMode(26, OUTPUT);
	//Wire.begin();
	pinMode(5, OUTPUT);
	//attachInterrupt(25, File_read, FALLING);
	//attachInterrupt(27, File_write, FALLING);

	Serial.print("Initializing SD card...");

	if (SD.begin(5))
		Serial.println("SD card is ready");

	if (!SD.begin(chipSelect))
		Serial.println("Card failed, or not present");
	// don't do anything more:

	Serial.println("card initialized.");
	checkbox_set[0].tag_main = "3039606243961e80000928ff";
	checkbox_set[0].tag_count[0] = "3039606243961e80000928ff"; //a posicao 0 de cada tag_count será a tag main
	checkbox_set[0].tag_count[3] = "e2003411b802011056264805";
	checkbox_set[0].tag_count[5] = "e2003411b802011056264666";

	checkbox_set[1].tag_main = "e2003411b802011056264798";
	checkbox_set[1].tag_count[0] = "e2003411b802011056264798";

}

void loop() {
	Menu_main (Read_menu_input());Serial.print("Number of tags in the system:");
	Serial.print(number_of_tags);
	delay(1000);

}

int Read_menu_input() {

	int menu_select;

	while (!Serial.available())
		;

	if (Serial.available()) {
		menu_select = Serial.parseInt();
		if (menu_select != 0 && menu_select != 1 && menu_select != 2
				&& menu_select != 3 && menu_select != 4 && menu_select != 5) {
			Serial.println("Input is incorrect");
			return -1;
		} else
			Serial.println("Menu selected: ");
		Serial.println(menu_select);
		return menu_select;

	}

}

void Menu_main(int menu_select) {
	Serial.print(commands_str);
	switch (menu_select) {
	case 1:

		read_single_tag();
		break;

	case 2:

		read_multiple_tags();
		break;
	case 3:
		break;
	case 4:
		break;
	case 5:
		break;

	default:

		break;
	}
}

void read_single_tag() {

	card = RFID.A_single_poll_of_instructions();
	Serial.print("\nCard ID:");
	Serial.print(card._EPC);

	RFID.clean_data();
}

void read_multiple_tags() {

	cards = RFID.Multiple_polling_instructions(20);

	for (size_t i = 0; i < cards.len; i++) {
		Serial.print("\nCard no. ");
		Serial.print(i);
		Serial.print(cards.card[i]._EPC);
		verificar_tag_existente(cards.card->_EPC);

	}
	RFID.clean_data();
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

	}

	for (int y = 0; y < 20; y++) {
		if (check2[y] != 1)
			check = 0;
	}

	info_caixa(check, pos);

	return check;

}
void info_caixa(int check, int pos) {
	Ds1302::DateTime now;
	rtc.getDateTime(&now);
	if (check == 0) {
		Serial.print("\nA caixa com a tag main ");
		Serial.print(pos);
		Serial.print(" nao esta completa");
	} else if (check == 1) {
		Serial.print("\nA caixa com a tag main ");
		Serial.print(pos);
		Serial.print(" esta completa");
	}
	Serial.print("\n");
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
}

void File_read() {
	myFile = SD.open(filename);
	if (myFile) {

		// read from the file until there's nothing else in it:
		while (myFile.available()) {
			Serial.write(myFile.read());
		}
		// close the file:
		myFile.close();
	} else {
		// if the file didn't open, print an error:
		Serial.println("error opening test.txt");
	}
}
void File_write() {

	//Check file name exist?
	myFile = SD.open(filename, FILE_WRITE);

	// if the file opened okay, write to it:
	if (myFile.available() == true) {

		myFile.println("testing 1, 2, 3.");
	   myFile.print(";");


		// ; corresponde a uma coluna seguinte, pode ficar para dedicar a uma designacao

		// close the file:
		myFile.close();
		Serial.println("done.");
	} else {
		// if the file didn't open, print an error:
		Serial.println("error opening test.txt");
	}
}

int verificar_tag_existente(String tag) {
	for (int i = 0; i < number_of_tags; i++) {
		if (tag == all_tags[i]) {
			return NULL;
		}
	}
	myFile = SD.open(filename, FILE_WRITE);

	// caso exista uma tag nova no sistema coloca no cartão SD
	if (myFile) {

		//myFile.println("");

		myFile.print(tag);
		 myFile.println(",");


		// ; corresponde a uma coluna seguinte, pode ficar para dedicar a uma designacao

		// close the file:
		myFile.close();
		Serial.println("done.");
	}
	number_of_tags++;
	all_tags[number_of_tags] = tag;

	return NULL;
}

