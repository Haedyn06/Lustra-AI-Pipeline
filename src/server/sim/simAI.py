import paho.mqtt.client as mqtt
import subprocess
import mariadb
import os
from dotenv import load_dotenv
from pathlib import Path

load_dotenv()
dbHost = os.getenv("DB_HOST")
dbUser = os.getenv("DB_USER")
dbPw = os.getenv("DB_PASSWORD")
dbName = os.getenv("DB_NAME")

serverIP = os.getenv("SERVER_IP")
serverPort = os.getenv("SERVER_PORT")


MQTT_BROKER = serverIP

#File Path Declaration
BASE_DIR = Path(__file__).resolve().parents[2]
triggerAI = os.getenv("TRIGGER_AI")

lustraInput = "lustrasim/lustratalk/input"
systemOutput = "lustrasim/lustratalk/response"

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

def handleInput(cur, inp):
    cur.callproc("StoreToUserChat", [inp, "simulator"])

#Receive Data
def onMessage(client, userdata, msg):
    topic = msg.topic
    message = msg.payload.decode()
    # subprocess.run(["notify-send", f"Received On: {msg.topic}", message])
    if topic == lustraInput:
        print(f"User said: {message}")
        with_db(handleInput, message)
        subprocess.Popen(["bash", triggerAI])


client = mqtt.Client()
client.connect(MQTT_BROKER, serverPort, 60)

client.subscribe(lustraInput)

client.on_message = onMessage
client.loop_forever()
