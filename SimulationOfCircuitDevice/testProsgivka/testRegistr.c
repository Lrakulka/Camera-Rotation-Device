#include <tiny85.h>
#include <delay.h>
void main(void)
{   
    int i=0;
    bit k=0;
   DDRB=0b011010;   
  // PORTB.5=1;    
 /*  PORTB.4=0;     
   PORTB.1=0;    
   PORTB.3=1;
   while(1)
   {     
      PORTB.1=~PORTB.1;    
      PORTB.3=~PORTB.3;            
      delay_ms(500);
       PORTB.1=~PORTB.1;    
      PORTB.3=~PORTB.3; 
      delay_ms(500);
      PORTB.4=~PORTB.4;       
       
   }         */    
   PORTB.4=1;  
   PORTB.4=0; 
   
   PORTB.1=0;
   PORTB.3=0;
   PORTB.1=1; 
   
   PORTB.1=0; 
   PORTB.3=0;
   PORTB.1=1;
   
   PORTB.1=0; 
   PORTB.3=0;
   PORTB.1=1;
   
   PORTB.1=0; 
   PORTB.3=1;
   PORTB.1=1;
   
   PORTB.1=0; 
   PORTB.3=0;
   PORTB.1=1;
               
   PORTB.1=0; 
   PORTB.3=0;
   PORTB.1=1;
   
   PORTB.1=0; 
   PORTB.3=0;
   PORTB.1=1; 
   
   PORTB.1=0; 
   PORTB.3=0;
   PORTB.1=1;
   
   PORTB.4=1;   
   PORTB.4=0;
}