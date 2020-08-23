#include <SPI.h>

// SPI default pins: CLK: 13, MOSI: 11 
// ADC
#define ADC_CS_PIN 3

// DAC, 
#define DAC_CS_PIN 2

// analog pin 2
#define POT_PIN  2 

#define adc0 0b00000110
#define adc1 0b00000000
#define adc2 0b00000000

byte outHi;
byte outLo;

int potValue = 0;

void setup()
{
  Serial.begin(9600);
  pinMode(DAC_CS_PIN, OUTPUT);
  pinMode(ADC_CS_PIN, OUTPUT);

  // default clock: 4MHz
  
  SPI.begin();
  SPI.setBitOrder(MSBFIRST);
  SPI.setDataMode(SPI_MODE0);
}


void loop()
{
  digitalWrite(ADC_CS_PIN, LOW);
  SPI.transfer(adc0);
  outHi = SPI.transfer(adc1);
  outLo = SPI.transfer(adc2);
  digitalWrite(ADC_CS_PIN, HIGH);

  unsigned int potValue = analogRead(POT_PIN);
  potValue = potValue << 4;
  

  // Combine MSB and LSB, and shift 4 to mutiply by 32 / getting 16 bit for output
  outHi = (outHi << 4) | (outLo >> 4);
  outLo = outLo << 4;
  
  // write to dac:
  digitalWrite(DAC_CS_PIN, LOW); 
  SPI.transfer(outHi);
  SPI.transfer(outLo);  
  digitalWrite(DAC_CS_PIN, HIGH);
  
  delayMicroseconds(potValue);


}
 

