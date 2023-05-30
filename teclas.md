teclas simples, modo normal:
codigo tecla

0000 0
8013 1
8025 2
0036 3
8046 4
0055 5
0063 6
8070 7
8089 8
009A 9
00AC P
80BF F
00CF ver tecla Z
80DC M
80EA 🔔
00F9 🔥

press largo:
P: 00AC 810A
F: 80BF 813C
🔥: 00F9 0119
🔔: 80EA 012F

TECLA Z:
00CF 0000: entrar en modo inclusion
00CF 8169: salir modo inclusion

---

respuestas alarma:
enviar 99 (armar)
6c28
49c1 -> cambia a "activada"

enviar codigo desact:
6819
c92b -> cambia a "desactivada"

estoy -> me voy: (z1 puerta abierta)
cbae c095 41a9 c91d

0xcbae -> cambia led a "me voy"

[0000452297]: > 0x80DC - 0 6 d 8
[0000452384]: < 0xCBAE - 0 7 ba c
[0000452471]: < 0xC095 - 1 2 9 c
[0000452558]: < 0x41A9 - 1 4 1a 4
[0000452644]: < 0xC91D - 1 6 91 c

0000048421]: > 0x80DC - 0 6 d 8
[0000048507]: < 0xCBAE - 0 7 ba c
[0000048594]: < 0xC095 - 1 2 9 c
[0000048681]: < 0x41A9 - 1 4 1a 4

me voy -> estoy: (z1 puerta abierta)
4be8 402a c18c 490e
0x4be8 -> cambia led a "Estoy"

[0000484677]: > 0x80DC - 0 6 d 8
[0000484763]: < 0x4BE8 - 0 4 be 4
[0000484849]: < 0x402A - 0 5 2 4
[0000484936]: < 0xC18C - 0 6 18 c
[0000485022]: < 0x490E - 0 7 90 4

abrir puerta con sensor inalambrico, z1:
[0000071727]: < 0xCC03 - 1 1 c0 c
[0000072222]: < 0x1212 - 0 1 21 1 -> parece ser apertura
[0000072309]: < 0xB08A - 0 5 8 b
[0000072396]: < 0xC91D - 1 6 91 c
[0000072728]: < 0xAC44 - 0 2 c4 a

cerrar puertacon seonsor inalambrico, z1:
[0000115115]: < 0x9419 - 1 4 41 9 -> parece ser cierre
[0000115202]: < 0x3003 - 1 1 0 3
[0000115288]: < 0xC92B - 1 5 92 c

comportamiento de apertura, z1 (remoto 315mhz)
la alarma parece tener un contador de aperturas, como todos los sensores envian:
0x1212 para indicar apertura, si se envia 2 veces, la alarma espera que se envie
0x9419 para indicar cierre

si envio 2 veces 0x1212, pero una sola 0x9419, la alarma sigue considerando
la zona 1 como "abierta".

pero si mando mas veces 0x9419, la puerta queda cerrada (se ve que el codigo de la
alarma verifica los cierres y no hace underflow)

con alarma activada:

apertura puerta:
[0000232433]: < 0x1212 - 0 1 21 1
[0000232520]: < 0xB08A - 0 5 8 b
[0000232606]: < 0x490E - 0 7 90 4

[0000359743]: < 0x1212 - 0 1 21 1
[0000359830]: < 0xE989 - 1 4 98 e
[0000359915]: < 0xB08A - 0 5 8 b -> hace prender led z1
[0000360003]: < 0xB881 - 1 0 88 b -> hace parpadear led z1 bajo/alto
[0000360089]: < 0xCBFB - 1 5 bf c
[0000360174]: < 0x4C8A - 0 5 c8 4
[0000360625]: < 0x49C1 - 1 0 9c 4

periodicamente:
ac44 -> todo el tiempo, no se que es

0x3808 -> hizo apagar leds zonas, parece indicar estado zonas
