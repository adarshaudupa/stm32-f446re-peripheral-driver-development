# STM32 UART CLI - LED Control

Simple command-line interface for LED control via UART on STM32F446RE using bare-metal register programming.

## Features
- UART communication (9600 baud, 8N1)
- Command parsing and execution
- LED control commands (ON/OFF/TOGGLE/STATUS)
- Interactive help menu
- Error handling for invalid commands
- Bare-metal register-level programming (no HAL abstraction)

## Hardware
- **Board:** STM32 Nucleo-F446RE
- **MCU:** STM32F446RET6 (ARM Cortex-M4, 180MHz)
- **LED:** User LED (PA5)
- **UART:** USART2 (PA2-TX, PA3-RX via ST-Link Virtual COM Port)

## Pinout
| Peripheral | Pin | Function |
|------------|-----|----------|
| LED | PA5 | User LED (LD2) |
| USART2 TX | PA2 | UART transmit |
| USART2 RX | PA3 | UART receive |

## Software
- **IDE:** STM32CubeIDE
- **Programming Style:** Bare-metal register manipulation
- **Language:** C
- **Compiler:** ARM GCC

## Commands
```
LED ON      - Turn LED on
LED OFF     - Turn LED off
TOGGLE      - Toggle LED state
STATUS      - Check current LED state
HELP        - Show available commands
```

## Usage
1. Connect Nucleo board via USB to PC
2. Open serial terminal (PuTTY, Tera Term, minicom, screen, etc.)
3. Configure terminal settings:
   - Baud rate: 115200
   - Data bits: 8
   - Parity: None
   - Stop bits: 1
   - Flow control: None
4. Type commands and press Enter

## Build & Flash
```bash
# Clone repository
git clone https://github.com/adarshaudupa/<repo-name>.git
cd <repo-name>

# Open project in STM32CubeIDE
# File → Open Projects from File System

# Build project
# Project → Build Project (or Ctrl+B)

# Flash to board
# Run → Debug (or F11)
# Run → Run (or F5)
```

## Example Session
```
> HELP
Available commands:
  LED ON   - Turn LED on
  LED OFF  - Turn LED off
  TOGGLE   - Toggle LED state
  STATUS   - Check LED state
  HELP     - Show this help

> LED ON
LED is now ON

> STATUS
LED is currently: ON

> TOGGLE
LED toggled

> STATUS
LED is currently: OFF

> LED OFF
LED is now OFF
```

## Key Implementation Details
- **UART:** Polling-based character reception using USART2->DR register
- **GPIO:** Direct register manipulation (ODR, BSRR) for LED control
- **Command Parsing:** String comparison using strcmp for command identification
- **Clock Configuration:** System clock and peripheral clocks configured via RCC
- **No HAL:** Complete bare-metal implementation for better understanding of hardware

## Technical Highlights
- Direct peripheral register access (RCC, GPIO, USART)
- Efficient bit manipulation for GPIO control
- Robust command buffer handling
- Clean separation of UART and GPIO logic
- Professional command-line interface pattern

## Project Structure
```
├── Core/
│   ├── Src/
│   │   └── main.c          # Main application and CLI logic
│   └── Inc/
│       └── main.h          # Function prototypes and defines
├── Drivers/
│   └── CMSIS/              # ARM CMSIS headers
└── README.md
```

## Future Improvements
- [ ] Non-blocking BLINK command using hardware timers (TIM2)
- [ ] Interrupt-driven UART reception (RXNE interrupt)
- [ ] DMA for efficient UART data transfer
- [ ] State machine for complex LED patterns
- [ ] Command history with arrow key navigation
- [ ] Multiple LED control
- [ ] PWM-based LED brightness control

## Learning Outcomes
This project demonstrates:
- STM32 peripheral initialization without HAL
- UART communication protocol implementation
- GPIO register manipulation
- String parsing and command handling
- Firmware design patterns for CLI applications

## Author
**Adarsha Udupa**  
Undergraduate Engineering Student  
Focus: Embedded Systems & Firmware Engineering

## License
MIT License - Feel free to use and modify for educational purposes.

---

*Built as part of STM32 bare-metal learning journey focusing on firmware fundamentals.*
