# EE2405-HW3

Connect XBeeA to PC.

Connect XBeeB to board, Rx to D0; Tx to D1; GND to GND; 5V to 5V, respectively.

Connect board to PC.

Set up hotspot, connect PC to that and record the IP of PC. Use the record IP to set up configuration and change the HOST IP in main.cpp, and then build and flash it to board.

Create MQTT server with mosquitto.conf, and run mqtt_client.py to keep subscribing and plot something when get some data.

Reset board so that board can connect to MQTT server.

Run eRPC_client.py, with parameters like >> py eRPC_client.py <serial port to XBee A> <start/stop>

Then we can get plot from mqtt_client.py when call stop with eRPC_client.py.
