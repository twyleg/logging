// Copyright (C) 2021 twyleg
#include "logger.h"

#include <spdlog/sinks/stdout_color_sinks.h>
#include "spdlog/sinks/basic_file_sink.h"
#include "spdlog/sinks/rotating_file_sink.h"
#include <spdlog/pattern_formatter.h>

#include <boost/dll.hpp>

#include <iomanip>
#include <ctime>

namespace Logging {

namespace {

constexpr const char* LOG_PATTERN = "[%Y%m%d-%T.%e] [%t] [%n] [%l]: %v";

constexpr const char* LOG_CONFIG_XSD = R"(<?xml version="1.0"?>
<xs:schema
	   id="logging"
	   xmlns:xs="http://www.w3.org/2001/XMLSchema"
	   xmlns:logging="http://twyleg.de/cpp/Logging"
	   targetNamespace="http://twyleg.de/cpp/Logging"
	   elementFormDefault="unqualified">

	   <xs:simpleType name="LogLevelEnum">
		   <xs:restriction base = "xs:string">
			   <xs:enumeration value="Debug"/>
			   <xs:enumeration value="Info"/>
			   <xs:enumeration value="Warn"/>
			   <xs:enumeration value="Error"/>
		   </xs:restriction>
	   </xs:simpleType>

	   <xs:complexType name="ModuleType">
		   <xs:attribute name="name" type="xs:string"/>
		   <xs:attribute name="logLevel" type="logging:LogLevelEnum"/>
	   </xs:complexType>

	   <xs:complexType name="LogModulesType">
		   <xs:sequence>
			   <xs:element name="Module" type="logging:ModuleType" maxOccurs="unbounded"/>
		   </xs:sequence>
	   </xs:complexType>

	   <xs:complexType name="LogLevelType">
		   <xs:sequence>
			   <xs:element name="Module" type="logging:ModuleType" minOccurs="0" maxOccurs="unbounded"/>
		   </xs:sequence>
		<xs:attribute name="defaultLogLevel" type="logging:LogLevelEnum" use="required"/>
	   </xs:complexType>

	   <xs:complexType name="ConsoleSinkType"/>

	   <xs:complexType name="FileSinkType">
		   <xs:attribute name="outputDir" use="required">
			   <xs:simpleType>
				   <xs:restriction base="xs:string">
					   <xs:minLength value="1"/>
				   </xs:restriction>
			   </xs:simpleType>
		   </xs:attribute>
	   </xs:complexType>

	   <xs:complexType name="RotatingFileSinkType">
		   <xs:complexContent>
			   <xs:extension base="logging:FileSinkType">
				   <xs:attribute name="maxSize" type="xs:integer" use="required"/>
				   <xs:attribute name="maxNumFiles" type="xs:integer" use="required"/>
			   </xs:extension>
		   </xs:complexContent>
	   </xs:complexType>

	   <xs:complexType name="SinksType">
		   <xs:sequence>
			   <xs:element name="ConsoleSink" type="logging:ConsoleSinkType" minOccurs="0" maxOccurs="1"/>
			   <xs:element name="SingleFileSink" type="logging:FileSinkType" minOccurs="0" maxOccurs="unbounded"/>
			   <xs:element name="RotatingFileSink" type="logging:RotatingFileSinkType" minOccurs="0" maxOccurs="unbounded"/>
			   <xs:element name="TimestampFileSink" type="logging:FileSinkType" minOccurs="0" maxOccurs="unbounded"/>
		   </xs:sequence>
	   </xs:complexType>

	   <xs:complexType name="LoggingType">
		   <xs:sequence>
			   <xs:element name="LogLevel" type="logging:LogLevelType"/>
			   <xs:element name="Sinks" type="logging:SinksType" minOccurs="0"/>
		   </xs:sequence>
	   </xs:complexType>

   </xs:schema>
)";

const std::unordered_map<std::string, Logger::Config::LogLevel> stringToLogLevelMapping{
	{"Debug", LL_DEBUG},
	{"Info", LL_INFO},
	{"Warn", LL_WARN},
	{"Error", LL_ERROR}
};


Logger::Config::LogLevel logLevelFromString(const std::string& logLevelString) {

	auto logLevelIt = stringToLogLevelMapping.find(logLevelString);
	if (logLevelIt == stringToLogLevelMapping.end()) {
		throw std::runtime_error(fmt::format("Unable to convert \"{}\" into a log level", logLevelString));
	}
	return logLevelIt->second;
}

std::string getBinaryName() {
	return boost::dll::program_location().filename().string();
}

std::string getTimestampPrefix() {
	std::time_t t = std::time(nullptr);
	std::tm tm = *std::localtime(&t);

	std::stringstream ss;
	ss << std::put_time(&tm, "%Y%m%d-%H%M%S");
	return ss.str();
}

auto LM = Logger::addModule("logger");

}

Logger::Logger() {
	spdlog::set_pattern(LOG_PATTERN);
	spdlog::set_level(spdlog::level::level_enum::debug);
}

void Logger::configure(const Config& config) {

	mModuleLogLevel = config.mModuleLogLevel;

	spdlog::set_level(config.mDefaultLogLevel);
	spdlog::details::registry::instance().apply_all([this](auto logger) {
		setModuleLogLevel(*logger);
	});

	for (const auto sink: config.mSinks) {
		if (sink.first == "ConsoleSink") {
			createConsoleSink();
		} else if (sink.first == "SingleFileSink") {
			auto outputDir = sink.second.getParameter<std::string>("outputDir");
			createSingleFileSink(*outputDir);
		} else if (sink.first == "RotatingFileSink") {
			auto outputDir = sink.second.getParameter<std::string>("outputDir");
			auto maxSize = sink.second.getParameter<int>("maxSize");
			auto maxNumFiles = sink.second.getParameter<int>("maxNumFiles");
			createRotatingFileSink(*outputDir, *maxSize, *maxNumFiles);
		} else if (sink.first == "TimestampFileSink") {
			auto outputDir = sink.second.getParameter<std::string>("outputDir");
			createTimestampFileSink(*outputDir);
		}
	}
}

void Logger::setModuleLogLevel(spdlog::logger& logger) {
	auto moduleSpecificLogLevelIt = mModuleLogLevel.find(logger.name());
	if (moduleSpecificLogLevelIt != mModuleLogLevel.end()) {
		logger.set_level(moduleSpecificLogLevelIt->second);
	} else {
		logger.set_level(spdlog::get_level());
	}
}

void Logger::addSink(spdlog::sink_ptr sink) {
	sink->set_pattern(LOG_PATTERN);
	sink->set_level(spdlog::level::level_enum::debug);
	attachSinkToLoggers(sink);
}

void Logger::removeAllSinks() {
	mSinks.clear();

	spdlog::details::registry::instance().apply_all([](auto logger) {
		logger->sinks().clear();
	});
}

void Logger::createConsoleSink() {
	addSink(std::make_shared<spdlog::sinks::stdout_color_sink_mt>());
}

void Logger::createSingleFileSink(const boost::filesystem::path& outputDir) {
	auto filePath = outputDir / fmt::format("{}.log", getBinaryName());
	addSink(std::make_shared<spdlog::sinks::basic_file_sink_mt>(filePath.string(), true));
}

void Logger::createRotatingFileSink(const boost::filesystem::path& outputDir, size_t maxSize, int maxNumFiles) {
	auto filePath = outputDir / fmt::format("{}.rotating.log", getBinaryName());
	addSink(std::make_shared<spdlog::sinks::rotating_file_sink_mt>(filePath.string(), maxSize, maxNumFiles));
}

void Logger::createTimestampFileSink(const boost::filesystem::path& outputDir) {
	auto filePath = outputDir / fmt::format("{}_{}.log", getTimestampPrefix(), getBinaryName());
	addSink(std::make_shared<spdlog::sinks::basic_file_sink_mt>(filePath.string(), true));
}

void Logger::attachSinkToLoggers(std::shared_ptr<spdlog::sinks::sink> sink) {

	mSinks.push_back(sink);

	spdlog::details::registry::instance().apply_all([sink](auto logger) {
		logger->sinks().push_back(sink);
	});
}

Logger& Logger::instance(){
	static Logger logger;
	return logger;
}

std::shared_ptr<spdlog::logger> Logger::addModule(const std::string& name) {
	auto logger = std::make_shared<spdlog::logger>(name);
	logger->set_pattern(LOG_PATTERN);
	logger->sinks() = Logger::instance().mSinks;
	Logger::instance().setModuleLogLevel(*logger);
	spdlog::register_logger(logger);
	return logger;
}

const char* Logger::Config::getXsdSchema() {
	return LOG_CONFIG_XSD;
}

Logger::Config Logger::Config::readConfig(const SimpleXercesc::XmlElement& logElem) {

	auto logLevelElem = logElem.getFirstChildElementByTag("LogLevel");
	auto defaultLogLevel = logLevelFromString(*logLevelElem->getAttributeByName<std::string>("defaultLogLevel"));

	ModuleLogLevelMap moduleLogLevelsMap;
	auto moduleLogLevelElemVector = logLevelElem->getChildElementsByTag("Module");
	for (const auto moduleLogLevelElem: moduleLogLevelElemVector) {
		const auto moduleName = *moduleLogLevelElem.getAttributeByName<std::string>("name");
		const auto moduleLogLevel = logLevelFromString(*moduleLogLevelElem.getAttributeByName<std::string>("logLevel"));
		moduleLogLevelsMap.emplace(moduleName, moduleLogLevel);
	}

	SinkMap sinksMap;
	auto sinksElem = logElem.getFirstChildElementByTag("Sinks");
	auto sinksElemVec = sinksElem->getChildElements();
	for (const auto sinksElem: sinksElemVec) {
		const auto sinkName = sinksElem.getTagName();
		const auto sinkAttributes = sinksElem.getAttributes();
		sinksMap.emplace(sinkName, SinkParameterMap(sinkAttributes));
	}

	return {
		defaultLogLevel,
		moduleLogLevelsMap,
		sinksMap
	};
}

}
