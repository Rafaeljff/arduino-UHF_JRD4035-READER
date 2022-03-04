#include "Arduino.h"
#include "RFID_command.h"
#include <stdio.h>
#include <Ds1302.h>
#include <SPI.h>
#include <SD.h>
#include <FS.h>
#include <string.h>
#include<stdbool.h>

Ds1302 rtc(15, 22, 21);

const static char *WeekDays[] = { "Monday", "Tuesday", "Wednesday", "Thursday",
		"Friday", "Saturday", "Sunday" };

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

//File myFile;
const int chipSelect = 5;
File myFile;

//ficheiro das tags
char filename_tags[] = "/taglist.CSV";
char filename_box_tag[] = "/boxlist.CSV";
//ficheiro das caixas

volatile int set_counts = 0;
const int button_read = 27;

void setup() {
	Serial2.begin(115200, SERIAL_8N1, 16, 17); //16.17
	Serial.begin(115200);
	//attachInterrupt(button_read, func_read_all, FALLING);
	rtc.init();
	RFID.Set_transmission_Power(2600);
	RFID.Set_the_Select_mode();
	RFID.Delay(100);
	RFID.Readcallback();
	RFID.clean_data();
	if (SD.begin(5))
		Serial.println("SD card is ready");

	if (!SD.begin(chipSelect))
		Serial.println("Card failed, or not present");
}
void loop() {

	while (Serial.available() == 0) {
	}

	char menuChoice = Serial.read();

	switch (menuChoice) {
	case '1':
		// temp sensor code goes here

		func_verificar_tags();
		func_get_time_date();
		break;

	case '2':
		// humidity sensor code goes here
		func_adicionar_nova_tagcaixa();
		break;

	case '3':
		func_adicionar_nova_tagcaixa_2();

		break;
	case '4':

		put_on_line("x3333");

		break;
	case '5':

		search_the_line("x1111", "64613");

		break;

	default:
		break;
	}

	delay(10000);
}
void func_verificar_tags() { //verifica se alguma tag lida corresponde a tags de uma caixa no cartao microsd
	int check = 0;
	String current_reading2[200];
	String current_reading[100];
	String tag_caixa;
	int count = 0;
	myFile = SD.open(filename_tags);

	if (myFile) {

		// read from the file until there's nothing else in it:
		Serial.print("\nlendo do cartao as tags que sao caixas ");
		while (myFile.available()) {

			current_reading2[count] = myFile.readStringUntil('\n,'); // para ler x

			Serial.print("\ncoluna ");
			Serial.print(count); // 00-99
			Serial.print(": ");
			Serial.print(current_reading2[count]);
			count++;

		}
		myFile.close();
	}

	cards = RFID.Multiple_polling_instructions(20);
	Serial.print(
			"\n --------------------------LEITURA------------------------ ");
	for (size_t i = 0; i < cards.len; i++) {
		current_reading[i] = cards.card[i]._EPC;
		current_reading[i] = current_reading[i].substring(19, 24);

		if (current_reading[i].length() == 5) {
			if (i == 0)
				set_counts++; // Apenas incrementa uma vez, basta haver tag

			Serial.print("\n tag ");
			Serial.print(i);
			Serial.print(" lida pelo leitor:");

			Serial.print(current_reading[i]); // 00-99
			Serial.print("\tRSSI:");
			Serial.print(cards.card[i]._RSSI); //Received Signal Strength Indicator
			if (i == cards.len) {

				Serial.print(
						"\n -------------------------- FIM DA LEITURA------------------------ ");
			}

		}

		for (int j = 0; j < count; j++) {
			if (current_reading[i] == current_reading2[j]) {
				Serial.print("\nTag associada a  uma caixa detetada : ");
				Serial.print(current_reading[i]);
				tag_caixa = current_reading[i];
				check = 1;
				RFID.clean_data();
			} else if (current_reading[i] != current_reading2[j]
					&& (j == count - 1) && (i == cards.len - 1) && check != 1) {
				Serial.print("\nTags detetadas nao sao caixas");
				RFID.clean_data();
			}

		}
	}

	RFID.clean_data();
	if (check == 1) {
		scan_tags_row(tag_caixa, current_reading);

	}
}

void scan_tags_row(String search_string, String current_reading_array[]) { // a partir de um ponteiro para uma determinada box tag que se encontra na primeira coluna, imprime o resto da linha com
	// os id das tags associadas

	String current_reading_1[30];
	String current_reading_2[30];
	String main_tag;
	String reading;
	myFile = SD.open(filename_box_tag);
	int count = 1;
	char *ptr;
	char char_array[5];
	int check = 0;
	int check2[20];
	int count2 = 0;
	main_tag = search_string + ",";
	main_tag.toCharArray(char_array, 8);

	if (myFile) {

		// read from the file until there's nothing else in it:
		//Serial.print("\n\ntags presentes no cartao associadas a caixa: ");
		//Serial.print(search_string);
		//Serial.print(":\n");

		if (myFile.find(char_array)) {
			myFile.position();
			while (myFile.available()) {
				reading = myFile.readStringUntil('\n');

				Serial.print(current_reading_2[count]);

				if (myFile.find(",")) {
					count++;
					break;
				}
			}
		}
		myFile.close();
		char column[reading.length()];

		Serial.print("\n\nTAGS que estao associadas à tag da caixa ");
		Serial.print(char_array);
		Serial.print(":\n");
		Serial.print(reading);

		Serial.print("\n\n-------------------------------------------");

		//Serial.print(reading.length());
		reading.toCharArray(column, reading.length());
		ptr = strtok(column, ",");
		while (ptr != NULL) {

			//Serial.print("\n tag");
			//Serial.print(count2);
			//Serial.print(current_reading_2[count2]);
			current_reading_2[count2] = ptr;

			count2++;
			ptr = strtok(NULL, ",");
		}

		for (int i = 0; i < count2; i++) {
			check2[i] = 0;

			for (int j = 0; j < cards.len; j++) {
				if (current_reading_2[i] == current_reading_array[j]) {
					check2[i] = 1;
				}
			}

		}
		for (int y = 0; y < count2; y++) {
			if (check2[y] != 1) {
				check = 1;
				Serial.print("\n\n A tag");
				Serial.print(current_reading_2[y]);
				Serial.print("esta em falta");
			}

		}
		if (check == 0) {

			Serial.print("\n A caixa esta completa");

		} else {

			Serial.print("\n A caixa nao esta completa");

		}

	}
}

void func_get_time_date() {
	static uint8_t last_second = 0;
	Ds1302::DateTime now;
	rtc.getDateTime(&now);

	if (last_second != now.second) {
		Serial.print("\n\n\n\nData: ");
		Serial.print(now.day);     // 01-31
		Serial.print(' ');
		Serial.print(WeekDays[now.dow - 1]);
		Serial.print(now.year);    // 00-99
		Serial.print('-');

		Serial.print(now.month);   // 01-12
		Serial.print('-');
		Serial.print("20");
		Serial.print(now.year);       // 01-31
		Serial.print(' ');
		Serial.print("\n Hora: ");
		Serial.print(now.hour);    // 00-23
		Serial.print(':');

		Serial.print(now.minute);  // 00-59

	}

}
String func_adicionar_novas_tags() {
	String current_reading;
	Serial.print(
			"\n --------------------------INSERIR NOVA TAG------------------------ ");

	card = RFID.A_single_poll_of_instructions();

	current_reading = card._EPC;
	current_reading = current_reading.substring(19, 24);
	RFID.clean_data();
	if (current_reading.length() != 0) {

		Serial.print("\n tag lida: ");

		Serial.print(current_reading); // 00-99

		Serial.print(
				"\n -------------------------- FIM DA LEITURA------------------------ ");
	}

	return current_reading;

}
void func_adicionar_nova_tagcaixa() {
	int check = 0;
	int pos = 0;
	String current_reading2[100];
	String last_reading;
	myFile = SD.open(filename_tags);
	int count = 0;
	String current_reading = func_adicionar_novas_tags();
	if (current_reading.length() == 5) {
		if (myFile) {

			// read from the file until there's nothing else in it:
			Serial.print("\nlendo do cartao as tags que sao caixas ");
			while (myFile.available()) {

				current_reading2[count] = myFile.readStringUntil(','); // para ler x

				if (count > 0 && current_reading2[count].length() == 5) {
					pos = myFile.position();
					last_reading = current_reading2[count];
				}
				Serial.print("\ncoluna ");
				Serial.print(count); // 00-99
				Serial.print(": ");
				Serial.print(current_reading2[count]);
				count++;

				Serial.print("\t pos:");
				Serial.print(pos);
			}
			myFile.close();
		}

		for (int j = 0; j < count; j++) {
			if (current_reading == current_reading2[j]) {
				Serial.print(
						"\nTag ja inserida no sistema e corresponde a uma caixa ");
				check = 1;
			} else if (current_reading != current_reading2[j]
					&& (j == count - 1) && check != 1) {
				check = 1;
				myFile = SD.open(filename_tags, FILE_WRITE);
				if (myFile) {
					myFile.seek(pos);
					myFile.print(',');
					myFile.print(current_reading);
					Serial.print("\nA tag lida:");
					Serial.print(current_reading);
					Serial.print("\nfoi registada como tag de caixa!");
					myFile.close();

				}

			}

		}
	}

}

void func_adicionar_nova_tagcaixa_2() {
	int check = 0;
	int pos = 0;
	String current_reading2[100];
	String last_reading;
	myFile = SD.open(filename_box_tag);
	int count = 0;
	String current_reading = func_adicionar_novas_tags();
	if (current_reading.length() == 5) {
		if (myFile) {

			// read from the file until there's nothing else in it:
			Serial.print(
					"\nProcurando no ficheiro de tags a caixa correspondente a tag lida");
			while (myFile.available()) {
				if (count > 0)
					myFile.readStringUntil('\n'); // para ler x

				current_reading2[count] = myFile.readStringUntil(',');
				pos = myFile.position();
				if (count > 0 && current_reading2[count].length() == 5) {
					pos = myFile.position();
					last_reading = current_reading2[count];

				}
				Serial.print(
						"\ntags lidas----------------------------------:  ");
				Serial.print(count); // 00-99
				Serial.print(": ");
				Serial.print(current_reading2[count]);
				count++;

				Serial.print("\t pos:");
				Serial.print(pos);
			}
			myFile.close();
		}

		for (int j = 0; j < count; j++) {
			if (current_reading == current_reading2[j]) {
				Serial.print(
						"\nTag ja inserida no sistema e corresponde a uma caixa ");
				check = 1;
			} else if (current_reading != current_reading2[j]
					&& (j == count - 1) && check != 1) {
				check = 1;
				myFile = SD.open(filename_box_tag, FILE_WRITE);
				if (myFile) {
					myFile.seek(pos);
					myFile.print(',');
					myFile.print('\n');
					myFile.print(current_reading);
					Serial.print("\nA tag lida:");
					Serial.print(current_reading);
					Serial.print("\nfoi registada como tag de caixa!");
					myFile.close();

				}

			}
			myFile.close();
		}
	}

}

int search_the_line(String search_string, String add_string) { //encontrar uma tag a partir de uma tag main

	myFile = SD.open(filename_box_tag);
	int count = 1;

	char char_array[6];

	char char_array_2[6];
	char last_reading_char[6];
	int check = 0;
	int count2 = 0;
	search_string.toCharArray(char_array, 6);

	add_string.toCharArray(char_array_2, 6);
	if (myFile) {

		// read from the file until there's nothing else in it:
		//Serial.print("\n\ntags presentes no cartao associadas a caixa: ");
		//Serial.print(":\n");

		while (myFile.available()) {

			if (myFile.find(char_array)) {
				myFile.position();
				Serial.print("\n\n\n foi encontrada a tag main");
				Serial.print(char_array);
				myFile.readStringUntil('\n');

			}
			if (myFile.find(char_array_2)) {
				Serial.print("\n\nFoi encontrada a tag associada a tag main");
				Serial.print(char_array_2);
				myFile.close();
				return 1;
			}
		}
	}
	myFile.close();

	Serial.print("\n\nNAO Foi encontrada a tag dessa posicao");
	return 0;
}

void put_on_line(String tag_main) {	// para uma tag main adquirir as tags que lhe estao associadas
	int check = 0;
	int pos = 0;
	int count = 0;
	int count2 = 0;
	char *ptr;
	char char_array[7];
	char last_reading_char[6];
	String current_reading_2[100];
	String reading;
	String last_reading;

	String current_reading = func_adicionar_novas_tags();
	tag_main = tag_main + ",";
	tag_main.toCharArray(char_array, 6);
	if (current_reading.length() == 5) {
		Serial.print("\nkkkkkkkkkkkkk");
		Serial.print(char_array);
		Serial.print("\nkkkkkkkkkkkkk");
		myFile = SD.open(filename_box_tag);
		if (myFile) {
			if (myFile.find(char_array)) {
				myFile.position();
				while (myFile.available()) {
					reading = myFile.readStringUntil('\n');

					Serial.print(current_reading_2[count]);

					if (myFile.find(",")) {
						count++;
						break;
					}
				}
			}
			myFile.close();
		}
		/***************************************************************************************************/

		char column[reading.length()];

		Serial.print("\n\nLinha completa da tag:");
		Serial.print("\n");
		Serial.print(char_array);
		Serial.print(":\n");
		Serial.print(reading);

		Serial.print("\n\n-------------------------------------------");

		reading.toCharArray(column, reading.length());
		ptr = strtok(column, ",");
		while (ptr != NULL) {
			Serial.print("debug");
			current_reading_2[count2] = ptr;
			last_reading = ptr;
			count2++;
			ptr = strtok(NULL, ",");
		}

		/***************************************************************************************************/

		last_reading.toCharArray(last_reading_char, 6);
		Serial.print("\nCHAR ARRAY NAME----------------------");
		Serial.print(last_reading_char);
		myFile = SD.open(filename_box_tag, FILE_WRITE);
		if (myFile) {
			while (myFile.available()) {
			if (myFile.find(last_reading_char)) {

				myFile.position();

				myFile.print(',');
				myFile.print("teste");
				Serial.print("\nA ultima tag lida:");
				Serial.print(last_reading_char);
				Serial.print("\n");
				myFile.close();
				}
			}
			myFile.close();
		}

	}

}

void eliminar_tag(String current_reading) {
	String current_reading2[100];
	myFile = SD.open(filename_tags);
	int count = 0;
	if (myFile) {

// read from the file until there's nothing else in it:
		Serial.print("\nlendo do cartao as tags que sao caixas ");
		while (myFile.available()) {

			current_reading2[count] = myFile.readStringUntil('\n,'); // para ler x

			if (current_reading2[count].length() != 5 && count > 0)
				break;

			Serial.print("\ncoluna ");
			Serial.print(count); // 00-99
			Serial.print(": ");
			Serial.print(current_reading2[count]);
			count++;

		}
		myFile.close();
	}

}
