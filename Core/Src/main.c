#include "stm32f4xx.h"
#include "uart.h"
#include <string.h>
#define CMD_BUFFER_SIZE 32

char cmd_buffer[CMD_BUFFER_SIZE];
uint8_t cmd_index = 0;


int main(void) {
    UART2_Init(9600);
    GPIOA->MODER &= ~(3<<10); //clear PA5 bit
    GPIOA->MODER |= (1<<10); //set PA5 in output mode
    UART2_SendString("---STM32 CLI---\r\n");
    UART2_SendString("Type HELP for commands\r\n\r\n");
    UART2_SendString("> ");

    while(1)
    {
    	char c = UART2_ReadChar(); //Extracts each character from the user enterred command
    	if(c=='\r') //checks if Enter(\r) is pressed
    	{
    		UART2_SendString("\r\n");

    		//Start of task
    		if(strcmp(cmd_buffer, "HELP")==0)
    		{
    		 UART2_SendString("Available commands:\r\n");
    		 UART2_SendString("  LED ON   - Turn LED on\r\n");
    		 UART2_SendString("  LED OFF  - Turn LED off\r\n");
    		 UART2_SendString("  TOGGLE   - Toggle LED state\r\n");
    		 UART2_SendString("  STATUS   - Check LED state\r\n");
    		 UART2_SendString("  HELP     - Show this help\r\n");
    		}
    		 else if(strcmp(cmd_buffer, "LED ON")==0)
			 {
			  GPIOA->ODR |= (1<<5);
			  UART2_SendString("LED turned ON\r\n");
			 }
			 else if(strcmp(cmd_buffer,"LED OFF")==0)
			 {
			  GPIOA->ODR &= ~(1<<5);
			  UART2_SendString("LED turned OFF\r\n");
			 }
			 else if(strcmp(cmd_buffer,"TOGGLE")==0)
			 {
			  GPIOA->ODR ^= (1<<5);
			  UART2_SendString("LED toggled\r\n");
			 }
			 else if(strcmp(cmd_buffer,"STATUS")==0)
			 {
				 if (GPIOA->ODR & (1 << 5)) //if ODR is 1 output is 1 so it means LED is ON at that instant
				 {
				  UART2_SendString("LED is ON\r\n");
				 }
				 else
				 {
				  UART2_SendString("LED is OFF\r\n");
				 }
			 }
			 else if (cmd_index > 0)
			 {  // Non-empty unknown command
			  UART2_SendString("Unknown command: ");
			  UART2_SendString(cmd_buffer);
			  UART2_SendString("\r\nType HELP for commands\r\n");
			 }
    		//Emd of task
    		//clear the command buffer
    		cmd_index = 0;
    		UART2_SendString("> ");
    	}
    	else if(c==127 || c==8) //128 is ASCII DEL button and ASCII 8 is Backspace
    	{
    	 if(cmd_index>0)
    	    cmd_index--;
    	   UART2_SendString("\b \b"); //To erase the character on screen
    	}
    	else if(cmd_index < CMD_BUFFER_SIZE-1) //normal characters
    	{
    		cmd_buffer[cmd_index++]=c;
    		cmd_buffer[cmd_index] = '\0';
    		UART2_SendChar(c); //echo the character typed
    	}
    }

}
