# _lolmon_ monitor

Lolmon is a small machine monitor (or not even that), which allows memory and
MMIO to be read/written and called (executed).

```
> help
help - Show help output for one or all commands
echo - Echo a few words
rb - Read one or more bytes
rh - Read one or more half-words (16-bit)
rw - Read one or more words (32-bit)
wb - Write one or more bytes
wh - Write one or more half-words (16-bit)
ww - Write one or more words (32-bit)
cb - Copy one or more bytes
ch - Copy one or more half-words (16-bit)
cw - Copy one or more words (32-bit)
sync - Synchronize caches
call - Call a function by address
src - Source/run script at address
flrd - Read from flash
flwr - Write data to flash; destination must be 4k-aligned
boot - Continue with the usual boot flow
```

More complex tasks can be scripted in Python, using the [interact.py](./interact.py) script:

```
$ python3 -i interact.py
lolmon detected!
>>> clk.dump()
bf500000: 06202033 00000033 00000001 06202033 00001117 00000101 03000f0f 0011ff03
bf500020: 00020000 000f0000 06202033 06202033 00040000 00000031 06202033 06202033
bf500040: 00000000 00000000 00000000 00000000 00000102 00776363 06202033 06202033
bf500060: 06202033 06202033 06202033 06202033 06202033 06202033 06202033 06202033
```


## Further examples

- Uploading and booting Linux through interact.py:
  `uart0.set_baud_rate(8*115200);A=0x81000000;l.write_file(A,'/home/jn/dev/linux/linux-git/build-mips/vmlinuz-dtb');l.call_linux_and_run_microcom(A)`
