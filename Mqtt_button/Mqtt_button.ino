/*
 * ***********************************新的芯片需要更改信息************************************
*/


//-----------------------------A-1 库引用开始-----------------//
#include <string.h>
#include <math.h>  // 数学工具库
#include <EEPROM.h>// eeprom  存储库

// WIFI库
#include <ESP8266WiFi.h>
//-------客户端---------
#include <ESP8266HTTPClient.h>
//---------服务器---------
#include <ESP8266WebServer.h>  
#include <FS.h> 

//--------MQTT
#include <PubSubClient.h>//MQTT库
WiFiClient espClient;
PubSubClient client(espClient);

//ESP获取自身ID
#ifdef ESP8266
extern "C" {
#include "user_interface.h"   //含有system_get_chip_id（）的库
}
#endif

//-----------------------------A-1 库引用结束-----------------//

//------------------------------------------- 修改 -------------------------------------------
// MQTT服务器
#define MQTT_SERVER "www.dongvdong.top"
#define PORT 1883
// 准入MQTT服务器的账号密码
#define MQTT_MYNAME      "love_00001_x"        //修改自己的名称
#define MQTT_SSID        "love_00001_x"  //修改账号
#define MQTT_PASSWORD    "love_00001_x"  //修改密码
#define MQTT_MYTOPIC     "lovelamp/love_00001_x/"  
/*
话题格式：
接收   lovelamp/+MQTT_MYNAME+"/r"     lovelamp/love_00001_x/r
发布   lovelamp/+MQTT_MYNAME+"/s"     lovelamp/love_00001_x/s
*/




 //---------------------------------------------------------------------------------------------//




ESP8266WebServer server ( 80 );  
//储存SN号
String SN;

/*----------------WIFI账号和密码--------------*/
 
#define DEFAULT_STASSID "lovelamp"
#define DEFAULT_STAPSW  "lovelamp" 

//下图说明了Arduino环境中使用的Flash布局：

//|--------------|-------|---------------|--|--|--|--|--|
//^              ^       ^               ^     ^
//Sketch    OTA update   File system   EEPROM  WiFi config (SDK)


// 用于存上次的WIFI和密码
#define MAGIC_NUMBER 0xAA
struct config_type
{
  char stassid[50]; //WIFI账号
  char stapsw[50];  //WIFI密码
  uint8_t magic;

  uint8_t mqtt_bool;  //是否有MQTT消息储存
  char mqtt_sever[50]; //www.dongvdong.top
  int  mqtt_port;  //1883
  char mqtt_nameid[100];//MQTT分配的唯一client id 
  char mqtt_ssid[20];  //MQTT唯一登陆账号
  char mqtt_psw[20];   //MQTT唯一登录密码
  char mqtt_topic[100]; //MQTT唯一接收和发布话题  接收/r   发送/s 
  
};
config_type config_wifi;


// 自己的话题 
String mqtt_mytopic_s= String()+ String(config_wifi.mqtt_topic)+"s";
String mqtt_mytopic_r= String()+ String(config_wifi.mqtt_topic)+"r";

//  网页
#define MQTT_MYTOPICS  mqtt_mytopic_s.c_str()
#define MQTT_MYTOPICR  mqtt_mytopic_r.c_str()
 
//------是否开启打印-----------------
#define Use_Serial Serial
#define smartLED D4 



//-----------------------------A-2 变量声明开始-----------------//
int PIN_Led = D4;
int PIN_Led_light = 0; 
bool PIN_Led_State=0;

int workmode=0;
int led_sudu=80;
/************************ 按键中断7**********************************/
//按键中断
int PIN_Led_Key= D2 ;// 不要上拉电阻

 
int sendbegin=0;         // 0  没发送  1 发送短消息  2 发送长消息
char sendmsg[100];// 发送话题-原始
String sendstr; // 接收消息-转换
int recbegin=0;         // 0  没接收  1 接收短消息  2 接收长消息
char recmsg[50];// 接收消息-原始
String recstr; // 接收消息-转换


//--------------HTTP请求------------------
struct http_request {  
  String  Referer;
  char host[20];
  int httpPort=80;
  String host_ur ;
  
  String usr_name;//账号
  String usr_pwd;//密码
 
  String postDate;

  };

String http_html;
//-----------------------------A-2 变量声明结束-----------------//


//-----------------------------A-3 函数声明开始-----------------//
void get_espid();
void LED_Int();

void Button_Int();
void highInterrupt();
void lowInterrupt();
void waitKey();

void wifi_Init();
void saveConfig();
void loadConfig();

void SET_AP();
int hdulogin(struct http_request ruqest) ;
void http_wifi_xidian();
String getContentType(String filename);
void handleNotFound();
void handleMain();
void handlePin();
void handleWifi();
void http_jiexi(http_request ruqest);
void handleWifi_wangye();
void handleTest();

void handleMqttConfig();

void smartConfig();//500闪烁
void Mqtt_message();
void mqtt_int();
void mqtt_reconnect() ;


//---------------------------A-3 函数声明结束------------------------//


//-----------------------------A-3 函数-----------------//
//3-1管脚初始化
void get_espid(){
   SN = (String )system_get_chip_id();
  Use_Serial.println(SN);
  
  }
void LED_Int(){
  
   pinMode(D4, OUTPUT); 
  
  }

/*************************** 3-2 按键中断函数执行*****************************/

// 高电平触发
void highInterrupt(){ 

  
   }
void lowInterrupt(){
 
  
  }




  /*************************** 2-2 按键LED函数初始化（）*****************************/ 
void Button_Int(){
 pinMode(PIN_Led_Key, INPUT);
 attachInterrupt(PIN_Led_Key, highInterrupt, RISING);
  }



void smartConfig()
{
  led_sudu=30;
  Serial.println("Start smartConfig module");
  pinMode(smartLED, OUTPUT);
  digitalWrite(smartLED, 0);
   
  WiFi.mode(WIFI_STA);
  Serial.println("\r\nWait for Smartconfig");
  WiFi.stopSmartConfig();
  WiFi.beginSmartConfig();

        unsigned long preTick = millis();
      int num = 0;
  while (1)
 {  
       
    if (millis() - preTick < 10 ) continue;//等待10ms
    preTick = millis();
    num++;
    if (num % led_sudu == 0)  //50*10=500ms=0.5s 反转一次
    {
        PIN_Led_State=!PIN_Led_State;
    Serial.print(".");             
    digitalWrite(smartLED, PIN_Led_State);
     
          }
    if (WiFi.smartConfigDone())
    {  Serial.println(" ");
      Serial.println("SmartConfig Success");      
      strcpy(config_wifi.stassid, WiFi.SSID().c_str());
      strcpy(config_wifi.stapsw, WiFi.psk().c_str());      
      Serial.printf("SSID:%s\r\n", WiFi.SSID().c_str());
      Serial.printf("PSW:%s\r\n", WiFi.psk().c_str());
      saveConfig();
      break;
    }
   
  }
}          

//等待按键动作  
/*************************6-1 上电出始等待 *************************************/
void waitKey()
{
  pinMode (PIN_Led, OUTPUT);
  pinMode (PIN_Led_Key , INPUT);
  digitalWrite(PIN_Led, 0);
  Serial.println(" press key 2s: smartconfig mode! \r\n press key 0s: connect last time wifi!");
      
  char keyCnt = 0;
  unsigned long preTick = millis();
  unsigned long preTick2 = millis();
  int num = 0;
  while (1)
  { 
    ESP.wdtFeed();
   
    if (millis() - preTick < 10 ) continue;//等待10ms
    preTick = millis();
    num++;
    if (num % led_sudu == 0 )  //50*10=500ms=0.5s 反转一次
    {
      PIN_Led_State = !PIN_Led_State;
      digitalWrite(PIN_Led, PIN_Led_State);
    
      Serial.print(".");
    }
     
     if(  workmode!=1 && digitalRead(PIN_Led_Key) == 1){
      // 按键触摸大于2S  进入自动配网模式
    if ( keyCnt >= 200 ) // 200*10=2000ms=2s  大于2s反转
    { //按2S 进入一键配置
       workmode=1;
      Serial.println("\r\n Short Press key");

            //  smartConfig();  workmode=0;

             SET_AP();       // 建立WIFI
             SPIFFS.begin(); // ESP8266自身文件系统 启用 此方法装入SPIFFS文件系统。必须在使用任何其他FS API之前调用它。如果文件系统已成功装入，则返回true，否则返回false。
             Server_int();  // HTTP服务器开启
             led_sudu=30;
    }
     }

   
      // 不按按键，自动连接上传WIFI
    if (millis() - preTick2 > 5000 && digitalRead(PIN_Led_Key) == 0) {   // 大于5S还灭有触摸，直接进入配网
       Serial.println("\r\n 10s timeout!");
            if(  workmode==0){ 
              
               wifi_Init();           
               WiFi.mode(WIFI_STA);
             
            
            }          
            return; 
            }
                                  
    
    if (digitalRead(PIN_Led_Key) == 1){ keyCnt++;}
    else{keyCnt = 0;}
    
  }

   Serial.println("\r\n wait key over!");  
  digitalWrite(PIN_Led, 0);
  digitalWrite(PIN_Led_Key, 0);
  
}


//3-2WIFI初始化
void wifi_Init(){
  led_sudu=80;
 // WiFi.mode(WIFI_STA);
  WiFi.mode(WIFI_AP_STA);
  workmode=0; 
  Use_Serial.println("Connecting to ");
  Use_Serial.println(config_wifi.stassid);
  WiFi.begin(config_wifi.stassid, config_wifi.stapsw);
   server.handleClient();   
  unsigned long preTick = millis();    
 int num;

  while (WiFi.status() != WL_CONNECTED) {
  
   if (millis() - preTick < 10 ) continue;//等待10ms
  preTick = millis();
   num++;
       
   server.handleClient();   
  
  ESP.wdtFeed();
  
 // delay(pwm_wifi_connect);

  if(num% led_sudu==0){
 
  PIN_Led_State = !PIN_Led_State;
  digitalWrite(PIN_Led, PIN_Led_State);
 
  Use_Serial.print(".");
  num=0;
  }

  
 
  }
  //WiFi.mode(WIFI_STA);
  Use_Serial.println("--------------WIFI CONNECT!-------------  ");
  Use_Serial.printf("SSID:%s\r\n", WiFi.SSID().c_str());
  Use_Serial.printf("PSW:%s\r\n", WiFi.psk().c_str());
  Use_Serial.println("----------------------------------------  ");
//  workmode=0; 
 
  }



/*
 *3-/2 保存参数到EEPROM
*/
void saveConfig()
{
  Serial.println("Save config!");
  Serial.print("WIFI_NAME:");
  Serial.println(config_wifi.stassid);
  Serial.print("WIFI_PSW:");
  Serial.println(config_wifi.stapsw);
  
  Serial.println("MQTT:");
 // Serial.println(config_wifi.mqtt_bool);
  Serial.println(config_wifi.mqtt_sever);
  Serial.println(config_wifi.mqtt_port);
  Serial.println(config_wifi.mqtt_nameid);
  Serial.println(config_wifi.mqtt_ssid);
  Serial.println(config_wifi.mqtt_psw);
  Serial.println(config_wifi.mqtt_topic);
  
  EEPROM.begin(1024*2);//一共4K
  uint8_t *p = (uint8_t*)(&config_wifi);
  for (int i = 0; i < sizeof(config_wifi); i++)
  {
    EEPROM.write(i, *(p + i));
  }
  EEPROM.commit();
}
/*
 * 从EEPROM加载参数
*/
void loadConfig()
{
  EEPROM.begin(1024*2);
  uint8_t *p = (uint8_t*)(&config_wifi);
  for (int i = 0; i < sizeof(config_wifi); i++)
  {
    *(p + i) = EEPROM.read(i);
  }
  EEPROM.commit();
  //出厂自带
  if (config_wifi.magic != MAGIC_NUMBER)
  {
    strcpy(config_wifi.stassid, DEFAULT_STASSID);
    strcpy(config_wifi.stapsw, DEFAULT_STAPSW);
    config_wifi.magic = MAGIC_NUMBER;
    Serial.println("Restore config!");
  }

  Serial.println("-----------------Read wifi config!----------------");
  Serial.print("stassid:");
  Serial.println(config_wifi.stassid);
  Serial.print("stapsw:");
  Serial.println(config_wifi.stapsw);
  Serial.println("--------------------------------------------------");  
  ESP.wdtFeed();
if (config_wifi.mqtt_bool !=MAGIC_NUMBER){
      config_wifi.mqtt_bool = MAGIC_NUMBER;
      strcpy(config_wifi.mqtt_sever, MQTT_SERVER); 
//       strcpy(config_wifi.mqtt_port, PORT); 
        config_wifi.mqtt_port=1883;
          strcpy(config_wifi.mqtt_nameid, MQTT_MYNAME);
            strcpy(config_wifi.mqtt_ssid, MQTT_SSID);
              strcpy(config_wifi.mqtt_psw, MQTT_PASSWORD);
                strcpy(config_wifi.mqtt_topic, MQTT_MYTOPIC);
    Serial.println("MQTT config!");
  }
  Serial.println("-----------------Read mqtt config!----------------");
  Serial.println(config_wifi.mqtt_sever);
  Serial.println(config_wifi.mqtt_port);
  Serial.println(config_wifi.mqtt_nameid);
  Serial.println(config_wifi.mqtt_ssid);
  Serial.println(config_wifi.mqtt_psw);
  Serial.println(config_wifi.mqtt_topic);
  Serial.println("--------------------------------------------------");  
  
  saveConfig();
 
}
 
  
//3-3ESP8266建立无线热点
void SET_AP(){
  WiFi.mode(WIFI_AP);
 // WiFi.mode(WIFI_AP_STA);
  // 设置内网
  IPAddress softLocal(192,168,4,1);   // 1 设置内网WIFI IP地址
  IPAddress softGateway(192,168,4,1);
  IPAddress softSubnet(255,255,255,0);
  WiFi.softAPConfig(softLocal, softGateway, softSubnet);
   
  String apName = ("Lovelamp_"+(String)ESP.getChipId());  // 2 设置WIFI名称
  const char *softAPName = apName.c_str();
   
  WiFi.softAP(softAPName, "admin");      // 3创建wifi  名称 +密码 admin

  Use_Serial.print("softAPName: ");  // 5输出WIFI 名称
  Use_Serial.println(apName);

  IPAddress myIP = WiFi.softAPIP();  // 4输出创建的WIFI IP地址
  Use_Serial.print("AP IP address: ");     
  Use_Serial.println(myIP);
  
  }
//3-4ESP建立网页服务器
void Server_int(){
  
   server.on ("/", handleMain); // 返回WIFI配网网页
   server.on ("/pin", HTTP_GET, handlePin); // 绑定‘/pin’地址到handlePin方法处理  ---- 开关灯请求 
   server.on ("/wifi", HTTP_GET, handleWifi); // 绑定‘/wifi’地址到handlePWIFI方法处理  --- 重新配网请求
   server.on ("/wifi_wangye", HTTP_GET, handleWifi_wangye);// 连接网页配网
   
   server.on ("/mqtt_html", HTTP_GET, handleMqttConfig);//返回MQTT配网网页
   server.on ("/mqtt", HTTP_GET, handleMqtt);//配置MQTT级本消息 
      
   server.on ("/test", HTTP_GET, handleTest);
   server.onNotFound ( handleNotFound ); // NotFound处理
   server.begin(); 
   
   Use_Serial.println ( "HTTP server started" );
  
  }

void handleMqtt(){
  
  File file = SPIFFS.open("/mqtt.html", "r");  

  size_t sent = server.streamFile(file, "text/html");  

  file.close();  

  return;  
  
  }
  
//3-5-1 网页服务器主页
void handleMain() {  

  /* 返回信息给浏览器（状态码，Content-type， 内容） 
   * 这里是访问当前设备ip直接返回一个String 
   */  

  Serial.print("handleMain");  

//打开一个文件。path应该是以斜线开头的绝对路径（例如/dir/filename.txt）。mode是一个指定访问模式的字符串。
//它可以是“r”，“w”，“a”，“r +”，“w +”，“a +”之一。这些模式的含义与fopenC功能相同
  File file = SPIFFS.open("/index.html", "r");  

  size_t sent = server.streamFile(file, "text/html");  

  file.close();  

  return;  

}  
//3-5-2 网页控制引脚
/* 引脚更改处理 
 * 访问地址为htp://192.162.xxx.xxx/pin?a=XXX的时候根据a的值来进行对应的处理 
 */  

void handlePin() {  

  if(server.hasArg("a")) { // 请求中是否包含有a的参数  

    String action = server.arg("a"); // 获得a参数的值  

    if(action == "on") { // a=on  

      digitalWrite(2, LOW); // 点亮8266上的蓝色led，led是低电平驱动，需要拉低才能亮  

      server.send ( 200, "text/html", "测试灯点亮！"); return; // 返回数据  

    } else if(action == "off") { // a=off  

      digitalWrite(2, HIGH); // 熄灭板载led  

      server.send ( 200, "text/html", "测试灯熄灭！"); return;  

    }  

    server.send ( 200, "text/html", "未知操作！"); return;  

  }  

  server.send ( 200, "text/html", "action no found");  

}  

//3-5-3 网页修改普通家庭WIFI连接账号密码
/* WIFI更改处理 
 * 访问地址为htp://192.162.xxx.xxx/wifi?config=on&name=Testwifi&pwd=123456
  根据wifi进入 WIFI数据处理函数
  根据config的值来进行 on
  根据name的值来进行  wifi名字传输
  根据pwd的值来进行   wifi密码传输
 */  

void handleWifi(){
  
  
   if(server.hasArg("config")) { // 请求中是否包含有a的参数  

    String config = server.arg("config"); // 获得a参数的值  
        String wifiname;
        String wifipwd;

     
        
    if(config == "on") { // a=on  
          if(server.hasArg("name")) { // 请求中是否包含有a的参数  
        wifiname = server.arg("name"); // 获得a参数的值

          }
          
    if(server.hasArg("pwd")) { // 请求中是否包含有a的参数  
         wifipwd = server.arg("pwd"); // 获得a参数的值    
           }
                  
          String backtxt= String()+" 无线WIFI\""+wifiname+"\"配置成功! \n\r 请重启！" ;// 用于串口和网页返回信息
          
          Use_Serial.println ( backtxt); // 串口打印给电脑
          
         // server.send ( 200, "text/html", backtxt); // 网页返回给手机提示
           // wifi连接开始

         wifiname.toCharArray(config_wifi.stassid, 50);    // 从网页得到的 WIFI名
         wifipwd.toCharArray(config_wifi.stapsw, 50);  //从网页得到的 WIFI密码               
         
         saveConfig();
         server.send ( 200, "text/html", backtxt); // 网页返回给手机提示  
         //ESP.reset();
         wifi_Init();
    
         return;          
           

    } else if(config == "off") { // a=off  
                server.send ( 200, "text/html", "config  is off!");
        return;

    }  

    server.send ( 200, "text/html", "unknown action"); return;  

  }  

  server.send ( 200, "text/html", "action no found");  
  
  }

//3-5-5-1 网页认证上网
void handleWifi_wangye(){
  if(server.hasArg("config")) { // 请求中是否包含有a的参数  
// 1 解析数据是否正常
    String config = server.arg("config"); // 获得a参数的值  
        String wifi_wname,wifi_wpwd,wifi_wip,wifi_postdata;
          
  if(config == "on") { // a=on  
        if(server.hasArg("wifi_wname")) { // 请求中是否包含有a的参数  
        wifi_wname = server.arg("wifi_wname"); // 获得a参数的值
           if(wifi_wname.length()==0){            
             server.send ( 200, "text/html", "please input WIFI名称!"); // 网页返回给手机提示
            return;} 
          }
        if(server.hasArg("wifi_wpwd")) { // 请求中是否包含有a的参数  
        wifi_wpwd = server.arg("wifi_wpwd"); // 获得a参数的值
               
          }
     if(server.hasArg("wifi_wip")) { // 请求中是否包含有a的参数  
         wifi_wip = server.arg("wifi_wip"); // 获得a参数的值    
          if(wifi_wip.length()==0){            
             server.send ( 200, "text/html", "please input 登陆网址!"); // 网页返回给手机提示
            return;} 

           }
        if(server.hasArg("wifi_postdata")) { // 请求中是否包含有a的参数  

            String message=server.arg(3);
            for (uint8_t i=4; i<server.args(); i++){
               message += "&" +server.argName(i) + "=" + server.arg(i) ;
             }
  
         wifi_postdata = message; // 获得a参数的值 
            if(wifi_postdata.length()==0){            
             server.send ( 200, "text/html", "please input 联网信息!"); // 网页返回给手机提示
            return;}    
           }
       
//--------------------------------- wifi连接-------------------------------------------------
         wifi_wname.toCharArray(config_wifi.stassid, 50);    // 从网页得到的 WIFI名
         wifi_wpwd.toCharArray(config_wifi.stapsw, 50);  //从网页得到的 WIFI密码               
         
         saveConfig();
      
         // wifi连接开始
         wifi_Init();
   
  //--------------------------------- 认证上网-------------------------------------------------      


  
// 2 发送认证
                
// ruqest.Referer="http://10.255.44.33/srun_portal_pc.php";
  http_request ruqest_http;
  ruqest_http.httpPort = 80; 
  ruqest_http.Referer=wifi_wip;// 登录网址
  ruqest_http.postDate=wifi_postdata;
   
   Use_Serial.println (wifi_wip);
   Use_Serial.println (wifi_postdata);
  

    
  http_jiexi(ruqest_http);
          
           return; 
  } else if(config == "off") { // a=off  
                server.send ( 200, "text/html", "config  is off!");
        return;
  }  
    server.send ( 200, "text/html", "unknown action"); return;  
  }  
      server.send ( 200, "text/html", "action no found");  

  }

//3-5-5-2 网页解析IP消息 
void http_jiexi(http_request ruqest){
  
    int datStart ; int datEnd;

    // 1 解析ip
   //   http://10.255.44.33/ 
    char h[]="http://";  char l[]="/";
    datStart  =  ruqest.Referer.indexOf(h)+ strlen(h);
    datEnd= ruqest.Referer.indexOf(l,datStart);
    String host=ruqest.Referer.substring(datStart, datEnd);
    host.toCharArray( ruqest.host , 20); 
    
    Use_Serial.println ( ruqest.host); // 串口打印给电脑

    // 2 解析请求的网页文件-登录页面
   //   srun_portal_pc.php?ac_id&";
    char s[] ="?"; 
    datStart  = ruqest.Referer.indexOf(l,datStart)+strlen(l);
   // datEnd = ruqest.Referer.indexOf(s,datStart)-strlen(s)+1;
   if(ruqest.Referer.indexOf(s,datStart)==-1)
    {datEnd = ruqest.Referer.length();}
    else{    
       datEnd = ruqest.Referer.indexOf(s,datStart)-1;
      }
    ruqest.host_ur  = String(ruqest.Referer.substring(datStart, datEnd));
    Use_Serial.println ( ruqest.host_ur); // 串口打印给电脑

 
    Use_Serial.println( ruqest.postDate);
     
   if (hdulogin(ruqest) == 0) {   
      Use_Serial.println("WEB Login Success!");
    }
    else {  
      Use_Serial.println("WEB Login Fail!");
    }    
          return;  
  
  }

//3-5-5-3 网页认证发送HTTP请求
/*---------------------------------------------------------------*/
int hdulogin(struct http_request ruqest) {
  WiFiClient client;

  if (!client.connect(ruqest.host, ruqest.httpPort)) {
    Use_Serial.println("connection failed");
    return 1;
  }
  delay(10);
 
  if (ruqest.postDate.length() && ruqest.postDate != "0") {
    String data = (String)ruqest.postDate;
    int length = data.length();

/*
gzip：GNU压缩格式
compress：UNIX系统的标准压缩格式
deflate：是一种同时使用了LZ77和哈弗曼编码的无损压缩格式
identity：不进行压缩
*/
    String postRequest =
                         (String)("POST ") + "/"+ruqest.host_ur+" HTTP/1.1\r\n" +
                         "Host: " +ruqest.host + "\r\n" +
                         "Connection: Keep Alive\r\n" +
                         "Content-Length: " + length + "\r\n" +
                         "Accept:text/html,application/xhtml+xml,application/xml;*/*\r\n" +
                         "Origin: http://"+ruqest.host+"\r\n" +
                          "Upgrade-Insecure-Requests: 1"+"\r\n" +
                         "Content-Type: application/x-www-form-urlencoded;" + "\r\n" +
                         "User-Agent: zyzandESP8266\r\n" +
                        //  "Accept-Encoding: gzip, deflate"+"\r\n" +
                          "Accept-Encoding: identity"+"\r\n" +
                          "Accept-Language: zh-CN,zh;q=0.9"+"\r\n" +                                             
                         "\r\n" +
                         data + "\r\n";
//  String postDate = String("")+"action=login&ac_id=1&user_ip=&nas_ip=&user_mac=&url=&username=+"+usr_name+"&password="+usr_pwd;
    client.print(postRequest);
    delay(600);
    //处理返回信息
    String line = client.readStringUntil('\n');
    Use_Serial.println(line);
    while (client.available() > 0) {
    Use_Serial.println(client.readStringUntil('\n'));
    line += client.readStringUntil('\n');
    
   // line += client.readStringUntil('\n');
    }

  //  http_html=line;
    
    client.stop();
    
    if (line.indexOf("时间") != -1 || line.indexOf("登陆") != -1) { //认证成功
      return 0;
    
    }
    else {
      return 2;
    }

  }
  client.stop();
  return 2;
}

void handleMqttConfig(){
   
   if(server.hasArg("mqtt_config")) { // 请求中是否包含有a的参数  

        String mqtt_config = server.arg("mqtt_config"); // 获得a参数的值  
     
        
     if(mqtt_config == "on") { // a=on  
      
          if(server.hasArg("mqtt_sever")) { // 请求中是否包含有a的参数  
          //config_wifi.mqtt_sever = server.arg("mqtt_sever"); // 获得a参数的值
            server.arg("mqtt_sever").toCharArray(config_wifi.mqtt_sever, 50);    // 从网页得到的 WIFI名
          
          }
          
         if(server.hasArg("mqtt_port")) { // 请求中是否包含有a的参数  

          
             String num  = server.arg("mqtt_port"); // 获得a参数的值  
      
              int i, len;
              int result=0;
              
              len = num.length();
             
              for(i=0; i<len; i++)
              {
                result = result * 10 + ( num[i] - '0' );
              }
                  config_wifi.mqtt_port  = result;   
                       }

        if(server.hasArg("mqtt_nameid")) { // 请求中是否包含有a的参数  
          //config_wifi.mqtt_nameid = server.arg("mqtt_nameid"); // 获得a参数的值
          server.arg("mqtt_nameid").toCharArray(config_wifi.mqtt_nameid, 100);    // 从网页得到的 WIFI名
          
          }
          
         if(server.hasArg("mqtt_ssid")) { // 请求中是否包含有a的参数  
       // config_wifi.mqtt_ssid = server.arg("mqtt_ssid"); // 获得a参数的值   
           server.arg("mqtt_ssid").toCharArray(config_wifi.mqtt_ssid, 20);    // 从网页得到的 WIFI名
           }

         if(server.hasArg("mqtt_psw")) { // 请求中是否包含有a的参数  
         // config_wifi.mqtt_psw = server.arg("mqtt_psw"); // 获得a参数的值
            server.arg("mqtt_psw").toCharArray(config_wifi.mqtt_psw, 20);    // 从网页得到的 WIFI名
          }
          
         if(server.hasArg("mqtt_topic")) { // 请求中是否包含有a的参数  
        // config_wifi.mqtt_topic = server.arg("mqtt_topic"); // 获得a参数的值    
           server.arg("mqtt_topic").toCharArray(config_wifi.mqtt_topic, 100);    // 从网页得到的 WIFI名
           }
            
         saveConfig();
         mqtt_int();
         server.send ( 200, "text/html", "服务器配置成功！请重启生效！"); // 网页返回给手机提示  
         //ESP.reset();              
         return;          
    } 
    else if(mqtt_config == "off") { // a=off  
        server.send ( 200, "text/html", "config  is off!");
        return;

    }  

    server.send ( 200, "text/html", "unknown action"); return;  

  }  

  server.send ( 200, "text/html", "action no found");  
  
  
  }

//3-5-6 网页测试
void handleTest(){ 
     server.send(200, "text/html", http_html);
  }

//3-5-。。。 网页没有对应请求如何处理
void handleNotFound() {  

  String path = server.uri();  

  Serial.print("load url:");  

  Serial.println(path);  

  String contentType = getContentType(path);  

  String pathWithGz = path + ".gz";  

  if(SPIFFS.exists(pathWithGz) || SPIFFS.exists(path)){  

    if(SPIFFS.exists(pathWithGz))  

      path += ".gz";  

    File file = SPIFFS.open(path, "r");  

    size_t sent = server.streamFile(file, contentType);  

    file.close(); 

  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET)?"GET":"POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i=0; i<server.args(); i++){
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message); 

    return;  

  }  

  String message = "File Not Found\n\n";  

  message += "URI: ";  

  message += server.uri();  

  message += "\nMethod: ";  

  message += ( server.method() == HTTP_GET ) ? "GET" : "POST";  

  message += "\nArguments: ";  

  message += server.args();  

  message += "\n";  

  for ( uint8_t i = 0; i < server.args(); i++ ) {  

    message += " " + server.argName ( i ) + ": " + server.arg ( i ) + "\n";  

  }  

  server.send ( 404, "text/plain", message );  

}  
// 解析请求的文件
/** 
 * 根据文件后缀获取html协议的返回内容类型 
 */  

String getContentType(String filename){  

  if(server.hasArg("download")) return "application/octet-stream";  

  else if(filename.endsWith(".htm")) return "text/html";  

  else if(filename.endsWith(".html")) return "text/html";  

  else if(filename.endsWith(".css")) return "text/css";  

  else if(filename.endsWith(".js")) return "application/javascript";  

  else if(filename.endsWith(".png")) return "image/png";  

  else if(filename.endsWith(".gif")) return "image/gif";  

  else if(filename.endsWith(".jpg")) return "image/jpeg";  

  else if(filename.endsWith(".ico")) return "image/x-icon";  

  else if(filename.endsWith(".xml")) return "text/xml";  

  else if(filename.endsWith(".pdf")) return "application/x-pdf";  

  else if(filename.endsWith(".zip")) return "application/x-zip";  

  else if(filename.endsWith(".gz")) return "application/x-gzip";  

  return "text/plain";  

}  

/************************* 4-2 服务器重连配置 *************************************/
 
void mqtt_reconnect() {//等待，直到连接上服务器


  if (!client.connected()){

        unsigned long preTick = millis();    
       int num;
 while (!client.connected()) {//如果没有连接上

       if (millis() - preTick < 10 ) continue;//等待10ms
        preTick = millis();
         num++;      

       if(num%100==0){
        PIN_Led_State = !PIN_Led_State;
        digitalWrite(PIN_Led, PIN_Led_State);
        Use_Serial.print(".");
        num=0;
        }
        
        server.handleClient(); 

   if (client.connect(((String)config_wifi.mqtt_nameid+SN).c_str(),config_wifi.mqtt_ssid,config_wifi.mqtt_psw)) {//接入时的用户名，尽量取一个很不常用的用户名
    
             client.subscribe(MQTT_MYTOPICR);//接收外来的数据时的intopic          
             client.publish(MQTT_MYTOPICS,"hello world ");          
             Use_Serial.println("Connect succes mqtt!");//重新连接
             Use_Serial.println(client.state());//重新连接
            return;

    } else {
      server.handleClient(); 
      Use_Serial.println("connect failed!");//连接失败
      Use_Serial.println(client.state());//重新连接
      Use_Serial.println(" try connect again in 2 seconds");//延时2秒后重新连接
      delay(2000);
      }  
     }

  }
}

/* 3 接收数据处理， 服务器回掉函数*/
void callback(char* topic, byte* payload, unsigned int length) {//用于接收数据
   //-----------------1数据解析-----------------------------//
  Use_Serial.print("Message arrived [");
  Use_Serial.print(topic);
  Use_Serial.print("] ");
   char  recmsg1[length+1];
   for (int i = length-1; i >=0; i--) {   
  recmsg1[i]=(char)payload[i];
  }  
   recmsg1[length]='\0';
   Serial.println(recmsg1); 
     
 //-------------2数据转换 char[]-String --------------------//
   
  String str(recmsg1); // char 转换String
  recstr=str;
  str="";
   
 //------------- 3开启动作执行标志位    --------------------//
  
/*------ W-ALL 数据总解析 动作执行 callback  执行一次---*/
   parseData(recstr);
 
}
 
/*************************** 4 服务器函数配置*****************************/
 
void Mqtt_message(){
 
    Use_Serial.println("----------Mqtt--------");

    Use_Serial.print("Name:");
    Use_Serial.println((String)MQTT_MYNAME);
     Use_Serial.println("pub_topic:");
     Use_Serial.println(MQTT_MYTOPICS);     
     Use_Serial.println("rec_topic:");
    Use_Serial.println(MQTT_MYTOPICR);  
  
   
  }

/*-------------------------1 总解析数据控制----------------------------------*/
 /*
  *功能： 接收WIFI消息，总动作控制
  *输入： callback得到数据  String recstr; 
   输出： bool
 */
bool parseData( String  str) {
   
              
 
  
 
}

void http_wait(){
  
   unsigned long preTick = millis();
           int num = 0;
               while(WiFi.status() != WL_CONNECTED&&workmode==1){
                  server.handleClient(); 
              if (millis() - preTick < 10 ) continue;//等待10ms
              preTick = millis();
              num++;
              if (num % led_sudu == 0 )  //50*10=500ms=0.5s 反转一次
              {
                PIN_Led_State = !PIN_Led_State;
                digitalWrite(PIN_Led, PIN_Led_State);            
                Serial.print(".");
                  }
               }
  
  }
//------------------------------------------- void setup() ------------------------------------------
void mqtt_int(){
   client.setServer( config_wifi.mqtt_sever, config_wifi.mqtt_port);
   client.setCallback(callback);
  }
void setup() {
    Use_Serial.begin(115200); //串口初始化
    get_espid();              //获取自身唯一ID
    LED_Int();                //管脚初始化
    Button_Int();             //按钮初始化     
    loadConfig();             //读取保存信息 WIFI账号和密码

     mqtt_int();
  
    waitKey();                //等待按键动作 
     
}




//------------------------------------------- void void loop()  ------------------------------------------


void loop() 
{

   // 等待网页请求
   server.handleClient();  

   // 等待WIFI连接
   if(WiFi.status() == WL_CONNECTED){  

     mqtt_reconnect();//确保连上服务器，否则一直等待。
     client.loop();//MUC接收数据的主循环函数。 
     
                     }
   else {
       //WIFI断开，重新连接WIFI             
         if(workmode==0)
                  {wifi_Init();   }      
         else if(workmode==1){                    
                  http_wait();
                      }
   }         
    
}


