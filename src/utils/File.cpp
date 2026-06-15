// ✟₊† mine
#include "File.hpp"


/* ꒰১ ☆ ⏔⏔⏔ ꒰ basic file operations ꒱ ⏔⏔⏔ ☆ ໒꒱ */


bool Utils::fileExists(const std::string& path) {
	return std::filesystem::exists(path); 
}

bool Utils::directoryExists(const std::string& path) {
	return std::filesystem::exists(path) && std::filesystem::is_directory(path);
}

bool Utils::createDirectory(const std::string& path) {
	std::error_code ec;
	return std::filesystem::create_directories(path, ec);
}

size_t Utils::getFileSize(const std::string& path) {
	std::error_code ec;
	auto size = std::filesystem::file_size(path, ec);
	return ec ? 0 : size;
}


/* ꒰১ ☆ ⏔⏔⏔ ꒰ read & write file operations ꒱ ⏔⏔⏔ ☆ ໒꒱ */


std::optional<std::string> Utils::readFileToString(const std::string& path) {

	std::ifstream file(path, std::ios::binary | std::ios::ate);
	if (!file.is_open()) {
		return std::nullopt;
	}

	std::streamsize size = file.tellg();
	file.seekg(0, std::ios::beg);

	std::string buffer;
	buffer.resize(static_cast<size_t>(size));

	if (!file.read(buffer.data(), size)) {
		return std::nullopt;
	}

	return buffer;
}

std::optional<std::vector<uint8_t>> Utils::readFileToBytes(const std::string& path) {

	std::ifstream file(path, std::ios::binary | std::ios::ate);
	if (!file.is_open()) {
		return std::nullopt;
	}

	std::streamsize size = file.tellg();
	file.seekg(0, std::ios::beg);

	std::vector<uint8_t> buffer(static_cast<size_t>(size));

	if (!file.read(reinterpret_cast<char*>(buffer.data()), size)) {
		return std::nullopt;
	}

	return buffer;
}

bool Utils::writeStringToFile(const std::string& path, const std::string& content) {

	std::ofstream file(path, std::ios::binary);
	if (!file.is_open()) {
		return false;
	}

	file.write(content.data(), static_cast<std::streamsize>(content.size()));
	return file.good();
}

bool Utils::writeBytesToFile(const std::string& path, const std::vector<uint8_t>& data) {

	std::ofstream file(path, std::ios::binary);
	if (!file.is_open()) {
		return false;
	}

	file.write(reinterpret_cast<const char*>(data.data()), static_cast<std::streamsize>(data.size()));
	return file.good();
}

std::vector<char> Utils::readSPIRVFile(const std::string& filename)
{
	std::ifstream file(filename, std::ios::ate | std::ios::binary);

	if (!file.is_open()) {
		throw std::runtime_error("Failed to open SPIR-V file: " + filename);
	}

	size_t fileSize = static_cast<size_t>(file.tellg());
	std::vector<char> buffer(fileSize);

	file.seekg(0);
	file.read(buffer.data(), fileSize);
	file.close();

	return buffer;
}

/* ꒰১ ☆ ⏔⏔⏔ ꒰ *.ini ꒱ ⏔⏔⏔ ☆ ໒꒱ */


Utils::IniStuff::IniStuff(const std::string& path) {
	load(path);	// ♡ loading contents of the *.ini file
}

bool Utils::IniStuff::load(const std::string& path) {

	std::ifstream file(path);
	if (!file.is_open()) {
		return false;
	}

	m_data.clear();

	std::string line;
	while (std::getline(file, line)) {
		size_t start = line.find_first_not_of(" \t\r\n");
		if (start == std::string::npos) {
			continue;
		}

		size_t end = line.find_last_not_of(" \t\r\n");
		line = line.substr(start, end - start + 1);

		if (line.empty() || line[0] == '#' || line[0] == ';') {
			continue;
		}

		size_t equalsPos = line.find('=');
		if (equalsPos == std::string::npos) {
			continue;
		}

		std::string key = line.substr(0, equalsPos);
		std::string value = line.substr(equalsPos + 1);

		key.erase(0, key.find_first_not_of(" \t"));
		key.erase(key.find_last_not_of(" \t") + 1);

		value.erase(0, value.find_first_not_of(" \t"));
		value.erase(value.find_last_not_of(" \t") + 1);

		m_data[key] = value;

		return true;
	}
}

bool Utils::IniStuff::save(const std::string& path) const {

	std::ofstream file(path);
	if (!file.is_open()) {
		return false;
	}

	for (const auto& [key, value] : m_data) {
		file << key << " = " << value << "\n";
	}

	return file.good();
}

std::optional<std::string> Utils::IniStuff::getValue(const std::string& key) const {

	auto it = m_data.find(key);
	if (it != m_data.end()) {
		return it->second;
	}
	return std::nullopt;
}

std::optional<int> Utils::IniStuff::getInt(const std::string& key) const {

	auto it = m_data.find(key);
	if (it == m_data.end()) {
		return std::nullopt;
	}

	const std::string& str = it->second;

	size_t start = str.find_first_not_of(" \t");
	if (start == std::string::npos) {
		return std::nullopt;
	}

	int value = 0;
	auto result = std::from_chars(
		str.data() + start,
		str.data() + str.size(),
		value
	);

	if (result.ec == std::errc()) {
		return value;
	}

	return std::nullopt;
}

std::optional<float> Utils::IniStuff::getFloat(const std::string& key) const {

	auto it = m_data.find(key);
	if (it == m_data.end()) {
		return std::nullopt;
	}

	const std::string& str = it->second;

	size_t start = str.find_first_not_of(" \t");
	if (start == std::string::npos) {
		return std::nullopt;
	}

	float value = 0.0f;
	auto result = std::from_chars(
		str.data() + start,
		str.data() + str.size(),
		value
	);

	if (result.ec == std::errc()) {
		return value;
	}

	return std::nullopt;
}

std::optional<bool> Utils::IniStuff::getBool(const std::string& key) const {

	auto it = m_data.find(key);
	if (it == m_data.end()) {
		return std::nullopt;
	}

	std::string value = it->second;

	size_t start = value.find_first_not_of(" \t");
	if (start == std::string::npos) {
		return std::nullopt;
	}

	size_t end = value.find_last_not_of(" \t");
	value = value.substr(start, end - start + 1);

	for (auto& c : value) {
		c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
	}

	if (value == "true" || value == "1" || value == "yes" || value == "on") {
		return true;
	}

	if (value == "false" || value == "0" || value == "no" || value == "off") {
		return false;
	}

	return std::nullopt;
}

void Utils::IniStuff::setValue(const std::string& key, const std::string& value) {
	m_data[key] = value;
}

void Utils::IniStuff::setValue(const std::string& key, int value) {
	m_data[key] = std::to_string(value);
}

void Utils::IniStuff::setValue(const std::string& key, float value) {
	m_data[key] = std::to_string(value);
}

void Utils::IniStuff::setValue(const std::string& key, bool value) {
	m_data[key] = value ? "true" : "false";
}


/* ꒰১ ☆ ⏔⏔⏔ ꒰ path utils ꒱ ⏔⏔⏔ ☆ ໒꒱ */

std::string Utils::getDirectory(const std::string& path) {

	auto pos = path.find_last_of("/\\");
	if (pos == std::string::npos) {
		return ".";
	}
	return path.substr(0, pos);
}

std::string Utils::getFilename(const std::string& path) {

	auto pos = path.find_last_of("/\\");
	if (pos == std::string::npos) {
		return path;
	}
	return path.substr(pos + 1);
}

std::string Utils::getExtension(const std::string& path) {

	auto filename = getFilename(path);
	auto pos = filename.find_last_of('.');
	if (pos == std::string::npos) {
		return "";
	}
	return filename.substr(pos);
}

std::string Utils::joinPath(const std::string& base, const std::string& sub) {

	if (base.empty()) return sub;
	if (sub.empty()) return base;

	bool baseHasSep = (base.back() == '/' || base.back() == '\\');
	bool subHasSep = (sub.front() == '/' || sub.front() == '\\');

	if (baseHasSep && subHasSep) {
		return base + sub.substr(1);
	}
	else if (!baseHasSep && !subHasSep) {
		return base + "/" + sub;
	}
	else {
		return base + sub;
	}
}