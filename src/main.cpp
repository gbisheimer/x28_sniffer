#include <Arduino.h>
#include "config.h"

#define BIT_TIME (6000 / 3)
#define OUTPIN 3

// tiempos de IDLE y ZERO en microsegundos
// zero basicamente ajusta la velocidad de transmision. el cero deberia durar 1250uS segun lo medido
// con un osciloscopio, y un 1 deberia durar 2500uS. asi que 2000 parece funcionar ok
#define IDLE (5000)
#define ZERO (2000)

void tx(uint16_t payload);
void isr();

uint16_t KEYBOARD_CODES[] = {
    0x0000, // 0
    0x8013,
    0x8025,
    0x0036,
    0x8046,
    0x0055, // ...
    0x0063,
    0x8070,
    0x8089,
    0x009A, // 9
    0x00AC, // P
    0x80BF, // F
    0x00CF, // Z (ver teclas.md)
    0x80DC, // M
    0x80EA, // Campana
    0x00F9, // Incendio
    0x00AC,
    0x810A,
    0x80BF,
    0x813C,
    0x00F9,
    0x0119,
    0x80EA,
    0x012F,
    0x8169};
typedef union
{
  struct
  {
    unsigned parity : 1;
    unsigned id : 3;
    unsigned data : 8;
    unsigned checksum : 4;
  };
  uint16_t word;
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
  Serial.println("Setup GPIOs");
  digitalWrite(LED_BUILTIN, 0);
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(OUTPIN, OUTPUT);
}

void setup_RCSwitch()
{
  Serial.println("Setup RCSwitch: RX on pin 2. TX on pin 13");
  attachInterrupt(digitalPinToInterrupt(2), isr, CHANGE);
}

// ***********************************************************
// Llamada a las funciones de inicialización
// ***********************************************************
void setup()
{
  Serial.begin(115200);
  setup_GPIOs();
  setup_RCSwitch();
}

char isKeyboardCode(uint16_t code)
{

  for (size_t i = 0; i < sizeof(KEYBOARD_CODES) / 2; i++)
  {
    if (KEYBOARD_CODES[i] == code)
      return true;
  }
  return false;
}

volatile char available = 0;

uint16_t recbuf = 0;
char curbit = 0;

//********************************************************
// Bucle principal del programa
//********************************************************

char debugstr[100] = "";
void loop()
{
  static unsigned long lastmsg = 0;
  static char sent = 0;

  digitalWrite(LED_BUILTIN, !digitalRead(2));
  if (!sent)
  {
    Serial.println("Sending");

    tx((unsigned int)0xc92b); // castear el arg a unsigned int
    // delay para simular dedos sobre un teclado?
    delay(500);

    sent = 1;
    Serial.println("Sent");
  }

  if (available)
  {
    if (millis() - lastmsg > 500)
      Serial.println("-----------------");

    lastmsg = millis();
    MPX_packet packet;
    packet.word = recbuf;
    available = 0;
    char direction[2] = "<";

    if (isKeyboardCode(packet.word))
      direction[0] = '>';

    sprintf(debugstr, "[%010lu]: %s 0x%04X - %1x %2x %3x %2x\n", millis(), direction, packet.word, packet.parity, packet.id, packet.data, packet.checksum);

    Serial.print(debugstr);
  }
}

void isr()
{
  static unsigned long prev_micros = 0;
  unsigned long curr_micros = micros();
  unsigned long length = curr_micros - prev_micros;

  if (!available && digitalRead(2) == 0)
  {
    if (length > IDLE)
    {
      recbuf = 0;
      curbit = 0;
    }
    else
    {
      recbuf = recbuf << 1;
      if (length > ZERO)
        recbuf |= 1;

      if (++curbit == 16)
        available = 1;
    }
  }

  prev_micros = curr_micros;
}

// rutina TX bitbanging, bloqueante
void tx(uint16_t payload)
{
  // esperar a que la linea este libre por al menos 5 bits
  int ncts = IDLE * 5;
  while (ncts)
  {
    if (digitalRead(2) == 0)
      ncts = IDLE * 5;
    else
      ncts--;
  }

  // start bit
  digitalWrite(OUTPIN, 1);
  delayMicroseconds(BIT_TIME);

  for (int i = 0; i < 16; i++)
  {
    digitalWrite(OUTPIN, 0);
    if (!(payload & (unsigned int)0x8000))
    {
      delayMicroseconds(BIT_TIME);
      digitalWrite(OUTPIN, 1);
      delayMicroseconds(2 * BIT_TIME);
    }
    else
    {
      delayMicroseconds(2 * BIT_TIME);
      digitalWrite(OUTPIN, 1);
      delayMicroseconds(BIT_TIME);
    }
    payload = payload << 1;
  }
  digitalWrite(OUTPIN, 0);
}
