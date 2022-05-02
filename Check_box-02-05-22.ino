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

//#include <Adafruit_GFX.h>
//#include <Adafruit_SSD1306.h>

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

//File myFile;
const int chipSelect = 5;
File myFile;
RTC_DS3231 rtc;
unsigned long lastTime = 0;
unsigned long timerDelay = 5000;
//Adafruit_SSD1306 display = Adafruit_SSD1306(128, 32, &Wire);
const char *ssid = "labs";
const char *password = "robot1cA!ESTG";


//const char *serverName = "http://10.20.228.80/post-esp-data.php";
// MQTT broker


/* LED pin */
int led = 26;
/* LED state */
volatile byte state_led = LOW;
// Keep this API Key value to be compatible with the PHP code provided in the project page.
// If you change the apiKeyValue value, the PHP file /post-esp-data.php also needs to have the same key

String sensorName = "esp32-modulo1";
String sensorLocation = "lab";

WiFiClient Wificlient;
PubSubClient client(Wificlient);
HTTPClient http;

//ficheiro das tags
char filename_tags[] = "/taglist.CSV";
char filename_box_tag[] = "/boxlist.CSV";
char filename_registos[] = "/registos.CSV";
char filename_raw_registos[] = "/todos_registos.CSV";
String file1_tags[30];
int file_counter_1 = 0;
int lastpos_file1 = 0;
String file2_tags[30];
int file_counter_2 = 0;
int lastpos_file2 = 0;
String last_column ;
String commands_str = { "\n1-Verificao de tags na caixa"
		"\n2-Inserir uma nova tag como tag de caixa"
		"\n3-inserir uma nova tag a uma caixa existente"
		"\n4-Opcao programador de inserir dados no cartao" };

int count_non_registered = 0;
int count_registered = 0;
void setup() {
	Serial2.begin(115200, SERIAL_8N1, 16, 17); //16.17
	Serial.begin(115200);
	//attachInterrupt(button_read, func_read_all, FALLING);
	//rtc.init();
	RFID.Set_transmission_Power(2600);
	RFID.Set_the_Select_mode();
	RFID.Delay(100);
	RFID.Readcallback();
	RFID.clean_data();
	//display.begin(SSD1306_SWITCHCAPVCC, 0x3C); // Address 0x3C for 128x32
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
	/*display.clearDisplay();
		display.setTextSize(2);
		display.setTextColor(SSD1306_WHITE);
		display.setCursor(0, 0);
		display.println("Checkbox");
		display.display();
	/*WiFi.begin(ssid, password);
	 Serial.println("Connecting to wifi...");
	 while (WiFi.status() != WL_CONNECTED) {
	 delay(500);
	 Serial.print("still not connected to wifi");
	 }
	 Serial.println("");
	 Serial.print("Connected to WiFi network with IP Address: ");
	 client.setServer(mqtt_server, 1883);
	 client.setCallback(callback);
	 client.connect("SmartLab");
	 // Your Domain name with URL path or IP address with path
	 http.begin(Wificlient, serverName);
	 */
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

}





void IRAM_ATTR onTimer() {
	state_led = !state_led;
	digitalWrite(led, state_led);
}

// The loop function is called in an endless loop
void loop() {
	int x, y, z;
	Menu_main (User_menu_input());
	delay
		(5000);


}
void Menu_main(int menu_select) {
	int check = 0;
	int pos;
	//Serial.print(commands_str);
	switch (menu_select) {
	case 1:

		func_verificar_tags();
		delay(1000);
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

	int menu_select;
Serial.print(commands_str);
/*
	display.clearDisplay();
	display.setTextSize(1);
	display.setTextColor(SSD1306_WHITE);
	display.setCursor(0, 0);
	display.println("1-verificacao");
	display.setCursor(0, 10);
	display.print("2-Adicionar tag caixa");
	display.setCursor(0, 20);
	display.print("3-Adicionar tag nova");
	display.setCursor(0, 30);
		display.print("4-Remover tags");
	display.display();
*/
while (!Serial.available())
		;

	if (Serial.available()) {
		menu_select = Serial.parseInt();
		if (menu_select != 0 && menu_select != 1 && menu_select != 2
				&& menu_select != 3 && menu_select != 4 && menu_select != 5) {
			Serial.println("opcao incorreta");

		} else
			Serial.println("Menu selected: ");
		Serial.println(menu_select);
		return menu_select;

	}

}

void func_verificar_tags() { //verifica se alguma tag lida corresponde a tags de uma caixa no cartao microsd
	int check = 0;
	String current_reading2[20];
	String current_reading[20];
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

			Serial.print("\n tag ");
			Serial.print(i);
			Serial.print(" lida pelo leitor:");

			Serial.print(current_reading[i]); // 00-99
			//Serial.print("\tRSSI:");
			//Serial.print(cards.card[i]._RSSI); //Received Signal Strength Indicator
			if (i == cards.len) {

				Serial.print(
						"\n -------------------------- FIM DA LEITURA------------------------ ");
			}

		}

		for (int j = 0; j < file_counter_1; j++) {

			if (current_reading[i] == file2_tags[j]) {
				//Serial.print("\nTag associada a  uma caixa detetada : ");
				//Serial.print(current_reading[i]);
				tag_caixa = current_reading[i];
				check = 1;


				RFID.clean_data();
			} else if (current_reading[i] != current_reading2[j]
					&& (j == file_counter_1 - 1) && (i == cards.len - 1)
					&& check != 1) {
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

		Serial.print("\n\nTAGS no cartao que estao associadas a tag da caixa lida: ");
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
				present_tags[present_tag_counter] = current_reading_2[y];
				present_tag_counter++;
			}
		}

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
		registo(search_string, present_tags, present_tag_counter, state);
		raw_registo(current_reading_array, search_string, state);
		myFile = SD.open(filename_box_tag, FILE_READ);

		for (int j = 0; j < cards.len; j++) {

			check3[j] = 0;
			for (int i = 0; i < count2; i++) {

				if (current_reading_array[j] == current_reading_2[i]) {
					check3[j] = 1;
				}

			}
			if (check3[j] == 0) {
				//current_reading_array_2[count3] = current_reading_array[j];
				//count3++;
				existing_tags(current_reading_array[j]);

			}

		}

	}

	//existing_tags2(current_reading_array_2, count3);

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
/*void existing_tags2(String search_string[], int num) {
 int count = 0;
 String row_reading[30];
 String current_reading[30];
 String tag_main;
 char *ptr;
 char char_array[5];
 int check = 0;
 int count2 = 0;
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
 for (int j = 0; j < num; j++) {
 if (search_string[j] == current_reading[i]) {
 check = 1;
 tag_main = current_reading[0];
 }
 if (check == 1) {
 Serial.print("\A tag ");
 Serial.print(search_string[j]);
 Serial.print("esta registada no sistema e ");
 Serial.print("corresponde a caixa: ");
 Serial.print(tag_main);
 Serial.print(
 "\n-------------------------------------------\n");
 } else {
 Serial.print("\A tag ");
 Serial.print(search_string[j]);
 Serial.print("\n  nao esta registada no sistema:  ");
 Serial.print(
 "\n-------------------------------------------\n");
 }
 }
 }
 }
 }
 }*/
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
	int check = 0;

	String last_reading;
	myFile = SD.open(filename_tags);

	if (myFile) {

		// read from the file until there's nothing else in it:
		//Serial.print("\nlendo do ficheiro1  as tags que sao caixas ");
		while (myFile.available()) {

			file1_tags[file_counter_1] = myFile.readStringUntil(','); // para ler x

			if (file_counter_1 > 0
					&& file1_tags[file_counter_1].length() == 5) {

				lastpos_file1 = myFile.position();
				last_reading = file1_tags[file_counter_1];
			}

			file_counter_1++;


		}
		myFile.close();
		Serial.print("\nlendo do cartao as tags que sao caixas");

		Serial.print("\nultima tag lida:");
		Serial.print(last_reading);

		imprimir_file1();

	}

}

void InserirBox_file_1() {
	String current_reading = func_adicionar_novas_tags();
	if (current_reading.length() != 0) {
		int check = 0;
		for (int j = 0; j < file_counter_1; j++) {
			if (current_reading == file1_tags[j]) {
				Serial.print(
						"\nTag ja inserida no sistema e corresponde a uma caixa ");
				check = 1;
			} else if (current_reading != file1_tags[j]
					&& (j == file_counter_1 - 1) && check != 1) {
				file_counter_1++;
				file1_tags[file_counter_1] = current_reading;
				check = 1;
				myFile = SD.open(filename_tags, FILE_WRITE);
				if (myFile) {
					myFile.seek(lastpos_file1);
					myFile.print(',');
					myFile.print(current_reading);
					Serial.print("\nA tag lida:");
					Serial.print(current_reading);
					Serial.print("\nfoi registada como tag de caixa!");
					myFile.close();
					inserirBox_file_2(current_reading);
				}
			}
		}
	}
}

void ler_file_2() {

	myFile = SD.open(filename_box_tag);
	String last_reading;
	if (myFile) {

		// read from the file until there's nothing else in it:
		//Serial.print("\nlendo do ficheiro2 as tags que sao de caixa:");
		while (myFile.available()) {
			//lastpos_file2 = myFile.position();

			if (file_counter_2 > 0)
				//last_column=myFile.readString();
			myFile.readStringUntil('\n'); // para ler x

			file2_tags[file_counter_2] = myFile.readStringUntil(',');

			////////////////////////////////////////////////////
			if (file_counter_2 > 0
								&& file2_tags[file_counter_2].length() == 0) {
			lastpos_file2 = myFile.position();
			}
			if (file_counter_2 > 0
					&& file2_tags[file_counter_2].length() == 5) {
				//lastpos_file2 = myFile.position();
				last_reading = file2_tags[file_counter_2];
			}

			file_counter_2++;

		}
		myFile.close();
		//Serial.print("\nUltima posicao do ficheiro2:");
		//Serial.print(last_reading);

		//Serial.print("\n:Posicao:");
		Serial.print(lastpos_file2);

//		imprimir_file2();
	}

}int ler_end_of_file_2() {

	int pos = 0;
	myFile = SD.open(filename_box_tag);

	if (myFile) {

		while (myFile.available()) {

			pos = myFile.position();
			last_column=myFile.readStringUntil('\n');
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

	ler_file_2();
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

					myFile.println(current_reading+',');
					Serial.print("\nA tag lida:");
					Serial.print(current_reading);
					Serial.print("\nfoi registada como tag de caixa!");
					myFile.close();
				}
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
	Serial.print("\n Colocar a tag nova a inserir no leitor e pressionar o botão 1 para ler");

	while (escolha != 1 || reading.length() != 5) {
		escolha = Serial.parseInt();
		if (escolha == 1) {
			reading = "";
			reading = func_adicionar_novas_tags();
			check = 1;

			delay(1000);
		}

	}
	if (check == 1) {
		check = 0;
		escolha = 0;
		Serial.print("\nTag lida:");

		Serial.print(reading);
		Serial.print("Pretende confirmar insercao 1-Sim  2-Nao");
		escolha = Serial.parseInt();
		while ((escolha != 1 || escolha != 2)) {

			escolha = Serial.parseInt();
			if (escolha == 1) {

				reading.toCharArray(new_tag, 6);

				myFile = SD.open(filename_box_tag);
				if (myFile) {

					if (myFile.find(new_tag)) {

						Serial.print(
								"\n Tag ja registada no sistema logo nao podera ser adicionada novamente!");
						Serial.print("\n voltar ao menu inicial");
						check = 1;
						myFile.close();

						return;
					}
					myFile.close();

				}

				if (check == 0) {
					Serial.print(
							"\n Primeira tag lida valida para insersao, por favor colocar  tag  correspondente a caixa que pretende associar e pressionar o botão 1 para ler");

					escolha = 0;

					while (escolha != 1 || reading_main.length() != 5) {
						escolha = Serial.parseInt();
						if (escolha == 1) {
							reading_main = "";
							reading_main = func_adicionar_novas_tags();

							delay(1000);

						}
					}
					escolha = 0;
					Serial.print("\n tag  de caixa lida:");
					Serial.print(reading_main);
					Serial.print("\nconfirmar insercao 1-Sim  2-Nao");
					while ((escolha != 1 || escolha != 2)) {

						escolha = Serial.parseInt();

					if (escolha == 1 && reading_main.length() == 5) {
						Serial.print("\n segunda tag lida!");
						Serial.print(reading_main);
						int escolha = 0;
						int check_main = 0;



						for (int j = 0; j < file_counter_2; j++) {
							if (reading_main == file2_tags[j]) {
								check_main = 1;
								Serial.print("\n tag nova:");
								Serial.print(reading);
								Serial.print("\n associada a tag");
								Serial.print("\n com sucesso");
								put_on_line(reading,reading_main);
								Serial.print(reading_main);
							} else if (j == file_counter_2 - 1
									&& check_main == 0) {

								Serial.print(
										"\n a segunda tag de caixa lida nao se encontra no sistema ou nao e uma tag correspondente a caixa para associar:");

								Serial.print("\n Voltar ao menu inicial");
								return;
								delay(2000);
							}

						}
					}
					}

				}

			} else if (escolha == 2) {

				Serial.print("\nVoltar para o menu inicial sem processo de insercao concluido");
				delay(1000);
				return;
			}

		}
	}

}


void put_on_line(String tag_nova, String tag_main) { // para uma tag main adquirir as tags que lhe estao associadas
int check = 0;
int pos = 0;
int count = 0;
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
//String tag_main = Inserir_tag_main_();
//String tag_nova = Inserir_tag_nova_();
//String current_reading = func_adicionar_novas_tags();
//String tag_main = "928ff";
//String tag_nova = "tag99";
tag_main_2 = tag_main + ",";
tag_main_2.toCharArray(char_array, 6);
if (tag_nova.length() != 0 && tag_main.length() == 5) {
	Serial.print("\nInsercao da tag:");
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
				if (myFile.find("\n")) { // serve para assimuir apenas a 1Âª coluna - sai na primeira ','
					//Serial.print("posicaooooo");
					//Serial.print(pos2);

					Serial.print(deletada);
					count++;
					break;
				}
			}
		}

		char column[reading.length()];
		/*Serial.print("\n\nLinha completa da tag:");

		Serial.print(char_array);
		Serial.print(":\n");
		Serial.print(reading);
		*/Serial.print("\n\n-------------------------------------------");
		reading.toCharArray(column, reading.length());
		ptr = strtok(column, ",");
		while (ptr != NULL) {
//				current_reading_2[count2] = ptr;
			last_reading = ptr;
//				count2++;
			ptr = strtok(NULL, ",");
		}

		last_reading.toCharArray(last_reading_char, 6);
		myFile = SD.open(filename_box_tag, FILE_READ);
		if (myFile) {
			if (myFile.find(last_reading_char)) {
				pos = myFile.position();
				//Serial.print("\n ultima posicao:");
				//Serial.print(pos);
				//myFile.close();
			}
			myFile.close();
			Serial.print(last_reading_char);
			myFile = SD.open(filename_box_tag, FILE_WRITE);
			if (myFile.seek(pos)) {
				myFile.print(',');
				myFile.println(tag_nova);
				myFile.print(deletada);

				if (myFile.seek(lastpos_file2)) {
					//Serial.print("entrooooooooooooooooou");
					//myFile.print("print");
					String teste = myFile.readStringUntil('\n');
					Serial.print(teste);
					myFile.position();
					myFile.close();

				}
				//myFile.close();
			}
			//myFile.close();
			//Serial.print("\nA ultima tag lida:");
			//Serial.print(last_reading_char);
			//Serial.print("\n");

		}

	}

}

/*myFile = SD.open(filename_box_tag, FILE_WRITE);
 if (myFile) {
 myFile.seek(pos);
 myFile.print(last_reading_char);
 myFile.print(",");
 myFile.print(tag_nova);
 myFile.close();
 Serial.print("\nA ultima tag lida:");
 Serial.print(last_reading_char);
 Serial.print("\n");
 myFile.close();
 }*/

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

	Serial.print("\nTags registadas no ficheiro 2:");

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
	myFile.print("9ddde,9dddb,9ddd2");
	myFile.close();

}

myFile = SD.open(filename_box_tag, FILE_WRITE);
if (myFile) {

// read from the file until there's nothing else in it:

	Serial.print("\n\nWriting to file2...");

	myFile.println("9ddd2,9d4df");
	myFile.println("9ddde,64613");
	myFile.println("9dddb,x2222,x3212,hhghgf");
	myFile.close();
	Serial.println("done.");
}

}




void remove_tag() {
if (file_counter_1 != 0) {

	Serial.print("\nTags main registadas no ficheiro 1:");



}
}





