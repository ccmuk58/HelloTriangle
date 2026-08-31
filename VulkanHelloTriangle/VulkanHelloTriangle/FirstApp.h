#pragma once

#include "Window.h"
#include "Pipeline.h"
#include "Device.h"
#include "Swapchain.h"
#include "Vertex.h"

namespace App {
	class FirstApp
	{
	private:
		static constexpr int width{ 800 };
		static constexpr int height{ 600 };

		void CreateVertexBuffer();
		void CreatePipelineLayout();
		void CreatePipeline();
		void CreateCommandBuffers();
		void DrawFrame();


		const std::vector<Core::Vertex> vertices
		{
			{{0.0f, -0.5f}, {1.0f, 0.0f, 0.0f}},
			{{0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}},
			{{-0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}}
		};
		VkBuffer vertexBuffer{ VK_NULL_HANDLE };
		VkDeviceMemory vertexBufferMemory{ VK_NULL_HANDLE };
		Core::Window window{ width, height, "Vulkan - Hello Triangle" };
		Core::Device device{ window };
		Core::SwapChain swapChain{ device, window.GetExtent() };
		std::unique_ptr<Core::Pipeline> pipeline;
		VkPipelineLayout pipelineLayout;
		std::vector<VkCommandBuffer> commandBuffers;

		FirstApp(const FirstApp&) = delete;
		FirstApp& operator=(const FirstApp&) = delete;

	public:
		FirstApp();
		~FirstApp();

		void Run();
	};
}
