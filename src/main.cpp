#include <Arduino.h>
#include "config.h"

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
  struct {
    int overload             : 4;
    bool underload           : 4;
    bool adc_mask_changed    : 1;
  };
  unsigned int word;
} MPX_packet;

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
  pinMode(RX_PIN, INPUT);
  digitalWrite(TX_PIN, 0);
  pinMode(TX_PIN, OUTPUT);
}

// ***********************************************************
// Llamada a las funciones de inicialización
// ***********************************************************
void setup()
{
  Serial.begin(38400);
  Serial.println("Setup GPIOs");
  setup_GPIOs();
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
// Detector de flancos
//
// Detecta si es ascendente o descendente
// mide el tiempo transcurrido desde el último cambio
//********************************************************



//********************************************************
// Bucle principal del programa
//********************************************************
void loop()
{
  unsigned long start_time = micros();

  timers();

  loop_duration_acum += (unsigned int)(micros() - start_time);

  // Promedia las mediciones de tiempo del loop principal
  if (loop_duration_cnt++ >= 1000)
  {
    loop_duration_cnt = 0;
    loop_duration_acum = 0;
  }
}
