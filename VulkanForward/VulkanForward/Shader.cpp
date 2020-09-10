#include "Shader.h"
#include <fstream>

namespace Graphics 
{
	static std::vector<char> readFile(const std::string& filename)
	{
		std::ifstream file(filename, std::ios::ate | std::ios::binary);

		if (!file.is_open())
		{
			throw std::runtime_error("FAILED TO OPEN FILE");
		}

		size_t fileSize = (size_t)file.tellg();
		std::vector<char> buffer(fileSize);

		file.seekg(0);
		file.read(buffer.data(), fileSize);

		file.close();

		return buffer;
	}

	Shader::Shader(const std::string& filename, VkDevice& device)
	{
		m_shaderCode = readFile(filename);
		m_device = device;

		VkShaderModuleCreateInfo createInfo;

		createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		createInfo.codeSize = m_shaderCode.size();
		createInfo.pCode = reinterpret_cast <const uint32_t*>(m_shaderCode.data());

		if (vkCreateShaderModule(device, &createInfo, nullptr, &m_module) != VK_SUCCESS) 
		{
			throw std::runtime_error(" Failed to Create ShaderModule!");
		}
	}

	Shader::~Shader() 
	{
		vkDestroyShaderModule(m_device, m_module, nullptr);
	}

}