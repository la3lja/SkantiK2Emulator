A Skanti TRP8000 series radio serial interface program for the Raspberry Pi. 
It reads/writes Skanti serial commands on one serial port, and read/writes a simplified set of Elecraft K2 or K3 
commands on another serial port. 
 
Developed because current amateur software has limited or no support for Skanti radios, however
most of them support the Elecraft radios. It has been tested with Wspr, WSJT_X, N1MM logger+, 
FlDigi, HRD v5, and a few others.

The program has also been tested with virtual serial ports on the Raspberry Pi. Using the command
"socat -d -d pty,raw,echo=0 pty,raw,echo=0" creates two linked virtual ports, usually /dev/pts/1 and /dev/pts/2
(numbering can change, so safest is to run it through crontab at boot). Setting /dev/pts/1 as the Skanti serial port
in the ini-file makes /dev/pts/2 available for other programs on the Raspberry. This has been tested and works 
with Wspr and band-hopping.
 
This version polls data from the CU (control unit). For this to work CU software have to be
version 92 or newer. Software was developed and tested on version 92. Version 80 will NOT work
with polling, however other commands should work.
 
INSTALL: Compile with make, edit skanti.ini to your likings and run skanti3 as normal user 
in the same directory where skanti.ini is located.

Feel free to use or modify this code as to your liking. Suggestions and bug reports are also very welcome!
 
The inireader library is copied from https://github.com/benhoyt/inih
