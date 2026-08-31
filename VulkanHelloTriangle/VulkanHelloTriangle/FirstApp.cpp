#include "stdafx.h"
#include "FirstApp.h"

#include <stdexcept>

namespace App {
	FirstApp::FirstApp()
	{
		CreateVertexBuffer();
		CreatePipelineLayout();
		CreatePipeline();
		CreateCommandBuffers();
	}

	FirstApp::~FirstApp()
	{
		vkDestroyBuffer(device.GetDevice(), vertexBuffer, nullptr);
		vkFreeMemory(device.GetDevice(), vertexBufferMemory, nullptr);
		pipeline.reset();
		vkDestroyPipelineLayout(device.GetDevice(), pipelineLayout, nullptr);
	}

	void FirstApp::CreateVertexBuffer()
	{
		VkDeviceSize bufferSize = sizeof(vertices[0]) * vertices.size();

		device.CreateBuffer(
			bufferSize,
			VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
			VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			vertexBuffer,
			vertexBufferMemory);

		void* mappedData{ nullptr };

		if (vkMapMemory(device.GetDevice(), vertexBufferMemory, 0, bufferSize, 0, &mappedData) != VK_SUCCESS)
		{
			throw std::runtime_error{
				"failed to map vertex buffer memory"
			};
		}

		std::memcpy(mappedData, vertices.data(), static_cast<size_t>(bufferSize));

		vkUnmapMemory(device.GetDevice(), vertexBufferMemory);
	}

	void FirstApp::CreatePipelineLayout()
	{
		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = 0;
		pipelineLayoutInfo.pSetLayouts = nullptr;
		pipelineLayoutInfo.pushConstantRangeCount = 0;
		pipelineLayoutInfo.pPushConstantRanges = nullptr;
		if (vkCreatePipelineLayout(device.GetDevice(), &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS)
		{
			throw std::runtime_error{ "failed to create graphics pipeline" };
		}
	}

	void FirstApp::CreatePipeline()
	{
		auto pipelineConfig{ Core::Pipeline::DefaultPipelineConfigInfo(swapChain.GetWidth(), swapChain.GetHeight()) };
		pipelineConfig.renderPass = swapChain.GetRenderPass();
		pipelineConfig.pipelineLayout = pipelineLayout;
		pipeline = std::make_unique<Core::Pipeline>(device, "simple_vs.vert.spv", "simple_fs.frag.spv", pipelineConfig);
	}

	void FirstApp::CreateCommandBuffers()
	{
		commandBuffers.resize(swapChain.GetImageCount());

		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.commandPool = device.GetCommandPool();
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandBufferCount = static_cast<uint32_t>(commandBuffers.size());

		if (vkAllocateCommandBuffers(
			device.GetDevice(), &allocInfo, commandBuffers.data()) != VK_SUCCESS)
		{
			throw std::runtime_error{ "failed to allocate command buffers" };
		}

		for (size_t i = 0; i < commandBuffers.size(); i++)
		{
			VkCommandBufferBeginInfo beginInfo{};
			beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

			if (vkBeginCommandBuffer(commandBuffers[i], &beginInfo) != VK_SUCCESS)
			{
				throw std::runtime_error{ "failed to begin recording command buffer" };
			}

			VkRenderPassBeginInfo renderPassInfo{};
			renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
			renderPassInfo.renderPass = swapChain.GetRenderPass();
			renderPassInfo.framebuffer = swapChain.GetFrameBuffer(static_cast<int>(i));
			renderPassInfo.renderArea.offset = { 0, 0 };
			renderPassInfo.renderArea.extent = swapChain.GetSwapChainExtent();

			std::array<VkClearValue, 2> clearValues{};
			clearValues[0].color = { { 0.05f, 0.05f, 0.05f, 1.0f } };
			clearValues[1].depthStencil = { 1.0f, 0 };
			renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
			renderPassInfo.pClearValues = clearValues.data();

			vkCmdBeginRenderPass(
				commandBuffers[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

			vkCmdBindPipeline(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->GetGraphicsPipeline());

			VkBuffer vertexBuffers[] = { vertexBuffer };
			VkDeviceSize offsets[] = { 0 };
			vkCmdBindVertexBuffers(commandBuffers[i], 0, 1, vertexBuffers, offsets);

			vkCmdDraw(commandBuffers[i], static_cast<uint32_t>(vertices.size()), 1, 0, 0);

			vkCmdEndRenderPass(commandBuffers[i]);

			if (vkEndCommandBuffer(commandBuffers[i]) != VK_SUCCESS)
			{
				throw std::runtime_error{ "failed to record command buffer" };
			}
		}
	}

	void FirstApp::DrawFrame()
	{
		uint32_t imageIndex{};
		VkResult result = swapChain.AcquireNextImage(&imageIndex);

		if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
		{
			throw std::runtime_error{ "failed to acquire swap chain image" };
		}

		result = swapChain.SubmitCommandBuffers(&commandBuffers[imageIndex], &imageIndex);
		if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
		{
			throw std::runtime_error{ "failed to present swap chain image" };
		}
	}

	void FirstApp::Run()
	{
		while (!window.IsClosed())
		{
			glfwPollEvents();
			DrawFrame();
		}

		vkDeviceWaitIdle(device.GetDevice());
	}
}