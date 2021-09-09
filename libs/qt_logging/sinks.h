// Copyright (C) 2021 twyleg
#pragma once
#include <spdlog/sinks/base_sink.h>
#include "spdlog/details/null_mutex.h"

#include <QObject>
#include <QStringListModel>


namespace Logging::Qt {

using base_sink = spdlog::sinks::base_sink <spdlog::details::null_mutex>;

class QStringListModelSink : public QObject, public base_sink
{
	Q_OBJECT
public:
	QStringListModelSink();

	QStringListModel mStringListModel;

public slots:
	void addMessage(QString message);

signals:
	void messageAdded(QString);

protected:
	void sink_it_(const spdlog::details::log_msg& msg) override;
	void flush_() override;
};


}

