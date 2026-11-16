# STM32F103C8 firmware

Minimal CMSIS project for the STM32F103C8 (Blue Pill), built with CMake and
the Arm GNU Toolchain.

## Requirements

- CMake 3.20 or newer
- `arm-none-eabi-gcc`, `arm-none-eabi-objcopy` and `arm-none-eabi-size`
- Make

On macOS with Homebrew:

```sh
brew install cmake
brew install --cask gcc-arm-embedded
```

## Command-line build

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build --target bin --parallel
```

Build products:

- `build/stm32_template.elf` — executable with debug information
- `build/stm32_template.bin` — binary firmware image for PICSimLab
- `build/stm32_template.hex` — Intel HEX firmware image
- `build/stm32_template.map` — linker map

## VS Code

The `.vscode` directory contains the recommended extensions, CMake settings,
and the default build task. Open this project folder and press `Cmd+Shift+B`.
The `STM32: Build firmware` task configures the project and creates all
firmware formats listed above.

## PICSimLab

Select the **Blue Pill** board and the **stm32f103c8t6** microcontroller, then
load `build/stm32_template.bin`.

The firmware for practice 3 uses:

- **PC13** — the onboard active-low LED;
- **PB0** — an active-low push button with the MCU's internal pull-up enabled.

Each debounced button press switches the LED between two clearly distinct
blink rates: normal and five times as fast. Holding the button does not
repeatedly change the rate; the next change happens only after releasing and
pressing it again. Set the PICSimLab **IO (MHz)** field to **8** for a convenient
visual blink rate.

The ready-to-open PICSimLab workspace is `practice3_button_frequency.pzw`.

## Практика 4 — SPI-дисплей SSD1306

[Комплект практики 4](practice4_ssd1306/) содержит исходники, готовую прошивку,
рабочее пространство PicSimLab и инструкцию сдачи.

- [Открыть основной исходный код](practice4_ssd1306/stm32_pm2026/main.c)
- [Скачать рабочее пространство Starinskaya_lab4.pzw](practice4_ssd1306/Starinskaya_lab4.pzw)
- [Описание, подключение, сборка и проверки](practice4_ssd1306/stm32_pm2026/README.md)

Сборка этой практики из корня репозитория:

```sh
cmake -S practice4_ssd1306/stm32_pm2026 -B build/practice4
cmake --build build/practice4
```

Результат — шахматная доска с клетками 8×8 пикселей на SSD1306 128×64.
Сборка и проверка кадра из ARM-кода прошли; границы проверки PicSimLab
описаны в README практики.
