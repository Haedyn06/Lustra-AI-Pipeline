import textwrap
import mariadb
from dotenv import load_dotenv
import os

# Get ENV
load_dotenv()
dbHost = os.getenv("DB_HOST")
dbUser = os.getenv("DB_USER")
dbPw = os.getenv("DB_PASSWORD")
dbName = os.getenv("DB_NAME")


#Initiate DB
conn = mariadb.connect(host=dbHost, user=dbUser, password=dbPw, database=dbName)
cursor = conn.cursor()

#30x5 per text line
def newLine(sentence, max_length=185):
    return "\n".join(textwrap.wrap(sentence, width=max_length))

#Get AI Response
cursor.execute("CALL GetResponse()")
rows = cursor.fetchall()

#Format For GUI
formatSentence = newLine(rows[0][0])
print(formatSentence)