// Vulkan Forward+.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#define GLFW_INCLUDE_VULKAN

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE


#include "VulkanProject.h"


namespace Graphics
{
	//static helpers first
	//callback for debug messenger
	static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(
		VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
		VkDebugUtilsMessageTypeFlagsEXT messageType,
		const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
		void* pUserData) {

		std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;

		return VK_FALSE;
	}

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

	//////////////////////////static helper space till here

	//initialize GLFW
	bool VulkanProject::VP_InitGLFW()
	{
		if (!glfwInit())
		{
			return false;
		}

		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

		m_window = glfwCreateWindow(Width, Height, "Vulkan window", nullptr, nullptr);

		return true;
	}

	//initialize vulkan
	bool VulkanProject::VP_InitVulkan()
	{
		CreateInstance();
		SetupDebugMessenger();
		CreateSurface();
		PickPhysicalDevice();
		CreateLogicalDevice();
		CreateSwapChain();
		CreateImageViews();
		CreateRenderPass();
		CreateGraphicsPipeline();
		CreateFrameBuffer();
		CreateCommandPools();
		CreateCommandBuffers();
		CreateSyncObjects();
		return true;
	}

	//checks if all class members have been populated
	bool VulkanProject::VP_CheckUP()
	{
		if (m_window == nullptr)
			return false;
		if (m_MainInstance == nullptr)
			return false;
		if (m_debugMessenger == nullptr)
			return false;
		if (m_surface == nullptr)
			return false;
		if (m_physicalDevice == nullptr)
			return false;
		if (m_logicalDevice == nullptr)
			return false;
		if (m_graphicsQueue == nullptr)
			return false;
		if (m_presentationQueue == nullptr)
			return false;
		if (m_swapChain == nullptr)
			return false;
		if (m_pipelineLayout == VK_NULL_HANDLE) 
			return false;
		if (m_traingleRenderPass == VK_NULL_HANDLE)
			return false;
		if (m_graphicsPipeline == VK_NULL_HANDLE)
			return false;
		if (m_commandPool == VK_NULL_HANDLE)
			return false;

		return true;
	}

	//release all resources.
	void VulkanProject::VP_CleanUP()
	{
		for (uint32_t i = 0; i < FramesInFlight; ++i) 
		{
			vkDestroySemaphore(m_logicalDevice, imageAvailableSemaphore[i], nullptr);
			vkDestroySemaphore(m_logicalDevice, renderFinishedSemaphore[i], nullptr);
			vkDestroyFence(m_logicalDevice, inFlightFences[i], nullptr);
		}
			
		vkDestroyCommandPool(m_logicalDevice, m_commandPool, nullptr);

		for (auto framebuff : m_swapChainFrameBuffers) 
		{
			vkDestroyFramebuffer(m_logicalDevice, framebuff, nullptr);
		}

		vkDestroyPipeline(m_logicalDevice, m_graphicsPipeline, nullptr);
		vkDestroyPipelineLayout(m_logicalDevice, m_pipelineLayout, nullptr);
		vkDestroyRenderPass(m_logicalDevice, m_traingleRenderPass, nullptr);

		for (auto imageView : m_swapChainImageViews)
		{
			vkDestroyImageView(m_logicalDevice, imageView, nullptr);
		}

		vkDestroySwapchainKHR(m_logicalDevice, m_swapChain, nullptr);
		vkDestroyDevice(m_logicalDevice, nullptr);

		if (m_enableValidationLayers)
		{
			DestroyDebugUtilsMessengerEXT(m_MainInstance, m_debugMessenger, nullptr);
		}
		vkDestroySurfaceKHR(m_MainInstance, m_surface, nullptr);
		vkDestroyInstance(m_MainInstance, nullptr);
		glfwDestroyWindow(m_window);
		glfwTerminate();
	}

	//main update for project
	void VulkanProject::VP_Run()
	{
		while (!glfwWindowShouldClose(m_window))
		{
			glfwPollEvents();
			DrawFrame();
		}

		vkDeviceWaitIdle(m_logicalDevice);
	}

	//creates a VKInstance with desired attirbs
	void VulkanProject::CreateInstance()
	{
		if (m_enableValidationLayers && !CheckValidationLayerSupport()) {
			throw std::runtime_error("validation layers requested, but not available!");
		}

		//app desc
		VkApplicationInfo appInfo{};
		appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		appInfo.pApplicationName = "Grpahics Engine";
		appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.pEngineName = "No Engine";
		appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.apiVersion = VK_API_VERSION_1_0;

		auto allExtentions = GetRequiredExtentions();

		//instance desc
		VkInstanceCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		createInfo.pApplicationInfo = &appInfo;
		createInfo.enabledExtensionCount = static_cast<uint32_t>(allExtentions.size());
		createInfo.ppEnabledExtensionNames = allExtentions.data();

		//Debugging inside the creation and destruction
		VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo;
		if (m_enableValidationLayers) {
			createInfo.enabledLayerCount = static_cast<uint32_t>(m_validationLayers.size());
			createInfo.ppEnabledLayerNames = m_validationLayers.data();

			PopulateDebugMessengerCreateInfo(debugCreateInfo);
			createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
		}
		else {
			createInfo.enabledLayerCount = 0;

			createInfo.pNext = nullptr;
		}

		if (vkCreateInstance(&createInfo, nullptr, &m_MainInstance) != VK_SUCCESS) {
			throw std::runtime_error("failed to create instance!");
		}

		//get all extension data(optional)
		uint32_t glfwExtensionCount = 0;
		vkEnumerateInstanceExtensionProperties(nullptr, &glfwExtensionCount, nullptr);
		m_extensionList.reserve(glfwExtensionCount);
		vkEnumerateInstanceExtensionProperties(nullptr, &glfwExtensionCount, m_extensionList.data());
	}

	//checks for validation layers
	bool VulkanProject::CheckValidationLayerSupport()
	{
		uint32_t layerCount;
		vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

		std::vector<VkLayerProperties> availableLayers(layerCount);
		vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

		for (const char* layerName : m_validationLayers) {
			bool layerFound = false;

			for (const auto& layerProperties : availableLayers) {
				if (strcmp(layerName, layerProperties.layerName) == 0) {
					layerFound = true;
					break;
				}
			}

			if (!layerFound) {
				return false;
			}
		}

		return true;
	}

	//Gets the required extensions for validation layer support
	std::vector<const char*> VulkanProject::GetRequiredExtentions()
	{
		uint32_t glfwExtensionCount = 0;
		const char** glfwExtensions;
		glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

		std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

		if (m_enableValidationLayers) {
			extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
		}

		return extensions;
	}

	//function for setting up the debug messenger for debug mode
	void VulkanProject::SetupDebugMessenger()
	{
		if (!m_enableValidationLayers)
			return;

		VkDebugUtilsMessengerCreateInfoEXT createInfo{};
		PopulateDebugMessengerCreateInfo(createInfo);

		if (CreateDebugUtilsMessengerEXT(m_MainInstance, &createInfo, nullptr, &m_debugMessenger) != VK_SUCCESS) {
			throw std::runtime_error("failed to set up debug messenger!");
		}
	}

	//function for creating external debug utils messenger ojbect.
	VkResult VulkanProject::CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
		const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger)
	{
		auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
		if (func != nullptr) {
			return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
		}
		else {
			return VK_ERROR_EXTENSION_NOT_PRESENT;
		}
	}

	//function for destroying external debug utils messenger ojbect.
	void VulkanProject::DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator)
	{
		auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
		if (func != nullptr) {
			func(instance, debugMessenger, pAllocator);
		}
	}

	//helper for populating debug messenger objects
	void VulkanProject::PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo) {
		createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
		createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
		createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
		createInfo.pfnUserCallback = DebugCallback;
	}

	//selects the best hardware device for the application
	void VulkanProject::PickPhysicalDevice()
	{
		uint32_t deviceCount = 0;
		vkEnumeratePhysicalDevices(m_MainInstance, &deviceCount, nullptr);

		if (deviceCount == 0) {
			throw std::runtime_error("failed to find GPUs with Vulkan support!");
		}

		std::vector<VkPhysicalDevice> deviceArr(deviceCount);
		vkEnumeratePhysicalDevices(m_MainInstance, &deviceCount, deviceArr.data());

		std::multimap<int, VkPhysicalDevice> deviceCandidates;

		for (const auto& device : deviceArr)
		{
			int score = GetDeviceScore(device);
			deviceCandidates.insert(std::make_pair(score, device));
		}

		if (deviceCandidates.rbegin()->first >= 0)
		{
			m_physicalDevice = deviceCandidates.rbegin()->second;
		}
		else
		{
			throw std::runtime_error("FAILED TO FIND A SUITABLE GPU");
		}
	}

	//rates GPU's based on their properties/features
	int VulkanProject::GetDeviceScore(VkPhysicalDevice device)
	{
		int score = 0;

		VkPhysicalDeviceProperties deviceProperties;
		vkGetPhysicalDeviceProperties(device, &deviceProperties);

		VkPhysicalDeviceFeatures deviceFeatures;
		vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

		//Preference to dedicated Discrete GPU's
		if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
		{
			score += 1000;
		}

		//preference to better texture processing capabilities.
		score += deviceProperties.limits.maxImageDimension2D;

		//Get queuefamily  avaialable for the device.
		QueueFamilyIndices indices = FindQueueFamilies(device);

		bool deviceExtensionsSupported = CheckDeviceExtensionSupport(device);
		bool swapChainSupportAdequate = false;

		//TODO:: swap chain support format/presentation count should influence score.
		if (deviceExtensionsSupported)
		{
			SwapChainSupportDetails swapChainDetails = QuerySwapChainSupport(device);
			swapChainSupportAdequate = !swapChainDetails.formats.empty() || !swapChainDetails.presentationModes.empty();
		}

		//do not use card if some desired queuefamilies are missing. Can lower the score instead if you have alternatives.
		if (!indices.IsComplete() || !deviceExtensionsSupported || !swapChainSupportAdequate) return 0;

		return score;
	}

	// Queries and lists all queuefamilies inside the desired structure format. 
	QueueFamilyIndices VulkanProject::FindQueueFamilies(VkPhysicalDevice device)
	{
		QueueFamilyIndices indices;

		uint32_t queueFamilyCount = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

		std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

		int i = 0;

		for (const auto& queueFamily : queueFamilies)
		{
			if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
			{
				indices.graphicsFamily = i;
			}

			VkBool32 presentSupport = false;
			vkGetPhysicalDeviceSurfaceSupportKHR(device, i, m_surface, &presentSupport);

			if (presentSupport)
			{
				indices.presentationFamily = i;
			}

			if (indices.IsComplete())
				break;

			++i;
		}

		return indices;
	}

	void VulkanProject::CreateLogicalDevice()
	{
		QueueFamilyIndices indices = FindQueueFamilies(m_physicalDevice);

		std::vector<VkDeviceQueueCreateInfo> queueCreateInfos{};
		std::set<uint32_t> uniqueQueueFamilies = { indices.graphicsFamily.value(), indices.presentationFamily.value() };

		float queuePriority = 1.0f;

		for (uint32_t queueFamily : uniqueQueueFamilies)
		{
			VkDeviceQueueCreateInfo queueCreateInfo{};
			queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			queueCreateInfo.queueFamilyIndex = queueFamily;
			queueCreateInfo.queueCount = 1;
			queueCreateInfo.pQueuePriorities = &queuePriority;
			queueCreateInfos.push_back(queueCreateInfo);
		}

		VkPhysicalDeviceFeatures deviceFeatures{};
		vkGetPhysicalDeviceFeatures(m_physicalDevice, &deviceFeatures);

		VkDeviceCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		createInfo.queueCreateInfoCount = static_cast<uint32_t> (queueCreateInfos.size());
		createInfo.pQueueCreateInfos = queueCreateInfos.data();

		createInfo.pEnabledFeatures = &deviceFeatures;

		createInfo.enabledExtensionCount = static_cast<uint32_t>(m_deviceExtensions.size());
		createInfo.ppEnabledExtensionNames = m_deviceExtensions.data();

		if (m_enableValidationLayers) {
			createInfo.enabledLayerCount = static_cast<uint32_t>(m_validationLayers.size());
			createInfo.ppEnabledLayerNames = m_validationLayers.data();
		}
		else {
			createInfo.enabledLayerCount = 0;
		}

		if (vkCreateDevice(m_physicalDevice, &createInfo, nullptr, &m_logicalDevice))
		{
			throw std::runtime_error("FAILED TO CREATE LOGICAL DEVICE");
		}

		vkGetDeviceQueue(m_logicalDevice, indices.graphicsFamily.value(), 0, &m_graphicsQueue);
		vkGetDeviceQueue(m_logicalDevice, indices.presentationFamily.value(), 0, &m_presentationQueue);
	}

	void VulkanProject::CreateSurface()
	{
		if (glfwCreateWindowSurface(m_MainInstance, m_window, nullptr, &m_surface) != VK_SUCCESS)
		{
			throw std::runtime_error("FAILED TO CREATE WINDOW SURFACE!");
		}
	}

	void VulkanProject::CreateSwapChain()
	{
		SwapChainSupportDetails details = QuerySwapChainSupport(m_physicalDevice);
		VkSurfaceFormatKHR surfaceFormat = ChooseSwapSurfaceFormat(details.formats);
		VkPresentModeKHR presentMode = ChooseSwapPresentMode(details.presentationModes);
		VkExtent2D extent = ChooseSwapExtent(details.capabilities);

		m_swapChainExtent = extent;
		m_swapChainFormat = surfaceFormat.format;

		uint32_t imageCount = details.capabilities.minImageCount + 1;

		if (details.capabilities.maxImageCount > 0 && imageCount > details.capabilities.maxImageCount)
		{
			imageCount = details.capabilities.maxImageCount;
		}

		VkSwapchainCreateInfoKHR createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		createInfo.surface = m_surface;
		createInfo.minImageCount = imageCount;
		createInfo.imageFormat = surfaceFormat.format;
		createInfo.imageColorSpace = surfaceFormat.colorSpace;
		createInfo.imageExtent = extent;
		createInfo.imageArrayLayers = 1;
		createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

		QueueFamilyIndices indices = FindQueueFamilies(m_physicalDevice);
		uint32_t queueFamilyIndices[] = { indices.graphicsFamily.value(), indices.presentationFamily.value() };

		if (indices.graphicsFamily != indices.presentationFamily)
		{
			createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
			createInfo.queueFamilyIndexCount = 2;
			createInfo.pQueueFamilyIndices = queueFamilyIndices;
		}
		else
		{
			createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
			createInfo.queueFamilyIndexCount = 0;
			createInfo.pQueueFamilyIndices = nullptr;
		}

		createInfo.preTransform = details.capabilities.currentTransform;
		createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		createInfo.presentMode = presentMode;
		createInfo.clipped = VK_TRUE;
		createInfo.oldSwapchain = VK_NULL_HANDLE;

		if (vkCreateSwapchainKHR(m_logicalDevice, &createInfo, nullptr, &m_swapChain) != VK_SUCCESS)
		{
			throw std::runtime_error(" FAILED TO CREATE SWAP CHAIN.");
		}

		vkGetSwapchainImagesKHR(m_logicalDevice, m_swapChain, &imageCount, nullptr);
		m_swapChainImages.resize(imageCount);
		vkGetSwapchainImagesKHR(m_logicalDevice, m_swapChain, &imageCount, m_swapChainImages.data());
	}

	void VulkanProject::CreateImageViews()
	{
		m_swapChainImageViews.resize(m_swapChainImages.size());

		for (size_t i = 0; i < m_swapChainImages.size(); ++i)
		{
			VkImageViewCreateInfo createInfo{};
			createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			createInfo.image = m_swapChainImages[i];
			createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
			createInfo.format = m_swapChainFormat;
			createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
			createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			createInfo.subresourceRange.baseMipLevel = 0;
			createInfo.subresourceRange.levelCount = 1;
			createInfo.subresourceRange.baseArrayLayer = 0;
			createInfo.subresourceRange.layerCount = 1;

			if (vkCreateImageView(m_logicalDevice, &createInfo, nullptr, &m_swapChainImageViews[i]) != VK_SUCCESS)
			{
				throw std::runtime_error("UNABLE TO CREATE IMAGE VIEW!");
			}
		}
	}

	bool VulkanProject::CheckDeviceExtensionSupport(VkPhysicalDevice device)
	{
		uint32_t deviceExtensionCount = 0;
		vkEnumerateDeviceExtensionProperties(device, nullptr, &deviceExtensionCount, nullptr);

		std::vector<VkExtensionProperties> availableExtensions(deviceExtensionCount);
		vkEnumerateDeviceExtensionProperties(device, nullptr, &deviceExtensionCount, availableExtensions.data());

		std::set<std::string> requiredExtensions(m_deviceExtensions.begin(), m_deviceExtensions.end());

		for (const auto& extension : availableExtensions)
		{
			requiredExtensions.erase(extension.extensionName);
		}

		return requiredExtensions.empty();
	}

	SwapChainSupportDetails VulkanProject::QuerySwapChainSupport(VkPhysicalDevice device)
	{
		SwapChainSupportDetails details;

		//populate capabilities
		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, m_surface, &details.capabilities);

		//populate formats
		uint32_t formatCount;
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, m_surface, &formatCount, nullptr);

		if (formatCount != 0)
		{
			details.formats.resize(formatCount);
			vkGetPhysicalDeviceSurfaceFormatsKHR(device, m_surface, &formatCount, details.formats.data());
		}

		//populate presentationModes
		uint32_t presentModeCount;
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, m_surface, &presentModeCount, nullptr);

		details.presentationModes.resize(presentModeCount);
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, m_surface, &presentModeCount, details.presentationModes.data());

		return details;
	}

	VkSurfaceFormatKHR VulkanProject::ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats)
	{
		for (const auto& availableFormat : availableFormats)
		{
			if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
			{
				return availableFormat;
			}
		}

		return availableFormats[0];
	}

	VkPresentModeKHR VulkanProject::ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availableModes)
	{
		for (const auto& availableMode : availableModes)
		{
			if (availableMode == VK_PRESENT_MODE_MAILBOX_KHR)
			{
				return availableMode;
			}
		}
		return VK_PRESENT_MODE_FIFO_KHR;
	}

	VkExtent2D VulkanProject::ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities)
	{
		if (capabilities.currentExtent.width != UINT32_MAX)
		{
			return capabilities.currentExtent;
		}
		else
		{
			VkExtent2D actualExtent = { Width, Height };

			actualExtent.width = std::max(capabilities.minImageExtent.width, std::min(capabilities.maxImageExtent.width, actualExtent.width));
			actualExtent.height = std::max(capabilities.minImageExtent.height, std::min(capabilities.maxImageExtent.height, actualExtent.height));

			return actualExtent;
		}
	}
	/////////////////Initial setup till here.

	VkShaderModule VulkanProject::CreateShaderModule(const std::vector<char>& code)
	{
		VkShaderModuleCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;

		
		createInfo.codeSize = code.size();

		createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());
		VkShaderModule shaderModule;
		if (vkCreateShaderModule(m_logicalDevice, &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
			throw std::runtime_error("failed to create shader module!");
		}
		return shaderModule;
	}

	void VulkanProject::CreateGraphicsPipeline()
	{
		//Shader vShader = Shader("Shaders/vShader.vert", m_logicalDevice);
		auto vertexcode = readFile("Shaders/vert.spv");
		VkShaderModule vertexModule = CreateShaderModule(vertexcode);
		VkPipelineShaderStageCreateInfo vertStageInfo{};
		vertStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		vertStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
		vertStageInfo.module = vertexModule;
		vertStageInfo.pName = "main";

		auto fragCode = readFile("Shaders/frag.spv");
		VkShaderModule fragModule = CreateShaderModule(fragCode);
		VkPipelineShaderStageCreateInfo fragStageInfo{};
		fragStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		fragStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
		fragStageInfo.module = fragModule;
		fragStageInfo.pName = "main";

		VkPipelineShaderStageCreateInfo shaderStages[] = { vertStageInfo, fragStageInfo };

		VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
		vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		vertexInputInfo.vertexBindingDescriptionCount = 0;
		vertexInputInfo.pVertexBindingDescriptions = nullptr; // Optional
		vertexInputInfo.vertexAttributeDescriptionCount = 0;
		vertexInputInfo.pVertexAttributeDescriptions = nullptr; // Optional

		VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
		inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		inputAssembly.primitiveRestartEnable = VK_FALSE;

		VkViewport viewport{};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = (float)m_swapChainExtent.width;
		viewport.height = (float)m_swapChainExtent.height;
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;

		VkRect2D scissor{};
		scissor.offset = { 0, 0 };
		scissor.extent = m_swapChainExtent;

		VkPipelineViewportStateCreateInfo viewportState{};
		viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		viewportState.viewportCount = 1;
		viewportState.pViewports = &viewport;
		viewportState.scissorCount = 1;
		viewportState.pScissors = &scissor;

		VkPipelineRasterizationStateCreateInfo rasterizer{};
		rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		rasterizer.depthClampEnable = VK_FALSE;
		rasterizer.rasterizerDiscardEnable = VK_FALSE;
		rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
		rasterizer.lineWidth = 1.0f;
		rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
		rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
		rasterizer.depthBiasEnable = VK_FALSE;
		rasterizer.depthBiasConstantFactor = 0.0f; // Optional
		rasterizer.depthBiasClamp = 0.0f; // Optional
		rasterizer.depthBiasSlopeFactor = 0.0f; // Optional

		VkPipelineMultisampleStateCreateInfo multisampling{};
		multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		multisampling.sampleShadingEnable = VK_FALSE;
		multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
		multisampling.minSampleShading = 1.0f; // Optional
		multisampling.pSampleMask = nullptr; // Optional
		multisampling.alphaToCoverageEnable = VK_FALSE; // Optional
		multisampling.alphaToOneEnable = VK_FALSE; // Optional

		VkPipelineColorBlendAttachmentState colorBlendAttachment{};
		colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		colorBlendAttachment.blendEnable = VK_FALSE;
		colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
		colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
		colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD; // Optional
		colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
		colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
		colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD; // Optional

		VkPipelineColorBlendStateCreateInfo colorBlending{};
		colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		colorBlending.logicOpEnable = VK_FALSE;
		colorBlending.logicOp = VK_LOGIC_OP_COPY; // Optional
		colorBlending.attachmentCount = 1;
		colorBlending.pAttachments = &colorBlendAttachment;
		colorBlending.blendConstants[0] = 0.0f; // Optional
		colorBlending.blendConstants[1] = 0.0f; // Optional
		colorBlending.blendConstants[2] = 0.0f; // Optional
		colorBlending.blendConstants[3] = 0.0f; // Optional

		VkDynamicState dynamicStates[] = 
		{
			VK_DYNAMIC_STATE_VIEWPORT,
			VK_DYNAMIC_STATE_LINE_WIDTH
		};

		VkPipelineDynamicStateCreateInfo dynamicState{};
		dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		dynamicState.dynamicStateCount = 2;
		dynamicState.pDynamicStates = dynamicStates;

		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = 0; // Optional
		pipelineLayoutInfo.pSetLayouts = nullptr; // Optional
		pipelineLayoutInfo.pushConstantRangeCount = 0; // Optional
		pipelineLayoutInfo.pPushConstantRanges = nullptr; // Optional

		if (vkCreatePipelineLayout(m_logicalDevice, &pipelineLayoutInfo, nullptr, &m_pipelineLayout) != VK_SUCCESS) {
			throw std::runtime_error("failed to create pipeline layout!");
		}

		
		assert(m_traingleRenderPass != VK_NULL_HANDLE);

		VkGraphicsPipelineCreateInfo graphicsPipelineInfo{};
		graphicsPipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		graphicsPipelineInfo.stageCount = 2;
		graphicsPipelineInfo.pStages = shaderStages;
		graphicsPipelineInfo.pVertexInputState = &vertexInputInfo;
		graphicsPipelineInfo.pInputAssemblyState = &inputAssembly;
		graphicsPipelineInfo.pViewportState = &viewportState;
		graphicsPipelineInfo.pRasterizationState = &rasterizer;
		graphicsPipelineInfo.pMultisampleState = &multisampling;
		graphicsPipelineInfo.pDepthStencilState = nullptr;
		graphicsPipelineInfo.pColorBlendState = &colorBlending;
		graphicsPipelineInfo.pDynamicState = nullptr;
		graphicsPipelineInfo.layout = m_pipelineLayout;
		graphicsPipelineInfo.renderPass = m_traingleRenderPass;
		graphicsPipelineInfo.subpass = 0;
		graphicsPipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
		graphicsPipelineInfo.basePipelineIndex = -1;

		if (vkCreateGraphicsPipelines(m_logicalDevice, VK_NULL_HANDLE, 1,&graphicsPipelineInfo, nullptr, &m_graphicsPipeline) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to Create Graphics Pipeline");
		}

		vkDestroyShaderModule(m_logicalDevice, vertexModule, nullptr);
		vkDestroyShaderModule(m_logicalDevice, fragModule, nullptr);
	}

	void VulkanProject::CreateRenderPass() 
	{
		VkAttachmentDescription colorAttachment{};
		colorAttachment.format = m_swapChainFormat;
		colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
		colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

		VkAttachmentReference colorAttachmentRef {};
		colorAttachmentRef.attachment = 0;
		colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkSubpassDescription subpass{};
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.colorAttachmentCount = 1;
		subpass.pColorAttachments = &colorAttachmentRef;

		VkRenderPassCreateInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassInfo.attachmentCount = 1; 
		renderPassInfo.pAttachments = &colorAttachment;
		renderPassInfo.subpassCount = 1;
		renderPassInfo.pSubpasses = &subpass;

		VkSubpassDependency dependency{};
		dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
		dependency.dstSubpass = 0;
		dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.srcAccessMask = 0;
		dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		
		renderPassInfo.dependencyCount = 1;
		renderPassInfo.pDependencies = &dependency;

		if (vkCreateRenderPass(m_logicalDevice, &renderPassInfo, nullptr, &m_traingleRenderPass) != VK_SUCCESS) 
		{
			throw std::runtime_error("Failed to Create RenderPass");
		}
		
	}

	void VulkanProject::CreateFrameBuffer() 
	{
		m_swapChainFrameBuffers.resize(m_swapChainImageViews.size());
		for (size_t i = 0; i < m_swapChainImageViews.size(); ++i)
		{
			VkImageView attachments[] = {m_swapChainImageViews[i]};
			
			VkFramebufferCreateInfo frameBufferInfo{};
			frameBufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			frameBufferInfo.renderPass = m_traingleRenderPass;
			frameBufferInfo.attachmentCount = 1;
			frameBufferInfo.pAttachments = attachments;
			frameBufferInfo.width = m_swapChainExtent.width;
			frameBufferInfo.height = m_swapChainExtent.height;
			frameBufferInfo.layers = 1;
			
			if (vkCreateFramebuffer(m_logicalDevice, &frameBufferInfo, nullptr, &m_swapChainFrameBuffers[i]) != VK_SUCCESS) 
			{
				throw std::runtime_error("Failed To Create FrameBuffer!");
			}
		}
	}

	void VulkanProject::CreateCommandPools() 
	{
		QueueFamilyIndices queueFamilies = FindQueueFamilies(m_physicalDevice);

		VkCommandPoolCreateInfo poolInfo{};
		poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		poolInfo.queueFamilyIndex = queueFamilies.graphicsFamily.value();
		poolInfo.flags = 0;

		if (vkCreateCommandPool(m_logicalDevice, &poolInfo, nullptr, &m_commandPool) != VK_SUCCESS) 
		{
			throw std::runtime_error("Failed to Create Command Pool!");
		}
	}

	void VulkanProject::CreateCommandBuffers() 
	{
		m_commandBuffers.resize(m_swapChainFrameBuffers.size());
		
		VkCommandBufferAllocateInfo cmdBuffInfo{};
		cmdBuffInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		cmdBuffInfo.commandPool = m_commandPool;
		cmdBuffInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		cmdBuffInfo.commandBufferCount = (uint32_t)m_commandBuffers.size();

		if (vkAllocateCommandBuffers(m_logicalDevice, &cmdBuffInfo, m_commandBuffers.data()) != VK_SUCCESS) 
		{
			throw std::runtime_error("Failed to Allocate Command Buffers!");
		}

		for (size_t i = 0; i < m_commandBuffers.size(); ++i) 
		{
			VkCommandBufferBeginInfo cmdBeginInfo{};
			cmdBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
			cmdBeginInfo.flags = 0;
			cmdBeginInfo.pInheritanceInfo = nullptr;

			if (vkBeginCommandBuffer(m_commandBuffers[i], &cmdBeginInfo) != VK_SUCCESS) 
			{
				throw std::runtime_error("failed to begin recording command  buffer");
			}

			VkRenderPassBeginInfo renderPassInfo{};
			renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
			renderPassInfo.renderPass = m_traingleRenderPass;
			renderPassInfo.framebuffer = m_swapChainFrameBuffers[i];

			renderPassInfo.renderArea.offset = { 0,0 };
			renderPassInfo.renderArea.extent = m_swapChainExtent;

			VkClearValue clearColor = { 0.0f, 0.0f,0.0f, 0.0f };
			renderPassInfo.clearValueCount = 1;
			renderPassInfo.pClearValues = &clearColor;

			vkCmdBeginRenderPass(m_commandBuffers[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

			vkCmdBindPipeline(m_commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, m_graphicsPipeline);
			vkCmdDraw(m_commandBuffers[i], 3, 1, 0, 0);

			vkCmdEndRenderPass(m_commandBuffers[i]);

			if (vkEndCommandBuffer(m_commandBuffers[i]) != VK_SUCCESS)
			{
				throw std::runtime_error("Failed to Record Command Buffer!");
			}


		}	
	}

	void VulkanProject::CreateSyncObjects() 
	{

		imageAvailableSemaphore.resize(FramesInFlight);
		renderFinishedSemaphore.resize(FramesInFlight);
		inFlightFences.resize(FramesInFlight);
		imagesInFlight.resize(m_swapChainImages.size(), VK_NULL_HANDLE);

		VkSemaphoreCreateInfo semaphoreInfo{};
		semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
		
		VkFenceCreateInfo fenceInfo{};
		fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

		for (uint32_t i = 0; i < FramesInFlight; ++i) 
		{
			if (vkCreateSemaphore(m_logicalDevice, &semaphoreInfo, nullptr, &imageAvailableSemaphore[i]) != VK_SUCCESS ||
				vkCreateSemaphore(m_logicalDevice, &semaphoreInfo, nullptr, &renderFinishedSemaphore[i]) != VK_SUCCESS ||
				vkCreateFence(m_logicalDevice, &fenceInfo, nullptr, &inFlightFences[i]) != VK_SUCCESS)
				throw std::runtime_error("Semaphore Creation Failed");
		}

		
	}

	void VulkanProject::DrawFrame()
	{
		vkWaitForFences(m_logicalDevice, 1, &inFlightFences[currentFrameIndex], VK_TRUE, UINT64_MAX);


		uint32_t imageIndex;
		vkAcquireNextImageKHR(m_logicalDevice, m_swapChain, UINT64_MAX, imageAvailableSemaphore[currentFrameIndex], VK_NULL_HANDLE, &imageIndex);

		if (imagesInFlight[imageIndex] != VK_NULL_HANDLE) 
		{
			vkWaitForFences(m_logicalDevice, 1, &imagesInFlight[imageIndex], VK_TRUE, UINT64_MAX);
		}

		imagesInFlight[imageIndex] = inFlightFences[currentFrameIndex];


		VkSubmitInfo info{};
		info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

		VkSemaphore drawSemaphore[] = { imageAvailableSemaphore[currentFrameIndex] };
		VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };

		info.waitSemaphoreCount = 1;
		info.pWaitSemaphores = drawSemaphore;
		info.pWaitDstStageMask = waitStages;
		info.commandBufferCount = 1;
		info.pCommandBuffers = &m_commandBuffers[imageIndex];

		VkSemaphore signalSemaphore[] = { renderFinishedSemaphore[currentFrameIndex] };
		info.signalSemaphoreCount = 1;
		info.pSignalSemaphores = signalSemaphore;
		
		vkResetFences(m_logicalDevice, 1, &inFlightFences[currentFrameIndex]);

		if (vkQueueSubmit(m_graphicsQueue, 1, &info, inFlightFences[currentFrameIndex]) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to submit draw Command buffer");
		}

		VkPresentInfoKHR presentInfo{};
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		presentInfo.waitSemaphoreCount = 1;
		presentInfo.pWaitSemaphores = signalSemaphore;

		VkSwapchainKHR swapChains[] = { m_swapChain };
		presentInfo.swapchainCount = 1; 
		presentInfo.pSwapchains = swapChains;
		presentInfo.pImageIndices = &imageIndex;
		presentInfo.pResults = nullptr;
	

		currentFrameIndex = (currentFrameIndex + 1) % FramesInFlight;

		vkQueuePresentKHR(m_presentationQueue, &presentInfo);
		vkQueueWaitIdle(m_presentationQueue);

	
	} 
}




int main() {

	Graphics::VulkanProject project = Graphics::VulkanProject();
	project.VP_InitGLFW();
	project.VP_InitVulkan();
	if (!project.VP_CheckUP()) 
	{
		std::cout << "Something went wrong!";
	}
	project.VP_Run();
	project.VP_CleanUP();

	return 0;
}