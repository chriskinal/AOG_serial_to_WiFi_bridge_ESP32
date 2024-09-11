// WIFI handling 19. Feb 2023 for ESP32  -------------------------------------------
// Mrch 2024: version for ESP AP only

void WiFi_handle_connection(void* pvParameters) {
    task_WiFiConnectRunning = true;

    bitSet(mainLoopDelay, 3);//give time for WiFi connection task

    //start access point
    WiFi_Start_AP();
    vTaskDelay(3000);//wait 3 seconds for AP start             

    if (my_WiFi_Mode != WIFI_AP) { vTaskDelay(1000); }

    vTaskDelay(1000);//wait 1 second for propper AP config  

/* no modul at the moment
    if (WiFiUDPToAOG.listen(Set.PortModulToAOG))
    {
        Serial.print("UDP writing to IP: ");
        Serial.println(WiFi_ipDestination);
        Serial.print("UDP writing to port: ");
        Serial.println(Set.PortDestination);
        Serial.print("UDP writing from port: ");
        Serial.println(Set.PortModulToAOG);
    }
    vTaskDelay(1000);//wait 1 second 
*/
    //init WiFi UDP listening to AOG on port 8888
    if (WiFiUDPFromAOG.listen(Set.PortFromAOG)) {
        Serial.print("UDP listening to AOG on port: ");
        Serial.println(Set.PortFromAOG);
    }
    vTaskDelay(1000);//wait 1 second 

    // UDP message from AgIO packet handling on port 8888
    WiFiUDPFromAOG.onPacket([](AsyncUDPPacket packet)
        {   //put data in buffer, as async: send in main loop
            byte nextBridgeDataRingCountIn = (BridgeData8888RingCountIn + 1) % BridgeArraySize;
            BridgePacket8888Length[nextBridgeDataRingCountIn] = packet.length();
            for (unsigned int i = 0; i < BridgePacket8888Length[nextBridgeDataRingCountIn]; i++) {
                BridgeData8888[nextBridgeDataRingCountIn][i] = packet.data()[i];
            }
            BridgeData8888RingCountIn = nextBridgeDataRingCountIn;

            // if (Set.debugmode) { Serial.print("got AOG data via WiFi on port 8888. packet lenght: "); Serial.println(packet.length()); }

            /*maybe AgIO will need heartbeat in future
            //AgIO heartbeat + scan request
            if ((packet.data()[0] == AgIO_msgHeader[0]) && (packet.data()[1] == AgIO_msgHeader[1]) &&
                (packet.data()[2] == AgIO_msgHeader[2])) {
                if ((Set.IMUType > 0) && (packet.data()[3] == AgIO_heartbeat) && (Set.AgIOHeartbeat_answer == 1))
                {//AgIO heartbeat
                    WiFiUDPToAOG.writeTo(helloFromIMU, sizeof(helloFromIMU), WiFi_ipDestination, Set.PortDestination);
                }
                else {
                    if (packet.data()[3] == AgIO_ScanRequest) {
                        //IP scan request
                        uint8_t scanReply[] = { 128, 129, 120, 203, 7,
                            WiFi.localIP()[0], WiFi.localIP()[1],WiFi.localIP()[2],WiFi.localIP()[3],0,0,0, 23 };
                        //checksum
                        int16_t CK_A = 0;
                        for (uint8_t i = 2; i < sizeof(scanReply) - 1; i++)
                        {
                            CK_A = (CK_A + scanReply[i]);
                        }
                        scanReply[sizeof(scanReply) - 1] = CK_A;
                        WiFiUDPToAOG.writeTo(scanReply, sizeof(scanReply), WiFi_ipDestination, Set.PortDestination);

                        if (Set.IMUType > 0) {
                            uint8_t scanReply[] = { 128, 129, 121, 203, 7,
                                WiFi.localIP()[0], WiFi.localIP()[1],WiFi.localIP()[2],WiFi.localIP()[3],0,0,0, 23 };
                            //checksum
                            int16_t CK_A = 0;
                            for (uint8_t i = 2; i < sizeof(scanReply) - 1; i++)
                            {
                                CK_A = (CK_A + scanReply[i]);
                            }
                            scanReply[sizeof(scanReply) - 1] = CK_A;
                            WiFiUDPToAOG.writeTo(scanReply, sizeof(scanReply), WiFi_ipDestination, Set.PortDestination);
                        }
                    }
                }
            }
            */
            // if (Set.debugmode) { Serial.print("got AOG data via WiFi. packet lenght: "); Serial.println(packet.length()); }

        });  // end of onPacket call

    vTaskDelay(1000);//wait 1 second 

    //init WiFi UDP listening to other modules port 9999
    if (WiFiUDPBridgeToAOG.listen(Set.PortDestination)) {
        Serial.print("UDP listening to other modules destination port: ");
        Serial.println(Set.PortDestination);
    }

    vTaskDelay(1000);//wait 1 second 

    // UDP message from other modules packet handling port 9999
    WiFiUDPBridgeToAOG.onPacket([](AsyncUDPPacket packet)
        {   //put data in buffer, as async: send in main loop
            byte nextBridgeDataRingCountIn = (BridgeData9999RingCountIn + 1) % BridgeArraySize;
            BridgePacket9999Length[nextBridgeDataRingCountIn] = packet.length();
            for (unsigned int i = 0; i < BridgePacket9999Length[nextBridgeDataRingCountIn]; i++) {
                BridgeData9999[nextBridgeDataRingCountIn][i] = packet.data()[i];
            }
            BridgeData9999RingCountIn = nextBridgeDataRingCountIn;

            // if (Set.debugmode) { Serial.print("got AOG data via WiFi on port 9999. packet lenght: "); Serial.println(packet.length()); }

        });  // end of onPacket call

    vTaskDelay(1000);//wait 1 second 

    /* maybe useful to send NTRIP data
    //init WiFi UPD listening to AOG
    if (WiFi_udpNtrip.listen(Set.AOGNtripPort))
    {
        Serial.print("NTRIP WiFi UDP Listening to port: ");
        Serial.println(Set.AOGNtripPort);
        Serial.println();
    }

    vTaskDelay(1000);//wait 1 second


    // UDP NTRIP packet handling
    WiFi_udpNtrip.onPacket([](AsyncUDPPacket packet)
        {
            if (Set.debugmode) { Serial.print("got NTRIP data via WiFi. packet lenght: "); Serial.println(packet.length()); }
            Serial1.write(packet.data(), packet.length());
        });  // end of onPacket call
    WiFiUDPRunning = true;

    vTaskDelay(1000);//wait 1 second
    */

    WiFiUDPRunning = true;

    //start Server for Webinterface
    WiFiStartServer();

    vTaskDelay(1000);//wait 1 second 

    WebIOTimeOut = millis() + (long(Set.timeoutWebIO) * 60000);
    xTaskCreate(doWebinterface, "WebIOHandle", 5000, NULL, 1, &taskHandle_WebIO);

    vTaskDelay(1000);//wait 1 second 


    Serial.println(); Serial.println();
    IPAddress myIP = WiFi.softAPIP();
    Serial.print("started settings Webinterface at: ");
    for (byte i = 0; i < 3; i++) {
        Serial.print(myIP[i]); Serial.print(".");
        delay(10);//output to serial mixes up
    }
    Serial.println(myIP[3]);
    delay(10);//output to serial mixes up
    Serial.println("type IP in Internet browser to get to webinterface");
    Serial.print("you need to be in WiFi network ");
    Serial.print(Set.ssid_ap);
    delay(10);//output to serial mixes up
    Serial.println(" to get access"); Serial.println(); Serial.println();

    LED_WIFI_ON = true;
    if (Set.LEDWiFi_PIN != 255) { digitalWrite(Set.LEDWiFi_PIN, Set.LEDWiFi_ON_Level); }

    if (Set.debugmode) { Serial.println("closing WiFi connection task"); Serial.println(); }
    task_WiFiConnectRunning = false;
    bitClear(mainLoopDelay, 3);

    delay(1);
    vTaskDelete(NULL);
    delay(1);
}

//-------------------------------------------------------------------------------------------------
// start WiFi Access Point

void WiFi_Start_AP() {
    WiFi.mode(WIFI_AP);   // Accesspoint
    WiFi.softAP(Set.ssid_ap, Set.password_ap);   
    //while (!SYSTEM_EVENT_AP_START) // wait until AP has started
    while (!WIFI_EVENT_AP_START) // wait until AP has started
    {
        delay(100);
        Serial.print(".");
    }
    delay(150);//right IP adress only with this delay 
    WiFi.softAPConfig(Set.WiFi_gwip, Set.WiFi_gwip, Set.mask);  // set fix IP for AP  
    delay(350);
    IPAddress myIP = WiFi.softAPIP();
    delay(300);
    Serial.print("Access point started - Name : ");
    Serial.println(Set.ssid_ap);
    Serial.print("Password: ");
    delay(10);//output to serial mixes up
    Serial.println(Set.password_ap);
    Serial.print("IP address: ");
    delay(10);//output to serial mixes up
    WiFi_ipDestination = myIP;       
    Serial.println(WiFi_ipDestination);
    WiFi_ipDestination[3] = Set.WiFi_ipDest_ending; 
    delay(10);//output to serial mixes up
    Serial.print("Sending Teensy data (from AgOpenGPS) to IP: ");
    Serial.println(WiFi_ipDestination);
    my_WiFi_Mode = WIFI_AP;
}

