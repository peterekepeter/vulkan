#include "pch.h"
#include "SwapChainFramebuffers.h"

SwapChainFramebuffers::SwapChainFramebuffers(VulkanSwapChain& swap, VkDevice device, VkRenderPass render_pass) : device(device) {
	auto& swapChainImageViews = swap.swapChainImageViews;
	auto& swapChainExtent = swap.extent;
	swapChainFramebuffers.resize(swapChainImageViews.size());
	for (size_t i = 0; i < swapChainImageViews.size(); i++) {
		VkImageView attachments[] = {
			swapChainImageViews[i]
		};

		VkFramebufferCreateInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		info.renderPass = render_pass;
		info.attachmentCount = 1;
		info.pAttachments = attachments;
		info.width = swapChainExtent.width;
		info.height = swapChainExtent.height;
		info.layers = 1;

		
		auto result = vkCreateFramebuffer(device, &info, nullptr, &swapChainFramebuffers[i]);
		ensure(result == VK_SUCCESS, "framebuffer was created");
	}
}

void SwapChainFramebuffers::free_members() {
	for (auto& framebuffer : swapChainFramebuffers)
	{
		vkDestroyFramebuffer(device, framebuffer, nullptr);
	}
}

void SwapChainFramebuffers::move_members(SwapChainFramebuffers&& other) {
	device = other.device;
	swapChainFramebuffers = std::move(other.swapChainFramebuffers);
}