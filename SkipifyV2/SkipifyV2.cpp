#include "pch.h"
#include "SkipifyV2.h"


BAKKESMOD_PLUGIN(SkipifyV2, "Skipify REVAMPED", plugin_version, PERMISSION_REPLAY)

std::shared_ptr<CVarManagerWrapper> _globalCvarManager;

void SkipifyV2::onLoad()
{
	_globalCvarManager = cvarManager;
	gameWrapper->HookEvent("Function GameEvent_Soccar_TA.ReplayPlayback.ShouldPlayReplay", std::bind(&SkipifyV2::Skip, this));

	cvarManager->registerNotifier("toggleskipreplay", [this](std::vector<std::string> params) {
		if (ShouldSkipReplay = !ShouldSkipReplay) {
			gameWrapper->ExecuteUnrealCommand("ReadyUp");

			gameWrapper->HookEvent("Function GameEvent_Soccar_TA.ReplayPlayback.ShouldPlayReplay", std::bind(&SkipifyV2::Skip, this));

			if (ShouldShowNotification)
			gameWrapper->Toast("SkipifyV2", "Auto Skipping is now Enabled!", "", 2.0f);
			
			if (ShouldLogInChat)
			gameWrapper->LogToChatbox("Auto Skipping is now Enabled!", "Skipify");
		}
		else {
			gameWrapper->UnhookEvent("Function GameEvent_Soccar_TA.ReplayPlayback.ShouldPlayReplay");
			
			if (ShouldShowNotification)
			gameWrapper->Toast("SkipifyV2", "Auto Skipping is now Disabled!", "", 2.0f);

			if (ShouldLogInChat)
			gameWrapper->LogToChatbox("Auto Skipping is now Disabled!", "Skipify");
		}}, "Bind to Toggle Replay Skip", PERMISSION_ALL);

		keybindCvar = std::make_unique<CVarWrapper>(cvarManager->registerCvar("skipKeybind", "0", "Toggle Skipify keybind", false));
		missingCvar = std::make_unique<CVarWrapper>(cvarManager->registerCvar("skipMissingCheckbox", "0", "Don't skip when teammate is missing", false));
		(reEnableCvar = std::make_unique<CVarWrapper>(cvarManager->registerCvar("skipEnableCheckbox", "0", "Re-enable skipping when match ends", false)))->addOnValueChanged([this](std::string, CVarWrapper cvar) {
			if (cvar.getBoolValue())
				gameWrapper->HookEvent("Function TAGame.GameEvent_Soccar_TA.EventMatchEnded", [&](std::string eventName) {
				if (!ShouldSkipReplay) {
					gameWrapper->Toast("SkipifyV2", "Auto Skipping is re-enabled!", "", 5.0f);
					gameWrapper->HookEvent("Function GameEvent_Soccar_TA.ReplayPlayback.ShouldPlayReplay", [&](std::string eventName) { gameWrapper->ExecuteUnrealCommand("ReadyUp"); });
					ShouldSkipReplay = true;
				}});
			else
				gameWrapper->UnhookEvent("Function TAGame.GameEvent_Soccar_TA.EventMatchEnded");
			});;
}

void SkipifyV2::Skip() {

	auto player = gameWrapper->GetPlayerController();
	if (player.IsNull()) { return; }
	auto pri = player.GetPRI();
	if (pri.IsNull()) { return; }
	auto spectating = pri.IsSpectator();


	if (missingCvar->getBoolValue()) {

		if (spectating && ShouldSkipAsSpectator) 
			gameWrapper->ExecuteUnrealCommand("ReadyUp");

		unsigned char team = gameWrapper->GetPlayerController().GetPRI().GetTeamNum();
		ServerWrapper server = gameWrapper->GetCurrentGameState();
		ArrayWrapper<PriWrapper> players = server.GetPRIs();
		unsigned int teamCount = 0;

		for (int i = 0; i < players.Count(); i++) {
			if (players.Get(i).GetTeamNum() == team) {
				teamCount++;
			}
		}

		if ((int)teamCount >= server.GetMaxTeamSize()) {
			gameWrapper->ExecuteUnrealCommand("ReadyUp");
		}
		else {

			gameWrapper->Toast("SkipReplay", "Not skipping because teammate is missing!", "", 5.0f);
		}
	}
	else {
		gameWrapper->ExecuteUnrealCommand("ReadyUp");
	}
}

void SkipifyV2::OnKeyPressed(ActorWrapper aw, void* params, std::string eventName)
{
	std::string key = gameWrapper->GetFNameByIndex(((keypress_t*)params)->key.Index);
	keyIndex = ((keysIt = find(keys.begin(), keys.end(), key)) != keys.end()) ? (int)(keysIt - keys.begin()) : -1;
	cvarManager->executeCommand("closemenu skipreplaybind; openmenu settings");
	gameWrapper->UnhookEvent("Function TAGame.GameViewportClient_TA.HandleKeyPress");
	gameWrapper->Execute([this](GameWrapper* gw) { OnBind(keys[keyIndex]); });
}

void SkipifyV2::RenderSettings() {
	static const char* keyText = "Key List";
	static const char* hintText = "Type to Filter";
	static bool reEnable = reEnableCvar->getBoolValue();
	static bool missing = missingCvar->getBoolValue();

	if (!keyIndex) {
		keybind = keybindCvar->getStringValue();
		keyIndex = (keysIt = find(keys.begin(), keys.end(), keybind)) != keys.end() ? (int)(keysIt - keys.begin()) : -1;
	}

	if (ImGui::Checkbox("Don't skip when teammate is missing", &missing))
		missingCvar->setValue(missing);
	if (ImGui::IsItemHovered())
		ImGui::SetTooltip("To allow disconnected teammate to reconnect");
	if (ImGui::Checkbox("Show Notifications", &ShouldShowNotification))
		reEnableCvar->setValue(ShouldShowNotification);
	if (ImGui::IsItemHovered())
		ImGui::SetTooltip("Show a notification on changing whether to skip or not");
	if (ImGui::Checkbox("Log To Chat", &ShouldLogInChat))
		reEnableCvar->setValue(ShouldLogInChat);
	if (ImGui::IsItemHovered())
		ImGui::SetTooltip("Show a Chat Messgae on changing whether to skip or not");
	if (ImGui::Checkbox("Skip As Spectator", &ShouldSkipAsSpectator)) 
		reEnableCvar->setValue(ShouldSkipAsSpectator);
	if (ImGui::IsItemHovered())
		ImGui::SetTooltip("When spectating a game, skip the replay, requires Replay skip to be on (pretty useless ik)");



	ImGui::TextUnformatted("Press the Set Keybind button below to bind command toggleskipreplay to a key:");
	if (ImGui::SearchableCombo("##keybind combo", &keyIndex, keys, keyText, hintText, 20))
		OnBind(keys[keyIndex]);

	ImGui::SameLine();
	if (ImGui::Button("Set Keybind", ImVec2(0, 0))) {
		gameWrapper->Execute([this](GameWrapper* gw) {
			cvarManager->executeCommand("closemenu settings; openmenu skipreplaybind");
			gameWrapper->HookEventWithCaller<ActorWrapper>("Function TAGame.GameViewportClient_TA.HandleKeyPress", std::bind(&SkipifyV2::OnKeyPressed, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
			});
	}

	ImGui::SameLine();
	if (ImGui::Button("Unbind", ImVec2(0, 0))) {
		if (keyIndex != -1) {
			keybindCvar->setValue("0");
			cvarManager->executeCommand("unbind " + keys[keyIndex]);
			gameWrapper->Toast("SkipReplay", "toggleskipreplay is now unbound!", "skipreplay_logo", 5.0f);
			keyIndex = -1;
		}
	}

	ImGui::TextUnformatted("The toggleskipreplay keybind can be used to disable and enable autoskipping for that rare occasion that you dont want to skip the replay!");
	ImGui::Separator();
	ImGui::SetCursorPosY(ImGui::GetWindowSize().y - 10);
	ImGui::TextUnformatted("v2 made by milkinabag aka IReallyLikeMilk");
}

void SkipifyV2::Render() {
	ImGui::SetNextWindowPos(ImGui::GetIO().DisplaySize, ImGuiCond_Once, ImVec2(0.5f, 0.5f));
	ImGui::SetNextWindowSize(ImVec2(140, 40), ImGuiCond_Once);
	ImGui::Begin("Set Keybind", &isWindowOpen, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize);
	ImGui::SetWindowFontScale(1.6f);
	ImGui::TextUnformatted("Press any key");
	ImGui::End();
}

void SkipifyV2::OnBind(std::string key)
{
	if (key != (keybind = keybindCvar->getStringValue())) {
		std::string toastMsg = "toggleskipreplay bound to " + keys[keyIndex];
		if (keybind != "0") {
			toastMsg += " and " + keybind + " is now unbound";
			cvarManager->executeCommand("unbind " + keybind);
		}
		toastMsg += "!";
		keybindCvar->setValue(key);
		cvarManager->executeCommand("bind " + key + " toggleskipreplay");
		gameWrapper->Toast("SkipReplay", toastMsg, "skipreplay_logo", 5.0f);
	}
}

void SkipifyV2::OnOpen()
{
	isWindowOpen = true;
}

void SkipifyV2::OnClose()
{
	isWindowOpen = false;
}

std::string SkipifyV2::GetPluginName()
{
	return "Skipify V2";
}

std::string SkipifyV2::GetMenuName()
{
	return "skipreplaybind";
}

std::string SkipifyV2::GetMenuTitle()
{
	return "";
}

void SkipifyV2::SetImGuiContext(uintptr_t ctx)
{
	ImGui::SetCurrentContext(reinterpret_cast<ImGuiContext*>(ctx));
}

bool SkipifyV2::ShouldBlockInput()
{
	return false;
}

bool SkipifyV2::IsActiveOverlay()
{
	return true;
}