#pragma once
#include "VulkanSwapChain.h"

class SwapChainFramebuffers
{
	DECLARE_MOVEABLE_TYPE(SwapChainFramebuffers)

public:
	std::vector<VkFramebuffer> swapChainFramebuffers;
	VkDevice device;

	SwapChainFramebuffers(VulkanSwapChain& swap, VkDevice device, VkRenderPass render_pass);
};