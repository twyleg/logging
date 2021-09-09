// Copyright (C) 2021 twyleg
#include <logging/logger.h>

#include <simple_xercesc/xml_reader.h>

#include <boost/filesystem.hpp>

#include <thread>

const constexpr char* CONFIG_XSD_STRING = R"(
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

	 <xs:element name="Config" type="ConfigType"/>

</xs:schema>
)";

boost::filesystem::path CONFIG_XML_PATH = boost::filesystem::current_path() / "config.xml";

Logging::Logger::Config readConfig() {
	SimpleXercesc::XmlReader xmlReader;

	xmlReader.loadXsdSchemaFromString(Logging::Logger::Config::getXsdSchema(), "logging.xsd");
	xmlReader.loadXsdSchemaFromString(CONFIG_XSD_STRING, "pauliebox_config.xsd");
	xmlReader.parseXmlFromFile(CONFIG_XML_PATH);

	auto docElem = xmlReader.getDocumentElement();
	auto loggingElem = docElem.getFirstChildElementByTag("Logging");

	return Logging::Logger::Config::readConfig(*loggingElem);
}

auto LM_A = Logging::Logger::addModule("test_module_a");
auto LM_B = Logging::Logger::addModule("test_module_b");
auto LM_C = Logging::Logger::addModule("test_module_c");

using namespace std::chrono_literals;

int main(int, char*[]){

	const auto config = readConfig();
	Logging::Logger::instance().configure(config);
	LOG(LM_A, LL_INFO, "Log example started");
	LOG(LM_A, LL_INFO, "Using log config file {}\n{}", CONFIG_XML_PATH, config);

	LOG(LM_A, LL_DEBUG, "Message on debug level that will be filtered");
	LOG(LM_B, LL_DEBUG, "Message on debug level that will be logged");
	LOG(LM_C, LL_DEBUG, "Message on debug level that will be filtered");

	LOG(LM_A, LL_INFO, "Message on info level that will be logged");
	LOG(LM_B, LL_INFO, "Message on info level that will be logged");
	LOG(LM_C, LL_INFO, "Message on info level that will be filtered");

	LOG(LM_A, LL_WARN, "Message on warn level that will be logged");
	LOG(LM_B, LL_WARN, "Message on warn level that will be logged");
	LOG(LM_C, LL_WARN, "Message on warn level that will be filtered");

	LOG(LM_A, LL_ERROR, "Message on error level that will be logged");
	LOG(LM_B, LL_ERROR, "Message on error level that will be logged");
	LOG(LM_C, LL_ERROR, "Message on error level that will be logged");


	int i=0;
	while (true) {
		LOG(LM_A, LL_DEBUG, "This message will be filtered");
		LOG(LM_B, LL_DEBUG, "i={}", i++);

		std::this_thread::sleep_for(1s);
	}


	return 0;
}

