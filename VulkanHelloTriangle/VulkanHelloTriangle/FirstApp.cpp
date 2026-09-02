#include "stdafx.h"
#include "FirstApp.h"

#include <stdexcept>

namespace App {
	FirstApp::FirstApp()
	{
		CreateVertexBuffer();
		CreateIndexBuffer();

		CreatePipelineLayout();
		CreatePipeline();

		CreateCommandBuffers();
	}

	FirstApp::~FirstApp()
	{

		vkDestroyBuffer(device.GetDevice(), indexBuffer, nullptr);
		vkFreeMemory(device.GetDevice(), indexBufferMemory, nullptr);

		vkDestroyBuffer(device.GetDevice(), vertexBuffer, nullptr);
		vkFreeMemory(device.GetDevice(), vertexBufferMemory, nullptr);
		
		pipeline.reset();
		vkDestroyPipelineLayout(device.GetDevice(), pipelineLayout, nullptr);
	}

	void FirstApp::CreateIndexBuffer()
	{
		const VkDeviceSize bufferSize =
			sizeof(indices[0]) * indices.size();

		VkBuffer stagingBuffer{ VK_NULL_HANDLE };
		VkDeviceMemory stagingBufferMemory{ VK_NULL_HANDLE };

		// temporary staging buffer
		device.CreateBuffer(
			bufferSize,
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
			VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			stagingBuffer,
			stagingBufferMemory);

		void* mappedData{ nullptr };

		if (vkMapMemory(device.GetDevice(),stagingBufferMemory,0,bufferSize,0,&mappedData) != VK_SUCCESS)
		{
			throw std::runtime_error{
				"failed to map index staging buffer memory"
			};
		}

		std::memcpy(mappedData, indices.data(), static_cast<size_t>(bufferSize));

		vkUnmapMemory(device.GetDevice(),stagingBufferMemory);

		device.CreateBuffer(
			bufferSize,
			VK_BUFFER_USAGE_TRANSFER_DST_BIT |
			VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			indexBuffer,
			indexBufferMemory);

		device.CopyBuffer(stagingBuffer, indexBuffer, bufferSize);
		vkDestroyBuffer(device.GetDevice(), stagingBuffer, nullptr);
		vkFreeMemory(device.GetDevice(), stagingBufferMemory, nullptr);
	}


	void FirstApp::CreateVertexBuffer()
	{
		VkDeviceSize bufferSize = sizeof(vertices[0]) * vertices.size();

		// temporary staging buffer
		VkBuffer stagingBuffer{ VK_NULL_HANDLE };
		VkDeviceMemory stagingBufferMemory{ VK_NULL_HANDLE };

		device.CreateBuffer(
			bufferSize,
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			stagingBuffer,
			stagingBufferMemory);

		// map staging buffer memory
		void* mappedData{ nullptr };
		if (vkMapMemory(device.GetDevice(), stagingBufferMemory, 0, bufferSize, 0, &mappedData) != VK_SUCCESS)
		{
			throw std::runtime_error{
				"failed to map staging buffer memory"
			};
		}

		// copy vertex data to staging buffer
		std::memcpy(mappedData, vertices.data(), static_cast<size_t>(bufferSize));
		
		vkUnmapMemory(device.GetDevice(), stagingBufferMemory);

		// create vertex buffer
		device.CreateBuffer(
			bufferSize,
			VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			vertexBuffer,
			vertexBufferMemory);

		// copy data from staging buffer to vertex buffer
		device.CopyBuffer(stagingBuffer, vertexBuffer, bufferSize);

		// destroy staging buffer
		vkDestroyBuffer(device.GetDevice(), stagingBuffer, nullptr);
		vkFreeMemory(device.GetDevice(), stagingBufferMemory, nullptr);
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
			
			vkCmdBindIndexBuffer(commandBuffers[i], indexBuffer, 0, VK_INDEX_TYPE_UINT16);

			vkCmdDrawIndexed(commandBuffers[i], static_cast<uint32_t>(indices.size()), 1, 0, 0, 0);

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