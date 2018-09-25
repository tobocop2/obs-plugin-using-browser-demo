#pragma once

#include <obs-module.h>
#include <util/config-file.h>

#include <xstring>

#include "StreamElementsUtils.hpp"

class StreamElementsConfig
{
private:
	const char* CONFIG_FILE_NAME = "obs-browser-streamelements.ini";

public:
	static const uint64_t STARTUP_FLAGS_ONBOARDING_MODE = 0x0000000000000001L;
	static const uint64_t STARTUP_FLAGS_SIGNED_IN       = 0x0000000000000002L;

private:
	StreamElementsConfig();
	~StreamElementsConfig();

public:
	static StreamElementsConfig* GetInstance() {
		if (!s_instance) {
			s_instance = new StreamElementsConfig();
		}

		return s_instance;
	}

public:
	config_t* GetConfig();
	void SaveConfig();

public:
	int64_t GetStreamElementsPluginVersion()
	{
		return config_get_uint(
			StreamElementsConfig::GetInstance()->GetConfig(),
			"Header",
			"Version");
	}

	int GetStartupFlags()
	{
		return (int)config_get_uint(
			StreamElementsConfig::GetInstance()->GetConfig(),
			"Startup",
			"Flags");
	}

	void SetStartupFlags(int value)
	{
		config_set_uint(
			StreamElementsConfig::GetInstance()->GetConfig(),
			"Startup",
			"Flags",
			value);

		SaveConfig();
	}

	std::string GetStartupState()
	{
		return config_get_string(
			StreamElementsConfig::GetInstance()->GetConfig(),
			"Startup",
			"State");
	}

	void SetStartupState(std::string value)
	{
		config_set_string(
			StreamElementsConfig::GetInstance()->GetConfig(),
			"Startup",
			"State",
			value.c_str());

		SaveConfig();
	}

	std::string GetUrlOnBoarding()
	{
		std::string result = GetCommandLineOptionValue("streamelements-onboarding-url");

		if (!result.size()) {
			result = "https://obs.streamelements.com/welcome";
		}

		return result;
	}

	std::string GetUrlReportIssue()
	{
		std::string result = GetCommandLineOptionValue("streamelements-report-issue-url");

		if (!result.size()) {
			result = "https://obs-reports.streamelements.com/api/report-issue";
		}

		return result;
	}

	std::string GetHeapAnalyticsAppId();

private:
	config_t* m_config = nullptr;

private:
	static StreamElementsConfig* s_instance;
};

