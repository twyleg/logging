// Copyright (C) 2021 twyleg
#include "helper.h"

#include <logging/logger.h>
#include <logging/sinks.h>

#include <simple_xercesc/xml_reader.h>

#include <fmt/format.h>
#include <gtest/gtest.h>

#include <boost/optional.hpp>

#include <string>
#include <algorithm>
#include <fstream>
#include <list>
#include <regex>

namespace Logging::Testing {

namespace {

const constexpr char* TEST_CONFIG_FILENAME = "test_config.xml";

const constexpr char* TEST_LOG_CONFIG_XSD = R"(
<xs:schema
	  id="config"
	  xmlns:xs="http://www.w3.org/2001/XMLSchema"
	  xmlns:logging="http://twyleg.de/cpp/Logging"
	  elementFormDefault="unqualified">

	  <xs:import schemaLocation="logging.xsd" namespace="http://twyleg.de/cpp/Logging"/>

	 <xs:complexType name="ConfigType">
		 <xs:sequence>
			 <xs:element name="Logging" type="logging:LoggingType"/>
			 <xs:element name="Foo" type="xs:string"/>
		 </xs:sequence>
	 </xs:complexType>

	 <xs:element name="TestConfig" type="ConfigType"/>

</xs:schema>
)";

Logging::Logger::Config readLogConfig(boost::filesystem::path configFilePath) {
	SimpleXercesc::XmlReader xmlReader;

	xmlReader.loadXsdSchemaFromString(Logging::Logger::Config::getXsdSchema(), "logging.xsd");
	xmlReader.loadXsdSchemaFromString(TEST_LOG_CONFIG_XSD, "test_config.xsd");
	xmlReader.parseXmlFromFile(configFilePath);

	auto docElem = xmlReader.getDocumentElement();
	auto loggingElem = docElem.getFirstChildElementByTag("Logging");

	return Logging::Logger::Config::readConfig(*loggingElem);
}

constexpr const char* VALID_TEST_CONFIG_WITHOUT_SINKS_XML = R"(
<TestConfig>
	<Logging>
		 <LogLevel defaultLogLevel="Debug"/>
		 <Sinks/>
	</Logging>
	 <Foo>Foobar</Foo>
</TestConfig>
)";

constexpr const char* VALID_TEST_CONFIG_WITH_SINKS_XML = R"(
<TestConfig>
	<Logging>
		 <LogLevel defaultLogLevel="Info">
			 <Module name="module_a" logLevel="Debug"/>
		 </LogLevel>
		 <Sinks>
			 <SingleFileSink outputDir="./log"/>
			 <RotatingFileSink outputDir="./log" maxSize="100" maxNumFiles="2"/>
			 <TimestampFileSink outputDir="./log"/>
		 </Sinks>
	</Logging>
	 <Foo>Foobar</Foo>
</TestConfig>
)";

constexpr const char* INVALID_TEST_CONFIG_XML = R"(
<TestConfig>
	<Logging>
		 <LogLevel defaultLogLevel="Info">
			 <Module name="module_a" logLevel="Debug"/>
		 </LogLevel>
		 <Sinks>
			 <!-- ERROR: Empty outputDir -->
			 <SingleFileSink outputDir=""/>
		 </Sinks>
	</Logging>
	 <Foo>Foobar</Foo>
</TestConfig>
)";

auto LM = Logging::Logger::addModule("module_a");

}

class LoggerConfigTest : public ::testing::Test {

public:
	LoggerConfigTest() {
		Logger::instance().removeAllSinks();
		createEmptyDirectory("./log/");
	}

protected:

	Logger::Config configure(const std::string& configString) {
		boost::filesystem::path configFilePath = boost::filesystem::current_path() / TEST_CONFIG_FILENAME;
		writeTextFile(configFilePath, configString);
		auto logConfig = readLogConfig(configFilePath);
		Logger::instance().configure(logConfig);
		return logConfig;
	}

	void expectSinkParameters(const Logger::Config::SinkMap& sinks, const std::string& sinkName,
			const std::unordered_map<std::string, std::string>& expectedParameterMap) {
		auto actualParameterMap = sinks.find(sinkName)->second;
		EXPECT_TRUE(actualParameterMap == expectedParameterMap);
	}
};

TEST_F(LoggerConfigTest, ValidConfig_ReadConfig_ReturnLogConfigFile) {
	auto logConfig = configure(VALID_TEST_CONFIG_WITH_SINKS_XML);

	EXPECT_EQ(logConfig.mDefaultLogLevel, LL_INFO);

	EXPECT_EQ(logConfig.mModuleLogLevel.size(), 1);
	EXPECT_EQ(logConfig.mModuleLogLevel.find("module_a")->second, LL_DEBUG);

	EXPECT_EQ(logConfig.mSinks.size(), 3);
	expectSinkParameters(logConfig.mSinks, "SingleFileSink", {{"outputDir", "./log"}});
	expectSinkParameters(logConfig.mSinks, "RotatingFileSink", {{"outputDir", "./log"}, {"maxSize", "100"}, {"maxNumFiles", "2"}});
	expectSinkParameters(logConfig.mSinks, "TimestampFileSink", {{"outputDir", "./log"}});
}

TEST_F(LoggerConfigTest, InvalidConfig_ReadConfig_Throw) {
	EXPECT_THROW(configure(INVALID_TEST_CONFIG_XML), SimpleXercesc::XmlReader::XmlException);
}

class LoggerTest : public LoggerConfigTest{

public:

	LoggerTest() {
		Logger::instance().addSink(mStringVectorSink);
	}

protected:

	void expectLineContains(unsigned int lineNumber, const std::string& expectedContent) {
		const auto& vec = mStringVectorSink->getContainer();
		EXPECT_NE(vec[lineNumber].find(expectedContent), std::string::npos);
	}

	void expectLogFileContains(const boost::filesystem::path& filePath, unsigned int lineNumber, const std::string& expectedContent) {
		auto vec = readTextFileToVector(filePath);
		EXPECT_NE(vec[lineNumber].find(expectedContent), std::string::npos);
	}

	void expectLogFileExists(const boost::filesystem::path& filePath) {
		EXPECT_TRUE(boost::filesystem::exists(filePath));
	}

	boost::optional<boost::filesystem::path> findLogFile(const std::string& filename) {
		const std::regex filenameRegex(filename);
		for(auto& p: boost::filesystem::directory_iterator("./log")) {
			if (std::regex_match(p.path().filename().string(), filenameRegex)) {
				return p.path();
			}
		}
		return boost::none;
	}

	std::shared_ptr<StringContainerSink<std::vector, std::mutex>> mStringVectorSink =
			std::make_shared<StringContainerSink<std::vector, std::mutex>>();
};

TEST_F(LoggerTest, ValidConfigWithMultipleSinks_LogMessage_MessageLoggedInFiles) {
	configure(VALID_TEST_CONFIG_WITH_SINKS_XML);

	boost::filesystem::path singleFilePath = "./log/test_logging.log";
	boost::filesystem::path rotatingFilePath = "./log/test_logging.rotating.log";
	boost::filesystem::path timestampFilePath = *findLogFile(R"(\d{8}-\d{6}_test_logging\.log)");

	expectLogFileExists(singleFilePath);
	expectLogFileExists(rotatingFilePath);
	expectLogFileExists(timestampFilePath);

	LOG(LM, LL_DEBUG, "log message {}", 42);
	FLUSH(LM);

	expectLogFileContains(singleFilePath, 0, "[debug]: log message 42");
	expectLogFileContains(rotatingFilePath, 0, "[debug]: log message 42");
	expectLogFileContains(timestampFilePath, 0, "[debug]: log message 42");
}

TEST_F(LoggerTest, ValidConfig_LogMessagesOnDifferentLevels_MessagesLogged) {
	configure(VALID_TEST_CONFIG_WITHOUT_SINKS_XML);

	LOG(LM, LL_DEBUG, "log message {}", 42);
	LOG(LM, LL_INFO, "log message {}", 43);
	LOG(LM, LL_WARN, "log message {}", 44);
	LOG(LM, LL_ERROR, "log message {}", 45);

	expectLineContains(0, "[debug]: log message 42");
	expectLineContains(1, "[info]: log message 43");
	expectLineContains(2, "[warning]: log message 44");
	expectLineContains(3, "[error]: log message 45");
}

}
