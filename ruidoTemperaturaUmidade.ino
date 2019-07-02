#include "DHT.h"

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <PubSubClient.h>

#include <math.h>

#define SSID "ssid da rede"
#define PASSWORD "senha da rede"

#define TOKEN "token do device no ThingsBoard"

//defines - mapeamento de pinos do NodeMCU
#define D0    16
#define D1    5
#define D2    4
#define D3    0
#define D4    2
#define D5    14
#define D6    12
#define D7    13
#define D8    15
#define D9    3
#define D10   1
#define DHTTYPE DHT11


// porta 80 para HTTP
ESP8266WebServer server(80);

// conteudo da pagina web
String page = "";

int sensorPin = A0;
float sensorValue = 0;
//float sensorValueAux = 0;

float confortoDb = 40.0;
float limiteDb = 50.0;
float limiteTempSup = 23.0;
float limiteTempInf = 20.0;
float limiteHumSup = 80.0;
float limiteHumInf = 40.0;

//char thingsboardServer[] = "demo.thingsboard.io";

const char* BROKER_MQTT = "demo.thingsboard.io"; //URL do broker MQTT que se deseja utilizar
int BROKER_PORT = 1883; // Porta do Broker MQTT

// Initialize the Ethernet client object
WiFiClient espClient;

PubSubClient MQTT(espClient);

int status = WL_IDLE_STATUS;
unsigned long lastSend;


// Initialize DHT sensor.
DHT dht(D1, DHTTYPE);

void setup() {
  // initialize serial for debugging
  Serial.begin(9600);
  dht.begin();

  page = "<h1>Ruido, Temperatura e Umidade</h1> <h3>DCC091 - IoT - 2019.1</h3> <h4>Alunos: <br> Braulio Silva Mendes Lucas <br> Joao Victor Dutra Balboa</h4> <h5> <br><br> Escolha o ambiente para leitura dos valores:</h5>  <p><a href=\"Biblioteca\"><button>Biblioteca</button></a>&nbsp; <a href=\"SalaAula\"><button>Sala de Aula</button></a>&nbsp; <a href=\"Apartamento\"><button>Apartamento</button></a>&nbsp; <a href=\"Escritorio\"><button>Escritorio</button></a>&nbsp; <a href=\"Restaurante\"><button>Restaurante</button></a></p>";
  pinMode(D7, OUTPUT);
  digitalWrite(D7, LOW);

  pinMode(D3, OUTPUT);
  digitalWrite(D3, LOW);

  pinMode(D5, OUTPUT);
  digitalWrite(D5, LOW);

  InitWiFi();
  MQTT.setServer(BROKER_MQTT, BROKER_PORT);
  //MQTT.setCallback(mqtt_callback);
  
  lastSend = 0;

  server.on("/", [](){
    server.send(200, "text/html", page);
  });
  server.on("/Apartamento", [](){
    server.send(200, "text/html", page);
    limiteDb = 45.0;
    limiteTempSup = 23.0;
    limiteTempInf = 20.0;
  });
  server.on("/Biblioteca", [](){
    server.send(200, "text/html", page);
    confortoDb = 35.0;
    limiteDb = 45.0;
    limiteTempSup = 23.0;
    limiteTempInf = 20.0;
    limiteHumSup = 80.0;
    limiteHumInf = 40.0;
  });
  server.on("/Escritorio", [](){
    server.send(200, "text/html", page);
    confortoDb = 30.0;
    limiteDb = 40.0;
    limiteTempSup = 23.0;
    limiteTempInf = 20.0;
    limiteHumSup = 80.0;
    limiteHumInf = 40.0;
  });
  server.on("/Restaurante", [](){
    server.send(200, "text/html", page);
    confortoDb = 40.0;
    limiteDb = 50.0;
    limiteTempSup = 23.0;
    limiteTempInf = 20.0;
    limiteHumSup = 80.0;
    limiteHumInf = 40.0;
  });
  server.on("/SalaAula", [](){
    server.send(200, "text/html", page);
    confortoDb = 40.0;
    limiteDb = 50.0;
    limiteTempSup = 23.0;
    limiteTempInf = 20.0;
    limiteHumSup = 80.0;
    limiteHumInf = 40.0;
  });

  server.begin();
}

void loop() {
  status = WiFi.status();
  if ( status != WL_CONNECTED) {
    while ( status != WL_CONNECTED) {
      Serial.print("Attempting to connect to WPA SSID: ");
      Serial.println(WIFI_AP);
      // Connect to WPA/WPA2 network
      status = WiFi.begin(SSID, PASSWORD);
      delay(500);
    }
    Serial.println("Connected to AP");
  }

  // atende ao cliente http
  server.handleClient();
  
  if ( !MQTT.connected() ) {
    reconnectMQTT();
  }

  if ( millis() - lastSend > 1000 ) { // Update and send only after 1 seconds
    getAndSendSoundData(confortoDb, limiteDb);
    getAndSendTemperatureAndHumidityData(limiteTempSup, limiteTempInf, limiteHumSup, limiteHumInf);
    lastSend = millis();
  }

  MQTT.loop();
}

void getAndSendSoundData(float conforto, float limite){
  
  sensorValue = analogRead(sensorPin);
  
  if(isnan(sensorValue)){
    Serial.println("Falha leitura sensor!");
  }

  Serial.print("ACD: ");
  Serial.print(sensorValue);

  //Formas de conversão para dB, usou-se apenas regras de 3 e subtração devido à baixa sensibilidade do sensor
  sensorValue = sensorValue - 348; //335;
  //sensorValue = (sensorValueAux+83.2073)/11.003;//https://circuitdigest.com/microcontroller-projects/arduino-sound-level-measurement
  //sensorValue = (sensorValueAux*35)/354;
  
  /* Fórmulas mais elaboradas de conversão
  sensorValue = 20*log10(sensorValueAux); 
  sensorValue = 16.801 * log(sensorValueAux/1023) + 9.872; 
  sensorValue = 10*log10(sensorValueAux); 
  sensorValue = 20*log10(sensorValueAux/1024);
  sensorValue = 20*log10(sensorValueAux/354)+35; 
  sensorValue = 10*log10(10/6) + 20*log10(5);  
  sensorValue = (sensorValueAux*25)/400; Serial.print(" |10: "); Serial.print(sensorValue);
  sensorValue = (sensorValueAux) * (5.0 / 1023);Serial.print(" |11: "); Serial.println(sensorValue);
  */

  Serial.print("\t dB: ");
  Serial.println(sensorValue);
  
  if(sensorValue > limite){
    digitalWrite(D7, HIGH);
  }
  else{
    digitalWrite(D7, LOW);
  }
  
  String db = String(sensorValue);
  String l = String(limite);
  String c = String(conforto);

  String payload = "{";
  payload += "\"Ruido_dB\":"; payload+= db; payload += ",";
  payload += "\"Conforto_dB\":"; payload+= c; payload += ",";
  payload += "\"Limite_dB\":"; payload+= l; 
  payload +="}";


  char attributes[100];
  payload.toCharArray(attributes, 100);
  MQTT.publish("v1/devices/me/telemetry", attributes);
  //Serial.println(attributes);
  
  
}

void getAndSendTemperatureAndHumidityData(float tempS, float tempI, float humS, float humI)
{
  //Serial.println("Collecting temperature data.");

  // Reading temperature or humidity takes about 250 milliseconds!
  float h = dht.readHumidity();
  // Read temperature as Celsius (the default)
  float t = dht.readTemperature();

  if(t > tempS || t < tempI){
    digitalWrite(D3, HIGH);
  }
  else{
    digitalWrite(D3, LOW);
  }

  
  if(h > humS || h < humI){
    digitalWrite(D5, HIGH);
  }
  else{
    digitalWrite(D5, LOW);
  }
  
  // Check if any reads failed and exit early (to try again).
  if (isnan(h) || isnan(t)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }
  Serial.print("Humidity: ");
  Serial.print(h);
  Serial.print("%\t Temperature: ");
  Serial.print(t);
  Serial.println(" *C ");

  String temperature = String(t);
  String humidity = String(h);

  /*
  // Just debug messages
  Serial.print( "Sending temperature and humidity : [" );
  Serial.print( temperature ); Serial.print( "," );
  Serial.print( humidity );
  Serial.print( "]   -> " );
  */
  
  // Prepare a payload string
  String lts = String(tempS);
  String lti = String(tempI);
  String lhs = String(humS);
  String lhi = String(humI);
  
  
  String payload = "{";
  payload += "\"Temperatura\":"; payload += temperature; payload += ",";
  payload += "\"Limite_Temperatura_Superior\":"; payload += lts; payload += ",";
  payload += "\"Limite_Temperatura_Inferior\":"; payload += lti;
  payload += "}";

  // Send payload
  char attributes[100];
  payload.toCharArray( attributes, 100 );
  MQTT.publish( "v1/devices/me/telemetry", attributes );
  //Serial.println( attributes );

  payload = "{";
  payload += "\"Umidade\":"; payload += humidity; payload += ",";
  payload += "\"Limite_Umidade_Superior\":"; payload += lhs; payload += ",";
  payload += "\"Limite_Umidade_Inferior\":"; payload += lhi;
  payload += "}";
  
  // Send payload
  attributes[100];
  payload.toCharArray( attributes, 100 );
  MQTT.publish( "v1/devices/me/telemetry", attributes );
  //Serial.println( attributes );
}

  void InitWiFi() 
{
    delay(10);
    Serial.println("------Conexao WI-FI------");
    Serial.print("Conectando-se na rede: ");
    Serial.println(SSID);
    Serial.println("Aguarde");
    
    reconnect();
}

void reconnect() 
{
    //se já está conectado a rede WI-FI, nada é feito. 
    //Caso contrário, são efetuadas tentativas de conexão
    if (WiFi.status() == WL_CONNECTED)
        return;
        
    WiFi.begin(SSID, PASSWORD); // Conecta na rede WI-FI
    
    while (WiFi.status() != WL_CONNECTED) 
    {
        delay(100);
        Serial.print(".");
    }

    Serial.println();
    Serial.print("Conectado com sucesso na rede ");
    Serial.print(SSID);
    Serial.println("IP obtido: ");
    Serial.println(WiFi.localIP());

}

//Função: reconecta-se ao broker MQTT (caso ainda não esteja conectado ou em caso de a conexão cair)
//        em caso de sucesso na conexão ou reconexão, o subscribe dos tópicos é refeito.
//Parâmetros: nenhum
//Retorno: nenhum
void reconnectMQTT() 
{
  while (!MQTT.connected()) 
      {
          Serial.print("* Tentando se conectar ao Broker MQTT: ");
          Serial.println(BROKER_MQTT);
          //if (MQTT.connect(ID_MQTT))
          if (MQTT.connect("Default", TOKEN, NULL))
          {
              Serial.println("Conectado com sucesso ao broker MQTT!");
              //MQTT.subscribe(TOPICO_SUBSCRIBE); 
          } 
          else 
          {
              Serial.println("Falha ao conectar no broker.");
              Serial.println("Havera nova tentatica de conexao em 2s");
              delay(2000);
          }
      }
}

