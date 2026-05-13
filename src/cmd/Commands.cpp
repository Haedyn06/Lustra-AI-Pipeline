#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <algorithm>
#include <cstdlib>
#include <cctype> 
#include <map>
#include <chrono>
#include <fmt/core.h>
#include <signal.h>
#include <unistd.h>
#include <mysql/mysql.h>
#include <ctime>
#include <filesystem>
#include <sstream>
#include <iomanip>

// File Paths
#include "./include/commands.h"
#include "./include/json.hpp"

//Binary File and Othe shit
using json = nlohmann::json;
using namespace std;

unordered_map<string, string> loadEnv(const string& path);
const string baseDir = filesystem::path(__FILE__).parent_path().string();
unordered_map<string, string> env = loadEnv(baseDir + "/../../.env");

string dbHost = env["DB_HOST"];
string dbUser = env["DB_USER"];
string dbPw   = env["DB_PASSWORD"];
string dbName = env["DB_NAME"];

string projectDir = env["PROJECT_DIR"];

// DATA DECLARATION //
map<string, vector<string>> openCode = { //For Coding
    {"hypr", {"hyper land", "hyperland", "hyper"}},
    {"rofi", {"rofi"}},
    {"waybar", {"waybar"}},
    {"eww", {"eww"}}
};
map<string, vector<string>> goApps = { //Look or Open For Apps
    {"Obsidian", {"notes", "note"}},
    {"Dolphin", {"file", "dolphin"}},
    {"Spotify", {"music", "spotify", "listen", "song"}},
    {"Gimp", {"art", "edit", "gimp", "draw"}},
    {"Teams", {"team", "teams"}},
    {"Virtualbox", {"vbox", "vm", "virtual", "emulate"}},
    {"Discord", {"discord"}}
};
map<string, vector<string>> openApps = { //Open Generic Apps
    {"chromium", {"browser", "web", "chrome", "google"}},
    {"firefox", {"firefox", "web2"}},
    {"alacritty", {"terminal", "alacritty", "term", "cmd"}},
    {"code", {"code", "vs", "vscode", "coding"}},
    {"chromium https://chatgpt.com/", {"gpt", "jippity", "chatgpt"}},
    {"chromium https://github.com/", {"get hub", "get", "github", "git"}},
    {"chromium https://www.instagram.com/", {"insta", "instagram", "message"}},
    {"chromium https://www.youtube.com/", {"youtube", "watch"}}
};

string getCurrentTime() {
    time_t now = time(0);
    tm* localTime = localtime(&now);

    int hour = localTime->tm_hour;
    string ampm = (hour >= 12) ? "PM" : "AM";
    hour = (hour % 12 == 0) ? 12 : (hour % 12);

    ostringstream oss;
    oss << setfill('0') << setw(2) << hour << ":"
        << setw(2) << localTime->tm_min << " " << ampm;

    return oss.str();
}

string getCurrentDate() {
    time_t now = time(0);
    tm* localTime = localtime(&now);

    char buffer[100];
    strftime(buffer, sizeof(buffer), "%b %d, %Y", localTime);
    return string(buffer);
}


//Map Declaration
map<string, string> codePaths, appPaths, applicationPaths;

// METHODS //
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

void activateLustra(string msgTxt) {
    MYSQL* conn = mysql_init(nullptr);
    if (!mysql_real_connect(conn, dbHost.c_str(), dbUser.c_str(), dbPw.c_str(), dbName.c_str(), 0, nullptr, 0)) {
        cerr << "❌ CommandLog DB Error: " << mysql_error(conn) << endl;
        return;
    }

    string storeInput;
    storeInput = "CALL StoreToUserChat('" + string(msgTxt) + "', 'MainSpeech');";
    
    mysql_query(conn, storeInput.c_str());
    mysql_close(conn);
    // return result;
}

void logCommandUsage() {
    MYSQL* conn = mysql_init(nullptr);
    if (!mysql_real_connect(conn, dbHost.c_str(), dbUser.c_str(), dbPw.c_str(), dbName.c_str(), 0, nullptr, 0)) {
        cerr << "❌ CommandLog DB Error: " << mysql_error(conn) << endl;
        return;
    }

    mysql_query(conn, "CALL storeToCommandLogs();");
    mysql_close(conn);
}


string lowered(const string& msg) { //Lower Input For Easier Readability
    string lowerCased = msg;
    transform(lowerCased.begin(), lowerCased.end(), lowerCased.begin(), ::tolower);
    return lowerCased;
}

void generateMappings() { //Loop Thorugh to Check Right Command Via Output
    for (const auto& [key, values] : openCode)
        for (const auto& alias : values) codePaths[alias] = env["HYPR_CONFIG_DIR"] + key;

    for (const auto& [key, values] : goApps)
        for (const auto& alias : values) appPaths[alias] = key;

    for (const auto& [key, values] : openApps) 
        for (const auto& alias : values) applicationPaths[alias] = key;
}

bool appList(int workspaceNum, const string& app) { //No clue
    FILE* pipe = popen("hyprctl clients", "r");
    if (!pipe) return false;

    char buffer[256];
    vector<string> lines;

    while (fgets(buffer, sizeof(buffer), pipe) != nullptr)
        lines.emplace_back(buffer);
    pclose(pipe);

    vector<string> appWorkspace;
    string workspaceLine = "workspace: " + to_string(workspaceNum) + " ";

    for (size_t i = 0; i < lines.size(); ++i) {
        if (lines[i].find(workspaceLine) != string::npos) {
            for (size_t j = i; j < min(i + 8UL, lines.size()); ++j) {
                if (lines[j].find("class:") != string::npos) {
                    string appClass = lines[j].substr(lines[j].find(":") + 1);
                    appClass.erase(remove(appClass.begin(), appClass.end(), '\n'), appClass.end());
                    appClass.erase(remove(appClass.begin(), appClass.end(), ' '), appClass.end());
                    appWorkspace.push_back(appClass);
                }
            }
        }
    }

    cout << "Apps in workspace " << workspaceNum << ": ";
    if (!appWorkspace.empty()) {
        for (const auto& a : appWorkspace) cout << a << ", ";
    } else {
        cout << "None";
    }
    cout << endl;

    if (find(appWorkspace.begin(), appWorkspace.end(), app) != appWorkspace.end()) {
        cout << "Over Here" << endl;
        return true;
    } else {
        cout << "Not here" << endl;
        return false;
    }
}

bool findApp(const string& appToOpen) { //Find app in workspaces
    for (int i = 1; i <= 10; ++i) {
        if (appList(i, appToOpen)) {
            string cmd = "hyprctl dispatch workspace " + to_string(i);
            system(cmd.c_str());
            return true;
        }
    }
    return false;
}

void openApp(const string& appToOpen) { //Open app
    if (!findApp(appToOpen)) {
        cout << appToOpen << " not found in any workspace" << endl;
        string cmd = lowered(appToOpen);
        system((cmd + " &").c_str());
    }
}

void switchWorkspace(string& msg) { //Switch to each workspace via command from map
    if (msg.find("after space") != string::npos) {
        logCommandUsage();
        system("hyprctl dispatch workspace e+1");
        return;
    }
    if (msg.find("before space") != string::npos) {
        logCommandUsage();
        system("hyprctl dispatch workspace e-1");
        return;
    }


    map<string, string> wordToDigit = {
        {"one", "1"}, {"one.", "1"}, 
        {"two", "2"}, {"two.", "2"}, {"too", "2"}, {"too.", "2"}, 
        {"three", "3"}, {"tree", "3"}, {"three.", "3"}, {"tree.", "3"},
        {"four", "4"}, {"for", "4"}, {"four.", "4"}, {"for.", "4"}, 
        {"five", "5"}, {"five.", "5"},
        {"six", "6"}, {"sick", "6"}, {"six.", "6"}, {"sick.", "6"}, 
        {"seven", "7"}, {"seven.", "7"}, 
        {"eight", "8"}, {"ate", "8"}, {"it", "8"}, {"eight.", "8"}, {"ate.", "8"}, {"it.", "8"},
        {"nine", "9"}, {"nine.", "9"}, 
        {"ten", "10"}, {"den", "10"}, {"ten.", "10"}, {"den.", "10"}
    };

    for (const auto& [word, digit] : wordToDigit) {
        if (msg.find(word) != string::npos) {
            logCommandUsage();
            msg.replace(msg.find(word), word.length(), digit);
            break;
        }
    }

    string num = "";
    vector<string> nums = {"6", "7", "8", "9", "10"};
    for (char c : msg) if (isdigit(c)) num += c;

    if (msg.find("devws") != string::npos || msg.find("dev space") != string::npos || msg.find("developer space") != string::npos) {
        logCommandUsage();
        system(fmt::format("hyprctl dispatch workspace {}", num).c_str());
    } else if (msg.find("sysws") != string::npos || msg.find("system space") != string::npos || msg.find("sys space") != string::npos) {
        logCommandUsage();
        if (!num.empty()) {
            int index = stoi(num) - 1;
            if (index >= 0 && index < nums.size()) system(fmt::format("hyprctl dispatch workspace {}", nums.at(index)).c_str());
        }
    }  
}

void audioControl(string& lowInp) { //Control music and volume

    if (lowInp.find("replay") != string::npos) {
        logCommandUsage();
        system("playerctl -p spotify previous");
    } 
    else if (lowInp.find("pause") != string::npos || lowInp.find("play") != string::npos) {
        logCommandUsage();
        system("playerctl -p spotify play-pause");
    } 
    else if (lowInp.find("go next") != string::npos) {
        logCommandUsage();
        system("playerctl -p spotify next");
    } 
    else if (lowInp.find("go back") != string::npos) {
        logCommandUsage();
        system("playerctl -p spotify previous; sleep 0.1; playerctl -p spotify previous");
    } 
    if (lowInp.find("volume") != string::npos) {
        logCommandUsage();
        string volNum = "";
        for (char l : lowInp) if (isdigit(l)) volNum += l;
        string volumeSet = "pactl set-sink-volume @DEFAULT_SINK@ " + volNum + "%";
        system(volumeSet.c_str());
    }
}

void PowerMenu(string& lowInp){ //Control Power
    if (lowInp.find("shut down") != string::npos || lowInp.find("power off") != string::npos) {
        logCommandUsage();
        system("systemctl poweroff");
    } else if (lowInp.find("restart pc") != string::npos) {
        logCommandUsage();
        system("systemctl reboot");
    } else if (lowInp.find("sleep pc") != string::npos) {
        logCommandUsage();
        system("systemctl suspend");
    } else if (lowInp.find("logout pc") != string::npos){
        logCommandUsage();
        system("hyprctl dispatch exit");
    } 
}

void SelectWindow(string& lowInp){ //Select Acrtive Window    
    
    if (lowInp.find("hyper right") != string::npos || lowInp.find("hyper write") != string::npos) {
        logCommandUsage();
        system("hyprctl dispatch movefocus r");
    } else if (lowInp.find("hyper left") != string::npos) {
        logCommandUsage();
        system("hyprctl dispatch movefocus l");
    } else if (lowInp.find("hyper up") != string::npos) {
        logCommandUsage();
        system("hyprctl dispatch movefocus u");
    } else if (lowInp.find("hyper down") != string::npos) {
        logCommandUsage();
        system("hyprctl dispatch movefocus d");
    }
    if (lowInp.find("hyper close") != string::npos || lowInp.find("close window") != string::npos || lowInp.find("hyperclose") != string::npos || lowInp.find("hyperclothes") != string::npos) {
        logCommandUsage();
        system("hyprctl dispatch killactive");
    }
}

void ManageWidget(string& msg){ //Open or Close Widget Dashboard

    if (msg.find("widget on") != string::npos || msg.find("dashboard on") != string::npos || msg.find("open dashboard") != string::npos) {
        logCommandUsage();
        system("hyprctl dispatch exec 'eww daemon && eww update && eww open Widget1 && eww open Widget2 && eww open Widget3 && eww open Widget4 && eww open Widget5 && eww open Widget6 && eww open Widget7'");
    } else if (msg.find("widget off") != string::npos || msg.find("dashboard off") != string::npos || msg.find("close dashboard") != string::npos) {
        logCommandUsage();
        system("pkill eww");
    } 
}

void GetApps(string& lowInp) { //Find Apps
    for (const auto& [alias, path] : codePaths) if (lowInp.find(alias) != string::npos) system(("code " + path).c_str());
    if (lowInp.find("open") != string::npos || lowInp.find("go to") != string::npos || lowInp.find("look for") != string::npos) {
        logCommandUsage();
        for (const auto& [alias, appName] : appPaths) if (lowInp.find(alias) != string::npos) openApp(appName);
        for (const auto& [alias, cmd] : applicationPaths) if (lowInp.find(alias) != string::npos) system(cmd.c_str());
    }
}

void getTime(string& msg) {
    playRandomAudio(projectDir + "/assets/audios/LustraWait");
    if (msg.find("time") != string::npos) {
        logCommandUsage();
        system("hyprctl dispatch workspace e+1");
        return;
    } else if (msg.find("date") != string::npos) {
        logCommandUsage();
        system("hyprctl dispatch workspace e-1");
        return;
    } else if (msg.find("weather") != string::npos) {
        logCommandUsage();
        system("hyprctl dispatch workspace e-1");
        return;
    } else if (msg.find("news") != string::npos) {
        logCommandUsage();
        system("hyprctl dispatch workspace e-1");
        return;
    } else if (msg.find("date") != string::npos) {
        logCommandUsage();
        system("hyprctl dispatch workspace e-1");
        return;
    }
}

void CmdExc(string& input) { //Compiled Commands within it corresponding functions
    generateMappings();
    string lowInp = lowered(input);

    GetApps(lowInp);
    switchWorkspace(lowInp);
    PowerMenu(lowInp);
    audioControl(lowInp);
    SelectWindow(lowInp);
    ManageWidget(lowInp);
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
