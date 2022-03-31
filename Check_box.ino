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

const char *ssid = "labs";
const char *password = "robot1cA!ESTG";

// REPLACE with your Domain name and URL path or IP address with path
const char *serverName = "http://10.20.228.136/post-esp-data.php";
// MQTT broker
const char *mqtt_server = "test.mosquitto.org";
const char *topic_pub = "SmartLab/ESP32/test";

/* LED pin */
int led = 26;
/* LED state */
volatile byte state_led = LOW;
// Keep this API Key value to be compatible with the PHP code provided in the project page.
// If you change the apiKeyValue value, the PHP file /post-esp-data.php also needs to have the same key
String apiKeyValue = "tPmAT5Ab3j7F9";
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

String commands_str = { "1-Verificação de tags na caixa"
		"\n2-Inserir novas tags" };
String commands_str_menu2 = { "1-Inserir nova tag de caixa"
		"\n2-associar uma tag a uma tag de caixa existente" };
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
	Serial.println("");
	Serial.print("Connected to WiFi network with IP Address: ");
	client.setServer(mqtt_server, 1883);
	client.setCallback(callback);
	client.connect("SmartLab");
	// Your Domain name with URL path or IP address with path
	http.begin(Wificlient, serverName);

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

void callback(char *topic, byte *payload, unsigned int length) {
	Serial.print("Message arrived [");
	Serial.print(topic);
	Serial.print("] ");
	for (int i = 0; i < length; i++) {
		Serial.print((char) payload[i]);
	}
	Serial.println();
}

/* MQTT
 void reconnect() {
 // Loop until we're reconnected
 while (!client.connected()) {
 Serial.print("Attempting MQTT connection...");
 // Create a random client ID
 String clientId = "ESP8266Client-";
 clientId += String(random(0xffff), HEX);
 // Attempt to connect
 if (client.connect(clientId.c_str())) {
 Serial.println("connected");
 // Once connected, publish an announcement...
 client.publish("outTopic", "hello world");
 // ... and resubscribe
 client.subscribe("inTopic");
 } else {
 Serial.print("failed, rc=");
 Serial.print(client.state());
 Serial.println(" try again in 5 seconds");
 // Wait 5 seconds before retrying
 delay(5000);
 }
 }
 }*/

void IRAM_ATTR onTimer() {
	state_led = !state_led;
	digitalWrite(led, state_led);
}

// The loop function is called in an endless loop
void loop() {
	int x, y, z;
	Menu_main (User_menu_input());delay
	(5000);
	timerAlarmDisable(timer);

	/*if(WiFi.status()== WL_CONNECTED){

	 // Your Domain name with URL path or IP address with path
	 //http.begin(Wificlient, serverName);
	 //http.begin("https://10.20.228.136/esp-data.php"); //HTTPS
	 // Specify content-type header
	 http.addHeader("Content-Type", "application/x-www-form-urlencoded");

	 // Prepare your HTTP POST request data
	 //    String httpRequestData = "api_key=" + apiKeyValue + "&sensor=" + sensorName
	 //                          + "&location=" + sensorLocation + "&value1=" + String(bme.readTemperature())
	 //                          + "&value2=" + String(bme.readHumidity()) + "&value3=" + String(bme.readPressure()/100.0F) + "";

	 String httpRequestData = "api_key=" + apiKeyValue + "&sensor=" + sensorName
	 + "&location=" + sensorLocation + "&value1=" + String(x)
	 + "&value2=" + String(y) + "&value3=" + String(z) + "";

	 Serial.print("httpRequestData: ");
	 Serial.println(httpRequestData);

	 // You can comment the httpRequestData variable above
	 // then, use the httpRequestData variable below (for testing purposes without the BME280 sensor)
	 //    String httpRequestData = "api_key=tPmAT5Ab3j7F9&sensor=BME280&location=Office&value1=24.75&value2=49.54&value3=1005.14";
	 //    Serial.println(httpRequestData);

	 // Send HTTP POST request
	 int httpResponseCode = http.POST(httpRequestData);

	 // If you need an HTTP request with a content type: text/plain
	 //http.addHeader("Content-Type", "text/plain");
	 //int httpResponseCode = http.POST("Hello, World!");

	 // If you need an HTTP request with a content type: application/json, use the following:
	 //http.addHeader("Content-Type", "application/json");
	 //int httpResponseCode = http.POST("{\"value1\":\"19\",\"value2\":\"67\",\"value3\":\"78\"}");

	 if (httpResponseCode>0) {
	 Serial.print("HTTP Response code: ");
	 Serial.println(httpResponseCode);
	 }
	 else {
	 Serial.print("Error code: ");
	 Serial.println(httpResponseCode);
	 }
	 // Free resources
	 http.end();
	 }
	 else {
	 Serial.println("WiFi Disconnected");
	 }
	 //Send an HTTP POST request every 30 seconds


	 // MQTT
	 if(!client.connected())
	 reconnect();
	 client.loop();
	 client.publish("MQTT/ESP32/test","SmartLab_test");


	 */

}
void Menu_main(int menu_select) {
	int check = 0;
	int pos;
	Serial.print(commands_str);
	switch (menu_select) {
	case 1:

		func_verificar_tags();
		break;

	case 2:
		InserirBox_file_1();
		break;
	case 3:
		put_on_line();
		break;
	case 4:
		//existing_tags("test5");
		break;
	case 5:
		break;

	default:

		break;
	}
}

int User_menu_input() {

	int menu_select;

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
			timerAlarmEnable(timer);

		}

		for (int j = 0; j < file_counter_1; j++) {
			if (current_reading[i] == file2_tags[j]) {
				Serial.print("\nTag associada a  uma caixa detetada : ");
				Serial.print(current_reading[i]);
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

		Serial.print("\n\nTAGS que estao associadas a tag da caixa lida: ");
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
			} else {
				Serial.print("\n tag ");
				Serial.print(current_reading_2[y]);
				Serial.print("\ presente na caixa");
				present_tags[present_tag_counter] = current_reading_2[y];
				present_tag_counter++;
			}
		}

		if (check == 0) {

			Serial.print("\n A caixa esta completa");
			state = "completa";
		} else {
			state = "incompleta";
			Serial.print("\n A caixa nao esta completa");
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
			for (int i = 0; i < count3; i++) {

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

			/*Serial.print("\nROW*************************************");
			 Serial.print(row_reading[counter]);
			 Serial.print("END*************************************");
			 */
			row_reading[counter].toCharArray(column,
					row_reading[counter].length());
			ptr = strtok(column, ",");
			while (ptr != NULL) {
				current_reading[count2] = ptr;
				count2++;
				ptr = strtok(NULL, ",");
			}

			for (int i = 0; i < count2; i++) {
				/*	Serial.print("\n*************************************");
				 Serial.print(current_reading[i]);
				 Serial.print("\n");*/
				if (search_string == current_reading[i]) {
					check = 1;
					tag_main = current_reading[0];

				}

			}

		}
		if (search_string != tag_main) {
			if (check == 1) {
				Serial.print("\A tag:");
				Serial.print(search_string);
				Serial.print(" esta registada no sistema e ");
				Serial.print("corresponde a caixa: ");
				Serial.print(tag_main);
				Serial.print("\n-------------------------------------------\n");
			} else {
				Serial.print("\A tag:");
				Serial.print(search_string);
				Serial.print(" nao esta registada no sistema:  ");

				Serial.print("\n-------------------------------------------\n");
			}
		} else {

			Serial.print("A tag:");
			Serial.print(search_string);
			Serial.print(" e uma tag caixa");
			Serial.print("\n------------------------------------------\n");
		}

	}
}
void raw_registo(String present_tags[], String main, String state) {
	DateTime now = rtc.now();
	myFile = SD.open(filename_raw_registos, FILE_WRITE);
	String all = "";
	if (myFile) {
		myFile.print('\n');
		myFile.print("ESP32-modulo1");
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
			//all=all+present_tags[i];

		}
		myFile.print(state);
		myFile.print('\n');
		myFile.close();

		if (WiFi.status() == WL_CONNECTED) {

			http.addHeader("Content-Type", "application/x-www-form-urlencoded");

			String httpRequestData = "api_key=" + apiKeyValue + "&sensor="
					+ sensorName + "&location=" + sensorLocation + "&value1="
					+ String(state) + "&value2=" + String(main) + "&value3="
					+ String(present_tags[0]) + "&value4="
					+ String(present_tags[1]) + "&value5="
					+ String(present_tags[2]) + "&value6="
					+ String(present_tags[3]) + "&value7="
					+ String(present_tags[4]) + "&value8="
					+ String(present_tags[5]) + "&value9="
					+ String(present_tags[6]) + "&value10="
					+ String(present_tags[7] + "");

			/*  String httpRequestData = "api_key=" + apiKeyValue + "&sensor=" + sensorName
			 + "&location=" + sensorLocation + "&value1=" + String(state)
			 + "&value2=" + String(main)+ "&value3=" + all +"";

			 */

			Serial.print("httpRequestData: ");
			Serial.println(httpRequestData);
			int httpResponseCode = http.POST(httpRequestData);

			/* if (httpResponseCode>0) {
			 Serial.print("HTTP Response code: ");
			 Serial.println(httpResponseCode);
			 }
			 else {
			 Serial.print("Error code: ");
			 Serial.println(httpResponseCode);
			 }*/

			http.end();
		} else {
			Serial.println("WiFi Disconnected");

		}

	}

}
void registo(String main, String present_tags[], int count, String state) {
	DateTime now = rtc.now();
	myFile = SD.open(filename_registos, FILE_WRITE);

	if (myFile) {
		myFile.print('\n');
		myFile.print("ESP32-modulo1");
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
		myFile.print('\n');
		myFile.close();

	}

}
void existing_tags2(String search_string[], int num) {
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
			//Serial.print("\ncoluna ");
			//Serial.print(file_counter_1); // 00-99
			//Serial.print(": ");
			//Serial.print(file1_tags[file_counter_1]);
			file_counter_1++;

			//Serial.print("\t pos:");
			//Serial.print(lastpos_file1);
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
	int check = 0;
	for (int j = 0; j < file_counter_1; j++) {
		if (current_reading == file1_tags[j]) {
			Serial.print(
					"\nTag ja inserida no sistema e corresponde a uma caixa ");
			check = 1;
		} else if (current_reading != file1_tags[j] && (j == file_counter_1 - 1)
				&& check != 1) {
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

void ler_file_2() {

	myFile = SD.open(filename_box_tag);
	String last_reading;
	if (myFile) {

		// read from the file until there's nothing else in it:
		Serial.print("\nlendo do ficheiro2 as tags que sao de caixa:");
		while (myFile.available()) {
			if (file_counter_2 > 0)
				myFile.readStringUntil('\n'); // para ler x

			file2_tags[file_counter_2] = myFile.readStringUntil(',');

			////////////////////////////////////////////////////

			lastpos_file2 = myFile.position();
			if (file_counter_2 > 0
					&& file2_tags[file_counter_2].length() == 5) {
				lastpos_file2 = myFile.position();
				last_reading = file2_tags[file_counter_2];
			}
			///////////////////////////////////////////////////
			//Serial.print("\ntags mains lidas do ficheiro do equipamento----------------------------------:  ");
			//Serial.print(file_counter_2); // 00-99
			//Serial.print(": ");
			//Serial.print(file2_tags[file_counter_2]);
			file_counter_2++;

		}
		myFile.close();
		Serial.print("\nUltima posicao do ficheiro2:");
		Serial.print(last_reading);
		imprimir_file2();
	}

}

void inserirBox_file_2(String current_reading) {
	int check = 0;
	if (current_reading.length() != 0) {
		for (int j = 0; j < file_counter_2; j++) {
			if (current_reading == file2_tags[j]) {
				if (current_reading != 0)
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
					myFile.print(',');
					myFile.print('\n');
					myFile.print(current_reading);
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

/*void write_sd() {
 myFile = SD.open(filename_box_tag, FILE_WRITE);
 if (myFile) {
 // read from the file until there's nothing else in it:
 Serial.print("\n\nWriting to file 1...");
 myFile.print("928ff, 9fffe, 9fff2.");
 // close the file:
 myFile.close();
 Serial.println("done.");
 }
 myFile = SD.open(filename_box_tag, FILE_WRITE);
 if (myFile) {
 // read from the file until there's nothing else in it:
 Serial.print("\n\nWriting to file 2...");
 myFile.println("928ff, 9fffe, 9fff2.");
 // close the file:
 myFile.close();
 Serial.println("done.");
 }
 }*/
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
String Inserir_tag_nova_() {
	Serial.print("\nInserir tag nova a caixa");
	String reading;
	card = RFID.A_single_poll_of_instructions();
	reading = card._EPC;
	reading = reading.substring(19, 24);
	while (card._EPC.length() != 24) {
		if (card._EPC.length() != 24) {
			Serial.println("\nInserir nova tag");
		} else {
			if (card._EPC.length() == 24) {
				return reading;
			}
		}
		RFID.clean_data(); //Empty the data after using it 使用完数据后要将数据清空}
		delay(1000);
	}
	RFID.clean_data();
}

void put_on_line() { // para uma tag main adquirir as tags que lhe estao associadas
	int check = 0;
	int pos = 0;
	int count = 0;
	int count2 = 0;
	char *ptr;
	char char_array[7];
	char last_reading_char[6];
	String current_reading_2[100];
	String reading;
	String tag_main_2;
	String last_reading;
	//String tag_main = Inserir_tag_main_();
	//String tag_nova = Inserir_tag_nova_();
	//String current_reading = func_adicionar_novas_tags();
	String tag_main = "928ff";
	String tag_nova = "merda";
	tag_main_2 = tag_main + ",";
	tag_main_2.toCharArray(char_array, 6);
	if (tag_nova.length() == 5 && tag_main.length() == 5) {
		Serial.print("\nInserir na caixa:");
		Serial.print(char_array);
		Serial.print("\nA tag");
		Serial.print(tag_nova);
		myFile = SD.open(filename_box_tag, FILE_READ);
		if (myFile) {
			if (myFile.find(char_array)) {
				myFile.position();
				while (myFile.available()) {
					reading = myFile.readStringUntil('\n');
					Serial.print(current_reading_2[count]);
					if (myFile.find(",")) {
						pos = myFile.position();
						count++;
						break;
					}
				}
			}
			myFile.close();
		}
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
			current_reading_2[count2] = ptr;
			last_reading = ptr;
			count2++;
			ptr = strtok(NULL, ",");
		}
		last_reading.toCharArray(last_reading_char, 7);
		myFile = SD.open(filename_box_tag);
		if (myFile) {
			if (myFile.find(char_array)) {
				pos = myFile.position();
				myFile.close();
				Serial.print(last_reading_char);
				myFile = SD.open(filename_box_tag, FILE_WRITE);
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
				}
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

	}
}
