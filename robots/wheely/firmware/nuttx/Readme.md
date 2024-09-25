## Compile

Change directory to 
```bash
cd robots/wheely/firmware/nuttx/nuttx
```
and execute
```bash
./tools/configure.sh wheely:nsh
make
```

## Flash
Install stlink and upload `nuttx.bin` to controller.
