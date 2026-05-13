import mariadb
import pygame
from gtts import gTTS
import os
from elevenlabs import save
from elevenlabs.client import ElevenLabs
from elevenlabs.core.api_error import ApiError
from dotenv import load_dotenv
import json
from pathlib import Path

load_dotenv()
dbHost = os.getenv("DB_HOST")
dbUser = os.getenv("DB_USER")
dbPw = os.getenv("DB_PASSWORD")
dbName = os.getenv("DB_NAME")

apiKeys = json.loads(os.getenv("ELEVENLABS_KEYS", "[]"))

BASE_DIR = Path(__file__).resolve().parents[2]
responseAudio = BASE_DIR / "assets" / "audios" / "ResponseOut.mp3"
textResponse = BASE_DIR / "data" / "tempRespond.txt"

#PLAY AUDIO
def playAudio(audFile=responseAudio):
    pygame.mixer.pre_init()
    pygame.mixer.init()
    pygame.mixer.music.load(audFile)
    pygame.mixer.music.play()
    while pygame.mixer.music.get_busy():
        pygame.time.Clock().tick(10)


#DATABASE INITIATION
with open(textResponse, "r") as file: currentMessage = file.read()

conn = mariadb.connect(host=dbHost, user=dbUser, password=dbPw, database=dbName)
cursor = conn.cursor()
cursor.execute("CALL GetResponse()")
message = cursor.fetchall()

#VOICE EXECUTION
success = False

if currentMessage == message[0][0]:
    print("Current Message Playing..")
    playAudio()
else:
    for apiKey in apiKeys:
        try:
            client = ElevenLabs(api_key=apiKey)
            response = message[0][0]

            audio = client.text_to_speech.convert(
                voice_id="0TfZ4rvne3QI7UjDxVkM",
                model_id="eleven_multilingual_v2",
                text=response,
                output_format="mp3_44100_128",
            )

            with open(textResponse, "w") as file:
                file.write(response)

            save(audio, responseAudio)
            success = True
            break

        except ApiError as err:
            print(err)
            continue
        
    if not success: #Switch to Google TTS
        tts = gTTS(message[0][0], lang='en')
        tts.save(responseAudio)

    playAudio()
