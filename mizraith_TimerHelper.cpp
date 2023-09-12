/*********************************************************************** 
  This is a library for the Arduino for centralizing timer/counter
  setup.  I find that the code to setup Timer0 and Timer1 can be
  a little convoluted and take a lot of research.  Much of that
  research is pulled out of the datasheet and included here for
  quick reference.
  
  PLEASE NOTE that Time0 is used by Arduino for the millis() and delay()
  methods, so its use should be avoided unless you are sure you will NOT
  be using those Arduino calls.
  
  AUTHOR: Red Byer (www.redstoyland.com) 
  DATE:   9/16/2013
  Code available on github: https://github.com/mizraith/
  
  BSD license, all text above must be included in any redistribution
 **********************************************************************/

#include <avr/pgmspace.h>
#include "mizraith_TimerHelper.h"

//BASIC stuff
#if ARDUINO >= 100
 #include "Arduino.h"
#else
 #include "WProgram.h"
#endif



//#########################################################
// TIMER/COUNTER CONTROLS 
//   Timer 0 is PD4    but is used by Arduino so be careful
//   Timer 1 is PD5
//#########################################################

/**
* Setup Timer 0 to count and interrupt
*
* --- TCCR0A ---
* COM0A1  COM0A0  COM0B1 COM0B0  xxx  xxx  WGM01  WGM00
* --- TCCR0B ---
* FOC0A   FOC0B   xxx     xxx    WGM02 CS02  CS01  CS00
*
*    In non-PWM Mode COM0A1:0 == 0b00  Normal port operation, OC0A disconnected
*        same for COM0B1:0   
* For WGM02 WGM01 WGM00 
*  0b000  Normal mode, with a TOP of 0xFF and update of OCRx immediately
*           Counting is upwards.  No counter clear performed.  Simply overruns and restarts.
*  0b010  CTC Mode with top of OCRA and update of OCRx immediately.
*        Clear Timer on Compare Match, OCR0A sets top.  Counter is cleared when TCNT0 reaches OCR0A
*        Need to set the OCF0A interrupt flag
*
* FOC0A/FOC0B should be set zero. In non-PWM mode it's a strobe. 
*        But does not generate any interrupt if using CTC mode.
* 
* CS02:0  Clock Select:
*   0b000  No clock source. Timer/Counter stopped
*   0b110  External clock on T0 pin FALLING EDGE
*   0b111  External clock on T0 pin RISING EDGE
*
* TCNT0  Timer counter register
*
* OCR0A/OCR0B  Output Compare Register
*
* --- TIMSK0 ---
* xxx xxx xxx xxx xxx OCIE0B  OCIE0A  TOIE0
*    OCIE0B/A are the Output Compare Interrupt Enable bits
*    TOIE0 is the Timer Overflow Interrup Enable
*
*  Must write OCIE0A to 1 and then I-bit in Status Register (SREG)
*
* --- TIFR0 ---   Timer Counter Interrupt Flag Register
*  xx xx xx xx   xx OCF0B OCF0A TOV0
*   OCF0B/A  Output Compare A/B Match Flag 
*          is set when a match occurs.  Cleared by hardware when executing interrupt.
*          Can also be cleared by writing a 1 to the flag.
*
*/

  
void mizraith_TimerHelper::setupTimer0ForCounting(uint8_t count) {
  //set WGM2:0 to 0b010 for CTC
  //set CS02:0 to 0b111 for rising edge external clock


  TCCR0A = 0;
  TCCR0B = 0;
  TIMSK0 = 0;
 
  TCCR0A |= (1 << WGM01);
//  TCCR0A = _BV(WGM01);

  TCCR0B |= (1 << CS02);
  TCCR0B |= (1 << CS01);
  TCCR0B |= (1 << CS00);

//  TCCR0B = _BV(CS02) || _BV(CS01) || _BV(CS00);
  
  TCNT0 = 0;
  
  OCR0A = count;      // SET COUNTER 
  
 // TIMSK0 = _BV(OCIE0A);   // SET INTERRUPTS
  TIMSK0 |= (1 << OCIE0A);
}

/**
* @function: setupTimer1ForCounting
* @param count  16-bit integer to go into OCR1A
*
* TCCR1A = [ COM1A1, COM1A0, COM1B1, COM1B0, xxx, xxx, WGM11, WGM10]
* TCCR1B = [ ICNC1,  ICES1,  xxx,    WGM13, WGM12, CS12, CS11, CS10]
* TCCR1C = [ FOC1A,  FOC1B, xxx, xxx, xxx, xxx, xxx, xxx]
* TIMSK1 = [ xxxx,  xxxx,  ICIE1,  xxxx, xxxx, OCIE1B, OCIE1A, TOIE1]
*
*  Set COM1A, COM1B to 0 for normal operation with OC1A/OC1B disonnected.
*  Set WGM13:0 to 0b0100 for CTC Mode using OCR1A
*
*  We won't use the Input Capture Noise Canceler (ICNC1)
*  CS12:0 to 0b111 for external source on T1, clock on rising edge.
*
*  TCNT1H and TCNT1L  (TCNT1) 
*  OCR1AH / OCR1AL  Output Compare Register 1A
*       Can we set this by controlling OCR1A only?  Or do we need to bit shift.
*
*  Set OCIE1A
*/
void mizraith_TimerHelper::setupTimer1ForCounting(int count) {
  //set WGM1[3:0] to 0b0100 for CTC mode using OCR1A. Clear Timer on Compare Match, OCR1A sets top. 
  //                            Counter is cleared when TCNT0 reaches OCR0A
  //set CS1[2:0] to 0b111 for external rising edge T1 clocking.
  //set OCR1A to count
  //set TIMSK1 to OCIE1A

  //clear it out
  TCCR1A = 0;      //nothing else to set
  TCCR1B = 0;
  TIMSK1 = 0;
 
  TCCR1B |= (1 << WGM12);
  TCCR1B |= (1 << CS12);
  TCCR1B |= (1 << CS11);
  TCCR1B |= (1 << CS10);  
  
  TCNT1 = 0;
  
  OCR1A = count;      // SET COUNTER 
  
  TIMSK1 |= (1 << OCIE1A);
}

void mizraith_TimerHelper::setupTimer1ForInternalPhaseCorrectPWM(int count) {
  //set WGM1[3:0] to 0b0100 for CTC mode using OCR1A. Clear Timer on Compare Match, OCR1A sets top. 
  //                            Counter is cleared when TCNT0 reaches OCR0A
  //set CS1[2:0] to 0b001 for internal clocking.
  //set OCR1A to count
  //set TIMSK1 to OCIE1A
//  ----- Timer1 Information -----
//TCCR1A: 00000011
//TCCR1B: 00010101
//TIMSK1: 00000010
//OCR1A : 11100000

  //clear it out
  TCCR1A = 0;      //nothing else to set
  TCCR1B = 0;
  TIMSK1 = 0;
  
  TCCR1B |= (1 << WGM13);
  //TCCR1A |= (1 << WGM12);
  TCCR1A |= (1 << WGM11);     // WGM13:10 0b1011 is Phase Correct with OCR1A at top 
  TCCR1A |= (1 << WGM10);     // WGM12:10 0b000 is normal mode   WGM12:10 0b100 is CTC mode 
  
  TCCR1B |= (1 << CS12);       //CS12:10  0b101 = clk/1024  (pg 139)
  //TCCR1B |= (1 << CS11);
  TCCR1B |= (1 << CS10);  
  
  TCCR1A |= (1 << COM1A1);      // COM1x1:0 0b11 sets OC1x on compare match when upcoaunting, clear when downcoutning
  TCCR1A |= (1 << COM1A0);     //  COM1x1:0 0b00  OC1x disconnected.  COM1A1:0 0b01  OC1A connected
  
  
  
  TCNT1 = 0;
  
  OCR1A = count;      // SET COUNTER 
  
  TIMSK1 |= (1 << OCIE1A);
}


void mizraith_TimerHelper::setTimer1Count(int count) {
	OCR1A = count;      // SET COUNTER    may cause hiccups
}


//void mizraith_TimerHelper::setupTimer2ForInternalPhaseCorrectPWM(int count) {
//  //set WGM1[3:0] to 0b0100 for CTC mode using OCR1A. Clear Timer on Compare Match, OCR1A sets top. 
//  //                            Counter is cleared when TCNT0 reaches OCR0A
//  //set CS1[2:0] to 0b001 for internal clocking.
//  //set OCR1A to count
//  //set TIMSK1 to OCIE1A
//
//  //clear it out
//  TCCR1A = 0;      //nothing else to set
//  TCCR1B = 0;
//  TIMSK1 = 0;
//  
//  TCCR1B |= (1 << WGM13);
//  //TCCR1A |= (1 << WGM12);
//  TCCR1A |= (1 << WGM11);     // WGM13:10 0b1011 is Phase Correct with OCR1A at top 
//  TCCR1A |= (1 << WGM10);     // WGM12:10 0b000 is normal mode   WGM12:10 0b100 is CTC mode 
//  
//  TCCR1B |= (1 << CS12);       //CS12:10  0b101 = clk/1024  (pg 139)
//  //TCCR1B |= (1 << CS11);
//  TCCR1B |= (1 << CS10);  
//  
//  //TCCR1A |= (1 << COM1A1);      // COM1x1:0 0b11 sets OC1x on compare match when upcoaunting, clear when downcoutning
//  // TCCR1A |= (1 << COM1A0);     //  COM1x1:0 0b00  OC1x disconnected.  COM1A1:0 0b01  OC1A connected
//  
//  
//  
//  TCNT1 = 0;
//  
//  OCR2A = count;      // SET COUNTER 
//  
//  TIMSK1 |= (1 << OCIE1A);
//}
//
//
//void mizraith_TimerHelper::setTimer2Count(int count) {
//	OCR2A = count;      // SET COUNTER    may cause hiccups
//}




void mizraith_TimerHelper::printTimer0Info() {
  char bitstring[] = "00000000";
  
  Serial.println(" ");  
  Serial.println("----- Timer1 Information -----");
  Serial.print("TCCR0A: ");
  getBinaryString(TCCR0A, bitstring);
  Serial.println(bitstring);
//  Serial.println(TCCR0A, BIN);
  Serial.print("TCCR0B: ");
  getBinaryString(TCCR0B, bitstring);
  Serial.println(bitstring);
//  Serial.println(TCCR0B, BIN);
  Serial.print("TIMSK0: ");
  getBinaryString(TIMSK0, bitstring);
  Serial.println(bitstring);
//  Serial.println(TIMSK0, BIN);
  Serial.println("------------------------------");
  Serial.println(" ");  
}


void mizraith_TimerHelper::printTimer1Info() {
  char bitstring[] = "00000000";
  
  Serial.println(" ");  
  Serial.println("----- Timer1 Information -----");
  Serial.print("TCCR1A: ");
  getBinaryString(TCCR1A, bitstring);
  Serial.println(bitstring);
//  Serial.println(TCCR1A, BIN);
  Serial.print("TCCR1B: ");
  getBinaryString((uint8_t)TCCR1B, bitstring);
  Serial.println(bitstring);
//  Serial.println(TCCR1B, BIN);
  Serial.print("TIMSK1: ");
  getBinaryString(TIMSK1, bitstring);
  Serial.println(bitstring);
//  Serial.println(TIMSK1, BIN);
  Serial.print("OCR1A : " );
  getBinaryString(OCR1A, bitstring);
  Serial.println(bitstring);  
//  Serial.println(OCR1A, BIN);
  Serial.println("------------------------------");
  Serial.println(" ");  
}





/**
 * Takes one byte and loads up bytestr[8] with the value
 *  eg  "8" loads bytestr with "00000111"
 */
void mizraith_TimerHelper::getBinaryString(uint8_t byteval, char bytestr[]) 
{
	uint8_t bitv;
	int i = 0;
	
    for (i = 0; i < 8; i++) {                    
           bitv = (byteval >> i) & 1;
           if (bitv == 0) {
               bytestr[7-i] = '0';
           } else {
               bytestr[7-i] = '1';
           }
    }
}

//works as a class variable, and can't call static, as it's in the .h as static
//char mizraith_TimerHelper::mystring[25] = "Funny, isn't it?"; 
  
//char * mizraith_TimerHelper::returnString() 
//{
//    //the following line works
//   //static char mizraith_TimerHelper::mystring[25] = "Funny, isn't it?"; 
//   char demostring[] = "This is the demo string";
//  strcpy(mystring, demostring);
//    return mystring;
//}


