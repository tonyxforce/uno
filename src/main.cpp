#include <Arduino.h>

#include "DFRobotDFPlayerMini.h"

#define TURNON 1
#define PAIR 2
#define CONNECT 3
#define TURNOFF 4
#define DISCONNECT 5
#define MAXVOL 6

#include "SoftwareSerial.h"

static const uint8_t outSerialTx = D1;
static const uint8_t outSerialRx = D2;
SoftwareSerial outSerial(outSerialRx, outSerialTx);

static const uint8_t inSerialTx = D4;
static const uint8_t inSerialRx = D5;
SoftwareSerial inSerial(inSerialRx, inSerialTx);

static const uint8_t PIN_MP3_TX = D7;
static const uint8_t PIN_MP3_RX = D6;
SoftwareSerial dfpSerial(PIN_MP3_RX, PIN_MP3_TX);

DFRobotDFPlayerMini player;
#define myDFPlayer player

const int numArrays = 15;
const int numElements = 14;

String generateArrayString(int index)
{
  String arrayString = "{\"seg\":{\"0\":{\"col\":[";

  // Generate the elements and concatenate to arrayString
  for (int j = 0; j < index; j++)
  {
    arrayString += "\"FF4d00\",";
  }

  // Add the last element
  arrayString += "\"FF0084\",";

  for (int j = 0; j < numArrays - (index + 1); j++)
  {
    arrayString += "\"000000\",";
  }
  arrayString += "000000";

  // Concatenate the suffix
  arrayString += "}]}}";

  return arrayString;
}

void generateAndPrintArrays()
{
  for (int i = 0; i < numArrays; i++)
  {
    String arrayString = generateArrayString(i);
    outSerial.println(arrayString);
    delay(200);
  }
}

void dfPlay(int id)
{
  myDFPlayer.enableDAC();
  delay(100);
  player.play(id);
  delay(100);

  while (digitalRead(D0) == LOW)
  {
    yield();
  }
  delay(100);

  myDFPlayer.disableDAC();
}

void setup()
{
  pinMode(D0, INPUT_PULLUP);

  Serial.begin(9600);
  delay(500);
  Serial.println("\r\n\r\nhwserial ok");
  delay(500);

  inSerial.begin(9600);
  Serial.println("Serial Txd is on pin: " + String(inSerialTx));
  Serial.println("Serial Rxd is on pin: " + String(inSerialRx));

  outSerial.begin(115200);
  Serial.println("Serial2 Txd is on pin: " + String(outSerialTx));
  Serial.println("Serial2 Rxd is on pin: " + String(outSerialRx));

  dfpSerial.begin(9600);

  inSerial.println("inSerial");
  outSerial.println("outSerial");
  Serial.println("Serial");
  dfpSerial.println("dfpSerial");

  delay(3000);
  if (player.begin(dfpSerial))
  {
    player.volume(20); // 30 is very loud
    player.EQ(DFPLAYER_EQ_BASS);

    myDFPlayer.enableDAC();
    delay(100);
    player.play(TURNON);
    delay(100);
    /*     for(int i = 0;i<15;i++){
          outSerial.println(on[i]);
          delay(100);
        } */
    generateAndPrintArrays();
    while (!digitalRead(D0))
    {
      yield();
    }
    myDFPlayer.disableDAC();

    Serial.println("dfplayer OK");
  }
  else
  {
    Serial.println("Connecting to DFPlayer Mini failed!");
  }
}

String inString = "";
int volume = 0;

bool beforeFirstConnect = true;
bool isConnected = false;

void loop()
{
  if (Serial.available())
  {
    delay(100);
    while (Serial.available())
    {
      outSerial.println(Serial.readStringUntil('\n'));
    }
  }
  if (outSerial.available())
  {
    delay(100);
    while (outSerial.available())
    {
      Serial.println(outSerial.readStringUntil('\n'));
    }
  }
  if (inSerial.available() > 0) // //TODO replace serial name
  {
    delay(100);
    while (inSerial.available()) // //TODO replace serial name
    {
      char readChar = inSerial.read(); // //TODO replace serial name
      if (readChar == '\n')
      {
        Serial.println(inString);

        switch (inString[0])
        {
        case '1': // df play
          Serial.println("Playing...");
          dfPlay(inString[1] - '0');
          Serial.println("Done playing!");
          break;

        case '2':                     // echo
          inSerial.println(inString); // //TODO replace serial name
          break;

        case '3':
          inSerial.println("Pong!"); // //TODO replace serial name
          break;

        case '4': // connecting...
          if (!isConnected && beforeFirstConnect)
          {
            beforeFirstConnect = false;
            isConnected = true;
            dfPlay(CONNECT);
          }
          break;

        case '5': // ble data
          Serial.println(inString);
          break;

        case '6': // volume
          inString.remove(0, 1);
          volume = inString.toInt();
          Serial.print("volume changed to ");
          Serial.println(volume);
          if (volume == 127)
          {
            dfPlay(MAXVOL);
          }
          break;

        case '7': // disconnecting...
          if (isConnected)
          {
            isConnected = false;
            dfPlay(DISCONNECT);
          }
          break;

        case '8': // connected!
          if (!isConnected)
          {
            isConnected = true;
            dfPlay(CONNECT);
          }
          break;
        case '9': // disconnected!
          if (isConnected)
          {
            isConnected = false;
            dfPlay(DISCONNECT);
          }
          break;
        case 'a': // turnoff
          dfPlay(DISCONNECT);
          inSerial.println("ok");
        }
        inString = "";
        while (inSerial.available() > 0)
        {
          inSerial.read();
        }
      }
      else
      {
        inString += readChar;
      }
      // Serial.print("in: '");
      // Serial.print(inString);
      // Serial.print("', readChar: '");
      // if (readChar == '\n')
      //   Serial.print("\\n");
      // else
      //   Serial.print(readChar);
      // Serial.println("'");
      // delay(500);
    }
  }
  // player.play(1);
}