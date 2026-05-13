import paho.mqtt.client as mqtt
import subprocess
import mariadb
import os
import json
from dotenv import load_dotenv
from pathlib import Path

load_dotenv()
dbHost = os.getenv("DB_HOST")
dbUser = os.getenv("DB_USER")
dbPw = os.getenv("DB_PASSWORD")
dbName = os.getenv("DB_NAME")

serverIP = os.getenv("SERVER_IP")
serverPort = os.getenv("SERVER_PORT")

BASE_DIR = Path(__file__).resolve().parents[2]
triggerAI = os.getenv("TRIGGER_AI")

#MQTT SUBS
subs = ["webAi/chat/input", "webAi/chat/output", "webAi/log/reqlog", "webAi/control/reqAdd/input", "webAi/control/reqAdd/output", "webAi/control/reqAdd/confirmation","webAi/control/reqRemove/amount", "webAi/control/reqClear/confirm"]

#INITIATE  DB
def with_db(callback, *args):
    try:
        conn = mariadb.connect(host=dbHost, user=dbUser, password=dbPw, database=dbName)
        cursor = conn.cursor()
        callback(cursor, *args)
        conn.commit()
    except Exception as e:
        print(f"DB error: {e}")
    finally:
        cursor.close()
        conn.close()

def getDB():
    return mariadb.connect(host=dbHost, user=dbUser, password=dbPw, database=dbName)



def packageChatlog(num):
    with getDB() as conn:
        cur = conn.cursor()
        cur.callproc("GetRecentMsgIII", [num])
        rows = cur.fetchall()

    json_data = []
    for row in rows:
        json_data.append({
            "id": row[0],
            "input": row[1],
            "response": row[2],
            "date": row[3].strftime("%Y-%m-%d %H:%M:%S"),
            "source": row[4],
            "action": row[5]
        })

    return json.dumps(json_data, indent=6)

#Connect to MQTT Server
def on_connect(client, userdata, flags, rc):
    for i in range(len(subs)):
        client.subscribe(subs[i])

def handleInput(cur, inp):
    cur.callproc("StoreToUserChat", [inp, "website"])

def handleOutput(cur, out):
    cur.callproc("StoreToAIChat", [out])

def handleChat(cur):
    cur.callproc("StoreToChat", [None])

#Listen For Messages
def on_message(client, userdata, msg):
    topic = msg.topic
    payload = msg.payload.decode()
    print(payload)
    if topic == subs[0]:
        with_db(handleInput, payload)
        subprocess.Popen(["bash", triggerAI])
    elif topic == subs[2]:
        client.publish("webAi/log/reclog", packageChatlog(8))
    elif topic == subs[3]:
        with_db(handleInput, payload)
    elif topic == subs[4]:
        with_db(handleOutput, payload)
    elif topic == subs[5]:
        with_db(handleChat)
    elif topic == subs[6] and payload.isdigit():
        print("Delted")
        with_db(lambda cur, x: cur.execute("CALL DeleteSomeChats(?)", (int(x),)), payload)

    elif topic == subs[7]:
        print("Lmaoo you cant lol")

#Stay MQTT Active and Listening
client = mqtt.Client()
client.on_connect = on_connect
client.on_message = on_message
client.connect(serverIP, serverPort, 60)
client.loop_forever()