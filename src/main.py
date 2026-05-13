import os, sys, re, subprocess, asyncio, json
import mariadb
import paho.mqtt.client as mqtt
from langchain_ollama import OllamaLLM
from langchain_core.prompts import ChatPromptTemplate
from dotenv import load_dotenv
from pathlib import Path


load_dotenv()
dbHost = os.getenv("DB_HOST")
dbUser = os.getenv("DB_USER")
dbPw = os.getenv("DB_PASSWORD")
dbName = os.getenv("DB_NAME")

webIP = os.getenv("SERVER_IP")
webPort = os.getenv("SERVER_PORT")

#File  Paths
BASE_DIR = Path(__file__).resolve().parents[1]
prefFile = BASE_DIR / "configs" / "AIPreference.json"
lustraVoiceFile = BASE_DIR / "src" / "general" / "AIVoice.py"

# ollama env
os.environ["OLLAMA_NUM_GPU_LAYERS"] = "12"
os.environ["OLLAMA_USE_FLASH_ATTENTION"] = "1"

#Web Server
noWeb = True
client = mqtt.Client()

try:
    client.connect(webIP, int(webPort), 5)
    client.loop_start()
    noWeb = False
    print("MQTT connected")
except Exception as e:
    client = None
    noWeb = True
    print(f"MQTT disabled: {e}")

# AI Model and Prompt Template
with open(prefFile, "r") as file:
    prefs = json.load(file)

aiName = prefs["aiName"]
userName = prefs["userName"]
modelName = prefs["model"]
personality = prefs["personality"]
instructions = prefs["instructions"]
isAdult = prefs["isAdult"]


template = f"""
You are {personality}.

{instructions}
Address the user as {userName} and call yourself {aiName} (or another name the user prefers).

{"Adult responses are allowed." if isAdult else "Keep responses safe and non-explicit."}

Previous User: {{last_user_msg}}
Previous Lustra: {{last_ai_msg}}
User Query: {{usr_input}}

Lustra:
"""

#Initiate AI Model
model = OllamaLLM(model=modelName, streaming=True)
prompt = ChatPromptTemplate.from_template(template)
chain = prompt | model

#Initiate Ai Chatlog Database
def dbConnection():
    return mariadb.connect(host=dbHost, user=dbUser, password=dbPw, database=dbName)


#Retrieve Past Messages From DB
def chatHistory(pastLogs=5):
    with dbConnection() as db:
        cursor = db.cursor()
        cursor.execute("CALL GetRecentMsg(?)", (pastLogs,))
        return list(reversed(cursor.fetchall()))


#Store To Chatlogs
def saveChatLogs(aiResponse):
    act = re.search(r"\*(.*?)\*", aiResponse)
    action = act.group(1) if act else None

    with dbConnection() as db:
        cursor = db.cursor()
        cursor.execute("CALL StoreToAIChat(?)", (aiResponse,))
        cursor.callproc("StoreToChat", [action])
        db.commit()


def packageChatlog(num):
    with dbConnection() as db:
        cursor = db.cursor()
        cursor.callproc("GetRecentMsgIII", [num])
        rows = cursor.fetchall()

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

#Process AI Chatbot
async def convo(usrInput):
    history = chatHistory(5)  # list of tuples

    if history:
        last_user_msg, last_ai_msg = history[-1]
    else:
        last_user_msg = ""
        last_ai_msg = ""

    inputs = {
        "last_user_msg": last_user_msg,
        "last_ai_msg": last_ai_msg,
        "usr_input": usrInput
    }


    loop = asyncio.get_event_loop()


    result = await loop.run_in_executor(None, chain.invoke, inputs)
    return result


def execSim(response):
    saveChatLogs(response)
    if client:
        client.publish("lustrasim/lustratalk/response", packageChatlog(1))

def execWeb(response):
    saveChatLogs(response)
    if client:
        client.publish("webAi/chat/output", response)

async def main():
    with dbConnection() as db:
        cursor = db.cursor()
        cursor.execute("CALL GetInput()")
        UserInp = cursor.fetchone()[0]
        cursor.execute("CALL GetInputSource()")
        sourceInput = cursor.fetchone()[0]

    result = await convo(UserInp)

    if sourceInput == "simulator" and client:
        execSim(result)
    elif sourceInput == "website" and client:
        execWeb(result)
    else:
        saveChatLogs(result)

    processes = [
        subprocess.Popen(["notify-send", "From Lustra", result]),
        subprocess.Popen(["python", str(lustraVoiceFile)])    
    ]

    for process in processes:
        process.wait()

if __name__ == "__main__":
    asyncio.run(main())
    if client:
        client.loop_stop()
        client.disconnect()
