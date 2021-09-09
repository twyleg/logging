// Copyright (C) 2021 twyleg
#include "sinks.h"

#include <fmt/format.h>

namespace Logging::Qt {

namespace {
QString removeLastNewLine(const QString& initialString) {
	int pos = initialString.lastIndexOf(QChar('\n'));

	if (pos != -1) {
		return initialString.left(pos);
	} else {
		return initialString;
	}
}
}

QStringListModelSink::QStringListModelSink()
	: QObject(),
	  base_sink(),
	  mStringListModel(this)
{
	QObject::connect(
			this, SIGNAL(messageAdded(QString)),
			this, SLOT(addMessage(QString)),
			::Qt::QueuedConnection
	);
}

void QStringListModelSink::addMessage(QString message) {
	mStringListModel.insertRow(mStringListModel.rowCount());
	QModelIndex index = mStringListModel.index(mStringListModel.rowCount()-1);
	mStringListModel.setData(index, message);
}

void QStringListModelSink::sink_it_(const spdlog::details::log_msg& msg) {
	spdlog::memory_buf_t formatted;
	base_sink::formatter_->format(msg, formatted);
	QString str = removeLastNewLine(QString::fromStdString(fmt::to_string(formatted)));
	emit messageAdded(str);
}

void QStringListModelSink::flush_() {}

}
