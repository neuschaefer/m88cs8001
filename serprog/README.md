# Serial port based unbricking

1. Run `make` or download the result via [GitHub Actions](https://github.com/neuschaefer/m88cs8001/actions?query=branch%3Amain)
2. Download [flashrom](https://www.flashrom.org/Downloads)
3. Run: `python3 bootrom.py serprog.bin --dev /dev/ttyUSB0 --listen-once` and power-cycle the device
4. Run: `flashrom -p serprog:dev=/dev/ttyUSB0`
