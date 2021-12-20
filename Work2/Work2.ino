#include "ESP8266.h"
#include <SoftwareDebug.h>
#include "Config.h"

#define SS_RX (2)
#define SS_TX (3)
#define Debug (Serial)
#define SERVER_PORT (8090)

#define SENSOR_PIN (4)
#define VIB_PIN (5)
#define LASER_PIN (6)

char* staIP = "";
SoftwareDebug ss(SS_RX,SS_TX); //rx,tx -> esp_tx,esp_rx
ESP8266 wifi(ss); //with baudrate 9600

int devIndex = -1;
int getDevIndex(const char* ip) {
  int numDevs = sizeof(ipList)/sizeof(char*);
  for(int i = 0;i < numDevs;i++) {
    if(strcmp(ipList[i], ip) == 0) return i;
  }

  return -1;
}

bool isSTAIP(const char* info) {
  return strcmp("+CIFSR:STAIP",info) == 0;
}

const char* getSTAIP(const char* ipInfo) {
  char buf[256] = {0};
  int len = strlen(ipInfo);
  strncpy(buf, ipInfo, len);
  buf[len] = 0;
  const char* delim = ",\n\r";

  int targetTokenIdx = -1;
  int tokenIdx = 0;

  const char* token = strtok(buf, delim);

  while(token != NULL)
  {
     if(isSTAIP(token)) targetTokenIdx = tokenIdx + 1;
     else if(targetTokenIdx == tokenIdx) {
        char* finalToken = malloc(sizeof(char) * 20);
        const char* realIPStart = token + 1;
        len = strlen(realIPStart) - 1;
        strncpy(finalToken, realIPStart, len); //to strip off double quotation
        finalToken[len] = 0;
        return finalToken;
     }

     token = strtok(NULL, delim);
     tokenIdx++;
  }

  Debug.println("cannot find ip");
  return "";
}

void wifiSetup() {
    Debug.print("wifi setup begin\r\n");

    if (wifi.setOprToStationSoftAP()) {
        Debug.print("to station + softap ok\r\n");
    } else {
        Debug.print("to station + softap err\r\n");
    }

    if (wifi.joinAP(SSID, PASSWORD)) {
        Debug.print("Join AP success\r\n");
        Debug.print("IP: ");
        //const char* ip = wifi.getLocalIP().c_str();
        //Debug.println(wifi.getLocalIP().c_str());
        staIP = getSTAIP(wifi.getLocalIP().c_str());
        Debug.print(staIP);
        Debug.print(",Dev index:");
        devIndex = getDevIndex(staIP);
        Debug.println(devIndex);
    } else {
        Debug.print("Join AP failure\r\n");
    }

    Debug.print("wifi setup end\r\n");
}

void startTCPServer() {
  if (wifi.enableMUX()) {
      Debug.print("multiple ok\r\n");
  }
  else {
      Debug.print("multiple err\r\n");
  }

  if (wifi.startTCPServer(SERVER_PORT)) {
      Debug.print("start tcp server ok\r\n");
  } else {
      Debug.print("start tcp server err\r\n");
  }

  if (wifi.setTCPServerTimeout(10)) {
      Debug.print("set tcp server timout 10 seconds\r\n");
  } else {
      Debug.print("set tcp server timout err\r\n");
  }
}

void sendToDev(int devIdx, const char* msg) {
    Debug.print("try to send to ");
    Debug.println(ipList[devIdx]);

    uint8_t mux_id = 0;
    if (wifi.createTCP(mux_id, ipList[devIdx], SERVER_PORT)) {
       Debug.print("create tcp ");
       Debug.print(mux_id);
       Debug.println(" ok");
    }
    else {
       Debug.print("create tcp ");
       Debug.print(mux_id);
       Debug.println(" err");
    }

    wifi.send(mux_id, (const uint8_t*)msg, strlen(msg));

    if (wifi.releaseTCP(mux_id)) {
        Debug.print("release tcp ");
        Debug.print(mux_id);
        Debug.println(" ok");
    } else {
        Debug.print("release tcp ");
        Debug.print(mux_id);
        Debug.println(" err");
    }
}

void setup(void)
{
    Debug.begin(9600);
    wifiSetup();
    startTCPServer();
    sendTestMsgToDev(1);
}

void sendTestMsgToDev(int targetDev){
    char temp[100] = {0};
    strcpy(temp, "Hi, echo from ");
    strcpy(temp + strlen(temp), staIP);
    sendToDev(targetDev,temp);
}

void loop(void)
{
    uint8_t buffer[256] = {0};
    uint8_t mux_id;
    uint32_t len = wifi.recv(&mux_id, buffer, sizeof(buffer), 100);
    if (len > 0) {
        Debug.print("Status:[");
        Debug.print(wifi.getIPStatus().c_str());
        Debug.println("]");

        Debug.print("Received from :");
        Debug.print(mux_id);
        Debug.print("[");
        for(uint32_t i = 0; i < len; i++) {
            Debug.print((char)buffer[i]);
        }
        Debug.print("]\r\n");

        if(wifi.send(mux_id, buffer, len)) {
            Debug.print("send back ok\r\n");
        } else {
            Debug.print("send back err\r\n");
        }

        if (wifi.releaseTCP(mux_id)) {
            Debug.print("release tcp ");
            Debug.print(mux_id);
            Debug.println(" ok");
        } else {
            Debug.print("release tcp ");
            Debug.print(mux_id);
            Debug.println(" err");
        }

        Debug.print("Status:[");
        Debug.print(wifi.getIPStatus().c_str());
        Debug.println("]");
    }
}
