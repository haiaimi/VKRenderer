#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <vec4.hpp>
#include <mat4x4.hpp>

#include <stdexcept>
#include <iostream>
#include <stdlib.h>
#include <vector>
#include <vulkan/vulkan_core.h>
#include <map>
#include <optional>
#include <set>
#include <cstdint>
#include <algorithm>
#include "../Public/Common/FunctionLibrary.h"
#include "../Public/Common/VertexInput.h"
#include <chrono>
#include <gtc/matrix_transform.hpp>

const uint32_t WIDTH = 800;
const uint32_t HEIGHT = 600;

const int MAX_FRAMES_IN_FLIGHT = 2;

const std::vector<const char*> ValidationLayers = { "VK_LAYER_KHRONOS_validation" };
const std::vector<const char*> DeviceExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };

#ifdef NDEBUG
	const bool EnableValidationLayerss = false;
#else
	const bool EnableValidationLayers = true;
#endif // NDEBUG

	static void FramebufferResizeCallback(GLFWwindow* Window, int Width, int Height);

class HelloTriangleApplication
{
public:
	void run()
	{
		InitWindow();
		InitVulkan();
		MainLoop();
		Cleanup();
	}

private:
	void InitWindow()
	{
		glfwInit();

		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

		Window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", nullptr, nullptr);
		glfwSetWindowUserPointer(Window, this);
		glfwSetFramebufferSizeCallback(Window, FramebufferResizeCallback);
	}

	void InitVulkan()
	{
		CreateInstance();
		SetupDebugMessenger();
		CreateSurface();
		PickPhysicalDevice();
		CreateLogicalDevice();
		CreateSwapChain();
		CreateImageViews();
		CreateRenderPass();
		CreateDescriptorSetLayout();
		CreateGraphicsPipeline();
		CreateFramebuffers();
		CreateCommandPool();
		CreateVertexBuffers();
		CreateIndexBuffers();
		CreateUniformBuffers();
		CreateDescriptorPool();
		CreateDescriptorSets();
		CreateCommandBuffers();
		CreateSyncObjects();
	}

	void CreateInstance()
	{
		if (EnableValidationLayers && !CheckValidationLayerSupport())
		{
			throw std::runtime_error("Validation layers requested, but not avaliable");
		}

		VkApplicationInfo AppInfo;
		AppInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		AppInfo.pApplicationName = "Hello Triangle";
		AppInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
		AppInfo.pEngineName = "No Engine";
		AppInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
		AppInfo.apiVersion = VK_API_VERSION_1_0;
		AppInfo.pNext = nullptr;

		/// Create vulkan instance
		// Get the required extension on this platform

		// Begin Enumerate all extension supported
		uint32_t ExtensionCount = 0;
		vkEnumerateInstanceExtensionProperties(nullptr, &ExtensionCount, nullptr);
		std::vector<VkExtensionProperties> Extensions(ExtensionCount);
		vkEnumerateInstanceExtensionProperties(nullptr, &ExtensionCount, Extensions.data());
		
		// Print all
		std::cout << "avaliable extension:\n";
		for (const auto& Iter : Extensions)
		{
			std::cout << '\t' << Iter.extensionName << '\n';
		}
		/// End
		auto glfwExtensions = GetRequiredExtensions();

		VkInstanceCreateInfo CreateInfo;

		VkDebugUtilsMessengerCreateInfoEXT DebugCreateInfo;
		if (EnableValidationLayers)
		{
			CreateInfo.enabledLayerCount = static_cast<uint32_t>(ValidationLayers.size());
			CreateInfo.ppEnabledLayerNames = ValidationLayers.data();
			PopulateDebugMessengerCreateInfo(DebugCreateInfo);
			CreateInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&DebugCreateInfo;
		}
		else
		{
			CreateInfo.enabledLayerCount = 0;
			CreateInfo.pNext = nullptr;
		}

		CreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		CreateInfo.pApplicationInfo = &AppInfo;
		CreateInfo.flags = 0;
		CreateInfo.enabledExtensionCount = static_cast<uint32_t>(glfwExtensions.size());
		CreateInfo.ppEnabledExtensionNames = glfwExtensions.data();


		if (vkCreateInstance(&CreateInfo, nullptr, &Instance) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create instace");
		}
	}

	std::vector<const char*> GetRequiredExtensions()
	{
		uint32_t glfwExtensionCount = 0;
		const char** glfwExtensions = nullptr;
		glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

		std::vector<const char*> Extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

		if (EnableValidationLayers)
			Extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
		return Extensions;
	}

	void SetupDebugMessenger()
	{
		if (!EnableValidationLayers)return;

		VkDebugUtilsMessengerCreateInfoEXT CreateInfo;
		PopulateDebugMessengerCreateInfo(CreateInfo);

		if (CreateDebugUtilsMessagerEXT(Instance, &CreateInfo, nullptr, &DebugMessenger) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to set up debug messenger!");
		}
	}

	void PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& CreateInfo)
	{
		CreateInfo = {};
		CreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
		CreateInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
									VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
									VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
		CreateInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
								VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
								VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
		CreateInfo.pfnUserCallback = DebugCallback;
	}

	void DestroyDebugUtilMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator)
	{
		auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)
			vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");

		if (func != nullptr)
		{
			func(instance, debugMessenger, pAllocator);
		}
	}

	VkResult CreateDebugUtilsMessagerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
		const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger)
	{
		auto func = (PFN_vkCreateDebugUtilsMessengerEXT)
			vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");

		if (func != nullptr)
			return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
		else
			return VK_ERROR_EXTENSION_NOT_PRESENT;
	}

	static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(
		VkDebugUtilsMessageSeverityFlagBitsEXT MessageSeverity,
		VkDebugUtilsMessageTypeFlagsEXT MessageType,
		const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
		void* pUserData)
	{
		std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;

		return VK_FALSE;
	}

	bool CheckValidationLayerSupport()
	{
		uint32_t LayerCount;
		vkEnumerateInstanceLayerProperties(&LayerCount, nullptr);

		std::vector<VkLayerProperties> AvailableLayers(LayerCount);
		vkEnumerateInstanceLayerProperties(&LayerCount, AvailableLayers.data());

		for (const char* LayerName : ValidationLayers)
		{
			bool LayerFound = false;

			for(const auto& LayerProperties : AvailableLayers)
			{
				if (strcmp(LayerName, LayerProperties.layerName) == 0)
				{
					LayerFound = true;
					break;
				}
			}

			if (!LayerFound)
				return false;
		}

		return true;
	}

	bool CheckDeviceExtensionsSupport(VkPhysicalDevice DeviceParam)
	{
		uint32_t ExtensionCount = 0;
		vkEnumerateDeviceExtensionProperties(DeviceParam, nullptr, &ExtensionCount, nullptr);
		std::vector<VkExtensionProperties> AvailableExtensions(ExtensionCount);
		vkEnumerateDeviceExtensionProperties(DeviceParam, nullptr, &ExtensionCount, AvailableExtensions.data());

		std::set<std::string> RequiredExtensions(DeviceExtensions.begin(), DeviceExtensions.end());

		for (const auto& Iter : AvailableExtensions)
		{
			RequiredExtensions.erase(Iter.extensionName);
		}

		return RequiredExtensions.empty();
	}

	void CreateSurface()
	{
		if (glfwCreateWindowSurface(Instance, Window, nullptr, &Surface) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create window surface");
		}
	}

	void PickPhysicalDevice()
	{
		uint32_t DeviceCount = 0;
		vkEnumeratePhysicalDevices(Instance, &DeviceCount, nullptr);

		if (DeviceCount == 0)
		{
			throw std::runtime_error("failed to find GPUs with Vulkan support");
		}

		std::vector<VkPhysicalDevice> Devices(DeviceCount);
		vkEnumeratePhysicalDevices(Instance, &DeviceCount, Devices.data());

		// First, Choose a device that support the specified feature, such as geometry shader
		for (const auto& Iter : Devices)
		{
			if (IsDeviceSuitable(Iter))
			{
				PhysicDevice = Iter;
				break;
			}
		}

		// Second, Choose a heighest performance deviceW
		/*std::multimap<int, VkPhysicalDevice> Candidates;

		for (const auto& Iter : Devices)
		{
			int Score = RateDeviceSuitability(Iter);
			Candidates.insert(std::make_pair(Score, Iter));
		}

		if (Candidates.rbegin()->first > 0)
			PhysicDevice = Candidates.rbegin()->second;
		else
			throw std::runtime_error("failed to find a suitable GPU");*/

		if (PhysicDevice == VK_NULL_HANDLE)
		{
			throw std::runtime_error("failed to find a suitable GPU");
		}
	}

	bool IsDeviceSuitable(VkPhysicalDevice DeviceParam)
	{
		//Check by supported queue family
		QueueFamilyIndices Indices = FindQueueFamilies(DeviceParam);

		bool ExtensionsSupported = CheckDeviceExtensionsSupport(DeviceParam);

		bool SwapChainAdequate = false;
		if (ExtensionsSupported)
		{
			SwapChainSupportDetails SwapChainSupport = QuerySwapChainSupport(DeviceParam);
			SwapChainAdequate = !SwapChainSupport.Formats.empty() && !SwapChainSupport.PresentModes.empty();
		}
		return Indices.GraphicsFamily.has_value() && ExtensionsSupported && SwapChainAdequate;

		/*VkPhysicalDeviceProperties DeviceProperties;
		vkGetPhysicalDeviceProperties(Device, &DeviceProperties);
		
		VkPhysicalDeviceFeatures DeviceFeatures;
		vkGetPhysicalDeviceFeatures(Device, &DeviceFeatures);

		return DeviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_CPU && DeviceFeatures.geometryShader;*/
	}

	int RateDeviceSuitability(VkPhysicalDevice Device)
	{
		VkPhysicalDeviceProperties DeviceProperties;
		vkGetPhysicalDeviceProperties(Device, &DeviceProperties);
		VkPhysicalDeviceFeatures DeviceFeatures;
		vkGetPhysicalDeviceFeatures(Device, &DeviceFeatures);

		int Score = 0;

		if (DeviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
			Score += 1000;
		Score += DeviceProperties.limits.maxImageDimension2D;

		if (!DeviceFeatures.geometryShader)
			return 0;

		return Score;
	}

	struct QueueFamilyIndices
	{
		std::optional<uint32_t> GraphicsFamily;
		std::optional<uint32_t> PresentFamily;

		bool IsComplete()
		{
			return GraphicsFamily.has_value() &&
				PresentFamily.has_value();
		}
	};

	struct SwapChainSupportDetails
	{
		VkSurfaceCapabilitiesKHR Capabilities;
		std::vector<VkSurfaceFormatKHR> Formats;
		std::vector<VkPresentModeKHR> PresentModes;
	};

	VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& AvailableFormats)
		{
			for (const auto& AvailableFormat : AvailableFormats)
			{
				if (AvailableFormat.format == VK_FORMAT_B8G8R8A8_SRGB &&
					AvailableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
					return AvailableFormat;
			}
			return AvailableFormats[0];
		}

		VkPresentModeKHR ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& AvailablePresentModes)
		{
			for (const auto& AvailablePresentMode : AvailablePresentModes)
			{
				// This mode use triple buffer, it has low latency and avoid tearing, very good
				if (AvailablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR)
					return AvailablePresentMode;
			}
			return VK_PRESENT_MODE_FIFO_KHR;
		}

		// Choose the swap buffer size
		VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& Capabilities)
		{
			// if Capabilities.currentExtent.width == UINT32_MAX means that we can use differ resolution compared with window resolution
			if (Capabilities.currentExtent.width != UINT32_MAX)
			{
				return Capabilities.currentExtent;
			}
			else
			{
				int Width, Height;
				glfwGetFramebufferSize(Window, &Width, &Height);
				VkExtent2D ActualExtent = { static_cast<uint32_t>(Width), static_cast<uint32_t>(Height) };
				ActualExtent.width = std::max(Capabilities.minImageExtent.width, std::min(Capabilities.maxImageExtent.width, ActualExtent.width));
				ActualExtent.height = std::max(Capabilities.minImageExtent.height, std::min(Capabilities.maxImageExtent.height, ActualExtent.height));
			}

			return VkExtent2D();
		}

	// Find queue family
	QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice Device)
	{
		QueueFamilyIndices Indices;

		uint32_t QueueFamilyCount = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(Device, &QueueFamilyCount, nullptr);
		std::vector<VkQueueFamilyProperties> QueueFamilies(QueueFamilyCount);
		vkGetPhysicalDeviceQueueFamilyProperties(Device, &QueueFamilyCount, QueueFamilies.data());

		VkBool32 PresentSupport = false;

		int i = 0;
		for (const auto& Iter : QueueFamilies)
		{
			if (Iter.queueFlags & VK_QUEUE_GRAPHICS_BIT)
				Indices.GraphicsFamily = i;

			// check weather this device support current surface
			vkGetPhysicalDeviceSurfaceSupportKHR(Device, i, Surface, &PresentSupport);
			if (PresentSupport)
				Indices.PresentFamily = i;

			if (Indices.IsComplete())
				break;
			i++;
		}

		return Indices;
	}

	SwapChainSupportDetails QuerySwapChainSupport(VkPhysicalDevice DeviceParam)
	{
		SwapChainSupportDetails Details;

		// Basic surface capabilities(min/max number of images in swap chain, and min/max size of images)
		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(DeviceParam, Surface, &Details.Capabilities);

		// Surface format
		uint32_t FormatCount;
		vkGetPhysicalDeviceSurfaceFormatsKHR(DeviceParam, Surface, &FormatCount, nullptr);
		if (FormatCount != 0)
		{
			Details.Formats.resize(FormatCount);
			vkGetPhysicalDeviceSurfaceFormatsKHR(DeviceParam, Surface, &FormatCount, Details.Formats.data());
		}

		// Available surface presentation modes
		uint32_t PresentModeCount;
		vkGetPhysicalDeviceSurfacePresentModesKHR(DeviceParam, Surface, &PresentModeCount, nullptr);

		if (PresentModeCount != 0)
		{
			Details.PresentModes.resize(PresentModeCount);
			vkGetPhysicalDeviceSurfacePresentModesKHR(DeviceParam, Surface, &PresentModeCount, Details.PresentModes.data());
		}

		return Details;
	}

	void CreateLogicalDevice()
	{
		QueueFamilyIndices Indices = FindQueueFamilies(PhysicDevice);

		VkDeviceQueueCreateInfo QueueCreateInfo{};
		QueueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		QueueCreateInfo.queueFamilyIndex = Indices.GraphicsFamily.value();
		QueueCreateInfo.queueCount = 1;

		float QueuePriority = 1.f;
		QueueCreateInfo.pQueuePriorities = &QueuePriority;

		VkPhysicalDeviceFeatures DeviceFeatures{};
		vkGetPhysicalDeviceFeatures(PhysicDevice, &DeviceFeatures);

		VkDeviceCreateInfo CreateInfo{};
		CreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		CreateInfo.pQueueCreateInfos = &QueueCreateInfo;
		CreateInfo.queueCreateInfoCount = 1;

		CreateInfo.pEnabledFeatures = &DeviceFeatures;
		CreateInfo.enabledExtensionCount = 0;

		if (EnableValidationLayers)
		{
			CreateInfo.enabledLayerCount = static_cast<uint32_t>(ValidationLayers.size());
			CreateInfo.ppEnabledLayerNames = ValidationLayers.data();
		}
		else
		{
			CreateInfo.enabledLayerCount = 0;
		}

		// Open swap extension
		CreateInfo.enabledExtensionCount = static_cast<uint32_t>(DeviceExtensions.size());
		CreateInfo.ppEnabledExtensionNames = DeviceExtensions.data();

		std::vector<VkDeviceQueueCreateInfo> QueueCreateInfos;
		std::set<uint32_t> UniqueQueueFamilies = { Indices.GraphicsFamily.value(), Indices.PresentFamily.value() };

		for (uint32_t QueueFamily : UniqueQueueFamilies)
		{
			VkDeviceQueueCreateInfo QueueCreateInfo{};
			QueueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			QueueCreateInfo.queueFamilyIndex = QueueFamily;
			QueueCreateInfo.queueCount = 1;
			QueueCreateInfo.pQueuePriorities = &QueuePriority;
			QueueCreateInfos.push_back(QueueCreateInfo);
		}

		CreateInfo.queueCreateInfoCount = static_cast<uint32_t>(QueueCreateInfos.size());
		CreateInfo.pQueueCreateInfos = QueueCreateInfos.data();

		if (vkCreateDevice(PhysicDevice, &CreateInfo, nullptr, &Device) != VK_SUCCESS)
			throw std::runtime_error("failed to create logical device");

		// Get device queue
		vkGetDeviceQueue(Device, Indices.GraphicsFamily.value(), 0, &GraphicsQueue);
		vkGetDeviceQueue(Device, Indices.PresentFamily.value(), 0, &PresentQueue);
	}

	void CreateSwapChain()
	{
		SwapChainSupportDetails SwapChainSupport = QuerySwapChainSupport(PhysicDevice);

		VkSurfaceFormatKHR SurfaceFormat = ChooseSwapSurfaceFormat(SwapChainSupport.Formats);
		VkPresentModeKHR PresentMode = ChooseSwapPresentMode(SwapChainSupport.PresentModes);
		VkExtent2D Extent = ChooseSwapExtent(SwapChainSupport.Capabilities);

		uint32_t ImageCount = SwapChainSupport.Capabilities.minImageCount + 1;
		if (SwapChainSupport.Capabilities.maxImageCount > 0 && ImageCount > SwapChainSupport.Capabilities.maxImageCount)
			ImageCount = SwapChainSupport.Capabilities.maxImageCount;

		VkSwapchainCreateInfoKHR CreateInfo{};
		CreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		CreateInfo.surface = Surface;
		CreateInfo.minImageCount = ImageCount;
		CreateInfo.imageFormat = SurfaceFormat.format;
		CreateInfo.imageColorSpace = SurfaceFormat.colorSpace;
		CreateInfo.imageExtent = Extent;
		CreateInfo.imageArrayLayers = 1;
		CreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

		QueueFamilyIndices Indices = FindQueueFamilies(PhysicDevice);
		uint32_t QueueFamilyIndices[] = { Indices.GraphicsFamily.value(), Indices.PresentFamily.value() };

		if (Indices.GraphicsFamily != Indices.PresentFamily)
		{
			CreateInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
			CreateInfo.queueFamilyIndexCount = 2;
			CreateInfo.pQueueFamilyIndices = QueueFamilyIndices;
		}
		else
		{
			CreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
			CreateInfo.queueFamilyIndexCount = 0;
			CreateInfo.pQueueFamilyIndices = nullptr;
		}
		CreateInfo.preTransform = SwapChainSupport.Capabilities.currentTransform;
		CreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		CreateInfo.presentMode = PresentMode;
		CreateInfo.clipped = VK_TRUE;
		CreateInfo.oldSwapchain = VK_NULL_HANDLE;   //used when swap chain recreated(resize viewport...)

		if (vkCreateSwapchainKHR(Device, &CreateInfo, nullptr, &SwapChain) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create swap chain!");
		}

		vkGetSwapchainImagesKHR(Device, SwapChain, &ImageCount, nullptr);
		SwapChainImages.resize(ImageCount);
		vkGetSwapchainImagesKHR(Device, SwapChain, &ImageCount, SwapChainImages.data());

		SwapChainImageFormat = SurfaceFormat.format;
		SwapChainExtent = Extent;
	}

	void CreateImageViews()
	{
		SwapChainImageViews.resize(SwapChainImages.size());
		for (size_t i = 0; i < SwapChainImages.size(); ++i)
		{
			VkImageViewCreateInfo CreateInfo{};
			CreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			CreateInfo.image = SwapChainImages[i];
			CreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
			CreateInfo.format = SwapChainImageFormat;

			CreateInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
			CreateInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
			CreateInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
			CreateInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

			CreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			CreateInfo.subresourceRange.baseMipLevel = 0;
			CreateInfo.subresourceRange.levelCount = 1;
			CreateInfo.subresourceRange.baseArrayLayer = 0;
			CreateInfo.subresourceRange.layerCount = 1;

			if (vkCreateImageView(Device, &CreateInfo, nullptr, &SwapChainImageViews[i]) != VK_SUCCESS)
			{
				throw std::runtime_error("failed to create views");
			}
		}
	}

	/// For pipeline
	///  

	void CreateRenderPass()
	{
		VkAttachmentDescription ColorAttachment{};
		ColorAttachment.format = SwapChainImageFormat;
		ColorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
		ColorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		ColorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		ColorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		ColorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;  // content of framebuffer will be undefined after rendering operation 
		ColorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		ColorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;   // presented in the swap chain

		//Subpasses: subpasses are subsequent rendering operations that depend on the contents of framebuffers in previous passes
		VkAttachmentReference ColorAttachmentRef{};
		ColorAttachmentRef.attachment = 0;
		ColorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkSubpassDescription Subpass{};
		Subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		Subpass.colorAttachmentCount = 1;
		Subpass.pColorAttachments = &ColorAttachmentRef;

		VkRenderPassCreateInfo RenderPassInfo{};
		RenderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		RenderPassInfo.attachmentCount = 1;
		RenderPassInfo.pAttachments = &ColorAttachment;
		RenderPassInfo.subpassCount = 1;
		RenderPassInfo.pSubpasses = &Subpass;

		VkSubpassDependency Dependency{};
		Dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
		Dependency.dstSubpass = 0;
		Dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		Dependency.srcAccessMask = 0;
		Dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		Dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

		RenderPassInfo.dependencyCount = 1;
		RenderPassInfo.pDependencies = &Dependency;

		if (vkCreateRenderPass(Device, &RenderPassInfo, nullptr, &RenderPass) != VK_SUCCESS) 
		{
			throw std::runtime_error("failed to create render pass!");
		}
	}

	void CreateDescriptorSetLayout()
	{
		VkDescriptorSetLayoutBinding UBOLayoutBindings{};
		UBOLayoutBindings.binding = 0;
		UBOLayoutBindings.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		UBOLayoutBindings.descriptorCount = 1;
		UBOLayoutBindings.stageFlags = VK_SHADER_STAGE_VERTEX_BIT; // Decide the descriptor referenced by which shader stage
		UBOLayoutBindings.pImmutableSamplers = nullptr;

		VkDescriptorSetLayoutCreateInfo LayoutInfo{};
		LayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		LayoutInfo.bindingCount = 1;
		LayoutInfo.pBindings = &UBOLayoutBindings;

		if (vkCreateDescriptorSetLayout(Device, &LayoutInfo, nullptr, &DescriptorSetLayout) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create descriptor set layout");
		}
	}

	void CreateGraphicsPipeline()
	{
		auto VertShaderCode = ReadFile("Shaders/vert.spv");
		auto FragShaderCode = ReadFile("Shaders/frag.spv");

		VkShaderModule VertShaderModule = CreateShaderModule(VertShaderCode);
		VkShaderModule FragShaderModule = CreateShaderModule(FragShaderCode);

		VkPipelineShaderStageCreateInfo VertShaderStageInfo{};
		VertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		VertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
		VertShaderStageInfo.module = VertShaderModule;
		VertShaderStageInfo.pName = "main";
		VertShaderStageInfo.pSpecializationInfo = nullptr;

		VkPipelineShaderStageCreateInfo FragShaderStageInfo{};
		FragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		FragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
		FragShaderStageInfo.module = FragShaderModule;
		FragShaderStageInfo.pName = "main";
		FragShaderStageInfo.pSpecializationInfo = nullptr;

		auto BindingDescription = Vertex::GetBindingDescription();
		auto AttributeDescriptions = Vertex::GetAttributeDescriptions();

		VkPipelineVertexInputStateCreateInfo VertexInputInfo{};
		VertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		VertexInputInfo.vertexBindingDescriptionCount = 0;
		VertexInputInfo.pVertexBindingDescriptions = nullptr;
		VertexInputInfo.vertexAttributeDescriptionCount = 0;
		VertexInputInfo.pVertexAttributeDescriptions = 0;
		VertexInputInfo.vertexBindingDescriptionCount = 1;
		VertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(AttributeDescriptions.size());
		VertexInputInfo.pVertexBindingDescriptions = &BindingDescription;
		VertexInputInfo.pVertexAttributeDescriptions = AttributeDescriptions.data();

		VkPipelineInputAssemblyStateCreateInfo InputAssembly{};
		InputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		InputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		InputAssembly.primitiveRestartEnable = VK_FALSE;

		VkViewport ViewPort{};
		ViewPort.x = 0.0f;
		ViewPort.y = 0.0f;
		ViewPort.width = (float)SwapChainExtent.width;
		ViewPort.height = (float)SwapChainExtent.height;
		ViewPort.minDepth = 0.f;
		ViewPort.maxDepth = 1.f;

		VkRect2D Scissor{};
		Scissor.offset = { 0, 0 };
		Scissor.extent = SwapChainExtent;

		VkPipelineViewportStateCreateInfo ViewportState{};
		ViewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		ViewportState.viewportCount = 1;
		ViewportState.pViewports = &ViewPort;
		ViewportState.scissorCount = 1;
		ViewportState.pScissors = &Scissor;

		VkPipelineRasterizationStateCreateInfo Rasterizer{};
		Rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		Rasterizer.depthClampEnable = VK_FALSE;
		Rasterizer.rasterizerDiscardEnable = VK_FALSE;
		Rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
		Rasterizer.lineWidth = 1.0f;
		Rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
		Rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
		Rasterizer.depthBiasEnable = VK_FALSE;
		Rasterizer.depthBiasConstantFactor = 0.0f;
		Rasterizer.depthBiasClamp = 0.0f;
		Rasterizer.depthBiasSlopeFactor = 0.0f;

		VkPipelineMultisampleStateCreateInfo Multisampling{};
		Multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		Multisampling.sampleShadingEnable = VK_FALSE;
		Multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
		Multisampling.minSampleShading = 1.f;
		Multisampling.pSampleMask = nullptr;
		Multisampling.alphaToCoverageEnable = VK_FALSE;
		Multisampling.alphaToOneEnable = VK_FALSE;

		//VkPipelineDepthStencilStateCreateInfo
		VkPipelineColorBlendAttachmentState ColorBlendAttachment{};
		ColorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | 
											  VK_COLOR_COMPONENT_G_BIT |
											  VK_COLOR_COMPONENT_B_BIT |
											  VK_COLOR_COMPONENT_A_BIT;
		ColorBlendAttachment.blendEnable = VK_FALSE;
		ColorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
		ColorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
		ColorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
		ColorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
		ColorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
		ColorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

		// Enable blend
		/*ColorBlendAttachment.blendEnable = VK_TRUE;
		ColorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
		ColorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
		ColorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
		ColorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
		ColorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
		ColorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;*/

		VkPipelineColorBlendStateCreateInfo ColorBlending{};
		ColorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		ColorBlending.logicOpEnable = VK_FALSE;
		ColorBlending.logicOp = VK_LOGIC_OP_COPY;
		ColorBlending.attachmentCount = 1;
		ColorBlending.pAttachments = &ColorBlendAttachment;
		ColorBlending.blendConstants[0] = 0.f;
		ColorBlending.blendConstants[1] = 0.f;
		ColorBlending.blendConstants[2] = 0.f;
		ColorBlending.blendConstants[3] = 0.f;

		/*VkDynamicState DynamicStates[] = {
			VK_DYNAMIC_STATE_VIEWPORT,
			VK_DYNAMIC_STATE_LINE_WIDTH
		};

		VkPipelineDynamicStateCreateInfo DynamicState{};
		DynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		DynamicState.dynamicStateCount = 2;
		DynamicState.pDynamicStates = DynamicStates;*/

		VkPipelineLayoutCreateInfo PipelineLayoutInfo{};
		PipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		PipelineLayoutInfo.setLayoutCount = 1;
		PipelineLayoutInfo.pSetLayouts = &DescriptorSetLayout;
		PipelineLayoutInfo.pushConstantRangeCount = 0;
		PipelineLayoutInfo.pPushConstantRanges = nullptr;

		if (vkCreatePipelineLayout(Device, &PipelineLayoutInfo, nullptr, &PipelineLayout) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create pipeline layout!");
		}
		
		VkPipelineShaderStageCreateInfo ShaderStages[2] = { VertShaderStageInfo, FragShaderStageInfo };

		VkGraphicsPipelineCreateInfo PipelineInfo{};
		PipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		PipelineInfo.stageCount = 2;
		PipelineInfo.pStages = ShaderStages;

		PipelineInfo.pVertexInputState = &VertexInputInfo;
		PipelineInfo.pInputAssemblyState = &InputAssembly;
		PipelineInfo.pViewportState = &ViewportState;
		PipelineInfo.pRasterizationState = &Rasterizer;
		PipelineInfo.pMultisampleState = &Multisampling;
		PipelineInfo.pDepthStencilState = nullptr;
		PipelineInfo.pColorBlendState = &ColorBlending;
		PipelineInfo.pDynamicState = nullptr;
		PipelineInfo.layout = PipelineLayout;
		PipelineInfo.renderPass = RenderPass;
		PipelineInfo.subpass = 0;
		PipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
		//PipelineInfo.basePipelineIndex = -1;

		if (vkCreateGraphicsPipelines(Device, VK_NULL_HANDLE, 1, &PipelineInfo, nullptr, &GraphicsPipeline) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create graphics pipeline");
		}

		vkDestroyShaderModule(Device, FragShaderModule, nullptr);
		vkDestroyShaderModule(Device, VertShaderModule, nullptr);

	}

	VkShaderModule CreateShaderModule(const std::vector<char>& Code)
	{
		VkShaderModuleCreateInfo CreateInfo{};
		CreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		CreateInfo.codeSize = Code.size();
		CreateInfo.pCode = reinterpret_cast<const uint32_t*>(Code.data());

		VkShaderModule ShaderModule;
		if (vkCreateShaderModule(Device, &CreateInfo, nullptr, &ShaderModule) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create shader module");
		}

		return ShaderModule;
	}

	void CreateFramebuffers()
	{
		SwapChainFrambuffers.resize(SwapChainImageViews.size());

		for (size_t i = 0; i < SwapChainImageViews.size(); ++i)
		{
			VkImageView Attachments[] = { SwapChainImageViews[i] };

			VkFramebufferCreateInfo FramebufferInfo{};
			FramebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			FramebufferInfo.renderPass = RenderPass;
			FramebufferInfo.attachmentCount = 1;
			FramebufferInfo.pAttachments = Attachments;
			FramebufferInfo.width = SwapChainExtent.width;
			FramebufferInfo.height = SwapChainExtent.height;
			FramebufferInfo.layers = 1;

			if (vkCreateFramebuffer(Device, &FramebufferInfo, nullptr, &SwapChainFrambuffers[i]) != VK_SUCCESS)
			{
				throw std::runtime_error("failed to create frambuffer");
			}
		}
	}
	/// For pipeline

	/// For command
	///
	void CreateCommandPool()
	{
		QueueFamilyIndices QueueFamilyIndices = FindQueueFamilies(PhysicDevice);

		VkCommandPoolCreateInfo PoolInfo;
		PoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		PoolInfo.queueFamilyIndex = QueueFamilyIndices.GraphicsFamily.value();
		PoolInfo.flags = 0;

		PoolInfo.pNext = nullptr;

		if (vkCreateCommandPool(Device, &PoolInfo, nullptr, &CommandPool) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create command pool!");
		}
	}

	void CreateCommandBuffers()
	{
		//each swapchainframebuffer need a command buffer
		CommandBuffer.resize(SwapChainFrambuffers.size());

		VkCommandBufferAllocateInfo AllocInfo{};
		AllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		AllocInfo.commandPool = CommandPool;
		AllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;  //this buffer can not be called from other command buffers
		AllocInfo.commandBufferCount = (uint32_t)CommandBuffer.size();
		
		if (vkAllocateCommandBuffers(Device, &AllocInfo, CommandBuffer.data()) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to allocate command buffer");
		}

		for (size_t i = 0; i < CommandBuffer.size(); ++i)
		{
			VkCommandBufferBeginInfo BeginInfo{};
			BeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
			BeginInfo.flags = 0;
			BeginInfo.pInheritanceInfo = nullptr;

			if (vkBeginCommandBuffer(CommandBuffer[i], &BeginInfo) != VK_SUCCESS)
			{
				throw std::runtime_error("failed to begin recording command buffer!");
			}

			VkRenderPassBeginInfo RenderPassInfo{};
			RenderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
			RenderPassInfo.renderPass = RenderPass;
			RenderPassInfo.framebuffer = SwapChainFrambuffers[i];
			RenderPassInfo.renderArea.offset = { 0,0 };
			RenderPassInfo.renderArea.extent = SwapChainExtent;

			VkClearValue ClearColor = { 0.f, 0.f, 0.f, 1.f };
			RenderPassInfo.clearValueCount = 1;
			RenderPassInfo.pClearValues = &ClearColor;

			vkCmdBeginRenderPass(CommandBuffer[i], &RenderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
			vkCmdBindPipeline(CommandBuffer[i], VK_PIPELINE_BIND_POINT_GRAPHICS, GraphicsPipeline); //VK_PIPELINE_BIND_POINT_GRAPHICS means that pipeline is graphics pipeline

			VkDeviceSize Offsets[1] = { 0 };
			vkCmdBindVertexBuffers(CommandBuffer[i], 0, 1, &VertexBuffer, Offsets);
			
			vkCmdBindIndexBuffer(CommandBuffer[i], IndexBuffer, 0, VK_INDEX_TYPE_UINT16);
			
			vkCmdBindDescriptorSets(CommandBuffer[i], VK_PIPELINE_BIND_POINT_GRAPHICS, PipelineLayout, 0, 1, &DescriptorSets[i], 0, nullptr);
			vkCmdDrawIndexed(CommandBuffer[i], static_cast<uint32_t>(Indices.size()), 1, 0, 0, 0);
			//vkCmdDraw(CommandBuffer[i], 3, 1, 0, 0);
			vkCmdEndRenderPass(CommandBuffer[i]);
			if (vkEndCommandBuffer(CommandBuffer[i]) != VK_SUCCESS)
			{
				throw std::runtime_error("failed to record command buffer");
			}
		}
	}

	void CreateBuffer(VkDeviceSize Size, VkBufferUsageFlags Usage, VkMemoryPropertyFlags Properties, VkBuffer& Buffer, VkDeviceMemory& BufferMemory)
	{
		VkBufferCreateInfo BufferInfo{};
		BufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		BufferInfo.size = Size;
		BufferInfo.usage = Usage;
		BufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		if (vkCreateBuffer(Device, &BufferInfo, nullptr, &Buffer) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create buffer");
		}

		VkMemoryRequirements MemRequirements;
		vkGetBufferMemoryRequirements(Device, Buffer, &MemRequirements);

		VkMemoryAllocateInfo AllocInfo{};
		AllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		AllocInfo.allocationSize = MemRequirements.size;
		AllocInfo.memoryTypeIndex = FindMemoryType(MemRequirements.memoryTypeBits, Properties);

		if (vkAllocateMemory(Device, &AllocInfo, nullptr, &BufferMemory) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to allocate buffer memory!");
		}
		vkBindBufferMemory(Device, Buffer, BufferMemory, 0);
	}

	void CopyBuffer(VkBuffer SrcBuffer, VkBuffer DstBuffer, VkDeviceSize Size)
	{
		VkCommandBufferAllocateInfo AllocInfo{};
		AllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		AllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		AllocInfo.commandPool = CommandPool;
		AllocInfo.commandBufferCount = 1;
		
		VkCommandBuffer CommandBuffer;
		vkAllocateCommandBuffers(Device, &AllocInfo, &CommandBuffer);

		VkCommandBufferBeginInfo BeginInfo{};
		BeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		BeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

		vkBeginCommandBuffer(CommandBuffer, &BeginInfo);

		VkBufferCopy CopyRegion{};
		CopyRegion.srcOffset = 0;
		CopyRegion.dstOffset = 0;
		CopyRegion.size = Size;
		vkCmdCopyBuffer(CommandBuffer, SrcBuffer, DstBuffer, 1, &CopyRegion);

		vkEndCommandBuffer(CommandBuffer);

		VkSubmitInfo SubmitInfo{};
		SubmitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		SubmitInfo.commandBufferCount = 1;
		SubmitInfo.pCommandBuffers = &CommandBuffer;

		vkQueueSubmit(GraphicsQueue, 1, &SubmitInfo, VK_NULL_HANDLE);
		vkQueueWaitIdle(GraphicsQueue); // we can also use vkWaitForFences

		vkFreeCommandBuffers(Device, CommandPool, 1, &CommandBuffer);
	}

	void CreateVertexBuffers()
	{
		VkDeviceSize BufferSize = sizeof(Vertices[0]) * Vertices.size();
		// The staging buffer was used to copy data to actual vertex buffer
		VkBuffer StagingBuffer;
		VkDeviceMemory StagingBufferMemory;
		CreateBuffer(BufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
			VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, StagingBuffer, StagingBufferMemory);

		void* Data;
		vkMapMemory(Device, StagingBufferMemory, 0, BufferSize, 0, &Data);
		memcpy(Data, Vertices.data(), (size_t)BufferSize);
		vkUnmapMemory(Device, StagingBufferMemory);

		CreateBuffer(BufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
			VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, VertexBuffer, VertexBufferMemory);

		CopyBuffer(StagingBuffer, VertexBuffer, BufferSize);

		// Destroy staging buffer
		vkDestroyBuffer(Device, StagingBuffer, nullptr);
		vkFreeMemory(Device, StagingBufferMemory, nullptr);

		//VkBufferCreateInfo BufferInfo{};
		//BufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		//BufferInfo.size = sizeof(Vertices[0]) * Vertices.size();
		//BufferInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
		//BufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		//VkMemoryRequirements MemRequirements;
		//vkGetBufferMemoryRequirements(Device, VertexBuffer, &MemRequirements);

		//VkMemoryAllocateInfo AllocInfo{};
		//AllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		//AllocInfo.allocationSize = MemRequirements.size;
		//AllocInfo.memoryTypeIndex = FindMemoryType(MemRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT  // this memory can write it from cpu
		//																		| VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

		//if (vkAllocateMemory(Device, &AllocInfo, nullptr, &VertexBufferMemory) != VK_SUCCESS)
		//{
		//	throw std::runtime_error("failed to allocate vertex buffer memory!");
		//}

		//if(vkCreateBuffer(Device, &BufferInfo, nullptr, &VertexBuffer) != VK_SUCCESS)
		//{ 
		//	throw std::runtime_error("failed to create vertex buffer!");
		//}

		//vkBindBufferMemory(Device, VertexBuffer, VertexBufferMemory, 0);
	}

	void CreateIndexBuffers()
	{
		VkDeviceSize BufferSize = sizeof(Indices[0]) * Indices.size();

		VkBuffer StagingBuffer;
		VkDeviceMemory StagingBufferMemory;
		CreateBuffer(BufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			StagingBuffer, StagingBufferMemory);

		void* Data;
		vkMapMemory(Device, StagingBufferMemory, 0, BufferSize, 0, &Data);
		memcpy(Data, Indices.data(), (size_t)BufferSize);
		vkUnmapMemory(Device, StagingBufferMemory);

		CreateBuffer(BufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, IndexBuffer, IndexBufferMemory);

		CopyBuffer(StagingBuffer, IndexBuffer, BufferSize);
		vkFreeMemory(Device, StagingBufferMemory, nullptr);
	}

	void CreateUniformBuffers()
	{
		VkDeviceSize BufferSize = sizeof(UniformBufferObject);

		UniformBuffers.resize(SwapChainImages.size());
		UniformBuffersMemory.resize(SwapChainImages.size());

		for (size_t i = 0; i < SwapChainImages.size(); ++i)
		{
			CreateBuffer(BufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, 
							UniformBuffers[i], UniformBuffersMemory[i]);
		}
	}

	void UpdateUniformBuffer(uint32_t CurrentImage)
	{
		static auto StartTime = std::chrono::high_resolution_clock::now();
		auto CurrentTime = std::chrono::high_resolution_clock::now();
		float Time = std::chrono::duration<float, std::chrono::seconds::period>(CurrentTime - StartTime).count();

		UniformBufferObject Ubo{};
		Ubo.Model = glm::rotate(glm::mat4(1.f), Time * glm::radians(90.f), glm::vec3(0.f, 0.f, 1.f));
		Ubo.View = glm::lookAt(glm::vec3(2.f, 2.f, 2.f), glm::vec3(0.f, 0.f, 0.f), glm::vec3(0.f, 0.f, 1.f));
		Ubo.Proj = glm::perspective(glm::radians(45.f), SwapChainExtent.width / (float)SwapChainExtent.height, 0.1f, 10.f);
		Ubo.Proj[1][1] *= -1;
		
		void* Data;
		vkMapMemory(Device, UniformBuffersMemory[CurrentImage], 0, sizeof(Ubo), 0, &Data);
		memcpy(Data, &Ubo, sizeof(Ubo));
		vkUnmapMemory(Device, UniformBuffersMemory[CurrentImage]);
	}

	void CreateDescriptorPool()
	{
		VkDescriptorPoolSize PoolSize{};
		PoolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		PoolSize.descriptorCount = static_cast<uint32_t>(SwapChainImages.size());

		VkDescriptorPoolCreateInfo PoolInfo{};
		PoolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		PoolInfo.poolSizeCount = 1;
		PoolInfo.pPoolSizes = &PoolSize;
		PoolInfo.maxSets = static_cast<uint32_t>(SwapChainImages.size());

		if (vkCreateDescriptorPool(Device, &PoolInfo, nullptr, &DescriptorPool) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create descriptor pool!");
		}
	}

	void CreateDescriptorSets()
	{
		std::vector<VkDescriptorSetLayout> Layouts(SwapChainImages.size(), DescriptorSetLayout);
		VkDescriptorSetAllocateInfo AllocInfo{};
		AllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		AllocInfo.descriptorPool = DescriptorPool;
		AllocInfo.descriptorSetCount = static_cast<uint32_t>(SwapChainImages.size());
		AllocInfo.pSetLayouts = Layouts.data();

		DescriptorSets.resize(SwapChainImages.size());
		if (vkAllocateDescriptorSets(Device, &AllocInfo, DescriptorSets.data()) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to allocate descriptor sets!");
		}

		for (size_t i = 0; i < SwapChainImages.size(); ++i)
		{
			VkDescriptorBufferInfo BufferInfo{};
			BufferInfo.buffer = UniformBuffers[i];
			BufferInfo.offset = 0;
			BufferInfo.range = sizeof(UniformBufferObject);

			VkWriteDescriptorSet DescriptorWrite{};
			DescriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			DescriptorWrite.dstSet = DescriptorSets[i];
			DescriptorWrite.dstBinding = 0;
			DescriptorWrite.dstArrayElement = 0;
			DescriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			DescriptorWrite.descriptorCount = 1;

			DescriptorWrite.pBufferInfo = &BufferInfo;
			DescriptorWrite.pImageInfo = nullptr;
			DescriptorWrite.pTexelBufferView = nullptr;

			vkUpdateDescriptorSets(Device, 1, &DescriptorWrite, 0, nullptr);
		}
	}

	// because graphics offer different types of memory to allocate, we should find the right type of memory to use 
	uint32_t FindMemoryType(uint32_t TypeFilter, VkMemoryPropertyFlags Properties)
	{
		VkPhysicalDeviceMemoryProperties MemProperties;
		vkGetPhysicalDeviceMemoryProperties(PhysicDevice, &MemProperties);

		for (uint32_t i = 0; i < MemProperties.memoryTypeCount; ++i)
		{
			if (TypeFilter & (1 << i) && (MemProperties.memoryTypes[i].propertyFlags & Properties) == Properties)
				return i;
		}

		throw std::runtime_error("failed to find suitable memory type!");
	}

	/// For command
	
	void CreateSyncObjects()
	{
		ImageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
		RenderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
		InFlightFences.resize(MAX_FRAMES_IN_FLIGHT);
		ImagesInFlight.resize(SwapChainImages.size(), VK_NULL_HANDLE);

		VkSemaphoreCreateInfo SemphoreInfo{};
		SemphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

		VkFenceCreateInfo FenceInfo{};
		FenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		FenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
		{
			if (vkCreateSemaphore(Device, &SemphoreInfo, nullptr, &ImageAvailableSemaphores[i]) != VK_SUCCESS ||
				vkCreateSemaphore(Device, &SemphoreInfo, nullptr, &RenderFinishedSemaphores[i]) != VK_SUCCESS ||
				vkCreateFence(Device, &FenceInfo, nullptr, &InFlightFences[i]) != VK_SUCCESS)
			{
				throw std::runtime_error("failed to create semaphores!");
			}
		}
		
	}

	void DrawFrame()
	{
		vkWaitForFences(Device, 1, &InFlightFences[CurrentFrame], VK_TRUE, UINT64_MAX);

		uint32_t ImageIndex;
		VkResult Result = vkAcquireNextImageKHR(Device, SwapChain, UINT64_MAX, ImageAvailableSemaphores[CurrentFrame], VK_NULL_HANDLE, &ImageIndex);

		/*if (Result == VK_ERROR_OUT_OF_DATE_KHR)
		{
			RecreateSwapChain();
			return;
		}
		else if (Result != VK_SUCCESS && Result != VK_SUBOPTIMAL_KHR)
		{
			throw std::runtime_error("failed to acquire swap chain image");
		}*/

		if(ImagesInFlight[ImageIndex] != VK_NULL_HANDLE)
		{
			vkWaitForFences(Device, 1, &ImagesInFlight[ImageIndex], VK_TRUE, UINT64_MAX);
		}
		ImagesInFlight[ImageIndex] = InFlightFences[CurrentFrame];

		UpdateUniformBuffer(ImageIndex);

		VkSubmitInfo SubmitInfo{};
		SubmitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

		VkSemaphore WaitSemaphores[] = { ImageAvailableSemaphores[CurrentFrame] };
		VkPipelineStageFlags WaitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
		SubmitInfo.waitSemaphoreCount = 1;
		SubmitInfo.pWaitSemaphores = WaitSemaphores;
		SubmitInfo.pWaitDstStageMask = WaitStages;

		SubmitInfo.commandBufferCount = 1;
		SubmitInfo.pCommandBuffers = &CommandBuffer[ImageIndex];
		//SubmitInfo.pSubmits

		VkSemaphore SignalSemphores[] = { RenderFinishedSemaphores[CurrentFrame] };
		SubmitInfo.signalSemaphoreCount = 1;
		SubmitInfo.pSignalSemaphores = SignalSemphores;     //specify which semaphores to signal once the command buffer(s) have finished execution

		vkResetFences(Device, 1, &InFlightFences[CurrentFrame]);

		if (vkQueueSubmit(GraphicsQueue, 1, &SubmitInfo, InFlightFences[CurrentFrame]) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to submit draw command buffer!");
		}

		VkPresentInfoKHR PresentInfo{};
		PresentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

		PresentInfo.waitSemaphoreCount = 1;
		PresentInfo.pWaitSemaphores = SignalSemphores;

		VkSwapchainKHR SwapChains[] = { SwapChain };
		PresentInfo.swapchainCount = 1;
		PresentInfo.pSwapchains = SwapChains;
		PresentInfo.pImageIndices = &ImageIndex;
		PresentInfo.pResults = nullptr;

		Result = vkQueuePresentKHR(PresentQueue, &PresentInfo);
		if (Result == VK_ERROR_OUT_OF_DATE_KHR || Result == VK_SUBOPTIMAL_KHR || FramebufferResized) 
		{
			FramebufferResized = false;
			RecreateSwapChain();
		}
		else if (Result != VK_SUCCESS) 
		{
			throw std::runtime_error("failed to present swap chain image!");
		}
		//vkQueuePresentKHR(PresentQueue, &PresentInfo);

		CurrentFrame = (CurrentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
		//vkQueueWaitIdle(PresentQueue);
	}
	//validation layer: Validation Error: [ VUID-vkQueueSubmit-pCommandBuffers-00071 ] Object 0: handle = 0x25ed602e270, type = VK_OBJECT_TYPE_DEVICE; | MessageID = 0x2e2f4d65 | VkCommandBuffer 0x25ed88a9b90[] is already in use and is not marked for simultaneous use. The Vulkan spec states: If any element of the pCommandBuffers member of any element of pSubmits was not recorded with the VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT, it must not be in the pending state (https://vulkan.lunarg.com/doc/view/1.2.141.0/windows/1.2-extensions/vkspec.html#VUID-vkQueueSubmit-pCommandBuffers-00071)

	void RecreateSwapChain()
	{
		int Width = 0, Height = 0;

		glfwGetFramebufferSize(Window, &Width, &Height);
		while (Width == 0 || Height == 0)
		{
			glfwGetFramebufferSize(Window, &Width, &Height);
			glfwWaitEvents();
		}

		vkDeviceWaitIdle(Device);

		CleanupSwapChain();

		vkDestroyBuffer(Device, IndexBuffer, nullptr);
		vkFreeMemory(Device, IndexBufferMemory, nullptr);
		
		vkDestroyBuffer(Device, VertexBuffer, nullptr);
		vkFreeMemory(Device, VertexBufferMemory, nullptr);

		CreateSwapChain();
		CreateImageViews();
		CreateRenderPass();
		CreateGraphicsPipeline();
		CreateFramebuffers();
		CreateUniformBuffers();
		CreateDescriptorPool();
		CreateDescriptorSets();
		CreateCommandBuffers();
	}

	void CleanupSwapChain()
	{
		for (size_t i = 0; i < SwapChainFrambuffers.size(); ++i)
		{
			vkDestroyFramebuffer(Device, SwapChainFrambuffers[i], nullptr);

			vkDestroyBuffer(Device, UniformBuffers[i], nullptr);
			vkFreeMemory(Device, UniformBuffersMemory[i], nullptr);
		}
		vkDestroyDescriptorPool(Device, DescriptorPool, nullptr);

		vkFreeCommandBuffers(Device, CommandPool, static_cast<uint32_t>(CommandBuffer.size()), CommandBuffer.data());

		vkDestroyPipeline(Device, GraphicsPipeline, nullptr);
		vkDestroyPipelineLayout(Device, PipelineLayout, nullptr);
		vkDestroyRenderPass(Device, RenderPass, nullptr);

		for (size_t i = 0; i < SwapChainImageViews.size(); ++i)
		{
			vkDestroyImageView(Device, SwapChainImageViews[i], nullptr);
			//vkFreeMemory(Device, UniformBuffersMemory[i], nullptr);
		}

		vkDestroySwapchainKHR(Device, SwapChain, nullptr);
	}

	void MainLoop()
	{
		while (!glfwWindowShouldClose(Window))
		{
			glfwPollEvents();
			DrawFrame();
		}

		vkQueueWaitIdle(PresentQueue);
	}

	void Cleanup()
	{
		CleanupSwapChain();

		vkDestroyBuffer(Device, VertexBuffer, nullptr);
		vkFreeMemory(Device, VertexBufferMemory, nullptr);

		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
		{
			vkDestroySemaphore(Device, RenderFinishedSemaphores[i], nullptr);
			vkDestroySemaphore(Device, ImageAvailableSemaphores[i], nullptr);
			vkDestroyFence(Device, InFlightFences[i], nullptr);
		}

		vkDestroyCommandPool(Device, CommandPool, nullptr);
		
		vkDestroyDevice(Device, nullptr);

		if (EnableValidationLayers)
		{
			DestroyDebugUtilMessengerEXT(Instance, DebugMessenger, nullptr);
		}

		vkDestroySurfaceKHR(Instance, Surface, nullptr);
		vkDestroyInstance(Instance, nullptr);

		glfwDestroyWindow(Window);

		glfwTerminate();
	}

public:
	bool FramebufferResized = false;

private:
	GLFWwindow* Window = nullptr;

	VkInstance Instance;

	VkDebugUtilsMessengerEXT DebugMessenger;

	VkPhysicalDevice PhysicDevice = VK_NULL_HANDLE;

	VkDevice Device;

	VkQueue GraphicsQueue;

	VkSurfaceKHR Surface;

	VkQueue PresentQueue;

	VkSwapchainKHR SwapChain;

	std::vector<VkImage> SwapChainImages;

	std::vector<VkImageView> SwapChainImageViews;

	VkFormat SwapChainImageFormat;

	VkExtent2D SwapChainExtent;

	VkRenderPass RenderPass;
	VkDescriptorSetLayout DescriptorSetLayout;
	VkPipelineLayout PipelineLayout;

	VkPipeline GraphicsPipeline;

	std::vector<VkFramebuffer> SwapChainFrambuffers;

	VkCommandPool CommandPool;

	std::vector<VkCommandBuffer> CommandBuffer;

	std::vector<VkSemaphore> ImageAvailableSemaphores;
	std::vector<VkSemaphore> RenderFinishedSemaphores;
	std::vector<VkFence> InFlightFences;
	std::vector<VkFence> ImagesInFlight;

	//bool FramebufferResized = false;

	VkBuffer VertexBuffer;
	VkDeviceMemory VertexBufferMemory;

	VkBuffer IndexBuffer;
	VkDeviceMemory IndexBufferMemory;

	std::vector<VkBuffer> UniformBuffers;
	std::vector<VkDeviceMemory> UniformBuffersMemory;

	VkDescriptorPool DescriptorPool;
	std::vector<VkDescriptorSet> DescriptorSets;

	size_t CurrentFrame = 0;
};

static void FramebufferResizeCallback(GLFWwindow* Window, int Width, int Height)
{
	auto App = reinterpret_cast<HelloTriangleApplication*>(glfwGetWindowUserPointer(Window));
	if (App)
	{
		App->FramebufferResized = true;
	}
}


//template <typename T, uint32_t N>
//char(&UE4ArrayCountHelper(const T(&)[N]))[N];
//
//
//template <typename T, int Val>
//T&& testVa = Val;
//
//
//constexpr int N = 3;
//
//
//
//char(
//	&ArrayTest(
//		int
//	)
//)[N];



int main()
{
	/*char str1[] = "adfsafaf";
	memcpy(str1, str1 + 1, 3);

	uint32_t ut = 0xffff0000 >> 16;*/

	HelloTriangleApplication App;

	try 
	{
		App.run();
	}
	catch(const std::exception& e)
	{
		std::cerr << e.what() << std::endl;
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;


	/*glfwInit();
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

	uint32_t extensionCount = 0;
	GLFWwindow* window = glfwCreateWindow(800, 600, "VK Window", nullptr, nullptr);

	std::cout << extensionCount << "extensions supported\n";

	glm::mat4 matrix;
	glm::vec4 vec;
	auto test = matrix * vec;

	while (!glfwWindowShouldClose(window))
	{
		glfwPollEvents();
	}

	glfwDestroyWindow(window);
		
	glfwTerminate();
	return 0;*/
}