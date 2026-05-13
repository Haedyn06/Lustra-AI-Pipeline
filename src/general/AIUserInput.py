import os
import sys
import subprocess
import mariadb
from dotenv import load_dotenv
from pathlib import Path

load_dotenv()
dbHost = os.getenv("DB_HOST")
dbUser = os.getenv("DB_USER")
dbPw = os.getenv("DB_PASSWORD")
dbName = os.getenv("DB_NAME")



#File Path Declaration
BASE_DIR = Path(__file__).resolve().parents[2]
waitAudio = BASE_DIR / "assets" / "audios" / "pleaseWait.mp3"
triggerAI = os.getenv("TRIGGER_AI")

#Initiate DB
conn = mariadb.connect(host=dbHost, user=dbUser, password=dbPw, database=dbName)
cursor = conn.cursor()

#Get Response From DB
cursor.execute("CALL GetResponse()")
respond = cursor.fetchall()

#Avoid Double Input
if respond[0][0] == "Lustra is responding..":
    subprocess.Popen(["ffplay", "-nodisp", "-autoexit", waitAudio])
    subprocess.Popen(["notify-send", "From Lustra", "Well, well, it seems like you're trying to test my patience. " "But let me remind you that good things come to those who wait. " "So if you really want a response from me, you'll just have to be patient and wait for it like the little minx you are."])
else:
    #Send To Main AI Executions
    if len(sys.argv) > 1:
        UserInp = " ".join(sys.argv[1:]).strip()
        
        cursor.callproc("StoreToUserChat", [UserInp, "main"])
        cursor.execute("CALL StoreToAIChat(?)", ("Lustra is responding..",))
        conn.commit()
        subprocess.run(triggerAI, shell=True)

cursor.close()
conn.close()