# STM32F446RE Peripheral Driver Development

Bare-metal register-level drivers for STM32F446RE Nucleo board. Built to understand peripheral timing, interrupt-driven architecture, and multi-peripheral integration without HAL abstractions.

---

## Overview

This repository contains **from-scratch drivers** for STM32F446RE peripherals, written directly against RM0390 register definitions. Focus areas:

- **Interrupt-driven UART** (TX/RX with circular buffer)
- **GPIO** (LED control, button input)
- **TIM2** (periodic interrupts for task scheduling)
- **ADC1** (single-channel analog reads)
- **I2C1** (master mode, single/multi-byte transactions)

Primary goal: **Learn how microcontroller peripherals work at the hardware level**, not just call library functions.

---

## Features

### UART2 (Interrupt-Driven)

- **TX**: Blocking write with TXE flag polling
- **RX**: Interrupt-driven with 64-byte circular buffer
- **ISR Design**: Minimal latency—ISR only writes to buffer and sets flag; main loop processes data
- **Configuration**: 9600 baud (runtime-configurable via BRR calculation from APB1 clock)

**Key Learning**: Understanding `RXNE` interrupt timing, SR flag clearing sequence, and why ISRs must be fast.

### GPIO

- **LED Control**: PA5 (onboard LED) with ON/OFF/TOGGLE functions
- **Button Input**: PC13 (onboard user button) with polling
- **Mental Model**: Direct register manipulation (MODER, ODR, IDR) instead of HAL wrappers

### TIM2 (Non-Blocking Timer)

- **Configuration**: PSC/ARR calculated from APB1 clock for desired update frequency
- **Usage Pattern**: Timer ISR sets flag → main loop checks flag → task executes
- **Why This Matters**: Moves from blocking `for` loops to event-driven architecture

**Implementation Note**: Initial version assumed 16 MHz APB1; corrected after discovering actual clock = 45 MHz (see "Critical Bugs Fixed" below).

### ADC1 (Analog-to-Digital Converter)

- **Single-Channel Polling**: PA0 → 12-bit value (0–4095)
- **Integration**: ADC reads triggered by TIM2 flag for periodic sampling
- **Use Case**: Continuous analog monitoring with UART streaming

### I2C1 (Master Mode)

- **Physical Setup**: PB8 (SCL), PB9 (SDA), configured as open-drain with internal pull-ups
- **Transactions Implemented**:
  - `I2C1_ReadRegister()`: Single-byte read (START → addr+W → reg → repeated START → addr+R → NACK → STOP)
  - `I2C1_ReadMulti()`: Multi-byte read with correct ACK/NACK sequencing
- **Target Device**: BME280 (I2C address 0x76, WHO_AM_I register 0xD0)

**Status**: Basic transaction sequence works; debugging ongoing for sensor-specific timing issues.

---

## Architecture Decisions

### 1. Interrupt-Driven vs. Polling

| Peripheral | Strategy | Reason |
|-----------|----------|--------|
| UART RX | **Interrupt** | CPU-free operation; data captured immediately |
| UART TX | **Polling** | Simple; TX is fast enough to block briefly |
| TIM2 | **Interrupt** | Generates periodic events without CPU burn |
| ADC | **Polling** | Single conversion is fast; no need for ISR overhead |
| I2C | **Polling** | Complex state machine; easier to debug with explicit waits |

### 2. Circular Buffer for UART RX

- **Size**: 64 bytes (tuned for expected message lengths)
- **ISR Role**: Write byte to buffer, increment head, return immediately
- **Main Loop Role**: Check tail != head, read byte, increment tail
- **Overflow Handling**: Currently wraps silently (no overflow detection—acknowledged limitation)

### 3. Clock Awareness

**Critical Issue Discovered**: Peripheral timing registers (BRR, PSC/ARR, I2C CCR) depend on **actual bus clock**, not assumed values.

**Solution in Progress**:
- Implement `get_apb1_freq_hz()` to read `RCC->CFGR` + prescaler tables at runtime
- Use computed frequency for all peripheral init functions
- No more hard-coded "16 MHz" assumptions

---

## Project Structure

```
stm32-f446re-peripheral-driver-development/
├── Src/
│   ├── main.c          # Application logic (UART echo, ADC print, I2C test)
│   ├── uart2.c         # UART driver (TX/RX + circular buffer)
│   ├── gpio.c          # LED + button control
│   ├── tim2.c          # Timer init + ISR
│   ├── adc1.c          # ADC init + read function
│   └── i2c1.c          # I2C master driver
├── Inc/
│   ├── uart2.h
│   ├── gpio.h
│   ├── tim2.h
│   ├── adc1.h
│   └── i2c1.h
└── README.md
```

**Naming Convention**: Peripheral names match hardware instance (UART2, TIM2, I2C1) for clarity when debugging against reference manual.

---

## Technical Highlights

### 1. Register-Level Configuration

**Example: UART2 GPIO Alternate Function Setup**

```c
// PA2 (TX), PA3 (RX) → AF mode
GPIOA->MODER &= ~((3<<4) | (3<<6));
GPIOA->MODER |= (2<<4) | (2<<6);

// Select AF7 (USART2)
GPIOA->AFR[0] &= ~((0xF<<8) | (0xF<<12));
GPIOA->AFR[0] |= (7<<8) | (7<<12);
```

**Why This Matters**: Understanding mode registers, alternate function routing, and pin multiplexing is **essential for firmware roles** where HAL isn't available or suitable.

### 2. I2C Transaction Sequencing

**Single-Byte Register Read Pattern**:

1. START → wait `SB` flag
2. Send slave address (write) → wait `ADDR` → clear `ADDR` by reading `SR2`
3. Send register address → wait `TXE`, then `BTF`
4. Repeated START → wait `SB`
5. Send slave address (read) → wait `ADDR`
6. **Disable ACK** → clear `ADDR` → **generate STOP**
7. Wait `RXNE` → read `DR`

**Lesson Learned**: I2C flag clearing sequence is order-sensitive. Missing `SR2` read = `ADDR` flag stays set = transaction hangs.

### 3. Debugging with Instrumentation

**Pattern Used**:

```c
UART2_SendString("Step 1: START\r\n");
I2C1->CR1 |= (1 << 8);

UART2_SendString("Step 2: Wait SB\r\n");
while (!(I2C1->SR1 & (1 << 0)));

UART2_SendString("Step 3: SB set\r\n");
```

**Impact**: Pinpointed exact hang location (START never generating `SB` flag) → diagnosed root cause (incorrect `I2C->CR2` value due to wrong APB1 frequency assumption).

### 4. Timer Math

**Formula**:

```
f_update = f_apb1 / ((PSC + 1) * (ARR + 1))
```

**Example (1 Hz from 16 MHz APB1)**:

```c
TIM2->PSC = 15999;  // PSC+1 = 16000
TIM2->ARR = 999;    // ARR+1 = 1000
// Result: 16,000,000 / (16000 * 1000) = 1 Hz ✓
```

**Critical Dependency**: This only works if `f_apb1` is actually 16 MHz. If APB1 = 45 MHz (as discovered during I2C debug), math breaks → timer runs at wrong frequency.

---

## Critical Bugs Fixed

### Bug 1: I2C Peripheral Clock Mismatch

**Symptom**: I2C transactions hung at START condition; `SB` flag never set.

**Root Cause**:

```c
I2C1->CR2 = 16;  // Hard-coded "16 MHz"
```

Actual APB1 clock = **45 MHz** (configured by bootloader).

**Impact**: `CR2` value controls internal I2C timing calculations. Wrong value → START condition never generated correctly.

**Fix (In Progress)**:

```c
uint32_t apb1_freq = get_apb1_freq_hz();  // Read from RCC->CFGR
I2C1->CR2 = apb1_freq / 1000000;          // Set CR2 to actual APB1 MHz
I2C1->CCR = apb1_freq / (100000 * 2);     // 100 kHz I2C
I2C1->TRISE = (apb1_freq / 1000000) + 1;
```

**Lesson**: **Never assume peripheral clocks.** Always derive from `RCC->CFGR` at runtime.

### Bug 2: UART BRR Calculated for Wrong Clock

**Symptom**: UART worked at 9600 baud "by luck" because default HSI = 16 MHz matched assumption.

**Hidden Risk**: If system clock changes (PLL, HSE, different prescaler), baud rate breaks.

**Fix**:

```c
uint32_t apb1_freq = get_apb1_freq_hz();
USART2->BRR = apb1_freq / baudrate;  // Correct for any APB1 config
```

---

## Limitations / Next Steps

### Current Limitations

1. **No Timeouts**: All `while (!(flag))` loops hang forever if flag never sets → need timeout counters + error return codes.
2. **No Error Handling**: Functions return raw data, not status codes (`I2C_OK`, `I2C_TIMEOUT`, `I2C_NACK`).
3. **UART RX Overflow**: Circular buffer wraps silently if full; no overflow detection or notification.
4. **I2C Clock Config Incomplete**: `get_apb1_freq_hz()` not yet implemented (scheduled for immediate next commit).
5. **No Unit Tests**: Manual testing only via UART logs and LED observation.
6. **ADC Single-Channel Only**: No multi-channel scanning, no DMA.

### Next Steps

1. **Fix I2C Clock Bug** (Today)
   - Implement `get_apb1_freq_hz()` using `RCC->CFGR` + prescaler tables
   - Update all peripheral inits to use computed frequencies
   - Verify BME280 chip ID read (expect 0x60)

2. **Add Robustness** (This Week)
   - Timeout counters on all blocking waits
   - Return status codes from I2C functions
   - UART RX overflow flag

3. **Sensor Integration** (Next Week)
   - MPU6050 (6-axis IMU) over I2C
   - Multi-byte register reads (accel/gyro data)
   - Endianness handling for 16-bit signed values

4. **Project Integration** (Weeks 2-3)
   - Integrate into Creat-A-Thon 7.0 project (accident detection system)
   - GPS module (UART RX parsing)
   - ESP32 communication (UART TX)

---

## Build & Run

### Hardware

- **Board**: STM32 Nucleo F446RE
- **Debugger**: ST-LINK/V2-1 (onboard)
- **UART**: PA2 (TX), PA3 (RX) → connect to USB-UART adapter at 9600 baud, 8N1

### Toolchain

- **Compiler**: `arm-none-eabi-gcc`
- **Build System**: Makefile or STM32CubeIDE project
- **Flashing**: `st-flash` or STM32CubeProgrammer

### Quick Start

1. Clone repo
2. Open in STM32CubeIDE or build with Makefile
3. Flash to Nucleo board via ST-LINK
4. Connect UART2 (PA2/PA3) to serial terminal at 9600 baud
5. Observe UART output:
   - `"START\r\n"` on boot
   - `"I2C init done\r\n"` after I2C peripheral enabled
   - BME280 chip ID read result (currently debugging)

---

## Key Learnings

### 1. Clocks Are First-Class Configuration

Peripheral timing registers are **functions of bus clocks**, not constants. Any driver that assumes a fixed clock frequency is **fragile and non-portable**.

**Old Mindset**: "16 MHz is the clock."  
**New Mindset**: "What is APB1 right now? Let me compute it from `RCC->CFGR`."

### 2. ISRs Must Be Fast and Predictable

**Pattern**:
- ISR: Store data, set flag, return immediately
- Main loop: Check flag, process data, clear flag

**Anti-pattern**:
- ISR: Call `printf()`, do string parsing, make decisions (= unpredictable latency)

### 3. Debugging = Instrumentation Before Theory

**Effective Debugging Flow**:
1. Add UART prints between every major step
2. Binary-search to find **exact line where code hangs**
3. **Then** consult datasheet for that specific flag/register

**Ineffective Flow**:
1. Stare at code
2. Re-read entire datasheet chapter
3. Change random things
4. Hope it works

### 4. Multi-File Structure Scales

Single `main.c` worked for GPIO blink. Does **not** work for 5+ peripherals.

**Benefits of Separation**:
- Easier to debug (isolate driver vs. application logic)
- Reusable across projects
- Cleaner git diffs

---

## Resources

- **STM32F446RE Datasheet**: Pinouts, electrical specs
- **RM0390 Reference Manual**: Register descriptions (UART, I2C, TIM, ADC, RCC chapters heavily used)
- **ARM Cortex-M4 Generic User Guide**: Exception model, NVIC, interrupt priorities
- **Debugging Logs**: See `docs/` folder for detailed I2C debug session notes

---

## Author Notes

This repo represents **real learning progression**, not polished tutorial code. Some sections are clean (UART driver), others are mid-debug (I2C clock fix in progress).

**Philosophy**: Build → Break → Fix → Understand. The clock tree gap wasn't learned from a textbook; it was discovered through **failed I2C transactions and systematic debugging**.

Current focus: Fix I2C clock bug, complete BME280 integration, then move to MPU6050 for Creat-A-Thon project.

---

**Last Updated**: March 5, 2026  
**Status**: UART/GPIO/TIM2/ADC working; I2C debugging in progress (clock config fix scheduled today)