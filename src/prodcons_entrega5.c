#include <stdint.h>
#include <stdbool.h>
#include "system_tm4c1294.h" // CMSIS-Core
#include "driverleds.h" // device drivers
#include "driverbuttons.h"
#include "cmsis_os2.h" // CMSIS-RTOS
#include "inc/hw_memmap.h"
#include "driverlib/gpio.h"
#include "driverlib/sysctl.h"
#include "driverlib/systick.h"
#include "driverlib/pin_map.h"
#include "driverlib/interrupt.h"

#define BUFFER_SIZE 16

uint8_t counter = 0, index = 0;
osThreadId_t consumidor_id;
osSemaphoreId_t vazio_id, cheio_id;
uint8_t buffer[BUFFER_SIZE];
int tick, tick_anterior;

void consumidor(void *arg){
    uint8_t index_o = 0, state;
  
  while(1){
    index_o = index;
    osSemaphoreAcquire(cheio_id, osWaitForever); // há dado disponível?
    state = buffer[index_o]; // retira do buffer
    osSemaphoreRelease(vazio_id); // sinaliza um espaço a mais
    
    index_o++;
    if(index_o >= BUFFER_SIZE) // incrementa índice de retirada do buffer
      index_o = 0;
    
    LEDWrite(LED4 | LED3 | LED2 | LED1, state); // apresenta informação consumida
  } // while
/*
  uint8_t state;
  
  while(1){
    osSemaphoreAcquire(cheio_id, osWaitForever); // há dado disponível?
    state = buffer[index]; // retira do buffer
    osSemaphoreRelease(vazio_id); // sinaliza um espaço a mais
    
    index++;
    if(index >= BUFFER_SIZE) // incrementa índice de retirada do buffer
      index = 0;
    
    LEDWrite(LED4 | LED3 | LED2 | LED1, state); // apresenta informação consumida
    osDelay(50);
  }
  */
  
   // while
} // consumidor


void GPIOJ_Handler(){
  /*
  ButtonIntClear(USW1);
  if(osSemaphoreAcquire(vazio_id, 0) == osOK){
    buffer[index] = counter;
    index++;
    if(index >= BUFFER_SIZE){
      index = 0;
    }
    counter++;
    counter &= 0x0F;
    osSemaphoreRelease(cheio_id);
  }
*/ 
/*
  uint32_t status=0;
  status = GPIOIntStatus(GPIO_PORTJ_BASE,true);
  osSemaphoreAcquire(vazio_id, 0); // há espaço disponível?
  buffer[index%8] = counter; // coloca no buffer
  osSemaphoreRelease(cheio_id); // sinaliza um espaço a menos
  index++; // incrementa índice de colocação no buffer
  if(index >= BUFFER_SIZE){
    index = 0;
  }
  counter++;
  counter &= 0x0F; // produz nova informação
  GPIOIntClear(GPIO_PORTJ_BASE,status);
*/
  ButtonIntClear(USW1);
  if(osSemaphoreAcquire(vazio_id, 0) == osOK){
    buffer[index] = counter;
    
    index++;
    if(index >= BUFFER_SIZE){
      index = 0;
    }
    counter++;
    counter &= 0x0F;
    osSemaphoreRelease(cheio_id);
  }
}



void interruptInit(){
  SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOJ); // Habilita GPIO J (push-button SW1 = PJ0)
  while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOJ)); // Aguarda final da habilitação
  GPIOPinTypeGPIOInput(GPIO_PORTJ_BASE, GPIO_PIN_0); // push-buttons SW1 e SW2 como entrada
  GPIOPadConfigSet(GPIO_PORTJ_BASE, GPIO_PIN_0, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU);
  GPIOIntTypeSet(GPIO_PORTJ_BASE,GPIO_PIN_0,GPIO_FALLING_EDGE);
  GPIOIntRegisterPin(GPIO_PORTJ_BASE, 0 ,GPIOJ_Handler);
  GPIOIntEnable(GPIO_PORTJ_BASE, GPIO_INT_PIN_0);
}

void main(void){
  SystemInit();
  interruptInit();
  LEDInit(LED4 | LED3 | LED2 | LED1);
  osKernelInitialize();
  consumidor_id = osThreadNew(consumidor, NULL, NULL);
  vazio_id = osSemaphoreNew(BUFFER_SIZE, BUFFER_SIZE, NULL); // espaços disponíveis = BUFFER_SIZE
  cheio_id = osSemaphoreNew(BUFFER_SIZE, 0, NULL); // espaços ocupados = 0
  counter = 0;
  if(osKernelGetState() == osKernelReady)
    osKernelStart();

  while(1);
} // main
