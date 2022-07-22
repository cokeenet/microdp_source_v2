
/*Code writed by cokee
 *https://cokee.tech &&Email:bluecitynetwork@outlook.com
 *Rules:GPL V3.0
 *Rev 3.0a
 */
#define SUN 0
#define SUN_CLOUD 1
#define CLOUD 2
#define RAIN 3
#define THUNDER 4
#define SNOW 5
#define MIST 6 //雾
#define MOON 7
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>
#include <ArduinoOTA.h>
#include <WiFiClientSecure.h>
#include <ESP8266mDNS.h>
#include <TimeLib.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <ESP8266HTTPUpdateServer.h>
#include <U8g2lib.h>
#include <coredecls.h>
#include <include/WiFiState.h>
#include <LittleFS.h> // LittleFS is declared
bool wea = false, cw[7], offline;
unsigned long pMillis = 0;
// WiFiClient aclient;
U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /*reset=*/U8X8_PIN_NONE);
ESP8266WebServer webserver(80);
ESP8266HTTPUpdateServer httpUpdater;
#define DEF_WIFI_SSID "BlueCityNetworks"
#define DEF_WIWI_PASSWORD "666688881234"
#define AP_NAME "CokeeMicroDP_3983" // random num

static const char ntpServerName[] = "ntp1.aliyun.com"; // NTP服务器，阿里云
int timeZone = 8;                                      //时区，北京时间为+8
String ver = "V2",hitok="加油!",lang="notset";
WiFiUDP Udp;
unsigned int localPort = 8888; // 用于侦听UDP数据包的本地端口
// uint32_t read_time=0;
time_t getNtpTime();
void sendNTPpacket(IPAddress &address);
void oledClockDisplay();
void sendCommand(int command, int value);
void initdisplay();
bool powersave = false;
boolean isNTPConnected = false, clk = true;

struct nv_s {
  WiFiState wss; // core's WiFi save state

  struct {
    uint32_t crc32;
    uint32_t rstCount;  // stores the Deep Sleep reset count
    // you can add anything else here that you want to save, must be 4-byte aligned
  } rtcData;
};
static nv_s* nv = (nv_s*)RTC_USER_MEM;

struct WifiPwd {
  char ssid[32];
  char pwd[64];
};
WiFiClient client;
/*
  //保存参数到eeprom


void saveConfig(WifiPwd *wifipwd) {
  EEPROM.begin(EEPROM_START);
  uint8_t *p = (uint8_t*)(wifipwd);
  for (int i = 0; i < sizeof(*wifipwd); i++) {
    EEPROM.write(i, *(p + i));
  }
  EEPROM.commit();
  // 释放内存
  delete wifipwd;
}

  // 获取wifi账号密码信息
WifiPwd* loadConfigs() {
  // 为变量请求内存
  WifiPwd *pvalue  = new WifiPwd;   
  EEPROM.begin(EEPROM_START);
  uint8_t *p = (uint8_t*)(pvalue);
  for (int i = 0; i < sizeof(*pvalue); i++) {
    *(p + i) = EEPROM.read(i);
  }
  EEPROM.commit();
  return pvalue;
}


   //清空wifi账号和密码

void clearConfig() {
  EEPROM.begin(EEPROM_START);
  // 这里为啥是96 ，因为在结构体声明的长度之和就是96
  for (int i = 0; i < 96; i++) {
    EEPROM.write(i, 0);
  }
  EEPROM.commit();
}*/
const uint8_t u8g2_font_unifont_dx[2900] U8G2_FONT_SECTION("u8g2_font_unifont_dx") =
    "\206\0\3\2\4\5\3\5\6\20\21\0\374\12\374\13\374\1\310\3\367\5\202 \5\0\30I!\10\261\374"
    "J\34\244\0\42\10\64zMD\246\0#\25\271\370\324\25\263\246\341\224\205Y\230E\303%\13\263\60\313"
    "\0$\27\327\352\320\205\203\242$S$eJ\272&Z$EK\242\14b\6%\26\270\370\320\204I\251"
    "\22UjI\213\322O\225\250\22U\212\11\0&\31\273\372\330\355@\26gq\226\3;\220\204\211\26\25"
    "\225(\325\304m\320\4'\6\61zI\14(\14\363\312J%Q\22\365\267(\13)\15\364\310J\204\265"
    "\60\353S\26\325\0*\10S<\317D\311%+\14w\34\327\305\265a\310\342\32\0,\6\61\332H\14"
    "-\7\24<Q\14\1.\6\21\372H\4/\16\266\370\314\206i\230\26\323\60\15S\0\60\15\267\372P"
    "m\225\324\257I\226M\0\61\13\266\372PiIT\355\247a\62\15\266\372\320\14\211\64\246\305^\207\1"
    "\63\21\267\372\320\14\212\266\306\251\264\3\262\272)\203\2\64\23\270\370\320\306j\222&a\224u\12\243a"
    "Ps \2\65\20\267\372P\134\342\226!\321\344X\335\242!\2\66\22\267\372P\15Q(W\206D["
    "\255I\226\15\11\0\67\15\267\372P\34\342\264\234\306\325\270\10\70\21\267\372\320\14\212\266\252\233\64i\253"
    "\272)\203\2\71\22\267\372\320\14\221\226\244\326M\31\222\70\26\243!\2:\7\201\372H\304\1;\7\241"
    "\332H\304\3<\15\207\34W\207\232U\7t@\7\2=\12W,W\34r\206C\0>\16\207\34W"
    "\344\200\16\350\200\252\31c\0\77\17\267\370N\15\211\266\306\251(\306\71\71\3@'\356\332\336\16\263\34"
    "\346PQJ\242HJ\42-\314\244\64\223\322LJ\243$\12\245$\33\222-'\344H:\14\32\0A"
    "\26\272\370T\352\210\16Dq\24Gi\330\66\14R\34\305I\216\4B\24\267\372R\14R\246\204I\230"
    "d\321\220eJj\34\206\10C\26\271\372\324\15\231\30\305:\222#\71\222#\71\24'b\70D\0D"
    "\26\271\372V\334R%Nr@\7t@\7t@NRe\330\0E\15\267\372P\34\344\346a\211\233"
    "\207!F\14\266\372P\34\322\326aH\273\2G\27\271\372V\15\232\30\305:\222#\71\22\16:\220\304"
    "\211\250\15\21\0H\26\271\372V\344\200\16\350\200\16\350\300\203\16\350\200\16\350\200\16\4I\7\261\372H"
    "<\10J\12\264\370\312l\375iP\0K\24\267\374R\204I\246DR-\11\307(\213\244,\12\223P"
    "L\11\265\374N\204\375\343 M\35\273\372\134\350\300\220\3C\234(i\242\244\211\224ER\26iI\246"
    "%\231\226db\1N\24\271\372X\310\263\222*\242\24j\231\30\211\221\232\310\263\0O\26\272\372\330\15"
    "\241)NrDGtDGt\244\34\211\342\220\1P\16\266\374R\14J\66\272%\203\222\266\2Q\33"
    "\352\312\330\15\241)NrDGtDGt\244\34\211\342\240\3\71\226C\13\0R\22\267\372R\134\302"
    "\325\70\14Q$eQ\246\204I\32S\22\267\372\320\14\212\266\352@\16\354\200\254n\312\240\0T\26\270"
    "\370P\34\264\34\310\201\34\310\201\34\310\201\34\310\201\34\10\1U\14\270\372V\304\376\71\11\263!\2V"
    "\25\272\370T\344H\71\212#Q\13[\243\70\212\243\34\320\21\21W\34\276\370^\244\216J(FY\22"
    "FY\224\225\32\243\306$L\302$LRQ\256\1X\26\271\370\322\244Q\232U\223\70\311\201\34H\322"
    ",\314\252I\16\4Y\25\271\370R\250J\232\25\263\64\211w Gr$Gr$\4Z\26\271\370\322"
    "\14\203\16\344\200\16\344@\16\344@\16\344\200\254\3\17[\12\364\312J\14Z\377\267!\134\15\306\370L"
    "\304i\71\215\323\70-\7]\12\363\310JL\375\237\206\0^\16x:\327\311j\24FY\30\205I\34"
    "_\7\27\330N\34\2`\7#\212Ih\1a\17\206\372\320\14I\230&\303\220\211\232\262\4b\20\307"
    "\372R\304]\206DKR\327-I&\0c\15\206\372\316\14Q\246\266j\311\220\0d\16\307\372Rw"
    "Zj\253k\222)C\22e\20\207\372P\15Q\226\244\303\71\7\302hH\0f\14\305\370\312Iai"
    "\220\302>\1g\22\267\312\322\14ImuM\62eHb\61\32\42\0h\14\307\372R\304]\206D["
    "}\15i\10\301\372Hd\303 j\12\362\310\310\344@\322_\6k\21\307\372P\304\35\223,\252%\241"
    "\222\225\62%\24l\6\301\372H|m\22\213\372Z$\323\42\35C\61\24C\61\24C\261\0n\13\207"
    "\372R$C\242\255\276\6o\14\207\372Rm\225\324\65\311\262\11p\20\267\312R$C\242%\251\353\226"
    "$S\134\6q\16\267\312R-\265\325\65\311\224!\211\33r\12\204\372J$C\224\365\6s\14\205\372"
    "\316,\231\270\212Z\262\0t\13\245\370\312\204\331\240\204\275\16u\13\207\372R\244\276n\312\220\4v\21"
    "\207\370N\244I\26eQV\11\223\60I\63\0w\24\213\370V\204\241&&Q\22\265D\225\250f\324"
    "DM\2x\17\207\370NhQVI\323\61\311*i\0y\22\267\310P\244I\26eQV\11\223P"
    "\256\206*\0z\14\205\372N\14bV\314\212\203\0{\14\365\310\312IaO\242\330\253\0|\7\361\312"
    "H\374\0}\14\364\310J\210Y\217Q\326\213\4~\10'LW\354\300\20\0\0\0\4\377\377N-\33"
    "\354\354`\347X\16\306\303C\230\212\251\230\212\351\360 \246u\60\7s\64\5N\221\32\336\352`\15\207"
    "\234\77\15\17\262\16\352h\226#\241\134\35\16a\16%\0O\64)\376\352\340\205r\232\225\332$%\312"
    "\264\244\226\244\225d\70Di\16\244\71\220\14\207(\315\201\64\7\322\34Hs M\1Qm\36\356\352"
    "\340\352pN\310\311\303\203\316Q\312\241\60\7\324\70\26s$\323\241D\307\22\0Q\260#\356\352\340#"
    "\71\234\312Y\65R\227DG\222\65K\344,)fQ\22fQ[I\254Ib\16\256\0Q\267%\16"
    "\333\340#\71j]\265H\215BY\11%\261NJ\206[\216d:\240\205\221\226*a\16\344\204\34\16"
    "\1Q\273(\376\352\340#\71\232\3\321pH\304\34\311r\70\312\241,\7\222a\320\342Z\224fR\22"
    "i\245$\13#\35\312A\25Sw$\356\352`\353\210\224\305J\224c\71\62<\347\310\360 \226\207C"
    "\266&C\224\346@\266\244\71\20\17C\6Y\32'\14\335\340\346\230\216\15\203\26GRV\33rD\36"
    "\222T\33\206\60\216\224\60\322\242\34\31rD\7\206t\310\21\0Y'\37\355\352`\347h\216\346h<"
    "<\304\71\252cI\216E\71\222\351@\250\306\222\216\14\71\10Y\71$\356\352`\353h\216\14\357@\26"
    "GY\34e\71\220D\341\360 \353`\222\203Q\16\205\252\16H\71\226\0\134\17\37\356\352\340\3\71\234"
    "\303\71\234#Z\224\206YX\13\233\322\60JS\71\315\201\34\316A\35\134\30 \356\352\340\3\71\234C"
    "Y\24g\305\232\26\226\322\34\316\341\34\31\16\71\224\303\71\34\17\17\2^&\42\356\352\340\225\342,\312"
    "\206\7\255\24gQ\66|Gs \315\206kV\315\252Y\65K\252Y\242\1_:-\356\352`\14\321"
    "\260Fa\32\205i\64LC\32\206\303\240\204Y\224\14I\26\205I\26\205\311\60\210i\16\244Q\230&"
    "\222\64$Q\0bl(\356\352`\351h\66\214\71 \15\242\232\312\241\16$\311\60(c\22)a\224"
    "da\224d\245Z\42e\305\222\226I\0f\37$\356\352`\15\207\60G\302\341\20\346H\230#\341p"
    "\310\241\34\11sd\70Hi\254\14w(\207\343\341Aft\61\376\352\340C\341\220\14\203\22\245a\24"
    "\15C\22\245a\224\14\17\71\26E\303\220D\325$\212\206!\31\242\64\211\242a\310\201\64\7\322\34\310"
    "$\0f\264'\356\352`\15\207\60G\302\341\20\346H\70\34b-\35^\263tx\320\244$U\242d"
    "\22\67eP\22\35\21#\0g\11&\376\352`\347p\16|PsTG\207AUCe\30\62)\315"
    "\201\64\7\16\71\220\346@\232\3i\16\204\32\0g*#\15\333`\347h\216\346\310p\310\201\34\315\321"
    "xxHwL\311\221\212\34U\245L\322B\71Gc\0l\231$\355\352\340C\251\230#Y\216FI"
    "\232D\212TK\243\34\215\262\64\312\342$\313\201,\7\62qGf\0mn#\356\352\340\203\232\62\354"
    "`\226\3Q\22\211\231i\30t\264*\247q\62\34\242\70\315\201P\7rp\4p\355(\357\352\340\205"
    "\71\24\306\303T\216\206Y\213R)\211\62%\223\342l\211\262(\225\244X\207\212RI+eQ\230C"
    "\0ry%\356\352\340\245rZ\211\206!J\322p\330\302(\32N\71\234\244\251\62\274\3i&\245a"
    "\224\346@\232N\0w\345'\376\352\340\345p\216\346\360 \15J\24fI\24fa-\254\15\227,\254"
    "ebV\311JY\64(r\266\3Y\0\220\350\61\16\333\340\345\204pxH\242\34\213\42\71\312\242,"
    "\11\243,\211\206AIt,\212\206)K\302(K\302(K\302(K\206)\221\302(\207C\0\225\364"
    "%\374\334\340\344\240\64\14Q\216\350\230\64LR\30Ia$\205\221\64LR\30Ia$\15\223\216\351"
    "\230\16\11\226\64%\354\354`\14\312\60DJ(\25\225,T\262aH\244P*j\311\60dI\250H"
    "\241\32\212\251\230ji\2\226\65)\375\354\340\3\342\220\245\321pP\262\70\311\342$K\302$*&\321"
    "\60$QX\12+\303\203\32\346@\230\3a\16\204\0\226\350'\356\352`|\220s\70G\206\247\60\215"
    "\222(\221Z\242\66-\211\222(\221Z\242\66-\211\302\64J\303(G$\0\226\352 \354\354`|\215"
    "\207\207\60\35\206d\70(\203\16\17\207\34\314\301h\70\344`\64\34r\60\1\226\367%\356\352\340\14\7"
    "\35\311\201_ceP\206D\315\241!\31t\342p\10\263\60\34\16a\26\206\303!\314\221\10\226\371'"
    "\356\352\340\14\357H\16|J\303hP\206mH\6UG\207c\216D\312\60\245a\224\16K\224\345H"
    "\70\34\22\0\226\376%\356\352\340\14\357@\216\14Oa\32\15\213\224\15\312\16\34\304!\313\261u\30\222"
    "a\312\6y\314\1\61\274\1\227>,\356\352\340\14\357@\216\14\17\311\260\14I\62,C\22i\261\62"
    "\34\242$\32\206hjK\222aH\214q\222\14CT\214\225\341\0\230\316(\356\352`\15\207\60G\302"
    "\34\11\223,\12\23\251\30%Y\230ia\246\205Qc\242%Y\42gI\16D:\266\203\2\230\323/"
    "\356\352\340\14\322 eQ\26e\321 ER\26)\322-Q\26%\245AJJY\244(\303\220\330"
    "\222,L\262$M\322\70\325\201A\1\237\231&\356\352\340\352h\246#i\64<\250Q\216Eu \252"
    "\3Q\222\3\231\216d:\20\346\200\64KJ\246\304\203\2\0";
const unsigned char xing[] U8X8_PROGMEM = {
    0x00, 0x00, 0xF8, 0x0F, 0x08, 0x08, 0xF8, 0x0F, 0x08, 0x08, 0xF8, 0x0F, 0x80, 0x00, 0x88, 0x00,
    0xF8, 0x1F, 0x84, 0x00, 0x82, 0x00, 0xF8, 0x0F, 0x80, 0x00, 0x80, 0x00, 0xFE, 0x3F, 0x00, 0x00}; /*星*/
const unsigned char liu[] U8X8_PROGMEM = {
    0x40, 0x00, 0x80, 0x00, 0x00, 0x01, 0x00, 0x01, 0x00, 0x00, 0xFF, 0x7F, 0x00, 0x00, 0x00, 0x00,
    0x20, 0x02, 0x20, 0x04, 0x10, 0x08, 0x10, 0x10, 0x08, 0x10, 0x04, 0x20, 0x02, 0x20, 0x00, 0x00}; /*六*/
uint32_t resetCount = 0;
const char *langlist PROGMEM =
    "简体中文|Simplified Chinese\n"
    "英文(美国)|English(US)";
void setup()
{

    timeZone=8;
    pinMode(D7, INPUT_PULLUP);
    pinMode(D5, OUTPUT);
    Serial.begin(115200);
    Serial.print(F("\nReset reason = "));
    String resetCause = ESP.getResetReason();
    Serial.println(resetCause);
    resetCount = 0;
    LittleFS.begin(); 
    // bool U8G2::begin(menu_select_pin,menu_next_pin,menu_prev_pin,menu_up_pin = U8X8_PIN_NONE,menu_down_pin = U8X8_PIN_NONE, uint8_t menu_home_pin = U8X8_PIN_NONE)
    u8g2.begin(D5, D7, D6);
    u8g2.enableUTF8Print();
    u8g2.setFontMode(0);
    // pinMode(A0,INPUT);
    u8g2.clearBuffer();
    if(lang=="notset")
    {
        u8g2.setFont(u8g2_font_wqy13_t_gb2312a);
        int c1 = u8g2.userInterfaceSelectionList("Language/语言", c1, langlist);
            if(c1 == 1)lang="zh_cn";
            if(c1 == 2)lang="en_us";
    }
    // u8g2.setFont(u8g2_font_unifont_dx);
    u8g2.clearBuffer();
    if(lang=="en_us")
    {
        u8g2.setFont(u8g2_font_wqy13_t_gb2312a);
        u8g2.setCursor(0, 14);
        u8g2.print("Waiting for WiFi");
        u8g2.setCursor(0, 30);
        u8g2.print("Press OK to config");
        u8g2.setCursor(0, 47);
        u8g2.print("CokeeTech " + ver);
        u8g2.sendBuffer();
    }
    else if(lang=="zh_cn")
    {
        u8g2.setFont(u8g2_font_wqy13_t_gb2312a);
        u8g2.setCursor(0, 14);
        u8g2.print("正在连接网络");
        u8g2.setCursor(0, 30);
        u8g2.print("按下OK键来配置网络");
        u8g2.setCursor(0, 47);
        u8g2.print("CokeeTech " + ver);
        u8g2.sendBuffer();
    }
    
    int j = 0;
    WiFi.begin("CokeeNetworks", "666688881234");
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(200);
        if (u8g2.getMenuEvent() == U8X8_MSG_GPIO_MENU_SELECT && !wea)
        {
            Wifiinfo();
            scanWF();    
        }
        if (WiFi.status() == WL_NO_SSID_AVAIL || WiFi.status() == WL_CONNECT_FAILED)
        {
            Wifiinfo();
            scanWF(); 
        }
    }
    u8g2.setFont(u8g2_font_wqy13_t_gb2312a);
    u8g2.clearBuffer();
    if(lang=="zh_cn")
    {
        u8g2.setCursor(0, 14);
        u8g2.print("网络已连接 >_<");
        u8g2.setCursor(0, 30);
        u8g2.print("3 秒后进入主界面");
        u8g2.setCursor(0, 47);
        u8g2.print(WiFi.SSID());
        u8g2.setCursor(0, 64);
        u8g2.print("IP:"+WiFi.localIP().toString());
        u8g2.sendBuffer();
    }
    else if(lang=="en_us")
    {
        u8g2.setCursor(0, 14);
        u8g2.print("Connected! >_<");
        u8g2.setCursor(0, 30);
        u8g2.print("Loading system...");
        u8g2.setCursor(0, 47);
        u8g2.print(WiFi.SSID());
        u8g2.setCursor(0, 64);
        u8g2.print("IP:"+WiFi.localIP().toString());
        u8g2.sendBuffer();
    }
    
    delay(3000);
    Serial.print("timeZone:");
    Serial.print(timeZone);
    Serial.println(WiFi.localIP());
    Udp.begin(localPort);
    setSyncProvider(getNtpTime);
    setSyncInterval(30); //每300秒同步一次时间
    isNTPConnected = true;
    webserver.on("/sw", []()
    {
        if(wea)
            wea=false;
        else
            wea=true;
        webserver.send(200,"text/plain","switch weather ok"); 
    });
    MDNS.begin("cokeetech");
    MDNS.addService("http", "tcp", 80);
    ArduinoOTA.setHostname("CokeeMDP_");
    ArduinoOTA.setPassword("cokeetech@233");
    ArduinoOTA.onStart([]()
    {
        String type;
        if(ArduinoOTA.getCommand()==U_FLASH)
        {
            type="Software";
        }
        else
        {// U_FS
            type="FileSystem";
        }
        u8g2.clearBuffer();
        u8g2.setFont(u8g2_font_helvR10_tr);
        //u8g2.setFont(u8g2_font_unifont_dx);
        u8g2.setCursor(0,14);
        u8g2.print("[OTA]Start updating "+type);
        u8g2.setCursor(0,30);
        u8g2.print("Don't turn off the power");
        u8g2.sendBuffer();
        // NOTE:if updating FS this would be the place to unmount FS using FS.end()
        Serial.println("Start updating "+type); 
    });
    ArduinoOTA.onEnd([]()
    {
        u8g2.clearBuffer();
        u8g2.setCursor(0,14);
        u8g2.print("[OTA]Updating Done");
        u8g2.sendBuffer();
        Serial.println("\nEnd");
        delay(3000); 
    });
    ArduinoOTA.onProgress([](unsigned int progress, unsigned int total)
    {
        u8g2.clearBuffer();
        u8g2.setCursor(0,14);
        u8g2.printf("[OTA]Progress:%u%%\r",(progress/(total/100)));
        u8g2.setCursor(0,30);
        u8g2.print("Don't turn off the power");
        u8g2.sendBuffer();
        Serial.printf("Progress:%u%%\r",(progress/(total/100))); 
    });
    ArduinoOTA.onError([](ota_error_t error)
    {
        String t;
        Serial.printf("Error[%u]:",error);
        if(error==OTA_AUTH_ERROR)
        {
            Serial.println("Auth Failed");
            t="Auth Failed";
        }
        else if(error==OTA_BEGIN_ERROR)
        {
            Serial.println("Begin Failed");
            t="Begin Failed";
        }
        else if(error==OTA_CONNECT_ERROR)
        {
            Serial.println("Connect Failed");
            t="Connect Failed";
        }
        else if(error==OTA_RECEIVE_ERROR)
        {
            Serial.println("Receive Failed");
            t="[!]Receive Failed";
        }
        else if(error==OTA_END_ERROR)
        {
            Serial.println("End Failed");
            t="[!]End Failed";
        }
        u8g2.clearBuffer();
        u8g2.setCursor(0,14);
        u8g2.print("[OTA]Error:");
        u8g2.print(t);
        u8g2.sendBuffer(); 
    });
    ArduinoOTA.begin();
    httpUpdater.setup(&webserver, "/update", "cokee", "cokeetech@233");
    webserver.begin();
    Serial.println("Ready");
    Serial.print("IP address:");
    Serial.println(WiFi.localIP());
    // attachInterrupt(D6,bn,FALLING);
    // sersons.begin();
}

time_t prevDisplay = 0; //当时钟已经显示
void loop()
{
    if (u8g2.getMenuEvent() == U8X8_MSG_GPIO_MENU_SELECT && !wea)
    {
        if (wea)
            return;
        else
            btn_event();
    }
    if (timeStatus() != timeNotSet && !wea)
    {
        if (now() != prevDisplay)
        { //时间改变时更新显示
            prevDisplay = now();
            oledClockDisplay();
        }
    }
    webserver.handleClient();
    ArduinoOTA.handle();
    MDNS.update();
} 
void btn_event()
{
    //u8g2.setFont(u8g2_font_6x12_tr);
    u8g2.setFont(u8g2_font_wqy13_t_gb2312a);
    uint8_t current_selection = 0,c1=0;
    const char *string_list =
        "Weather\n"//1
        "Zhixue Score\n"//2
        "Alarm\n"//3
        "WiFi Info\n"//4
        "WiFi List\n"//5
        "WiFi Config\n"//6
        "TimeZone\n"//7
        "Display Rotation\n"//8
        "Powersave\n"//9
        "Check update\n"//10
        "Language/语言\n";//11
        "About Device\n"//12
        "[Dev]Hitokoto";//13
    const char *string_list_chn =
        "天气\n"//1
        "智学网\n"//2
        "闹钟\n"//3
        "WiFi信息\n"//4
        "WiFi列表\n"//5
        "WiFi配置\n"//6
        "时区\n"//7
        "屏幕旋转\n"//8
        "省电模式\n"//9
        "检查固件更新\n"//10
        "语言Language\n";//11
        "关于本机\n"//12
        "[Dev]Hitokoto";//13
    const char *s9 =
    "0 degree\n"
    "180 degree";
    const char *s9_ch =
    "0 度-正常\n"
    "180 度";
    const char *tzl =
    "UTC+\n"
    "UTC-";
    if(lang=="en_us")current_selection = u8g2.userInterfaceSelectionList("Main Menu",current_selection,string_list);
    else if(lang=="zh_cn")current_selection = u8g2.userInterfaceSelectionList("主菜单",current_selection,string_list_chn);
    if (current_selection == 0)u8g2.userInterfaceMessage("Nothing selected. >_<","",""," ok ");
    else
    {
        switch (current_selection)
        {
        case 1:
            weather();
            break;
        case 3:
            alarm();
            break;
        case 4:
            Wifiinfo();
            break;
        case 5:
            scanWF();
            break;
        case 6:
            WiFi.disconnect();
            break;
        case 7:
        uint8_t tz;   
        current_selection = u8g2.userInterfaceSelectionList("选择/Choose",current_selection,tzl);
        if(current_selection==1)
        {
            if(lang=="en_us")
           {
              u8g2.userInterfaceInputValue("Timezone|UTC+","UTC+",&tz,0,12,2," hours");
           } 
           else if(lang=="zh_cn")
           {
            u8g2.userInterfaceInputValue("时区(世界协调时差)","UTC+",&tz,0,12,2," 小时");
           }
           timeZone=tz;
        }
        else if(current_selection==2)
        {
            if(lang=="en_us")
           {
              u8g2.userInterfaceInputValue("Timezone|UTC-","UTC-",&tz,0,12,2," hours");
           } 
           else if(lang=="zh_cn")
           {
            u8g2.userInterfaceInputValue("时区(世界协调时差)","UTC-",&tz,0,12,2," 小时");
           }
           timeZone=0-tz;
        }
              
            break;
        case 8:
            if(lang=="en_us")c1 = u8g2.userInterfaceSelectionList("Rotation", c1, s9);
            else if(lang=="zh_cn")c1 = u8g2.userInterfaceSelectionList("屏幕旋转", c1, s9_ch);
            if(c1 == 1)u8g2.setDisplayRotation(U8G2_R0);
            if(c1 == 2)u8g2.setDisplayRotation(U8G2_R2);
            break;
        case 9:
            powersave = true;
            u8g2.setPowerSave(1);
            WiFi.shutdown(nv->wss);
            break;
        case 11:
            c1 = u8g2.userInterfaceSelectionList("语言/Language", c1, langlist);
            if(c1 == 1)lang="zh_cn";
            if(c1 == 2)lang="en_us";
            break;
        case 12:
            u8g2.userInterfaceMessage(String(String("DeviceID:") + String(ESP.getChipId())).c_str(), "Cokee Tech.", "MicroDP v3", " ok ");
            break;
        case 13:
            hitoko();
            draw_state_scroll1();
            break;
        default:
            u8g2.userInterfaceMessage(
                "Selection:",
                u8x8_GetStringLineStart(current_selection - 1, string_list),
                "That function is TODO >_<",
                " ok \n cancel ");
            break;
        }
    }
    // scanWF();
}
void Wifiinfo()
{
    String wfs;
      switch (WiFi.status())
      {
        case 0:
          wfs = String("Changing...");
          break;
        case 1:
          wfs = String("NO_SSID");
          break;
        case 3:
          wfs = String("CONNECTED");
          break;
        case 4:
          wfs = String("CN_FAIL");
          break;
        case 6:
          wfs = String("WRONG_PW");
          break;
        case 7:
          wfs = String("DISCONNECTED");
          break;
      }

      u8g2.userInterfaceMessage(
        wfs.c_str(),
        String(WiFi.SSID()).c_str(),
        String(WiFi.RSSI()).c_str(),
        " ok ");
}/*
int SelectionList(String title,int num,const char* list)
{
  int current_pos=0;
  //title start
  for(;;)
  {
    u8g2.setFont(u8g2_font_wqy13_t_gb2312a);
    int y=1;//调节位移速度，可用于加快字符位移速度
    for(int x=0;x<256;x+=y)//x+=y ----->等价于x=x+y
    {
      u8g2.clearBuffer();         // 清除内部缓冲
      u8g2.drawStr(x,10,"Hello World!");  // 第一段输出位置
      u8g2.drawStr(x-128,10,"Hello World!");//第二段输出位置
      delay(20);//延时程序，可以降低位移速度
      u8g2.sendBuffer();          // transfer internal memory to the displa
    }
  
  //title end
    for(;;)
      {
        int event=0;
        event = u8x8_GetMenuEvent();
        if ( event == U8X8_MSG_GPIO_MENU_SELECT )
          return current_pos+1;		
        else if ( event == U8X8_MSG_GPIO_MENU_HOME )
          return 0;
        else if ( event == U8X8_MSG_GPIO_MENU_NEXT || event == U8X8_MSG_GPIO_MENU_DOWN )
        {
          //u8sl_Next(&u8sl);
          break;
        }
        else if ( event == U8X8_MSG_GPIO_MENU_PREV || event == U8X8_MSG_GPIO_MENU_UP )
        {
         // u8sl_Prev(&u8sl);
          break;
        }
      }
  }
}*/
void alarm()
{
    u8g2.setFont(u8g2_font_6x12_tr);
    uint8_t current_selection = 0;
    const char *string_list =
        "[New Alarm]\n";
    // for()

    current_selection = u8g2.userInterfaceSelectionList(
        "Alarm List",
        current_selection,
        string_list);
    if (current_selection == 0)
    {
        u8g2.userInterfaceMessage(
            "Nothing selected.",
            "",
            "",
            " ok ");
    }
    else
    {
        switch (current_selection)
        {
        case 1:
            weather();
            break;
        case 3:
            alarm();
            break;

        default:
            u8g2.userInterfaceMessage(
                "Selection:",
                u8x8_GetStringLineStart(-1, string_list),
                "",
                " ok \n cancel ");
            break;
        }
    }
}
void oledClockDisplay()
{
    if (wea)
        return;
    int years, months, days, hours, minutes, seconds, weekdays;
    years = year();
    months = month();
    days = day();
    hours = hour();
    minutes = minute();
    seconds = second();
    weekdays = weekday();
    // Serial.printf("%d/%d/%d%d:%d:%d Weekday:%d\n",years,months,days,hours,minutes,seconds,weekdays);
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_wqy13_t_gb2312a);
    u8g2.setCursor(0, 14);
    u8g2.print("");
    draw_state_scroll1();
    draw_State();
    u8g2.setFont(u8g2_font_wqy13_t_gb2312a);
    String currentTime = "";
    if (hours < 10)
        currentTime += 0;
    currentTime += hours;
    currentTime += ":";
    if (minutes < 10)
        currentTime += 0;
    currentTime += minutes;
    currentTime += ":";
    if (seconds < 10)
        currentTime += 0;
    currentTime += seconds;
    String currentDay = "";
    currentDay += years;
    currentDay += "/";
    if (months < 10)
        currentDay += 0;
    currentDay += months;
    currentDay += "/";
    if (days < 10)
        currentDay += 0;
    currentDay += days;
    for (int t = 1; t <= 7; t++)
       /* if (ch == hours && cm == minutes && cw[t])
        {
            break;
            dp(true);
        }*/
    u8g2.setFont(u8g2_font_logisoso24_tr);
    u8g2.setCursor(0, 44);
    u8g2.print(currentTime);
    u8g2.setCursor(0, 61);
    u8g2.setFont(u8g2_font_wqy13_t_gb2312a);
    u8g2.print(currentDay);
    u8g2.drawXBM(80, 48, 16, 16, xing);
    u8g2.setCursor(95, 62);
    u8g2.print("期");
    if (weekdays == 1)
        u8g2.print("日");
    else if (weekdays == 2)
        u8g2.print("一");
    else if (weekdays == 3)
        u8g2.print("二");
    else if (weekdays == 4)
        u8g2.print("三");
    else if (weekdays == 5)
        u8g2.print("四");
    else if (weekdays == 6)
        u8g2.print("五");
    else if (weekdays == 7)
        u8g2.drawXBM(111, 49, 16, 16, liu);
    u8g2.sendBuffer();
}

/*--------NTP 代码----------*/

const int NTP_PACKET_SIZE = 48;     // NTP时间在消息的前48个字节里
byte packetBuffer[NTP_PACKET_SIZE]; // 输入输出包的缓冲区

time_t getNtpTime()
{
    IPAddress ntpServerIP; // NTP服务器的地址

    while (Udp.parsePacket() > 0)
        ; // 丢弃以前接收的任何数据包
          // 从池中获取随机服务器
    WiFi.hostByName(ntpServerName, ntpServerIP);
    Serial.print(ntpServerName);
    Serial.print(":");
    Serial.println(ntpServerIP);
    sendNTPpacket(ntpServerIP);
    uint32_t beginWait = millis();
    while (millis() - beginWait < 1500)
    {
        int size = Udp.parsePacket();
        if (size >= NTP_PACKET_SIZE)
        {
            Serial.println("Receive NTP Response");
            isNTPConnected = true;
            Udp.read(packetBuffer, NTP_PACKET_SIZE); // 将数据包读取到缓冲区
            unsigned long secsSince1900;
            // 将从位置40开始的四个字节转换为长整型，只取前32位整数部分
            secsSince1900 = (unsigned long)packetBuffer[40] << 24;
            secsSince1900 |= (unsigned long)packetBuffer[41] << 16;
            secsSince1900 |= (unsigned long)packetBuffer[42] << 8;
            secsSince1900 |= (unsigned long)packetBuffer[43];
            Serial.println(secsSince1900);
            Serial.println(secsSince1900 - 2208988800UL + timeZone * SECS_PER_HOUR);
            return secsSince1900 - 2208988800UL + timeZone * SECS_PER_HOUR;
        }
    }
    Serial.println(F("No NTP Response:-(")); //无NTP响应
    isNTPConnected = false;
    return 0; //如果未得到时间则返回0
}

// 向给定地址的时间服务器发送NTP请求
void sendNTPpacket(IPAddress &address)
{
    memset(packetBuffer, 0, NTP_PACKET_SIZE);
    packetBuffer[0] = 0b11100011; // LI,Version,Mode
    packetBuffer[1] = 0;          // Stratum,or type of clock
    packetBuffer[2] = 6;          // Polling Interval
    packetBuffer[3] = 0xEC;       // Peer Clock Precision
                                  // 8 bytes of zero for Root Delay&Root Dispersion
    packetBuffer[12] = 49;
    packetBuffer[13] = 0x4E;
    packetBuffer[14] = 49;
    packetBuffer[15] = 52;
    Udp.beginPacket(address, 123); // NTP需要使用的UDP端口号为123
    Udp.write(packetBuffer, NTP_PACKET_SIZE);
    Udp.endPacket();
}

void scanWF()
{
  int i;
  String menu;
  uint8_t current_selection = 1;
  u8g2.setFont(u8g2_font_wqy13_t_chinese2);
  wea = true;
  //  wificonfig.enableAP();
  WiFi.scanNetworks();
  while (WiFi.scanComplete() >= 0)
  {

    if (WiFi.scanComplete() == 0)
    {
      u8g2.userInterfaceMessage(
        "No any wifi",
        "",
        "",
        " ok ");
      goto ex;
    }

    u8g2.clearBuffer();
    for (i = 0; i < WiFi.scanComplete(); i++)
    {
      if (WiFi.encryptionType(i) == ENC_TYPE_NONE && WiFi.SSID(current_selection - 1) != DEF_WIFI_SSID)
        menu += "[O]";
      if (i < WiFi.scanComplete())
        menu += String(WiFi.SSID(i)) + "\n";
      else
        menu += String(WiFi.SSID(i));
    }
    menu += "\n[Don't connect]";
    current_selection = u8g2.userInterfaceSelectionList(
                          "WIFI List",
                          current_selection,
                          menu.c_str());

    if (current_selection == 0)
    {
      u8g2.userInterfaceMessage(
        "Nothing selected.",
        "",
        "",
        " OK ");
      goto ex;
    }
    else
    {

      const char*sd = u8x8_GetStringLineStart(current_selection - 1, menu.c_str());
      Serial.println(sd);
      if (sd == "[Don't connect]") {
        offline = true;
        goto ex;
      }
      if (WiFi.encryptionType(current_selection - 1) != ENC_TYPE_NONE)
      {
        String inp = "";
        //u8g2.userInterfaceInputValue("Need Password", "PW:", &inp, 0, 5, 1, " V");
        u8g2.userInterfaceMessage(
          "Password:",
          inp.c_str(),
          "",
          " Connect ");
        Serial.println(inp);
        u8g2.clearBuffer();
        u8g2.print("Try to connect");
        u8g2.sendBuffer();
        WiFi.begin(WiFi.SSID(current_selection - 1), inp);
        unsigned long c = millis();

        while (WiFi.status() != WL_CONNECTED)
        {
          delay(100);
          u8g2.print(".");
          u8g2.sendBuffer();
          if (millis() - c >= 10000)
          {
            wea = false;
            c = 0;
            return;
          }
        }
        wea = false;
        return;
        c = 0;
        /*u8g2.clearBuffer();u8g2.setCursor(0,14);
                  u8g2.print(F("Wifi isn't open!"));u8g2.sendBuffer();delay(3000);*/
      }
      else
      {
        if (WiFi.SSID(current_selection - 1) != DEF_WIFI_SSID)
          WiFi.begin(WiFi.SSID(current_selection - 1), "");
        WiFi.begin(WiFi.SSID(current_selection - 1), "");
        u8g2.clearBuffer();
        u8g2.setCursor(0, 14);
        unsigned long c = millis();
        while (WiFi.status() != WL_CONNECTED)
        {

          delay(100);
          u8g2.print(".");
          u8g2.sendBuffer();
          if (millis() - c >= 10000)
          {
            wea = false;
            c = 0;
            return;
          }
        }
        wea = false;
        c = 0;
        return;
      }
    }
    delay(10);
  }

ex:
  wea = false;
  return;
}
void draw_State() {
  u8g2.setFont(u8g2_font_open_iconic_www_1x_t);
  if (WiFi.status() == WL_CONNECTED) {
    u8g2.drawGlyph(0, 8, 72);
  } else if (WiFi.status() == WL_DISCONNECTED ) {
    if ((millis() % 1000) > 500)
      u8g2.drawGlyph(0, 8, 69);
  } else if (WiFi.status() == WL_NO_SSID_AVAIL ) {
    if ((millis() % 1000) > 500)
      u8g2.drawGlyph(0, 8, 79);
  } else if (WiFi.status() == WL_IDLE_STATUS ) {
    if ((millis() % 1000) > 500)
      u8g2.drawGlyph(0, 8, 81);
  }

  //show weather state

}
int C_F;
int16_t OFFSET = -(int16_t)u8g2.getDisplayWidth();;
void drawScrollString1(int x_offset, int16_t offset, const char *s) {
  static char buf[36];  // should for screen with up to 256 pixel width
  size_t len;
  size_t char_offset = 0;
  u8g2_uint_t dx = 0;
  size_t visible = 0;
  len = strlen(s);
  if ( offset < 0 )
  {
    char_offset = (-offset) / 6;
    dx = offset + char_offset * 6;
    if ( char_offset >= u8g2.getDisplayWidth() / 6 )
      return;
    visible = u8g2.getDisplayWidth() / 6 - char_offset + 1;
    strncpy(buf, s, visible);
    buf[visible] = '\0';
    //u8g2.setFont(u8g2_font_6x10_mf);
    u8g2.setFont(u8g2_font_wqy12_t_chinese2);
    u8g2.drawStr(char_offset * 6 - dx + x_offset, 7, buf);
  }
  else
  {
    char_offset = offset / 6;
    if ( char_offset >= len )
      return; // nothing visible
    dx = offset - char_offset * 6;
    visible = len - char_offset;
    if ( visible > u8g2.getDisplayWidth() / 6 + 1 )
      visible = u8g2.getDisplayWidth() / 6 + 1;
    strncpy(buf, s + char_offset, visible);
    buf[visible] = '\0';
    //u8g2.setFont(u8g2_font_6x10_mf);
    u8g2.setFont(u8g2_font_wqy12_t_chinese2);
    u8g2.drawStr(-dx + x_offset, 7, buf);

  }

}

void draw_state_scroll1() {
  drawScrollString1(16, -(int16_t)u8g2.getDisplayWidth() , hitok.c_str()/*"我们各自努力，最高处见。"*/);
  int16_t len = strlen(hitok.c_str());
  OFFSET += 2;
  if ( OFFSET > len * 8 + 1 ) {
    C_F = 1;
    hitoko();
    OFFSET = -(int16_t)u8g2.getDisplayWidth();
  }

}

void weather()
{
    WiFiClient aclient;
    String json_from_server, city = "Fuyang";
    String url = "http://api.seniverse.com/v3/weather/daily.json?key=SISi82MwzaMbmQqSh&location=fuyang&language=zh-Hans&unit=c&start=0&days=3";
    // http://open.iciba.com/dsapi/
    // https://api.ooopn.com/ciba/api.php
    HTTPClient http;

    Serial.print("[HTTP] begin...\n");
    if (http.begin(aclient, url.c_str()))
    { // HTTP

        Serial.print("[HTTP] GET...\n");
        // start connection and send HTTP header
        int httpCode = http.GET();

        // httpCode will be negative on error
        if (httpCode > 0)
        {
            // HTTP header has been send and Server response header has been handled
            Serial.printf("[HTTP] GET... code:%d\n", httpCode);

            // file found at server
            if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY)
            {
                json_from_server = http.getString();
                Serial.println(json_from_server);
            }
        }
        else
        {
            Serial.printf("[HTTP] GET... failed,error:%s\n", http.errorToString(httpCode).c_str());
        }

        http.end();
    }
    else
    {
        Serial.printf("[HTTP} Unable to connect\n");
        return;
    }
    const size_t capacity = JSON_ARRAY_SIZE(1) + JSON_ARRAY_SIZE(3) + JSON_OBJECT_SIZE(1) + JSON_OBJECT_SIZE(3) + JSON_OBJECT_SIZE(6) + 3 * JSON_OBJECT_SIZE(14) + 800;
    DynamicJsonDocument doc(capacity);
    deserializeJson(doc, json_from_server.c_str());

    JsonObject results_0 = doc["results"][0];
    JsonObject results_0_location = results_0["location"];
    const char *results_0_location_name = results_0_location["name"];                       // "阜阳"
    const char *results_0_location_country = results_0_location["country"];                 // "CN"
    const char *results_0_location_path = results_0_location["path"];                       // "阜阳,阜阳,安徽,中国"
    const char *results_0_location_timezone = results_0_location["timezone"];               // "Asia/Shanghai"
    const char *results_0_location_timezone_offset = results_0_location["timezone_offset"]; // "+08:00"

    JsonArray results_0_daily = results_0["daily"];

    JsonObject results_0_daily_0 = results_0_daily[0];
    const char *results_0_daily_0_date = results_0_daily_0["date"];             // "2020-04-04"
    const char *results_0_daily_0_text_day = results_0_daily_0["text_day"];     // "多云"
    const char *results_0_daily_0_code_day = results_0_daily_0["code_day"];     // "4"
    const char *results_0_daily_0_text_night = results_0_daily_0["text_night"]; // "多云"
    const char *results_0_daily_0_code_night = results_0_daily_0["code_night"]; // "4"
    const char *results_0_daily_0_high = results_0_daily_0["high"];             // "21"
    const char *results_0_daily_0_low = results_0_daily_0["low"];               // "6"
    const char *results_0_daily_0_rainfall = results_0_daily_0["rainfall"];     // "0.0"
    const char *results_0_daily_0_humidity = results_0_daily_0["humidity"];     // "62"

    JsonObject results_0_daily_1 = results_0_daily[1];
    const char *results_0_daily_1_date = results_0_daily_1["date"];             // "2020-04-05"
    const char *results_0_daily_1_text_day = results_0_daily_1["text_day"];     // "多云"
    const char *results_0_daily_1_code_day = results_0_daily_1["code_day"];     // "4"
    const char *results_0_daily_1_text_night = results_0_daily_1["text_night"]; // "阴"
    const char *results_0_daily_1_code_night = results_0_daily_1["code_night"]; // "9"
    const char *results_0_daily_1_high = results_0_daily_1["high"];             // "17"
    const char *results_0_daily_1_low = results_0_daily_1["low"];               // "7"
    const char *results_0_daily_1_rainfall = results_0_daily_1["rainfall"];     // "0.0"
    const char *results_0_daily_1_humidity = results_0_daily_1["humidity"];     // "47"

    JsonObject results_0_daily_2 = results_0_daily[2];
    const char *results_0_daily_2_date = results_0_daily_2["date"];             // "2020-04-06"
    const char *results_0_daily_2_text_day = results_0_daily_2["text_day"];     // "晴"
    const char *results_0_daily_2_code_day = results_0_daily_2["code_day"];     // "0"
    const char *results_0_daily_2_text_night = results_0_daily_2["text_night"]; // "多云"
    const char *results_0_daily_2_code_night = results_0_daily_2["code_night"]; // "4"
    const char *results_0_daily_2_high = results_0_daily_2["high"];             // "19"
    const char *results_0_daily_2_low = results_0_daily_2["low"];               // "8"
    const char *results_0_daily_2_rainfall = results_0_daily_2["rainfall"];     // "0.0"
    const char *results_0_daily_2_humidity = results_0_daily_2["humidity"];     // "48"

    const char *results_0_last_update = results_0["last_update"]; // "2020-04-04T17:25:51+08:00"

    int wtemp[3] = {(atoi(results_0_daily_0_high) + atoi(results_0_daily_0_low)) / 2, (atoi(results_0_daily_1_high) + atoi(results_0_daily_1_low)) / 2, (atoi(results_0_daily_2_high) + atoi(results_0_daily_2_low)) / 2};
    int tdst[3] = {atoi(results_0_daily_0_code_day), atoi(results_0_daily_1_code_day), atoi(results_0_daily_2_code_day)};
    String wtext[3] = {city + " today's weather:" + String(results_0_daily_0_text_day), city + " tomorrow's weather:" + String(results_0_daily_1_text_day), city + " day after tomorrow's weather:" + String(results_0_daily_2_text_day)};

    Serial.println();
    Serial.println(wtext[0]);
    Serial.println(wtext[1]);
    Serial.println(wtext[2]);
    draw(wtext[0].c_str(), pdweather(tdst[0]), wtemp[0]);
    // Wait 5 seconds
    delay(2000);
    draw(wtext[1].c_str(), pdweather(tdst[1]), wtemp[1]);
    delay(2000);
    draw(wtext[2].c_str(), pdweather(tdst[2]), wtemp[2]);
    delay(5000);
    wea = false;
    /*#define SUN  0
        #define SUN_CLOUD  1
        #define CLOUD 2
        #define RAIN 3
        #define SNOW 5
        #define MIST 6//雾
        #define MOON 7*/
}
int pdweather(int ctd)
{
    int tds = 0;
    if (ctd == 0 || ctd == 3 || ctd == 38)
    {
        tds = SUN;
    }
    else if (ctd == 1 || ctd == 2 || ctd == 38)
    {
        tds = MOON;
    }
    else if (ctd >= 4 && ctd <= 8)
    {
        tds = SUN_CLOUD;
    }
    else if (ctd >= 10 && ctd <= 25)
    {
        tds = RAIN;
    }
    else if (ctd == 9)
    {
        tds = CLOUD;
    }
    else if (ctd == 30 || ctd == 31)
    {
        tds = MIST;
    }
    return tds;
}
void drawWeatherSymbol(u8g2_uint_t x, u8g2_uint_t y, uint8_t symbol)
{
    // fonts used:
    // u8g2_font_open_iconic_embedded_6x_t
    // u8g2_font_open_iconic_weather_6x_t
    // encoding values,see:https://github.com/olikraus/u8g2/wiki/fntgrpiconic

    switch (symbol)

    {

    case SUN:

        u8g2.setFont(u8g2_font_open_iconic_weather_6x_t);

        u8g2.drawGlyph(x, y, 69);

        break;

    case SUN_CLOUD:

        u8g2.setFont(u8g2_font_open_iconic_weather_6x_t);

        u8g2.drawGlyph(x, y, 65);

        break;

    case CLOUD:

        u8g2.setFont(u8g2_font_open_iconic_weather_6x_t);

        u8g2.drawGlyph(x, y, 64);

        break;

    case RAIN:

        u8g2.setFont(u8g2_font_open_iconic_weather_6x_t);

        u8g2.drawGlyph(x, y, 67);

        break;

    case THUNDER: //雷

        u8g2.setFont(u8g2_font_open_iconic_embedded_6x_t);

        u8g2.drawGlyph(x, y, 67);

        break;

    case SNOW: //雪

        u8g2.setFont(u8g2_font_open_iconic_weather_6x_t);

        u8g2.drawGlyph(x, y, 68);

        break;

    case MIST: //雾

        u8g2.setFont(u8g2_font_open_iconic_text_6x_t);

        u8g2.drawGlyph(x, y, 64);

        break;

    case MOON:

        u8g2.setFont(u8g2_font_open_iconic_weather_6x_t);

        u8g2.drawGlyph(x, y, 66);

        break;
    }
}

void drawWeather(uint8_t symbol, int degree)
{
    drawWeatherSymbol(0, 48, symbol);
    u8g2.setFont(u8g2_font_logisoso32_tf);
    u8g2.setCursor(48 + 3, 42);
    u8g2.print(degree);
    u8g2.print("°C"); // requires enableUTF8Print()
}
/*
  Draw a string with specified pixel offset.
  The offset can be negative.
  Limitation:The monochrome font with 8 pixel per glyph
*/
void drawScrollString(int16_t offset, const char *s)
{
    static char buf[36]; // should for screen with up to 256 pixel width
    size_t len;
    size_t char_offset = 0;
    u8g2_uint_t dx = 0;
    size_t visible = 0;
    len = strlen(s);
    if (offset < 0)
    {
        char_offset = (-offset) / 8;
        dx = offset + char_offset * 8;
        if (char_offset >= u8g2.getDisplayWidth() / 8)
            return;
        visible = u8g2.getDisplayWidth() / 8 - char_offset + 1;
        strncpy(buf, s, visible);
        buf[visible] = '\0';
        // u8g2.setFont(u8g2_font_8x13_mf);
        u8g2.enableUTF8Print();
        u8g2.setFont(u8g2_font_unifont_dx);
        u8g2.drawUTF8(char_offset * 8 - dx, 62, buf);
    }
    else
    {
        char_offset = offset / 8;
        if (char_offset >= len)
            return; // nothing visible
        dx = offset - char_offset * 8;
        visible = len - char_offset;
        if (visible > u8g2.getDisplayWidth() / 8 + 1)
            visible = u8g2.getDisplayWidth() / 8 + 1;
        strncpy(buf, s + char_offset, visible);
        buf[visible] = '\0';
        // u8g2.setFont(u8g2_font_8x13_mf);
        u8g2.enableUTF8Print();
        u8g2.setFont(u8g2_font_unifont_dx);

        u8g2.drawUTF8(-dx, 62, buf);
    }
}

void draw(const char *s, uint8_t symbol, int degree)
{
    int16_t offset = -(int16_t)u8g2.getDisplayWidth();
    int16_t len = strlen(s);
    for (;;)
    {
        u8g2.firstPage();
        do
        {
            drawWeather(symbol, degree);
            drawScrollString(offset, s);
        } while (u8g2.nextPage());
        delay(20);
        offset += 2;
        if (offset > len * 8 + 1)
            break;
    }
}
void hitoko() {
  //http://api.guaqb.cn/v1/onesaid/
  HTTPClient Client1;
  String url = "http://api.guaqb.cn/v1/onesaid/";
  String httpString;
  if (Client1.begin(client, url.c_str()))
  { // HTTP

    Serial.print("[HTTP] GET...\n");
    // start connection and send HTTP header
    int httpCode = Client1.GET();

    // httpCode will be negative on error
    if (httpCode > 0)
    {
      // HTTP header has been send and Server response header has been handled
      Serial.printf("[HTTP] GET... code:%d\n", httpCode);

      // file found at server
      if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY)
      {
        hitok = Client1.getString();
        //hitok = String(dsr) + "天" + Client1.getString();
        Serial.println(hitok);
      }
    }
    else
    {
      Serial.printf("[HTTP] GET... failed,error:%s\n", Client1.errorToString(httpCode).c_str());
    }

    Client1.end();
  }
}
/*
  draw("What a beautiful day!",SUN,27);
  draw("The sun's come out!",SUN_CLOUD,19);
  draw("It's raining cats and dogs.",RAIN,8);
  draw("That sounds like thunder.",THUNDER,12);
  draw("It's stopped raining",CLOUD,15);
*/