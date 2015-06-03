/*
  INTERFAZ WEB QUE CONTROLA A ARDUINO
 
 Servidor web que utiliza SD para mostrar una pagina web que permite configurar
 el comportamiento de los pines libres (los 6 analogicos y los digitales 3, 5, 6, 7, 8 y 9).
 Se usa una Arduino Wiznet Ethernet shield, el puerto serie y una tarjeta SD. 
 
 Tambien se pueden gernerar funciones javascript que automaticen las acciones sobre los
 pines de arduino mediante la interfaz web.
 
 Circuito:
 * Ethernet shield usa los  pines 10, 11, 12, 13
 * Tarjeta SD usa el pin 4.
 * Puerto serie usa los pines 0 y 1.
 
 * Las entradas analogicas configuradas por defecto en los pines A0 .. A5 (esto es opcional)
 
 
 creado el 11 de mayo de 2015
 por J. I. Gil Palacios
 
 basado en Web Server
 created 18 Dec 2009
 by David A. Mellis
 modified 9 Apr 2012
 by Tom Igoe
 
 basado en SD card read/write
 created   Nov 2010
 by David A. Mellis
 modified 9 Apr 2012
 by Tom Igoe
 
 El codigo de este ejemplo es de dominio pblico
 
 */

#include <SPI.h>
#include <Ethernet.h>
#include <SD.h>

File myFile;

// Pon una direccion MAC y la direccion IP debajo.
// la direccion IP dependera de la red local:
byte mac[] = { 
  0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
IPAddress ip(192,168,1,100);

// Inicializa la libreria del servidor Ethernet 
// con la direccione IP y el puerto que queramos usar 
// (el port 80 se usa por defecto para HTTP):
EthernetServer server(80);

const int chipSelect = 4; 

//arrays para gestionar ordenes y recepcion de informacion
//++DEBEMOS DECLARAR EN ENTRADAS LOS PINES DIGITALES QUE USAREMOS COMO ENTRADAS O SALIDAS PUES DESDE LA PAGINA WEB AUNQUE LOS CAMBIEMOS NO CAMBIAN EN ARDUINO++++
int entradas[]={1,1,0,0,1,1,1};//pines digitales que meten dato IMPUT (1) o generan salida OUTPUT (0) CORRESPONDEN A LOS NUMEROS DEL ARRAY POSIBLES
int posibles[]={2,3,5,6,7,8,9};//pines digitales que quedan libres para usar. por usar puerto serie se pierden (0,1) shield ethernet (10,11,12,13) y tarjeta SD (4)
int digitPWM[]={0,1,1,1,0,0,1};//pines que permiten salida PWM (1) los que no lo hacen (0), se corresponden en posicion con array posibles
//funcion para analizar la solicitud
String cadena=""; //Creamos una cadena de caracteres vacia que almacena la peticion http para su posterior analisis


void setup() {
 // Se abre la comunicacion serie y se espera hasta que el puerto se abre:
  Serial.begin(9600);
   while (!Serial) {
    ; // se espera a conectar con el puerto serie. Necesario solo para Leonardo.
  }
  //configuramos los 7 pines digitales disponibles para su uso como entrada o salida.
  for(int i=0;i<7;i++){
    if (entradas[i]==1){
      pinMode(posibles[i], INPUT);
      Serial.print("PIN: "); Serial.print(posibles[i]);Serial.println(" INPUT");
    }else{
      pinMode(posibles[i], OUTPUT);
      Serial.print("PIN: "); Serial.print(posibles[i]);Serial.println(" OUTPUT");
    }
  }
  // Iniciamos la conexion Etherne y el servidor:
  Ethernet.begin(mac, ip);
  server.begin();
  Serial.print("server is at ");
  Serial.println(Ethernet.localIP());
  ////
  Serial.print("Initializing SD card...");
  // En la Ethernet Shield, CS es el pin 4. Se configura como salida por defecto.
  // Note que incluso si no se usa como pin CS, El pin hardware SS
  // (10 en la mayoria de las placas Arduino, 53 en la Mega) debe ser por defecto configurada como salida 
  // o la libreria SD no funcionara. 
   pinMode(10, OUTPUT); //digitalWrite(10, HIGH);
   
  if (!SD.begin(4)) {
    Serial.println("initialization failed!");
    return;
  }
  Serial.println("initialization done.");
}

void loop() {
  // Escuchando clientes entrantes
  EthernetClient client = server.available();
  char aux;//varible para hacer la conversion a caracteres de lo leido de SD
  if (client) {
    //Serial.println("new client");
    // Una solicitud http termina con una linea en blanco
    boolean currentLineIsBlank = true;
    cadena=""; //Creamos una cadena de caracteres vacia
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        //Serial.write(c);
        cadena.concat(c);//vamos encadenando al String 'cadena' con el siguiente caracter de la peticion HTTP (c). De esta manera convertimos la peticion HTTP a un String
        
        // Si hemos recibido una linea en blanco se completo la solicitud y pasamos a generar la respuesta       
        if (c == '\n' && currentLineIsBlank) { 
          //Ya que hemos convertido la peticion HTTP a una cadena de caracteres, ahora podremos buscar partes del texto.
          int posicion=cadena.indexOf("/IFRAME"); //Guardamos la posicion de la instancia "destino=" a la variable 'posicion' que es lo que nos permite distinguir si es la primera llamda (pagina) o no (iframe)
 
          if(posicion>0)//Si se encuentra el termino debemos enviar las lecturas de de las entradas y a procesar las salidas
          {
            for (int analogChannel = 0; analogChannel < 6; analogChannel++) {//generamos las 6 entradas analogicas (siempre se envian todas)
              int sensorReading = analogRead(analogChannel);
              client.print(sensorReading);
              client.print("#");//separamos canal
            }
            client.print("#");//separamos analogicos de digitales poniendo un segundo #, se juntan dos ##
	    posicion=cadena.indexOf("codigo=")+7;//sumando las 7 letras de codigo
            String codigo=cadena.substring(posicion,posicion+35);//se obtiene la codificacion de lo que se hace sobre los pines digitales
            //Serial.println("++******++");
	    //Serial.println(cadena.substring(posicion,posicion+35));
            //Serial.println("++******++");
 
	    //analizamos el codigo de la solicitud para recabar los valores o ordenar las acciones
	    for(int i=0; i<7; i++){//se tratan por orden los 7 pines activos 2,3,5,6,7,8 y 9, si hay incongruencias se envia como entrada E mas el valor leido si es posible
                 String aux=codigo.substring(i*5,i*5+5);
                 if(entradas[i]==1){//tratamos las posibles entradas
                   int val = digitalRead(posibles[i]);
                   client.print(val);
                   if(aux.substring(0,1)=="0") client.print("E");//avisamos que no se puede escribir la entrada si el pin esta conectado como entrada
                 }else{//tratamos las salidas
                   if(aux.substring(0,1)=="1"){
                     client.print("E");//avisamos que se trata de leer en un pin conectado como salida
                   }else{//mandamos salida a uno conectado como salida
                     if(aux.substring(2,5)=="iii"){///mandamos una salida digital
                       client.print("-");//es salida no debe haber entrada por eso enviamos -
                       if(aux.substring(1,2)=="1") digitalWrite(posibles[i], HIGH);
                       else if(aux.substring(1,2)=="0") digitalWrite(posibles[i], LOW);
                       else client.print("nvE");//no es 0 o 1 y mandamos el error de valor distinto de 1 o 0 y por tanto no valido
                     }else{//queremos salida analogica por PWM
                       if(digitPWM[i]==1){//admite el tipo de salida PWM
                         int valoranalgico=aux.substring(2,5).toInt();//convertimos la cadena a entero
                         if (valoranalgico<0) valoranalgico=0;//ajustamos a 0 los valores negativos
                         else if (valoranalgico>255) valoranalgico=255;//ajustamos a 255 los valores mayores que el tope de 255
                         analogWrite(posibles[i], valoranalgico);//mandamos la salida analogica codificada
                         client.print("-");//es salida no debe haber entrada por eso enviamos -
                       }else{//no admite el tipo de salida PWM
                         client.print(posibles[i]);client.print("naE");//mandamos el error de que el puerto no admite salida analogica
                       }
                     }
                   }
                 }
	         if(i<6) client.print("#");//escribimos el separador de item, tras los valores del ultimo (sexto de los usables) pin (9) no debe ir
	    }
          }else if(cadena.indexOf("/CONFIGURA")>0){//primer acceso se debe pasar los pines digitales INPUT y OUTPUT que se han configurado en la placa arduino
            for (int i = 0; i < 6; i++) {//indicamos que pines digitales de los posibles se han puesto en INPUT y cuales en OUTPUT
              client.print(entradas[i]);
              client.print("#");//separamos canal
            }//el septimo no lleva separador
	    client.print(entradas[7]);
          }else{ //Para cualquier otra solicitud se devuelve la pagina IFRAME.HTM que tiene la interfaz web y se envia la pagina guardada en SD en el directorio raiz.
            //reinicializaSD();
            myFile = SD.open("IFRAME.HTM");
            if (myFile) {
              //Serial.println("IFRAME.HTM");
    
              // Se lee del archivo hasta que ya no que se complete la lectura:
              while (myFile.available()) {
                aux=myFile.read();//convertimos los numeros que se leen a caracter ya que aux es de tipo char
                client.print(aux);//se envia el caracter al cliente.
              }
              // cerramos el archivo:
              myFile.close();
            } else {
  	       // Si no se abre el archivo se envia al puerto seri un mensaje de error:
             // Serial.println("error abriendo IFRAME.HTM");
            }
          }
          break;
        }
        if (c == '\n') {
          // Se comienza una nueva linea
          currentLineIsBlank = true;
        } 
        else if (c != '\r') {
          // obtenemos un caracter en la nueva linea por tanto ya no es una linea en blanco
          currentLineIsBlank = false;
        }
      }
    }
    // le damos tiempo al navegador para que reciba los datos
    delay(1);
    // cerramos la conexion:
    client.stop();
    //Serial.println("cliente desconectado");
  }
}

