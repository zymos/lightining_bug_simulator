//////////////////////////////////////////////////////////////////////////////
//
// Lightning bugs emulator
//
//    Desciption: Blinks 17 LEDs to simulate a bunch of lightning bugs blinking. 
//      Spread the LEDs out in your garden or anywhere for a bunch of sparse 
//      random blinks.
//
//    Hardware:
//      Arduino, I used Arduino Pro Mini
//          Should work with most or any AVR with minor changes
//      240-330 Ohm resistors for 5V
//      120-180 Ohm Resistors for 3.3V
//      common green LEDs
//
//    Author: ZyMOS 4/2020 - 2022
//    
//    License: GPLv3
//
///////////////////////////////////////////////////////////////////////////////


//Include libraries
#include <Arduino.h>
#include <avr/wdt.h> // Watch-dog timer
// #include <MemoryFree.h> // check for memory leaks




//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
////  Configure
////

// to debug or not to debug (turns on serial)
#define debug 0

// Run LED test sweep
#define led_test 0
#define inf_led_test 0

// Frequency
#ifdef F_CPU
  #undef F_CPU
#endif
#define F_CPU 8000000UL // (UL unsigned long) Hz
#define frequency 8000000 // Hz

// interval between LED operation
#define counter_timer_div 10 //miliseconds

// pins used for LEDs
const uint8_t pin_address[] = {
  2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19
  // 2,3
};


// LED on/off times 
//   random generated between min and max, with mean 60% of the time
//   in 1/10 of a sec (ie 10=1s)
#define min_on_time 2 // in 1/10s
#define max_on_time 7 // 1/10s
#define mean_on_time 3 // 1/10s

#define min_off_time 50 // 1/10s
#define max_off_time 500 // 1/10s
#define mean_off_time 300 // 1/10s

// probablility of LED going off [NOT SURE IF THIS IS REALLY NEEDED]
#define led_probability 20 // (led_probability)/1000 chance, 
// (this will occur every counter_timer_div so keep low)


// Max LEDs allowed to be on
#define max_leds_on 3


// Inverted LED (led normally on) 
//   1 = LEDs common Vcc
//   0 = LEDs common GND
#define led_is_inverted 1






////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
//// Code
////





/////////////////////////////////////////////////
// Global variables
//

const uint8_t max_leds = sizeof(pin_address) / sizeof(pin_address[0]);

// Various counters
uint16_t time_counter=60;

// const uint8_t max_leds = 17;
uint16_t led_on_count[max_leds];
uint16_t led_off_count[max_leds];
uint8_t led_available[max_leds];
uint8_t led_on[max_leds];

// timer1 division, start time for timer1
const int timer_div=65635-frequency/counter_timer_div/256;

// delay within loop()
const uint8_t loop_delay=3;

// Soft reset counter
uint32_t softreset_count = 0;
//  6h * 3200s/m * 1000ms/s / loop_delay
//  30m * 60 s/m * 1000 ms/s / loop_delay
const uint32_t softreset_count_max = 600000;// //6 * 3200 * 1000 / loop_delay;





///////////////////////////////////////////////////
// Functions
//

// // Creates a 100ms counter
// //    Uses timer interrupt
ISR(TIMER1_OVF_vect){  // Reset timer, add to count
  // TCNT1 = 59286;    //reset timer 
  TCNT1 = timer_div;

  time_counter++;   // increments every 100ms?10ms

  // Reset all, so it wont get stuck at max
  // uint8_t x=0;
  // if(time_counter > 65000){
  //   time_counter = 0;
  //   for(x=0;x<max_leds;x++){
  //     led_on_count[x] = 0;
  //     led_off_count[x] = 0;
  //     led_available[x] = 1;
  //     led_on[x] = 0;
  //   }
  // }
}






long weighted_random_probability(long min, long max, long mean){
  // shifts probability to be more likly to be mean 
  //   is in 3-piece piecewise equation

  //  Serial.println(weighted_random_probability(100,700,300));

  // random number 1 to 1000
  long random_num = random(1,1000);

  // change to floats so fraction arnt rounded
  float meanf = (float)mean;
  float minf = (float)min;
  float maxf = (float)max;

  // Serial.print(random_num);
  // Serial.print(", ");


  if( random_num < 400){ // 0-30% -> min to mean
    return (long)((meanf-minf)/(300)*(random_num) + minf);
  }else if (random_num >600)  { // 60-100% -> mean to max
    return (long)((maxf-meanf)/(1000-600)*(random_num - 600) + meanf);
  }else{ // 30-60% -> mean
    return (long)mean;
  }
}





// // Turns on/off LED
void set_led(uint8_t led_number, uint8_t led_status){
  uint8_t pin_num;

  // dont use pin 0-1 they are TXD, RXD
  //    Ard0/PD0 = RX
  //    Ard1/PD1 = TX

   led_number += 2;

  //pin_num = pin_address[led_number];

   pin_num = led_number;

  // turn led on/off, invert if set
  if(led_status == 1){
    if(led_is_inverted == 1){
      digitalWrite(pin_num,LOW);
    }else{
      digitalWrite(pin_num,HIGH);
    }
  }else{
    if(led_is_inverted == 1){
      digitalWrite(pin_num,HIGH);
    }else{
      digitalWrite(pin_num,LOW);
    }
  }

  // Reset the counter, if it doesn't get reset after a time, MCU gets soft reset
  softreset_count = 0;
}




// // Blinks on/off the leds for each led
void bleep(uint8_t led_number){

  uint8_t total_leds_on=0;

  // if (time_counter  > (65535 - max_off_time)){ //FIXME, check this
  //   // Time counter will reset soon, 16-bit
  //   // force resetting leds
  //     set_led(led_number,0); // makes sure its off
  //     led_on[led_number] = 0;    
  //     led_available[led_number] = 1; // LED is available again

  // }else 
  // Serial.print(led_off_count[led_number]);
  //if(debug){Serial.print(", [");}
  if(time_counter <= led_off_count[led_number] && led_available[led_number] == 1){
    //if(debug){Serial.print("!");}
    // Serial.print(led_off_count[led_number] );
    // Serial.print("}");
  }else if (led_available[led_number] == 1){ // LED is not set or waiting

 // Serial.print("[");
 // Serial.print(random(1,100));
 // Serial.print("]");
    // LED must not previously been on for
    // Serial.print('-');
        // if(random(1,100) <= led_probability){ 
        //   Serial.print('*');
        // }
    if(random(1,1000) <= led_probability){ // LED (chance every delay)


      // Check how many LEDs are on           FIXME
      for(uint8_t i=0; i < max_leds; i++){
        if(led_on[i]){
          total_leds_on++;
        }
      }
      // for(uint8_t i=0; i < max_leds; i++){ 
      //   if(digitalRead(pin_address[i]) && !led_is_inverted){
      //     total_leds_on++;
      //   }else if (!digitalRead(pin_address[i]) && led_is_inverted){
      //     total_leds_on++;
      //   }
      // }
   //   total_leds_on = 1;

      // prevent too many leds on
      if( total_leds_on < max_leds_on ){ 
        set_led(led_number,1);
        // led_on_count[led_number] = random(min_on_time,max_on_time) + time_counter; // Time LED stays on
        // led_off_count[led_number] = random(min_off_time,max_off_time) + led_on_count[led_number]; // time LED 
        led_on_count[led_number] = weighted_random_probability(min_on_time, max_on_time, mean_on_time) + time_counter; // Time LED stays on
        led_off_count[led_number] = weighted_random_probability(min_off_time,max_off_time, mean_off_time) + led_on_count[led_number]; // time LED remain off after its on
        led_available[led_number] = 0; // its on so not available
        led_on[led_number] = 1;
        //if(debug){Serial.print("+");}
      }//else{
      //  if(debug){Serial.print("x");}
      //}
    }//else{
    //  if(debug){Serial.print("-");}
    //} 
  }else{ // LED is already in process
    if(time_counter >= led_on_count[led_number]){ // LED on time is over
      set_led(led_number,0);
      led_on[led_number] = 0;
      led_available[led_number] = 1; // LED is available again
      //if(debug){Serial.print("^");}
    }else if(time_counter >= led_off_count[led_number]){ 
      //if(debug){Serial.print("~");}
      // Off time is over
      // or counter has/will reset  (time_counter + max_off_time) < time_counter 
      set_led(led_number,0); // makes sure its off
      led_on[led_number] = 0;    
      led_available[led_number] = 1; // LED is available again
    }//else{
    //  if(debug){Serial.print("*");}
    //}
  }
  //if(debug){
    //Serial.print("](");
    //Serial.print(led_on_count[led_number]);
    //Serial.print(", ");
    //Serial.print(led_off_count[led_number]);   
    //Serial.print(") ");
  //}
  // if(debug){
  //   Serial.print(time_counter);
  //   Serial.print(": num=");
  //   Serial.print(led_number);
  //   Serial.print(": avail=");
  //   Serial.print(led_available[led_number]);
  //   Serial.print(": on_cnt=");
  //   Serial.print(led_on_count[led_number]);
  //   Serial.print(": off_cnt=");
  //   Serial.print(led_off_count[led_number]);
  //   Serial.print(": on?");
  //   Serial.println(led_on[led_number]);              
  // }
}





// LED tests
void led_test_suite(void){
  // Test LEDs

  uint8_t z = 1;

  
  if(led_test or inf_led_test){
    delay(500);
    if(debug){
      Serial.println("Testing LEDs..."); 
    }

    // uint16_t delay_count = time_counter;
    
    // inf led test
    z =1;

    while(z){
      if(debug){Serial.print("long led test...");}
      for(int y=0;y<10;y++){
        for(uint8_t x=0; x<max_leds; x++){
          set_led(x, !led_is_inverted);
        }        
        delay(250);
        for(uint8_t x=0; x<max_leds; x++){
          set_led(x, led_is_inverted);
        }  
        delay(250);           
      }
      for(int y=0;y<10;y++){
        for(int x=0; x<max_leds; x++){
          set_led(x, led_is_inverted);    
          delay(500);
          set_led(x, !led_is_inverted);
          delay(500);
        }           
      }
      
    
      for(uint8_t x=0; x<max_leds; x++){
        set_led(x, !led_is_inverted);
        // lets u check timing of actual circuit (1s)
      // while(time_counter < 10 + delay_count ){
          delay(500);
      // }
      // delay_count = time_counter;
      }
      for(uint8_t x=0; x<max_leds; x++){
        set_led(x, !led_is_inverted);
        // lets u check timing of actual circuit (1s)
      // while(time_counter < 10 + delay_count ){
          delay(500);
      // }
      // delay_count = time_counter;
      }

      for(uint8_t x=0; x<max_leds; x++){
        set_led(x, led_is_inverted);
        // lets u check timing of actual circuit (1s)
      //  while(10 + delay_count > time_counter){
        delay(500);
      //  }
      //  delay_count = time_counter;
      }
    // stop the test if not inf
    if(!inf_led_test){
      z=0;
    }
    }

  }

}




//////////////////////////////////////////////////////
// Overload check
void overload_reset(void){

  // if the counter is about to overload
  if( time_counter > 65536 - max_off_time - 100){
    // reset counter
    time_counter = 10;
    // reset all led counters
    for(uint8_t x=0; x<max_leds; x++){
      led_on_count[x] = 0;
      led_off_count[x] = 0;
      led_available[x] = 1;
      led_on[x] = 0;

      // turn off all leds
      set_led(x,0);
    }
  }
}


////////////////////////////////////////////
// Software reset
//
void(* soft_reset_function) (void) = 0;//declare reset function at address 0





///////////////////////////////////////////////////
// Initialize
//
void setup() {

  // Serial
  if(debug){
    Serial.begin(9600);           // set up Serial library at 9600 bps
    Serial.println("Starting program..."); 
  }


  // Set output pins
  for(uint8_t x=0; x<max_leds; x++){
    pinMode(pin_address[x], OUTPUT);

    // make all pin available
    led_available[x] =1;
  }
 
  // Initalize counter (timer 1) 
  TCCR1A = 0;
  TCCR1B = 0;
  TCNT1 = timer_div; //59286 ; //reset timer 65536-16000000/256/10 = 100ms
  TCCR1B |= (1 << CS12); // Prescaler 256
  TIMSK1 |= (1 << TOIE1); // interrupt (increments count every 100ms)

  // LED tests
  led_test_suite();

  // pause before start
  delay(250);

  // Reset to OFF
  for(uint8_t x=0; x<max_leds; x++){
    set_led(x, !led_is_inverted);
  }
  if(debug){
    Serial.println("Starting Lightning bugs..."); 
  }
  // while(time_counter < 100){
    // Serial.println(weighted_random_probability(100,700,300));
  // }
  // while(1){1;}

  wdt_enable(WDTO_8S); //Enable WDT with a timeout of 8 seconds
}




////////////////////////////////////////////////////////////////////////////////////////
// Main loop
//

  // uint16_t ccc = 0;
  // uint16_t ddd = 0;


void loop(){
  // DO NOT PUT AN INFINIT LOOP IN loop(),  //
  //   It fails after hours of looping      //

  // Soft Reset
  //  counter gets reset when led function is called
  //  if counter goes above max, MCU preforms a soft reset
  //  WDT should catch these, but wasn't, for what ever reason
//  if( softreset_count > softreset_count_max){
//    soft_reset_function();
//  }else{
//    softreset_count++;
//  }

  //while(time_counter < 3000){

    // debug, prints interval number
    if(debug){
      Serial.print(time_counter);
      Serial.print('-');
    }

    // process all leds
    for(uint8_t led_number=0;led_number<max_leds;led_number++){
      // Check and light up LEDs, decide to blink or wait
      
      bleep(led_number);

      // prints status of LEDs
      if(debug){ Serial.print(led_on[led_number]); }
    }

    // debug (newline)
    if(debug){ Serial.println(); }


    // Overload check: if counter is about to overload, reset
    overload_reset();
  
    wdt_reset(); //kick the watchdog

    // a delay loop
    delay(loop_delay);


    // ccc++;

    // if(ccc > 50000){
    //   Serial.print(ddd);
    //   Serial.print("freeMemory()=");
    //   Serial.println(freeMemory());
    //   ccc=0;
    //   ddd++;
    // } 
}


