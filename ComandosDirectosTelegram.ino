//LIBRERIAS
#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <OneWire.h>
#include <DallasTemperature.h>

#define _DEBUG_
//VARIABLES
const unsigned long BOT_MTBS = 100; // mean time between scan messages
const int ReleSolar = D4;         //Salida del Relé Solar
const int ReleElect = D6;         //Salida del Relé Electrico

unsigned long bot_lasttime;          // last time messages' scan has been done
int EstadoReleSolar = 0;          //Guarda el estado de los reles
int EstadoReleElect = 0;
bool peticion = false;            //false para leer la temperatura del electrico
                                  //true para leer la tempertaura del solar
// Parámetros sensor temperatura (DS18B20)
const int TempSolar = D5;           //Pin del sensor DS18B20 en el termo Solar
const int TempElect = D7;           //Pin del sensor DS18B20 en el termo Electrico
OneWire SensorSolar(TempSolar);
OneWire SensorElect(TempElect);
DallasTemperature SensorSolarDS18B20(&SensorSolar);
DallasTemperature SensorElectDS18B20(&SensorElect);
/**********************************************************************************
                 FUNCIÓN PARA LEER TEMPERATURA DS18B20
**********************************************************************************/
float ObtenerTemperatura(bool i) {
  // Mensaje lectura temperatura
  SensorSolarDS18B20.requestTemperatures();
  SensorElectDS18B20.requestTemperatures();
  // Leer temperatura
  float TemperaturaSolar = SensorSolarDS18B20.getTempCByIndex(0);
  float TemperaturaElect = SensorElectDS18B20.getTempCByIndex(0);

#ifdef _DEBUG_
  Serial.print("La temperatura es de ");
  Serial.println(TemperaturaSolar, 3);
#endif
    if (i==true)
      return TemperaturaSolar;
    else
      return TemperaturaElect;
}//float ObtenerTemperatura
/********************************************************************************/
// Credenciales de la conexión WiFi
#define WIFI_SSID "DIAZARRANTE"
#define WIFI_PASSWORD "TLWw5ExPxZt9vkV5fMn2"
// Telegram BOT Token (Get from Botfather)
#define BOT_TOKEN "1579159553:AAFoF2QTnTZb4I8SBgvlNalGuJabaGkaNPM"

//Seguridad de telegram
X509List cert(TELEGRAM_CERTIFICATE_ROOT);
WiFiClientSecure secured_client;
UniversalTelegramBot bot(BOT_TOKEN, secured_client);
/********************************************************************************
                FUNCIÓN QUE VE SI HAY NUEVOS MENSAJES
*********************************************************************************/
void handleNewMessages(int numNewMessages)
{
  Serial.print("handleNewMessages ");
  Serial.println(numNewMessages);
  
  String answer;

  for (int i = 0; i < numNewMessages; i++)
  {
    telegramMessage &msg = bot.messages[i];
    String from_name = bot.messages[i].from_name;
    String chat_id = bot.messages[i].chat_id;
    //String text = bot.messages[i].text;
    Serial.println("Recibido " + msg.text);
    bot.sendMessage(chat_id, "Comando recibido, procesando....", "");
    /*if (msg.text == "/ayuda")
      answer = "¿Necesitas ayuda, uh? yo tambien! usa /start /temp /options o /status para empezar";
    else if (msg.text == "/quiensoy")
      answer = "Bienvenido amigo tu nombre en telegram es: " + msg.from_name + ".... muy guapo por cierto¡¡";
    else if (msg.text == "/estado")
      answer = "Todo está bien por aquí, gracias por preguntar!";*/
    if (msg.text == "/temp"){
      float temp = ObtenerTemperatura(peticion = true);
      float temp2 = ObtenerTemperatura(peticion = false);
      answer ="La temperatura del TermoSolar es de  ";
      answer += temp;
      answer +="\n La temperatura del TermoElectrico es de  ";
      answer += temp2;
      }
        bot.sendMessage(msg.chat_id, answer);
    if (msg.text == "/solon"){
      digitalWrite(ReleSolar, LOW); // turn the LED on (HIGH is the voltage level)
      digitalWrite(ReleElect, HIGH); // turn the LED on (HIGH is the voltage level)
      EstadoReleSolar = 1;
      EstadoReleElect = 0;
      bot.sendMessage(chat_id, "Termo Solar Activado", "");
    }
    else if (msg.text == "/soloff"){
      EstadoReleSolar = 0;
      digitalWrite(ReleSolar, HIGH); // turn the LED off (LOW is the voltage level)
      bot.sendMessage(chat_id, "Termo Solar Desactivado", "");
    }
    if (msg.text == "/eleon"){
      digitalWrite(ReleElect, LOW); // turn the LED on (HIGH is the voltage level)
      digitalWrite(ReleSolar, HIGH); // turn the LED on (HIGH is the voltage level)
      EstadoReleElect = 1;
      EstadoReleSolar = 0;
      bot.sendMessage(chat_id, "Termo Electrico Activado", "");
    }
    else if (msg.text == "/eleoff"){
      EstadoReleElect = 0;
      digitalWrite(ReleElect, HIGH); // turn the LED off (LOW is the voltage level)
      bot.sendMessage(chat_id, "Termo Electrico Desactivado", "");
    }
    else if (msg.text == "/status"){
      if (EstadoReleSolar){
        bot.sendMessage(chat_id, "SOLAR ON", "");
      }
      else{
        bot.sendMessage(chat_id, "SOLAR OFF", "");
      }
    }
    else if (msg.text == "/options"){
      String keyboardJson = "[[\"/solon\", \"/soloff\"],[\"/eleon\", \"/eleoff\"],[\"/status\",\"/temp\"]]";
      bot.sendMessageWithReplyKeyboard(chat_id, "Elige una de las siguientes opciones", "", keyboardJson, true);
    }
    else if (msg.text == "/start"){
      String welcome = "Bienvenid@ a la librería de Universal Arduino Telegram Bot, " + from_name + ".\n";
      welcome += "/temp : Muestra la temperatura de los termos\n";
      welcome += "/solon : Encender el termo SOLAR\n";
      welcome += "/solooff : Apagar el termo SOLAR\n";
      welcome += "/eleon : Encender el termo ELECTRICO\n";
      welcome += "/eleoff : Apagar el termo ELECTRICO\n";
      welcome += "/status : Muestra el estado de las valvulas\n";
      welcome += "/options : Menú rápido para comandos\n";
      bot.sendMessage(chat_id, welcome, "Markdown");
    }
  }
}

void setup()
{
  Serial.begin(115200);
  Serial.println();
  SensorSolarDS18B20.begin();

  // attempt to connect to Wifi network:
  configTime(0, 0, "pool.ntp.org");      // get UTC time via NTP
  secured_client.setTrustAnchors(&cert); // Add root certificate for api.telegram.org
  Serial.print("Connecting to Wifi SSID ");
  Serial.print(WIFI_SSID);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(500);
  }
  Serial.print("\nWiFi connected. IP address: ");
  Serial.println(WiFi.localIP());
  pinMode(ReleSolar, OUTPUT); // initialize digital ledPin as an output.
  pinMode(ReleElect, OUTPUT); // initialize digital ledPin as an output.
  delay(10);
  digitalWrite(ReleSolar, HIGH); // initialize pin as off
  digitalWrite(ReleElect,HIGH);  // initialize pin as off
  // Check NTP/Time, usually it is instantaneous and you can delete the code below.
  Serial.print("Retrieving time: ");
  time_t now = time(nullptr);
  while (now < 24 * 3600)
  {
    Serial.print(".");
    delay(100);
    now = time(nullptr);
  }
  Serial.println(now);
}//void setup()

void loop(){
  if (millis() - bot_lasttime > BOT_MTBS){
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);

    while (numNewMessages){
      Serial.println("got response");
      handleNewMessages(numNewMessages);
      numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    }
    bot_lasttime = millis();
  }
}//void loop()