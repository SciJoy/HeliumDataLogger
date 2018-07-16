from __future__ import (absolute_import, division,
                        print_function, unicode_literals)
from builtins import *

import time
from helium_client import Helium
import RPi.GPIO as GPIO
import paho.mqtt.client as mqtt
import json
from random import *

#Connect to Helium
helium=Helium(b'/dev/serial0')
helium.connect()
channel = helium.create_channel(b'Hackster Project')

#To keep track of pins and sensor names

dictESP1 = {2:"",4:"",5:"",12:"",13:"",14:"",15:"",16:"",17:"",18:"",19:"",21:"",22:"",23:"",25:"",26:"",27:"",32:"",33:"",34:"",35:""}
dictESP2 = {2:"",4:"",5:"",12:"",13:"",14:"",15:"",16:"",17:"",18:"",19:"",21:"",22:"",23:"",25:"",26:"",27:"",32:"",33:"",34:"",35:""}
dictESP3 = {2:"",4:"",5:"",12:"",13:"",14:"",15:"",16:"",17:"",18:"",19:"",21:"",22:"",23:"",25:"",26:"",27:"",32:"",33:"",34:"",35:""}

configTracking = 0

#Subscribe to topics
def on_connect(client, userdata, flags, rc):
    print("Connected with result code " + str(rc))
    client.subscribe('outTopic')
    client.subscribe('ToPi/ESP1')
    client.subscribe('ToPi/ESP2')
    client.subscribe('ToPi/ESP3')

#After getting message from ESP send data to Helium
def on_message(client, userdata, msg):
    print('{0} got: {1}'.format(msg.topic, msg.payload))
    if msg.topic == 'ToPi/ESP1':
        payload = (msg.payload)
        payload2 = payload.decode("utf-8")
        Epoch,Count,Min,Max,Avg,Pin=payload2.split(",")
        PinI = int(Pin)
        sensor = dictESP1[PinI]
        gcpPayload = json.dumps({"E":1,"T":Epoch,"S":sensor,"C":Count,"M":Min,"X":Max,"A":Avg})
        json.loads(gcpPayload)
        print('To GCP1: ')
        print(gcpPayload)
        bytesGCP = str.encode(gcpPayload)
        channel.send(bytesGCP)
    if msg.topic == 'ToPi/ESP2':
        payload = (msg.payload)
        payload2 = payload.decode("utf-8")
        Epoch,Count,Min,Max,Avg,Pin=payload2.split(",")
        PinI = int(Pin)
        sensor = dictESP2[PinI]
        gcpPayload = json.dumps({"E":2,"T":Epoch,"S":sensor,"C":Count,"M":Min,"X":Max,"A":Avg})
        json.loads(gcpPayload)
        print('To GCP2: ')
        print(gcpPayload)
        bytesGCP = str.encode(gcpPayload)
        channel.send(bytesGCP)
    if msg.topic == 'ToPi/ESP3':
        payload = (msg.payload)
        payload2 = payload.decode("utf-8")
        Epoch,Count,Min,Max,Avg,Pin=payload2.split(",")
        PinI = int(Pin)
        sensor = dictESP3[PinI]
        gcpPayload = json.dumps({"E":3,"T":Epoch,"S":sensor,"C":Count,"M":Min,"X":Max,"A":Avg})
        json.loads(gcpPayload)
        print('To GCP3:')
        print(gcpPayload)
        bytesGCP = str.encode(gcpPayload)
        channel.send(bytesGCP)

#conencting to MQTT
client = mqtt.Client()
client.on_connect = on_connect
client.on_message = on_message
client.connect('localhost', 1883, 60)
client.loop_start()
time.sleep(10)

print('Script is running, press Ctrl-C to quit...')

while True:
    #Get config varibales from GCP
    config = channel.config()
    configMode = config.get(b'channel.configMode')

    #Get Config Variables and Send to ESP1
    E1T = config.get(b'channel.E1T')
    if E1T != None:
        E1T = config.get(b'channel.E1T')
        E1R = config.get(b'channel.E1R')
        E1S = config.get(b'channel.E1S')
        print(E1T)
        print(E1R)
        print(E1S)
        configPayloadE1 = E1T + b'-'+ E1R
        E1TString = E1T.decode("utf-8")
        E1Pin,E1ST,E1ET=E1TString.split("-")
        E1PinInt = int(E1Pin)
        dictESP1[E1PinInt] = E1S.decode("utf-8")
        print('Config Variables 1')
        print(configPayloadE1)
        client.publish('FromPi/ESP1', configPayloadE1)

    #Get Config Variables and Send to ESP2
    E2T = config.get(b'channel.E2T')
    if E2T != None:
        E2T = config.get(b'channel.E2T')
        E2R = config.get(b'channel.E2R')
        E2S = config.get(b'channel.E2S')
        configPayloadE2 = E2T + b'-'+ E2R
        E2TString = E2T.decode("utf-8")
        E2Pin,E2ST,E2ET=E2TString.split("-")
        E2PinInt = int(E2Pin)
        dictESP2[E2PinInt] = E2S.decode("utf-8")
        print('Config Variables 2:')
        print(configPayloadE2)
        client.publish('FromPi/ESP2', configPayloadE2)
            
    #Get Config Variables and Send to ESP3
    E3T = config.get(b'channel.E3T')
    if E3T != None:
        E3T = config.get(b'channel.E3T')
        E3R = config.get(b'channel.E3R')
        E3S = config.get(b'channel.E3S')
        print(E3S)
        configPayloadE3 = E3T + b'-'+ E3R
        E3TString = E3T.decode("utf-8")
        E3Pin,E3ST,E3ET=E3TString.split("-")
        E3PinInt = int(E3Pin)
        dictESP3[E3PinInt] = E3S.decode("utf-8")
        print('Config Variables 3: ')
        print(configPayloadE3)
        client.publish('FromPi/ESP3', configPayloadE3)

    if configMode == None:
        break
    time.sleep(10)
    print('')
    configTracking = time.time()

while True:
    configWait = 11120
    
    now = time.time()
    elapsed = now - configTracking
    if elapsed > configWait:
        break












