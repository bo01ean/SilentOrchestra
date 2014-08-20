// HARD PINS
const unsigned int DRIVEPORT = 10;
const unsigned int BUTTONPORT = 5;
const byte HALLPIN = 0; /// SHOULD BE DIGITAL 2  (0) or Digital 3 (1)

const unsigned int MAX_RPS = 1800 / 60;
const unsigned int MAX_POWER = 4;//

// MOTOR STUFF
float power = 66.0;
float voltage = 0.0;
boolean ascending = true;
byte ACCELFACTOR = 1; // /10

byte highest = 33;
byte lowest = 0;

// OPERATIONAL CONSTRAINTS
const unsigned int TARGETRPS = 300 / 60;
unsigned int maxPower = 100;
float maxVoltage = 2.5;
     
// RPM Stuff
volatile byte revcount = 0;
short tmprevcount = 0;

float accumulator = 0.0;// 16 sample average
unsigned short lastmillis = 0;

float r = 0.00000;

void setup()
{
  Serial.begin(9600);
  setupHallSensor();
  accelerate();
}

void setupHallSensor()
{
  EIFR = bit (INTF0);  // clear flag for interrupt 0
  Serial.println("Hall Setup....");  
  delay(1000);
  attachInterrupt(HALLPIN, interruptHandler, RISING);
}

void interruptHandler()
{ 
  revcount++;
}

void accelerate()
{
  ascending = true;
  signalMotor();
}

void decelerate()
{
  ascending = false;
  signalMotor();  
}

void signalMotor()
{
  short direction = (ascending) ? 1 : -1;
  power += (direction * (ACCELFACTOR / 10));
  checkRange();
  analogWrite(DRIVEPORT, (int) power);
}


void checkRange()
{
  //power = map(power, 0, 1023, 0, 255);
  power = ( power > maxPower) ? maxPower : power;
  power = (power < 0 ) ? 0 : power;
}

void loop(){

  failsafes();

  if (millis() - lastmillis == 1000) { //Update every one second, this will be equal to reading frequency (Hz).
    detachInterrupt(HALLPIN);//Disable interrupt when calculating
    tmprevcount = revcount;

    if(tmprevcount >= highest) {
      tmprevcount = r;
    }
    //r = (a>>4);	// output result is 1/16th of accumulator
    r = accumulator/16;//get average
    //a -= r;   // subtract l/16th of the accumulator
    accumulator -= r;
    //a += n;     // add in the new sample
    accumulator += tmprevcount;

    if ((tmprevcount) < TARGETRPS) {  
      if (voltage < maxVoltage) {
        accelerate();
      }
    }
    printStatus();
    lastmillis = millis(); // Update lastmillis
    revcount = 0; // Restart the RPM counter
    attachInterrupt(HALLPIN, interruptHandler, FALLING); //enable interrupt
  }     
}





























float powerToVolts(float p)
{
  return (p/255) * 5;
}


void printDouble( double val, unsigned int precision){
// prints val with number of decimal places determine by precision
// NOTE: precision is 1 followed by the number of zeros for the desired number of decimial places
// example: printDouble( 3.1415, 100); // prints 3.14 (two decimal places)
    Serial.print (int(val));  //prints the int part
    Serial.print("."); // print the decimal point
    unsigned int frac;
    if(val >= 0)
        frac = (val - int(val)) * precision;
    else
        frac = (int(val)- val ) * precision;
    Serial.print(frac,DEC) ;
} 


void failsafes() {
  if (tmprevcount > MAX_RPS) {
    error();  
  }

  if (voltage > MAX_POWER) {
    error();  
  }

}

void error() {
  ACCELFACTOR = 0;
  ascending = false;
  analogWrite(DRIVEPORT, 0); 
  Serial.println("ERROR"); 
}

void printStatus()
{
    voltage = powerToVolts(power);   
  
    Serial.print(" Power: ");    
    Serial.print(power);

    Serial.print(" Voltage: ");    
    Serial.print(voltage);

    Serial.print(" Hz(tmprevcount): ");    
    Serial.print(tmprevcount);

    //Serial.print(" Accumulator: ");    
    //printDouble(accumulator, 100);

    //Serial.print(" RPS: ");
    //printDouble(tmprevcount, 100);
 
    //Serial.print(" RPM: ");
    //printDouble((accumulator/16) * 60, 100);
    Serial.println(""); 
}
