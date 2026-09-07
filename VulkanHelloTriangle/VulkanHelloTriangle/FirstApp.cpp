#include "stdafx.h"
#include "FirstApp.h"
#
#include <stdexcept>
#include <chrono>
#include <glm/gtc/matrix_transform.hpp>

namespace App {
	FirstApp::FirstApp()
	{
		CreateVertexBuffer();
		CreateIndexBuffer();

		CreateDescriptorSetLayout();
		CreateUniformBuffers();
		CreateDescriptorPool();
		CreateDescriptorSets();

		CreatePipelineLayout();
		CreatePipeline();
		CreateCommandBuffers();
	}

	FirstApp::~FirstApp()
	{
		for (size_t i = 0; i < uniformBuffers.size(); ++i)
		{
			vkDestroyBuffer(device.GetDevice(), uniformBuffers[i], nullptr);
			vkFreeMemory(device.GetDevice(), uniformBuffersMemory[i], nullptr);
		}

		vkDestroyDescriptorPool(device.GetDevice(), descriptorPool, nullptr);

		pipeline.reset();
		vkDestroyPipelineLayout(device.GetDevice(), pipelineLayout, nullptr);
		vkDestroyDescriptorSetLayout(
			device.GetDevice(), descriptorSetLayout, nullptr);
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

	void FirstApp::CreateDescriptorSetLayout()
	{
		VkDescriptorSetLayoutBinding uboLayoutBinding{};
		uboLayoutBinding.binding = 0;
		uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		uboLayoutBinding.descriptorCount = 1;
		uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
		uboLayoutBinding.pImmutableSamplers = nullptr;

		VkDescriptorSetLayoutCreateInfo layoutInfo{};
		layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		layoutInfo.bindingCount = 1;
		layoutInfo.pBindings = &uboLayoutBinding;

		if (vkCreateDescriptorSetLayout(
			device.GetDevice(), &layoutInfo, nullptr, &descriptorSetLayout) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create descriptor set layout");
		}
	}

	void FirstApp::CreateUniformBuffers()
	{
		const VkDeviceSize bufferSize = sizeof(Core::UniformBufferObject);
		const size_t imageCount = swapChain.GetImageCount();

		uniformBuffers.resize(imageCount);
		uniformBuffersMemory.resize(imageCount);
		uniformBuffersMapped.resize(imageCount);

		for (size_t i = 0; i < imageCount; ++i)
		{
			device.CreateBuffer(
				bufferSize,
				VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
				VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
				uniformBuffers[i],
				uniformBuffersMemory[i]);

			if (vkMapMemory(
				device.GetDevice(),
				uniformBuffersMemory[i],
				0,
				bufferSize,
				0,
				&uniformBuffersMapped[i]) != VK_SUCCESS)
			{
				throw std::runtime_error("failed to map uniform buffer memory");
			}
		}
	}

	void FirstApp::CreateDescriptorPool()
	{
		VkDescriptorPoolSize poolSize{};
		poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		poolSize.descriptorCount =
			static_cast<uint32_t>(swapChain.GetImageCount());

		VkDescriptorPoolCreateInfo poolInfo{};
		poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		poolInfo.poolSizeCount = 1;
		poolInfo.pPoolSizes = &poolSize;
		poolInfo.maxSets = static_cast<uint32_t>(swapChain.GetImageCount());

		if (vkCreateDescriptorPool(
			device.GetDevice(), &poolInfo, nullptr, &descriptorPool) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create descriptor pool");
		}
	}

	void FirstApp::CreateDescriptorSets()
	{
		const size_t imageCount = swapChain.GetImageCount();

		std::vector<VkDescriptorSetLayout> layouts(
			imageCount, descriptorSetLayout);

		VkDescriptorSetAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo.descriptorPool = descriptorPool;
		allocInfo.descriptorSetCount = static_cast<uint32_t>(imageCount);
		allocInfo.pSetLayouts = layouts.data();

		descriptorSets.resize(imageCount);

		if (vkAllocateDescriptorSets(
			device.GetDevice(), &allocInfo, descriptorSets.data()) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to allocate descriptor sets");
		}

		for (size_t i = 0; i < imageCount; ++i)
		{
			VkDescriptorBufferInfo bufferInfo{};
			bufferInfo.buffer = uniformBuffers[i];
			bufferInfo.offset = 0;
			bufferInfo.range = sizeof(Core::UniformBufferObject);

			VkWriteDescriptorSet descriptorWrite{};
			descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrite.dstSet = descriptorSets[i];
			descriptorWrite.dstBinding = 0;
			descriptorWrite.dstArrayElement = 0;
			descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			descriptorWrite.descriptorCount = 1;
			descriptorWrite.pBufferInfo = &bufferInfo;

			vkUpdateDescriptorSets(
				device.GetDevice(), 1, &descriptorWrite, 0, nullptr);
		}
	}

	void FirstApp::UpdateUniformBuffer(uint32_t imageIndex)
	{
		static const auto startTime = std::chrono::high_resolution_clock::now();

		const auto currentTime = std::chrono::high_resolution_clock::now();
		const float time =
			std::chrono::duration<float>(
				currentTime - startTime).count();

		Core::UniformBufferObject ubo{};

		ubo.model = glm::rotate(
			glm::mat4{ 1.0f },
			time * glm::radians(90.0f),
			glm::vec3{ 0.0f, 0.0f, 1.0f });

		ubo.view = glm::lookAt(
			glm::vec3{ 2.0f, 2.0f, 2.0f },
			glm::vec3{ 0.0f, 0.0f, 0.0f },
			glm::vec3{ 0.0f, 0.0f, 1.0f });

		ubo.proj = glm::perspective(
			glm::radians(45.0f),
			swapChain.ExtentAspectRatio(),
			0.1f,
			10.0f);

		ubo.proj[1][1] *= -1.0f;

		std::memcpy(
			uniformBuffersMapped[imageIndex],
			&ubo,
			sizeof(ubo));
	}
	void FirstApp::CreatePipelineLayout()
	{
		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = 1;
		pipelineLayoutInfo.pSetLayouts = &descriptorSetLayout;
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

			vkCmdBindDescriptorSets(
				commandBuffers[i],
				VK_PIPELINE_BIND_POINT_GRAPHICS,
				pipelineLayout,
				0,
				1,
				&descriptorSets[i],
				0,
				nullptr);

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

		swapChain.WaitForImage(imageIndex);
		UpdateUniformBuffer(imageIndex);

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