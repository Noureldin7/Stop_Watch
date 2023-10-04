#include <avr/io.h>
#include <avr/interrupt.h>
// 0: Seconds ---> 5: Hours
// Values for enabling 7-segment displays
unsigned char enables[6] = { 1, 2, 4, 8, 16, 32};
// 0: Seconds ---> 5: Hours
// Values displayed on each 7-segment display
unsigned char values[6] = {0};
// 0: Seconds ---> 5: Hours
// Values indicating which 7-segment displays have the dot active
unsigned char dots[6] = {64,64,0,64,0,64};
// 0: Seconds ---> 5: Hours
// Values indicating max values for each 7-segment display
unsigned char max[6] = {10,6,10,6,10,10};
// Circular Index for multiplexed configuration
unsigned char enable_index = 0;
void port_init()
{
	DDRC |= 0x0F;
	PORTC &= ~0x0F;
	DDRA |= 0x7F;
	PORTA &= ~0x7F;
	DDRD &= ~(1<<PD2) & ~(1<<PD3);
	PORTD |= (1<<PD2);
	DDRB &= ~(1<<PB2);
	PORTB |= (1<<PB2);
}
void interrupt_init()
{
	SREG |= (1<<SREG_I);
	MCUCR |= (1<<ISC01) | (1<<ISC10) | (1<<ISC11);
	GICR |= (1<<INT0) | (1<<INT1) | (1<<INT2);
}
void timer_init()
{
	TCCR1A |= (1<<FOC1A);
	TCCR1B |= (1<<WGM12) | (1<<CS10) | (1<<CS11);
	OCR1A = 15625;
	TIMSK |= (1<<OCIE1A);
}
// Circularly displays the current value for each 7-segment display
void circular_display_update()
{
	PORTA = (PORTA &= ~0x7F) | enables[enable_index] | dots[enable_index];
	PORTC = (PORTC & ~0x0F) | values[enable_index];
	enable_index = (enable_index +1)%6;
}
int main()
{
	port_init();
	interrupt_init();
	timer_init();
	while(1)
	{
		circular_display_update();
	}
}
ISR(TIMER1_COMPA_vect)
{
	unsigned char curr_index = 0;
	do
	{
		values[curr_index] = (values[curr_index]+1)%max[curr_index];
	}
	while(values[curr_index++]==0 && curr_index!=6);
}
ISR(INT2_vect)
{
	// Clear values
	for(int i = 0; i<6; i++)
	{
		values[i] = 0;
	}
}
ISR(INT1_vect)
{
	// Turn Off Timer
	TCCR1B &= ~(1<<CS12) & ~(1<<CS11) & ~(1<<CS10);
}
ISR(INT0_vect)
{
	// Turn On Timer
	TCCR1B |= (1<<CS11) | (1<<CS10);
}
