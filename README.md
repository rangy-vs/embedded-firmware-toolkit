# Embedded Firmware Toolkit

![ci](../../actions/workflows/ci.yml/badge.svg)

Small, dependency-free C11 building blocks that nearly every firmware project re-implements, written to be portable (no `malloc`, no libc beyond `string.h`) and unit-tested on the host.

| Module | What it does |
|---|---|
| `ring_buffer` | Power-of-two byte FIFO; uses **all** slots (indices run over 2×capacity). SPSC-safe for ISR→main-loop use. |
| `crc` | CRC-16/CCITT-FALSE and CRC-8, incremental, verified against the standard `"123456789"` check values (`0x29B1`, `0xF4`). |
| `debounce` | Integrator debouncer for noisy buttons/switches, rate-independent. |
| `cmd_parser` | Byte-at-a-time framed-protocol state machine: `0xAA │ LEN │ payload │ CRC16`. Resyncs after noise, rejects bad length/CRC, keeps link-quality counters. |

```bash
make test       # 804 checks, built with -Wall -Wextra -Wpedantic -Werror
make sanitize   # same tests under AddressSanitizer + UBSan
```

## Design notes
- **Payloads may contain the start byte `0xAA`:** framing relies on length + CRC, not byte-stuffing, and a test covers it.
- **Compile-clean under `-Werror`**, and CI runs the sanitizer build, so buffer overruns and UB fail the build.
- **Target-agnostic:** swap the host test harness for your MCU toolchain; nothing in `src/` touches hardware. Index reads/writes in `ring_buffer` assume atomic word access (true on most 32-bit MCUs; use a critical section otherwise).
