#pragma once
#include <vector>
#include <string>
#include <vulkan/vulkan.h>

namespace Graphics 
{

	class Shader 
	{
	private:
		VkShaderModule m_module;
		VkDevice m_device;
		std::vector<char> m_shaderCode;

	public: 

		~Shader();

		inline VkShaderModule GetModule() const { return m_module; }
		Shader(const std::string& filename, VkDevice& device);
	};
}