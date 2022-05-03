import paho.mqtt.client as paho
import time
import matplotlib.pyplot as plt
import numpy as np
import re
# https://os.mbed.com/teams/mqtt/wiki/Using-MQTT#python-client

# MQTT broker hosted on local machine
mqttc = paho.Client()

# Settings for connection
# TODO: revise host to your IP
host = "192.168.1.33"
topic = "Mbed"
x = np.linspace(0,1,1000)
y = np.linspace(0,1,1000)
z = np.linspace(0,1,1000)
cnt = 0
# Callbacks
def on_connect(self, mosq, obj, rc):
    print("Connected rc: " + str(rc))

def on_message(mosq, obj, msg):
    
    global x, y, z, cnt
    txt = str(msg.payload)
    txt = txt[2:-5]
    print("[Received] Topic: " + msg.topic + ", Message: " + txt)
    if (txt == "End"):
        x, y, z = [x[:cnt], y[:cnt], z[:cnt]]
        time = np.linspace(0, cnt * 0.1, cnt)
        fig, ax = plt.subplots(3, 1)
        ax[0].plot(time, x)
        ax[0].set_xlabel('Time')
        ax[0].set_ylabel('ax(t)')
        ax[1].plot(time, y)
        ax[1].set_xlabel('Time')
        ax[1].set_ylabel('ay(t)')
        ax[2].plot(time, z)
        ax[2].set_xlabel('Time')
        ax[2].set_ylabel('az(t)')
        plt.suptitle("Accelerometer")
        #plt.subplots_adjust(bottom=0.1, top=0.9, wspace=None, hspace=0.25)
        plt.show()
        x = np.linspace(0,1,1000)
        y = np.linspace(0,1,1000)
        z = np.linspace(0,1,1000)
        cnt = 0
    else:
        #print(re.findall("[-\d]+", txt))
        #print(int(i) for i in re.findall("[-\d]+", txt))
        x[cnt], y[cnt], z[cnt] = [int(i) for i in re.findall("[-\d]+", txt)]
        cnt += 1


def on_subscribe(mosq, obj, mid, granted_qos):
    print("Subscribed OK")

def on_unsubscribe(mosq, obj, mid, granted_qos):
    print("Unsubscribed OK")

# Set callbacks
mqttc.on_message = on_message
mqttc.on_connect = on_connect
mqttc.on_subscribe = on_subscribe
mqttc.on_unsubscribe = on_unsubscribe

# Connect and subscribe
print("Connecting to " + host + "/" + topic)
mqttc.connect(host, port=1883, keepalive=60)
mqttc.subscribe(topic, 0)

# Publish messages from Python
'''
while (1):
    ret = mqttc.subscribe()
    if (ret[0] != 0):
            print("Publish failed")
    mqttc.loop()
    time.sleep(1.5)
    num += 1
'''
# Loop forever, receiving messages
mqttc.loop_forever()