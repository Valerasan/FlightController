# FlightController

STM32F411 (WeAct BlackPill V2.0) flight controller firmware. PlatformIO +
STM32CubeMX for peripheral config, with a hand-written C++ application layer
on top. Currently reads a CRSF RC receiver over UART1.

## Toolchain

- **PlatformIO** (`platform = ststm32`, `framework = stm32cube`) for
  building/flashing.
- **STM32CubeMX** (`FlightController.ioc`) for clocks/pin/peripheral
  configuration — generates `Core/` and `USB_DEVICE/`.
- **ST-Link** over SWD for flashing/debugging.

## Project layout

| Path | What it is |
|---|---|
| `Core/Src`, `Core/Inc` | CubeMX-generated: `main.c`, clock config, `stm32f4xx_it.c` (interrupt handlers), HAL MSP init. `src_dir`/`include_dir` in `platformio.ini` point here. |
| `USB_DEVICE/` | CubeMX-generated USB CDC (virtual COM port) glue. |
| `App/Inc`, `App/Src` | Hand-written C++ application code — this is where actual firmware logic lives. Compiled via an explicit `build_src_filter`/`build_flags` in `platformio.ini` since it sits outside `Core/`. |
| `boards/blackpill_f411ce.json` | **Required, do not delete.** See below. |
| `tools/cubemx_cleanup.py` | Runs automatically before every build (`extra_scripts` in `platformio.ini`); deletes IDE project scaffolding CubeMX regenerates (`EWARM/`, `Drivers/`, etc.) that PlatformIO doesn't need. |
| `FlightController.ioc` | CubeMX project file. |

### `App/` structure

- `app.cpp` — `app_init()`/`app_loop()`, called from `Core/Src/main.c`'s
  `USER CODE BEGIN 2` / `USER CODE BEGIN WHILE` sections. CRSF channel
  decoding (`crsf_parse_channels`, `crsf_to_pwm_us`) lives here for now.
- `uart.h`/`uart.cpp` — `UartBase`: generic interrupt-driven UART receive
  (byte-at-a-time), instance registry (`MAX_UARTS`), dispatches
  `HAL_UART_RxCpltCallback` to the right instance. Protocol-agnostic —
  `parseByte()` is a pure-virtual hook.
- `uart_crsf.h`/`uart_crsf.cpp` — `UartCrsf : public UartBase`, implements
  CRSF frame sync/length/CRC parsing. Exposes the last valid frame via
  `frameReady()`/`frameType()`/`framePayload()`/`consumeFrame()`.
- `log.h`/`log.cpp` — `log_printf()`, writes to the USB CDC virtual COM
  port via `CDC_Transmit_FS`. **Do not use `printf()` directly** — the
  build links `--specs=nosys.specs`, so its `_write()` syscall is a stub
  and output silently goes nowhere.
- `define.h` — `LOG(fmt, ...)` macro (wraps `log_printf`, appends `\n`;
  `fmt` must be a string literal), LED helper macros.

## Building / flashing

```
pio run                 # build
pio run -t upload       # build + flash over ST-Link
```

## Editing peripherals in CubeMX

1. Open `FlightController.ioc` in STM32CubeMX, make changes, **Generate
   Code**.
2. `pio run` — `tools/cubemx_cleanup.py` runs automatically and strips the
   regenerated IDE scaffolding (EWARM/Drivers/etc.) CubeMX writes out.
3. If CubeMX regenerated `Core/Src/main.c` from scratch (new peripherals
   added a fresh `USER CODE` skeleton), re-add the two lines it doesn't
   preserve elsewhere:
   ```c
   /* USER CODE BEGIN 2 */
   app_init();
   /* USER CODE END 2 */
   ...
   while (1) {
     app_loop();
   /* USER CODE END WHILE */
   ```
   (Content inside existing `USER CODE BEGIN/END` markers survives
   regeneration — this is only needed if a section didn't exist before.)

