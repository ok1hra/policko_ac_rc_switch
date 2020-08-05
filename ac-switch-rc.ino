// AC switch RC (Arduino MEGA)
// ----------------------
// 2018-04 by OK1HRA
// rev 0.1
// set via url
// wget http://192.168.1.200/?S1
const char* REV = "20200724";
/* Changelog
------------
20190825 - change 63 timeout to 10min
20190728 - add on/off S/s in url
20190727 - add uptime and rev to html
20190726 - add button name
*/

#define EthModule        // enable Ethernet module
// #define __USE_DHCP__       // Uncoment to Enable DHCP

bool EnableEthernet = 1;
bool EnableDHCP     = 1;

// byte OutputPin[] = { 54, 2, 3, 5, 6, 7, 8, 9, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 53, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 65, 66, 67, 68, 69, 1};
byte OutputPin[] = { 5,3,2,54,9,8,7,6, 14,13,12,11, 18,17,16,15, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 53, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 65, 66, 67, 68, 69, 1};
bool OutputPinStatus[63];
const int SERBAUD = 115200;    // [baud] Serial port baudrate

//  #include <util.h>
  #include <SPI.h>
  #include <Ethernet.h>
  // #include <Dhcp.h>
  #include <EthernetServer.h>
  // #include <SPI.h>
  // #include <EthernetUdp.h>
  // #include <PubSubClient.h>
  byte mac[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xD1};
  // IPAddress ip(192, 168, 11, 200);         // IP
  // IPAddress gateway(192, 168, 11, 1);    // GATE
  IPAddress ip(192, 168, 11, 200);         // IP
  IPAddress gateway(192, 168, 1, 0);    // GATE
  IPAddress subnet(255, 255, 255, 0);     // MASK
  IPAddress myDns(8, 8, 8, 8);            // DNS (google pub)
  EthernetServer server(80);              // server PORT
  String HTTP_req;
  long WatchDog63Timeout[2] = {0, 600000};

//------------------------------------------------------------------------
void setup(){

  for (int i = 0; i < 63; i++){
  	  pinMode(OutputPin[i], OUTPUT);
  	  digitalWrite(OutputPin[i], LOW);
  }
  Serial.begin(115200);
  Serial.println("Remote AC switch");
  Serial.print("Version: ");
  Serial.println(REV);

  #if defined(EthModule)
    #if defined __USE_DHCP__
      Ethernet.begin(mac);
    #else
      Ethernet.begin(mac, ip, myDns, gateway, subnet);
    #endif
    server.begin();
    Serial.print("server is at http://");
    Serial.println(Ethernet.localIP());
  #endif

  // initial ALL off
  for (int i = 0; i < 63; i++){
    SetPin(i, false);
  }
}
//------------------------------------------------------------------------

void loop() {
  WebInterface();
  WatchDog63();
  // Snake();
}
//------------------------------------------------------------------------
void Snake(){
  for (int i=0; i<63; i++) {
    digitalWrite(OutputPin[i], HIGH);
    delay(50);
  }
  for (int i=0; i<63; i++) {
    digitalWrite(OutputPin[i], LOW);
    delay(50);
  }
}

//------------------------------------------------------------------------
void WatchDog63(){
  if(millis()-WatchDog63Timeout[0] > WatchDog63Timeout[1] && OutputPinStatus[63-1]==true){
    digitalWrite(OutputPin[63-1], LOW);
    OutputPinStatus[63-1]=false;
  }
}

//------------------------------------------------------------------------
void SwitchPin(int PIN, String STATUS){
  if(STATUS=="s"){
    // Serial.println("L");
    digitalWrite(OutputPin[PIN-1], LOW);
    OutputPinStatus[PIN-1]=false;
  }
  if(STATUS=="S"){
    // Serial.println("H");
    digitalWrite(OutputPin[PIN-1], HIGH);
    OutputPinStatus[PIN-1]=true;
    if(PIN==63){
      WatchDog63Timeout[0]=millis();    //  <--------------------------WATCHDOG
    }
  }

  CheckAlwaysOn(16, 0, 62); // check 1-62, from 0-63, to 0-63

  Serial.print(millis()/1000);
  Serial.print("s status ");
  for (int i = 0; i < 63; i++){
    Serial.print(OutputPinStatus[i]);
    Serial.print(" ");
  }
  Serial.println();
}
//------------------------------------------------------------------------
void ReversePin(int PIN){
  if(OutputPinStatus[PIN-1]==0){
    digitalWrite(OutputPin[PIN-1], HIGH);
    OutputPinStatus[PIN-1]=true;
    if(PIN==63){
      WatchDog63Timeout[0]=millis();    //  <--------------------------WATCHDOG
    }
  }else{
    digitalWrite(OutputPin[PIN-1], LOW);
    OutputPinStatus[PIN-1]=false;
  }
  CheckAlwaysOn(16, 0, 62); // check 1-62, from 0-63, to 0-63

  Serial.print(millis()/1000);
  Serial.print("s status ");
  for (int i = 0; i < 63; i++){
    Serial.print(OutputPinStatus[i]);
    Serial.print(" ");
  }
  Serial.println();
}
//------------------------------------------------------------------------

void CheckAlwaysOn(int ON, int FROM, int TO){ // 16
  int sum = 0;
  for (int i = FROM; i < TO; i++){
    if(OutputPinStatus[i]!=0 && i != ON-1){
      sum++;
    }
  }
  if(sum>0 && OutputPinStatus[ON-1]==false){
    digitalWrite(OutputPin[ON-1], HIGH);
    OutputPinStatus[ON-1]=true;
  }
  if(sum==0 && OutputPinStatus[ON-1]==true){
    digitalWrite(OutputPin[ON-1], LOW);
    OutputPinStatus[ON-1]=false;
  }
}
//------------------------------------------------------------------------

void SetPin(int PIN, int VALUE){
  OutputPinStatus[PIN-1]=VALUE;
  digitalWrite(OutputPin[PIN-1], VALUE);
  CheckAlwaysOn(16, 0, 62); // check 1-62, from 0-63, to 0-63

  Serial.print(millis()/1000);
  Serial.print("s status ");
  for (int i = 0; i < 63; i++){
    Serial.print(OutputPinStatus[i]);
    Serial.print(" ");
  }
  Serial.println();

}
//------------------------------------------------------------------------

void WebInterface(){
  EthernetClient client = server.available();
  if (client) {
    boolean currentLineIsBlank = true;
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        HTTP_req += c;
        if (c == '\n' && currentLineIsBlank) {
          client.println("HTTP/1.1 200 OK");
          client.println("Content-Type: text/html");
          client.println("Connection: close");
          client.println();
          client.println("<!DOCTYPE html>");
          client.println("<html>");
          client.println("<head>");
          client.print("<title>RC AC switch OL7M</title>");
          client.print("<meta http-equiv=\"refresh\" content=\"10;url=http://");
          client.print(Ethernet.localIP());
          client.println("\">");
          client.println("<link href='http://fonts.googleapis.com/css?family=Roboto+Condensed:300italic,400italic,700italic,400,700,300&subset=latin-ext' rel='stylesheet' type='text/css'>");
          client.println("<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0, user-scalable=no\">");
          client.println("<meta name=\"mobile-web-app-capable\" content=\"yes\">");
          client.println("<style type=\"text/css\">");
          client.println("body {font-family: 'Roboto Condensed',sans-serif,Arial,Tahoma,Verdana;background: #ccc;}");
          client.println("a:link  {color: #888;font-weight: bold;text-decoration: none;}");
          client.println("a:visited  {color: #888;font-weight: bold;text-decoration: none;}");
          client.println("a:hover  {color: #888;font-weight: bold;text-decoration: none;}");
          client.println("input {border: 2px solid #bbb; background: #ccc;margin: 10px 5px 0 0;-webkit-border-radius: 5px;-moz-border-radius: 5px;border-radius: 5px;color : #333;}");
          client.println("input:hover {border: 2px solid #080;}");
          client.println("input.g  {background: #080; color: #fff;}");
          client.println("input.gr {background: #800; color: #fff;}");
          client.println("input.u  {background: #fff;}");
          client.println(".bcd {color: #888; border: 0px solid #fff;background: #ccc;margin: 10px 5px 0 0px;-webkit-border-radius: 5px;-moz-border-radius: 5px;border-radius: 5px}");
          client.println(".bcdr {color: #fff; border: 0px solid #080;background: #800;padding: 0px 7px 0px 7px;margin: 10px 2px 0 0px;-webkit-border-radius: 5px;-moz-border-radius: 5px;border-radius: 5px}");
          client.println(".ptt {border: 2px solid #800;background: #ccc;margin: 10px 2px 0 10px;padding: 1px 7px 1px 7px;-webkit-border-radius: 5px;-moz-border-radius: 5px;border-radius: 5px;color : #800;}");
          client.println("</style>");
          client.println("</head>");
          client.println("<body>");
          // client.print("<p><b>AC switch</b></p>");
          client.println(F("<form method=\"get\">"));
          String GETOUTPUT = HTTP_req.substring(7, 9);
          Serial.println(GETOUTPUT.toInt());
          Serial.println(HTTP_req.substring(6, 7));
          if(GETOUTPUT.toInt()>0){
            // SetPin(GETOUTPUT.toInt(), !OutputPinStatus[GETOUTPUT.toInt()]);
            // ReversePin(GETOUTPUT.toInt());
            SwitchPin(GETOUTPUT.toInt(), HTTP_req.substring(6, 7));
          }

          for (int i = 1; i < 64; i++) {
            // always ON
            if(i==16){
              client.print("<span class=\"");
              if (OutputPinStatus[i-1] == true) {
                client.print("bcdr");
              }else{
                client.print("bcd");
              }
              client.print("\">");
              client.print("ON");
              client.print("</span><hr>");

            // Other outputs
            }else{
              client.print("<input type=\"submit\" name=\"");
              if(OutputPinStatus[i-1]==true){
                client.print("s");
              }else{
                client.print("S");
              }
              client.print(i);
              client.print("\" value=\"");
              if(i==1 || i==5 || i==9 || i==13){client.print("[ ");}
              if(i==16){
                client.print("ON");
              }else{
                client.print(i);
              }
              if(i==1 || i==5 || i==9 || i==13){client.print(" ]");}
              client.print("\" class=\"");
              if (OutputPinStatus[i-1] == true) {
                client.print("g");
              }else{
                if(i==63 || i==5 || i==1){      // <------------------------------------
                  client.print("u");    // used outputs sign in html
                }else{
                  client.print("r");
                }
              }
              // 1
              if(i==1){client.print("\" title=\"RC1 TS480");}
              // 5
              if(i==5){client.print("\" title=\"RC2 7800");}
              // 61
              if(i==61){client.print("\" title=\"30m HB9CW");}
              // 62
              if(i==62){client.print("\" title=\"30m dipole");}
              // 63
              if(i==63){client.print("\" title=\"Halogeny timeout+10min");}

              client.print("\">");
              if(i==4 || i==8 || i==12 || i==16){client.print("<br>");}
            }
          }

          client.println("</form>");
          client.println("<br><a href=\".\" onclick=\"window.open( this.href, this.href, 'width=200,height=600,left=0,top=0,menubar=no,location=no,status=no' ); return false;\" > split&#8599;</a>");
          client.print("<br>Uptime: ");
          client.print(millis()/1000);
          client.print("s | rev: ");
          client.print(REV);
          client.println("<br>URL control");
          client.println("<br>wget http://192.168.1.200/?S60 [ON]");
          client.println("<br>wget http://192.168.1.200/?s60 [OFF]");
          client.println("</body>");
          client.println("</html>");

          Serial.print(HTTP_req);
          HTTP_req = "";
          break;
        }
        if (c == '\n') {
          currentLineIsBlank = true;
        }
        else if (c != '\r') {
          currentLineIsBlank = false;
        }
      }
    }
    delay(1);
    client.stop();
  }
}
