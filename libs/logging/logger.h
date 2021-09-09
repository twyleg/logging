// Copyright (C) 2021 twyleg
#pragma once
#include <simple_xercesc/xml_element.h>

#include <spdlog/spdlog.h>
#include "spdlog/fmt/ostr.h"

#include <boost/filesystem.hpp>

#include <iosfwd>

#define LL_DEBUG spdlog::level::level_enum::debug
#define LL_INFO spdlog::level::level_enum::info
#define LL_WARN spdlog::level::level_enum::warn
#define LL_ERROR spdlog::level::level_enum::err

#define LOG(logModule, logLevel, ...) logModule->log(logLevel, __VA_ARGS__)
#define FLUSH(logModule) logModule->flush()



namespace Logging {

class Logger {

public:

	struct Config {

		class SinkParameterMap : public std::unordered_map<std::string, std::string> {
		public:

			SinkParameterMap(const std::unordered_map<std::string, std::string>& map)
				: std::unordered_map<std::string, std::string>(map)
			{}

			template<class T>
			boost::optional<T> getParameter(const std::string& parameterName) const {
				const auto it = find(parameterName);
				if (it != end()) {
					return boost::lexical_cast<T>(it->second);
				} else {
					return boost::none;
				}
			}
		};

		using LogLevel = spdlog::level::level_enum;
		using ModuleLogLevelMap = std::unordered_map<std::string, LogLevel>;
		using SinkMap = std::unordered_map<std::string, SinkParameterMap>;

		static Config readConfig(const SimpleXercesc::XmlElement& logElem);
		static const char* getXsdSchema();

		const LogLevel mDefaultLogLevel;
		const ModuleLogLevelMap mModuleLogLevel;
		const SinkMap mSinks;

	};

	Logger();
	void configure(const Config&);

	void addSink(spdlog::sink_ptr);
	void removeAllSinks();

	static Logger& instance();
	static std::shared_ptr<spdlog::logger> addModule(const std::string& name);

private:

	void setModuleLogLevel(spdlog::logger&);

	void createConsoleSink();
	void createSingleFileSink(const boost::filesystem::path&);
	void createRotatingFileSink(const boost::filesystem::path&, size_t, int maxNumFiles);
	void createTimestampFileSink(const boost::filesystem::path&);
	void attachSinkToLoggers(std::shared_ptr<spdlog::sinks::sink> sink);

	std::unordered_map<std::string, Config::LogLevel> mModuleLogLevel;
	std::vector<std::shared_ptr<spdlog::sinks::sink>> mSinks;


};

template<class Stream>
Stream& operator<<(Stream& os, const Logger::Config::LogLevel& logLevel) {
	switch (logLevel) {
	case LL_DEBUG:
		os << "Debug";
		break;
	case LL_INFO:
		os << "Info";
		break;
	case LL_WARN:
		os << "Warn";
		break;
	case LL_ERROR:
		os << "Error";
		break;
	default:
		os << "Unknown";
		break;
	}
	return os;
}

template<class Stream>
Stream& operator<<(Stream& os, const Logger::Config& config) {

	os << "Log config:" << std::endl;
	os << "  Default log level: " << config.mDefaultLogLevel << std::endl;
	os << "  Module specific log levels:";
	if (config.mModuleLogLevel.size()) {
		for (const auto moduleLogLevel: config.mModuleLogLevel) {
			os << std::endl << "    \"" << moduleLogLevel.first << "\": " << moduleLogLevel.second;
		}
	} else {
		os  << std::endl << "none";
	}
	os << std::endl << "  Sinks:";
	if (config.mSinks.size()) {
		for (const auto sink: config.mSinks) {
			os << std::endl << "    " << sink.first << ": ";
			const Logger::Config::SinkParameterMap& sinkParameterMap = sink.second;
			if (sinkParameterMap.size()) {
				for (const auto parameterMap: sinkParameterMap) {
					os << "\"" << parameterMap.first << "\":\"" << parameterMap.second << "\" ";
				}
			} else {
				os << "--";
			}
		}
	} else {
		os  << std::endl << "none";
	}

	return os;
}

}

