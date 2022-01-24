#include "sdk/sdk.hpp"

auto main() -> __int32
{
	const auto api = "xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx";
	const auto target = "player";

	auto window = FindWindowA("LWJGL", 0);

	if (window != 0)
	{
		static const auto get_latest_path = [&]() -> std::string
		{
			std::string return_path;

			auto files = sdk::get_accessed_files(sdk::open_window(window));

			for (const auto& file : files)
			{
				if (file.find("latest.log") != std::string::npos)
					return_path = file;
			}

			return return_path.erase(0, 4);
		};

		static const auto path = get_latest_path();

		auto uuid = sdk::get_uuid_from_name(target);

		std::cout << "sniping " << target << " - " << uuid << std::endl;

		for (; window; Sleep(500))
		{
			if (GetForegroundWindow() != window)
				continue;

			if (sdk::is_in_menu(window))
				continue;

			auto status = sdk::get_current_game(api, uuid);

			if (status["success"])
			{
				if (status["session"]["online"])
				{
					auto changed_games = [&]() -> bool
					{
						auto changed = false;

						/* pass 1 */
						{
							static std::string previous_game_data;

							if (previous_game_data.size() != 0 && sdk::get_most_recent_game(api, uuid).dump() != previous_game_data)
								std::cout << "changed lobbies, pass 1" << std::endl, changed = true;

							previous_game_data = sdk::get_most_recent_game(api, uuid).dump();

						}

						/* pass 2 */
						{
							static std::string last_mode;

							if (last_mode.size() != 0 && last_mode != status["session"]["mode"] && status["session"]["mode"] != "LOBBY")
								std::cout << "changed lobbies, pass 2" << std::endl, changed = true;

							last_mode = status["session"]["mode"];
						}

						return changed;
					};

					if (changed_games(/* check if player has changed games */))
					{
						std::string mode = status["session"]["mode"];

						for (auto x = 0; x < mode.size(); ++x)
							mode[x] = std::tolower(mode[x]);

						for (auto x = 0; x < 10 /* timeout */; ++x)
						{
							sdk::send_chat_message(0x54, "/play " + mode), Sleep(100);

							auto players = sdk::get_players(path);

							if (std::find(players.begin(), players.end(), target) != players.end())
							{
								std::cout << "sniped player lobby" << std::endl; break;
							}
						}
					}
				} 
				else std::cout << "offline" << std::endl;
			}
			else
			{
				if (status["throttle"])
					std::cout << "rate limited" << std::endl, Sleep(2000);

				continue;
			}
		}
	}

	return std::cin.get() != 0;
}
