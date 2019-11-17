#!/bin/bash

# Location of OpenOCD Board .cfg files
OPENOCD_BOARD_DIR=/usr/share/openocd/scripts/board
DEBUGGER=arm-none-eabi-gdb

# Start xterm with openocd in the background
xterm -e openocd -f $OPENOCD_BOARD_DIR/ek-tm4c123gxl.cfg &

# Save the PID of the background process
XTERM_PID=$!

# Wait for the hardware to be ready
sleep 2

# Execute some initialisation commands via gdb
$DEBUGGER --batch --command=init.gdb gcc/main.axf

# Start the gdb gui
nemiver --remote=localhost:3333 --gdb-binary="$(which $DEBUGGER)" gcc/main.axf

# Close xterm when the user has exited nemiver
kill $XTERM_PID

