# FPGA-Peripheral-Integration-SystemComprehensive FPGA system shell for Intel MAX 10 (10M08SAE144C8G):

Top entity: LogicalStep_top.

Integrates:

- LEDs (standard + EGM)

- LCD controller

- Seven-segment display

- Slide switches + push buttons

- vAudio codec (I²C + audio I/O)

- UART, SD card, SDRAM

- Uses ifdef macros to enable/disable peripherals.

- Serves as a flexible base design for experiments and labs.
