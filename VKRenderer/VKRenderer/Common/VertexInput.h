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

	static std::array<VkVertexInputBindingDescription, 2> GetAttributeDescriptions()
	{
	
	}
};



const std::vector<Vertex> Vertices =
{
	{{0.f, -0.5f}, {1.f, 0.f, 0.f}},
	{{0.5f, 0.5f}, {0.f, 1.f, 0.f}},
	{{-0.5f, 0.5f}, {0.f, 0.f, 1.f}}
}