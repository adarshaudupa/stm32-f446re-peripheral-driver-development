# STM32 F446RE Learning Journey

## Week 1: GPIO Fundamentals

### Day 1-2: Pull-up Resistors and Input Configuration

**Goal**: Understand how pull-up resistors prevent floating GPIO inputs.

#### Hardware Setup
- **Pin used**: PC13 (Blue Button on Nucleo board)
- **Hardware pull-up**: 4.7kΩ external resistor on board
- **Software pull-up**: Disabled (set to GPIO_NOPULL in code)

#### What I Observed
- **Button not pressed**: PC13 reads HIGH (1) — pulled to VDD by external 4.7kΩ resistor
- **Button pressed**: PC13 reads LOW (0) — button connects pin directly to GND


#### Code Explanation
The main loop reads PC13 every 500ms and prints the state over UART.

#### Why Pull-ups Matter
Without a pull-up, the pin floats and reads random values. The external 4.7kΩ resistor keeps the pin stable at HIGH when the button isn't pressed.

#### Key Takeaway
Pull-up resistors prevent floating inputs by connecting the pin to VDD through a resistor. This ensures stable logic levels.

### Day 3-4: LED Control with Push-Pull Output

**Goal**: Control PA5 (green LED) as push-pull output and understand GPIO output registers.

#### Hardware Setup
- **Pin used**: PA5 (LD2 - green LED on Nucleo board)
- **Configuration**: GPIO Output, Push-Pull mode
- **LED circuit**: PA5 → LED → current limiting resistor → GND

#### Code
```c
while (1)
{
  // Turn LED ON - set PA5 to HIGH (3.3V)
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_SET);
  HAL_Delay(500);

  // Turn LED OFF - set PA5 to LOW (0V)
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET);
  HAL_Delay(500);
}
```
### Day 5-6: Open-Drain Output (Register-Level)

**Goal**: Understand open-drain GPIO configuration and validate with register-level debugging.

#### Hardware Setup
- **Pin**: PA9 (open-drain output)
- **External pull-up**: 330Ω to 3.3V (temporary, 2.2kΩ recommended for production)
- **Validation**: Digital multimeter + UART debug output

#### Register Configuration
```c
// Enable GPIOA clock (required before any GPIO access)
RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;

// Set PA9 to output mode
GPIOA->MODER &= ~(0x3 << 18);  // Clear bits [19:18]
GPIOA->MODER |= (0x1 << 18);   // Set to 01 (output)

// Set PA9 to open-drain
GPIOA->OTYPER |= (1 << 9);     // 1 = open-drain, 0 = push-pull

// Disable internal pull-up/down
GPIOA->PUPDR &= ~(0x3 << 18);  // 00 = no pull
```
