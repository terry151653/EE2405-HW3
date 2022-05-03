# led_test_client.py
# Test client for erpc led server example
# Author: becksteing/Jing-Jia Liou
# Date: 02/13/2022
# Blinks LEDs on a connected Mbed-enabled board running the erpc LED server example

from time import sleep
import erpc
from hw3 import *
import sys


if len(sys.argv) != 3:
    print("Usage: python eRPC_client.py <serial port to use> <start/stop>")
    exit()
# Initialize all erpc infrastructure
xport = erpc.transport.SerialTransport(sys.argv[1], 9600)
client_mgr = erpc.client.ClientManager(xport, erpc.basic_codec.BasicCodec)
client = client.accelerometerClient(client_mgr)
if(sys.argv[2] == "start"):
    print("Call service_on ")
    client.service_on()
else:
    print("Call service_off ")
    client.service_off()
sys.exit()
