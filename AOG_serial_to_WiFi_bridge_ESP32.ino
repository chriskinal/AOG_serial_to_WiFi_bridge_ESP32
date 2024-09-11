//ESP32 programm for use on XIOA ESP32 C3 on AgOpenGPS AllInOne PCB for WiFi access point function

//if using ESP32S3 set CPU frequency to 160MHz to reduce heat

byte vers_nr = 51;     //as long as version number isn't changed, the settings below will be kept in EEPROM, for minor chnages change only version text
char VersionTXT[150] = " - 25. Aug 2024 by MTZ8302 and others<br>WiFi to serial (Teensy) bridge, OTA)";

//code copied from everywere in the internet and combined by Matthias Hammer (MTZ8302) and others 2024

//change stettings to your need. Afterwards you can change them via webinterface x.x.x.1 (192.168.137.1)

//use serial monitor at USB port, to get debug messages and IP for webinterface at ESP start at 115200 baud.

//the settings below are written as defalt values and can be reloaded.
//So if changing settings set EEPROM_clear = true; (line ~78) - flash - boot - reset to EEPROM_clear = false - flash again to keep them as defauls

struct set {
    //connection plan:
    //  put in here description how ESP and Teensy are connected
    uint8_t ESP_RXSerialToTeensy = D7;// 44;//4;               // ESP RX pin connected to Teensy TX pin
    uint8_t ESP_TXSerialToTeensy = D6;//43;//5;               // ESP TX pin connected to Teensy RX pin (for ESP32 Pico D4 use 2, 5 doesn't work)
    uint8_t SerialToTeensyBaudrateNr = 6;           // 6 = 460 800 = recommended  
 
    uint8_t res_RX2 = 255;                              // not used at the moment
    uint8_t res_TX2 = 255;                              // not used at the moment
    uint8_t Serial2BaudrateNr = 4;                  // not used at the moment  -  4 = 115 200 = recommended  

    uint8_t Button_WiFi_rescan_PIN = 255;           // not used at the moment  -  Button to rescan/reconnect WiFi networks / push to GND

    uint8_t LEDWiFi_PIN = D8; //=D8                      // WiFi Status LED 255 = none/off
    uint8_t LEDWiFi_ON_Level = HIGH;                // HIGH = LED on high, LOW = LED on low

    uint8_t BaudrateNrUSB = 4;                      //4 = 115 200 = recommended - USB for debug only

    //Network---------------------------------------------------------------------------------------------
    //tractors WiFi or mobile hotspots
    char ssid1[24] = "";                         // not used at the moment  -   WiFi network Client name
    char password1[24] = "";                     // not used at the moment  -   WiFi network password//Accesspoint name and password
    char ssid2[24] = "";                         // not used at the moment  -   WiFi network Client name
    char password2[24] = "";                     // not used at the moment  -   WiFi network password//Accesspoint name and password

    char ssid_ap[24] = "Fendt_209V";//"AgOpenGPS_net";          // name of Access point. Default: AgOpenGPS_net
    char password_ap[24] = "";                   // password for Access point. Default: no password/none

    int timeoutRouterWiFi = 20;                  // not used at the moment  -   time (s) to search for existing WiFi, then starting Accesspoint 

    uint8_t timeoutWebIO = 255;                     // time (min) afterwards webinterface is switched off, 255 = always on

//WiFi
    //default: 192.168.137 = same as Windows hotspot, so other WiFi could be used with Win hotspot, if this hardware is not available
    uint8_t WiFi_myip[4] = { 192, 168, 137, 79 };   // not used at the moment  - ESP WiFi module. Default: 192, 168, 137, 79 
    uint8_t WiFi_gwip[4] = { 192, 168, 137, 1 };    // IP of Accesspoint. Default: 192, 168, 137, 1 
    uint8_t WiFi_ipDest_ending = 255;               // 255 = to all (default), don't change if you are not really sure what you are doing!!
    uint8_t mask[4] = { 255, 255, 255, 0 };
    uint8_t myDNS[4] = { 8, 8, 8, 8 };              //optional

    unsigned int PortModulToAOG = 5544;          // not used at the moment  -  this is port of this module: Autosteer = 5577 IMU = 5566 GPS = 
    unsigned int PortFromAOG = 8888;             // port to listen for AOG
    unsigned int AOGNtripPort = 2233;            // not used at the moment  -  port NTRIP data from AOG comes in
    unsigned int PortDestination = 9999;         // Port of AOG that listens

    uint8_t DataTransVia = 10;                      // not used at the moment  -  transfer data via 0 = USB / 7 = WiFi UDP / 10 = Ethernet UDP

    uint8_t WiFi_AOG_data_bridge = true;            // don't change, as purpose of this module: other modules can log into ESP32 WiFi and data is forewarded to AGO via Ethernet or USB
    bool bridgeTeensytoUSB = false;              // for debug use only

    uint8_t NtripClientBy = 1;                      // not used at the moment  -  NTRIP client 0:off /1: listens for AOG NTRIP to UDP (WiFi/Ethernet) or USB serial /2: use ESP32 WiFi NTIRP client
    uint8_t AgIOHeartbeat_answer = 1;		           // not used at the moment  -  0: don't answer 1: answer

    uint8_t reserve0 = 0;
    uint8_t reserve1 = 0;
    uint8_t reserve2 = 0;
    uint8_t reserve3 = 0;

    bool debugmode = false;

}; set Set;


bool EEPROM_clear = false;  //set to true when changing settings to write them as default values: true -> flash -> boot -> false -> flash again

// Baudrates for serial
unsigned long baudrates[]
{
  9600,//0
  19200,//1
  38400,//2
  57600,//3
  115200,//4
  230400,//5
  460800,//6
};
const uint32_t nrBaudrates = sizeof(baudrates) / sizeof(baudrates[0]);

/* not used at the moment
bool GPSBaudrateValid = false;

//AgIO communcation
uint8_t AgIO_msgHeader[] = { 128,129,127};
byte AgIO_heartbeat = 200;
byte AgIO_subnet_chng = 201;//subnet change from AgIO, only used with Ethernet, WiFi always DHCP
byte AgIO_ScanRequest = 202;//AgIO asking for IP -> Ethernet/WiFi GPS sends IP 
uint8_t helloFromIMU[] = { 128, 129, 121, 121, 5, 0, 0, 0, 0, 0, 71 }; //hello from IMU sent back
*/

// WiFistatus LED 
// blink times: searching WIFI: blinking 4x faster; connected: blinking as times set; data available: light on; no data for 10 seconds: blinking
unsigned int LED_WIFI_time = 0;
unsigned int LED_WIFI_pulse = 2000;   //light on in ms 
unsigned int LED_WIFI_pause = 1500;   //light off in ms
boolean LED_WIFI_ON = false;

//WIFI+Ethernet
unsigned long now = 0,  WebIOLastUsePlus3 = 0, WebIOTimeOut = 0, TeensyDataTime = 0, oneSecLoopTime = 0;
byte  my_WiFi_Mode = 0;   // WIFI_STA = 1 = Workstation  WIFI_AP = 2  = Accesspoint
IPAddress WiFi_ipDestination; //set in network.ino
bool  WebIORunning = false, WiFiUDPRunning = false, task_WiFiConnectRunning = false;
unsigned int packetLength = 0;
byte incomingBytes[254];

const byte BridgeArraySize = 10;
byte BridgeData9999[BridgeArraySize][254], BridgeData8888[BridgeArraySize][254], FromTeensyData[BridgeArraySize][254];
unsigned int BridgePacket9999Length[BridgeArraySize] = { 0,0,0,0,0,0,0,0,0,0 }, BridgePacket8888Length[BridgeArraySize] = { 0,0,0,0,0,0,0,0,0,0 }, FromTeensyDataLength[BridgeArraySize] = { 0,0,0,0,0,0,0,0,0,0 };
byte BridgeData9999RingCountIn = 0, BridgeData9999RingCountOut = 0, BridgeData8888RingCountIn = 0, BridgeData8888RingCountOut = 0, FromTeensyDataRingCount = 0;


#include <AsyncUDP.h>
#include <WiFiClient.h>
#include <WebServer.h>
#include <Update.h>
#include <WiFi.h>
#include <EEPROM.h>


//instances----------------------------------------------------------------------------------------

AsyncUDP WiFiUDPFromAOG;
AsyncUDP WiFiUDPToAOG;
AsyncUDP WiFiUDPBridgeToAOG;
// not used at the moment - AsyncUDP WiFi_udpNtrip;
WebServer WiFi_Server(80);

byte mainLoopDelay = 1;
TaskHandle_t taskHandle_WiFi_connect;
TaskHandle_t taskHandle_LEDBlink;
TaskHandle_t taskHandle_WebIO;


// SETUP ------------------------------------------------------------------------------------------

void setup()
{
    delay(250);//waiting for stable power

    //start serial USB (for debug)
    Serial.begin(baudrates[Set.BaudrateNrUSB]);
    delay(50);

    restoreEEprom();
    delay(150);

    //Teensy: serial 1
    Serial1.begin(baudrates[Set.SerialToTeensyBaudrateNr], SERIAL_8N1, Set.ESP_RXSerialToTeensy, Set.ESP_TXSerialToTeensy);
    delay(50);
    Serial.println();//new line

    if (Set.LEDWiFi_PIN != 255) { pinMode(Set.LEDWiFi_PIN, OUTPUT); }
    // not used at the moment - if (Set.Button_WiFi_rescan_PIN != 255) { pinMode(Set.Button_WiFi_rescan_PIN, INPUT_PULLUP); }



    Serial.print("ESP code Version Nr ");
    Serial.print(vers_nr);
    Serial.println(VersionTXT);
    Serial.println();

    Serial.print("Serial communication with Teensy at ");
    Serial.print(baudrates[Set.SerialToTeensyBaudrateNr]);
    Serial.println(" baud");
    Serial.println();

    //start WiFi
    xTaskCreate(WiFi_handle_connection, "WiFiConnectHandle", 3072, NULL, 1, &taskHandle_WiFi_connect);
    delay(500);

    //handle WiFi LED status
    xTaskCreate(WiFi_LED_blink, "WiFiLEDBlink", 3072, NULL, 0, &taskHandle_LEDBlink);
    delay(500);   
    
    vTaskDelay(5000); //waiting for other tasks
}

// MAIN loop  -------------------------------------------------------------------------------------------

void loop()
{
    now = millis();
    //WiFi UDP data handled via AsyncUDP: only called once, works with .onPacket, sends every time, data comes in

    //Teensy data?
    while (Serial1.available()) {
        packetLength = Serial1.available();
        Serial1.read(incomingBytes, packetLength);

        //pass data to USB for debug            
        if (Set.bridgeTeensytoUSB) {
            Serial.write(incomingBytes, packetLength);
            Serial.println();
        }

        //WiFi
        if (WiFiUDPRunning) {
            WiFiUDPFromAOG.writeTo(incomingBytes, packetLength, WiFi_ipDestination, Set.PortFromAOG);
        }
        TeensyDataTime = now;

        //put data in ring buffer to show in webinterface
        byte nextFromTeensyDataRingCount = (FromTeensyDataRingCount + 1) % BridgeArraySize;
        FromTeensyDataLength[nextFromTeensyDataRingCount] = packetLength;
        for (unsigned int i = 0; i < packetLength; i++) {
            FromTeensyData[nextFromTeensyDataRingCount][i] = incomingBytes[i];
        }
        FromTeensyDataRingCount = nextFromTeensyDataRingCount;
    }

    //USB data to Teensy only debug use
    if (Set.bridgeTeensytoUSB) {
        while (Serial.available()) {
            packetLength = Serial.available();
            packetLength = Serial.read(incomingBytes, packetLength);

            //USB -> Teensy
            Serial1.write(incomingBytes, packetLength);

            //USB - > WiFi
            if (WiFiUDPRunning) {
                WiFiUDPFromAOG.writeTo(incomingBytes, packetLength, WiFi_ipDestination, Set.PortFromAOG);
            }
        }
    }

    //WiFi to Teensy
    for (;;) {
        if (BridgeData9999RingCountOut == BridgeData9999RingCountIn) { break; }
    /*    for (byte b = 0; b < BridgePacket9999Length[BridgeData9999RingCountOut]; b++) {
            Serial1.write(BridgeData9999[BridgeData9999RingCountOut][b]);
        }*/
        Serial1.write(BridgeData9999[BridgeData9999RingCountOut], BridgePacket9999Length[BridgeData9999RingCountOut]);
        Serial1.println();
        BridgeData9999RingCountOut = (BridgeData9999RingCountOut + 1) % BridgeArraySize;
    }
    /* don't foreward data comming at WiFi 8888 to Teensy -> cable -> AGO as it will cause loop, if computer is connected via Ethernet and WiFi
    for (;;) {
        if (BridgeData8888RingCountOut == BridgeData8888RingCountIn) { break; }
        Serial1.write(BridgeData8888[BridgeData8888RingCountOut], BridgePacket8888Length[BridgeData8888RingCountOut]);
        BridgeData8888RingCountOut = (BridgeData8888RingCountOut + 1) % BridgeArraySize;
        Serial.println("bridge 8888 to Teensy");
    }*/

    vTaskDelay(mainLoopDelay);//2

    if (now > oneSecLoopTime) {
        oneSecLoopTime = now + 1000;

        vTaskDelay(2);//give more time for other tasks

        /*nothing usefull to do in this environment
        //check WiFi connection
        if ((Set.DataTransVia > 5) && (Set.DataTransVia < 10) && (WiFi_connect_step == 0)) {
            if (my_WiFi_Mode == WIFI_STA) {
                if (WiFi.status() != WL_CONNECTED) {
                    WiFi_STA_connect_call_nr = 0;//to enable DHCP for WiFi
                    WiFi_connect_step = 1;//ping
                    my_WiFi_Mode = 0;//set 0 to end Ntrip task
                    WiFiUDPRunning = false;
                    if (!task_WiFiConnectRunning) {
                        //start WiFi
                        xTaskCreate(WiFi_handle_connection, "WiFiConnectHandle", 3072, NULL, 1, &taskHandle_WiFi_connect);
                        delay(200);
                    }
                }
            }
            //WiFi rescann button pressed?
            if (Set.Button_WiFi_rescan_PIN != 255) {
                if (!digitalRead(Set.Button_WiFi_rescan_PIN)) {
                    Serial.println("WiFi rescan button pressed");
                    WiFi_STA_connect_call_nr = 0;//to enable DHCP for WiFi
                    WiFi_connect_step = 3;
                    my_WiFi_Mode = 0;//set 0 to end Ntrip task
                    WiFiUDPRunning = false;
                    if (!task_WiFiConnectRunning) {
                        //start WiFi
                        xTaskCreate(WiFi_handle_connection, "WiFiConnectHandle", 3072, NULL, 1, &taskHandle_WiFi_connect);
                        delay(500);
                    }
                }
            }
        }//WiFi reconnect*/
    }
}

//-------------------------------------------------------------------------------------------------
//9. Maerz 2021 25.8.2024 blink number of connected clients to WiFi AP

void WiFi_LED_blink(void* pvParameters)
{
    unsigned long now, saveTime = 0;
    byte blkSpeed;

    for (;;) {

        now = millis();
        if (!WiFiUDPRunning) { blkSpeed = 2; }//blink slowly, when WiFi AP is created
        else {
            if (now > (TeensyDataTime + 10000))//got Data via Teensy the last 10 sec?
            {
                blkSpeed = 0; // blink fast, if no data via Teensy for 10 sec
            }
            else {
                //all OK = LED on                
                blkSpeed = 255;
                LED_WIFI_ON = true;
                if (Set.LEDWiFi_PIN != 255) { digitalWrite(Set.LEDWiFi_PIN, Set.LEDWiFi_ON_Level); }
                vTaskDelay(4000);

                //blink numbers of connected WiFi clients
                byte WiFiClientsNr = WiFi.softAPgetStationNum();
                if (WiFiClientsNr) {
                    digitalWrite(Set.LEDWiFi_PIN, !Set.LEDWiFi_ON_Level);
                    vTaskDelay(1000);
                    for (byte b = 0; b < WiFiClientsNr; b++) {
                        if (Set.LEDWiFi_PIN != 255) {
                            digitalWrite(Set.LEDWiFi_PIN, Set.LEDWiFi_ON_Level);
                            vTaskDelay(300);
                            digitalWrite(Set.LEDWiFi_PIN, !Set.LEDWiFi_ON_Level);
                            vTaskDelay(300);
                        }
                    }
                    vTaskDelay(700);
                    digitalWrite(Set.LEDWiFi_PIN, Set.LEDWiFi_ON_Level);
                    vTaskDelay(500);
                }
            }
        }

        if (blkSpeed != 255) {
            if (!LED_WIFI_ON) {
                LED_WIFI_time = now;
                LED_WIFI_ON = true;
                if (Set.LEDWiFi_PIN != 255) { digitalWrite(Set.LEDWiFi_PIN, Set.LEDWiFi_ON_Level); }

                vTaskDelay(LED_WIFI_pause >> blkSpeed);

            }
            if (LED_WIFI_ON) {
                LED_WIFI_time = now;
                LED_WIFI_ON = false;
                if (Set.LEDWiFi_PIN != 255) { digitalWrite(Set.LEDWiFi_PIN, !Set.LEDWiFi_ON_Level); }

                vTaskDelay(LED_WIFI_pulse >> blkSpeed);
            }
        }
    }
}
