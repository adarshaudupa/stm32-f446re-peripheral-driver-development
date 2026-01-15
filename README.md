# STM32F446RE Register-Level Peripheral Driver Development

A hands-on learning journey in **register-level embedded systems programming** on the STM32F446RE microcontroller.

## 🎯 Goal

Understand **how microcontrollers actually work** by writing code that directly manipulates hardware registers—not using abstraction libraries (HAL). This builds the mental models required for professional embedded systems engineering.

## 📚 What We've Built (Days 1-9)

### Week 1: GPIO Fundamentals
- **Day 1:** LED Blink using GPIO Output (PA5)
  - Enabled GPIOA clock via RCC
  - Configured PA5 as output using MODER register
  - Toggled LED using ODR (Output Data Register)
  - Mental model: GPIO pins are controlled by setting/clearing specific bits in registers

### Week 2: UART Serial Communication
- **Day 1:** UART Theory + Hardware Architecture
  - Understood asynchronous serial protocol (start bit, 8 data bits, stop bit)
  - Baud rate synchronization (9600 bps = 104 μs per bit)
  - USART2 hardware (PA2=TX, PA3=RX)

- **Day 2:** UART Transmit (TX) Implementation ✅ **Complete**
  - Configured PA2 as Alternate Function (AF7 = USART2_TX)
  - Calculated baud rate register (BRR) for 9600 baud
  - Implemented TXE (Transmit Data Register Empty) synchronization
  - Built `UART2_SendString()` function
  - Mental model: 2-stage pipeline (DR buffer → Shift Register) prevents data loss

## 🔧 Technology Stack

| Component | Version |
|-----------|---------|
| Microcontroller | STM32F446RET6 (ARM Cortex-M4, 180 MHz) |
| IDE | STM32CubeIDE v1.13.0+ |
| Compiler | GCC (arm-none-eabi) |
| Debugger | ST-LINK/V2-1 (on-board) |
| Protocol | UART @ 9600 baud |

## 📋 Key Concepts Learned

### 1. Registers and Memory-Mapped I/O
- Peripherals are controlled by writing to memory addresses
- Each register is a 32-bit control panel with individual bit switches
- Example: `GPIOA->ODR` controls GPIO output voltages

### 2. Clock System (RCC)
- Every peripheral starts disabled (saves power)
- Must enable clock before accessing peripheral registers
- Three bus systems: AHB1 (GPIO), APB1 (USART2), APB2 (USART1/6)

### 3. Bit Operations
```c
GPIOA->ODR |= (1 << 5);    // Set bit 5 (OR operation)
GPIOA->ODR &= ~(1 << 5);   // Clear bit 5 (AND NOT)
GPIOA->ODR ^= (1 << 5);    // Toggle bit 5 (XOR)
```

### 4. Alternate Functions (AF)
- GPIO pins can be controlled by peripherals (UART, SPI, I2C, etc.)
- MODER register selects mode: 01=Output, 10=Alternate Function
- AFR register selects which peripheral (AF7=USART2 on PA2)

### 5. Hardware Synchronization
- CPU is 1000x faster than UART hardware
- Must wait for TXE flag before writing next byte
- Pipeline architecture (DR + Shift Register) enables buffering

## 📂 Project Structure

```
stm32-learning/
├── Core/
│   ├── Src/
│   │   └── main.c              # Implementation code
│   └── Inc/
│       └── main.h              # Function declarations
├── Drivers/
│   └── CMSIS/                  # ARM Cortex-M4 core definitions
├── Debug/                      # Compiled binaries
└── README.md                   # This file
```

## 🚀 How to Use This Repository

### 1. Setup Hardware
- Connect STM32 Nucleo-F446RE via USB
- For UART testing: USB-to-Serial adapter on PA2 (3.3V logic)
- For signal analysis: Logic analyzer on PA2/PA5

### 2. Build and Flash
```bash
# In STM32CubeIDE
Build → Build Project
Run → Run
```

### 3. Test
- **LED:** Observe PA5 LED blinking every ~500ms
- **UART:** Connect serial terminal (9600 baud, 8N1) to see "Hello\n" output

## 🧠 Mental Models Built

### GPIO Output Pipeline
```
RCC->AHB1ENR (enable clock)
    ↓
GPIOA->MODER (set mode: output)
    ↓
GPIOA->ODR (set/clear output voltage)
    ↓
Physical pin voltage change (3.3V or 0V)
```

### UART TX Pipeline
```
Enable RCC clocks (GPIOA, USART2)
    ↓
Configure PA2 as AF7 (USART2_TX)
    ↓
Set baud rate (BRR register)
    ↓
Enable USART + TX (CR1 register)
    ↓
Wait for TXE=1
    ↓
Write byte to DR
    ↓
Hardware moves DR → Shift Register → TX pin (bit-by-bit)
    ↓
Loop back for next byte
```

## 📖 Learning Resources

### Official Documentation
- [STM32F446RE Datasheet](https://www.st.com/resource/en/datasheet/stm32f446re.pdf) - Pinouts, electrical specs
- [RM0390 Reference Manual](https://www.st.com/resource/en/reference_manual/rm0390-stm32f446xx-advanced-armbased-32bit-mcus-stmicroelectronics.pdf) - Register descriptions (1300+ pages)
- [Nucleo-64 User Manual](https://www.st.com/resource/en/user_manual/um1724-stm32-nucleo-64-development-board-stmicroelectronics.pdf) - Board layout, pinouts

### Code Comments
Each function in `main.c` has inline comments explaining:
- Why each register is configured
- What each bit does
- The order of operations (why it matters)

## 📅 Next Steps (Days 10+)

- **Day 10:** UART Receive (RX) + interrupt handling
- **Week 3:** Timer PWM output (for servo control)
- **Week 4:** ADC analog-to-digital conversion
- **Week 5:** SPI communication + external peripherals

## 💡 Core Philosophy

> "A person can understand anything if explained from first principles. We don't use magic."

- No HAL abstractions that hide complexity
- Every line of code corresponds to a hardware action
- Mental models over memorization
- Why before What

## 🔗 References

| Topic | File | Lines |
|-------|------|-------|
| GPIO Output | `main.c` | 1-50 |
| UART Initialization | `main.c` | 51-120 |
| UART Transmit | `main.c` | 121-145 |
| Main Loop | `main.c` | 146-160 |

## 📝 License

Educational use only. Reference documentation from STMicroelectronics.

---

**Last Updated:** January 16, 2026  
**Status:** ✅ Week 2 Day 2 Complete  
**Next Session:** Day 10 (UART RX + Interrupts)
