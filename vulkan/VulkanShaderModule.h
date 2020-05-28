#pragma once

#include "stdafx.h"

class VulkanShaderModule
{
public:
	VkShaderModule shaderModule;
	VkDevice device;

	VulkanShaderModule(VkDevice device, const char* data, size_t size);
	VulkanShaderModule(VkDevice device, const std::vector<char>& binary);

	~VulkanShaderModule() { Free(); }

	// no copy
	VulkanShaderModule(const VulkanShaderModule& other) = delete;
	VulkanShaderModule& operator = (const VulkanShaderModule& other) = delete;

	// movable
	VulkanShaderModule(VulkanShaderModule&& other) noexcept { Steal(other); }
	VulkanShaderModule& operator = (VulkanShaderModule&& other) noexcept { Free(); Steal(other); }

private:
	void Init(const char* binaryData, size_t size);
	VulkanShaderModule& Steal(VulkanShaderModule& other);
	void Free();
};