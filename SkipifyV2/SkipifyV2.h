#pragma once

#include "GuiBase.h"
#include "bakkesmod/plugin/bakkesmodplugin.h"
#include "bakkesmod/plugin/pluginwindow.h"
#include "bakkesmod/plugin/PluginSettingsWindow.h"

#include "version.h"
constexpr auto plugin_version = stringify(VERSION_MAJOR) "." stringify(VERSION_MINOR) "." stringify(VERSION_PATCH) "." stringify(VERSION_BUILD);


class SkipifyV2 : public BakkesMod::Plugin::BakkesModPlugin, public BakkesMod::Plugin::PluginSettingsWindow, public BakkesMod::Plugin::PluginWindow
{

	//std::shared_ptr<bool> enabled;

	//Boilerplate
	void onLoad() override;
	void Skip();

	// no the code below is not mine

	std::string keybind;
	const std::vector<std::string> keys = {
		"F1", "F2", "F3", "F4", "F5", "F6", "F7", "F8", "F9", "F10", "F11", "F12", "A", "B", "C", "D", "E", "F", "G", "H", "I", "J", "K", "L", "M", "N", "O", "P", "Q",
		"R", "S", "T", "U", "V", "W", "X", "Y", "Z", "Escape", "Tab", "Tilde", "ScrollLock", "Pause", "One", "Two", "Three", "Four", "Five", "Six", "Seven", "Eight", "Nine", "Zero", "Underscore",
		"Equals", "Backslash", "LeftBracket", "RightBracket", "Enter", "CapsLock", "Semicolon", "Quote", "LeftShift", "Comma", "Period", "Slash", "RightShift", "LeftControl", "LeftAlt", "SpaceBar",
		"RightAlt", "RightControl", "Left", "Up", "Down", "Right", "Home", "End", "Insert", "PageUp", "Delete", "PageDown", "NumLock", "Divide", "Multiply", "Subtract", "Add", "NumPadOne", "NumPadTwo",
		"NumPadThree", "NumPadFour", " NumPadFive", "NumPadSix", "NumPadSeven", "NumPadEight", "NumPadNine", "NumPadZero", "Decimal", "LeftMouseButton", "RightMouseButton", "ThumbMouseButton",
		"ThumbMouseButton2", "MouseScrollUp", "MouseScrollDown", "MouseX", "MouseY", "XboxTypeS_LeftThumbStick", "XboxTypeS_RightThumbStick", "XboxTypeS_DPad_Up", "XboxTypeS_DPad_Left",
		"XboxTypeS_DPad_Right", "XboxTypeS_DPad_Down", "XboxTypeS_Back", "XboxTypeS_Start", "XboxTypeS_Y", "XboxTypeS_X", "XboxTypeS_B", "XboxTypeS_A", "XboxTypeS_LeftShoulder", "XboxTypeS_RightShoulder",
		"XboxTypeS_LeftTrigger", "XboxTypeS_RightTrigger", "XboxTypeS_LeftTriggerAxis", "XboxTypeS_RightTriggerAxis", "XboxTypeS_LeftX", "XboxTypeS_LeftY", "XboxTypeS_RightX",	"XboxTypeS_RightY"
	};
	std::vector<std::string>::const_iterator keysIt;


	struct keypress_t {
		int ControllerID;
		struct {
			int Index;
			int Number;
		} key;
		unsigned char EventType;
		float AmountDepressed;
		unsigned int bGamepad;
		unsigned int ReturnValue;
	};


	//void onUnload() override; // Uncomment and implement if you need a unload method

public:
	//void RenderSettings() override; // Uncomment if you wanna render your own tab in the settings menu
	//void RenderWindow() override; // Uncomment if you want to render your own plugin window

	void SetImGuiContext(uintptr_t ctx) override;

	bool isWindowOpen = false;
	bool isMinimized = false;
	int keyIndex = 0;
	std::unique_ptr<CVarWrapper> reEnableCvar;
	std::unique_ptr<CVarWrapper> keybindCvar;
	std::unique_ptr<CVarWrapper> missingCvar;

	bool ShouldSkipAsSpectator = true;
	bool ShouldSkipReplay = true;

	bool ShouldShowNotification = true;
	bool ShouldLogInChat = false;


	virtual void Render() override;
	void RenderSettings() override;
	void OnBind(std::string key);
	void OnKeyPressed(ActorWrapper aw, void* params, std::string eventName);
	std::string GetPluginName() override;
	virtual std::string GetMenuName() override;
	virtual std::string GetMenuTitle() override;
	virtual bool ShouldBlockInput() override;
	virtual bool IsActiveOverlay() override;
	virtual void OnOpen() override;
	virtual void OnClose() override;
};
