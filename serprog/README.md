# Serial port based unbricking

1. Run `make` or download the result via GitHub Actions
2. Download [flashrom](https://www.flashrom.org/Downloads)
3. Run: `python3 bootrom.py serprog.bin --dev /dev/ttyUSB0 --listen-once`
4. Run: `flashrom -p serprog:dev=/dev/ttyUSB0`
