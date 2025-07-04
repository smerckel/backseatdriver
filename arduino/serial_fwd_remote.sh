#!/bin/bash

# Script that can be used to link serial ports over a network
#                                                                                               #
#                                                                                               #
#   arduino -> /dev/ttyACM0  ----\                                                              #
#                                |                                                              #
#                                |                                                              #
#                            ( network)                                                         #
#                                |                                                              #
#                                \-------> /dev/ttyUSB1 -> other device                         #
#                                                                                               #
#   run:                                    run:                                                #
#   serial_fwd_remote.sh                    serial_fwd_local.sh                                 #
#                                                                                               #
#   Both scripts are identical. Their behaviour (being local or remote) depends on the name     #
#   of the script.
#

SERIAL_DEVICE_REMOTE="/dev/ttyUSB0"
SERIAL_DEVICE_LOCAL="/dev/ttyUSB1"
IP_LOCAL="10.61.7.16"
TCP_PORT_LOCAL=12345

SOCAT="/usr/bin/socat"

progname=$(basename $0)

if [ ${progname} == "serial_fwd_remote.sh" ]; then
    ${SOCAT} -d -d ${SERIAL_DEVICE_REMOTE},raw,echo=0 TCP:${IP_LOCAL}:${TCP_PORT_LOCAL}
elif [ ${progname} == "serial_fwd_local.sh" ]; then
    ${SOCAT} -d -d TCP-LISTEN:${TCP_PORT_LOCAL},fork ${SERIAL_DEVICE_LOCAL},raw,echo=0
else
    echo "Script has wrong name. Don't know what to do. Exiting."
    exit 1
fi
