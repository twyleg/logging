// Copyright (C) 2021 twyleg
#pragma once

#include <spdlog/sinks/base_sink.h>
#include "spdlog/details/null_mutex.h"

#include <fmt/format.h>

#include <mutex>
#include <list>
#include <algorithm>
#include <string>

namespace Logging {

namespace {
std::string removeLastNewLine(std::string initialString) {
	while (initialString.back() == '\n' || initialString.back() == '\r') {
		initialString.pop_back();
	}
	return initialString;
}
}

template<template<class, class> class Container, class Mutex>
class StringContainerSink : public spdlog::sinks::base_sink <Mutex> {

public:

	const Container<std::string, std::allocator<std::string>>& getContainer(){ return mContainer; };

protected:
	void sink_it_(const spdlog::details::log_msg& msg) override {
		spdlog::memory_buf_t formatted;
		spdlog::sinks::base_sink<Mutex>::formatter_->format(msg, formatted);
		mContainer.emplace_back(removeLastNewLine(fmt::to_string(formatted)));
	}

	void flush_() override {}
private:

	template<class Stream, template<class, class> class Container_, class Mutex_>
	friend Stream& operator<<(Stream&, const StringContainerSink<Container_, Mutex_>&);

	Container<std::string, std::allocator<std::string>> mContainer;
};

template<class Stream, template<class, class> class Container, class Mutex>
Stream& operator<<(Stream& os, const StringContainerSink<Container, Mutex>& stringContainerSink) {
	for (size_t i=0; i<stringContainerSink.mContainer.size(); ++i) {
		os << i << ": " << stringContainerSink.mContainer[i] << std::endl;
	}
	return os;
}


template<template<class, class> class Container>
using StringListSinkMt = StringContainerSink<Container, std::mutex>;

template<template<class, class> class Container>
using StringListSinkSt = StringContainerSink<Container, spdlog::details::null_mutex>;



}
