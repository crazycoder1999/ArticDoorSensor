
#include <WiFi101.h>
#include <ArduinoJson.h>
//TESTED WITH GENUINO/ARDUINO MKR1000
//Web REST API params
char server[] = "api.artik.cloud";  
int port = 443; //(port 443 is default for HTTPS)
String AuthorizationData = "Authorization: Bearer <DEVICE TOKEN>"; // CHANGE HERE , YOU FOUND THE VALUE ON ARTIC.CLOUD/mydevices

float insTemp;
char buf[200];
#define LED 6
#define DOORSWITCH 2
#define OPENSTATE 1
#define CLOSEDSTATE 0

int ledState = 0;
int openCount = 0;  //the door changed to open in the last 15 minutes
int closedCount = 0; //the door changed to closed
int failedSend = 0; //number of failed send of data.. it counts how many failed times it tried to send the data. 

WiFiSSLClient client;
char ssid[] = "<YOURWIFISSID>";      // CHANGE HERE
char pass[] = "<YOURWIFIPASSWORD>"; // CHANGE HERE
int status = WL_IDLE_STATUS;

unsigned long lastRequest = 0L;
const unsigned long TIMEFORUPDATE = 1000L * 15L * 60L; //15 minutes
int doorState = 0;

void setup() {
	Serial.begin(9600);
 	pinMode(LED,OUTPUT);
	pinMode(DOORSWITCH,INPUT_PULLUP); 
	Serial.println("Starting.");
}

//onboard led off
void ledOFF() {
	digitalWrite(LED,LOW);
}

//onboard led on
void ledON() {
	digitalWrite(LED,LOW);
}

//loop to attempt connecting to wifi
void connectWiFi() {
	int retry = 10;
	while (status != WL_CONNECTED) {
		Serial.print   ("Attempting to connect to WiFi ");
		Serial.println (retry);
		status = WiFi.begin(ssid,pass);
		delay(30); 
		retry--;
		if(retry < 0) {
			Serial.println("Failed to connect..");
			break;		
		}
	}
	if( status != WL_CONNECTED && retry >= 0 ) {
		Serial.println("connected to WiFi ");
  }
}

// helper function.. 
void disconnectWiFi() {
	WiFi.disconnect();
}

//http post request to artic cloud
void newArticRequest() {
	client.connect(server, port);

	if (!client.connected()) { 
		Serial.println("Artic Request Error.");
		failedSend++;
	} else {

		Serial.println("Data open "+String(openCount) +" closed "+ String(closedCount));
		client.println("POST /v1.1/messages HTTP/1.1");
		client.println("Host: api.artik.cloud");
		client.println("Accept: */*");
		client.println("Content-Type: application/json");
		client.println("Connection: close");
		client.println(AuthorizationData);

       		// Automated POST data section
		client.print("Content-Length: ");
		client.println(makeJSON()); // loads buf, returns length
		client.println();
		client.println(buf);
		Serial.println("Artic Request Sent.");
		client.stop();       
		failedSend=0;
	}

	lastRequest = millis();
}

//this check , to control if the device should send data to artic
bool timeToSend(){
  
	if( (millis() - lastRequest) > TIMEFORUPDATE ) {
    		Serial.println("TIME TO SEND OK.");
		return true;
	}
	//Serial.println("NOT READY TO SEND." + String(millis()) +" "+ String (lastRequest) );
	return false;
}
  
void loop() {
  
	int tmpDoorSwitch = digitalRead(DOORSWITCH); //check the switch
  	Serial.println("Switch status: " + String(tmpDoorSwitch));
	if( tmpDoorSwitch != doorState ) { //check the door switch, if it is changed
		doorState = tmpDoorSwitch;
		if(doorState == OPENSTATE) //the door changed to open
			openCount++;
		if(doorState == CLOSEDSTATE) //the door changed to close
			closedCount++;
	}
	if( timeToSend() ) {
		delay(200);
		Serial.println("TIME TO SEND.");
		ledON();
		Serial.println("Connecting...");
		connectWiFi();
		newArticRequest(); 
		disconnectWiFi();
    		Serial.println("DONE...");
		ledOFF();
		openCount = 0;
		closedCount = 0;
	}
	delay(1000);
}

//prepare JSON packet to send
int makeJSON() {  
	StaticJsonBuffer<250> jsonBuffer; // reserve spot in memory

	JsonObject& root = jsonBuffer.createObject(); // create root objects
	root["sdid"] = "<DEVICE_ID>"; // CHANGE HERE , YOU FOUND THE VALUE ON ARTIC.CLOUD/mydevices
	root["type"] = "message";

	JsonObject& dataPair = root.createNestedObject("data"); // create nested objects
	dataPair["open"] = openCount;  
	dataPair["close"] = closedCount;  
	dataPair["failed"] = failedSend;  

	root.printTo(buf, sizeof(buf)); // JSON-print to buffer

	return (root.measureLength()); // also return length
}

