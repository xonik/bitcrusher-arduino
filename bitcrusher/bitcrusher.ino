/**
 * Bitcrusher PoC - does sample rate reduction and bit reduction. 
 * 
 * Voltage controllable through analog pin 2 and 3 (0-5v)
 * 
 * Pins used:
 * 13: SPI clock
 * 12: SPI MISO
 * 11: SPI MOSI
 *  3: ADC Chip Select (CS)
 *  2: DAC Chip Select (CS)
 * A3: Sample rate CV 
 * A2. Bit resolution CV
 * 
 * ADC: MCP3208
 * DAC: DAC8830
 * 
 * NB: Unipolar, signal must be 0-5v
 */

#include <SPI.h>

// SPI default pins: CLK: 13, MOSI: 11 
// Chip Select pins:
#define ADC_CS_PIN 3
#define DAC_CS_PIN 2

// Analog pins:
#define SAMPLE_RATE_POT_PIN  2 
#define BITS_POT_PIN 3

#define adc0 0b00000110
#define adc1 0b00000000
#define adc2 0b00000000

int bitsToRemove = 0;
int timerCompareValue = 1;

void setup()
{
  // stop interrupts
  cli();

  // set timer1 interrupt
  // Clear control registers
  TCCR1A = 0;
  TCCR1B = 0;
  
  // initialize counter value to 0
  TCNT1  = 0;

  // set compare match register (target value for timer), should be between 1 and 1024.
  OCR1A = 1;
  
  // turn on CTC mode
  TCCR1B |= (1 << WGM12);
  
  // Set CS12 bit for 1/8 prescaler
  TCCR1B |= (1 << CS11);
  
  // Enable timer compare interrupt
  TIMSK1 |= (1 << OCIE1A);
  
  Serial.begin(9600);
  pinMode(DAC_CS_PIN, OUTPUT);
  pinMode(ADC_CS_PIN, OUTPUT);

  // Default SPI clock: 4MHz  
  SPI.begin();
  SPI.setBitOrder(MSBFIRST);
  SPI.setDataMode(SPI_MODE0);

  // Enable interrupts
  sei();
}

// Timer1 interrupt handler
ISR(TIMER1_COMPA_vect){
    
  digitalWrite(ADC_CS_PIN, LOW);
  SPI.transfer(adc0);
  byte outHi = SPI.transfer(adc1);
  byte outLo = SPI.transfer(adc2);
  digitalWrite(ADC_CS_PIN, HIGH);


  // Concatenate to 12 bit value (ADC is 12 bit)
  unsigned int value = (((outHi & 0b00001111) << 8) | outLo) << 4;
  
  // Remove bits for bit reduction (bitcrushing)
  value = (value >> bitsToRemove) << bitsToRemove;

  // set new interval. Not sure if necessary to do this inside interrupt or if it could be done in loop, but at least it
  // should be safe to do it here.
  OCR1A = timerCompareValue;
  
  // write to dac:
  digitalWrite(DAC_CS_PIN, LOW); 
  SPI.transfer(highByte(value));
  SPI.transfer(lowByte(value));  
  digitalWrite(DAC_CS_PIN, HIGH);
}

void loop()
{
  unsigned int bitsPotValue = analogRead(BITS_POT_PIN);
  byte bits = (bitsPotValue >> 6) + 1;
  bitsToRemove = 16 - bits;

  // leave at least one bit...
  if(bitsToRemove == 16) {
    bitsToRemove = 15
  }
  
  // set compare match register to change sample rate
  unsigned int sampleRatePotValue = analogRead(SAMPLE_RATE_POT_PIN);  
  if(sampleRatePotValue > 18){
    // MCU locks up if this gets too low. Probably because it never leaves the interrupt handler
    timerCompareValue = sampleRatePotValue * 4;
  }
  delay(1);
}
 

