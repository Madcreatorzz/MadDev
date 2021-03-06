/**
 * Author Teemu Mäntykallio
 * 
 * Plot TMC2130 motor load using the stallGuard value.
 * You can finetune the reading by changing the STALL_VALUE.
 * This will let you control at which load the value will read 0
 * and the stall flag will be triggered. This will also set pin DIAG1 high.
 * A higher STALL_VALUE will make the reading less sensitive and
 * a lower STALL_VALUE will make it more sensitive.
 * 
 * You can control the rotation speed with
 * 0 Stop
 * 1 Resume
 * + Speed up
 * - Slow down
 */
#define MAX_SPEED  40 // In timer value
#define MIN_SPEED  1000

#define STALL_VALUE 0 // [-64..63]

#define EN_PIN    4  // Nano v3:  16 Mega:  38  //enable (CFG6)
#define DIR_PIN   13//6  //      19      55  //direction
#define STEP_PIN  15//7  //     18      54  //step
#define CS_PIN    0 //      17      64  //chip select


#include <SPI.h>
#include <TMC2130Stepper.h>
#include <TMC2130Stepper_REGDEFS.h>
TMC2130Stepper driver = TMC2130Stepper(EN_PIN, DIR_PIN, STEP_PIN, CS_PIN,5,17,16);

bool state = true;
hw_timer_t * timer = NULL;

void IRAM_ATTR onTimer() {
digitalWrite(STEP_PIN, !state);
}

bool vsense;

uint16_t rms_current(uint8_t CS, float Rsense = 0.11) {
  return (float)(CS+1)/32.0 * (vsense?0.180:0.325)/(Rsense+0.02) / 1.41421 * 1000;
}

void setup() {
  //init serial port
  {
    Serial.begin(250000); //init serial port and set baudrate
    while(!Serial); //wait for serial port to connect (needed for Leonardo only)
    Serial.println("\nStart...");
    driver.begin();
  }

  //set TMC2130 config
  {
    driver.rms_current(600); // mA
    driver.microsteps(16);
    driver.diag1_stall(1);
    driver.diag1_active_high(1);
    driver.coolstep_min_speed(0xFFFFF); // 20bit max
    driver.THIGH(0);
    driver.semin(5);
    driver.semax(2);
    driver.sedn(0b01);
    driver.sg_stall_value(STALL_VALUE);
  }

  // Set stepper interrupt
  {
    cli();//stop interrupts
    TCCR1A = 0;// set entire TCCR1A register to 0
    TCCR1B = 0;// same for TCCR1B
    TCNT1  = 0;//initialize counter value to 0
    OCR1A = 256;// = (16*10^6) / (1*1024) - 1 (must be <65536)
    // turn on CTC mode
    TCCR1B |= (1 << WGM12);
    // Set CS11 bits for 8 prescaler
    TCCR1B |= (1 << CS11);// | (1 << CS10);  
    // enable timer compare interrupt
    TIMSK1 |= (1 << OCIE1A);
    sei();//allow interrupts
  }

  //TMC2130 outputs on (LOW active)
  digitalWrite(EN_PIN, LOW);

  vsense = driver.vsense();
}


void loop()
{
  static uint32_t last_time=0;
  uint32_t ms = millis();

  digitalWrite(STEP_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(STEP_PIN, LOW);
  delayMicroseconds(10);

  while(Serial.available() > 0) {
    int8_t read_byte = Serial.read();
    if (read_byte == '0')      { digitalWrite( EN_PIN, HIGH ); }
    else if (read_byte == '1') { digitalWrite( EN_PIN,  LOW ); }
 
  }
    
  if((ms-last_time) > 100) //run every 0.1s
  {
    last_time = ms;
    uint32_t drv_status = driver.DRV_STATUS();
    Serial.print("0 ");
    Serial.print((drv_status & SG_RESULT_bm)>>SG_RESULT_bp , DEC);
    Serial.print(" ");
    Serial.println(rms_current((drv_status & CS_ACTUAL_bm)>>CS_ACTUAL_bp), DEC);
  }
}

