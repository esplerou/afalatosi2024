#include <DFRobot_HX711.h>
#include <WiFiClientSecure.h>
#include <Wire.h>
#include <DHT22.h>
#include <HTTPClient.h>
#include "DFRobot_LCD.h"

#define TIME_WIFI 5*60*1000
#define TIME_READ 5*1000

DFRobot_LCD lcd(16,2);  //16 characters and 2 lines of show

//Το ασύρματο δίκτυο
#define NETWORK_SIZE 2
String ssid[ NETWORK_SIZE] = {"COSMOTE-763943", "dal2"}; //--> Your wifi name or SSID.
String password[ NETWORK_SIZE] = {"esperinolerou", "13571357"}; //--> Your wifi password.
int network = 0;
WiFiClientSecure client; //--> Create a WiFiClientSecure object.

//----------------------------------------Host & httpsPort
const char* host = "script.google.com";
//const char* host = "koinotopia.gr";
const int httpsPort = 443;
String GOOGLE_ID = "AKfycby6-YHCxQes1GdezQUhMMWGND1XgOWmCrUTMKUpZ3ruomfF91vBgOQlTCto4cTemXaY"; //--> spreadsheet script ID
//----------------------------------------

//Η συνδεσμολογία
#define HT1_PIN   14
#define HT2_PIN   13
#define HT3_PIN   12

#define BAROS1_PIN   19
#define BAROS2_PIN   20

#define NERO_PIN 4

#define FOS1_PIN  5
#define FOS2_PIN  6
#define FOS3_PIN  7

#define KOYMPI_ANTLIA_PIN 47
#define KOYMPI_WIFI_PIN 48

#define MOTER_PIN 16
#define WIFI_PIN 17

//Οι αισθητήρες
DHT22 ht1( HT1_PIN);
DHT22 ht2( HT2_PIN);
DHT22 ht3( HT3_PIN);

DFRobot_HX711 MyScale(BAROS1_PIN, BAROS2_PIN);

int orio_nero = 0;
int diarkeia_moter = 1000;
int apostasi_moter = 5 * 60;
int start_moter =  0;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);

  lcd.init();
  lcd.clear();              // clear display
  lcd.setCursor(0, 0);      // move cursor to   (0, 0)
  lcd.print("ESPERINO"); 
  lcd.setCursor(0, 1);      // move cursor to   (0, 0)
  lcd.print("GYMNASIO LEROU"); 
    
  pinMode(MOTER_PIN, OUTPUT); //--> On Board LED port Direction output
  pinMode(WIFI_PIN, OUTPUT); //--> On Board LED port Direction output
  pinMode(KOYMPI_ANTLIA_PIN, INPUT); //--> On Board LED port Direction output
  pinMode(KOYMPI_WIFI_PIN, INPUT); //--> On Board LED port Direction output
  
  analogReadResolution(12);

  MyScale.setCalibration(1992);

  delay(500);
}

int time_wifi =  0;
int time_diabasma = 0;

void loop() {
  int t = millis();

  if( t < time_diabasma) {
    elegxos_koumpi();
    delay( 100);
    return;
  }

  int apostoli;
  if( t > time_wifi) {
    apostoli = 1;
  } else {
    apostoli = 0;
  }
  Serial.println( "Apostoli WIFI=" + String( apostoli) + " t=" + String( t) + " time_wifi=" + String( time_wifi));
  
  diabasma( apostoli); 
}

int connectWifi() {
  lcd.setCursor( 15, 0);
  lcd.print( "c");

  for ( int i = 0; i < NETWORK_SIZE; i++) {
    WiFi.begin(ssid[ network], password[ network]); //--> Connect to your WiFi router
    Serial.println("");
    //----------------------------------------Wait for connection
    Serial.print("Connecting " + ssid[ network]);
    int start = millis();
    while ( millis() < start + 20 * 1000) {
      if ( WiFi.status() == WL_CONNECTED) {
        Serial.print("Successfully connected to : ");
        Serial.println( ssid[ network]);
        Serial.print("IP address: ");
        Serial.println( WiFi.localIP());
        client.setInsecure();
        return 1;
      }
      Serial.print(".");
      delay( 500);
    }
    network = (network + 1) % NETWORK_SIZE;
  }

  lcd.setCursor( 15, 0);
  lcd.print( "C");

  return 0;
}

int diabasma_nero() {
  int nero = analogRead( NERO_PIN);
  Serial.println( "diabasma_nero=" + nero);
  delay(100);
  return map(nero, 0, 1024*4, 1024*4 - 1, 0);
}

int elegxos_moter() {
  int nero = diabasma_nero();
  if( nero > orio_nero) {
    return 0;
  }
  int start = millis();

  //Δεν θέλουμε να ανοίγει συχνά η αντλία για λόγους ασφαλείας.
  //Θα ανοίγει το πολύ μία φορά κάθε 5 λεπτά.
  if( start < start_moter + apostasi_moter * 1000) {
    return 0;
  }
  lcd.clear();
  lcd.setCursor( 0, 0);
  lcd.print( "MOTER ANOIGMA");
    
  //Άνοιγμα μοτέρ.
  start_moter = start;
  int diarkeia_moter;
  // Θέλουμε το μοτέρ για λόγους ασφαλείας να ανοίγει για μικρό χρονικό διάστημα π.χ. 2 δευτερόλεπτα.
  while( millis() < start + diarkeia_moter) {
    digitalWrite(MOTER_PIN, HIGH);
    delay( 10);
    nero = diabasma_nero();
    diarkeia_moter = millis() - start;
    if( nero > orio_nero) {
      break;
    }
    elegxos_koumpi();
  }
  
  digitalWrite(MOTER_PIN, LOW); //--> Turn off Led On Board

  lcd.clear();
  lcd.setCursor( 0, 0);
  lcd.print( "MOTER KLEISIMO");
  lcd.setCursor( 0, 1);
  lcd.print( String( diarkeia_moter));

  delay( 5000);

  return diarkeia_moter;
}

void diabasma(int apostoli) {

  //1η θερμοκρασία / υγρασία.
  float t1 = ht1.getTemperature();
  float h1 = ht1.getHumidity();
  if (ht1.getLastError() != ht1.OK) {
    t1 = 0;
    h1 = 0;
    Serial.print("last error 1:");
    Serial.println(ht1.getLastError());
  }
  delay( 100);

  //2η θερμοκρασία / υγρασία.
  float t2 = ht2.getTemperature();
  float h2 = ht2.getHumidity();
  if (ht2.getLastError() != ht1.OK) {
    t2 = 0;
    h2 = 0;
    Serial.print("last error 2:");
    Serial.println(ht2.getLastError());
  }
  delay( 100);

  //3η θερμοκρασία / υγρασία.
  float t3 = ht3.getTemperature();
  float h3 = ht3.getHumidity();
  if (ht3.getLastError() != ht1.OK) {
    t3 = 0;
    h3 = 0;
    Serial.print("last error 3:");
    Serial.println(ht3.getLastError());
  }
  delay( 100);

  //Νερό
  int nero = diabasma_nero();
  float baros = MyScale.readWeight();

  //Φως
  float fos1 = map(analogRead( FOS1_PIN), 0, 1024*4, 99000, 0) / 1000;
  float fos2 = map(analogRead( FOS2_PIN), 0, 1024*4, 99000, 0) / 1000;
  float fos3 = map(analogRead( FOS3_PIN), 0, 1024*4, 99000, 0) / 1000;

  int diarkeiaMoter = 0;
  if( apostoli) {    
    diarkeiaMoter = elegxos_moter();
  }
  othoni( t1, h1, t2, h2, t3, h3, nero, baros, fos1, fos2, fos3, diarkeiaMoter);

  if( apostoli == 0) {
    return;
  }
  
  // Πότε να ξαναστείλει τα δεδομένα μέσω wifi.
  time_wifi = millis() + TIME_WIFI;
  
  digitalWrite(WIFI_PIN, HIGH);
  if( !connectWifi()) {
    return;
  }
  if (!client.connect(host, httpsPort)) {
    Serial.println("connection failed");
    lcd.setCursor( 15, 0);
    lcd.print( "F");
    return;
  }

  lcd.setCursor( 15, 0);
  lcd.print( "W");

  //----------------------------------------Processing data and sending data
  String url = "https://script.google.com/macros/s/" + GOOGLE_ID;
  url += "/exec?wat=" + String(nero);
  url += "&wei=" + String(baros);
  if( isvalid_dht( t1, h1)) {
    url += "&tem1=" + String(t1, 1) + "&hum1=" + String(h1, 1);
  }
  if( isvalid_dht( t2, h2)) {
    url += "&tem2=" + String(t2, 1) + "&hum2=" + String(h2, 1);
  }
  if( isvalid_dht( t3, h3)) {
    url += "&tem3=" + String(t3, 1) + "&hum3=" + String(h3, 1);
  }
  url += "&wat=" + String(nero) + "&wei=" + String(baros);
  url += "&lig1=" + String(fos1, 3) + "&lig2=" + String(fos2, 3)+ "&lig3=" + String(fos3, 3);
  url += "&mot=" + String(diarkeiaMoter);
  
  Serial.println("requesting URL: " + url);

  HTTPClient http;
  
  http.begin(url.c_str()); //Specify the URL and certificate
  http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);
  int httpCode = http.GET();
  String ret;
  if (httpCode > 0) { //Check for the returning code
    ret = http.getString();
    Serial.println(httpCode);
    Serial.println( String( "ret=") + ret);
  } else {
    Serial.println("Error on HTTP request");
  }
  http.end();

  client.stop();
  digitalWrite(WIFI_PIN, LOW);

  diabasma_google( ret);

  Serial.println("closing connection");
  Serial.println("==========");
  Serial.println();
}

void diabasma_google( String result) {
  String error;
  int ind1 = result.indexOf('|');
  if( ind1 >= 0) {
    error = result.substring( 0, ind1);
    int num = result.substring( ind1 + 1).toInt();
    if( num > 0) {
      orio_nero = num;
    }

    int ind2 = result.indexOf( '|', ind1 + 1);
    int diarkeia = result.substring( ind2 + 1).toInt();
    if( diarkeia > 0) {
      diarkeia_moter = diarkeia;
    }

    int ind3 = result.indexOf( '|', ind2 + 1);
    int apostasi = result.substring( ind3 + 1).toInt();
    if( apostasi > 0) {
      apostasi_moter = apostasi;
    }
    
    Serial.println( String( "error=") + error + " num=" + num + " orio=" + orio_nero + " diarkeia_moter=" + diarkeia_moter + " apostasi_moter=" + apostasi_moter);
   }
}

void othoni( float t1, float h1, float t2, float h2, float t3, float h3, int nero, float baros, float fos1, float fos2, float fos3, int diarkeia_moter) {
  lcd.clear();

  Serial.println( "t1=" + String( t1) + " t2=" + String( t2) + "t3=" + String( t3));
  Serial.println( "h1=" + String( h1) + " h2=" + String( h2) + "h3=" + String( h3));
  Serial.println( "nero1=" + String( nero) + " baros=" + String( baros));
  Serial.println( "fos1=" + String( fos1) + " fos2=" + String( fos2) + " fos3=" + String( fos3));
  
  //Θερμοκρασία1/Θερμοκρασία2 (2 ψηφία)
  if( isvalid_dht( t1, h1)) {
    lcd.setCursor( 0, 0);
    lcd.print( String( t1, 0));
    lcd.setCursor( 3, 0);
    lcd.print( String( h1, 0));
  }

  //Υγρασία1/Υγρασία2 (2 ψηφία)
  if( isvalid_dht( t2, h2)) {
    lcd.setCursor( 0, 1);
    lcd.print( String( t2, 0));
    lcd.setCursor( 3, 1);
    lcd.print( String( h2, 0));
  }

  //Φως1/Φως2 (2 ψηφία)
  lcd.setCursor( 6, 0);
  lcd.print( String( fos1, 0));
  lcd.setCursor( 6, 1);
  lcd.print( String( fos2, 0));

  //Νερό, Βάρος (4 ψηφία)
  lcd.setCursor( 9, 0);
  lcd.print( String( nero));
  lcd.setCursor( 9, 1);
  lcd.print( String( baros, 1));

  //Όριο νερό, Διάρκεια μοτέρ (2 ψηφία)
  lcd.setCursor( 14, 0);
  lcd.print( String( orio_nero));
  lcd.setCursor( 14, 1);
  lcd.print( String( diarkeia_moter));

  //Πότε να ξαναδιαβάσει τους αισθητήρες.
  time_diabasma = millis() + TIME_READ; 
  
  Serial.println( "Send LCD");
}

void elegxos_koumpi() {
  int d=digitalRead( KOYMPI_WIFI_PIN);

  if( d == 1) {
    lcd.setCursor( 15, 0);
    lcd.print( "B");
    diabasma( 1);
  }

  //Ελέγχω το κουμπί για το μορέρ
  d=digitalRead( KOYMPI_ANTLIA_PIN);
  while( d == 1) {
    lcd.setCursor( 15, 0);
    lcd.print( "m");
    digitalWrite(MOTER_PIN, HIGH);
    delay( 50);
    d=digitalRead( KOYMPI_ANTLIA_PIN);
  }
  digitalWrite(MOTER_PIN, LOW); //--> Turn off Led On Board
  lcd.setCursor( 15, 0);
  lcd.print( " ");
}

int isvalid_dht( float temp, float hum) {
  if( temp == 25.5 && hum == 25.5) {
    return 0;
  }
  if( temp == 0 && hum == 0) {
    return 0;
  }

  return 1;
}
