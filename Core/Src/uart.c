/*
 * uart.c
 *
 * UART driver implementation for STM32F446RE
 */

#include "uart.h"
#include "stm32f4xx.h"  // Register definitions

// ============================================================================
// CIRCULAR BUFFER FOR RX DATA
// ============================================================================

#define RX_BUFFER_SIZE 64

volatile char rx_buffer[RX_BUFFER_SIZE];
volatile uint8_t rx_head = 0;  // ISR writes here
volatile uint8_t rx_tail = 0;  // Main reads here

// ============================================================================
// INITIALIZATION
// ============================================================================

void UART2_Init(uint32_t baudrate) {
	// ========================================================================
	    // STEP 1: ENABLE CLOCKS
	    // ========================================================================
	// Enable GPIOA clock (AHB1 bus, bit 0)
	RCC->AHB1ENR |= (1<<0);
	 // Enable USART2 clock (APB1 bus, bit 17)
	RCC->APB1ENR |= (1<<17);
    // ========================================================================
    // STEP 2: CONFIGURE GPIO PINS (PA2=TX, PA3=RX)
    // ========================================================================
    // PA2 (TX) and PA3 (RX) → Alternate Function mode
    // Clear mode bits for PA2 [5:4] and PA3 [7:6]
	GPIOA->MODER &= ~((3<<4) | (3<<6));
	// Set PA2 and PA3 to AF mode (0b10)
	GPIOA->MODER |= (2<<4) | (2<<6);
	// ========================================================================
	// STEP 3: SELECT ALTERNATE FUNCTION (AF7 = USART2)
	// ========================================================================
	// Clear AF bits for PA2 [11:8] and PA3 [15:12]
	GPIOA->AFR[0] &= ~((0xF<<8) | (0xF<<12));  //[0] because AFRL is index 0 and AFRH is index 1
	// Set PA2 and PA3 to AF7 (USART2)
	GPIOA->AFR[0] |= (7<<8) | (7<<12); 		   //AFR is a 2 element array not a single register
    // ========================================================================
    // STEP 4: CONFIGURE BAUD RATE
    // ========================================================================
	// Formula: BRR = f_PCLK / (16 * baudrate)
	// APB1 clock = 16 MHz (default after reset)
	// For 9600: BRR = 16000000 / (16 * 9600) = 104.166
	uint32_t brr_value = 16000000 / (16 * baudrate);
	USART2->BRR = 0x683;
	// ========================================================================
	// STEP 5: ENABLE USART, TX, AND RX
	// ========================================================================
	// CR1 bit 13: UE (USART Enable)
	// CR1 bit 3:  TE (Transmitter Enable)
	// CR1 bit 2:  RE (Receiver Enable) ← NEW for RX

	// ========================================================================
	// STEP 5: ENABLE USART, TX, AND RX
	// ========================================================================

	USART2->CR1 = (1 << 13) | (1 << 3) | (1 << 2);

	 // Enable RXNE interrupt
	    USART2->CR1 |= (1 << 5);

	    // Enable USART2 interrupt in NVIC
	    NVIC->ISER[1] |= (1 << 6);

	    // Flush any garbage data
	    for (volatile int i = 0; i < 1000; i++);
	    if (USART2->SR & (1 << 5)) {
	        volatile uint32_t temp = USART2->DR;
	        (void)temp;
}
}

// ============================================================================
// TRANSMIT FUNCTIONS
// ============================================================================

void UART2_SendChar(char ch) {
    // ========================================================================
    // WAIT FOR TXE FLAG (Transmit Data Register Empty)
    // ========================================================================

    // SR bit 7: TXE = 1 when DR is empty (safe to write)
    // Blocking wait: spin here until TXE = 1
    while (!(USART2->SR & (1 << 7)));

    // ========================================================================
    // WRITE CHARACTER TO DATA REGISTER
    // ========================================================================

    // Writing to DR triggers hardware transmission
    // Hardware will:
    //   1. Move byte from DR to shift register
    //   2. Clear TXE flag (DR now empty)
    //   3. Serialize bits to TX pin (PA2)
    //   4. Set TXE flag again when done (ready for next byte)
    USART2->DR = ch;
}


void UART2_SendString(char *str) {
    // ========================================================================
    // TRANSMIT EACH CHARACTER UNTIL NULL TERMINATOR
    // ========================================================================

    // Loop through string until '\0'
    while (*str) {
        UART2_SendChar(*str);  // Send current character
        str++;                 // Move to next character
    }
}
//char msg[] = "Hello";
// Memory: ['H']['e']['l']['l']['o']['\0']
//           ↑
//          str points here initially
// *str = 'H' → send it
// str++ → now points to 'e'
// Loop until *str = '\0' (null terminator)


// ============================================================================
// RECEIVE FUNCTIONS
// ============================================================================

char UART2_ReadChar(void) {
    // Wait for data (buffer not empty)
    while (rx_head == rx_tail);

    // Read from buffer
    char c = rx_buffer[rx_tail];

    // Move tail forward
    rx_tail++;

    // Wrap around if needed
    if (rx_tail >= RX_BUFFER_SIZE) {
        rx_tail = 0;
    }

    return c;
}



// DR is 9 bits: [8][7][6][5][4][3][2][1][0]
//                ↑
//           Bit 8 = parity (we don't use it)

// & 0xFF masks to only bits [7:0]
/*
 * 0xFF = 0b11111111 (8 bits)

Example:
DR = 0b100101010 (9 bits, 'A' with parity bit set)
DR & 0xFF = 0b001000001 = 0x41 = 'A' ✓
*/


uint8_t UART2_DataAvailable(void) {
    return (rx_head != rx_tail);
}

// ============================================================================
// INTERRUPT SERVICE ROUTINE
// ============================================================================

void USART2_IRQHandler(void) {
    // Check if RXNE (byte received)
    if (USART2->SR & (1 << 5)) {
        // Read byte (clears RXNE flag)
        char received_byte = USART2->DR;

        // Store in buffer
        rx_buffer[rx_head] = received_byte;

        // Move head forward
        rx_head++;

        // Wrap around if needed
        if (rx_head >= RX_BUFFER_SIZE) {
            rx_head = 0;
        }
    }
}
