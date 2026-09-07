#pragma once

#include "Window.h"
#include "Pipeline.h"
#include "Device.h"
#include "Swapchain.h"
#include "Vertex.h"
#include "UniformBufferObject.h"

#include <cstdint>

namespace App {
	class FirstApp
	{
	private:
		static constexpr int width{ 800 };
		static constexpr int height{ 600 };

		void CreateVertexBuffer();
		void CreateIndexBuffer();
		void CreateDescriptorSetLayout();
		void CreateUniformBuffers();
		void CreateDescriptorPool();
		void CreateDescriptorSets();
		void UpdateUniformBuffer(uint32_t imageIndex);
		void CreatePipelineLayout();
		void CreatePipeline();
		void CreateCommandBuffers();
		void DrawFrame();


		const std::vector<Core::Vertex> vertices
		{
			{{-0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}},
			{{ 0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}},
			{{ 0.5f,  0.5f}, {0.0f, 0.0f, 1.0f}},
			{{-0.5f,  0.5f}, {1.0f, 1.0f, 1.0f}}
		};
		const std::vector<std::uint16_t> indices
		{
			0, 1, 2,
			2, 3, 0
		};
		VkBuffer vertexBuffer{ VK_NULL_HANDLE };
		VkDeviceMemory vertexBufferMemory{ VK_NULL_HANDLE };
		VkBuffer indexBuffer{ VK_NULL_HANDLE };
		VkDeviceMemory indexBufferMemory{ VK_NULL_HANDLE };
		
		Core::Window window{ width, height, "Vulkan - Hello Triangle" };
		Core::Device device{ window };
		Core::SwapChain swapChain{ device, window.GetExtent() };
		std::unique_ptr<Core::Pipeline> pipeline;
		VkPipelineLayout pipelineLayout;
		std::vector<VkCommandBuffer> commandBuffers;

		VkDescriptorSetLayout descriptorSetLayout{ VK_NULL_HANDLE };
		VkDescriptorPool descriptorPool{ VK_NULL_HANDLE };
		std::vector<VkBuffer> uniformBuffers;
		std::vector<VkDeviceMemory> uniformBuffersMemory;
		std::vector<void*> uniformBuffersMapped;
		std::vector<VkDescriptorSet> descriptorSets;

		FirstApp(const FirstApp&) = delete;
		FirstApp& operator=(const FirstApp&) = delete;

	public:
		FirstApp();
		~FirstApp();

		void Run();
	};
}
