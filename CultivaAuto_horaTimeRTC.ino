//  ==============================--------------------------------------- Librerias ------------------------------------==========================================
#include <Arduino.h>

#include <ESP8266WiFi.h>
#include <Hash.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>

#include <Adafruit_Sensor.h>
#include <DHT.h>

//#include <Time.h>

// ============ Librerias RTC ==================
#include <Wire.h>
#include "RTClib.h"

RTC_DS1307 rtc;

#include <Time.h>

// ======================= DEACRIPCION PINES ==========================================================0
/**
 * D1 = RTC SCL
 * D2 = RTC SDA
 * D3 = RELE IN1
 * D4 = DHT 22 
 * D5 = RELE IN2
 * D6 = RELE IN3
 * D7 = RELE IN4
 * D8 = RELE IN5
 */

//=================================================================================================================================================================================
//--------------- Declaracion de Varables  -----------------------


// ------- RED GENERADA -------------------------
const char* passwordNode = "cultiva2022";
const char* ssidNode = "Cultiva_Node_1";

#define DHTPIN 0     // pin D3
#define DHTTYPE    DHT22     // DHT 22 (AM2302)
DHT dht(DHTPIN, DHTTYPE);

// ---- RSSI -----
long rssi;

//----  Rele ----
int pinReleLuz = 2; // D4
int pinReleRiego = 14; // D5
int pinReleVentilador = 12; // D6
int pinReleAux1 = 13; // D7
int pinReleAux2 = 15; // D8
//int pinReleAux3 = 13; // D7

//------- parametros encendido automatico -------------
int horaInicioAutLuz = 18; // Hora inicio automatizacion luz
int minutoInicioAutLuz = 0; // Minuto inicio automatizacion luz
int horaFinAutLuz = 23; // Hora fin automatizacion luz
int minutoFinAutLuz = 0; // Minuto inicio automatizacion luz


// current temperature & humidity, updated in loop()
float t = 0.0;
float h = 0.0;

String fechaHora;
String estadoLuzStr="APAGADO";
String estadoLuzForceStr = "AUTONOMO";

//variables de control
boolean estadoLuz = false;
boolean estadoLuzForce = false;

boolean estadoRiego = false;
String estadoRiegoStr="APAGADO";

// Variables Fecha RTC
int segundo,minuto,hora,dia,mes;
long anio; //variable año
DateTime HoraFecha;

boolean isSetHoraLocal = true;
// Create AsyncWebServer object on port 80
AsyncWebServer server(80);

// Generally, you should use "unsigned long" for variables that hold time
// The value will quickly become too large for an int to store
unsigned long previousMillis = 0;    // will store last time DHT was updated

// Updates DHT readings every 10 seconds
const long interval = 1000;  

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>
<head>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <style>
    html {
     font-family: Arial;
     display: inline-block;
     margin: 0px auto;
     text-align: center;
    }
    h2 { font-size: 2.0rem; }
    p { font-size: 2.0rem; }
    .units { font-size: 1.2rem; }
    .labels{
      font-size: 1 rem;
      vertical-align:middle;
      padding-bottom: 10px;
    }
    
      .boton_amarillo{
      display: block; 
      text-align: center;
      align-items: center;
      vertical-align: middle;line-height: 80px;
      text-decoration: none;
      padding: 10px; 
      width:200px; 
      height:108px;
      font-weight: 400;
      font-size: 30px;
      color: #000000;
      background-color: #E6E6DA;
      border-radius: 6px;
      border: 2px solid #0016b0;
      }

      .boton_config{
        display: block; 
        text-align: center;
        align-items: center;
        vertical-align: middle;
        line-height: 80px;
        text-decoration: none;
        padding: 10px; 
        width:200px; 
        height:108px;
        font-weight: 400;
        font-size: 30px;
        color: #ffffff;
        background-color: #8C623E;
        border-radius: 6px;
        border: 2px solid #0ca66e;
        }
        
      .boton_azul_claro{
        display: block; 
        text-align: center;
        align-items: center;
        vertical-align: middle;
        line-height: 80px;
        text-decoration: none;
        padding: 10px; 
        width:200px; 
        height:108px;
        font-weight: 400;
        font-size: 30px;
        color: #ffffff;
        background-color: #2A4D39;
        border-radius: 6px;
        border: 2px solid #0ca66e;
      }
      .boton_azul_ventilador{
        display: block; 
        text-align: center;
        align-items: center;
        vertical-align: middle;
        line-height: 80px;
        text-decoration: none;
        padding: 10px; 
        width:200px; 
        height:108px;
        font-weight: 400;
        font-size: 30px;
        color: #ffffff;
        background-color: #85A65B;
        border-radius: 6px;
        border: 2px solid #0ca66e;
      }

      .auxiliar{
        display: block; 
        text-align: center;
        align-items: center;
        vertical-align: middle;
        line-height: 80px;
        text-decoration: none;
        padding: 10px; 
        width:200px; 
        height:108px;
        font-weight: 400;
        font-size: 30px;
        color: #ffffff;
        background-color: #CED998;
        border-radius: 6px;
        border: 2px solid #0ca66e;
      }
  </style>
</head>
<body>
  <h2>--- Node 1 ---</h2>
  <p>
    <span class="labels">Temperature</span> 
    <span id="temperature">%TEMPERATURE%</span>
    <sup class="units">&deg;C</sup>
  </p>
  <p>
    <span class="labels">Humidity</span>
    <span id="humidity">%HUMIDITY%</span>
    <sup class="units">%</sup>
  </p>

  <div align='center'>
    <TABLE>
      <TR>
        <TD>
        </TD>
      </TR>
      <TR>
        
      </TR>
      <TR>
        <TD>
          <p id="botonLuz" class='boton_amarillo' onclick="encenderLuz()">
            LUZ
          </p>
        </TD>
      </TR>
      <TR>
        <TD>
          
        </TD> 
      </TR>
      <TR>
        <TD>
           <p>
            <span id="estadoLogica">%ESTADO%</span>
          </p>
        </TD> 
      </TR>
      <TR>
        
      </TR>
      <TR>
        <TD>
          <p>
            <span id="fechahora">%DATE%</span>
          </p>
        </TD>
      </TR>
      
      </TR>
      <TR>
        <TD>
          <a class='boton_config' >
            Config
          </a>
        </TD>
      </TR>
      <TR>
        
      </TR>
      <TR>
        <TD>
        </TD>
      </TR>
      
      </TR>
      <TR>
        <TD>
          <a class='boton_azul_claro' onclick="actionRiego()">
            Riego
          </a>
        </TD>
      </TR>
      <TR>
        
      </TR>
      <TR>
        <TD>
          
        </TD> 
      </TR>
      
      </TR>
      <TR>
        <TD>
          <a class='boton_azul_ventilador'>
            Ventilador
          </a>
        </TD>
      </TR>
      <TR>
        
      </TR>
      <TR>
        <TD>
        </TD>
      </TR>
      
      </TR>
      <TR>
        <TD>
          <a class='auxiliar' >
            Auxiliar 1
          </a>
        </TD>
      </TR>
      <TR>
        
      </TR>
      <TR>
        <TD>
        </TD> 
      </TR>
      
      </TR>
      <TR>
        <TD>
          <a class='auxiliar'>
            Auxiliar 2
          </a>
        </TD>
      </TR>
      <TR>
        
      </TR>
      <TR>
        <TD>
        </TD> 
      </TR>
      
      </TR>
      <TR>
        <TD>
          <a class='auxiliar'>
            Auxiliar 3
          </a>
        </TD>
      </TR>

      <TR>
        
      </TR>
      <TR>
        <TD>
        </TD> 
      </TR>
      
      </TR>
      <TR>
        <TD>
          <a class='auxiliar' onclick="restaurar()">
            Restaurar
          </a>
        </TD>
      </TR>
  
  
    </TABLE>
  </div>
  
</body>
<script>
setInterval(function ( ) {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("temperature").innerHTML = this.responseText;
    }
  };
  xhttp.open("GET", "/temperature", true);
  xhttp.send();
}, 5000 ) ;

setInterval(function ( ) {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("humidity").innerHTML = this.responseText;
    }
  };
  xhttp.open("GET", "/humidity", true);
  xhttp.send();
}, 5000 ) ;

setInterval(function ( ) {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      if(xhttp.responseText == "ENCENDIDO"){
        document.getElementById("botonLuz").style.backgroundColor= "#EBE72F";
      } else if(xhttp.responseText == "APAGADO"){
        document.getElementById("botonLuz").style.backgroundColor= "#E6E6DA";
      }
    }
  };
  xhttp.open("GET", "/luzDaemon", true);
  xhttp.send();
}, 400 ) ;

setInterval(function ( ) {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("fechahora").innerHTML = this.responseText;
    }
  };
  xhttp.open("GET", "/fechahora", true);
  xhttp.send();
}, 1000 ) ;

function encenderLuz() {
  var xhttp = new XMLHttpRequest();
  
  
  xhttp.open("GET", "/luz", true);

  xhttp.onreadystatechange = function (aEvt) {
    if (xhttp.readyState == 4) {
       if(xhttp.status == 200){
          if(xhttp.responseText == "ENCENDIDO"){
            document.getElementById("botonLuz").style.backgroundColor= "#EBE72F";
          } else if(xhttp.responseText == "APAGADO"){
            document.getElementById("botonLuz").style.backgroundColor= "#E6E6DA";
          }
       }
    }
  };
  
  xhttp.send();
 }

 function restaurar() {
  var xhttp = new XMLHttpRequest();
  
  
  xhttp.open("GET", "/restaurar", true);

  xhttp.onreadystatechange = function (aEvt) {
    if (xhttp.readyState == 4) {
       if(xhttp.status == 200){
       }
    }
    
  };
  xhttp.send();
 }

 setInterval(function ( ) {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("estadoLogica").innerHTML = this.responseText;
    }
  };
  xhttp.open("GET", "/estadologicaDaemon", true);
  xhttp.send();
}, 400 ) ;

function actionRiego() {
  var xhttp = new XMLHttpRequest();
  
  
  xhttp.open("GET", "/riego", true);

  xhttp.onreadystatechange = function (aEvt) {
    if (xhttp.readyState == 4) {
       if(xhttp.status == 200){
       }
    }
  };
  
  xhttp.send();
 }

</script>
</html>)rawliteral";

String processor(const String& var){
  Serial.println(var);
  if(var == "TEMPERATURE"){
    return String(t);
  } else if(var == "HUMIDITY"){
    return String(h);
  } else if (var == "DATE"){
    return fechaHora;
  } else if (var == "ESTADO"){
    return fechaHora;
  }
  return String();
}

String eventos(const String& var){
  Serial.println(var);
  
  return String();
}

//  ------------------------------------------------------------------ Setups  ----------------------------------------------------------------------//

void setup(){
  // Serial port for debugging purposes
  Serial.begin(115200);
  //rtc.adjust(DateTime(2022,3,20,22,32,0));
  inicializarPines();

  rtc.begin(); //Inicializamos el RTC
  dht.begin();

  //setTime(13,51,30,18,03,2022);

  WiFi.mode(WIFI_AP_STA);
  
  Serial.print("Setting AP (Access Point)…");
  // Remove the password parameter, if you want the AP (Access Point) to be open
  WiFi.softAP(ssidNode, passwordNode);

  IPAddress IP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(IP);

  // Print ESP8266 Local IP Address
  Serial.println(WiFi.localIP());

  // Route for root / web page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", index_html, processor);
  });
  server.on("/temperature", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", String(t).c_str());
  });
  server.on("/humidity", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", String(h).c_str());
  });

  server.on("/fechahora", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", fechaHora.c_str());
  });
  server.on("/luz", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", estadoLuzStr.c_str());
    cambiarEstadoLuz();
  });

  server.on("/luzDaemon", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", estadoLuzStr.c_str());
  });

  server.on("/restaurar", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", estadoLuzForceStr.c_str());
    estadoLuzForce = false;
    estadoLuzForceStr = "AUTONOMO";
    Serial.println("=========  AUTONOMO  =========");
  });

  server.on("/estadologicaDaemon", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", estadoLuzForceStr.c_str());
  });

  server.on("/riego", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", estadoLuzStr.c_str());
    cambiarEstadoRiego();
  });

  // Start server
  server.begin();
}

//-----------------------------------------------------------------  loops  ---------------------------------------------------------------------------------------//
 
void loop(){  

  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
    // save the last time you updated the DHT values
    //loopRelog();
    loopRTC();
    loopSenal();
    loopAcciones();
    previousMillis = currentMillis;
    // Read temperature as Celsius (the default)
    float newT = dht.readTemperature();
    // Read temperature as Fahrenheit (isFahrenheit = true)
    //float newT = dht.readTemperature(true);
    // if temperature read failed, don't change t value
    if (isnan(newT)) {
      Serial.println("Fallo leyendo datos");
    }
    else {
      t = newT;
      Serial.println(t);
    }
    // Read Humidity
    float newH = dht.readHumidity();
    // if humidity read failed, don't change h value 
    if (isnan(newH)) {
      Serial.println("Fallo leyendo DHT sensor!");
    }
    else {
      h = newH;
      Serial.println(h);
    }
  }

  if(isSetHoraLocal){
    setHoraLocal();
  }
}

// -------------------------------------Loop Hora --------------------------------
/**
* @Autor RagooS
* @Descripccion Metodo permite capturar informacion para la hora del sistema
* @Fecha 17/03/22
*/
/*void loopRelog() {
  // put your main code here, to run repeatedly:
  time_t t = now();

  fechaHora = (String) hour(t);
  fechaHora += ":";
  fechaHora += (String) minute(t);
  fechaHora += ":";
  fechaHora += (String) second(t);
  fechaHora += " - ";

  fechaHora += (String) day(t);
  fechaHora += "/";
  fechaHora += (String) month(t);
  fechaHora += "/";
  fechaHora += (String) year(t);
  Serial.println(fechaHora);

  if(estadoLuzForce){
    Serial.println("ESTADO LUZ FORSADA");
  } else {
    if(hour(t)>=horaInicioAutLuz && hour(t)<=horaFinAutLuz){

      if(horaInicioAutLuz == horaFinAutLuz){
        if(minute(t) >= minutoInicioAutLuz && minute(t) <= minutoFinAutLuz){
          if(!estadoLuz){
            encenderLuz();
          }
        } else {
          if(estadoLuz){
            apagarLuz();
          }
        }
      } else if(hour(t) == horaInicioAutLuz){
        if(minute(t) >= minutoInicioAutLuz){
          if(!estadoLuz){
            encenderLuz();
          }
        } else {
          if(estadoLuz){
            apagarLuz();
          }
        }
      } else if(hour(t) == horaFinAutLuz){
        if(minute(t) <= minutoFinAutLuz){
          if(!estadoLuz){
            encenderLuz();
          }
        } else {
          if(estadoLuz){
            apagarLuz();
          }
        }
      } else {
        if(!estadoLuz){
          encenderLuz();
        }
      }
      
    } else {
      if(estadoLuz){
        apagarLuz();
      }
    }
  }
}*/

void setHoraLocal(){
  loopRTC();
  setTime(hora,minuto,segundo,dia,mes,anio);
  isSetHoraLocal = false;
}

/**
* @Autor RagooS
* @Descripccion Metodo permite cambiar el estado de la luz
* @Fecha 29/03/22
*/
void cambiarEstadoRiego(){
  if(estadoRiego){
      apagarRiego();
    } else {
      encenderRiego();
    }
}

/**
* @Autor RagooS
* @Descripccion Metodo permite enviar señal de encendido al rele
* @Fecha 29/03/22
*/
void encenderRiego(){
  digitalWrite(pinReleRiego,LOW);
  estadoRiego = true;
  //estadoLuzStr = "ENCENDIDO";
  Serial.println("RIEGO ENCENDIDO");
}

/**
* @Autor RagooS
* @Descripccion Metodo permite enviar señal de encendido al rele
* @Fecha 29/03/22
*/
void apagarRiego(){
  digitalWrite(pinReleRiego,HIGH);
  estadoRiego = false;
  //estadoLuzStr = "ENCENDIDO";
  Serial.println("RIEGO APAGADO");
}


/**
* @Autor RagooS
* @Descripccion Metodo permite enviar señal de encendido al rele
* @Fecha 17/03/22
*/
void encenderLuz(){
  digitalWrite(pinReleLuz,LOW);
  estadoLuz = true;
  estadoLuzStr = "ENCENDIDO";
  Serial.println("ENCENDIDO");
}

/**
* @Autor RagooS
* @Descripccion Metodo permite enviar señal de agado al rele
* @Fecha 17/03/22
*/
void apagarLuz(){
  digitalWrite(pinReleLuz,HIGH);
  estadoLuz = false;
  estadoLuzStr = "APAGADO";
  Serial.println("APAGADO");
}

/**
* @Autor RagooS
* @Descripccion Metodo permite inicializar los pines del node
* @Fecha 17/03/22
*/
void inicializarPines(){

  pinMode(pinReleLuz, OUTPUT);
  pinMode(pinReleRiego, OUTPUT);
  pinMode(pinReleVentilador, OUTPUT);
  pinMode(pinReleAux1, OUTPUT);
  pinMode(pinReleAux2, OUTPUT);
  //pinMode(pinReleAux3, OUTPUT);
  
  digitalWrite(pinReleLuz,HIGH);
  digitalWrite(pinReleRiego,HIGH);
  digitalWrite(pinReleVentilador,HIGH);
  digitalWrite(pinReleAux1,HIGH);
  digitalWrite(pinReleAux2,HIGH);
  //digitalWrite(pinReleAux3,HIGH);
  
}

// --- loop Señal Wifi ------------------
void loopSenal() {
  unsigned long before = millis();
  rssi = WiFi.RSSI();
  
  unsigned long after = millis();
  //Serial.println(" ---- Signal strength: ");
  Serial.print(rssi);
  Serial.println("dBm");
}


// ------------------------------------- Señal Wifi RSSI --------------------------------

// Return RSSI or 0 if target ssidNode not found
int32_t getRSSI(const char* target_ssid) {
  byte available_networks = WiFi.scanNetworks();

  for (int network = 0; network < available_networks; network++) {
    String wifi = WiFi.SSID(network);
    char* wifi_ssid;
    wifi.toCharArray(wifi_ssid, 10);
    if (strcmp(wifi_ssid, target_ssid) == 0) {
      return WiFi.RSSI();
    }
  }
  return 0;
}

/**
* @Autor RagooS
* @Descripccion Metodo permite cambiar el estado de la luz
* @Fecha 17/03/22
*/
void cambiarEstadoLuz(){
  
  if(estadoLuz){
      estadoLuzForce = true;
      estadoLuzForceStr = "FORSADO";
      apagarLuz();
    } else {
      estadoLuzForce = true;
      estadoLuzForceStr = "FORSADO";
      encenderLuz();
    }
    
}

// ---------------------------------- Relog RTC -------------------------------------------------

void loopRTC(){
  HoraFecha = rtc.now(); //obtenemos la hora y fecha actual
    
  segundo=HoraFecha.second();
  minuto=HoraFecha.minute();
  hora=HoraFecha.hour();
  dia=HoraFecha.day();
  mes=HoraFecha.month();
  anio=HoraFecha.year();

  fechaHora = (String) hora;
  fechaHora += ":";
  fechaHora += (String) minuto;
  fechaHora += ":";
  fechaHora += (String) segundo;

  fechaHora += "  ";
  fechaHora += (String) dia;
  fechaHora += "/";
  fechaHora += (String) mes;
  fechaHora += "/";
  fechaHora += (String) anio;

  //Enviamos por el puerto serie la hora y fecha.
  Serial.print("Fecha Hora = ");
  Serial.print(fechaHora);
  Serial.println();
}

// ---------------------------------- accion en horas -------------------------------------------------
void loopAcciones(){
  
  if(estadoLuzForce){
    Serial.println("=====  ESTADO LUZ FORSADA  =====");
  } else {
    if(hora>=horaInicioAutLuz && hora<=horaFinAutLuz){

      if(horaInicioAutLuz == horaFinAutLuz){
        if(minuto >= minutoInicioAutLuz && minuto <= minutoFinAutLuz){
          if(!estadoLuz){
            encenderLuz();
          }
        } else {
          if(estadoLuz){
            apagarLuz();
          }
        }
      } else if(hora == horaInicioAutLuz){
        if(minuto >= minutoInicioAutLuz){
          if(!estadoLuz){
            encenderLuz();
          }
        } else {
          if(estadoLuz){
            apagarLuz();
          }
        }
      } else if(hora == horaFinAutLuz){
        if(minuto <= minutoFinAutLuz){
          if(!estadoLuz){
            encenderLuz();
          }
        } else {
          if(estadoLuz){
            apagarLuz();
          }
        }
      } else {
        if(!estadoLuz){
          encenderLuz();
        }
      }
      
    } else {
      if(estadoLuz){
        apagarLuz();
      }
    }
  }
}
