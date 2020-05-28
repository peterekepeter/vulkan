#include "stdafx.h"
#include "VulkanShaderModule.h"

VulkanShaderModule::VulkanShaderModule(VkDevice device, const char* data, size_t size) : device(device) {
	Init(data, size);
}

VulkanShaderModule::VulkanShaderModule(VkDevice device, const std::vector<char>& binary) : device(device) {
	Init(binary.data(), binary.size());
}

void VulkanShaderModule::Init(const char* binaryData, size_t size) {
	VkShaderModuleCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createInfo.codeSize = size;
	createInfo.pCode = reinterpret_cast<const uint32_t*>(binaryData);

	// actually create it
	if (vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
		throw std::runtime_error("failed to create shader module!");
	}
}

VulkanShaderModule& VulkanShaderModule::Steal(VulkanShaderModule& other) {
	this->shaderModule = other.shaderModule;
	this->device = other.device;
	other.shaderModule = VK_NULL_HANDLE;
	other.device = VK_NULL_HANDLE;
	return *this;
}

void VulkanShaderModule::Free() {
	if (shaderModule != VK_NULL_HANDLE) {
		vkDestroyShaderModule(device, shaderModule, nullptr);
		shaderModule = VK_NULL_HANDLE;
	}
}
