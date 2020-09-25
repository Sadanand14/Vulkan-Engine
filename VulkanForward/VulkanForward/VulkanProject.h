#pragma once

#include "Shader.h"
#include <GLFW/glfw3.h>
#include <cstdint>

#include <iostream>
#include <optional>

namespace Graphics
{

	const uint32_t Width = 1280;
	const uint32_t Height = 720;
			

	struct QueueFamilyIndices
	{
		std::optional<uint32_t> graphicsFamily;
		std::optional<uint32_t> presentationFamily;
		bool IsComplete()
		{
			return graphicsFamily.has_value() && presentationFamily.has_value();
		}
	};

	struct SwapChainSupportDetails
	{
		VkSurfaceCapabilitiesKHR capabilities;
		std::vector<VkSurfaceFormatKHR> formats;
		std::vector<VkPresentModeKHR> presentationModes;
	};

	class VulkanProject
	{
	private:
		GLFWwindow* m_window;
		VkInstance m_MainInstance;
		VkDebugUtilsMessengerEXT m_debugMessenger;
		VkSurfaceKHR m_surface;
		VkPhysicalDevice m_physicalDevice;
		VkDevice m_logicalDevice;
		VkSwapchainKHR m_swapChain;

		VkQueue m_graphicsQueue;
		VkQueue m_presentationQueue;
		VkFormat m_swapChainFormat;
		VkExtent2D m_swapChainExtent;
		VkPipelineLayout m_pipelineLayout;
		VkRenderPass m_renderPass;

		std::vector<VkImage> m_swapChainImages;
		std::vector<VkImageView> m_swapChainImagesViews;
		std::vector<VkExtensionProperties> m_extensionList;
		const std::vector<const char*> m_deviceExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
		const std::vector<const char*> m_validationLayers = { "VK_LAYER_KHRONOS_validation" };

#ifdef NDEBUG
		const bool m_enablevalidationLayers = false;
#else
		const bool m_enableValidationLayers = true;
#endif // !NDebug

	public:
		bool VP_InitGLFW();
		bool VP_InitVulkan();
		void VP_CleanUP();
		void VP_Run();
		bool VP_CheckUP();

	private:
		//setup functions for vulkan
		void CreateInstance();
		bool CheckValidationLayerSupport();
		bool CheckDeviceExtensionSupport(VkPhysicalDevice device);
		void SetupDebugMessenger();
		void CreateSurface();
		void CreateLogicalDevice();
		void CreateSwapChain();
		void CreateImageViews();
		void CreateRenderPass();

		//setup functions for graphics'
		VkShaderModule CreateShaderModule(const std::vector<char>& code);
		void CreateGraphicsPipeline();
		int GetDeviceScore(VkPhysicalDevice device);
		void PickPhysicalDevice();
		VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
		VkPresentModeKHR ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availableModes);
		VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);
		QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice device);
		SwapChainSupportDetails QuerySwapChainSupport(VkPhysicalDevice device);


		std::vector<const char*> GetRequiredExtentions();

		void PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);

		VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
			const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger);
		void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator);
	};

	
}

