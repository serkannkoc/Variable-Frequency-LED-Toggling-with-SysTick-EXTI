#include "stm32l476xx.h"

#define SysTick_CTRL_CLKSOURCE 1UL<<2
#define SysTick_CTRL_TICKINT 1UL<<1
#define SysTick_CTRL_ENABLE 1UL<<0
volatile int32_t TimeDelay;

void SysTick_Init(){
	SysTick->CTRL = 0; // Disable SysTick
	SysTick->LOAD = 1000 - 1; // Set reload register
	// Set interrupt priority of SysTick to least urgency (i.e., largest priority value)
	NVIC_SetPriority (SysTick_IRQn, (1<<__NVIC_PRIO_BITS) - 1);
	SysTick->VAL = 0; // Reset the SysTick counter value
	// Select processor clock: 1 = processor clock; 0 = external clock
	SysTick->CTRL |= SysTick_CTRL_CLKSOURCE;
	// Enables SysTick interrupt, 1 = Enable, 0 = Disable
	SysTick->CTRL |= SysTick_CTRL_TICKINT;
	// Enable SysTick
	SysTick->CTRL |= SysTick_CTRL_ENABLE;
}

void SysTick_Handler (void) { // SysTick interrupt service routine
	if (TimeDelay > 0) // Prevent it from being negative
		TimeDelay--; // TimeDelay is a global volatile variable
}

void delay (void) {
	while(TimeDelay != 0); // Busy wait
}

void LED_Init(void){
	RCC->AHB2ENR |= RCC_AHB2ENR_GPIOAEN; // Enable clock of Port A
	GPIOA->MODER &= ~GPIO_MODER_MODER5; // Clear mode bits
	GPIOA->MODER |= GPIO_MODER_MODER5_0; // Set mode to output
	GPIOA->OTYPER &= ~GPIO_OTYPER_OT_5; // Select push-pull output
}

void EXTI_Init(void){
	RCC->AHB2ENR |= RCC_AHB2ENR_GPIOCEN; // Enable clock of Port C
	GPIOC->MODER &= ~GPIO_MODER_MODER13; // Set PC.13 as digital input
	GPIOC->PUPDR &= ~GPIO_PUPDR_PUPDR13; 
	GPIOC->PUPDR |= GPIO_PUPDR_PUPDR13_1;// Pull down
	NVIC_EnableIRQ(EXTI15_10_IRQn); // Enable Interrupt 
	// Connect External Line to the GPI
	RCC->APB2ENR |= RCC_APB2ENR_SYSCFGEN;
	SYSCFG->EXTICR[0] &= ~SYSCFG_EXTICR4_EXTI15;
	SYSCFG->EXTICR[0] |= SYSCFG_EXTICR4_EXTI15_PC;
	// Interrupt Mask Register
	// 0 = marked, 1 = not masked (enabled)
	EXTI->IMR1 |= EXTI_IMR1_IM15;
	// Rising trigger selection
	// 0 = trigger disabled, 1 = trigger enabled
	EXTI->RTSR1 |= EXTI_RTSR1_RT15;
	while(1);
}

void EXTI15_10_IRQHandler(void){
	// Check for EXTI 15 interrupt pending flag
	if ((EXTI->PR1 & EXTI_PR1_PIF15) != 0) {
		if(TimeDelay == 500){
			TimeDelay = 1000;
		} else{
			TimeDelay = 500;
		}
	// Cleared interrupt pending flag by writing 1
	EXTI->PR1 |= EXTI_PR1_PIF15;
	}
}


int main(void){
	SysTick_Init();
	LED_Init();
	EXTI_Init();
  TimeDelay = 500;
	while(1){
		GPIOA->ODR ^= GPIO_ODR_ODR_5; // Output 1 to turn on green LED
		delay();
	}
}
