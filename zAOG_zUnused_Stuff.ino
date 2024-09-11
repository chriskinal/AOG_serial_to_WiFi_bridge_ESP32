//not deleted, as maybe usefull to detect Teensy baudrate

/*
// Mrz 2023 stolen from Jaaps code

//checks ESP32 Serial1 for UBlox F9P chip: sends UBX request at different baudrates and wait for answer
void GPSGetBaudrate() {
    //UBX packet to send and ich GPS answers
    byte mon_rate[] = { 0xB5, 0x62, 0x06, 0x08, 0x00, 0x00, 0x00, 0x00 };
    byte mon_rate_size = sizeof(mon_rate);
    //add checksum
    for (int i = 2; i < (mon_rate_size - 2); i++) {
        mon_rate[mon_rate_size - 2] += ((unsigned char*)(&mon_rate))[i];
        mon_rate[mon_rate_size - 1] += mon_rate[mon_rate_size - 2];
    }

    // Check baudrate
    bool communicationSuccessfull = false, checkActualBaudrate = true;
    unsigned long baudrate = 0;

    Serial.println("Checking for GPS serial baudrate.");

    for (byte i = 0; i < nrBaudrates; i++)
    {
        if (checkActualBaudrate) {//first run, baudrate allready set is checked, then start with first in array
            baudrate = baudrates[Set.SerialToTeensyBaudrateNr];

            Serial.print(F("Checking baudrate: "));
            Serial.println(baudrate);
        }
        else {
            baudrate = baudrates[i];

            Serial.print(F("Checking baudrate: "));
            Serial.println(baudrate);

            //Serial1.begin(Set.GPS_baudrate, SERIAL_8N1, Set.ESP_RXSerialToTeensy, Set.ESP_TXSerialToTeensy);
            Serial1.updateBaudRate(baudrate);
            delay(20);
            vTaskDelay(50);
        }

        // first send dumb data to make sure its on
        Serial1.write(0xFF);

        // Clear
        while (Serial1.available() > 0)
        {
            Serial1.read();
        }

        // Send request
        Serial1.write(mon_rate, 8);

        unsigned long millis_read = millis();
        const unsigned long UART_TIMEOUT = 1000;
        int ubxFrameCounter = 0;
        bool isUbx = false;
        uint8_t incoming = 0;

        uint8_t requestedClass = 6;// packetCfg.cls;
        uint8_t requestedID = 8;// packetCfg.id;

        uint8_t packetBufCls = 0;
        uint8_t packetBufId = 0;

        do
        {
            while (Serial1.available() > 0)
            {
                incoming = Serial1.read();

                //Serial.print(incoming); Serial.print(" ");

                if (!isUbx && incoming == 0xB5) // UBX binary frames start with 0xB5, aka ?
                {
                    ubxFrameCounter = 0;
                    isUbx = true;
                }

                if (isUbx)
                {
                    // Decide what type of response this is
                    if ((ubxFrameCounter == 0) && (incoming != 0xB5))      // ISO micro
                    {
                        isUbx = false;                                            // Something went wrong. Reset.
                    }
                    else if ((ubxFrameCounter == 1) && (incoming != 0x62)) // ASCII 'b'
                    {
                        isUbx = false;                                            // Something went wrong. Reset.
                    }
                    else if (ubxFrameCounter == 1 && incoming == 0x62)
                    {
                        // Serial.println("UBX_SYNCH_2");
                        // isUbx should be still true
                    }
                    else if (ubxFrameCounter == 2) // Class
                    {
                        // Record the class in packetBuf until we know what to do with it
                        packetBufCls = incoming; // (Duplication)
                    }
                    else if (ubxFrameCounter == 3) // ID
                    {
                        // Record the ID in packetBuf until we know what to do with it
                        packetBufId = incoming; // (Duplication)

                        // We can now identify the type of response
                        // If the packet we are receiving is not an ACK then check for a class and ID match
                        if (packetBufCls != 0x05)
                        {
                            // This is not an ACK so check for a class and ID match
                            if ((packetBufCls == requestedClass) && (packetBufId == requestedID))
                            {
                                // This is not an ACK and we have a class and ID match
                                communicationSuccessfull = true;
                                //
                                if (!checkActualBaudrate) { Set.SerialToTeensyBaudrateNr = i; }
                            }
                            else
                            {
                                // This is not an ACK and we do not have a class and ID match
                                // so we should keep diverting data into packetBuf and ignore the payload
                                isUbx = false;
                            }
                        }
                    }
                }

                // Finally, increment the frame counter
                ubxFrameCounter++;
            }
        } while (millis() - millis_read < UART_TIMEOUT);

        if (communicationSuccessfull)
        {
            break;
        }
        if (checkActualBaudrate) { checkActualBaudrate = false; i = 0; }//first run, baudrate allready set was checked, then start with first in array
    }

    if (communicationSuccessfull)
    {
        Serial.println();
        Serial.print(F("Found u-blox GPS reciever at baudrate: "));
        Serial.println(baudrate);
        GPSBaudrateValid = true;
    }
    else {
        Serial.println("u-blox GNSS not detected. Please check wiring.");
        GPSBaudrateValid = false;
    }
    vTaskDelay(5);
}

*/




/*  Webinterface: process request

        if (WiFi_Server.argName(n) == "BaudrTeensy") {
            temLong = WiFi_Server.arg(n).toInt();
            if ((temLong <= 20) && (temLong >= 0)) {
                Set.SerialToTeensyBaudrateNr = byte(temLong);
                Serial1.updateBaudRate(baudrates[Set.SerialToTeensyBaudrateNr]);
                delay(50);
            }
        }
        if (WiFi_Server.argName(n) == "BaudrUSB") {
            temLong = WiFi_Server.arg(n).toInt();
            if ((temLong <= 20) && (temLong >= 0)) {
                Set.BaudrateNrUSB = byte(temLong);
                Serial.updateBaudRate(baudrates[Set.BaudrateNrUSB]);
                delay(50);
            }
        }



if (WiFi_Server.argName(n) == "AgIOHeartBeat") {
            if (WiFi_Server.arg(n) == "true") { Set.AgIOHeartbeat_answer = 1; }
            else { Set.AgIOHeartbeat_answer = 0; }
        }


                /*PIN set not used at the moment
        if (WiFi_Server.argName(n) == "ESP_TXSerialToTeensy") {
            temLong = WiFi_Server.arg(n).toInt();
            if ((temLong >= 0) && (temLong <= 40)) {
                Set.ESP_TXSerialToTeensy = byte(temLong);
                Serial1.end();
                delay(20);
                Serial1.begin(115200, SERIAL_8N1, Set.ESP_RXSerialToTeensy, Set.ESP_TXSerialToTeensy);
                delay(5);
            }
        }
        if (WiFi_Server.argName(n) == "ESP_RXSerialToTeensy") {
            temLong = WiFi_Server.arg(n).toInt();
            if ((temLong >= 0) && (temLong <= 40)) {
                Set.ESP_RXSerialToTeensy = byte(temLong);
                Serial1.end();
                delay(20);
                Serial1.begin(115200, SERIAL_8N1, Set.ESP_RXSerialToTeensy, Set.ESP_TXSerialToTeensy);
                delay(5);
            }
        }
        if (WiFi_Server.argName(n) == "TX2") {
            temLong = WiFi_Server.arg(n).toInt();
            if ((temLong >= 0) && (temLong <= 40)) {
                Set.TX2 = byte(temLong);
                Serial2.end();
                delay(20);
                Serial2.begin(115200, SERIAL_8N1, Set.RX2, Set.TX2);
                delay(5);
            }
        }
        if (WiFi_Server.argName(n) == "RX2") {
            temLong = WiFi_Server.arg(n).toInt();
            if ((temLong >= 0) && (temLong <= 40)) {
                Set.RX2 = byte(temLong);
                Serial2.end();
                delay(20);
                Serial2.begin(115200, SERIAL_8N1, Set.RX2, Set.TX2);
                delay(5);
            }
        }

        if (WiFi_Server.argName(n) == "WiFiResc") {
            temLong = WiFi_Server.arg(n).toInt();
            if ((temLong >= 0) && (temLong <= 40)) {
                Set.LEDWiFi_PIN = byte(temLong);
                pinMode(Set.Button_WiFi_rescan_PIN, INPUT_PULLUP);
            }
        }

               if (WiFi_Server.argName(n) == "GPSGetBaud") {
            if (Set.debugmode) { Serial.println("Get GPS serial baudrate pressed in webinterface"); }
            GPSGetBaudrate();
        }

        if (WiFi_Server.argName(n) == "timeoutRout") {
            temLong = WiFi_Server.arg(n).toInt();
            if ((temLong >= 20) && (temLong <= 1000)) { Set.timeoutRouterWiFi = int(temLong); }
        }
        if (WiFi_Server.argName(n) == "SSID_MY1") {
            for (int i = 0; i < 24; i++) Set.ssid1[i] = 0x00;
            temInt = WiFi_Server.arg(n).length() + 1;
            WiFi_Server.arg(n).toCharArray(Set.ssid1, temInt);
        }
        if (WiFi_Server.argName(n) == "Password_MY1") {
            for (int i = 0; i < 24; i++) Set.password1[i] = 0x00;
            temInt = WiFi_Server.arg(n).length() + 1;
            WiFi_Server.arg(n).toCharArray(Set.password1, temInt);
        }
        if (WiFi_Server.argName(n) == "SSID_MY2") {
            for (int i = 0; i < 24; i++) Set.ssid2[i] = 0x00;
            temInt = WiFi_Server.arg(n).length() + 1;
            WiFi_Server.arg(n).toCharArray(Set.ssid2, temInt);
        }
        if (WiFi_Server.argName(n) == "Password_MY2") {
            for (int i = 0; i < 24; i++) Set.password2[i] = 0x00;
            temInt = WiFi_Server.arg(n).length() + 1;
            WiFi_Server.arg(n).toCharArray(Set.password2, temInt);
        }
        if (WiFi_Server.argName(n) == "SSID_MY3") {
            for (int i = 0; i < 24; i++) Set.ssid3[i] = 0x00;
            temInt = WiFi_Server.arg(n).length() + 1;
            WiFi_Server.arg(n).toCharArray(Set.ssid3, temInt);
        }
        if (WiFi_Server.argName(n) == "Password_MY3") {
            for (int i = 0; i < 24; i++) Set.password3[i] = 0x00;
            temInt = WiFi_Server.arg(n).length() + 1;
            WiFi_Server.arg(n).toCharArray(Set.password3, temInt);
        }
        if (WiFi_Server.argName(n) == "SSID_MY4") {
            for (int i = 0; i < 24; i++) Set.ssid4[i] = 0x00;
            temInt = WiFi_Server.arg(n).length() + 1;
            WiFi_Server.arg(n).toCharArray(Set.ssid4, temInt);
        }
        if (WiFi_Server.argName(n) == "Password_MY4") {
            for (int i = 0; i < 24; i++) Set.password4[i] = 0x00;
            temInt = WiFi_Server.arg(n).length() + 1;
            WiFi_Server.arg(n).toCharArray(Set.password4, temInt);
        }
        if (WiFi_Server.argName(n) == "SSID_MY5") {
            for (int i = 0; i < 24; i++) Set.ssid5[i] = 0x00;
            temInt = WiFi_Server.arg(n).length() + 1;
            WiFi_Server.arg(n).toCharArray(Set.ssid5, temInt);
        }
        if (WiFi_Server.argName(n) == "Password_MY5") {
            for (int i = 0; i < 24; i++) Set.password5[i] = 0x00;
            temInt = WiFi_Server.arg(n).length() + 1;
            WiFi_Server.arg(n).toCharArray(Set.password5, temInt);
        }

        if (WiFi_Server.argName(n) == "TX2") {
            temLong = WiFi_Server.arg(n).toInt();
            if ((temLong >= 0) && (temLong <= 40)) {
                Set.TX2 = byte(temLong);
                Serial2.end();
                delay(20);
                Serial2.begin(115200, SERIAL_8N1, Set.RX2, Set.TX2);
                delay(5);
            }
        }
        if (WiFi_Server.argName(n) == "RX2") {
            temLong = WiFi_Server.arg(n).toInt();
            if ((temLong >= 0) && (temLong <= 40)) {
                Set.RX2 = byte(temLong);
                Serial2.end();
                delay(20);
                Serial2.begin(115200, SERIAL_8N1, Set.RX2, Set.TX2);
                delay(5);
            }
        }
        if (WiFi_Server.argName(n) == "WiFiResc") {
            temLong = WiFi_Server.arg(n).toInt();
            if ((temLong >= 0) && (temLong <= 40)) {
                Set.LEDWiFi_PIN = byte(temLong);
                pinMode(Set.Button_WiFi_rescan_PIN, INPUT_PULLUP);
            }
        }
              if (WiFi_Server.argName(n) == "timeoutRoutEth") {
            temLong = WiFi_Server.arg(n).toInt();
            if ((temLong >= 20) && (temLong <= 1000)) { Set.timeoutRouterEth = int(temLong); }
        }
        if (WiFi_Server.argName(n) == "EthIP0") {
            temInt = WiFi_Server.arg(n).toInt();
            Set.Eth_myip[0] = byte(temInt);
        }
        if (WiFi_Server.argName(n) == "EthIP1") {
            temInt = WiFi_Server.arg(n).toInt();
            Set.Eth_myip[1] = byte(temInt);
        }
        if (WiFi_Server.argName(n) == "EthIP2") {
            temInt = WiFi_Server.arg(n).toInt();
            Set.Eth_myip[2] = byte(temInt);
        }
        if (WiFi_Server.argName(n) == "EthIP3") {
            temInt = WiFi_Server.arg(n).toInt();
            Set.Eth_myip[3] = byte(temInt);
        }
        if (WiFi_Server.argName(n) == "EthStatIP") {
            temInt = WiFi_Server.arg(n).toInt();
            if (temInt == 1) { Set.Eth_static_IP = true; }
            else { Set.Eth_static_IP = false; }
        }
        if (WiFi_Server.argName(n) == "EthIPDest") {
            temInt = WiFi_Server.arg(n).toInt();
            Set.Eth_ipDest_ending = byte(temInt);
        }
        if (WiFi_Server.argName(n) == "EthMac0") {
            temInt = WiFi_Server.arg(n).toInt();
            Set.Eth_mac[0] = byte(temInt);
        }
        if (WiFi_Server.argName(n) == "EthMac1") {
            temInt = WiFi_Server.arg(n).toInt();
            Set.Eth_mac[1] = byte(temInt);
        }
        if (WiFi_Server.argName(n) == "EthMac2") {
            temInt = WiFi_Server.arg(n).toInt();
            Set.Eth_mac[2] = byte(temInt);
        }
        if (WiFi_Server.argName(n) == "EthMac3") {
            temInt = WiFi_Server.arg(n).toInt();
            Set.Eth_mac[3] = byte(temInt);
        }
        if (WiFi_Server.argName(n) == "EthMac4") {
            temInt = WiFi_Server.arg(n).toInt();
            Set.Eth_mac[4] = byte(temInt);
        }
        if (WiFi_Server.argName(n) == "EthMac5") {
            temInt = WiFi_Server.arg(n).toInt();
            Set.Eth_mac[5] = byte(temInt);
        }



                /*          if (WiFi_Server.argName(n) == "debugmUBX") {
                      if (WiFi_Server.arg(n) == "true") { Set.debugmodeUBX = true; }
                      else { Set.debugmodeUBX = false; }
                  }
                  if (WiFi_Server.argName(n) == "debugmHead") {
                      if (WiFi_Server.arg(n) == "true") { Set.debugmodeHeading = true; }
                      else { Set.debugmodeHeading = false; }
                  }
                  if (WiFi_Server.argName(n) == "debugmVirtAnt") {
                      if (WiFi_Server.arg(n) == "true") { Set.debugmodeVirtAnt = true; }
                      else { Set.debugmodeVirtAnt = false; }
                  }
                  if (WiFi_Server.argName(n) == "debugmFiltPos") {
                      if (WiFi_Server.arg(n) == "true") { Set.debugmodeFilterPos = true; }
                      else { Set.debugmodeFilterPos = false; }
                  }

                  if (WiFi_Server.argName(n) == "debugmNtrip") {
                      if (WiFi_Server.arg(n) == "true") { Set.debugmodeNTRIP = true; }
                      else { Set.debugmodeNTRIP = false; }
                  }
                  if (WiFi_Server.argName(n) == "debugmRAW") {
                      if (WiFi_Server.arg(n) == "true") { Set.debugmodeRAW = true; }
                      else { Set.debugmodeRAW = false; }
                  }
       */




/* Webinterface: make HTML string




    strcat(HTML_String, "<tr><td><b>#1 Network SSID:</b></td>");
    strcat(HTML_String, "<td><input type=\"text\" onchange=\"sendVal('/?SSID_MY1='+this.value)\" style= \"width:200px\" name=\"SSID_MY1\" maxlength=\"22\" Value =\"");
    strcat(HTML_String, Set.ssid1);
    strcat(HTML_String, "\"></td>");
    strcat(HTML_String, "<td><input type= \"button\" onclick= \"sendVal('/?Save=true')\" style= \"width:120px\" value=\"Save\"></button></td></tr>");

    strcat(HTML_String, "<tr><td><b>#1 Password:</b></td>");
    strcat(HTML_String, "<td>");
    strcat(HTML_String, "<input type=\"text\" onchange=\"sendVal('/?Password_MY1='+this.value)\" style= \"width:200px\" name=\"Password_MY1\" maxlength=\"22\" Value =\"");
    strcat(HTML_String, Set.password1);
    strcat(HTML_String, "\"></td></tr>");

    strcat(HTML_String, "<tr> <td colspan=\"3\">&nbsp;</td> </tr>");

    strcat(HTML_String, "<tr><td><b>#2 Network SSID:</b></td>");
    strcat(HTML_String, "<td><input type=\"text\" onchange=\"sendVal('/?SSID_MY2='+this.value)\" style= \"width:200px\" name=\"SSID_MY2\" maxlength=\"22\" Value =\"");
    strcat(HTML_String, Set.ssid2);
    strcat(HTML_String, "\"></td>");

    strcat(HTML_String, "<tr><td><b>#2 Password:</b></td>");
    strcat(HTML_String, "<td>");
    strcat(HTML_String, "<input type=\"text\" onchange=\"sendVal('/?Password_MY2='+this.value)\" style= \"width:200px\" name=\"Password_MY2\" maxlength=\"22\" Value =\"");
    strcat(HTML_String, Set.password2);
    strcat(HTML_String, "\"></td></tr>");

    strcat(HTML_String, "<tr> <td colspan=\"3\">&nbsp;</td> </tr>");

    strcat(HTML_String, "<tr><td><b>#3 Network SSID:</b></td>");
    strcat(HTML_String, "<td><input type=\"text\" onchange=\"sendVal('/?SSID_MY3='+this.value)\" style= \"width:200px\" name=\"SSID_MY3\" maxlength=\"22\" Value =\"");
    strcat(HTML_String, Set.ssid3);
    strcat(HTML_String, "\"></td>");

    strcat(HTML_String, "<tr><td><b>#3 Password:</b></td>");
    strcat(HTML_String, "<td>");
    strcat(HTML_String, "<input type=\"text\" onchange=\"sendVal('/?Password_MY3='+this.value)\" style= \"width:200px\" name=\"Password_MY3\" maxlength=\"22\" Value =\"");
    strcat(HTML_String, Set.password3);
    strcat(HTML_String, "\"></td></tr>");

    strcat(HTML_String, "<tr> <td colspan=\"3\">&nbsp;</td> </tr>");

    strcat(HTML_String, "<tr><td><b>#4 Network SSID:</b></td>");
    strcat(HTML_String, "<td><input type=\"text\" onchange=\"sendVal('/?SSID_MY4='+this.value)\" style= \"width:200px\" name=\"SSID_MY4\" maxlength=\"22\" Value =\"");
    strcat(HTML_String, Set.ssid4);
    strcat(HTML_String, "\"></td>");

    strcat(HTML_String, "<tr><td><b>#4 Password:</b></td>");
    strcat(HTML_String, "<td>");
    strcat(HTML_String, "<input type=\"text\" onchange=\"sendVal('/?Password_MY4='+this.value)\" style= \"width:200px\" name=\"Password_MY4\" maxlength=\"22\" Value =\"");
    strcat(HTML_String, Set.password4);
    strcat(HTML_String, "\"></td></tr>");

    strcat(HTML_String, "<tr> <td colspan=\"3\">&nbsp;</td> </tr>");

    strcat(HTML_String, "<tr><td><b>#5 Network SSID:</b></td>");
    strcat(HTML_String, "<td><input type=\"text\" onchange=\"sendVal('/?SSID_MY5='+this.value)\" style= \"width:200px\" name=\"SSID_MY5\" maxlength=\"22\" Value =\"");
    strcat(HTML_String, Set.ssid5);
    strcat(HTML_String, "\"></td>");

    strcat(HTML_String, "<tr><td><b>#5 Password:</b></td>");
    strcat(HTML_String, "<td>");
    strcat(HTML_String, "<input type=\"text\" onchange=\"sendVal('/?Password_MY5='+this.value)\" style= \"width:200px\" name=\"Password_MY5\" maxlength=\"22\" Value =\"");
    strcat(HTML_String, Set.password5);
    strcat(HTML_String, "\"></td></tr>");

    strcat(HTML_String, "<tr> <td colspan=\"3\">&nbsp;</td> </tr>");

    strcat(HTML_String, "<tr>");
    strcat(HTML_String, "<td colspan=\"3\">time, trying to connect to networks from list above</td></tr>");
    strcat(HTML_String, "<td colspan=\"3\">after time has passed, access point is opened (SSID and password below</td></tr>");
    strcat(HTML_String, "<tr><td><b>Timeout (s):</b></td><td><input type = \"number\" onchange=\"sendVal('/?timeoutRout='+this.value)\" name = \"timeoutRout\" min = \"20\" max = \"1000\" step = \"1\" style= \"width:200px\" value = \"");// placeholder = \"");
    strcati(HTML_String, Set.timeoutRouterWiFi);
    strcat(HTML_String, "\"></td>");
    strcat(HTML_String, "</tr>");

    strcat(HTML_String, "<tr> <td colspan=\"3\">&nbsp;</td> </tr>");


      //---------------------------------------------------------------------------------------------
    // Data transfer via USB/Wifi
    strcat(HTML_String, "<h2>USB, WiFi or Ethernet data transfer</h2>");
    strcat(HTML_String, "<form>");
    strcat(HTML_String, "<table>");
    set_colgroup(150, 400, 150, 0, 0);

    //transfer data via 0 = USB / 7 = WiFi UDP / 10 = Ethernet
    strcat(HTML_String, "<tr>");
    strcat(HTML_String, "<td></td><td><input type = \"radio\" onclick=\"sendVal('/?DataTransfVia=0')\" name=\"DataTransfVia\" id=\"JZ\" value=\"0\"");
    if (Set.DataTransVia == 0)strcat(HTML_String, " CHECKED");
    strcat(HTML_String, "><label for=\"JZ\">USB at ");
    strcati(HTML_String, baudrates[Set.SerialToTeensyBaudrateNr]);
    strcat(HTML_String, " baud</label></td>");
    strcat(HTML_String, "<td><input type= \"button\" onclick= \"sendVal('/?Save=true')\" style= \"width:120px\" value=\"Save\"></button></td>");
    strcat(HTML_String, "</tr>");
    strcat(HTML_String, "><label for=\"JZ\">USB + mobile Hotspot for NTRIP</label></td></tr>");

    strcat(HTML_String, "<tr>");
    strcat(HTML_String, "<td></td><td><input type = \"radio\" onclick=\"sendVal('/?DataTransfVia=7')\" name=\"DataTransfVia\" id=\"JZ\" value=\"7\"");
    if (Set.DataTransVia == 7)strcat(HTML_String, " CHECKED");
    strcat(HTML_String, "><label for=\"JZ\">WiFi (UDP) (default)</label></td></tr>");

    strcat(HTML_String, "<tr>");
    strcat(HTML_String, "<td></td><td colspan=\"2\"><input type = \"radio\" onclick=\"sendVal('/?DataTransfVia=10')\" name=\"DataTransfVia\" id=\"JZ\" value=\"10\"");
    if (Set.DataTransVia == 10)strcat(HTML_String, " CHECKED");
    strcat(HTML_String, "><label for=\"JZ\">Ethernet (UDP) Ethernet hardware needed!!</label></td></tr>");

    strcat(HTML_String, "</table>");
    strcat(HTML_String, "</form>");
    strcat(HTML_String, "<br><hr>");

/*
    //---------------------------------------------------------------------------------------------
    // NTRIP

    strcat(HTML_String, "<h2>NTRIP client</h2>");
    strcat(HTML_String, "<form>");
    strcat(HTML_String, "<table>");
    set_colgroup(150, 400, 150, 0, 0);

    //transfer data via 0 = OFF / 1 = AOG NTRIP / 2 = ESP32 NTRIP client
    strcat(HTML_String, "<tr>");
    strcat(HTML_String, "<td></td><td><input type = \"radio\" onclick=\"sendVal('/?NtripClientBy=0')\" name=\"NtripClientBy\" id=\"JZ\" value=\"0\"");
    if (Set.NtripClientBy == 0)strcat(HTML_String, " CHECKED");
    strcat(HTML_String, "><label for=\"JZ\">OFF</label></td>");
    strcat(HTML_String, "<td><input type= \"button\" onclick= \"sendVal('/?Save=true')\" style= \"width:120px\" value=\"Save\"></button></td>");
    strcat(HTML_String, "</tr>");

    strcat(HTML_String, "<tr>");
    strcat(HTML_String, "<td></td><td colspan=\"2\"><input type = \"radio\" onclick=\"sendVal('/?NtripClientBy=1')\" name=\"NtripClientBy\" id=\"JZ\" value=\"1\"");
    if (Set.NtripClientBy == 1)strcat(HTML_String, " CHECKED");
    strcat(HTML_String, "><label for=\"JZ\">AOG NTRIP client (for WiFi/Ethernet UDP set port 2233 in AOG)</label></td></tr>");

    strcat(HTML_String, "<tr>");
    strcat(HTML_String, "<td></td><td colspan=\"2\"><input type = \"radio\" onclick=\"sendVal('/?NtripClientBy=2')\" name=\"NtripClientBy\" id=\"JZ\" value=\"2\"");
    if (Set.NtripClientBy == 2)strcat(HTML_String, " CHECKED");
    strcat(HTML_String, "><label for=\"JZ\">Roof unit NTRIP client (access data set here)</label></td></tr>");

    strcat(HTML_String, "</table>");
    strcat(HTML_String, "</form>");
    strcat(HTML_String, "<br><hr>");



       //---------------------------------------------------------------------------------------------
    // GPS Get Baudrate
    strcat(HTML_String, "<h2>ESP32 serial1 baudrate connected to GPS u-blox chip UART 1 baudrate</h2>");
    strcat(HTML_String, "If the baudrate between ESP32 and u-blox F9P does not fit, no data will be transferd.<br>Run get u-blox baudrate below, if UCenter is not working or no GPS data is comming.<br><br>");
    strcat(HTML_String, "<form>");
    strcat(HTML_String, "<table>");
    set_colgroup(50, 180, 180, 0, 0);

    strcat(HTML_String, "<tr> <td colspan=\"2\">ESP32 serial1 to GPS baudrate</td><td>ESP32 USB baudrate</td></tr>");
    for (byte i = 0; i < nrBaudrates; i++) {//nrBaudrates
        strcat(HTML_String, "<tr><td>");
        if ((Set.SerialToTeensyBaudrateNr == i) && GPSBaudrateValid) strcat(HTML_String, "u-blox verified");
        strcat(HTML_String, "</td><td><input type = \"radio\" onclick=\"sendVal('/?BaudrTeensy=");
        strcati(HTML_String, i);
        strcat(HTML_String, "')\" name=\"baudrate\" id=\"JZ\" value=\"");
        strcati(HTML_String, i);
        strcat(HTML_String, "\"");
        if (Set.SerialToTeensyBaudrateNr == i) strcat(HTML_String, "CHECKED");
        strcat(HTML_String, "><label for=\"JZ\"> ");
        strcati(HTML_String, baudrates[i]);
        strcat(HTML_String, "</label></td>");

        strcat(HTML_String, "<td><input type = \"radio\" onclick=\"sendVal('/?BaudrUSB=");
        strcati(HTML_String, i);
        strcat(HTML_String, "')\" name=\"baudrateUSB\" id=\"JZ2\" value=\"");
        strcati(HTML_String, i);
        strcat(HTML_String, "\"");
        if (Set.BaudrateNrUSB == i) strcat(HTML_String, "CHECKED");
        strcat(HTML_String, "><label for=\"JZ2\"> ");
        strcati(HTML_String, baudrates[i]);
        strcat(HTML_String, "</label></td></tr>");
    }
    strcat(HTML_String, "<tr> <td colspan=\"3\">&nbsp;</td> </tr>");

    strcat(HTML_String, "<tr> <td colspan=\"3\">Click get u-blox baudrate to check and<b> wait 15 seconds and reload the page!</b></td> </tr>");
    strcat(HTML_String, "<tr><td></td><td><input type= \"button\" onclick= \"sendVal('/?GPSGetBaud=true');setTimeout(location.reload.bind(location), 7000);\" style= \"width:200px\" value=\"get u-blox baudrate\"></button></td>");
    strcat(HTML_String, "<td><input type= \"button\" onclick= \"sendVal('/?Save=true')\" style= \"width:120px\" value=\"Save\"></button></td>");

    strcat(HTML_String, "</tr></table>");
    strcat(HTML_String, "</form>");
    strcat(HTML_String, "<br><br>If it worked klick the Save button!<br><hr>");

    //-------------------------------------------------------------
    // Checkboxes debugmode
    strcat(HTML_String, "<h2>Debugmode</h2>");
    strcat(HTML_String, "<form>");
    strcat(HTML_String, "<table>");
    set_colgroup(150, 400, 150, 0, 0);

    strcat(HTML_String, "<tr> <td colspan=\"3\">debugmode sends messages to USB serial at ");
    strcati(HTML_String, baudrates[Set.SerialToTeensyBaudrateNr]);
    strcat(HTML_String, " baud </td>");

    strcat(HTML_String, "<tr>");
    strcat(HTML_String, "<td></td><td><input type=\"checkbox\" onclick=\"sendVal('/?debugmode='+this.checked)\" name=\"debugmode\" id = \"Part\" value = \"1\" ");
    if (Set.debugmode == 1) strcat(HTML_String, "checked ");
    strcat(HTML_String, "> ");
    strcat(HTML_String, "<label for =\"Part\"> debugmode on</label></td>");
    strcat(HTML_String, "<td><input type= \"button\" onclick= \"sendVal('/?Save=true')\" style= \"width:120px\" value=\"Save\"></button></td></tr>");



    //---------------------------------------------------------------------------------------------
    // GPIO PINs selection
/*
    strcat(HTML_String, "<h2>Hardware GPIO selection</h2>");
    strcat(HTML_String, "<br>");
    strcat(HTML_String, "<b>ESP_RXSerialToTeensy at ESP32 = ESP_TXSerialToTeensy at GPS board</b><br>");

    strcat(HTML_String, "<b>select # of GPIO pin at ESP32</b><br><br>");
    strcat(HTML_String, "<form>");
    strcat(HTML_String, "<input type= \"button\" onclick= \"sendVal('/?Save=true')\" style= \"width:120px\" value=\"Save\"></button><br>");
    strcat(HTML_String, "<table>");
    set_colgroup(100, 100, 100, 100, 150, 150);
    strcat(HTML_String, "<tr>");
    strcat(HTML_String, "<td><b>ESP_RXSerialToTeensy</b></td>");
    strcat(HTML_String, "<td><b>ESP_TXSerialToTeensy</b></td>");
    strcat(HTML_String, "<td><b>RX2</b> </td>");
    strcat(HTML_String, "<td><b>TX2</b></td>");
    strcat(HTML_String, "<td><b>pin for WiFi/NTRIP indication LED</b></td>");
    strcat(HTML_String, "<td><b>pin for WiFi rescan button</b></td>");
    strcat(HTML_String, "</tr>");
    for (int i = 2; i < 34; i++) {
        //skip not usabel GPIOs
        if (i == 3) { i++; } //3: 6-11: not use! USB 12: ESP wouldn't boot
        if (i == 6) { i = 13; }

        strcat(HTML_String, "<tr>");

        strcat(HTML_String, "<td><input type = \"radio\" onclick=\"sendVal('/?ESP_RXSerialToTeensy=");
        strcati(HTML_String, i);
        strcat(HTML_String, "')\" name=\"ESP_RXSerialToTeensy\" id=\"GPIORX1\" value=\"");
        strcati(HTML_String, i);
        strcat(HTML_String, "\"");
        if (Set.ESP_RXSerialToTeensy == i) { strcat(HTML_String, " CHECKED"); }
        strcat(HTML_String, "><label for=\"JZ");
        strcati(HTML_String, i);
        strcat(HTML_String, "\">");
        strcati(HTML_String, i);
        strcat(HTML_String, "</label></td>");
        strcat(HTML_String, "</td>");

        strcat(HTML_String, "<td><input type = \"radio\" onclick=\"sendVal('/?ESP_TXSerialToTeensy=");
        strcati(HTML_String, i);
        strcat(HTML_String, "')\" name=\"ESP_TXSerialToTeensy\" id=\"GPIOTX1\" value=\"");
        strcati(HTML_String, i);
        strcat(HTML_String, "\"");
        if (Set.ESP_TXSerialToTeensy == i) { strcat(HTML_String, " CHECKED"); }
        strcat(HTML_String, "><label for=\"JZ");
        strcati(HTML_String, i);
        strcat(HTML_String, "\">");
        strcati(HTML_String, i);
        strcat(HTML_String, "</label></td>");

        strcat(HTML_String, "<td><input type = \"radio\" onclick=\"sendVal('/?RX2=");
        strcati(HTML_String, i);
        strcat(HTML_String, "')\" name=\"RX2\" id=\"GPIORX2\" value=\"");
        strcati(HTML_String, i);
        strcat(HTML_String, "\"");
        if (Set.RX2 == i) { strcat(HTML_String, " CHECKED"); }
        strcat(HTML_String, "><label for=\"JZ");
        strcati(HTML_String, i);
        strcat(HTML_String, "\">");
        strcati(HTML_String, i);
        strcat(HTML_String, "</label></td>");

        strcat(HTML_String, "<td><input type = \"radio\" onclick=\"sendVal('/?TX2=");
        strcati(HTML_String, i);
        strcat(HTML_String, "')\" name=\"TX2\" id=\"GPIOTX2\" value=\"");
        strcati(HTML_String, i);
        strcat(HTML_String, "\"");
        if (Set.TX2 == i) { strcat(HTML_String, " CHECKED"); }
        strcat(HTML_String, "><label for=\"JZ");
        strcati(HTML_String, i);
        strcat(HTML_String, "\">");
        strcati(HTML_String, i);
        strcat(HTML_String, "</label></td>");

        strcat(HTML_String, "<td><input type = \"radio\" onclick=\"sendVal('/?LED=");
        strcati(HTML_String, i);
        strcat(HTML_String, "')\" name=\"LED\" id=\"GPIOLED\" value=\"");
        strcati(HTML_String, i);
        strcat(HTML_String, "\"");
        if (Set.LEDWiFi_PIN == i) { strcat(HTML_String, " CHECKED"); }
        strcat(HTML_String, "><label for=\"JZ");
        strcati(HTML_String, i);
        strcat(HTML_String, "\">");
        strcati(HTML_String, i);
        strcat(HTML_String, "</label></td>");

        strcat(HTML_String, "<td><input type = \"radio\" onclick=\"sendVal('/?WiFiResc=");
        strcati(HTML_String, i);
        strcat(HTML_String, "')\" name=\"WiFi rescan button\" id=\"GPIOWiFiResc\" value=\"");
        strcati(HTML_String, i);
        strcat(HTML_String, "\"");
        if (Set.Button_WiFi_rescan_PIN == i) { strcat(HTML_String, " CHECKED"); }
        strcat(HTML_String, "><label for=\"JZ");
        strcati(HTML_String, i);
        strcat(HTML_String, "\">");
        strcati(HTML_String, i);
        strcat(HTML_String, "</label></td>");

        strcat(HTML_String, "</tr>");
    }
    strcat(HTML_String, "</table>");
    strcat(HTML_String, "</form><hr>");
*/




