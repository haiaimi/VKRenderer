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

const uint32_t WIDTH = 800;
const uint32_t HEIGHT = 600;

const std::vector<const char*> ValidationLayers = { "VK_LAYER_KHRONOS_validation" };

#ifdef NDEBUG
	const bool EnableValidationLayerss = false;
#else
	const bool EnableValidationLayers = true;
#endif // NDEBUG


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
	}

	void InitVulkan()
	{
		CreateInstance();
		SetupDebugMessenger();
		PickPhysicalDevice();
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
		/*for (const auto& Iter : Devices)
		{
			if (IsDeviceSuitable(Iter))
			{
				PhysicDevice = Iter;
				break;
			}
		}*/

		// Second, Choose a heighest performance deviceW
		std::multimap<int, VkPhysicalDevice> Candidates;

		for (const auto& Iter : Devices)
		{
			int Score = RateDeviceSuitability(Iter);
			Candidates.insert(std::make_pair(Score, Iter));
		}

		if (Candidates.rbegin()->first > 0)
			PhysicDevice = Candidates.rbegin()->second;
		else
			throw std::runtime_error("failed to find a suitable GPU");

		if (PhysicDevice == VK_NULL_HANDLE)
		{
			throw std::runtime_error("failed to find a suitable GPU");
		}
	}

	bool IsDeviceSuitable(VkPhysicalDevice Device)
	{
		// Check by supported queue family
		//QueueFamilyIndices Indices = FindQueueFamilies(Device);
		//return Indices.GraphicsFamily.has_value();

		VkPhysicalDeviceProperties DeviceProperties;
		vkGetPhysicalDeviceProperties(Device, &DeviceProperties);
		
		VkPhysicalDeviceFeatures DeviceFeatures;
		vkGetPhysicalDeviceFeatures(Device, &DeviceFeatures);

		return DeviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_CPU && DeviceFeatures.geometryShader;
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

		bool IsComplete()
		{
			return GraphicsFamily.has_value();
		}
	};

	// Find queue family
	QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice Device)
	{
		QueueFamilyIndices Indices;

		uint32_t QueueFamilyCount = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(Device, &QueueFamilyCount, nullptr);
		std::vector<VkQueueFamilyProperties> QueueFamilies(QueueFamilyCount);
		vkGetPhysicalDeviceQueueFamilyProperties(Device, &QueueFamilyCount, QueueFamilies.data());

		int i = 0;
		for (const auto& Iter : QueueFamilies)
		{
			if (Iter.queueFlags & VK_QUEUE_GRAPHICS_BIT)
				Indices.GraphicsFamily = i;

			if (Indices.IsComplete())
				break;
			i++;
		}

		return Indices;
	}

	void MainLoop()
	{
		while (!glfwWindowShouldClose(Window))
		{
			glfwPollEvents();
		}
	}

	void Cleanup()
	{
		if (EnableValidationLayers)
		{
			DestroyDebugUtilMessengerEXT(Instance, DebugMessenger, nullptr);
		}

		vkDestroyInstance(Instance, nullptr);

		glfwDestroyWindow(Window);

		glfwTerminate();
	}

private:
	GLFWwindow* Window = nullptr;

	VkInstance Instance;

	VkDebugUtilsMessengerEXT DebugMessenger;

	VkPhysicalDevice PhysicDevice = VK_NULL_HANDLE;
};

int main()
{
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