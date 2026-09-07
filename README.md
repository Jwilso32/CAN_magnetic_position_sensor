AEM PRND Pad CAN messages<br>
birate = 500k @ 75.0% Sample point <br>
<p></p>
idle:<br>
  roller counter on byte 4 from 00h to FFh
  ex: 04 1B F9 XX 00 00 FF 21
  every .1 and 3.874 on rollover
Park:<br>
  pressed:  04 1B 01 01 01 21 FF FF <br>
  released: 04 1B 01 01 00 21 FF FF <br>
Reverse:<br>
  pressed:  04 1B 02 01 01 21 FF FF<br>
  released: 04 1B 02 01 00 21 FF FF <br>
Neutral:<br>
  pressed:  04 1B 03 01 01 21 FF FF<br>
  released: 04 1B 03 01 00 21 FF FF <br>
Drive:<br>
  pressed:  04 1B 04 01 01 21 FF FF<br>
  released: 04 1B 04 01 00 21 FF FF <br>
Elect:<br>
  pressed:  04 1B 05 01 01 21 FF FF<br>
  released: 04 1B 05 01 00 21 FF FF <br>
slow/fast:<br>
  pressed:  04 1B 06 01 01 21 FF FF<br>
  released: 04 1B 06 01 00 21 FF FF <br>
Aux:<br>
  pressed:  04 1B 07 01 01 21 FF FF<br>
  released: 04 1B 07 01 00 21 FF FF <br>
Trac+:<br>
  pressed:  04 1B 08 01 01 21 FF FF<br>
  released: 04 1B 08 01 00 21 FF FF <br>
