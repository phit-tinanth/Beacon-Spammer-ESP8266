#include <ESP8266WiFi.h>

extern "C" {
  #include "user_interface.h"
}

String ssid = "                                                               _";
byte channel;

uint8_t packet[128] = { 0x80, 0x00, 0x00, 0x00, 0xff, 0xff,
                        0xff, 0xff, 0xff, 0xff, 0x01, 0x02, 
                        0x03, 0x04, 0x05, 0x06, 0x01, 0x02, 
                        0x03, 0x04, 0x05, 0x06, 0xc0, 0x6c, 
                        0x83, 0x51, 0xf7, 0x8f, 0x0f, 0x00, 
                        0x00, 0x00, 0x64, 0x00, 0x01, 0x04, 
                        0x00, 0x06, 0x72, 0x72, 0x72, 0x72, 
                        0x72, 0x72, 0x01, 0x08, 0x82, 0x84,
                        0x8b, 0x96, 0x24, 0x30, 0x48, 0x6c, 
                        0x03, 0x01, 0x04};

void setup() {
  delay(100);
  wifi_set_opmode(STATION_MODE);
  wifi_promiscuous_enable(1); 
}

void loop() { 
    int channels[] = {8, 9, 10, 11, 12};
    channel = channels[random(0, 5)];
    //channel = random(1,14);
    wifi_set_channel(channel);
    delay(1);
    packet[10] = packet[16] = random(256);
    packet[11] = packet[17] = random(256);
    packet[12] = packet[18] = random(256);
    packet[13] = packet[19] = random(256);
    packet[14] = packet[20] = random(256);
    packet[15] = packet[21] = random(256);

    packet[38] = ssid[random(65)];
    packet[39] = ssid[random(65)];
    packet[40] = ssid[random(65)];
    packet[41] = ssid[random(65)];
    packet[42] = ssid[random(65)];
    packet[43] = ssid[random(65)];
    packet[44] = ssid[random(65)];
    packet[45] = ssid[random(65)];
    packet[46] = ssid[random(65)];
    packet[47] = ssid[random(65)];
    packet[48] = ssid[random(65)];
    packet[49] = ssid[random(65)];
    packet[50] = ssid[random(65)];
    packet[51] = ssid[random(65)];
    
    packet[57] = channel;
    
    wifi_send_pkt_freedom(packet, 60, 0);
    wifi_send_pkt_freedom(packet, 60, 0);
    wifi_send_pkt_freedom(packet, 60, 0);
    wifi_send_pkt_freedom(packet, 60, 0);
    wifi_send_pkt_freedom(packet, 60, 0);
    wifi_send_pkt_freedom(packet, 60, 0);

    delay(1);
}
