#include <Arduino.h>
#include <RCSwitch.h>
#include "config.h"

RCSwitch mySwitch = RCSwitch();

bool clock_10ms, clock_10ms_ant, pulso_10ms;
bool clock_100ms, clock_100ms_ant, pulso_100ms;
bool clock_1s, clock_1s_ant, pulso_1s;

unsigned long isr_duration_acum;
unsigned long isr_align_duration_acum;
unsigned long loop_duration_acum;

unsigned int isr_duration_cnt = 0;
unsigned int isr_align_duration_cnt = 0;
unsigned int loop_duration_cnt = 0;

typedef union {
  struct
  {
    bool parity;
    int ID : 3;
    int cmd : 8;
    int checksum : 4;
  };
  unsigned int word;
} MPX_packet;

MPX_packet MPX_buffer[10];
int MPX_buffer_index;

// Trama MPX
//----------
// Largo del paquete: 16 bits + 1 bit de start
// Tiempo del paquete: 61ms para 16 bits + bit de start
// Bit de start: 1.27ms
// Bit de datos: 3.8125ms

// Procedimiento de lectura
//-------------------------
// Detecto flancos y mido el tiempo entre cambios
// primer flanco descendente: comienza el bit de start
// primer flanco ascendente: comienza el bit de dato
// Leer 16 bits
// El último cambio es un flanco ascendente
// Si el tiempo entre flancos supera los 3.81ms, se considera timeout y se cancela el paquete
//

// ***********************************************************
// Configura los puertos del Arduino
// ***********************************************************
void setup_GPIOs()
{
  Serial.println("Setup GPIOs");
  pinMode(RX_PIN, INPUT);
  digitalWrite(TX_PIN, 0);
  pinMode(TX_PIN, OUTPUT);
}

void setup_RCSwitch()
{
  Serial.println("Setup RCSwitch: RX on pin 2. TX on pin 13");
  
  // Transmitter is connected to Arduino Pin #13
  mySwitch.enableTransmit(13);

  // Receiver on pin #2 => that is inerrupt 0
  mySwitch.enableReceive(digitalPinToInterrupt(2));

  // Optional set protocol (default is 1, will work for most outlets)
  mySwitch.setProtocol(13);
}

// ***********************************************************
// Llamada a las funciones de inicialización
// ***********************************************************
void setup()
{
  Serial.begin(19200);
  setup_GPIOs();
  setup_RCSwitch();
}

//********************************************************
// Timers
//********************************************************
void timers()
{
  clock_10ms_ant = clock_10ms;
  clock_10ms = (millis() % 10) >= 5;
  pulso_10ms = clock_10ms && !clock_10ms_ant;

  clock_100ms_ant = clock_100ms;
  clock_100ms = (millis() % 100) >= 50;
  pulso_100ms = clock_100ms && !clock_100ms_ant;

  clock_1s_ant = clock_1s;
  clock_1s = (millis() % 1000) >= 500;
  pulso_1s = clock_1s && !clock_1s_ant;
}

//********************************************************
// Bucle principal del programa
//********************************************************
void loop()
{
  unsigned long start_time = micros();

  timers();

  if (mySwitch.available())
  {

    int value = mySwitch.getReceivedValue();

    if (value == 0)
    {
      Serial.print("Unknown encoding");
    }
    else
    {
      Serial.print("Received ");
      Serial.print(mySwitch.getReceivedValue());
      Serial.print(" / ");
      Serial.print(mySwitch.getReceivedBitlength());
      Serial.print("bit ");
      Serial.print("Protocol: ");
      Serial.println(mySwitch.getReceivedProtocol());
    }

    mySwitch.resetAvailable();
  }

  loop_duration_acum += (unsigned int)(micros() - start_time);

  // Promedia las mediciones de tiempo del loop principal
  if (loop_duration_cnt++ >= 1000)
  {
    loop_duration_cnt = 0;
    loop_duration_acum = 0;
  }
}
