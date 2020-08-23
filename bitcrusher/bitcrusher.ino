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
  //stop interrupts
  cli();

  //set timer1 interrupt at 1Hz
  TCCR1A = 0;// set entire TCCR1A register to 0
  TCCR1B = 0;// same for TCCR1B
  TCNT1  = 0;//initialize counter value to 0

  // set compare match register, should be between 1 and 1024.
  OCR1A = 1;
  
  // turn on CTC mode
  TCCR1B |= (1 << WGM12);
  
  // Set CS12 bit for 256 prescaler
  TCCR1B |= (1 << CS11);
  
  // enable timer compare interrupt
  TIMSK1 |= (1 << OCIE1A);
  
  Serial.begin(9600);
  pinMode(DAC_CS_PIN, OUTPUT);
  pinMode(ADC_CS_PIN, OUTPUT);

  // default clock: 4MHz
  
  SPI.begin();
  SPI.setBitOrder(MSBFIRST);
  SPI.setDataMode(SPI_MODE0);

  //allow interrupts
  sei();
}


//timer1 interrupt
ISR(TIMER1_COMPA_vect){
  digitalWrite(ADC_CS_PIN, LOW);
  SPI.transfer(adc0);
  outHi = SPI.transfer(adc1);
  outLo = SPI.transfer(adc2);
  digitalWrite(ADC_CS_PIN, HIGH);

  // Combine MSB and LSB, and shift 4 to mutiply by 32 / getting 16 bit for output
  outHi = (outHi << 4) | (outLo >> 4);
  outLo = outLo << 4;
  
  // write to dac:
  digitalWrite(DAC_CS_PIN, LOW); 
  SPI.transfer(outHi);
  SPI.transfer(outLo);  
  digitalWrite(DAC_CS_PIN, HIGH);
}


void loop()
{
  unsigned int potValue = analogRead(POT_PIN);
  
  // set compare match register, should be between 1 and 1024.
  OCR1A = potValue * 4 + 1;
  delay(1);
}
 

