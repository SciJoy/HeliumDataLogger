//Libaries to be included
#include <WiFi.h> //To connect to wifi
#include <PubSubClient.h> //For MQTT

// Update these with values suitable for your network.
const char* ssid = "";  //Name of your wifi network
const char* password = ""; //Password for wifi network
const char* mqtt_server = ""; //For us, the static IP address of our RPi3

//Getting date and time
const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 3600;
const int   daylightOffset_sec = 3600;
unsigned long currentTime;

//Initializing arrays and variables

//The subArray is the inforamtion from topics sent from Pi.
//SubArray - {pin#, Start Time (in epoch), End Time(in epoch),Sample Rate (millis), Report Frequency (millis),Pin Type (0-Digtial, 1- Analog, 2-I2C)}
int subArray [21][6]={{2,0,0,0,0,0},{4,0,0,0,0,0},{5,0,0,0,0,0},{12,0,0,0,0,0},{13,0,0,0,0,0},{14,0,0,0,0,0},
{15,0,0,0,0,0},{16,0,0,0,0,0},{17,0,0,0,0,0},{18,0,0,0,0,0},{19,0,0,0,0,0},{21,0,0,0,0,0},{22,0,0,0,0,0},
{23,0,0,0,0,0},{25,0,0,0,0,0},{26,0,0,0,0,0},{27,0,0,0,0,0},{32,0,0,0,0,0},{33,0,0,0,0,0},{34,0,0,0,0,0},{35,0,0,0,0,0}};

//pubArray is tracking information that will be used to send data to the Pi.
//pubArray - {Count, Sum, Min, Max}
int pubArray [21][4]={{0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0},
{0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0},
{0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0},{0,0,0,0}};

//These track the time since last publishing and the last sample of data.
int lastReportFrequncy[21] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
int lastSampleRate[21] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

//Connecting to WiFi
WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
char msg[50];


void setup() {

  //Initialize pin modes
  for (int i = 0; i < 21; i++) {
    pinMode(subArray[i][0], INPUT);
    
  }
  pinMode(4, INPUT);
  //Setting up serial monitor
  Serial.begin(115200);

  //WiFi and PubSub functions for setup
  setup_wifi();
    client.setServer(mqtt_server, 1883);
    client.setCallback(callback);

  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  printLocalTime();
}

void loop() {
  //Connecting to internet and MQTT
  if (!client.connected()) {
    reconnect();
  }
  client.loop(); //I believe this sends it to the callback function

  //Looking for current time
  time_t now;
  currentTime = time(&now);

  //This read the digital and analog pins and tracks count, sum, min, and max
  for (int i = 0; i < 21; i++) {
      unsigned long now = millis(); //For time tracking



    if(subArray[i][1] <= currentTime <= subArray[i][2]){//If current is within Start and End times

      if(now - lastSampleRate[i] > subArray[i][3]){ //Tracks time since last sample and comparts to sample rate
        lastSampleRate[i] = now;

        if (subArray[i][5] == 0 && digitalRead(subArray[i][0]) == HIGH) {//If digital pin reads HIGH, increment count
          pubArray[i][0]++;
        }
        else if (subArray[i][5] == 1) {//Analog pin
          int raw = analogRead(subArray[i][0]);//Read pin

          //running average
          pubArray[i][0]++; //counting
          pubArray[i][1] = pubArray[i][1] + raw; //sum


          if(pubArray[i][0] == 1){//for first count
          pubArray[i][2] = raw; //set min to raw
          pubArray[i][3] = raw; //set max to raw
          }

          if (raw < pubArray[i][2]) {//Finding min value
            pubArray[i][2] = raw;
          }

          if (raw > pubArray[i][3]) {//Finding max value
            pubArray[i][3] = raw;
          }
        }

      }

      //Publishing to the Pi
      if(now - lastReportFrequncy[i] > subArray[i][4]){//Time tracking
        int average;
        lastReportFrequncy[i] = now;

        if (pubArray[i][0] == 0){ //To prevent dividing by zero if no digitial counts occur
         average = 00;
         } else{
          average = pubArray[i][1]/pubArray[i][0]; //sum/counts
        }

        //Publishing current time, count, min, max, avg, pin number
        snprintf (msg, 75, "%d,%d,%d,%d,%d,%d",currentTime,pubArray[i][0],pubArray[i][2],pubArray[i][3],average,subArray[i][0]);
        Serial.println("");
        Serial.print("~~Publish message: ");
        Serial.println(msg);
        client.publish("ToPi/ESP3", msg);

        //reset the tracking array
        pubArray[i][0]=0;
        pubArray[i][1]=0;
        pubArray[i][2]=0;
        pubArray[i][3]=0;
               }
    }

delay(5); //for testing
  }

}




void setup_wifi() {

  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

//Need to put all the topics to subscribe to in this section
void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("ESP8266Client")) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      client.publish("outTopic", "Connected to Out Topic ESP3");
      // ... and resubscribe
      client.subscribe("FromPi/ESP3");


    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retryin g
      delay(5000);
    }
  }
}

void callback(char* topic, byte* payload, unsigned int length) {
//  Serial.println("");
//  Serial.print("Message arrived [");
//  Serial.print(topic);
//  Serial.print("] ");

  //Converts the payload to a char array
  char payloadArray[55];
  for (int i = 0; i < length; i++) {
   Serial.print((char)payload[i]);
    payloadArray[i] = (char)payload[i];
  }

  //Converts the topic to a char array
  char topicArray[60];
  for (int i = 0; i < 18; i++) {
    Serial.print((char)topic[i]);
    topicArray[i] = (char)topic[i];
  }
  Serial.println();

  //adds nulls to the end of the arrays to convert them
  int k = length + 1;
  payloadArray[k] = NULL;
  topicArray[k] = NULL; //look at why this works with length

  String str(topicArray);
  Serial.print("string: ");
  Serial.println(str);

int configArray[6] = {0,0,0,0,0,0}; 
  byte i=0;
  char * pch;
  pch = strtok (payloadArray,"-");
  while (pch != NULL)
  {
    configArray[i] = atoi(pch);
    Serial.println(configArray[i]);
    pch = strtok (NULL,"-");
    i++;
  }

//---------By Pin------------------- 
if (configArray[0] == 2){
   for (int i = 1; i<6; i++){
     subArray[0][i] = configArray[i];
   }
}

if (configArray[0] == 4){
   for (int i = 1; i<6; i++){
     subArray[1][i] = configArray[i];
   }
}

if (configArray[0] == 5){
   for (int i = 1; i<6; i++){
     subArray[2][i] = configArray[i];
   }
}

if (configArray[0] == 12){
   for (int i = 1; i<6; i++){
     subArray[3][i] = configArray[i];
   }
}

if (configArray[0] == 13){
   for (int i = 1; i<6; i++){
     subArray[4][i] = configArray[i];
   }
}

if (configArray[0] == 14){
   for (int i = 1; i<6; i++){
     subArray[5][i] = configArray[i];
   }
}

if (configArray[0] == 15){
   for (int i = 1; i<6; i++){
     subArray[6][i] = configArray[i];
   }
}

if (configArray[0] == 16){
   for (int i = 1; i<6; i++){
     subArray[7][i] = configArray[i];
   }
}

if (configArray[0] == 17){
   for (int i = 1; i<6; i++){
     subArray[8][i] = configArray[i];
   }
}

if (configArray[0] == 18){
   for (int i = 1; i<6; i++){
     subArray[9][i] = configArray[i];
   }
}

if (configArray[0] == 19){
   for (int i = 1; i<6; i++){
     subArray[10][i] = configArray[i];
   }
}

if (configArray[0] == 21){
   for (int i = 1; i<6; i++){
     subArray[11][i] = configArray[i];
   }
}

if (configArray[0] == 22){
   for (int i = 1; i<6; i++){
     subArray[12][i] = configArray[i];
   }
}

if (configArray[0] == 23){
   for (int i = 1; i<6; i++){
     subArray[13][i] = configArray[i];
   }
}


if (configArray[0] == 25){
   for (int i = 1; i<6; i++){
     subArray[14][i] = configArray[i];
   }
}

if (configArray[0] == 26){
   for (int i = 1; i<6; i++){
     subArray[15][i] = configArray[i];
   }
}

if (configArray[0] == 27){
   for (int i = 1; i<6; i++){
     subArray[16][i] = configArray[i];
   }
}

if (configArray[0] == 32){
   for (int i = 1; i<6; i++){
     subArray[17][i] = configArray[i];
   }
}

if (configArray[0] == 33){
   for (int i = 1; i<6; i++){
     subArray[18][i] = configArray[i];
   }
}

if (configArray[0] == 34){
   for (int i = 1; i<6; i++){
     subArray[19][i] = configArray[i];
   }
}

if (configArray[0] == 35){
   for (int i = 1; i<6; i++){
     subArray[20][i] = configArray[i];
   }
}

Serial.println("");

}

void printLocalTime()
{
  struct tm timeinfo;
  if(!getLocalTime(&timeinfo)){
    Serial.println("Failed to obtain time");
    return;
  }
  Serial.println(&timeinfo);
}

