#include "Arduino.h"
#include "RFID_command.h"
#include <stdio.h>

#include <SPI.h>
#include <SD.h>
#include <FS.h>
#include <string.h>
#include<stdbool.h>
#include "RTClib.h"
#include "Arduino.h"
#include <WiFi.h>
#include <PubSubClient.h>
#include <HTTPClient.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Keypad.h>
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 32 // OLED display height, in pixels
hw_timer_t *timer = NULL;
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
Adafruit_SSD1306 display = Adafruit_SSD1306(128, 32, &Wire);
File myFile;
RTC_DS3231 rtc;
const int chipSelect = 5;
unsigned long lastTime = 0;
unsigned long timerDelay = 5000;

const char *ssid = "Vodafone-FD27C0";
const char *password = "vodafone10";
//const char *ssid = "labs";
//const char *password = "robot1cA!ESTG";
/*const char *server_tag_main = "http://10.20.228.241:1880/tag_main";
 const char *server_present_tags = "http://10.20.228.241:1880/present_tags";
 const char *server_statebox = "http://10.20.228.241:1880/statebox";*/

const char *server_tag_main = "http://192.168.1.72:1880/tag_main";
const char *server_present_tags = "http://192.168.1.72:1880/present_tags";
const char *server_statebox = "http://192.168.1.72:1880/statebox";
String server_menu_select = "http://192.168.1.72:1880/buton";
/*
 const char *server_tag_main = "http://10.20.228.194:1880/tag_main";
 const char *server_present_tags = "http://10.20.228.194:1880/present_tags";
 const char *server_statebox = "http://10.20.228.194:1880/statebox";
 String server_menu_select = "http://10.20.228.194:1880/buton";
 */
/*const char *server_tag_main = "http://10.20.228.130:1880/tag_main";
 const char *server_present_tags = "http://10.20.228.130:1880/present_tags";
 const char *server_statebox = "http://10.20.228.130:1880/statebox";
 String server_menu_select = "http://10.20.228.130:1880/buton";*/

//keyboard
const byte ROWS = 4; //four rows
const byte COLS = 4; //three columns
char keys[ROWS][COLS] = { { '1', '2', '3', '4' }, { '5', '6', '7', '8' }, { '9',
		'10', '11', '12' }, { '13', '14', '15', '16' }, };
byte rowPins[ROWS] = { 4, 0, 15, 32 }; //connect to the row pinouts of the kpd
byte colPins[COLS] = { 12, 14, 27, 26 }; //connect to the column pinouts of the kpd

Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

/* LED pin */
int led = 25;

volatile byte state_led = LOW;
// Keep this API Key value to be compatible with the PHP code provided in the project page.
// If you change the apiKeyValue value, the PHP file /post-esp-data.php also needs to have the same key

WiFiClient Wificlient;
PubSubClient client(Wificlient);
HTTPClient http;

//ficheiro das tags
char filename_tags[] = "/taglist.CSV";
char filename_box_tag[] = "/boxlist.CSV";
char filename_registos[] = "/registos.CSV";
char filename_raw_registos[] = "/todos_registos.CSV";
String file1_tags[30];
String last_file1_tag;
int file_counter_1 = 0;
int lastpos_file1 = 0;
String file2_tags[30];
int file_counter_2 = 0;
int lastpos_file2 = 0;
String last_column;
String commands_str = { "\n1-Verificao de tags na caixa"
		"\n2-Inserir uma nova tag como tag de caixa"
		"\n3-inserir uma nova tag a uma caixa existente"
		"\n4-Opcao programador de inserir dados no cartao" };

int count_non_registered = 0;
int count_registered = 0;
String global = "";
String all_present_tags = "";

void setup() {
	Serial2.begin(115200, SERIAL_8N1, 16, 17); //16.17
	Serial.begin(115200);
	//attachInterrupt(button_read, func_read_all, FALLING);

	RFID.Set_transmission_Power(2600);
	RFID.Set_the_Select_mode();
	RFID.Delay(100);
	RFID.Readcallback();
	RFID.clean_data();
	display.begin(SSD1306_SWITCHCAPVCC, 0x3C); // Address 0x3C for 128x32
	display.clearDisplay();
	display.setTextSize(2);
	display.setTextColor(SSD1306_WHITE);
	display.setCursor(0, 15);
	display.println("SMART LAB ");
	display.setCursor(0, 0);
	display.println("CHECKBOX");
	display.display();

	if (SD.begin(5))
		Serial.println("SD card is ready");

	if (!SD.begin(chipSelect))
		Serial.println("Card failed, or not present");

	pinMode(25, OUTPUT);
	//attachInterrupt(25, write_sd, FALLING);
	rtc.begin();

	if (!rtc.begin()) {
		Serial.println("Couldn't find RTC");
		Serial.flush();
		while (1)
			delay(10);
	}

	WiFi.begin(ssid, password);
	Serial.println("Connecting to wifi...");
	while (WiFi.status() != WL_CONNECTED) {
		delay(500);
		Serial.print("still not connected to wifi");
	}

	Serial.print("Connected to WiFi network with IP Address: ");

	// Your Domain name with URL path or IP address with path
	// http.begin(Wificlient, serverName);

	ler_file_1();
	ler_file_2();

	pinMode(led, OUTPUT);

	/* Use 1st timer of 4 */
	/* 1 tick take 1/(80MHZ/80) = 1us so we set divider 80 and count up */
	timer = timerBegin(0, 80, true);

	/* Attach onTimer function to our timer */
	timerAttachInterrupt(timer, &onTimer, true);

	/* Set alarm to call onTimer function every second 1 tick is 1us
	 => 1 second is 1000000us */
	/* Repeat the alarm (third parameter) */
	timerAlarmWrite(timer, 1000000, true);

	/* Start an alarm */

	delay(5000);
}

void IRAM_ATTR onTimer() {
	state_led = !state_led;
	digitalWrite(led, state_led);
}

// The loop function is called in an endless loop
void loop() {

	Menu_main();
	RFID.Delay(150);
	delay(5000);

}
void Menu_main() {

	int selection = menu_select_all();
	Serial.print(commands_str);

	display.clearDisplay();
	display.setTextSize(0.5);
	display.setTextColor(SSD1306_WHITE);

	display.setCursor(0, 0);
	display.println("1-verificacao");
	display.setCursor(0, 7);
	display.print("2-Adicionar tag caixa");
	display.setCursor(0, 14);
	display.print("3-Adicionar tag nova");
	display.setCursor(0, 21);
	display.print("4-Remover tags");
	display.display();

	switch (selection) {
	case 1:
		func_verificar_tags();
		RFID.Delay(150);
		break;
	case 2:
		InserirBox_file_1();
		break;
	case 3:
		Inserir_tag_nova_();
		break;
	case 4:
		write_sd();
		break;
	case 5:
		Serial.print(ler_end_of_file_2());
		break;

	default:

		break;

	}
}

int User_menu_input() {

	if (!Serial.available()) {
		return 0;
	}
	if (Serial.available()) {
		int menu_select = Serial.parseInt();
		if (menu_select != 1 && menu_select != 2 && menu_select != 3
				&& menu_select != 4 && menu_select != 5 && menu_select != 6
				&& menu_select != 7) {
			Serial.println("opcao incorreta");
			return 0;
		} else {
			Serial.println("Menu selected: ");
			Serial.println(menu_select);
			return menu_select;
		}
	}
}

void func_verificar_tags() { //verifica se alguma tag lida corresponde a tags de uma caixa no cartao microsd
	RFID.clean_data();
	int check = 0;
	String current_reading2[20];
	String current_reading[20];
	String tag_caixa;
	int count = 0;
	global = "";
	all_present_tags = "";
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

	for (size_t i = 0; i < cards.len; i++) {

		current_reading[i] = cards.card[i]._EPC;

	}
	size_t size = cards.len;
	RFID.clean_data();
	Serial.print(
			"\n -------------------------- LEITURA CONCLUIDA------------------------ ");
	for (size_t i = 0; i < size; i++) {

		current_reading[i] = current_reading[i].substring(19, 24);

		if (current_reading[i].length() == 5) {

			Serial.print("\n tag ");
			Serial.print(i);
			Serial.print(" lida pelo leitor:");

			Serial.print(current_reading[i]); // 00-99
			//Serial.print("\tRSSI:");
			//Serial.print(cards.card[i]._RSSI); //Received Signal Strength Indicator

		}

		for (int j = 0; j < file_counter_1; j++) {

			if (current_reading[i] == file2_tags[j]
					|| current_reading[i] == file2_tags[file_counter_1]) {

				tag_caixa = current_reading[i];
				check = 1;

				//RFID.clean_data();
			} else if (current_reading[i] != current_reading2[j]
					&& (j == file_counter_1 - 1) && (i == cards.len - 1)
					&& check != 1) {
				Serial.print("\nTags detetadas nao sao caixas");

				display.clearDisplay();
				display.setTextSize(0.5);
				display.setTextColor(SSD1306_WHITE);
				display.setCursor(0, 0);
				display.print("tags detetadas");
				display.setCursor(0, 7);
				display.print("nao correspondem");
				display.setCursor(0, 14);
				display.print("a tags de");
				display.setCursor(0, 21);
				display.print("caixa");
				display.display();

				delay(3000);
				display.clearDisplay();

				//RFID.clean_data();
			}

		}

	}

	if (check == 1) {
		scan_tags_row(tag_caixa, current_reading);

	}
}

void scan_tags_row(String search_string, String current_reading_array[]) { // a partir de um ponteiro para uma determinada box tag que se encontra na primeira coluna, imprime o resto da linha com
// os id das tags associadas

	char *ptr;
	char char_array[5];
	int check = 0;
	int check2[20];
	int check3[20];
	int count2 = 0;
	int count3 = 0;
	int count = 1;

	String current_reading_2[30];
	String missing_tags[30];
	String present_tags[30];
	String main_tag;
	String reading;
	String current;
	String state;

	int missing_tag_counter = 0;
	int present_tag_counter = 0;

	myFile = SD.open(filename_box_tag);
	main_tag = search_string + ",";
	main_tag.toCharArray(char_array, 8);

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
		char column[reading.length()];

		Serial.print(
				"\n\nTAGS no cartao que estao associadas a tag da caixa lida: ");
		Serial.print(search_string);
		Serial.print(":\n");
		Serial.print(reading);
		Serial.print("\n\n-------------------------------------------");

		reading.toCharArray(column, reading.length());
		ptr = strtok(column, ",");
		while (ptr != NULL) {
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

				missing_tags[missing_tag_counter] = current_reading_2[y];
				missing_tag_counter++;
				Serial.print("\n tag:");
				Serial.print(current_reading_2[y]);
				Serial.print("nao presente na leitura");

			} else {
				Serial.print("\n tag:");
				Serial.print(current_reading_2[y]);
				Serial.print("presente na leitura de caixa");
				if (current_reading_2[y] != search_string)
					present_tags[present_tag_counter] = current_reading_2[y];
				present_tag_counter++;
				all_present_tags = all_present_tags + ","
						+ current_reading_2[y];

			}
		}
		global = all_present_tags;
		if (check == 0) {

			Serial.print("\nESTADO:CAIXA COMPLETA");
			state = "completa";
			timerAlarmEnable(timer);

		} else {
			state = "incompleta";
			timerAlarmDisable(timer);

			Serial.print("\nESTADO:CAIXA INCOMPLETA");
			Serial.print("\n tags em falta:");
			for (int k = 0; k < missing_tag_counter; k++) {
				Serial.print(missing_tags[k]);
				Serial.print(" ");

			}
			Serial.print(" \n\n------------------------");
		}

		float percentagem = float(present_tag_counter / count2) * 100;
		Serial.print("NUMERO DE TAGS PRESENTES:");
		Serial.println(present_tag_counter);
		Serial.print("NUMERO DE TAGS totais:");
		Serial.println(count2);

		display.clearDisplay();
		display.setTextSize(1);
		display.setTextColor(SSD1306_WHITE);
		display.setCursor(0, 0);
		display.print("Caixa principal:");
		display.print(search_string);
		display.setCursor(0, 10);
		display.print("estado:");
		display.print(state);
		display.setCursor(0, 20);
		display.print("tags completas:");
		display.print(percentagem);
		display.print("%");
		display.display();

		registo(search_string, present_tags, present_tag_counter, state);
		raw_registo(current_reading_array, search_string, state);

		//myFile = SD.open(filename_box_tag, FILE_READ);
		global = all_present_tags;
		Serial.print(all_present_tags);

		for (int j = 0; j < cards.len; j++) {

			check3[j] = 0;
			for (int i = 0; i < count2; i++) {

				if (current_reading_array[j] == current_reading_2[i]) {
					check3[j] = 1;
				}

			}
			if (check3[j] == 0) {

				existing_tags(current_reading_array[j]);

			}

		}

	}
	if (WiFi.status() == WL_CONNECTED) {

		http.begin(server_tag_main);
		//http.addHeader("tag_principal", "text/plain");
		http.POST(search_string);
		http.end();

		http.begin(server_statebox);
		//	http.addHeader("estado", "text/plain");
		http.POST(state);
		http.end();

		http.begin(server_present_tags);
		//http.addHeader("todas_tags", "text/plain");
		http.POST(global);

		http.end();

	}
}

void existing_tags(String search_string) {
	int count = 0;

	String row_reading[30];

	String non_registered[10];
	String registered[10];
	String main_tags[5];
	String current_reading[30];

	String tag_main;
	char *ptr;
	char char_array[5];
	int check = 0;
	int count2 = 0;
	int check_main = 0;
	Serial.print("\n");

	myFile = SD.open(filename_box_tag, FILE_READ);

	if (myFile) {

		while (myFile.available()) {
			row_reading[count] = myFile.readStringUntil('\n');
			count++;
		}
		myFile.close();

		for (int counter = 0; counter < count; counter++) {
			count2 = 0;
			char column[row_reading[counter].length()];

			row_reading[counter].toCharArray(column,
					row_reading[counter].length());
			ptr = strtok(column, ",");
			while (ptr != NULL) {
				current_reading[count2] = ptr;
				count2++;
				ptr = strtok(NULL, ",");
			}

			for (int i = 0; i < count2; i++) {

				if (search_string == current_reading[i]) {
					check = 1;
					tag_main = current_reading[0];

				}
			}

		}

		if (search_string != tag_main) {

			if (check == 1) {

				Serial.print("\nA tag:");
				Serial.print(search_string);
				Serial.print(" esta registada no sistema e ");
				Serial.print("corresponde a caixa: ");
				Serial.print(tag_main);
				Serial.print("\n-------------------------------------------\n");

				global = global + "," + search_string;
			} else {

				Serial.print("\A tag: ");
				Serial.print(search_string);
				Serial.print("nao esta registada no sistema:  ");

				Serial.print("\n-------------------------------------------\n");

			}
		} else {

			Serial.print("A tag: ");
			Serial.print(search_string);
			Serial.print(" e uma tag associada a caixa");
			Serial.print("\n------------------------------------------\n");
		}
	}
}
void raw_registo(String present_tags[], String main, String state) {
	DateTime now = rtc.now();

	String last_reading;
	int file_counter = 0;
	int pos_reg = 0;
	myFile = SD.open(filename_raw_registos);

	if (myFile) {

		// read from the file until there's nothing else in it:
		//Serial.print("\nlendo do ficheiro1  as tags que sao caixas ");
		while (myFile.available()) {

			last_reading = myFile.readStringUntil(','); // para ler x

			if (last_reading.length() != 0) {
				pos_reg = myFile.position();

			} else
				pos_reg = 0;

			file_counter++;

		}
		myFile.close();
		myFile = SD.open(filename_raw_registos, FILE_WRITE);
		if (myFile) {
			if (myFile.seek(pos_reg)) {
				myFile.println("ESP32-modulo1");
				myFile.print(',');
				myFile.print(now.hour());
				myFile.print(":");
				myFile.print(now.minute());
				myFile.print(":");
				myFile.print(now.second());

				myFile.print(',');
				myFile.print(now.day());
				myFile.print("/");

				myFile.print(now.month());
				myFile.print("/");

				myFile.print(now.year());
				myFile.print(',');
				myFile.print("tags presentes:");
				for (int i; i < cards.len; i++) {
					myFile.print(present_tags[i]);
					myFile.print(',');

				}
				myFile.print(state);
				myFile.print('\n');

			}
			myFile.close();

		}
	}
}
void registo(String main, String present_tags[], int count, String state) {
	DateTime now = rtc.now();
	String file_registo[30];
	String last_reading;
	int file_counter = 0;
	int pos_reg;
	myFile = SD.open(filename_registos);

	if (myFile) {

		while (myFile.available()) {

			last_reading = myFile.readStringUntil(','); // para ler x

			if (last_reading.length() != 0) {
				pos_reg = myFile.position();

			} else
				pos_reg = 0;

			file_counter++;

		}
		myFile.close();

		myFile = SD.open(filename_registos, FILE_WRITE);

		if (myFile) {
			if (myFile.seek(pos_reg)) {
				//myFile.println('\n');
				myFile.println("ESP32-modulo1");
				myFile.print(',');
				myFile.print(now.hour());
				myFile.print(":");
				myFile.print(now.minute());
				myFile.print(":");
				myFile.print(now.second());

				myFile.print(',');
				myFile.print(now.day());
				myFile.print("/");

				myFile.print(now.month());
				myFile.print("/");

				myFile.print(now.year());
				myFile.print(',');

				myFile.print("Caixa-" + main);
				myFile.print(',');
				myFile.print("tags presentes:");
				for (int i; i < count; i++) {
					myFile.print(present_tags[i]);
					myFile.print(',');
				}
				myFile.print(state);
				myFile.close();

			}
		}

	}
}

String func_adicionar_novas_tags() {
	String current_reading;
	card = RFID.A_single_poll_of_instructions();
	current_reading = card._EPC;
	current_reading = current_reading.substring(19, 24);
	if (current_reading.length() != 0) {
		Serial.print(
				"\n --------------------------TAG LIDA------------------------ ");
		Serial.print(current_reading); // 00-99
		Serial.print(
				"\n -------------------------- FIM DA LEITURA------------------------ ");
		RFID.clean_data();
		return current_reading;
	}
	Serial.print(
			"\n -------------------------- TAG NAO LIDA------------------------ ");
	RFID.clean_data();
}

void ler_file_1() {

	myFile = SD.open(filename_tags);

	if (myFile) {

		// read from the file until there's nothing else in it:
		//Serial.print("\nlendo do ficheiro1  as tags que sao caixas ");
		while (myFile.available()) {

			file1_tags[file_counter_1] = myFile.readStringUntil(','); // para ler x

			if (file_counter_1 > 0
					&& file1_tags[file_counter_1].length() == 5) {

				lastpos_file1 = myFile.position();
				last_file1_tag = file1_tags[file_counter_1];
			}

			file_counter_1++;

		}
		myFile.close();
		Serial.print("\nlendo do cartao as tags que sao caixas");

		Serial.print("\nultima tag lida:");
		Serial.print(last_file1_tag);

		imprimir_file1();

	}

}
void update_file_1() {

	file_counter_1 = 0;
	myFile = SD.open(filename_tags);
	if (myFile) {

		// read from the file until there's nothing else in it:
		//Serial.print("\nlendo do ficheiro1  as tags que sao caixas ");
		while (myFile.available()) {

			file1_tags[file_counter_1] = myFile.readStringUntil(','); // para ler x

			if (file1_tags[file_counter_1].length() == 5) {

				lastpos_file1 = myFile.position();
			}

			file_counter_1++;

		}
		myFile.close();

	}

}
void InserirBox_file_1() {
	String current_reading = func_adicionar_novas_tags();
	char char_current_reading[6];
	current_reading.toCharArray(char_current_reading, 6);
	int check = 0;
	int check_componente = 0;
	if (current_reading.length() != 0) {
		myFile = SD.open(filename_box_tag);
		if (myFile) {
			if (myFile.find(char_current_reading))
				check_componente = 1;
			myFile.close();
		}

		for (int j = 0; j < file_counter_1; j++) {
			if (current_reading == file1_tags[j] && check != 1) {
				Serial.print(
						"\nTag ja inserida no sistema e corresponde a uma caixa ");
				check = 1;

				display.clearDisplay();
				display.setTextSize(0.5);
				display.setTextColor(SSD1306_WHITE);
				display.setCursor(0, 0);
				display.println("Tag lida:" + current_reading);

				display.setCursor(0, 7);
				display.print(" ja se encontra");

				display.setCursor(0, 14);
				display.print("registada no sistema");
				display.setCursor(0, 21);
				display.print("operacao cancelada!");
				display.display();

				delay(2000);}
				else if (check_componente == 1 && check != 1&&(j == file_counter_1 - 1)) {

					Serial.print(
							"\nTag ja inserida no sistema e corresponde a um componente! ");
					check = 1;

					display.clearDisplay();
					display.setTextSize(0.5);
					display.setTextColor(SSD1306_WHITE);
					display.setCursor(0, 0);
					display.println("Tag lida:" + current_reading);

					display.setCursor(0, 7);
					display.print(" ja se encontra");

					display.setCursor(0, 14);
					display.print("registada no sistema");
					display.setCursor(0, 21);
					display.print("operacao cancelada!");
					display.display();


			} else if (current_reading != file1_tags[j]
					&& (j == file_counter_1 - 1) && check != 1&&check_componente!=1) {

				check = 2;

				myFile = SD.open(filename_tags, FILE_WRITE);
				if (myFile) {
					myFile.seek(lastpos_file1);

					myFile.print(',');
					myFile.print(current_reading);

					myFile.close();
					inserirBox_file_2(current_reading);
				}
				Serial.print("\nA tag lida:");
				Serial.print(current_reading);

				Serial.print("\nfoi registada como tag de caixa!");
				last_file1_tag = current_reading;
				update_file_1();
			}
		}

	}

	delay(3000);
	display.clearDisplay();
}

void ler_file_2() {

	myFile = SD.open(filename_box_tag);
	String last_reading;
	if (myFile) {

		// read from the file until there's nothing else in it:
		Serial.print("\nlendo do ficheiro2 as tags que sao de caixa:");
		while (myFile.available()) {
			//lastpos_file2 = myFile.position();

			if (file_counter_2 > 0)
				//last_column=myFile.readString();
				file2_tags[file_counter_2] = myFile.readStringUntil('\n'); // para ler x
			lastpos_file2 = myFile.position();
			file2_tags[file_counter_2] = file2_tags[file_counter_2].substring(0,
					5);

			////////////////////////////////////////////////////
			if (file_counter_2 > 0
					&& file2_tags[file_counter_2].length() == 0) {
				//lastpos_file2 = myFile.position();
			}
			if (file_counter_2 > 0
					&& file2_tags[file_counter_2].length() == 5) {
				//lastpos_file2 = myFile.position();
				last_reading = file2_tags[file_counter_2];
			}

			file_counter_2++;

		}
		myFile.close();
		Serial.print("\nUltima posicao do ficheiro2:");
		Serial.print(last_reading);

		Serial.print("\n:Posicao:");
		Serial.print(lastpos_file2);

		imprimir_file2();
	}

}

void update_ler_file2() {

	myFile = SD.open(filename_box_tag);
	String last_reading;
	if (myFile) {
		while (myFile.available()) {

			if (file_counter_2 > 0)

				file2_tags[file_counter_2] = myFile.readStringUntil('\n'); // para ler x
			lastpos_file2 = myFile.position();
			file2_tags[file_counter_2] = file2_tags[file_counter_2].substring(0,
					5);

			if (file_counter_2 > 0
					&& file2_tags[file_counter_2].length() == 0) {

			}
			if (file_counter_2 > 0
					&& file2_tags[file_counter_2].length() == 5) {

				last_reading = file2_tags[file_counter_2];
			}

			file_counter_2++;

		}
		myFile.close();
		//imprimir_file2();
	}

}
int ler_end_of_file_2() {

	int pos = 0;
	myFile = SD.open(filename_box_tag);

	if (myFile) {

		while (myFile.available()) {

			pos = myFile.position();
			last_column = myFile.readStringUntil('\n');
		}
		myFile.close();

	}
	Serial.print("\nUltima posicao do ficheiro2:");
	Serial.print(last_column);
	Serial.print("\n:Posicao:");
	Serial.print(pos);
	return pos;
}
void inserirBox_file_2(String current_reading) {

	update_ler_file2();
	int check = 0;
	if (current_reading.length() != 0) {
		for (int j = 0; j < file_counter_2; j++) {
			if (current_reading == file2_tags[j]) {
				if (current_reading.length() != 0)
					Serial.print(

					"\nTag ja inserida no sistema e corresponde a uma caixa ");
				check = 1;
			} else if (current_reading != file2_tags[j]
					&& (j == file_counter_2 - 1) && check != 1) {
				check = 1;
				file_counter_2++;
				file2_tags[file_counter_2] = current_reading;
				myFile = SD.open(filename_box_tag, FILE_WRITE);
				if (myFile) {
					myFile.seek(lastpos_file2);
					//	myFile.print('\n');
					myFile.println(current_reading);
					Serial.print("\nA tag lida:");
					Serial.print(current_reading);
					Serial.print(
							"\nfoi registada como tag de caixa no ficheiro 2!");
					myFile.close();
				}

				display.clearDisplay();
				display.setTextSize(0.5);
				display.setTextColor(SSD1306_WHITE);
				display.setCursor(0, 0);
				display.print("Tag lida:");
				display.print(current_reading);
				display.setCursor(0, 7);
				display.print(" foi adicionada");

				display.setCursor(0, 14);
				display.print("como tag caixa");
				display.setCursor(0, 21);
				display.print("com sucesso");
				display.display();
				delay(2000);

			}
		}
	} else
		Serial.print("nao ha tags lidas");
}

String Inserir_tag_main_() {
	Serial.print(
			"\nInserir tag de caixa a que se pretende associar numa nova tag");
	String current_reading;
	card = RFID.A_single_poll_of_instructions();
	current_reading = card._EPC;
	current_reading = current_reading.substring(19, 24);
	if (current_reading.length() != 0) {
		Serial.print(
				"\n --------------------------TAG LIDA------------------------ ");
		Serial.print(current_reading); // 00-99
		Serial.print(
				"\n -------------------------- FIM DA LEITURA------------------------ ");
		RFID.clean_data();
		return current_reading;
	}
	Serial.print(
			"\n -------------------------- TAG NAO LIDA------------------------ ");
	RFID.clean_data();
}
void Inserir_tag_nova_() {
	int check = 0;
	char new_tag[6];
	char tag_main[6];
	String reading_main;
	String reading;

	int check_main;
	int escolha = 0;

	Serial.print(
			"\n Colocar a tag nova a inserir no leitor e pressionar o botão 1 para ler");

	display.clearDisplay();
	display.setTextSize(0.5);
	display.setTextColor(SSD1306_WHITE);
	display.setCursor(0, 0);
	display.print("colocar tag a");
	display.setCursor(0, 7);
	display.print("registar no");
	display.setCursor(0, 14);
	display.print("leitor e permir");
	display.setCursor(0, 21);
	display.print("o botao 1");
	display.display();

	while (escolha == 0) {
		escolha = menu_select_all();
		if (escolha == 1) {
			reading = "";
			reading = func_adicionar_novas_tags();
			check = 1;

		}
		delay(1000);
	}
	if (check == 1) {
		check = 0;
		escolha = 0;
		Serial.print("\nTag lida:");

		Serial.print(reading);
		Serial.print("Pretende confirmar insercao 1-Sim  2-Nao");

		display.clearDisplay();
		display.setTextSize(0.5);
		display.setTextColor(SSD1306_WHITE);
		display.setCursor(0, 0);
		display.print("tag lida:");
		display.print(reading);
		display.setCursor(0, 7);
		display.print("se deseja registar");
		display.setCursor(0, 14);
		display.print("permir o botao 1");

		display.display();
		escolha = menu_select_all();
		while (escolha != 1 || escolha != 2) {
			escolha = 0;
			escolha = menu_select_all();
			delay(1000);

			if (escolha == 1) {

				reading.toCharArray(new_tag, 6);

				myFile = SD.open(filename_box_tag);
				if (myFile) {

					if (myFile.find(new_tag)) {

						display.clearDisplay();
						display.setTextSize(0.5);
						display.setTextColor(SSD1306_WHITE);
						display.setCursor(0, 0);
						display.print("operacao cancelada!!");
						display.setCursor(0, 7);
						display.print("tag lida:");
						display.print(reading);
						display.setCursor(0, 14);
						display.print("ja se encontra");
						display.setCursor(0, 21);
						display.print("no sistema");
						display.display();
						Serial.print(
								"\n Tag ja registada no sistema logo nao podera ser adicionada novamente!");
						Serial.print("\n voltar ao menu inicial");
						check = 1;
						myFile.close();
						delay(3000);
						return;
					}
					myFile.close();

				}

				if (check == 0) {

					display.clearDisplay();
					display.setTextSize(0.5);
					display.setTextColor(SSD1306_WHITE);
					display.setCursor(0, 0);
					display.print("tag lida:");
					display.print(reading);
					display.setCursor(0, 7);
					display.print("valida para registo");
					display.print(reading);
					display.setCursor(0, 14);
					display.print("permir 1 para");
					display.setCursor(0, 21);
					display.print("ler caixa a associar");
					display.display();

					Serial.print(
							"\n Primeira tag lida valida para insersao, por favor colocar  tag  correspondente a caixa que pretende associar e pressionar o botão 1 para ler");

					escolha = 0;

					while (menu_select_all() != 1 || reading_main.length() != 5) {
						escolha = menu_select_all();
						delay(1000);
						if (escolha == 1) {
							reading_main = "";
							reading_main = func_adicionar_novas_tags();

							delay(1000);

						}
					}

					display.clearDisplay();
					display.setTextSize(0.5);
					display.setTextColor(SSD1306_WHITE);
					display.setCursor(0, 0);
					display.print("tag caixa :");
					display.print(reading_main);
					display.setCursor(0, 7);
					display.print("pretende associar a tag:");

					display.setCursor(0, 14);
					display.print(reading);
					display.print("a caixa:");
					display.print(reading_main);
					display.setCursor(0, 21);
					display.print("permir botao 1-confirmar");
					display.display();

					escolha = 0;
					Serial.print("\n tag  de caixa lida:");
					Serial.print(reading_main);
					Serial.print("\nconfirmar insercao 1-Sim  2-Nao");
					while ((menu_select_all() != 1 || menu_select_all() != 2)) {

						escolha = menu_select_all();
						delay(1000);
						if (escolha == 1 && reading_main.length() == 5) {
							Serial.print("\n segunda tag lida!");
							Serial.print(reading_main);
							int escolha = 0;
							int check_main = 0;

							for (int j = 0; j < file_counter_2; j++) {
								if (reading_main == file2_tags[j]) {
									check_main = 1;

									display.clearDisplay();
									display.setTextSize(0.5);
									display.setTextColor(SSD1306_WHITE);
									display.setCursor(0, 0);
									display.print("tag lida:");
									display.print(reading);
									display.setCursor(0, 7);
									display.print("foi associada a caixa:");

									display.setCursor(0, 14);
									display.print(reading_main);
									display.print(" com");
									display.print(reading_main);
									display.setCursor(0, 21);
									display.print("sucesso!!");
									display.display();

									Serial.print("\n tag nova: ");
									Serial.print(reading);
									Serial.print("\n associada a tag");
									Serial.print(tag_main);
									Serial.print("\n com sucesso");
									put_on_line(reading, reading_main);
									Serial.print(reading_main);
								} else if (j == file_counter_2 - 1
										&& check_main == 0) {
									display.clearDisplay();
									display.setTextSize(0.5);
									display.setTextColor(SSD1306_WHITE);
									display.setCursor(0, 0);
									display.print("tag de caixa:");
									display.print(reading_main);
									display.setCursor(0, 7);
									display.print("nao se encontra");

									display.setCursor(0, 14);

									display.print("no sistema ");

									display.setCursor(0, 21);
									display.print("operacao cancelada");
									display.display();

									Serial.print("\n tag nova: ");
									Serial.print(reading);
									Serial.print("\n associada a tag");
									Serial.print(tag_main);
									Serial.print("\n com sucesso");
									Serial.print(
											"\n a segunda tag de caixa lida nao se encontra no sistema ou nao e uma tag correspondente a caixa para associar:");

									Serial.print("\n Voltar ao menu inicial");
									delay(2000);
									return;

								}

							}
						}
					}

				}

			} else if (escolha == 2) {

				Serial.print(
						"\nVoltar para o menu inicial sem processo de insercao concluido");
				delay(1000);
				return;
			}

		}
	}

}

void put_on_line(String tag_nova, String tag_main) { // para uma tag main adquirir as tags que lhe estao associadas

	int pos = 0;
	int count2 = 0;
	char *ptr;
	char char_array[6];
	char last_reading_char[6];
	String current_reading_2[30];
	String reading;
	String tag_main_2;
	String last_reading;
	String deletada;
	int pos2 = 0;

	tag_main_2 = tag_main;
	tag_main_2.toCharArray(char_array, 6);
	if (tag_nova.length() != 0 && tag_main.length() == 5) {
		Serial.print("\nInsercao da tag :");
		Serial.print(tag_nova);
		Serial.print("na caixa: ");
		Serial.print(char_array);
		myFile = SD.open(filename_box_tag, FILE_READ);
		if (myFile) {
			if (myFile.find(char_array)) {  // procura a tag main
				myFile.position();         // devolve a posiÃ§Ã£o
				while (myFile.available()) {
					reading = myFile.readStringUntil('\n');
					deletada = myFile.readStringUntil('\0');
					if (myFile.find('\n')) { // serve para assimuir apenas a 1Âª coluna - sai na primeira ','

						break;
					}
				}
			}

			char column[reading.length()];
			Serial.print("\n\nLinha completa da tag:");

			Serial.print(char_array);
			Serial.print(":\n");
			Serial.print(reading);
			Serial.print("\n\n-------------------------------------------");
			reading.toCharArray(column, reading.length());
			if (reading.length() > 5) {
				ptr = strtok(column, ",");
				while (ptr != NULL) {
					last_reading = ptr;

					ptr = strtok(NULL, ",");
				}

				last_reading.toCharArray(last_reading_char, 6);
			} else {
				last_reading = tag_main;
				last_reading.toCharArray(last_reading_char, 6);

				Serial.print(last_reading);

			}
			myFile = SD.open(filename_box_tag, FILE_READ);
			if (myFile) {

				if (myFile.find(last_reading_char)) {

					pos = myFile.position();

				}
				myFile.close();
				Serial.print(last_reading_char);
				myFile = SD.open(filename_box_tag, FILE_WRITE);

				if (myFile.seek(pos)) {
					myFile.print(',');
					myFile.println(tag_nova);

					myFile.print(deletada);

					if (myFile.seek(lastpos_file2)) {
						String teste = myFile.readStringUntil('\n');
						Serial.print(teste);
						myFile.position();
						myFile.close();

					}

				}
				myFile.close();

				Serial.print("\nA ultima tag lida:");
				Serial.print(last_reading_char);
				Serial.print("\n");

			}

		}

	}

}

void imprimir_file1() {
	if (file_counter_1 != 0) {

		Serial.print("\nTags main registadas no ficheiro 1:");

		for (int i = 0; i < file_counter_1; i++) {
			Serial.print("\n");
			Serial.print(file1_tags[i]);

		}

	}
}

void imprimir_file2() {
	if (file_counter_2 != 0) {

		Serial.print("\nTags main registadas no ficheiro 2:");

		for (int i = 0; i < file_counter_2; i++) {
			Serial.print("\n");
			Serial.print(file2_tags[i]);

		}
		Serial.print("\nultima coluna");
		Serial.print(last_column);
	}
}
void write_sd() {
	myFile = SD.open(filename_tags, FILE_WRITE);

	if (myFile) {

// read from the file until there's nothing else in it:

		Serial.print("\n\nWriting to file1...");
		myFile.print("9ddd2,9ddde");
		myFile.close();

	}

	myFile = SD.open(filename_box_tag, FILE_WRITE);
	if (myFile) {

// read from the file until there's nothing else in it:

		Serial.print("\n\nWriting to file2...");

		myFile.println("9ddd2");
		myFile.println("9ddde,928ff,9dddb,9d4df");

		myFile.close();
		Serial.println("done.");
	}

}

void remove_tag() {
	if (file_counter_1 != 0) {

		Serial.print("\nTags main registadas no ficheiro 1:");

	}

}

int http_get_message() {
	int button = 0;
	if (WiFi.status() == WL_CONNECTED) {   //Check WiFi connection status

		http.begin(server_menu_select);  //Specify destination for HTTP request
		//http.addHeader("Content-Type","application/json" );             //Specify content-type header

		int httpResponseCode = http.GET();   //Send the actual POST request

		if (httpResponseCode > 0) {
			String response = http.getString(); //Get the response to the request

			//Serial.println("RESPOSTAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA");   //Print return code
			Serial.println(response);           //Print request answer

			if (response == "verificar_caixa") {
				button = 1;

			}

			else if (response == "adicionar_caixa") {
				button = 2;

			}

			else if (response == "adicionar_tag") {
				button = 3;

			}

			Serial.println("\nResponse from Node-red:");
			Serial.println(response);
			Serial.println(button);
			http.end();

			return button;
		} else {
			http.end();
			return 0;
		}

	} else {

		Serial.print("Error on Wifi connection! ");

	}

	http.end();
	return 0;
}

int menu_select_keyboard_() {

	char key = keypad.getKey();
//int user_menu = User_menu_input();

	int select = 0;
	if (key == '1')
		select = 1;
	else if (key == '2')
		select = 2;
	else if (key == '3')
		select = 3;
	else if (key == '4')
		select = 4;
	else if (key == '5')
		select = 5;
	else
		select = 0;

	Serial.print("\nResponse from keyboard:");
	Serial.print(select);

	return select;

}

int menu_select_all() {
	int user_menu;
	int keyboard_select;
	int http_menu;
	user_menu = User_menu_input();
	keyboard_select = menu_select_keyboard_();
// http_menu = http_get_message();

	int selection = 0;
	if (user_menu == 1 || http_menu == 1 || keyboard_select == 1)
		selection = 1;
	else if (user_menu == 2 || http_menu == 2 || keyboard_select == 2)
		selection = 2;
	else if (user_menu == 3 || http_menu == 3 || keyboard_select == 3)
		selection = 3;
	else if (user_menu == 4 || http_menu == 4 || keyboard_select == 4)
		selection = 4;
	else if (user_menu == 5 || http_menu == 5 || keyboard_select == 5)
		selection = 5;
	else
		selection = 0;

	return selection;

}
