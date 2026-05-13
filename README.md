# AI LUSTRA COMMAND PIPELINE

**Note:** Restructuring and Optimization is still in progress, so expect some bugs and unoptimized code. Had to redo some code due to many private information being exposed, so I am still in the process of re-implementing some features and optimizing the code. This was created on Apr 12, 2025. If you want to contribute or have any suggestions, feel free to open an issue or a pull request!

### Features
- Voice Commands (Only in Arch Linux)
- Desktop Commands (Only in Arch Linux)
- Generative AI Responses
- Compatible in a Website, Terminal, or Unity Game Simulation
- Logs Chats
- Works Offline


### To Be Added/Implemented

#### In Progress/Refactoring
- Supported Unity Game Simulation
- Supported Website + Web Server
- Raspberry Pi + IoT Hardware Support
- Working Commands + Voice Commands
- Clear & Clean Setup Instructions + Documentation

#### Under Consideration
- Personalization Options
- Improved Database Structure
- Claw Code Implementation


### Setup (Unfinished)

#### Requirements:
- C++20 Compiler
- MySQL Database
- Whisper.cpp (For Speech to Text)
- ElevenLabs API Keys (For Text to Speech)


1. Clone the Repository
2. Create a .env file in the root directory with the following variables:
```
DB_HOST="Database Host"
DB_USER="Database username"
DB_PASSWORD="Database Password"
DB_NAME="Database Name"

SERVER_IP="SERVER IP"
SERVER_PORT="SERVER PORT"


HYPR_CONFIG_DIR="HYPRLAND CONFIG PATH"
PROJECT_DIR="THIS REPO DIRECRTORY PATH"

WHISPER_MODEL="WHISPER MODEL PATH"
WHISPER_CLI="WHISPER CLI PATH"

ELEVENLABS_KEYS=["LIST OF API KEYS"]
```
3. Build the project using the following command:

**For Linux Only**
```
g++ -std=c++20 main.cpp src/cmd/Commands.cpp src/cmd/VoiceCmd.cpp -o build/voiceCommands -lfmt `mysql_config --cflags --libs`
```
...

**To Be Continued...**






