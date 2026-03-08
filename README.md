# STM32F446RE Peripheral Driver Development

**Bare-metal register-level peripheral drivers for STM32F446RE Nucleo board**

[![License: MIT](https://img.shields.io/badge/License-MIT-blue.svg)](LICENSE)

---

##  Overview

This repository is a **driver laboratory** for learning and developing bare-metal peripheral drivers for the STM32F446RE microcontroller. Each peripheral (GPIO, UART, TIM, ADC, I2C) is implemented from first principles using direct register access—no HAL, no CubeMX-generated code, just reference manual and determination.

**Purpose:** Build reusable, well-understood driver modules and timing intuition for future application projects (IMU-based pothole detection, RC car firmware, accident detection systems).

**Philosophy:** 
- Understand hardware at the lowest level before abstracting
- Every line of code maps to a specific register bit in RM0390
- Clean, modular architecture that scales to production firmware

---

##  Current Implementation Status

###  Implemented

| Peripheral | Status | Features |
|------------|--------|----------|
| **GPIO** |  Complete | Push-pull/open-drain output, pull-up/down input, BSRR atomic writes |
| **UART2** |  Complete | Interrupt-driven RX with circular buffer, polling TX, manual BRR calculation |
| **TIM2** |  Complete | Periodic interrupts (1 Hz), runtime ARR modification, LED state machine |
| **ADC1** |  Complete | Single-channel polling, software-triggered, 12-bit resolution |

###  In Progress

| Peripheral | Status | Target |
|------------|--------|--------|
| **ADC1 + DMA** |  Planned | Circular buffer, timer-triggered, ping-pong processing |
| **I2C1** |  Next | 400 kHz fast mode, LSM6DS3TR-C IMU communication |
| **Clock Tree** |  Next | Dynamic frequency calculation helpers (`get_apb1_freq_hz()`) |

###  Future Work

- SPI1 (external flash/SD card)
- DMA for UART TX
- External interrupts (EXTI) for button debounce
- Low-power modes (STOP/STANDBY)

---

##  Hardware Requirements

| Component | Specification |
|-----------|--------------|
| **Board** | STM32 Nucleo-F446RE |
| **MCU** | STM32F446RET6 (ARM Cortex-M4 @ 180 MHz max, 512 KB Flash, 128 KB SRAM) |
| **Onboard LED** | PA5 (LD2) |
| **User Button** | PC13 (B1, active-low with external pull-up) |
| **ADC Input** | PA0 (ADC1_IN0, test with potentiometer or jumper wires) |
| **I2C (Planned)** | PB8 (SCL), PB9 (SDA) for LSM6DS3TR-C IMU |

**External Hardware (Optional):**
- Potentiometer on PA0 for ADC testing
- LSM6DS3TR-C breakout board for I2C testing
- Logic analyzer for protocol debugging

---

##  Quick Start

### Prerequisites

- **IDE:** STM32CubeIDE (recommended) or ARM GCC toolchain
- **Debugger:** ST-LINK/V2-1 (integrated on Nucleo)
- **Terminal:** PuTTY, minicom, or `screen` for UART debugging
- **Documentation:** [STM32F446xx Reference Manual (RM0390)](https://www.st.com/resource/en/reference_manual/rm0390-stm32f446xx-advanced-armbased-32bit-mcus-stmicroelectronics.pdf)

### Build and Flash

```bash
# Clone repository
git clone https://github.com/adarshaudupa/stm32-f446re-peripheral-driver-development.git
cd stm32-f446re-peripheral-driver-development

# Option 1: STM32CubeIDE
# File → Import → Existing Projects into Workspace
# Select directory → Build (Ctrl+B) → Debug (F11)

# Option 2: Command Line (if using Makefile)
make clean && make
st-flash write build/main.bin 0x8000000
```

---

##  Project Structure

```
stm32-f446re-peripheral-driver-development/
├── Core/
│   ├── Inc/
│   │   ├── main.h
│   │   ├── gpio.h          # GPIO driver header
│   │   ├── uart2.h         # USART2 driver header
│   │   ├── tim2.h          # TIM2 driver header
│   │   ├── adc1.h          # ADC1 driver header
│   │   └── i2c1.h          # I2C1 driver (planned)
│   └── Src/
│       ├── main.c          # Test applications and integration
│       ├── gpio.c          # GPIO implementation
│       ├── uart2.c         # USART2 implementation + ISR
│       ├── tim2.c          # TIM2 implementation + ISR
│       ├── adc1.c          # ADC1 implementation
│       └── syscalls.c      # Newlib stubs
├── Drivers/
│   └── CMSIS/              # ARM CMSIS headers (vendor, unchanged)
├── Debug/                  # Build artifacts (gitignored)
├── .gitignore
├── LICENSE
└── README.md
```

**Design Principle:** Each peripheral lives in its own `.c/.h` pair. `main.c` only initializes drivers and runs test applications. Clean separation for easy reuse in future projects.

---

##  Technical Deep Dive

### 1. GPIO Driver

**Capabilities:**
- Configure pins as input (floating, pull-up, pull-down) or output (push-pull, open-drain)
- Atomic bit manipulation using BSRR (avoid read-modify-write hazards)
- LED control (PA5) and button reading (PC13)

**Key Registers:**
```c
// Example: Configure PA5 as push-pull output
GPIOA->MODER &= ~(3 << 10);   // Clear mode bits [11:10]
GPIOA->MODER |= (1 << 10);    // Set to output mode (0b01)
GPIOA->OTYPER &= ~(1 << 5);   // Push-pull (default)

// Atomic LED toggle using BSRR
GPIOA->ODR ^= (1 << 5);       // Read-modify-write (avoid in ISRs)
// OR safer:
if (GPIOA->ODR & (1 << 5))
    GPIOA->BSRR = (1 << 21);  // Reset bit [BR5]
else
    GPIOA->BSRR = (1 << 5);   // Set bit [BS5]
```

**Reference:** RM0390 Section 8 (GPIO)

---

### 2. UART2 Driver

**Capabilities:**
- Interrupt-driven RX with 256-byte circular buffer
- Blocking TX (optimized for debug logging)
- Manual baud rate calculation from APB1 clock
- Echo, backspace handling for CLI integration

**Initialization Sequence:**
```c
// 1. Enable clocks
RCC->AHB1ENR |= (1 << 0);   // GPIOA
RCC->APB1ENR |= (1 << 17);  // USART2

// 2. Configure PA2 (TX), PA3 (RX) as AF7
GPIOA->MODER &= ~((3 << 4) | (3 << 6));
GPIOA->MODER |= (2 << 4) | (2 << 6);     // AF mode
GPIOA->AFR[0] &= ~((0xF << 8) | (0xF << 12));
GPIOA->AFR[0] |= (7 << 8) | (7 << 12);   // AF7 = USART2

// 3. Configure baud rate
// BRR = fPCLK / (16 × baud)
// APB1 = 16 MHz, baud = 115200 → BRR ≈ 8.68 → 0x8B
USART2->BRR = 0x8B;

// 4. Enable USART
USART2->CR1 |= (1 << 13) | (1 << 3) | (1 << 2);  // UE, TE, RE
USART2->CR1 |= (1 << 5);                          // RXNEIE

// 5. Enable NVIC
NVIC_EnableIRQ(USART2_IRQn);
```

**ISR Pattern:**
```c
void USART2_IRQHandler(void) {
    if (USART2->SR & (1 << 5)) {  // RXNE set?
        char byte = USART2->DR;   // Reading DR clears RXNE
        rx_buffer[rx_head++] = byte;
        rx_head %= RX_BUFFER_SIZE;
    }
}
```

**Reference:** RM0390 Section 30 (USART)

---

### 3. TIM2 Driver

**Capabilities:**
- Generate periodic interrupts (configurable frequency via PSC/ARR)
- Runtime frequency modification (change ARR on-the-fly)
- LED state machine integration (auto-blink mode)

**Timer Frequency Calculation:**
```
Update frequency = Timer_Clock / ((PSC + 1) × (ARR + 1))

Example for 1 Hz with APB1 = 84 MHz (after PLL):
PSC = 8399, ARR = 9999
f_update = 84000000 / (8400 × 10000) = 1 Hz
```

**Initialization:**
```c
// 1. Enable TIM2 clock (APB1)
RCC->APB1ENR |= (1 << 0);

// 2. Configure prescaler and auto-reload
TIM2->PSC = 8399;   // (PSC + 1) = 8400
TIM2->ARR = 9999;   // (ARR + 1) = 10000

// 3. Enable update interrupt
TIM2->DIER |= (1 << 0);  // UIE
NVIC_EnableIRQ(TIM2_IRQn);

// 4. Start timer
TIM2->CR1 |= (1 << 0);   // CEN
```

**ISR:**
```c
void TIM2_IRQHandler(void) {
    if (TIM2->SR & (1 << 0)) {  // UIF set?
        TIM2->SR &= ~(1 << 0);  // Clear UIF (mandatory!)
        GPIOA->ODR ^= (1 << 5); // Toggle LED
    }
}
```

**Reference:** RM0390 Section 18 (General-purpose timers)

---

### 4. ADC1 Driver

**Capabilities:**
- Single-channel conversion (PA0 = ADC1_IN0)
- 12-bit resolution (0–4095 for 0–3.3V)
- Software-triggered, polling-based (EOC flag)
- Voltage-to-LED threshold logic

**Initialization:**
```c
// 1. Enable clocks
RCC->AHB1ENR |= (1 << 0);   // GPIOA
RCC->APB2ENR |= (1 << 8);   // ADC1

// 2. Configure PA0 as analog input
GPIOA->MODER |= (3 << 0);   // Analog mode (0b11)

// 3. Configure ADC
ADC1->CR2 &= ~(1 << 11);    // Right-aligned data
ADC1->SQR1 &= ~(0xF << 20); // 1 conversion in sequence
ADC1->SQR3 &= ~(0x1F << 0); // Channel 0 in SQ1
ADC1->SMPR2 |= (7 << 0);    // 480 cycles sampling time

// 4. Enable ADC
ADC1->CR2 |= (1 << 0);      // ADON
for (volatile int i = 0; i < 1000; i++);  // Stabilization delay
```

**Conversion Loop:**
```c
while (1) {
    ADC1->CR2 |= (1 << 30);         // SWSTART
    while (!(ADC1->SR & (1 << 1))); // Wait for EOC
    uint16_t adc_value = ADC1->DR;  // Read result (clears EOC)
    
    // Use value (print, threshold check, etc.)
    if (adc_value > 2000) {
        GPIOA->ODR |= (1 << 5);     // LED ON
    } else {
        GPIOA->ODR &= ~(1 << 5);    // LED OFF
    }
}
```

**Reference:** RM0390 Section 13 (ADC)

---

##  Test Applications

### Test 1: TIM2 1 Hz Blink
**File:** `main.c` (default test)

**Behavior:**
- TIM2 generates 1 Hz update interrupt
- ISR toggles PA5 LED (on for 1s, off for 1s)
- No busy-wait loops, CPU is free for other tasks

**Purpose:** Verify timer configuration, PSC/ARR calculation, interrupt handling, ISR flag clearing

---

### Test 2: Button-Controlled Timer Speed
**File:** `main.c` (variant)

**Behavior:**
- Main loop polls PC13 user button
- Button pressed → TIM2->ARR = 4999 (2 Hz blink)
- Button released → TIM2->ARR = 9999 (1 Hz blink)

**Purpose:** Demonstrate runtime timer reconfiguration, input reading, blocking polling trade-offs

---

### Test 3: ADC Streaming Over UART
**File:** `main.c` (variant)

**Behavior:**
- Periodic ADC conversion on PA0
- Convert `uint16_t` to ASCII string
- Print value via UART2
- LED ON if value > 2000, else OFF

**Hardware Test:**
```
PA0 floating       → Random flickering values (noise)
PA0 to GND         → Values near 0, LED OFF
PA0 to 3.3V        → Values near 4095, LED ON
PA0 to pot wiper   → Values change with pot, LED toggles at threshold
```

**Purpose:** Verify ADC calibration, UART printf integration, voltage mapping, threshold logic

---

##  Learning Outcomes

This repository demonstrates:

### Hardware Understanding
- Clock tree architecture (AHB, APB1, APB2 buses)
- Peripheral clock enable sequencing via RCC
- GPIO alternate function mapping (AFRL/AFRH registers)
- Prescaler and auto-reload calculations for timers
- ADC sampling time vs. conversion accuracy trade-offs

### Firmware Skills
- Register-level peripheral initialization
- NVIC interrupt priority and enable
- ISR design patterns (flag clearing, volatility, atomicity)
- Circular buffer implementation for UART RX
- Avoiding read-modify-write hazards (BSRR vs. ODR)

### Professional Practices
- Modular driver architecture (separation of concerns)
- CMSIS-compliant register access
- Version control for embedded projects
- Readable, maintainable low-level code
- Reference manual navigation and bit-field interpretation

---

##  Roadmap

### Short-Term (Next 2 Weeks)
- [ ] Implement dynamic clock frequency helpers (`get_apb1_freq_hz()`)
- [ ] Refactor UART/TIM/ADC to use clock helpers instead of hardcoded values
- [ ] I2C1 driver bring-up at 400 kHz
- [ ] Read WHO_AM_I register from LSM6DS3TR-C IMU

### Medium-Term (Next Month)
- [ ] ADC1 + DMA circular buffer with ping-pong processing
- [ ] Timer-triggered ADC (TIM2_TRGO → ADC1)
- [ ] Basic accelerometer data acquisition and UART streaming
- [ ] Simple impact detection algorithm (threshold + windowed variance)

### Long-Term (Next 3 Months)
- [ ] SPI driver for external flash/SD card logging
- [ ] DMA for UART TX (non-blocking high-throughput logging)
- [ ] State machine framework for application logic
- [ ] Integration into RC car / pothole detection project
- [ ] FreeRTOS port with task-based architecture

---

##  Known Issues

| Issue | Impact | Status |
|-------|--------|--------|
| Clock frequency hardcoded to 16 MHz | Breaks if PLL enabled | Fix in progress |
| ADC sampling time not optimized | Slower conversions than necessary | Low priority |
| No DMA for UART TX | Blocking prints freeze main loop | Planned |
| I2C driver missing | Can't talk to sensors yet | Next milestone |

---

##  References

### Official Documentation
- [STM32F446xx Reference Manual (RM0390)](https://www.st.com/resource/en/reference_manual/rm0390-stm32f446xx-advanced-armbased-32bit-mcus-stmicroelectronics.pdf) - **Primary resource**
- [STM32F446RE Datasheet](https://www.st.com/resource/en/datasheet/stm32f446re.pdf)
- [STM32 Nucleo-64 User Manual (UM1724)](https://www.st.com/resource/en/user_manual/um1724-stm32-nucleo64-boards-mb1136-stmicroelectronics.pdf)
- [ARM Cortex-M4 Technical Reference Manual](https://developer.arm.com/documentation/100166/0001)

### Learning Resources
- *Mastering STM32* by Carmine Noviello
- Fastbit Embedded Brain Academy (YouTube)
- STM32 community forums and Discord servers

---

##  Contributing

This is a personal learning repository, but feedback and improvements are welcome:

1. Fork the repository
2. Create a feature branch (`git checkout -b feature/driver-xyz`)
3. Commit with descriptive messages
4. Push and open a Pull Request

**Areas for contribution:**
- Additional peripheral drivers (SPI, CAN, USB)
- Bug fixes or optimization suggestions
- Documentation improvements
- Test application ideas

---

##  License

MIT License - see [LICENSE](LICENSE) file for details.

**TL;DR:** Free to use, modify, and distribute. Keep the copyright notice.

---

##  Author

**Adarsha Udupa Baikady**  
Undergraduate | Electronics & Instrumentation Engineering  
Focus: Embedded Systems & Firmware Development

- GitHub: [@adarshaudupa](https://github.com/adarshaudupa)
- LinkedIn: [adarsha-udupa-baikady](https://www.linkedin.com/in/adarsha-udupa-baikady-327a54219)
- Email: adarsha8505@gmail.com

---

##  Acknowledgments

This project exists because of:
- Direct mentorship and code reviews from embedded engineers
- STM32 reference manual authors (the real MVPs)
- The open-source embedded community
- Countless late-night debugging sessions with logic analyzers

---

**No HAL, no shortcuts—just registers, determination, and a reference manual.**