AEM PRND Pad CAN messages

idle:
  roller counter on byte 4 from 00h to FFh
  ex: 04 1B F9 XX 00 00 FF 21
  every .1 and 3.874 on rollover
Park:
  pressed:  04 1B 01 01 01 21 FF FF 
  released: 04 1B 01 01 00 21 FF FF 
Reverse:
  pressed:  04 1B 02 01 01 21 FF FF
  released: 04 1B 02 01 00 21 FF FF 
Neutral:
  pressed:  04 1B 03 01 01 21 FF FF
  released: 04 1B 03 01 00 21 FF FF 
Drive:
  pressed:  04 1B 04 01 01 21 FF FF
  released: 04 1B 04 01 00 21 FF FF 
Elect:
  pressed:  04 1B 05 01 01 21 FF FF
  released: 04 1B 05 01 00 21 FF FF 
slow/fast:
  pressed:  04 1B 06 01 01 21 FF FF
  released: 04 1B 06 01 00 21 FF FF 
Aux:
  pressed:  04 1B 07 01 01 21 FF FF
  released: 04 1B 07 01 00 21 FF FF 
Trac+:
  pressed:  04 1B 08 01 01 21 FF FF
  released: 04 1B 08 01 00 21 FF FF 
