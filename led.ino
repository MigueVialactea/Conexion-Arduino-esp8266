#include <SoftwareSerial.h>
#include <Servo.h>
// arduino Rx (pin 2) ---- ESP8266 Tx
  // arduino Tx (pin 3) ---- ESP8266 Rx
SoftwareSerial esp8266(3,2);

//variables servomotor
Servo servoMotor;

//variables de distancia
int input;
#define trigPin 7
#define echoPin 6
long duracion, distancia ;

void setup(){
  Serial.begin(9600);  // monitor serial del arduino
  esp8266.begin(9600); // baud rate del ESP8255
 
  pinMode(13,OUTPUT);
  digitalWrite(13,LOW);
 
  sendData("AT+RST\r\n",2000);      // resetear módulo
  sendData("AT+CWMODE=1\r\n",1000); // configurar como cliente
  sendData("AT+CWJAP=\"Embebidos\",\"12345678\"\r\n",8000); //SSID y contraseña para unirse a red 
  sendData("AT+CIFSR\r\n",1000);    // obtener dirección IP
  sendData("AT+CIPMUX=1\r\n",1000); // configurar para multiples conexiones
  sendData("AT+CIPSERVER=1,80\r\n",1000);         // servidor en el puerto 80

  //inicializaciones del servo
  // Iniciamos el servo para que empiece a trabajar con el pin 9
  servoMotor.attach(10);
  // Inicializamos al ángulo 0 el servomotor
  servoMotor.write(0);

  //inicializamos las variables de la distancia
  pinMode(trigPin, OUTPUT);
pinMode(echoPin, INPUT); 
}

void loop(){
  if(esp8266.available()){   // revisar si hay mensaje del ESP8266
    if(esp8266.find("+IPD,")){ // revisar si el servidor recibio datos
      delay(1400); // esperar que lleguen los datos hacia el buffer
      int conexionID = esp8266.read()-48; // obtener el ID de la conexión para poder responder
      esp8266.find("led="); // bucar el texto "led="
      int state = (esp8266.read()-48); // Obtener el estado del pin a mostrar
      if(state==1){
      digitalWrite(13,HIGH);
      delay(1000);
      servoMotor.write(90);
      delay(1000);
      servoMotor.write(0);
      delay(1200);
      servoMotor.write(90);
      delay(1200);
      servoMotor.write(0);
      delay(1200);
      digitalWrite(13,LOW);
        //activar el servo
      }
      //posiblemente un delay para que se acomoden las croquetitas
      //digitalWrite(13, state); // Cambiar estado del pin
      
      while(esp8266.available()){
        char c = esp8266.read();
        Serial.print(c);
      }
 
 //responder y cerrar la conexión para que el navegador no se quede cargando página web a enviar
      //Activamos el sensor y mandamos el resultado
      digitalWrite(trigPin, LOW);        // Nos aseguramos de que el trigger está desactivado
            delayMicroseconds(2);              // Para asegurarnos de que el trigger esta LOW
            digitalWrite(trigPin, HIGH);       // Activamos el pulso de salida
            delayMicroseconds(10);             // Esperamos 10µs. El pulso sigue active este tiempo
            digitalWrite(trigPin, LOW);        // Cortamos el pulso y a esperar el echo
            duracion = pulseIn(echoPin, HIGH) ;
            distancia = duracion / 2 / 29.1  ;
            delay (500) ;                  // Para limitar el número de mediciones
      distancia-=14;
      distancia*=100;
      distancia/=50;
      distancia=100-distancia;
      if(distancia>100){distancia=100;}
      if(distancia<0){distancia=0;}
      float gramos=(distancia*4000)/100;
      String webpage ="<p style=\"text-align:center; font-family: Century-Gothic; color: #0A0A2A; font-size:20px;\">"+String(distancia) + "% "+String(gramos)+"gr.</p>";
      /*if (digitalRead(13)==1){//state==1
        webpage += "<p style=\"text-align:center; color: #990070; font-size:30px;\">Led 13 = Encendido!</h1>";
      }
      else{
        webpage += "<p style=\"text-align:center; color: #990070; font-size:30px;\">Led 13 = Apagado!</h1>";
      }Formato*/
 
 // comando para enviar página web
      String comandoWebpage = "AT+CIPSEND=";
      comandoWebpage+=conexionID;
      comandoWebpage+=",";
      comandoWebpage+=webpage.length();
      comandoWebpage+="\r\n";
      sendData(comandoWebpage,1000);
      sendData(webpage,1000);
 
 // comando para terminar conexión
      String comandoCerrar = "AT+CIPCLOSE=";
      comandoCerrar+=conexionID;
      comandoCerrar+="\r\n";
      sendData(comandoCerrar,2000);
    }
  }
}


//Enviar comando al esp8266 y verificar la respuesta del módulo, todo esto dentro del tiempo timeout
void sendData(String comando, const int timeout){
  long int time = millis(); // medir el tiempo actual para verificar timeout
  esp8266.print(comando); // enviar el comando al ESP8266
  while( (time+timeout) > millis()){ //mientras no haya timeout
    while(esp8266.available()){ //mientras haya datos por leer
      // Leer los datos disponibles
      char c = esp8266.read(); // leer el siguiente caracter
      Serial.print(c);
    }
  } 
  return;
}
