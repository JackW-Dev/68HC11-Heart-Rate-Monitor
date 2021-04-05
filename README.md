# 68HC11-Heart-Rate-Monitor
ANSI C90 application for use with the Motorola 68HC11 microcontroller.
This application simulates a heart rate monitor by measuring voltage input.

This application will allow the user to:

1. Log in to an access controlled system via a username/password form
2. Monitor the patient's pulse (in this case simulated by an incoming squarewave)
3. Adjust the safe bounds for a pulse to fall between (lowest and highest safe BPM)
4. Trigger an alarm based on the BPM exceeding boundaries
5. Log events and allow the user to view the log

NOTE:
The version of this application that provides logging was produced remotely due to COVID-19 lockdown restrictions.
Due to this, access to the required hardware was not available and consequently, the system has only been tested via simulation and not on the chipset.
The version without logging was fully tested and operational prior to any lockdown restrictions.

This application was produced as a second year university assignment.
