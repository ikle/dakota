# FPGA Development Helper Tools

Usage Example:

1. Import Lattice FPGA databases from project Trellis:
```bash
$ PREFIX=/usr FAMILY=ECP5    DEVICE=LFE5U-25F     test/import-db
$ PREFIX=/usr FAMILY=MachXO2 DEVICE=LCMXO2-7000HC test/import-db
```
2. Map you design (output from nextpnr) to PNM bitmaps:
```bash
$ ./trellis-map ECP5 test/hdmi-test.trellis test/hdmi-test.pnm
```

The resulting file will contain two black-and-white bitmaps: one with the
actual contents of the chip configuration, the second — a mask, where only
those bits that have been changed by design are set.
