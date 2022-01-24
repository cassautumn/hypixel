#define _CRT_SECURE_NO_WARNINGS

#include <windows.h>
#include <iostream>
#include <fstream>
#include <thread>
#include <algorithm>
#include <filesystem>
#include <vector>
#include <wininet.h>
#include <psapi.h>

#pragma comment(lib, "wininet")

#include "misc/json.hpp"

namespace sdk
{
	extern auto get_request(std::string base, std::string path)->std::string;

	extern auto split_string(std::string string, const char* delim) -> std::vector<std::string>;

	extern auto open_window(HWND__* window) -> void*;

	extern auto get_accessed_files(void* process) -> std::vector<std::string>;
}

namespace sdk
{
	extern auto get_uuid_from_name(std::string name) -> std::string;

	extern auto send_chat_message(__int32 key, std::string message) -> void;

	extern auto is_in_menu(HWND__* window) -> bool;

	extern auto get_most_recent_game(std::string key, std::string uuid) -> nlohmann::json;

	extern auto get_current_game(std::string key, std::string uuid) -> nlohmann::json;

	extern auto get_players(std::string log_buf) -> std::vector<std::string>;
}