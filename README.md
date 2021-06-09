# FPGA Development Helper Tools

Usage Example:

1. Import Lattice FPGA databases from project Trellis:
```bash
$ PREFIX=/usr test/import-db ECP5    LFE5U-25F
$ PREFIX=/usr test/import-db MachXO2 LCMXO2-7000HC
```
2. Map you design (output from nextpnr) to PNM bitmaps:
```bash
$ ./trellis-map ECP5 test/hdmi-test.trellis test/hdmi-test.pnm
```

The resulting file will contain two black-and-white bitmaps: one with the
actual contents of the chip configuration, the second — a mask, where only
those bits that have been changed by design are set.

If your picture viewer cannot handle multi-picture images, you can split
them using pnmsplit, a utility from the Netpbm project:
```bash
$ pnmsplit test/hdmi-test.pnm test/hdmi-test-%d.pnm
pnmsplit: WRITING test/hdmi-test-0.pnm

pnmsplit: WRITING test/hdmi-test-1.pnm

$
```
