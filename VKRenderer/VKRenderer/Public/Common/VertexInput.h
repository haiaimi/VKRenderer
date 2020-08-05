#pragma once
#include <glm.hpp>
#include <vector>
#include <array>
#include <vulkan\vulkan_core.h>


struct Vertex
{
	glm::vec2 Pos;
	glm::vec3 Color;

	static VkVertexInputBindingDescription GetBindingDescription()
	{
		VkVertexInputBindingDescription BindDescription{};
		BindDescription.binding = 0;
		BindDescription.stride = sizeof(Vertex);
		BindDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

		return BindDescription;
	}

	static std::array<VkVertexInputAttributeDescription, 2> GetAttributeDescriptions()
	{
		std::array<VkVertexInputAttributeDescription, 2> AttributeDescriptions{};

		AttributeDescriptions[0].binding = 0;
		AttributeDescriptions[0].location = 0;
		AttributeDescriptions[0].format = VK_FORMAT_R32G32_SFLOAT;
		AttributeDescriptions[0].offset = offsetof(Vertex, Pos);

		AttributeDescriptions[1].binding = 0;
		AttributeDescriptions[1].location = 1;
		AttributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
		AttributeDescriptions[1].offset = offsetof(Vertex, Color);

		return AttributeDescriptions;
	}
};

const std::vector<Vertex> Vertices =
{
	{{0.f, -0.5f}, {1.f, 0.f, 0.f}},
	{{0.5f, 0.5f}, {0.f, 1.f, 0.f}},
	{{-0.5f, 0.5f}, {0.f, 0.f, 1.f}}
};

const std::vector<uint16_t> Indices =
{
	0, 1, 2, 2, 3, 0
};