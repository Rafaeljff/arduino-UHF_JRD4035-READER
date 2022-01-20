#include "Arduino.h"
#include "RFID_command.h"
#include <Ds1302.h>
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
const static char* WeekDays[] =
{
    "Monday",
    "Tuesday",
    "Wednesday",
    "Thursday",
    "Friday",
    "Saturday",
    "Sunday"
};
// DS1302 RTC instance


void setup() {
	Serial.begin(115200);

	Serial2.begin(115200, SERIAL_8N1, 16, 17); //16.17
	 rtc.init();
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
	Ds1302::DateTime now;
	//card = RFID.A_single_poll_of_instructions();
	cards=RFID.Multiple_polling_instructions(10);
	Cardinformation = RFID.NXP_Change_EAS(0x00000000);
     //Serial.println("RSSI :" + cards.card[1]._RSSI);
    // Serial.println("PC :" + cards.card[1]._PC);
     //Serial.println("EPC :" + cards.card[1]._EPC);
     ///kSerial.println(" ");

     for (size_t i = 0; i < cards.len; i++)
      {
       if(cards.card[i]._EPC.length() == 24)
          {

           Serial.println("RSSI :" + cards.card[i]._RSSI);//power level of the tag’s backscattered
           Serial.println("PC :" + cards.card[i]._PC);
           Serial.println("EPC :" + cards.card[i]._EPC);//Electronic Product Code / first writable memory bank.
           Serial.println("CRC :" + cards.card[i]._CRC);
           if(Cardinformation._UL.length() != 0)
           Serial.println("Card data :" +  Cardinformation._Data);
         if(i==cards.len){
        	 rtc.getDateTime(&now);
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
        	         Serial.print(':');

        	         Serial.print(now.second);  // 00-59
        	         Serial.println();



         }

           Serial.println(" ");


          }
      }



//read a single tag
	/*if(card._EPC.length() == 24){
     Serial.println("RSSI :" + card._RSSI);
     Serial.println("PC :" + card._PC);
     Serial.println("EPC :" + card._EPC);
     Serial.println("CRC :" + card._CRC);
     Serial.println("Card data :" +  Cardinformation._Data);
     Serial.println(" ");

   }

	 /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	       Used to get the SELECT parameter 用于获取Select参数
	       ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
	     //  Select = RFID.Get_the_select_parameter();
	     //  if(Select.Mask.length() != 0)
	     //  {
	     //    canvas.println("Mask :" + Select.Mask);
	     //    canvas.println("SelParam :" + Select.SelParam);
	     //    canvas.println("Ptr :" + Select.Ptr);
	     //    canvas.println("MaskLen :" + Select.MaskLen);
	     //    canvas.println("Truncate :" + Select.Truncate);
	     //    canvas.println(" ");
	     //  }
	     //    RFID.clean_data();

	     /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	       Used to change the PSF bit of the NXP G2X label 用于改变 NXP G2X 标签的 PSF 位
	       ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
	     //  Cardinformation = RFID.NXP_Change_EAS(0x00000000);
	     //  if(Cardinformation._UL.length() != 0)
	     //  {
	     //    canvas.println("UL :" + Cardinformation._UL);
	     //    canvas.println("PC :" + Cardinformation._PC);
	     //    canvas.println("EPC :" + Cardinformation._EPC);
	     //    canvas.println("Parameter :" + Cardinformation._Parameter);
	     //    canvas.println("ErrorCode :" + Cardinformation._ErrorCode);
	     //    canvas.println("Error :" + Cardinformation._Error);
	     //    canvas.println("Data :" + Cardinformation._Data);
	     //    canvas.println("Successful :" + Cardinformation._Successful);
	     //    canvas.println(" ");
	     //   }
	     //    RFID.clean_data();


	     /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	       Used to get the Query parameters 用于获取Query参数
	       ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
	     //  Query = RFID.Get_the_Query_parameter();
	     //  if(Query.QueryParameter.length() != 0)
	     //  {
	     //    canvas.println("QueryParameter :" + Query.QueryParameter);
	     //    canvas.println("DR :" + Query.DR);
	     //    canvas.println("M :" + Query.M);
	     //    canvas.println("TRext :" + Query.TRext);
	     //    canvas.println("Sel :" + Query.Sel);
	     //    canvas.println("Session :" + Query.Session);
	     //    canvas.println("Targetta :" + Query.Target);
	     //    canvas.println("Q :" + Query.Q);
	     //    canvas.println(" ");
	     //  }
	     //  RFID.clean_data();


	     /*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	       Used to get the Query parameters 用于读取接收解调器参数
	       ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
	     //  Read = RFID.Read_receive_demodulator_parameters();
	     //  if(Read.Mixer_G.length()!= 0)
	     //  {
	     //    canvas.println("Mixer_G :" + Read.Mixer_G);
	     //    canvas.println("IF_G :" + Read.IF_G);
	     //    canvas.println("Thrd :" + Read.Thrd);
	     //    canvas.println(" ");
	     //  }

	RFID.clean_data();

	delay(1000);




}

