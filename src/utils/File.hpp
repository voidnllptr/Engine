#pragma once

#ifndef FILE_HPP
#define FILE_HPP

// ✟₊† std
#include <iostream>
#include <filesystem>
#include <fstream>
#include <system_error>
#include <charconv>
#include <unordered_map>

namespace Utils {


	/* ꒰১ ☆ ⏔⏔⏔ ꒰ basic file operations ꒱ ⏔⏔⏔ ☆ ໒꒱ */


	// ʚ checks if a file exists at the specified path ɞ
	inline bool fileExists(const std::string& path);

	// ʚ checks if a directory exists at the specified path ɞ
	inline bool directoryExists(const std::string& path);

	// ʚ creates a directory and any missing parent directories ɞ
	// ʚ returns true if the directory was created or already exists ɞ
	inline bool createDirectory(const std::string& path);

	// ʚ gets the file size in bytes ɞ
	// ʚ returns 0 if the file doesn't exist or an error occurs ɞ
	inline size_t getFileSize(const std::string& path);


	/* ꒰১ ☆ ⏔⏔⏔ ꒰ read & write file operations ꒱ ⏔⏔⏔ ☆ ໒꒱ */


	// ʚ reads the file as a string ɞ
	inline std::optional<std::string> readFileToString(const std::string& path);

	// ʚ reads the file as a vector of bytes ɞ
	inline std::optional<std::vector<uint8_t>> readFileToBytes(const std::string& path);

	// ʚ writes a string to the file ɞ
	inline bool writeStringToFile(const std::string& path, const std::string& content);

	// ʚ writes a vector of bytes to the file ɞ
	inline bool writeBytesToFile(const std::string& path, const std::vector<uint8_t>& data);

	// Вспомогательная функция (можно вынести в Utils)
	std::vector<char> readSPIRVFile(const std::string& filename);

	/* ꒰১ ☆ ⏔⏔⏔ ꒰ *.ini ꒱ ⏔⏔⏔ ☆ ໒꒱ */


	// ʚ contains *.ini file interaction utils ɞ
	class IniStuff {
		std::unordered_map<std::string, std::string> m_data;

	public:
		// ʚ creates an empty instance ɞ
		IniStuff() = default;

		// ʚ loads contents from *.ini file upon construction ɞ
		explicit IniStuff(const std::string& path);

		// ʚ loads contents from *.ini file ɞ
		bool load(const std::string& path);

		// ʚ saves changes to *.ini file ɞ
		bool save(const std::string& path) const;

		// ʚ retrieves a value from the *.ini file by key ɞ
		// ʚ returns std::nullopt if not found ɞ
		std::optional<std::string> getValue(const std::string& key) const;
		std::optional<int> getInt(const std::string& key) const;
		std::optional<float> getFloat(const std::string& key) const;
		std::optional<bool> getBool(const std::string& key) const;

		// ʚ sets or updates a value in the *.ini file by key ɞ
		void setValue(const std::string& key, const std::string& value);
		void setValue(const std::string& key, int value);
		void setValue(const std::string& key, float value);
		void setValue(const std::string& key, bool value);
	};


	/* ꒰১ ☆ ⏔⏔⏔ ꒰ path utils ꒱ ⏔⏔⏔ ☆ ໒꒱ */


	// ʚ extracts directory path from the full path (e.g., "dir/subdir/file.txt" -> "dir/subdir") ɞ
	inline std::string getDirectory(const std::string& path);

	// ʚ extracts filename from the full path (e.g., "dir/file.txt" -> "file.txt") ɞ
	inline std::string getFilename(const std::string& path);

	// ʚ extracts extension from the full path (e.g., "file.txt" -> ".txt") ɞ
	inline std::string getExtension(const std::string& path);

	// ʚ joins two paths with a correct separator ɞ
	inline std::string joinPath(const std::string& base, const std::string& sub);
}

#endif