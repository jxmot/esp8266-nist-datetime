//////////////////////////////////////////////////////////////////////////////
/*
    Borrowed NIST/Time code from - 
    
    https://www.arduino.cc/en/Tutorial/UdpNtpClient
    
   
    todo:
    https://www.ipify.org/
    https://freegeoip.net/
    
*/
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <time.h>

#include "configData.h"

#include "debugLog.h"

// These are global so that the rest of the application code can
// use them.
configData *cfgdat = new configData();
debugLog *dlog = new debugLog(cfgdat->getBAUD());

// updates every 3 hours
#define NTP_UPDATE_INTERVAL ((3 * 60) * 60)     // ((minutes) * seconds_per_minute)

void ConnectToAP(char *ssid, char *passw);
void DecodeEpoch(unsigned long currentTime, unsigned long epoch);
void Request_Time();
bool Check_Time();
void sendNTPpacket(IPAddress & address);

// for Serial.print() calls out side of the logger, necessary if 
// the logger has been muted.
const int DEFAULT_SERIAL_BAUD = 115200;

// this should be "discovered" using our outside IP and geolocation
#define DST -5
#define ST -6
const int GMT_OFFSET = DST;

unsigned int localPort = 2390;      // local port to listen for UDP packets

// if having trouble connecting then take a look at - 
// http://tf.nist.gov/tf-cgi/servers.cgi
// for additional addresses
IPAddress timeServer(129, 6, 15, 28); // time.nist.gov NTP server

const int NTP_PACKET_SIZE = 48; // NTP time stamp is in the first 48 bytes of the message

byte packetBuffer[NTP_PACKET_SIZE]; //buffer to hold incoming and outgoing packets

//Unix time in seconds
unsigned long epoch = 0;
unsigned long LastNTP = 0;

// A UDP instance to let us send and receive packets over UDP
WiFiUDP udp;

int RequestedTime = 0;
int TimeCheckLoop = 0;

void setup() {
    // a necessary delay...
    delay(300);

    // let's check for errors before continuing...
    String errMsg;
    if(!cfgdat->getError(errMsg)) {
        // no errors!
        ConnectToAP(cfgdat->getSSID(), cfgdat->getPASS());
        //Get the time
        dlog->println("Starting UDP");
        udp.begin(localPort);
        dlog->print("Local port: ");
        dlog->println(udp.localPort());
        Request_Time();
        delay(2000);
        while (!Check_Time()) {
            delay(2000);
            TimeCheckLoop++;
            if (TimeCheckLoop > 5) {
                Request_Time();
            }
        }
    
    } else {
        Serial.begin(DEFAULT_SERIAL_BAUD);
        for(int ix = 0; ix < 3; ix++) {
            Serial.print("ERROR! - ");
            Serial.println(errMsg);
        }
    }
}

void loop() 
{
    int SecondsSinceLastNTP = (millis() - LastNTP) / 1000;
    
    dlog->print("Seconds since last NTP: ");
    dlog->println(SecondsSinceLastNTP);
    
    // Update NTP if it's time and we haven't already requested it.
    if (SecondsSinceLastNTP > NTP_UPDATE_INTERVAL and RequestedTime == 0) {
        Request_Time();
        RequestedTime = 1;
    }
    
    if (RequestedTime == 1) {
        Check_Time();
        TimeCheckLoop++;
        if (TimeCheckLoop > 5) {
            RequestedTime = 0;
        }
    }
    
    //Add seconds to Epoch using ESP8266 Millis as a reference
    //This way the unit keeps time between calls to NTP
    //Only Check_Time() should update epoch
    DecodeEpoch((epoch + SecondsSinceLastNTP), epoch);
    delay(500);
}

void ConnectToAP(char *ssid, char *passw) {
uint8_t mac[6];
char macStr[18] = { 0 };

    dlog->println("");
    dlog->println("");
    dlog->println("Attempting to Connect to WiFi....");
    
    while (true) {
        delay(1000);
        dlog->print("Connecting to ");
        dlog->println(ssid);
        WiFi.mode(WIFI_STA);
        WiFi.begin(ssid, passw);
        for (int x = 0; x < 5; x++) {
            delay(1000);
            if (WiFi.status() == WL_CONNECTED) {
                dlog->print("WiFi connected in ");
                dlog->print(x);
                dlog->println(" seconds");
                dlog->print("IP address: ");
                dlog->println(WiFi.localIP().toString());
            
                WiFi.macAddress(mac);
                sprintf(macStr, "%02X:%02X:%02X:%02X:%02X:%02X", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
                dlog->print("MAC address: ");
                dlog->println(macStr);
            
                return;
            }
        }
    }
}

void DecodeEpoch(unsigned long currentTime, unsigned long epochUTC) 
{
    //Update for local zone
    time_t currtime = currentTime + (GMT_OFFSET * 60 * 60);
    struct tm *timeinfo = localtime(&currtime);
    
    char buf[48];   
    sprintf(buf, "%d-%02d-%02d  %02d:%02d:%02d", 
            timeinfo->tm_year + 1900,
            timeinfo->tm_mon + 1,
            timeinfo->tm_mday,
            timeinfo->tm_hour, 
            timeinfo->tm_min, 
            timeinfo->tm_sec);
    dlog->println(buf);
}

void Request_Time() {
    dlog->println("Getting Time");
    sendNTPpacket(timeServer); // send an NTP packet to a time server
}

bool Check_Time() {
int cb = udp.parsePacket();

    if (!cb) {
        dlog->println("no packet yet");
        return false;
    } else {
        dlog->print("packet received, length=");
        dlog->println(cb);
        // We've received a packet, read the data from it
        udp.read(packetBuffer, NTP_PACKET_SIZE); // read the packet into the buffer
    
        //the timestamp starts at byte 40 of the received packet and is four bytes,
        // or two words, long. First, extract the two words:
    
        unsigned long highWord = word(packetBuffer[40], packetBuffer[41]);
        unsigned long lowWord = word(packetBuffer[42], packetBuffer[43]);
        // combine the four bytes (two words) into a long integer
        // this is NTP time (seconds since Jan 1 1900):
        unsigned long secsSince1900 = highWord << 16 | lowWord;
        //dlog->print("Seconds since Jan 1 1900 = " );
        // dlog->println(secsSince1900);
    
        // now convert NTP time into everyday time:
        //dlog->print("Unix time = ");
        // Unix time starts on Jan 1 1970. In seconds, that's 2208988800:
        const unsigned long seventyYears = 2208988800UL;
        // subtract seventy years:
        epoch = secsSince1900 - seventyYears;
        LastNTP = millis();
        RequestedTime = 0;
        TimeCheckLoop = 0;
        return true;
    }
}

// send an NTP request to the time server at the given address
void sendNTPpacket(IPAddress & address) {
    dlog->println("sending NTP packet...");
    // set all bytes in the buffer to 0
    memset(packetBuffer, 0, NTP_PACKET_SIZE);
    // Initialize values needed to form NTP request
    // (see URL above for details on the packets)
    packetBuffer[0] = 0b11100011;   // LI, Version, Mode
    packetBuffer[1] = 0;     // Stratum, or type of clock
    packetBuffer[2] = 6;     // Polling Interval
    packetBuffer[3] = 0xEC;  // Peer Clock Precision
    // 8 bytes of zero for Root Delay & Root Dispersion
    packetBuffer[12]  = 49;
    packetBuffer[13]  = 0x4E;
    packetBuffer[14]  = 49;
    packetBuffer[15]  = 52;
    
    // all NTP fields have been given values, now
    // you can send a packet requesting a timestamp:
    udp.beginPacket(address, 123); //NTP requests are to port 123
    udp.write(packetBuffer, NTP_PACKET_SIZE);
    udp.endPacket();
}

