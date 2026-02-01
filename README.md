# STM32 UART CLI

Bare-metal command-line interface for STM32F446RE with interrupt-driven UART communication. Built without HAL, using direct register manipulation.

## 🎯 Features

- **Register-level UART configuration** - Manual BRR calculation, CR1/SR/DR register access
- **Interrupt-driven RX** - 256-byte circular buffer with NVIC-configured ISR
- **Command parser** - Space-delimited tokenization with input validation
- **Non-blocking architecture** - Main loop remains responsive during command execution

### Supported Commands

```
STATUS          - Display system information
LED ON          - Turn on GPIO PA5 (onboard LED)
LED OFF         - Turn off GPIO PA5
LED TOGGLE      - Toggle LED state
ECHO <text>     - Echo back user input
```

## 🔧 Hardware Setup

**Board:** STM32 Nucleo F446RE  
**MCU:** STM32F446RET6 (ARM Cortex-M4, 180 MHz max)

**Pin Connections:**
| Peripheral | Pin  | Function |
|------------|------|----------|
| USART2 TX  | PA2  | Serial output (ST-Link VCP) |
| USART2 RX  | PA3  | Serial input (ST-Link VCP) |
| LED        | PA5  | Onboard green LED |

**No external hardware required** - uses ST-Link's built-in USB-to-serial converter.

## 📋 Technical Details

### UART Configuration

```c
// Clock: APB1 = 16 MHz (HSI, no PLL)
// Baud rate: 115200
// BRR calculation: PCLK1 / baud_rate = 16000000 / 115200 = 139 (0x8B)

RCC->APB1ENR |= RCC_APB1ENR_USART2EN;  // Enable USART2 clock
USART2->BRR = 139;                      // Set baud rate
USART2->CR1 |= USART_CR1_TE | USART_CR1_RE | USART_CR1_RXNEIE;  // TX, RX, RX interrupt
USART2->CR1 |= USART_CR1_UE;            // Enable USART
```

### Interrupt Handling

- **ISR:** `USART2_IRQHandler` triggered on RXNE flag
- **Priority:** NVIC priority 1 (higher than systick)
- **Buffer:** 256-byte circular buffer with head/tail pointers
- **Overflow protection:** Wraps around when full (oldest data lost)

### Memory Usage

| Component | Size | Type |
|-----------|------|------|
| RX Buffer | 256 bytes | Circular buffer (static) |
| TX Buffer | None (polling-based) | - |
| Command Buffer | 128 bytes | Linear buffer (static) |
| Total RAM | ~400 bytes | BSS segment |

## 🚀 Getting Started

### Prerequisites

- **Software:** STM32CubeIDE or ARM GCC toolchain + OpenOCD
- **Hardware:** STM32 Nucleo F446RE board
- **Terminal:** PuTTY, minicom, screen, or any serial terminal

### Build & Flash

**Option 1: STM32CubeIDE**
```
1. File > Import > Existing Projects into Workspace
2. Select this repository folder
3. Project > Build All
4. Run > Debug (flashes automatically)
```

**Option 2: Command Line (if using Makefile)**
```bash
make clean
make
st-flash write build/main.bin 0x8000000
```

### Usage

```bash
# Connect via serial terminal
screen /dev/ttyACM0 115200  # Linux/Mac
# Or use PuTTY on Windows: COM3, 115200 baud

# Example session:
> STATUS
STM32F446RE UART CLI
SYSCLK: 16 MHz
UART: 115200 baud

> LED ON
LED: ON

> ECHO Hello Embedded
Hello Embedded

> LED TOGGLE
LED: OFF
```

## 📚 What I Learned

**Register-Level Programming:**
- Configuring GPIO alternate functions (AFR registers)
- UART baud rate calculation from APB clock
- Reading STM32 reference manual (RM0390) for register details

**Interrupt Handling:**
- NVIC priority configuration
- ISR flag clearing (critical to prevent re-entry)
- Volatile variables for ISR-to-main communication

**Data Structures:**
- Circular buffer implementation (modulo arithmetic)
- String parsing without stdlib (custom tokenizer)

**Debugging:**
- Using GDB with OpenOCD for live debugging
- Understanding why missing `volatile` causes bugs
- ST-Link SWD vs UART debugging trade-offs

## 🐛 Known Issues

- [ ] BRR calculation was hardcoded in early commits (fixed)
- [ ] TX is blocking (interrupt-driven TX planned for v2)
- [ ] No command history (up/down arrow navigation)
- [ ] No backspace handling in echo command

## 🗺️ Roadmap

- [ ] **Week 1:** Add TIM2 timer interrupt for non-blocking BLINK command
- [ ] **Week 2-3:** I2C driver + BMP280 sensor integration
- [ ] **Week 4:** State machine for sensor polling
- [ ] **Week 5:** DMA-based TX for high-throughput logging

## 📖 Documentation

See `docs/` folder for detailed learning notes and register explanations.

## 📄 License

MIT License - See [LICENSE](LICENSE) file

## 🙏 References

- [STM32F446xx Reference Manual (RM0390)](https://www.st.com/resource/en/reference_manual/rm0390-stm32f446xx-advanced-armbased-32bit-mcus-stmicroelectronics.pdf)
- [STM32F446xC/E Datasheet](https://www.st.com/resource/en/datasheet/stm32f446re.pdf)
- Fastbit Embedded Brain Academy (YouTube)

---

**Built from scratch with no HAL.** Part of my embedded systems learning journey.

*Questions? Open an issue or reach out via [LinkedIn](https://linkedin.com/in/adarsha-udupa-baikady-327a54219)*
