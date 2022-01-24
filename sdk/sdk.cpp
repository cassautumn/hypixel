#include "sdk.hpp"

auto sdk::get_request(std::string base, std::string path) -> std::string
{
	auto h_internet = InternetOpenA("Mozilla/5.0 (Windows NT 10.0) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/92.0.4515.107 Safari/537.36", INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, 0);
	auto h_connect = InternetConnectA(h_internet, base.data(), 80, NULL, NULL, INTERNET_SERVICE_HTTP, 0, NULL);

	const char* accept_types[] = { "text/*", 0 };

	auto h_request = HttpOpenRequestA(h_connect, "GET", path.data(), NULL, NULL, accept_types, 0, 0);
	auto request_sent = HttpSendRequestA(h_request, NULL, 0, NULL, 0);

	std::string response;

	auto keep_reading = true;
	unsigned long bytes_read = 1;

	char buf[1024];

	while (keep_reading && bytes_read != 0)
	{
		keep_reading = InternetReadFile(h_request, &buf, 1024, &bytes_read);
		response.append(buf, bytes_read);
	}

	return response;
}

auto sdk::split_string(std::string string, const char* delim) -> std::vector<std::string>
{
	std::vector<std::string> return_vec;

	unsigned long pos_start = 0;

	for (auto pos = string.find(delim, pos_start); pos != std::string::npos; pos = string.find(delim, pos_start))
		return_vec.emplace_back(string.substr(pos_start, pos - pos_start)), pos_start = pos + strlen(delim);

	return_vec.emplace_back(string.substr(pos_start));

	return return_vec;
}

auto sdk::open_window(HWND__* window) -> void*
{
	unsigned long pid = 0;

	GetWindowThreadProcessId(window, &pid);

	return OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
}

auto sdk::get_accessed_files(void* process) -> std::vector<std::string>
{
	std::vector<std::string> files;

	for (unsigned __int64 address = 0x34; address < 0x1fff; address += 4)
	{
		void* temp = nullptr;

		if (DuplicateHandle(process, (void*)address, (void*)-1, &temp, 0, 0, DUPLICATE_SAME_ACCESS))
		{
			std::string path(255, 0);

			if (GetFinalPathNameByHandleA(temp, &path[0], 255, FILE_NAME_NORMALIZED) != 0)
				files.emplace_back(path);
		}

		CloseHandle(temp);
	}

	return files;
}

/* client */

auto sdk::get_uuid_from_name(std::string name) -> std::string
{
	auto data = sdk::get_request("api.mojang.com", "/users/profiles/minecraft/" + name);

	if (data.size() != 0)
		return nlohmann::json::parse(data)["id"];

	return std::string{};
}

auto sdk::send_chat_message(__int32 key, std::string message) -> void
{
	static auto get_keyboard_layout = [&]() -> HKL
	{
		std::string name(9, 0);

		if (GetKeyboardLayoutNameA(&name[0]))
			return LoadKeyboardLayoutA(name.data(), 0);

		return nullptr;
	};

	static auto layout = get_keyboard_layout();

	keybd_event(key, 0, 0, 0), keybd_event(key, 0, 0x2, 0), Sleep(100);

	for (const auto& letter : message) 
	{
		auto keycode = VkKeyScanExA(letter, layout);

		if (keycode >> 8)
		{
			keybd_event(0x10, 0, 0, 0), keybd_event(keycode, 0, 0, 0), 
				keybd_event(keycode, 0, 0x2, 0), keybd_event(0x10, 0, 0x2, 0);

		} else keybd_event(keycode, 0, 0, 0), keybd_event(keycode, 0, 0x2, 0);
	}

	keybd_event(0x0d, 0, 0, 0), keybd_event(0x0d, 0, 0x2, 0);
}

auto sdk::is_in_menu(HWND__* window) -> bool
{
	tagCURSORINFO info;

	if (GetCursorInfo(&info)) 
		return (__int32)info.hCursor > 65000 && (__int32)info.hCursor < 66000;

	return false;
}

auto sdk::get_most_recent_game(std::string key, std::string uuid) -> nlohmann::json
{
	auto data = sdk::get_request("api.hypixel.net", "/recentgames?key=" + key + "&uuid=" + uuid);

	if (data.size() != 0)
		return nlohmann::json::parse(data)["games"][0];

	return nlohmann::json{};
}

auto sdk::get_current_game(std::string key, std::string uuid) -> nlohmann::json
{
	auto data = sdk::get_request("api.hypixel.net", "/status?key=" + key + "&uuid=" + uuid);

	if (data.size() != 0)
		return nlohmann::json::parse(data);

	return nlohmann::json{};
}

auto sdk::get_players(std::string path) -> std::vector<std::string>
{
	std::vector<std::string> players;

	for (; players.size() == 0; Sleep(500))
	{
		sdk::send_chat_message(0x54, "/who");

		std::ifstream log_file(path);

		auto data = std::string((
			std::istreambuf_iterator<char>(log_file)), (std::istreambuf_iterator<char>()));

		std::string line;

		for (auto pos = data.find_last_of("\n"); pos > 0; --pos)
		{
			line.push_back(data[pos - 1]);

			if (line[line.size() - 1] == '\n')
				break;
		}

		auto reversed = std::string(line.rbegin(), line.rend()); reversed.erase(reversed.size() - 1);

		if (reversed.find("[CHAT] ONLINE:") != std::string::npos)
			players = sdk::split_string(reversed.substr(reversed.find(" [CHAT] ONLINE: ") + 16), ", ");

		log_file.close();
	}

	return players;
}