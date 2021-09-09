// Copyright (C) 2021 twyleg
#include <logging/logger.h>
#include <qt_logging/sinks.h>

#include <simple_xercesc/xml_reader.h>

#include <QWidget>
#include <QObject>
#include <QApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QTimer>

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


class Example {

public:

	Example(int argc, char* argv[])
		: mArgc(argc),
		  mArgv(argv),
		  mApplication(mArgc, mArgv)
	{
		initLogger();
		initUi();
	}

	int run() {

		LOG(LM_A, LL_INFO, "Qt log example started");

		QObject::connect(&mPrintTimer, &QTimer::timeout, [](){
			static int i=0;
			LOG(LM_A, LL_INFO, "log message: {}", i++);
		});

		mPrintTimer.setInterval(500);
		mPrintTimer.start();

		return mApplication.exec();
	}

private:

	void initLogger() {
		auto qStringListModelSink = std::make_shared<Logging::Qt::QStringListModelSink>();
		const auto config = readConfig();

		Logging::Logger::instance().configure(config);
		Logging::Logger::instance().addSink(qStringListModelSink);

		mEngine.rootContext()->setContextProperty("logMessages", &qStringListModelSink->mStringListModel);
	}

	void initUi() {
		mEngine.addImportPath("qrc:/");
		mEngine.load("qrc:/qml/Example.qml");
	}

	int mArgc;
	char **mArgv;

	QApplication mApplication;
	QQmlApplicationEngine mEngine;
	QTimer mPrintTimer;
};

int main(int argc, char* argv[]){
	Example example(argc, argv);
	return example.run();
}

