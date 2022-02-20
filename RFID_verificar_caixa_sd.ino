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

//File myFile;
const int chipSelect = 5;
File myFile;

//ficheiro das tags
char filename_tags[] = "/taglist.CSV";
char filename_box_tag[] = "/boxlist.CSV";
//ficheiro das caixas

typedef struct check_box {
	String tag_main;
	String tag_count[20];

} check_box_tags;

check_box_tags checkbox_set[100];
check_box_tags current_reading;
check_box_tags current_reading2;
volatile int set_counts = 0;

const int button_read = 27;

void setup() {

	Serial.begin(115200);
	//Print main menu

	checkbox_set[0].tag_main = "928ff";
	checkbox_set[0].tag_count[0] = "928ff"; //a posicao 0 de cada tag_count será a tag main
	checkbox_set[0].tag_count[3] = "64795";
	checkbox_set[0].tag_count[5] = "64805";

	//checkbox_set[0].tag_count[5] = "e2003411b802011056264666";
	checkbox_set[1].tag_main = "e2003411b802011056264798";

	Serial2.begin(115200, SERIAL_8N1, 16, 17); //16.17

	//attachInterrupt(button_read, func_read_all, FALLING);
	RFID.Set_transmission_Power(2600);
	RFID.Set_the_Select_mode();
	RFID.Delay(100);
	RFID.Readcallback();
	RFID.clean_data();
	if (SD.begin(5))
		Serial.println("SD card is ready");

	if (!SD.begin(chipSelect))
		Serial.println("Card failed, or not present");
	// don't do anything more:

}

void loop() {
	//card = RFID.A_single_poll_of_instructions();
	int pos;
	//int check_box = func_read_all(&pos);
	if (set_counts == 20)
		set_counts = 1;

	//if (check_box == 1)
	//check_box = caixa_completa(check_box, pos);

	scan_tags_row();
	//File_read();

	Serial.print("\nNumber of boxes registered: ");
	Serial.print(set_counts);

	/*Serial.print(current_reading2.tag_count[0]);
	 Serial.print(current_reading2.tag_count[1]);
	 Serial.print(current_reading2.tag_count[1]);
	 Serial.print(current_reading2.tag_count[2]);*/
	delay(5000);

}
void func_verificar_caixa(int *pos) {	//posicao da linha
	int check = 0;
	*pos = 0;
	int count = 0;
	myFile = SD.open(filename_tags);

	if (myFile) {

		// read from the file until there's nothing else in it:

		while (myFile.available()) {

			//	current_reading2.tag_count[count] = myFile.readStringUntil(','); // para ler x

			current_reading2.tag_count[count] = myFile.readStringUntil(','); // para ler x
			//if(myFile.find(myFile.position())){
			//	Serial.print("testeeeeeeeeeeeee");

			//Serial.print(myFile.position());
			//	}
			Serial.print("\nlendo do cartao sd a tag da coluna ");

			Serial.print(count); // 00-99
			Serial.print(": ");
			Serial.print(current_reading2.tag_count[count]);
			//	Serial.print(current_reading2.tag_count[count]);
			count++;

		}
		myFile.close();
	}

	cards = RFID.Multiple_polling_instructions(20);

	for (size_t i = 0; i < cards.len; i++) {
		current_reading.tag_count[i] = cards.card[i]._EPC;
		current_reading.tag_count[i] = current_reading.tag_count[i].substring(
				19, 24);

		if (current_reading.tag_count[i].length() == 5) {
			if (i == 0)
				set_counts++; // Apenas incrementa uma vez, basta haver tag

			Serial.print("\n tag");
			Serial.print(i);
			Serial.print(":");

			Serial.print(current_reading.tag_count[i]); // 00-99

			for (int j = 0; j < count; j++) {
				if (current_reading.tag_count[i]
						== current_reading2.tag_count[j]) {

					Serial.print("\n A tag");
					Serial.print(current_reading.tag_count[j]);
					Serial.print("e uma caixa");
				}
			}
			//	Serial.print(current_reading2.tag_count[count]);

		}

	}

	RFID.clean_data();
}

void func_verificar_tags(int *pos) {	//verifica se alguma tag lida corresponde a tags de uma caixa no cartao microsd
	int check = 0;
	*pos = 0;
	int count = 0;
	myFile = SD.open(filename_tags);

	if (myFile) {

		// read from the file until there's nothing else in it:

		while (myFile.available()) {

			//	current_reading2.tag_count[count] = myFile.readStringUntil(','); // para ler x

			current_reading2.tag_count[count] = myFile.readStringUntil(','); // para ler x
			//if(myFile.find(myFile.position())){
			//	Serial.print("testeeeeeeeeeeeee");

			//Serial.print(myFile.position());
			//	}
			Serial.print("\nlendo do cartao sd a tag da coluna ");

			Serial.print(count); // 00-99
			Serial.print(": ");
			Serial.print(current_reading2.tag_count[count]);
			//	Serial.print(current_reading2.tag_count[count]);
			count++;

		}
		myFile.close();
	}

	cards = RFID.Multiple_polling_instructions(20);

	for (size_t i = 0; i < cards.len; i++) {
		current_reading.tag_count[i] = cards.card[i]._EPC;
		current_reading.tag_count[i] = current_reading.tag_count[i].substring(
				19, 24);

		if (current_reading.tag_count[i].length() == 5) {
			if (i == 0)
				set_counts++; // Apenas incrementa uma vez, basta haver tag

			Serial.print("\n tag");
			Serial.print(i);
			Serial.print(":");

			Serial.print(current_reading.tag_count[i]); // 00-99

			for (int j = 0; j < count; j++) {
				if (current_reading.tag_count[i]
						== current_reading2.tag_count[j]) {

					Serial.print("\n A tag");
					Serial.print(current_reading.tag_count[j]);
					Serial.print("e uma caixa");



				}
			}
			//	Serial.print(current_reading2.tag_count[count]);

		}

	}

	RFID.clean_data();
}

void scan_tags_row() { // a partir de um ponteiro para uma determinada box tag que se encontra na primeira coluna, imprime o resto da linha com
						// os id das tags associadas

	String current_reading_1[30];
	myFile = SD.open(filename_box_tag);
	int count = 0;
	char char_array[5];

	if (myFile) {

		// read from the file until there's nothing else in it:

		while (myFile.available()) {

			if (myFile.find("")) {
				myFile.position();
				current_reading_1[count] = myFile.readStringUntil(',\n');
				Serial.print("testeeee:");
				Serial.print(current_reading_1[count]);
				count++;
			}

		}
		myFile.close();

	}




}
int func_read_all(int *pos) { //verifica se alguma tag lida corresponde a tags de uma caixa declaradas apenas na main
	int check = 0;
	*pos = 0;
	cards = RFID.Multiple_polling_instructions(20);

	for (size_t i = 0; i < cards.len; i++) {
		current_reading.tag_count[i] = cards.card[i]._EPC;
		current_reading.tag_count[i] = current_reading.tag_count[i].substring(
				19, 24);
		Serial.print("\n");
		Serial.print(current_reading.tag_count[i]);
		if (current_reading.tag_count[i].length() == 5) {
			if (i == 0)
				set_counts++; // Apenas incrementa uma vez, basta haver tag

			Serial.print("\t tag");
			Serial.print(i);
			Serial.print(":");

			Serial.print(current_reading.tag_count[i]); // 00-99

			for (int j = 0; j < set_counts; j++) {
				if (current_reading.tag_count[i] == checkbox_set[j].tag_main) {
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
				} else if (current_reading.tag_count[i]
						!= checkbox_set[j].tag_main && (j == set_counts - 1)
						&& check == 0 && i == (cards.len - 1)) {
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
int caixa_completa(int check, int pos) {//verifica se para uma determinada tag de uma caixa as tags lidas estao completas ou nao estao
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

	caixa_completa_info(check, pos);

	return check;

}
void caixa_completa_info(int check, int pos) {

	if (check == 0) {
		Serial.print("\n\nA caixa com a tag main ");
		Serial.print(pos);
		Serial.print("nao esta completa");
	} else if (check == 1) {
		Serial.print("\n\nA caixa com a tag main ");
		Serial.print(pos);
		Serial.print("esta completa");
	}

}
void File_read() {

	myFile = SD.open(filename_tags);
	if (myFile) {

// read from the file until there's nothing else in it:
		while (myFile.available()) {
			int i = 0;
			current_reading2.tag_count[i] = Serial.write(myFile.read());
			i++;

		}

		myFile.close();

	} else {
// if the file didn't open, print an error:
		Serial.println("error opening file");
	}
}
