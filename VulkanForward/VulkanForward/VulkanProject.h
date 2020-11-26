#pragma once

#include "Shader.h"
#include <GLFW/glfw3.h>
#include "Types.h"

namespace Graphics
{

	const uint32_t Width = 1280;
	const uint32_t Height = 720;
	const uint32_t FramesInFlight = 2;
	

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
		std::vector<VkFence> inFlightFences;
		std::vector<VkFence> imagesInFlight;
		size_t currentFrameIndex = 0;

		GLFWwindow* m_window;
		VkInstance m_MainInstance;
		VkDebugUtilsMessengerEXT m_debugMessenger;
		VkSurfaceKHR m_surface;
		VkPhysicalDevice m_physicalDevice;
		VkDevice m_logicalDevice;
		VkSwapchainKHR m_swapChain;
		std::vector<VkSemaphore> imageAvailableSemaphore, renderFinishedSemaphore;
		
		
		VkQueue m_graphicsQueue;
		VkQueue m_presentationQueue;
		VkFormat m_swapChainFormat;
		VkExtent2D m_swapChainExtent;
		VkPipelineLayout m_pipelineLayout = VK_NULL_HANDLE;
		VkRenderPass m_traingleRenderPass = VK_NULL_HANDLE;
		VkPipeline m_graphicsPipeline = VK_NULL_HANDLE;
		VkCommandPool m_commandPool = VK_NULL_HANDLE;

		std::vector<VkCommandBuffer> m_commandBuffers;
		std::vector<VkImage> m_swapChainImages;
		std::vector<VkImageView> m_swapChainImageViews;
		std::vector<VkExtensionProperties> m_extensionList;
		std::vector<VkFramebuffer> m_swapChainFrameBuffers;
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
		void CreateFrameBuffer();
		void CreateCommandPools();
		void CreateCommandBuffers();
		void DrawFrame();
		void CreateSyncObjects();

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

