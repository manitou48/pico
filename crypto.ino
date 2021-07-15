// crypto board (I2C) ECC, SHA, AES, TRNG
#include <SparkFun_ATECCX08a_Arduino_Library.h> //Click here to get the library: http://librarymanager/All#SparkFun_ATECCX08a
#include <Wire.h>

ATECCX08A atecc;
uint8_t myHash[32];
uint8_t tmpkey[32], clear[16], cipher[16], aesbuf[16];

// aes key can be in EEPROM key slots, embedded in data, or TempKey  16-byte block
// decrypt 1  encrypt 0
boolean doaes(uint8_t *in, uint8_t *out, int decrypt) {
  uint8_t param1;
  uint16_t param2;

  param1 = decrypt;
  param2 = 0xffff;   // key from TempKey
  if (!atecc.sendCommand(0x51, param1, param2, in, 16))return false;
  if (!atecc.receiveResponseData(RESPONSE_COUNT_SIZE + 16 + CRC_SIZE))
    return false;

  atecc.idleMode();

  if (!atecc.checkCount() || !atecc.checkCrc())
    return false;
  memcpy(out, atecc.inputBuffer + 3, 16);
  for (int i = 0; i < 24; i++) {
    Serial.print(atecc.inputBuffer[i], HEX);
    Serial.print(" ");
  }
  Serial.println();
}

void setup() {
  Wire.begin();
  Serial.begin(115200);
  while (!Serial);
  if (atecc.begin() == true)
  {
    Serial.println("Successful wakeUp(). I2C connections are good.");
  }
  else
  {
    Serial.println("Device not found. Check wiring.");
    while (1); // stall out forever
  }
  uint8_t buff[1024];
  uint32_t us = micros();
  atecc.sha256(buff, sizeof(buff), myHash);
  us = micros() - us;
  sprintf((char *)buff, "SHA256 %d us %f mbs\n", us, 8.*sizeof(buff) / us);
  Serial.print((char *)buff);

  for (int i = 0; i < 5; i++) {
    us = micros();
    atecc.updateRandom32Bytes();  //  in atecc.random32Bytes[]
    us = micros() - us;
    sprintf((char *)buff, "TRNG %d us %f mbs\n", us, 8.*32 / us);
    Serial.print((char *)buff);
    delay(1000);
  }

  for (int i = 0; i < sizeof(tmpkey); i++) tmpkey[i] = 100 + i;
  atecc.loadTempKey(tmpkey);
  for (int i = 0; i < sizeof(clear); i++) clear[i] = i;
  doaes(clear, cipher, 0);
  doaes(cipher, aesbuf, 1);
  Serial.print("aesbuf[3] ");
  Serial.println(aesbuf[3]);
}

void loop() {
  // put your main code here, to run repeatedly:

}
