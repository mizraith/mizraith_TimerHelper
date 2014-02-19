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

#ifndef _mizraith_TimerHelper_H_
#define _mizraith_TimerHelper_H_


class mizraith_TimerHelper {
  public:
    void setupTimer0ForCounting(uint8_t count);
    void setupTimer1ForCounting(int count);

    void printTimer0Info(); 
    void printTimer1Info();

    char * returnString();
    
  private:
	  void getBinaryString(uint8_t byteval, char bytestr[]);
      static char mystring[];
	  
};



#endif
