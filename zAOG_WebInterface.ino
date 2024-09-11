// Wifi variables & definitions
char HTML_String[25000];//55000 25.3.: 19603 bytes
int action;
long temLong = 0;
double temDoub = 0;
int temInt = 0;

#define ACTION_LoadDefaultVal   1
#define ACTION_RESTART          2

//-------------------------------------------------------------------------------------------------
//7. Maerz 2021
// Mrz 23: main loop delay

void doWebinterface(void* pvParameters) {
    for (;;) {
        WiFi_Server.handleClient(); //does the Webinterface
        if (WebIOLastUsePlus3 < millis()) {//not called in the last 3 sec
            //Serial.println("Webinterface no client for 3 sec");
            bitClear(mainLoopDelay, 2);
            vTaskDelay(1000);
        }
        else {
            bitSet(mainLoopDelay, 2);//delay main loop for 4 ms to have time for WebIO
            vTaskDelay(20);
        }
        if ((now > WebIOTimeOut) && (Set.timeoutWebIO != 255)) {
            WebIORunning = false;
            WiFi_Server.close();
            Serial.println("closing Webinterface task");
            delay(1);
            vTaskDelete(NULL);
            delay(1);
        }
    }
}

//-------------------------------------------------------------------------------------------------
//7. Maerz 2021
// Mrz 2023 WebbIOlastUse

void handleRoot() {
    make_HTML01();
    WiFi_Server.sendHeader("Connection", "close");
    WiFi_Server.send(200, "text/html", HTML_String);
    WebIOLastUsePlus3 = 3000 + millis();
    WebIOTimeOut = WebIOLastUsePlus3 + long((Set.timeoutWebIO * 60000));
    if (Set.debugmode) {
        Serial.print("used size of HTML string: "); Serial.println(strlen(HTML_String));
        Serial.println("Webpage root");
        Serial.print("Timeout WebIO: "); Serial.println(WebIOTimeOut);
    }
    process_Request();
}

//-------------------------------------------------------------------------------------------------
//10. Mai 2020
// Mrz 2023 OTA changed

void WiFiStartServer() {

    WiFi_Server.on("/", HTTP_GET, []() {handleRoot(); });
    //file selection for firmware update
    WiFi_Server.on("/serverIndex", HTTP_GET, []() {
        WiFi_Server.sendHeader("Connection", "close");
        WiFi_Server.send(200, "text/html", serverIndex);
        });
    //handling uploading firmware file 
    WiFi_Server.on("/update", HTTP_POST, [&]() {
        Serial1.end(); //close serials to be sure, that F9P config is not mixed up
        delay(50);
        //Serial2.end();
        //delay(50);
        WiFi_Server.sendHeader("Connection", "close");
        WiFi_Server.send(200, "text/plain", (Update.hasError()) ? "FAIL" : "OK");
        ESP.restart();
        }, [&]() {
            HTTPUpload& upload = WiFi_Server.upload();
            if (upload.status == UPLOAD_FILE_START) {
                Serial.printf("Update: %s\n", upload.filename.c_str());
                if (!Update.begin(UPDATE_SIZE_UNKNOWN)) { //start with max available size
                    Update.printError(Serial);
                }
            }
            else if (upload.status == UPLOAD_FILE_WRITE) {
                /* flashing firmware to ESP*/
                if (Update.write(upload.buf, upload.currentSize) != upload.currentSize) {
                    Update.printError(Serial);
                }
            }
            else if (upload.status == UPLOAD_FILE_END) {
                if (Update.end(true)) { //true to set the size to the current progress
                    Serial.printf("Update Success: %u\nRebooting...\n", upload.totalSize);
                }
                else {
                    Update.printError(Serial);
                }
            }
        });
    WiFi_Server.onNotFound(handleNotFound);

    WiFi_Server.begin();
}

//---------------------------------------------------------------------
// Process given values 10. Mai 2020
//---------------------------------------------------------------------
// Mrz 2024 version for ESP access point - lot's of unused stuff at the end

void process_Request()
{
    action = 0;
    if (Set.debugmode) { Serial.print("From webinterface: number of arguments: "); Serial.println(WiFi_Server.args()); }
    for (byte n = 0; n < WiFi_Server.args(); n++) {
        if (Set.debugmode) {
            Serial.print("argName "); Serial.print(WiFi_Server.argName(n));
            Serial.print(" val: "); Serial.println(WiFi_Server.arg(n));
        }
        if (WiFi_Server.argName(n) == "ACTION") {
            action = int(WiFi_Server.arg(n).toInt());
            if (Set.debugmode) { Serial.print("Action found: "); Serial.println(action); }
        }
        if (action != ACTION_RESTART) { EEprom_unblock_restart(); }
        if (action == ACTION_LoadDefaultVal) {
            if (Set.debugmode) { Serial.println("load default settings from EEPROM"); }
            EEprom_read_default();
            delay(2);
        }
        //save changes
        if (WiFi_Server.argName(n) == "Save") {
            if (Set.debugmode) { Serial.println("Save button pressed in webinterface"); }
            EEprom_write_all();
        }
        if (WiFi_Server.argName(n) == "SSID_AP") {
            for (int i = 0; i < 24; i++) Set.ssid_ap[i] = 0x00;
            temInt = WiFi_Server.arg(n).length() + 1;
            WiFi_Server.arg(n).toCharArray(Set.ssid_ap, temInt);
        }
        if (WiFi_Server.argName(n) == "Password_AP") {
            for (int i = 0; i < 24; i++) Set.password_ap[i] = 0x00;
            temInt = WiFi_Server.arg(n).length() + 1;
            WiFi_Server.arg(n).toCharArray(Set.password_ap, temInt);
        }
        if (WiFi_Server.argName(n) == "timeoutWebIO") {
            temLong = WiFi_Server.arg(n).toInt();
            if ((temLong >= 2) && (temLong <= 255)) { Set.timeoutWebIO = byte(temLong); }
        }
        if (WiFi_Server.argName(n) == "WiFiGWIP0") {
            temInt = WiFi_Server.arg(n).toInt();
            Set.WiFi_gwip[0] = byte(temInt);
        }
        if (WiFi_Server.argName(n) == "WiFiGWIP1") {
            temInt = WiFi_Server.arg(n).toInt();
            Set.WiFi_gwip[1] = byte(temInt);
        }
        if (WiFi_Server.argName(n) == "WiFiGWIP2") {
            temInt = WiFi_Server.arg(n).toInt();
            Set.WiFi_gwip[2] = byte(temInt);
        }
        if (WiFi_Server.argName(n) == "WiFiGWIP3") {
            temInt = WiFi_Server.arg(n).toInt();
            Set.WiFi_gwip[3] = byte(temInt);
        }
        if (WiFi_Server.argName(n) == "WiFiIPDest") {
            temInt = WiFi_Server.arg(n).toInt();
            Set.WiFi_ipDest_ending = byte(temInt);
        }
        if (WiFi_Server.argName(n) == "Teensy2USB") {
            if (WiFi_Server.arg(n) == "true") { Set.bridgeTeensytoUSB = true; }
            else { Set.bridgeTeensytoUSB = false; }
        }
        if (WiFi_Server.argName(n) == "debugmode") {
            if (WiFi_Server.arg(n) == "true") { Set.debugmode = true; }
            else { Set.debugmode = false; }
        }

        if (WiFi_Server.argName(n) == "WiFiLEDon") {
            temLong = WiFi_Server.arg(n).toInt();
            if ((temLong >= 0) && (temLong <= 40)) {
                Set.LEDWiFi_ON_Level = byte(temLong);
                if (Set.LEDWiFi_PIN != 255) {
                    if (LED_WIFI_ON) { digitalWrite(Set.LEDWiFi_PIN, Set.LEDWiFi_ON_Level); }
                    else { digitalWrite(Set.LEDWiFi_PIN, !Set.LEDWiFi_ON_Level); }
                }
            }
        }
        if (action == ACTION_RESTART) {
            Serial.println("reboot ESP32: selected by webinterface");
            EEprom_block_restart();//prevents from restarting, when webpage is reloaded. Is set to 0, when other ACTION than restart is called
            delay(2000);
            ESP.restart();
        }
    }
}

//-------------------------------------------------------------------------------------------------
// create HTML site
//-------------------------------------------------------------------------------------------------
// Mrz 2024 version for ESP access point

void make_HTML01() {

    strcpy(HTML_String, "<!DOCTYPE html>");
    strcat(HTML_String, "<html>");
    strcat(HTML_String, "<head>");
    strcat(HTML_String, "<title>ESP32 access point to AgOpenGPS config page</title>");
    strcat(HTML_String, "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0;\" />\r\n");
    strcat(HTML_String, "<style>divbox {background-color: lightgrey;width: 200px;border: 5px solid red;padding:10px;margin: 10px;}</style>");
    strcat(HTML_String, "</head>");
    strcat(HTML_String, "<body bgcolor=\"#66b3ff\">");//ff9900 ffcc00
    strcat(HTML_String, "<font color=\"#000000\" face=\"VERDANA,ARIAL,HELVETICA\">");
    strcat(HTML_String, "<h1>ESP32 access point</h1>");
    strcat(HTML_String, "Connects WiFi modules to AgOpenGPS by forewarding data to Teensy and Ethernet cable to Win tablet.<br><br>");
    strcat(HTML_String, "More settings like UPD ports... in setup zone of INO code<br><br>Version: ");
    strcati(HTML_String, vers_nr);
    strcat(HTML_String, VersionTXT);
    strcat(HTML_String, "<br><br>Serial communication with Teensy at ");
    strcati(HTML_String, baudrates[Set.SerialToTeensyBaudrateNr]);
    strcat(HTML_String, " baud<br><hr>");

    strcat(HTML_String, "<h2>Modules connected  at the moment: ");
    strcati(HTML_String, WiFi.softAPgetStationNum());
    strcat(HTML_String, "</h2><hr>");


    //---------------------------------------------------------------------------------------------  
    //load values of INO setup zone
    strcat(HTML_String, "<h2>Load default values of INO setup zone</h2>");
    strcat(HTML_String, "<form>");
    strcat(HTML_String, "<table>");
    set_colgroup(270, 250, 150, 0, 0);

    strcat(HTML_String, "<tr>");
    strcat(HTML_String, "<td colspan=\"2\">Button only loads default values, does NOT save them</td>");
    strcat(HTML_String, "<td><input type= \"button\" onclick= \"sendVal('/?ACTION=");
    strcati(HTML_String, ACTION_LoadDefaultVal);
    strcat(HTML_String, "')\" style= \"width:150px\" value=\"Load default values\"></button></td>");
    strcat(HTML_String, "</tr>");
    strcat(HTML_String, "</table>");
    strcat(HTML_String, "</form>");
    strcat(HTML_String, "<br><hr>");

    //-----------------------------------------------------------------------------------------
    // WiFi Client Access Data

    strcat(HTML_String, "<h2>WiFi Network Data</h2>");
    strcat(HTML_String, "<b>WiFi modules can access AgOpenGPS using the settings below:<br><br></b>");
    strcat(HTML_String, "<table><form>");
    set_colgroup(250, 300, 150, 0, 0);

    strcat(HTML_String, "<tr><td><b>Access Point SSID: (default: AgOpenGPS_net)</b></td>");
    strcat(HTML_String, "<td><input type=\"text\" onchange=\"sendVal('/?SSID_AP='+this.value)\" style= \"width:200px\" name=\"SSID_AP\" maxlength=\"22\" Value =\"");
    strcat(HTML_String, Set.ssid_ap);
    strcat(HTML_String, "\"></td>");

    strcat(HTML_String, "<tr><td><b>Access Point Password: (default: no password)</b></td>");
    strcat(HTML_String, "<td>");
    strcat(HTML_String, "<input type=\"text\" onchange=\"sendVal('/?Password_AP='+this.value)\" style= \"width:200px\" name=\"Password_AP\" maxlength=\"22\" Value =\"");
    strcat(HTML_String, Set.password_ap);
    strcat(HTML_String, "\"></td></tr>");
    strcat(HTML_String, "<tr> <td colspan=\"3\"><b>IP for WebInterface in AP mode will be ");
    for (byte i = 0; i < 3; i++) {
        strcati(HTML_String, Set.WiFi_gwip[i]);
        strcat(HTML_String, ".");
    }
    strcati(HTML_String, Set.WiFi_gwip[3]);
    strcat(HTML_String,"</b></td>");
    strcat(HTML_String, "<tr> <td colspan=\"3\">&nbsp;</td> </tr>");

    strcat(HTML_String, "<tr><td colspan=\"2\"><b>Restart ESP32 access point</b></td>");
    strcat(HTML_String, "<td><input type= \"button\" onclick= \"sendVal('/?ACTION=");
    strcati(HTML_String, ACTION_RESTART);
    strcat(HTML_String, "')\" style= \"width:120px\" value=\"Restart\"></button></td>");
    strcat(HTML_String, "</tr>");

    strcat(HTML_String, "</table>");
    strcat(HTML_String, "</form>");
    strcat(HTML_String, "<br><hr>");

    //-----------------------------------------------------------------------------------------
    // timeout webinterface

    strcat(HTML_String, "<h2>Webinterface timeout</h2>");
    strcat(HTML_String, "<form>");
    strcat(HTML_String, "After this time (minutes) from restart or last usage, webinterface is turned off.<br><br>");
    strcat(HTML_String, "Set to 255 to keep active.<br><br><table>");
    set_colgroup(250, 300, 150, 0, 0);

    strcat(HTML_String, "<tr><td><b>Webinterface timeout (min)</b></td><td><input type = \"number\"  onchange=\"sendVal('/?timeoutWebIO='+this.value)\" name = \"timeoutWebIO\" min = \"2\" max = \"255\" step = \"1\" style= \"width:200px\" value = \"");// placeholder = \"");
    strcati(HTML_String, Set.timeoutWebIO);
    strcat(HTML_String, "\"></td>");

    strcat(HTML_String, "<td><input type= \"button\" onclick= \"sendVal('/?Save=true')\" style= \"width:120px\" value=\"Save\"></button></td>");
    strcat(HTML_String, "</tr>");


    strcat(HTML_String, "</table>");
    strcat(HTML_String, "</form>");
    strcat(HTML_String, "<br><hr>");


    //-----------------------------------------------------------------------------------------
    // display UPD packet data for debug

    strcat(HTML_String, "<h2>Last UPD packets:</h2>");
    strcat(HTML_String, "<form>");

    strcat(HTML_String, "<b>UPD packets from AgOpenGPS comming to Teensy and are forewarded to ESP WiFi: </b><br>");
    for (byte i = 0; i < BridgeArraySize; i++)
    {
        byte item = (FromTeensyDataRingCount + BridgeArraySize - i) % BridgeArraySize;
        for (byte digit = 0; digit < FromTeensyDataLength[item]; digit++) {
            strcati(HTML_String, FromTeensyData[item][digit]);
            strcat(HTML_String, " ");
        }
        strcat(HTML_String, "<br>");
    }

    strcat(HTML_String, "</form>");
    strcat(HTML_String, "<br><hr>");

    strcat(HTML_String, "<b>UPD packets from AgOpenGPS comming on port: ");
    strcati(HTML_String, Set.PortFromAOG);
    strcat(HTML_String, " via WiFi<br>should be empty, as AgOpenGPS is connected to Teensy via cable</b><br>");
    for (byte i = 0; i < BridgeArraySize; i++)
    {
        byte item = (BridgeData8888RingCountIn + BridgeArraySize - i) % BridgeArraySize;
        for (byte digit = 0; digit < BridgePacket8888Length[item]; digit++) {
            strcati(HTML_String, BridgeData8888[item][digit]);
            strcat(HTML_String, " ");
        }
        strcat(HTML_String, "<br>");
    }

    strcat(HTML_String, "</form>");
    strcat(HTML_String, "<br><hr>");


    strcat(HTML_String, "<form>");
    strcat(HTML_String, "<b>UPD packets from modules to AgOpenGPS comming on port: ");
    strcati(HTML_String, Set.PortDestination);
    strcat(HTML_String, "<br></b>");
    for (byte i = 0; i < BridgeArraySize; i++)
    {
        byte item = (BridgeData9999RingCountIn + BridgeArraySize - i) % BridgeArraySize;
        for (byte digit = 0; digit < BridgePacket9999Length[item]; digit++) {
            strcati(HTML_String, BridgeData9999[item][digit]);
            strcat(HTML_String, " ");
        }
        strcat(HTML_String, "<br>");
    }
    strcat(HTML_String, "</form>");
    strcat(HTML_String, "<br><hr>");

    //---------------------------------------------------------------------------------------------  
    // WiFi IP settings 

    strcat(HTML_String, "<h2>WiFi IP settings</h2>");
    strcat(HTML_String, "<form>");
    //IP
    strcat(HTML_String, "<b>IP address for WiFi</b>");
    strcat(HTML_String, "<b>Default is 192.168.137.1  It's also the address of the Webinterface.</b>");
    strcat(HTML_String, "<table>");
    set_colgroup(150, 50, 50, 50, 50);
    strcat(HTML_String, "<tr><td>IP address</td><td><input type = \"number\"  onchange=\"sendVal('/?WiFiGWIP0='+this.value)\" name = \"WiFiIP\" min = \"0\" max = \"255\" step = \"1\" style= \"width:40px\" value = \"");
    strcati(HTML_String, Set.WiFi_gwip[0]);
    strcat(HTML_String, "\"></td>");
    strcat(HTML_String, "<td><input type = \"number\"  onchange=\"sendVal('/?WiFiGWIP1='+this.value)\" name = \"WiFiIP\" min = \"0\" max = \"255\" step = \"1\" style= \"width:40px\" value = \"");
    strcati(HTML_String, Set.WiFi_gwip[1]);
    strcat(HTML_String, "\"></td>");
    strcat(HTML_String, "<td><input type = \"number\"  onchange=\"sendVal('/?WiFiGWIP2='+this.value)\" name = \"WiFiIP\" min = \"0\" max = \"255\" step = \"1\" style= \"width:40px\" value = \"");
    strcati(HTML_String, Set.WiFi_gwip[2]);
    strcat(HTML_String, "\"></td>");
    strcat(HTML_String, "<td><input type = \"number\"  onchange=\"sendVal('/?WiFiGWIP3='+this.value)\" name = \"WiFiIP\" min = \"0\" max = \"255\" step = \"1\" style= \"width:40px\" value = \"");
    strcati(HTML_String, Set.WiFi_gwip[3]);
    strcat(HTML_String, "\"></td></table><br>");
/*
    //IP destination    
    strcat(HTML_String, "<table>");
    set_colgroup(250, 300, 150, 0, 0);
    strcat(HTML_String, "<tr><td colspan=\"2\"><b>IP address of destination</b></td>");
    strcat(HTML_String, "<td><input type= \"button\" onclick= \"sendVal('/?Save=true')\" style= \"width:120px\" value=\"Save\"></button></td>");
    strcat(HTML_String, "</tr></table><table>");
    strcat(HTML_String, "Destination's IP address, the first 3 numbers are set as above.<br>");
    strcat(HTML_String, "<b>Use 255 to send to every device in network (default).</b> Use IP of your Computer, if you don't have a router and fixed IPs");
    strcat(HTML_String, "<table>");
    set_colgroup(150, 50, 50, 50, 50);
    strcat(HTML_String, "<tr><td>IP address destination</td><td>xxx</td><td>xxx</td><td>xxx<td><input type = \"number\"  onchange=\"sendVal('/?WiFiIPDest='+this.value)\" name = \"WiFiIPDest\" min = \"0\" max = \"255\" step = \"1\" style= \"width:40px\" value = \"");
    strcati(HTML_String, Set.WiFi_ipDest_ending);
    strcat(HTML_String, "\"></td></table><br><hr>");*/

    //---------------------------------------------------------------------------------------------
    // Teensy to USB brigde
    strcat(HTML_String, "<h2>USB to Teensy+WiFi and WiFi to USB bridge</h2>");
    strcat(HTML_String, "Activate only for debug use at ESP32 USB port.<br>All data will be send from USB to at ");
    strcati(HTML_String, baudrates[Set.BaudrateNrUSB]);
    strcat(HTML_String, " baud.<br><br>");
    strcat(HTML_String, "<form>");
    strcat(HTML_String, "<table>");
    set_colgroup(150, 400, 150, 0, 0);

    strcat(HTML_String, "<tr><td></td><td><input type=\"checkbox\" onclick=\"sendVal('/?Teensy2USB='+this.checked)\" name=\"Teensy2USB\" id = \"Part\" value = \"1\" ");
    if (Set.bridgeTeensytoUSB == 1) { strcat(HTML_String, "checked "); }
    strcat(HTML_String, "> ");
    strcat(HTML_String, "<label for =\"Part\"> bridge Wifi+Teensy to USB and vice versa</label></td>");
    strcat(HTML_String, "<td><input type= \"button\" onclick= \"sendVal('/?Save=true')\" style= \"width:120px\" value=\"Save\"></button></td>");

    strcat(HTML_String, "</tr></table>");
    strcat(HTML_String, "</form>");
    strcat(HTML_String, "<br><hr>");

    //-------------------------------------------------------------
    // Checkboxes debugmode
    strcat(HTML_String, "<h2>Debugmode</h2>");
    strcat(HTML_String, "<form>");
    strcat(HTML_String, "<table>");
    set_colgroup(150, 400, 150, 0, 0);

    strcat(HTML_String, "<tr> <td colspan=\"3\">debugmode sends messages to USB serial at ");
    strcati(HTML_String, baudrates[Set.BaudrateNrUSB]);
    strcat(HTML_String, " baud </td>");

    strcat(HTML_String, "<tr>");
    strcat(HTML_String, "<td></td><td><input type=\"checkbox\" onclick=\"sendVal('/?debugmode='+this.checked)\" name=\"debugmode\" id = \"Part\" value = \"1\" ");
    if (Set.debugmode == 1) strcat(HTML_String, "checked ");
    strcat(HTML_String, "> ");
    strcat(HTML_String, "<label for =\"Part\"> debugmode on</label></td>");
    strcat(HTML_String, "<td><input type= \"button\" onclick= \"sendVal('/?Save=true')\" style= \"width:120px\" value=\"Save\"></button></td></tr>");

    strcat(HTML_String, "</table>");
    strcat(HTML_String, "</form>");
    strcat(HTML_String, "<br><hr>");

    //-------------------------------------------------------------
    // firmware update
    strcat(HTML_String, "<h2>Firmware Update for ESP32</h2>");
    strcat(HTML_String, "<form>");
    strcat(HTML_String, "<table>");
    set_colgroup(300, 250, 150, 0, 0);

    strcat(HTML_String, "<tr> <td colspan=\"3\">build a new firmware with Arduino IDE selecting</td> </tr>");
    strcat(HTML_String, "<tr> <td colspan=\"3\">Sketch -> Export compiled Binary</td> </tr>");
    strcat(HTML_String, "<tr> <td colspan=\"3\">upload this file via WiFi/Ethernet connection</td> </tr>");

    strcat(HTML_String, "<tr> <td colspan=\"3\">&nbsp;</td> </tr>");
    strcat(HTML_String, "<tr><td></td>");
    //button
    strcat(HTML_String, "<td><input type='submit' onclick='openUpload(this.form)' value='Open Firmware uploader'></td></tr>");

    strcat(HTML_String, "<script>");
    strcat(HTML_String, "function openUpload(form)");
    strcat(HTML_String, "{");
    strcat(HTML_String, "window.open('/serverIndex')");
    strcat(HTML_String, "}");
    strcat(HTML_String, "</script>");

    strcat(HTML_String, "</table>");
    strcat(HTML_String, "</form>");
    strcat(HTML_String, "<br><hr>");

    //---------------------------------------------------------------------------------------------  
    // WiFi LED light on high/low 
    strcat(HTML_String, "<h2>WiFi/NTRIP LED light on</h2>");
    strcat(HTML_String, "<form>");
    strcat(HTML_String, "<table>");
    set_colgroup(150, 400, 150, 0, 0);

    strcat(HTML_String, "<tr>");
    strcat(HTML_String, "<td></td><td><input type = \"radio\" onclick=\"sendVal('/?WiFiLEDon=0')\" name=\"WiFiLEDon\" id=\"JZ\" value=\"0\"");
    if (Set.LEDWiFi_ON_Level == 0)strcat(HTML_String, " CHECKED");
    strcat(HTML_String, "><label for=\"JZ\">LOW</label></td>");
    strcat(HTML_String, "<td><input type= \"button\" onclick= \"sendVal('/?Save=true')\" style= \"width:120px\" value=\"Save\"></button></td>");
    strcat(HTML_String, "</tr>");

    strcat(HTML_String, "<tr>");
    strcat(HTML_String, "<td></td><td><input type = \"radio\" onclick=\"sendVal('/?WiFiLEDon=1')\" name=\"WiFiLEDon\" id=\"JZ\" value=\"1\"");
    if (Set.LEDWiFi_ON_Level == 1)strcat(HTML_String, " CHECKED");
    strcat(HTML_String, "><label for=\"JZ\">HIGH</label></td></tr>");

    strcat(HTML_String, "</table>");
    strcat(HTML_String, "</form>");
    strcat(HTML_String, "<br><hr>");

    //script to send values from webpage to ESP for process request
    strcat(HTML_String, "<script>");
    strcat(HTML_String, "function sendVal(ArgStr)");
    strcat(HTML_String, "{");
    strcat(HTML_String, "  var xhttp = new XMLHttpRequest();");
    strcat(HTML_String, "  xhttp.open(\"GET\",ArgStr, true);");
    strcat(HTML_String, "  xhttp.send();");
    strcat(HTML_String, " if (ArgStr == '/?ACTION=");
    strcati(HTML_String, ACTION_LoadDefaultVal);
    strcat(HTML_String, "') { window.setTimeout('location.reload()',400); }");
    strcat(HTML_String, "}");
    strcat(HTML_String, "</script>");

}


//-------------------------------------------------------------------------------------------------

void handleNotFound() {
    const char* notFound =
        "<!doctype html>"
        "<html lang = \"en\">"
        "<head>"""
        "<meta charset = \"utf - 8\">"
        "<meta http - equiv = \"x - ua - compatible\" content = \"ie = edge\">"
        "<meta name = \"viewport\" content = \"width = device - width, initial - scale = 1.0\">"
        "<title>Redirecting</title>"
        "</head>"
        "<body onload = \"redirect()\">"
        "<h1 style = \"text - align: center; padding - top: 50px; display: block; \"><br>404 not found<br><br>Redirecting to settings page in 3 secs ...</h1>"
        "<script>"
        "function redirect() {"
        "setTimeout(function() {"
        "    window.location.replace(\"/root\");"//new landing page
        "}"
        ", 5000);"
        "}"
        "</script>"
        "</body>"
        "</html>";

	WiFi_Server.sendHeader("Connection", "close");
	WiFi_Server.send(200, "text/html", notFound);
	if (Set.debugmode) { Serial.println("redirecting from 404 not found to Webpage root"); }

}

//-------------------------------------------------------------------------------------------------

void set_colgroup(int w1, int w2, int w3, int w4, int w5) {
    strcat(HTML_String, "<colgroup>");
    set_colgroup1(w1);
    set_colgroup1(w2);
    set_colgroup1(w3);
    set_colgroup1(w4);
    set_colgroup1(w5);
    strcat(HTML_String, "</colgroup>");
}

void set_colgroup(int w1, int w2, int w3, int w4, int w5, int w6) {
    strcat(HTML_String, "<colgroup>");
    set_colgroup1(w1);
    set_colgroup1(w2);
    set_colgroup1(w3);
    set_colgroup1(w4);
    set_colgroup1(w5);
    set_colgroup1(w6);
    strcat(HTML_String, "</colgroup>");
}
void set_colgroup(int w1, int w2, int w3, int w4, int w5, int w6, int w7) {
    strcat(HTML_String, "<colgroup>");
    set_colgroup1(w1);
    set_colgroup1(w2);
    set_colgroup1(w3);
    set_colgroup1(w4);
    set_colgroup1(w5);
    set_colgroup1(w6);
    set_colgroup1(w7);
    strcat(HTML_String, "</colgroup>");
}
//------------------------------------------------------------------------------------------
void set_colgroup1(int ww) {
    if (ww == 0) return;
    strcat(HTML_String, "<col width=\"");
    strcati(HTML_String, ww);
    strcat(HTML_String, "\">");
}
//---------------------------------------------------------------------
void strcatf(char* tx, float f, byte leng, byte dezim) {
    char tmp[8];

    dtostrf(f, leng, dezim, tmp);//f,6,2,tmp
    strcat(tx, tmp);
}
//---------------------------------------------------------------------
void strcati(char* tx, int i) {
    char tmp[8];

    itoa(i, tmp, 10);
    strcat(tx, tmp);
}
