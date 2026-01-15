#include <string>
#include <thread>
#include <chrono>
#include <Windows.h>
#include "steam_api.h"
#pragma comment(lib, "steam_api64.lib")

static constexpr AppId_t kTargetAppId = 282800;

class LobbyBrowser
{
public:
	LobbyBrowser();
	void Refresh();
private:
	STEAM_CALLBACK(LobbyBrowser, OnLobbyMatchList, LobbyMatchList_t, m_cbLobbyMatchList);
	STEAM_CALLBACK(LobbyBrowser, OnLobbyDataUpdate, LobbyDataUpdate_t, m_cbLobbyDataUpdate);
};

LobbyBrowser::LobbyBrowser(void)
	: m_cbLobbyMatchList(this, &LobbyBrowser::OnLobbyMatchList),
	m_cbLobbyDataUpdate(this, &LobbyBrowser::OnLobbyDataUpdate)
{
}

void LobbyBrowser::Refresh(void) 
{
	SteamMatchmaking()->AddRequestLobbyListDistanceFilter(k_ELobbyDistanceFilterWorldwide);
	SteamMatchmaking()->AddRequestLobbyListResultCountFilter(50);
	SteamMatchmaking()->RequestLobbyList();
	std::puts("[OJ] RequestLobbyList sent");
}

void LobbyBrowser::OnLobbyMatchList(LobbyMatchList_t *p) 
{
	printf("[OJ] Found %u lobbies\n", p->m_nLobbiesMatching);
	for (uint32 i = 0; i < p->m_nLobbiesMatching; i++)
	{
		CSteamID lobby = SteamMatchmaking()->GetLobbyByIndex(i);
		int members = SteamMatchmaking()->GetNumLobbyMembers(lobby);
		int limit = SteamMatchmaking()->GetLobbyMemberLimit(lobby);
		SteamMatchmaking()->RequestLobbyData(lobby);
	}
}

void LobbyBrowser::OnLobbyDataUpdate(LobbyDataUpdate_t *p) 
{
	if (!p->m_bSuccess)
		return;
	CSteamID lobby(p->m_ulSteamIDLobby);
	for (int i = 0; i < SteamMatchmaking()->GetLobbyDataCount(lobby); i++)
	{
		char key[256] = { 0 };
		char val[2048] = { 0 };
		if (SteamMatchmaking()->GetLobbyDataByIndex(lobby, i, key, sizeof(key), val, sizeof(val)))
		{
			//printf("%s: %s\n", key, val);
			switch (i)
			{
				case 6:
				{
					printf("\nPublic：%s\n", strcmp(val, "Yes") == 0 ? "Yes" : "No");
					break;
				}
				case 7:
				{
					printf("Lobby：%s\n", val);
					break;
				}
				case 9:
				{
					printf("Raw Lobby：%s\n", val);
					break;
				}
				case 10:
				case 11:
				case 12:
				case 13:
				{
					printf("Player：%s\n", val);
					break;
				}
				case 14:
				{
					printf("Players：%s/4\n", val);
					break;

				}
				case 15:
				{
					printf("Mode：");
					switch (atoi(val))
					{
						case 0:
						{
							puts("PVP");
							break;
						}
						case 3:
						{
							puts("Coop");
							break;
						}
						case 6:
						{
							puts("Bounty");
							break;
						}
						case 9:
						{
							puts("Xmas");
							break;
						}
					}
				}
				default:
					break;
			}
		}
	}
}

int main(void)
{
	SetConsoleOutputCP(CP_UTF8);
	SetConsoleCP(CP_UTF8);
	if (!SteamAPI_Init())
	{
		puts("SteamAPI_Init failed");
		return 1;
	}
	if (!SteamApps()->BIsSubscribedApp(kTargetAppId))
	{
		puts("You do not own 100% Orange Juice");
		return 2;
	}
	LobbyBrowser browser;
	browser.Refresh();
	for (int i = 0; i < 300; i++)
	{
		SteamAPI_RunCallbacks();
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}
	SteamAPI_Shutdown();
	getchar();
	return 0;
}