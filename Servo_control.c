#include <lpc214x.h>   		  
#include <stdint.h>	  

#define SwitchPinNumber1 17  
#define SwitchPinNumber2 18	 
#define SwitchPinNumber3 19	 
#define SwitchPinNumber4 20	 

#define AD04 (1<<18) //Select AD0.4 function for P0.25		  
#define AD01 (1<<24) //Select AD0.1 function for P0.28	 
#define AD02 (1<<26) //Select AD0.2 function for P0.29	 
#define AD03 (1<<28) //Select AD0.3 function for P0.30 

#define SEL_AD04 (1<<4) //Select ADC channel 4	 
#define SEL_AD01 (1<<1) //Select ADC channel 1	 
#define SEL_AD02 (1<<2) //Select ADC channel 2	 
#define SEL_AD03 (1<<3) //Select ADC channel 3	 

#define CLKDIV (15-1)    // 4Mhz ADC clock (ADC_CLOCK=PCLK/CLKDIV) where "CLKDIV-1" is actually used , in our case PCLK=60mhz 
#define BURST_MODE_OFF (0<<16)           // 1 for on and 0 for off											
#define PowerUP (1<<21)																				 
#define START_NOW ((0<<26)|(0<<25)|(1<<24))   //001 for starting the conversion immediately		   
#define ADC_DONE (1UL<<31)																		  
#define VREF 3.3                            //Reference Voltage at VREF Pin						 

volatile int result1=0;	
volatile int result2=0;	
volatile int result3=0;	 
volatile int result4=0;	

volatile int convert1=0; 
volatile int convert2=0; 
volatile int convert3=0;
volatile int convert4=0;  

volatile unsigned int  switchStatus1;
volatile unsigned int  switchStatus2; 
volatile unsigned int  switchStatus3;
volatile unsigned int  switchStatus4; 

long map(long x, long in_min, long in_max, long out_min, long out_max)  // Map function to convert range  
{																										
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;						 
}																				 																				  

void delay_ms(uint16_t j)      /* loop to generate 1 milisecond delay with Cclk = 60MHz */	
{			
    uint16_t x,i;
	for(i=0;i<j;i++)
	{	 
    	for(x=0; x<6000; x++);
	}  				 
}	 
  
int main() 
{	      
	unsigned long AD0CR_setup = (CLKDIV<<8) | BURST_MODE_OFF | PowerUP; //Setting up ADC Mode     
	PINSEL0 = 0x000A800A;   // Configure P0.1,P0.7,P0.8 as PWM3,PWM2,PWM4 pins		  
    PINSEL2 = 0x00000000;   //Configure the PORT1 Pins as GPIO;						
    PINSEL1 |= AD04 | AD01 | AD02 | AD03 | (1<<10); // Configure P0.25,P0.28,P0.29,P0.30 as ADC0.4,   ADC0.1, ADC0.2, ADC0.3 Pins & P0.21 as PWM5	   	   
    IODIR1 = ((0<<17)|(0<<18)|(0<<19)|(0<<20)); //Configure Switch Pin as input	  
	PWMTCR = 0x02;          // Reset and disable counter for PWM	   
	PWMPR =  0x1D;          // Prescale Register value				 
	PWMMR0 = 20000;         // Time period of PWM wave, 20msec	   
	PWMMR4 = 0;                          // Initial values of PWM wave 0 msec
	PWMMR2 = 0;																   
	PWMMR3 = 0;				 
	PWMMR5 = 0;				  
	PWMMCR = 0x00000002;           // Reset on MR0 match
	PWMLER = 0x7C;                 // Latch enable for PWM2,PWM3,PWM4 and PWM5	
	PWMPCR = 0x7C00;         // Enable PWM2,PWM3,PWM4 and PWM5, single edge controlled PWM	
	PWMTCR = 0x09;                 // Enable PWM and counter
	while(1) 
	{
		switchStatus1 = (IOPIN1>>SwitchPinNumber1) & 0x01 ;  // Read the switch status
		switchStatus2 = (IOPIN1>>SwitchPinNumber2) & 0x01 ;  // Read the switch status
		switchStatus3 = (IOPIN1>>SwitchPinNumber3) & 0x01 ;  // Read the switch status
		switchStatus4 = (IOPIN1>>SwitchPinNumber4) & 0x01 ;  // Read the switch status
		if(switchStatus1 == 1)                      //Turn ON/OFF depending on switch status and start ADC conversion
		{ 
			AD0CR =  AD0CR_setup | SEL_AD01;         //Setup ADC for channel 1
			AD0CR |= START_NOW;                               //Start new Conversion at ADC1
			while( (AD0DR1 & ADC_DONE) == 0 )        //Check for ADC conversion
			{
				convert1 = (AD0DR1>>6) & 0x3ff;         //Get ADC value
				result1 = map(convert1,0,1023,0,2450);  //Convert ADC values in terms of dutycyle for PWM
				PWMMR5 = result1;                               //Set Duty Cylce value to PWM5
				PWMLER = 0x20;                          //Enable PWM5
				delay_ms(2);
			}
		}
		if(switchStatus2 == 1)                      //Turn ON/OFF depending on switch status and start ADC conversion
		{ 
			AD0CR =  AD0CR_setup | SEL_AD02;         //Setup ADC for channel 2
			AD0CR |= START_NOW;                      //Start new Conversion at ADC2
			while( (AD0DR2 & ADC_DONE) == 0 )        //Check for ADC conversion
			{
				convert2 = (AD0DR2>>6) & 0x3ff;        //Get ADC value
				result2 = map(convert2,0,1023,0,2450); //Convert ADC values in terms of dutycyle for PWM
				PWMMR4 = result2;                      //Set Duty Cylce value to PWM4
				PWMLER = 0x10;                         //Enable PWM4
				delay_ms(2); 
	         }
		}
		if(switchStatus3 == 1)                       //Turn ON/OFF depending on switch status and start ADC conversion
		{
			AD0CR =  AD0CR_setup | SEL_AD03;          //Setup ADC for channel 3  
			AD0CR |= START_NOW;                       //Start new Conversion at ADC3
			while( (AD0DR3 & ADC_DONE) == 0 )         //Check for ADC conversion
			{
				convert3 = (AD0DR3>>6) & 0x3ff;         //Get ADC value
				result3 = map(convert3,0,1023,0,2450);  //Convert ADC values in terms of dutycyle for PWM
				PWMMR3 = result3;                                 //Set Duty Cylce value to PWM3
				PWMLER = 0x08;                          //Enable PWM3
				delay_ms(2);
			}
		} 
		if(switchStatus4 == 1)                      //Turn ON/OFF depending on switch status and start ADC conversion
		{
			AD0CR =  AD0CR_setup | SEL_AD04;          //Setup ADC for channel 4
			AD0CR |= START_NOW;                       //Start new Conversion at ADC4
			while( (AD0DR4 & ADC_DONE) == 0 )         //Check for ADC conversion
			{
				convert4 = (AD0DR4>>6) & 0x3ff;           //Get ADC value
				result4 = map(convert4,0,1023,0,2450);    //Convert ADC values in terms of dutycyle for PWM
				PWMMR2 = result4;                         //Set Duty Cylce value to PWM  
				PWMLER = 0x04;                            //Enable PWM2
				delay_ms(2);
			}
		} 
	}
}

