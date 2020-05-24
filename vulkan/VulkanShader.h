#pragma once

#include "stdafx.h"

class VulkanShader
{
public:
	VkShaderModule shaderModule;
	VkDevice device;

	VulkanShader(VkDevice device, const char* data, size_t size);
	VulkanShader(VkDevice device, const std::vector<char>& binary);

	~VulkanShader() { Free(); }

	// no copy
	VulkanShader(const VulkanShader& other) = delete;
	VulkanShader& operator = (const VulkanShader& other) = delete;

	// movable
	VulkanShader(VulkanShader&& other) noexcept { Steal(other); }
	VulkanShader& operator = (VulkanShader&& other) noexcept { Free(); Steal(other); }

private:
	void Init(const char* binaryData, size_t size);
	VulkanShader& Steal(VulkanShader& other);
	void Free();
};