# *****************************************************
# Configuration file for the GNU ARM debugger, to be 
# included in the debug bash script debug_nemiver.sh .
# Abdulmaguid Eissa
# Nov 15, 2019
# *****************************************************

# Setting connection timeout 
set remotetimeout 10

# Specify remote target 
# Host is listening on port number 3333 for gdb connection
target remote :3333 

# Reset to known state 
monitor reset halt
load
monitor reset init

# Set a breakpoint at main()
#break main

# Run to the breakpoint
#continue
