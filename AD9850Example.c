/*AD9850 TEST PROGRAM
 *This is an example how the AD9850 can be used with an AVR.
 *all functions of the AD9850 are implemented, communication is handled via a 
 *8-Bit parallel connection
 *Since no special periphery is used it should run on any ATmega/ATtiny/ATXmega with enough digital outputs
 *One whole 8-Bit Port and three Bits from another Port are needed. 
 *In the example I used Port D for the parallel data and port B for the control pins. If you want to use another
 *Port, simply change the #defines at the beginning
 *
 *No warranty whatsoever is given for the following code.
 *
 * AD9850test.c
 *
 * Created: 18.02.2015 18:47:17
 *  Author: phil - Awesomecircuits.com
 *  License: MIT
 */ 
#define F_CPU	16000000

#include <avr/io.h>
#include <util/delay.h>
#include <math.h>



#define AD9850DataDIR	DDRD
#define AD9850DataPORT	PORTD					//parallel 8-Bit data Port
#define AD9850DataPIN	PIND
#define AD9850CtlDIR	DDRB
#define AD9850CtlPORT	PORTB
#define AD9850CtlPIN	PINB

#define WCL_PIN		0							//Data clock line
#define UDT_PIN		1							//Data latch line
#define RST_PIN		2							//Reset Pin

#define STAT_LED	5

uint8_t SetAD9850(uint32_t freq, uint16_t phase);
void sendbyte(uint8_t data);
void sendword(uint8_t word[5]);
void ad9850_reset(void);
void ad9850_sleep(void);
void ad9850_wakeup(void);

void blink(uint8_t n);



int main(void)
{
	//Setup AD9850
	AD9850DataDIR = 0xFF;
	AD9850CtlDIR |= ((1<<WCL_PIN)|(1<<UDT_PIN)|(1<<STAT_LED)|(1<<RST_PIN));
	AD9850DataPORT = 0x00;
	AD9850CtlPORT &= ~((1<<WCL_PIN)|(1<<UDT_PIN)|(1<<STAT_LED)|(1<<RST_PIN));

	

	
    while(1)
    {
		ad9850_reset();
		
		//Set Frequency different frequencys between 100Hz and 50MHz
		blink(1);
		SetAD9850(100, 0);
		_delay_ms(20000);
		blink(2);
		SetAD9850(1e3, 30);
		_delay_ms(20000);
		blink(3);
		SetAD9850(1e4, 60);
		_delay_ms(20000);
		blink(4);		
		SetAD9850(1e5, 90);
		_delay_ms(20000);
		blink(5);
		SetAD9850(1e6, 120);
		_delay_ms(20000);
		blink(6);
		SetAD9850(1e7, 150);
		_delay_ms(20000);
		blink(7);
		SetAD9850(2e7, 180);
		_delay_ms(20000);
		blink(8);
		SetAD9850(4e7, 210);
		_delay_ms(20000);
		blink(9);
		SetAD9850(5e7, 240);
		_delay_ms(20000);
		
		//Enter sleepmode 
		ad9850_sleep();
		_delay_ms(5000);
		//Wakeup again
		ad9850_wakeup();
		_delay_ms(5000);
    }
}


/*SetAD9850(uint32_t freq, uint16_t phase)
This function sets the AD9850 to a specified frequency and phase.
The frequency can be in the Range between 0.3 Hz and 50 MHz (very poor signal strenth at 50 MHz).
The phase can be set between 0 and 359 degree
The function return one on completion
This function is not speed optimized, if you want to alternate between fixed frequency points,
use static data words for the 9850 (the calculation of the frequency and phase word takes time)
*/ 
uint8_t SetAD9850(uint32_t freq,uint16_t phase){
	
	freq = freq * 34.359738368;
	phase =  (uint8_t) (phase/(360/32));
	
	
	uint8_t data[5];
	data[1] = freq;
	freq = freq >> 8;
	data[2] = freq;
	freq = freq >> 8;
	data[3] = freq; 
	freq = freq >> 8;
	data[4] = freq;
	freq = freq >> 8;
	data[5] =  phase;
	data[5] = data[5]<<3;
	
	sendword(data);
	return 1;
}

/*sendword(uint8_t word[5])
sends the 5 byte data word to the AD9850
*/
void sendword(uint8_t word[5]){
	uint8_t i=0;
	for(i=5; i>1; i--){
		sendbyte(word[i]);
	}
	AD9850CtlPORT |= (1<<UDT_PIN);
	AD9850CtlPORT &= ~(1<<UDT_PIN);

}

/*sendbyte(uint8_t data)
sends one byte to the AD9850
*/
void sendbyte(uint8_t data){
	AD9850DataPORT = data;				
	//nop;									//there must always be a 3.5 ns delay between data set and clock
	AD9850CtlPORT |= (1<<WCL_PIN);			
	//nop;									//min clock width is 3.5 ns
	AD9850CtlPORT &= ~(1<<WCL_PIN);
}

/*ad9850_reset(void)
resets the ad9850. On reset, all data registers are cleared and no signal is outputted
*/
void ad9850_reset(void){
	AD9850CtlPORT |= (1<<RST_PIN);
	_delay_us(1);
	AD9850CtlPORT &= ~(1<<RST_PIN);
	_delay_us(1);
}

/*ad9850_sleep
the AD9850 enters the sleep mode
*/
void ad9850_sleep(void){
	ad9850_reset();							//Just to be sure 
	sendbyte(0x04);							//Set control register to sleep
	AD9850CtlPORT |= (1<<UDT_PIN);			//Data Latch
	AD9850CtlPORT &= ~(1<<UDT_PIN);
}

/*ad9850_wakeup
the AD9850 wakes up from it's sleep mode and resets all its registers, no waveform is generated afterwards
*/
void ad9850_wakeup(void){
	ad9850_reset();							//Just to be sure
	sendbyte(0x00);							//Wakeup
	AD9850CtlPORT |= (1<<UDT_PIN);			//Data Latch
	AD9850CtlPORT &= ~(1<<UDT_PIN);
	ad9850_reset();
}

/*blink(uint_8t n)
blink n times with the status LED
*/
void blink(uint8_t n){
	while (n>=1)
	{
		PORTB |= (1<<STAT_LED);
		_delay_ms(500);
		PORTB &= ~(1<<STAT_LED);
		_delay_ms(500);
		n--;
	}
}