#include <iostream>
#include <fstream>
#include <filesystem>
#include <vector>
#include <string>
#include <algorithm>
#include <cstdlib>
#include <mysql/mysql.h>
#include <cctype> 
#include <map>
#include <unordered_map>
#include <thread>
#include <chrono>
#include <sstream>
#include "./include/commands.h"

using namespace std;

//Words to activate lustra
vector<string> wakeWords = {"lust", "neustra", "lucian", "nastra", "lustre", "lestra", "nastya", "lustra", "lister", "luster", "lucifer", "lester", "mr.", "mr", "mister", "luca", "nuster", "newster", "loose row", "loose", "rooster", "master", "sweetie", "nester", "mooster"};
vector<string> talkWords = {"speak", "speak to you", "talk", "talk to you", "chat"};
string getCmdResult;




//Variable & Function Declarations
// void playRandomAudio(string folderPath);
void transcribeSpeech(const string& whisperCLI, const string& modelPath, string duration, string which);
bool wordDetected(const string& speechWord, vector<string> typaWords);
int Inititations(MYSQL *conn);
void getTempCommand(string which);
unordered_map<string, string> loadEnv(const string& path);
void playRandomAudio(string folderPath);

//File Path Declaration
const string baseDir = filesystem::path(__FILE__).parent_path().string();
auto env = loadEnv(baseDir + "/../../.env");

string dbHost = env["DB_HOST"];
string dbUser = env["DB_USER"];
string dbPw   = env["DB_PASSWORD"];
string dbName = env["DB_NAME"];

string projectDir = env["PROJECT_DIR"];

const string modelPath = env["WHISPER_MODEL"];
const string whisperCLI = env["WHISPER_CLI"];
const string checkFile = projectDir + "/assets/audios/checkMessage.wav";
const string audioFile = projectDir + "/assets/audios/ResponseOut.mp3";
const string wakeAudioDir = projectDir + "/assets/audios/LustraWake";



//Main Execution
int main() {
    MYSQL* conn = mysql_init(nullptr);
    if (Inititations(conn)) return 1;
    
    while (true) {    //Listen for words
        cout << "Listening..." << endl;
        transcribeSpeech(whisperCLI, modelPath, "2", "cmdVoice");
        getTempCommand("cmdVoice");

         //Call and check for valid commands

        if (wordDetected(getCmdResult, wakeWords)) { //Listen for words to activate Lustra
            playRandomAudio(wakeAudioDir);
            transcribeSpeech(whisperCLI, modelPath, "3", "cmdVoice");
            getTempCommand("cmdVoice");
            CmdExc(getCmdResult);
            if (wordDetected(getCmdResult, talkWords)) {
                string cmd = "ffplay -nodisp -autoexit \"" + audioFile + "\" > /dev/null 2>&1";

                system(cmd.c_str());
                transcribeSpeech(whisperCLI, modelPath, "16", "lustraVoice");
                getTempCommand("lustraVoice");
                system("notify-send 'From Lustra' 'Lustra is responding..'");
                system(env["TRIGGER_AI"].c_str());
            }
        }
        this_thread::sleep_for(std::chrono::milliseconds(500)); //Save processing power  
    }
    return 0;
}

int Inititations(MYSQL *conn) { //Initiate DB
    if (!mysql_real_connect(conn, dbHost.c_str(), dbUser.c_str(), dbPw.c_str(), dbName.c_str(), 0, nullptr, 0)) {
        cerr << "Gone \n";
        return 1;
    }
    return 0;
}

//Functions
bool wordDetected(const string& speechWord, vector<string> typaWords) {
    string lowerCased = speechWord;
    transform(lowerCased.begin(), lowerCased.end(), lowerCased.begin(), ::tolower);
    for (const string& word : typaWords) if (lowerCased.find(word) != string::npos) return true;
    return false;
}

void playRandomAudio(string folderPath) {
    vector<string> audioFiles;

    for (const auto& entry : filesystem::directory_iterator(folderPath)) {
        if (entry.path().extension() == ".mp3") audioFiles.push_back(entry.path());
    }

    if (audioFiles.empty()) {
        cout << "No audio files found in " << folderPath << endl;
        return;
    }

    srand(time(0));
    int randomIndex = rand() % audioFiles.size();
    string selectedFile = audioFiles[randomIndex];

    string command = "ffplay -nodisp -autoexit \"" + selectedFile + "\" > /dev/null 2>&1";
    system(command.c_str());
}

void transcribeSpeech(const string& whisperCLI, const string& modelPath, string duration, string which) { // Audio to Text Processing
    //Initiate DB
    MYSQL *conn;
    MYSQL_ROW row;
    MYSQL_RES *res;
    string message;
    conn = mysql_init(nullptr);
    if (Inititations(conn)) return;
    
    string storeInput, msgTxt; //Trasnscribed Message Storage
    string msgcmd = whisperCLI + " -m " + modelPath + " -f " + checkFile + " -nt"; //Transcribe Message Command

    //Record Voice Via Audio File
    cout << "\nRecording.. \n";
    string recording = "ffmpeg -f alsa -i default -t " + duration + " -ac 1 -ar 16000 -y " + checkFile + " > /dev/null 2>&1"; 
    system(recording.c_str());
    
    //Transcribe Audio File Initiate Whisper API
    cout << "Transcribing..\n";
    
    FILE* pipe = popen(msgcmd.c_str(), "r"); //Acvtivate Command
    char buffer[256];
    while (fgets(buffer, sizeof(buffer), pipe)) msgTxt += buffer;
    pclose(pipe);

    char* txtSpoken = new char[2 * msgTxt.length() + 1];
    mysql_real_escape_string(conn, txtSpoken, msgTxt.c_str(), msgTxt.length());

    //Store to DB
    if (which == "lustraVoice") storeInput = "CALL StoreToUserChat('" + string(txtSpoken) + "', 'MainSpeech');";
    else if (which == "cmdVoice") storeInput = "CALL storeToTempSpeech('" + string(txtSpoken) + "');";
    mysql_query(conn, storeInput.c_str());
    delete[] txtSpoken;
}

void getTempCommand(string which) {
    MYSQL *conn;
    MYSQL_ROW row;
    MYSQL_RES *res;
    string message;
    conn = mysql_init(nullptr);
    if (Inititations(conn)) return;
    string result;

    if (which == "lustraVoice"){
        mysql_query(conn, "CALL StoreToAIChat('Lustra is responding..');");
        mysql_query(conn, "CALL GetInput();");
        res = mysql_store_result(conn);
    } else if (which == "cmdVoice") {
        mysql_query(conn, "CALL getTempSpeech();");
        res = mysql_store_result(conn);
        while ((row = mysql_fetch_row(res))) result = row[0];
    }

    mysql_free_result(res);
    mysql_close(conn);
    cout << result << endl;
    getCmdResult = result;
    // return result;
}


unordered_map<string, string> loadEnv(const string& path) {
    unordered_map<string, string> env;
    ifstream file(path);

    string line;

    while (getline(file, line)) {
        stringstream ss(line);

        string key, value;

        if (getline(ss, key, '=') &&
            getline(ss, value)) {
            env[key] = value;
        }
    }

    return env;
}