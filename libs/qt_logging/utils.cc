// Copyright (C) 2021 twyleg
#include "utils.h"

#include <logging/logger.h>

#include <QObject>

namespace Logging::Qt {

namespace {

std::unordered_map<std::string, std::shared_ptr<spdlog::logger>> QT_LOG_MODULES({
	{"qml", Logging::Logger::addModule("qml")},
	{"js", Logging::Logger::addModule("qml_js")}
});

spdlog::level::level_enum qtMsgTypeToSpdlogLevel(QtMsgType type) {

	using namespace spdlog::level;

	switch (type) {
	case QtInfoMsg:
		return level_enum::info;
	case QtWarningMsg:
		return level_enum::warn;
	case QtCriticalMsg:
		return level_enum::critical;
	case QtFatalMsg:
		return level_enum::err;
	default:
		return level_enum::debug;
	}
}

void redirectQtLogMessages(QtMsgType type, const QMessageLogContext &context, const QString &msg) {

	QByteArray localMsg = msg.toLocal8Bit();
	const char *file = context.file ? context.file : "";
	const char *function = context.function ? context.function : "";
	const char *category = context.category ? context.category : "";

	auto qtLogModuleIt = QT_LOG_MODULES.find(category);
	if (qtLogModuleIt == QT_LOG_MODULES.end()) {
		return;
	}

	auto qtLogModule = qtLogModuleIt->second;
	auto logLevel = qtMsgTypeToSpdlogLevel(type);

	qtLogModule->log(logLevel, "{} ({}:{}, {})", localMsg.constData(), file, context.line, function);
}

}

void initQmlLoggerRedirection() {
	qInstallMessageHandler(redirectQtLogMessages);
}

}
