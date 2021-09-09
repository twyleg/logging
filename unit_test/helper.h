// Copyright (C) 2021 twyleg
#pragma once

#include <boost/filesystem.hpp>

#include <fstream>
#include <vector>

namespace Logging::Testing {

inline boost::filesystem::path createEmptyDirectory(const boost::filesystem::path& dir) {
	boost::filesystem::remove_all(dir);
	boost::filesystem::create_directory(dir);
	return dir;
}

inline boost::filesystem::path getTempTestDir() {
	return createEmptyDirectory(boost::filesystem::current_path() / "logging_test");
}

inline std::string readTextFile(const boost::filesystem::path& filepath) {
	std::ifstream ifs(filepath);
	return {(std::istreambuf_iterator<char>(ifs)),
			(std::istreambuf_iterator<char>())};
}

inline std::vector<std::string> readTextFileToVector(const boost::filesystem::path& filepath) {
	std::ifstream ifs(filepath);
	std::vector<std::string> vec;
	std::string line;

	while (std::getline(ifs, line)) {
		vec.push_back(line);
	}

	return vec;
}

inline void writeTextFile(const boost::filesystem::path& filepath, const std::string_view& content) {
	std::ofstream ofs(filepath);
	ofs << content;
}

inline void removeFile(const boost::filesystem::path& filepath) {
	boost::filesystem::remove(filepath);
}

}
