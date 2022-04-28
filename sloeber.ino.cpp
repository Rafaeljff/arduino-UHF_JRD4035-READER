#ifdef __IN_ECLIPSE__
//This is a automatic generated file
//Please do not modify this file
//If you touch this file your change will be overwritten during the next build
//This file has been generated on 2022-04-27 07:52:36

#include "Arduino.h"
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

void setup() ;
void callback(char *topic, byte *payload, unsigned int length) ;
void IRAM_ATTR onTimer() ;
void loop() ;
void Menu_main(int menu_select) ;
int User_menu_input() ;
void func_verificar_tags() ;
void scan_tags_row(String search_string, String current_reading_array[]) ;
void existing_tags(String search_string) ;
void raw_registo(String present_tags[], String main, String state) ;
void registo(String main, String present_tags[], int count, String state) ;
String func_adicionar_novas_tags() ;
void ler_file_1() ;
void InserirBox_file_1() ;
void ler_file_2() ;
int ler_end_of_file_2() ;
void inserirBox_file_2(String current_reading) ;
String Inserir_tag_main_() ;
void Inserir_tag_nova_() ;
void put_on_line(String tag_nova, String tag_main) ;
void imprimir_file1() ;
void imprimir_file2() ;
void write_sd() ;

#include "Check_box.ino"


#endif
