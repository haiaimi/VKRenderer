#pragma once

#include <fstream>
#include <vector>

static std::vector<char> ReadFile(const std::string& FileName)
{
	std::ifstream File(FileName, std::ios::ate | std::ios::binary);

	if (!File.is_open())
	{
		throw std::runtime_error("failed to open file!");
	}

	size_t FileSize = (size_t)File.tellg();  //to the end

	std::vector<char> Buffer(FileSize);
	File.seekg(0); //to the beginning
	File.read(Buffer.data(), FileSize);

	File.close();

	return Buffer;
}

VkShaderModule CreateShaderModule(const std::vector<char>& Code)
{

}