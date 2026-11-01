# xnu/patches

Kernel patches applied in filename order on top of xnu-8796 by
`tools/xnu-build-local.sh` (and by `.github/workflows/xnu.yml`). `0001`-`0078`
are the sweet (Redmi Note 10 Pro) port inherited from
[dipa-studio/ios-sweet](https://github.com/dipa-studio/ios-sweet); `0079`+ is
the Pixel 1 (`sailfish`) port. `0175-stage-ionetworking.sh` and friends are
staged into the tree by the build script, not `git apply`ed.

Recent sailfish series:

- `0181` GICv3 SPI dispatch; Wi-Fi (QCA6174 INTA) and touch (TLMM 125) on
  interrupts, polling fallback (`sailfishirq=0` / `sweettouchirq=0`).
- `0182` UI performance: present on a kernel thread, fence waits without the
  VM lock, dirty-page-only submit flushes, touch coalescing (`sweettouchhz`).
- `0183` Opt-in CPU boost: `sweetboost=MHZ` / `sweetboostpwr=MHZ`, turbo via
  `sweetturbo=1`. Default off.
- `0184` Sensor hub bring-up: nanohub (STM32F4) comms over BLSP1 QUP6 SPI,
  nanohub packet protocol, sensor config/event protocol. Opt-in `sailhub=1`,
  `sailhubswap=1`, `sailhubdbg=1`. Untested on device.
- `0185` IOHIDEventService sensor services (accel/gyro/compass/ALS) fed by
  `sweet_hid_sensor()` from the 0184 driver.

CI: push-triggered on changes under this directory.
