#pragma once

#include "Device.h"

namespace Core {

	struct PipelineConfigInfo {
	};

	class Pipeline {
	private:
		Device& device;
		VkPipeline graphicsPipeline;
		VkShaderModule vertShaderModule;
		VkShaderModule fragShaderModule;

		static std::vector<char> ReadFile(const std::string_view& filePath);
		void CreateGraphicsPipeline(Device& device,
			const std::string_view& vertFilePath,
			const std::string_view& fragFilePath,
			const PipelineConfigInfo& configInfo);

		void CreateShaderModule(const std::vector<char*>& code, VkShaderModule* shaderModule);

	public:
		Pipeline(Device& device,
			const std::string_view& vertFilePath,
			const std::string_view& fragFilePath,
			const PipelineConfigInfo& configInfo);

		~Pipeline() {}

		Pipeline(const Pipeline&) = delete;
		Pipeline(Pipeline&&) = delete;

		static PipelineConfigInfo DefaultPipelineConfigInfo(uint32_t width, uint32_t height);
	};
}