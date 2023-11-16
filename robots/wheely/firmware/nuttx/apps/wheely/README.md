# maxi1m

### Selecting maxi1m apps

The maxi1m directory contains several sample applications that can be linked
with NuttX. The specific maxi1m app is selected in the
`boards/<arch-name>/<chip-name>/<board-name>/configs/<config>/defconfig` file
via the `CONFIG_MAXI1M_xyz` setting where `xyz` is the name of the example.
For example:

```conf
CONFIG_MAX1M_HELLO=y
```

Selects the `maxi1m/hello` _Hello, World!_ example.

