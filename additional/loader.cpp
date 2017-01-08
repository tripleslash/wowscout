#include <Windows.h>
#include "scout.h"

#include <iostream>
#include <string>
#include <thread>
#include <sstream>
#include <mutex>
#include <algorithm>
#include <cctype>
#include <chrono>
#include <future>
#include <map>

#include <boost/program_options.hpp>
#include <boost/filesystem.hpp>

std::map<int, ScoutContext*> context_map;

void attachProcess(int pid) {
    std::cout << "Attaching to process with id: " << pid << "..." << std::endl;

    ScoutContext* context = scoutAttachProcess((int)pid);
    if (context) {
        context_map.emplace(pid, context);
    } else {
        std::cerr << "Could not attach to process with id: " << pid << std::endl;
        std::cerr << "Last error: " << scoutErrorToString(scoutGetLastError()) << std::endl;
    }
}

BOOL CALLBACK windowEnumFunc(HWND hwnd, LPARAM lparam)
{
    wchar_t windowTitle[256];
    GetWindowTextW(hwnd, windowTitle, 256);

    std::wstring title = windowTitle;
    if (title == L"World of Warcraft") {
        DWORD pid = 0;
        GetWindowThreadProcessId(hwnd, &pid);

        attachProcess((int)pid);
    }

    return TRUE;
}

struct WindowCreateInfo {
    int pid;
    bool found;
};

BOOL CALLBACK checkWindowCreated(HWND hwnd, LPARAM lparam)
{
    wchar_t windowTitle[256];
    GetWindowTextW(hwnd, windowTitle, 256);

    std::wstring title = windowTitle;
    if (title == L"World of Warcraft") {
        DWORD pid = 0;
        GetWindowThreadProcessId(hwnd, &pid);

        auto createInfo = reinterpret_cast<WindowCreateInfo*>(lparam);
        if (pid == createInfo->pid) {
            createInfo->found = true;
            return FALSE;
        }
    }

    return TRUE;
}

BOOL WINAPI consoleCtrlHandler(DWORD ctrlType)
{
    return scoutShutdown();
}

void clear()
{
    COORD topLeft = { 0, 0 };
    HANDLE console = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_SCREEN_BUFFER_INFO screen;
    DWORD written;

    GetConsoleScreenBufferInfo(console, &screen);
    FillConsoleOutputCharacterA(
        console, ' ', screen.dwSize.X * screen.dwSize.Y, topLeft, &written
        );
    FillConsoleOutputAttribute(
        console, FOREGROUND_GREEN | FOREGROUND_RED | FOREGROUND_BLUE,
        screen.dwSize.X * screen.dwSize.Y, topLeft, &written
        );
    SetConsoleCursorPosition(console, topLeft);
}

namespace po = boost::program_options;
namespace fs = boost::filesystem;

int main(int argc, const char* argv[])
{
    std::wstring gamePathStr;
    std::vector<std::string> launchCommands;

    po::options_description desc{ "Allowed options" };
    desc.add_options()
        ("game,g", po::wvalue<std::wstring>(&gamePathStr), "game path")
        ("cmd,c", po::value<std::vector<std::string>>(&launchCommands)->multitoken(), "launch commands")
        ("help,h", "display help message");

    po::variables_map vm;

    try {
        po::store(po::parse_command_line(argc, argv, desc), vm);
        po::notify(vm);
    }
    catch (boost::program_options::error const& e) {
        std::cerr << "ERROR: " << e.what() << std::endl << std::endl;
        std::cerr << desc << std::endl;
        return EXIT_FAILURE;
    }

    if (vm.count("help")) {
        std::cout << desc << std::endl;
        return EXIT_SUCCESS;
    }

    int processId = 0;

    if (vm.count("game")) {
        fs::path gamePath{ gamePathStr };

        try {
            gamePath = fs::canonical(gamePath).make_preferred();
        } catch (fs::filesystem_error const& e) {
            std::cerr << "ERROR: " << e.what() << std::endl;
            return EXIT_FAILURE;
        }
        
        gamePathStr = gamePath.wstring();

        if (gamePathStr.length() >= MAX_PATH) {
            std::cerr << "ERROR: Game path needs to be shorter than MAX_PATH!" << std::endl;
            return EXIT_FAILURE;
        }

        STARTUPINFO startupInfo;
        PROCESS_INFORMATION processInfo;

        ZeroMemory(&startupInfo, sizeof(startupInfo));
        ZeroMemory(&processInfo, sizeof(processInfo));

        startupInfo.cb = sizeof(startupInfo);

        std::wostringstream woss;
        woss << L"\"" << gamePathStr << L"\" -console";

        std::vector<wchar_t> commandLine;
        std::wstring commandLineStr = woss.str();

        commandLine.assign(commandLineStr.begin(), commandLineStr.end());
        commandLine.push_back(0);

        if (!CreateProcess(gamePath.c_str(), commandLine.data(), NULL, NULL, FALSE, 0,
                NULL, gamePath.parent_path().c_str(), &startupInfo, &processInfo))
        {
            std::cerr << "ERROR: Failed to spawn new process!" << std::endl;
            return EXIT_FAILURE;
        }

        if (WaitForInputIdle(processInfo.hProcess, 10000)) {
            std::cerr << "ERROR: Process start timed out!" << std::endl;
            return EXIT_FAILURE;
        }

        processId = (int)processInfo.dwProcessId;

        WindowCreateInfo createInfo;
        createInfo.pid = processId;
        createInfo.found = false;
        
        while (!createInfo.found) {
            EnumWindows(checkWindowCreated, reinterpret_cast<LPARAM>(&createInfo));

            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
    }

    if (!scoutInit()) {
        std::cerr << "Could not initialize the library!" << std::endl;
        std::cerr << "Last error: " << scoutErrorToString(scoutGetLastError()) << std::endl;
        std::cin.get();
        return EXIT_FAILURE;
    }

    SetConsoleCtrlHandler(consoleCtrlHandler, TRUE);

    std::cout << "Scout version " << scoutGetVersion() << " initialized." << std::endl;

    if (processId != 0) {
        attachProcess(processId);
    } else {
        EnumWindows(windowEnumFunc, NULL);
    }

    std::cout << "Done. Type in exit to stop the program." << std::endl;

    if (context_map.size() == 1) {
        auto context = context_map.begin();
        std::cout << "Setting context to " << context->first << "..." << std::endl;
        scoutSetProcessContext(context->second);
    } else {
        std::cout << "Setting context to NULL..." << std::endl;
        scoutSetProcessContext(NULL);
    }

    if (vm.count("cmd")) {
        for (auto& command : launchCommands) {
            std::cout << "Executing \"" << command << "\"..." << std::endl;
            scoutExecuteCommand(command.c_str());
        }
    }

    std::cout << "===================================" << std::endl;

    {
        struct console_command {
            std::string cmd;
            std::string args;
            std::string input;
            std::promise<void> promise;
        };

        std::mutex console_mutex;
        console_command current_command;

        volatile bool running = true;

        std::thread console([&] {
            std::cout << "> ";

            std::string input;
            while (std::getline(std::cin, input)) {
                std::cin.clear();
                std::cin.sync();

                std::istringstream ss{ input };

                std::string cmd;
                if ((ss >> cmd) && !cmd.empty()) {
                    std::transform(cmd.begin(), cmd.end(), cmd.begin(), ::tolower);

                    if (cmd == "exit") {
                        break;
                    }

                    std::string args;
                    std::getline(ss, args);

                    console_command command{ cmd, args, input };
                    auto future = command.promise.get_future();

                    {
                        std::lock_guard<std::mutex> lock{ console_mutex };
                        current_command = std::move(command);
                    }

                    future.get();
                }

                std::cout << "> ";
            }
            running = false;
        });

        while (running) {
            {
                std::lock_guard<std::mutex> lock{ console_mutex };

                // process commands
                if (!current_command.cmd.empty()) {
                    std::istringstream ss{ current_command.args };

                    if (current_command.cmd == "setproc") {
                        int pid;
                        if (ss >> pid) {
                            std::cout << "Setting process context to " << pid << "." << std::endl;
                            scoutSetProcessContext(scoutGetContextForPid(pid));
                        } else {
                            std::cout << "No pid given. Setting context to NULL." << std::endl;
                            scoutSetProcessContext(NULL);
                        }
                    } else if (current_command.cmd == "cls" || current_command.cmd == "clear") {
                        scoutExecuteCommand("clear");
                        clear();
                    } else {
                        scoutExecuteCommand(current_command.input.c_str());
                    }
                }

                // read console messages
                char logEntries[65536];
                int length = scoutFetchLogEntries(logEntries, sizeof(logEntries));

                if (length > 0) {
                    std::string log{ logEntries, static_cast<size_t>(length) };
                    std::istringstream ss{ log };

                    std::string line;
                    while (std::getline(ss, line)) {
                        std::cout << line << std::endl;
                    }
                }

                if (!current_command.cmd.empty()) {
                    current_command.promise.set_value();
                    current_command.cmd.clear();
                }
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        }

        console.join();
    }

    if (!scoutShutdown()) {
        std::cerr << "Could not shutdown the library!" << std::endl;
        std::cerr << "Last error: " << scoutErrorToString(scoutGetLastError()) << std::endl;
        std::cin.get();
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
