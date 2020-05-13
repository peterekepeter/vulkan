// archeoptical.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include "Configuration.hpp"
#include "Playback.hpp"
#include "Vulkan.hpp"
#include "Window.hpp"
#include "VulkanApplicationBuilder.h"
#include "VulkanDebugUtilsMessenger.h"
#include "AppVulkanLogger.h"

static bool rebuildShaders()
{
	return system("build.bat") == 0;
}

static std::vector<char> readFile(const std::string& filename) {
    std::ifstream file(filename, std::ios::ate | std::ios::binary);

    if (!file.is_open()) {
        throw std::runtime_error("failed to open file!");
    }

	size_t fileSize = (size_t)file.tellg();
	std::vector<char> buffer(fileSize);
	file.seekg(0);
	file.read(buffer.data(), fileSize);
	file.close();

	return buffer;
}

static std::vector<char> readShader(const std::string& filename) {
	std::string newname = filename; // copy;
	newname = "spv\\" + filename + ".spv";
	return readFile(newname);
}

class VulkanShader
{
public:
	VkShaderModule shaderModule;
	VkDevice device;

	VulkanShader(VkDevice device, const std::vector<char>& code) : device(device) {
		VkShaderModuleCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		createInfo.codeSize = code.size();
		createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

		// actually create it
		if (vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
			throw std::runtime_error("failed to create shader module!");
		}
	}
	~VulkanShader() {
		vkDestroyShaderModule(device, shaderModule, nullptr);
	}
};


class RenderPass {
public:
	// owned
	VkRenderPass renderPass = VK_NULL_HANDLE;
	VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;
	VkDescriptorSetLayout descriptorSetLayout = VK_NULL_HANDLE;
	VkDevice device;

	RenderPass(VkDevice device, VkAttachmentDescription colorAttachment, VkSubpassDescription subpass) : device(device) {

		// desriptors
		VkDescriptorSetLayoutBinding uboLayoutBinding = {};
		uboLayoutBinding.binding = 0;
		uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		uboLayoutBinding.descriptorCount = 1;
		uboLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_VERTEX_BIT;

		VkDescriptorSetLayoutCreateInfo layoutInfo = {};
		layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		layoutInfo.bindingCount = 1;
		layoutInfo.pBindings = &uboLayoutBinding;

		if (vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &descriptorSetLayout) != VK_SUCCESS) {
			throw std::runtime_error("failed to create descriptor set layout!");
		}

		VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = 0; // Optional
		pipelineLayoutInfo.pSetLayouts = nullptr; // Optional
		pipelineLayoutInfo.pushConstantRangeCount = 0; // Optional
		pipelineLayoutInfo.pPushConstantRanges = nullptr; // Optional

		pipelineLayoutInfo.setLayoutCount = 1;
		pipelineLayoutInfo.pSetLayouts = &descriptorSetLayout;

		if (vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS) {
			throw std::runtime_error("failed to create pipeline layout!");
		}


		VkSubpassDependency dependency = {};
		dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
		dependency.dstSubpass = 0;

		dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.srcAccessMask = 0;

		dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

		VkRenderPassCreateInfo renderPassInfo = {};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassInfo.attachmentCount = 1;
		renderPassInfo.pAttachments = &colorAttachment;
		renderPassInfo.subpassCount = 1;
		renderPassInfo.pSubpasses = &subpass;

		renderPassInfo.dependencyCount = 1;
		renderPassInfo.pDependencies = &dependency;

		if (vkCreateRenderPass(device, &renderPassInfo, nullptr, &renderPass) != VK_SUCCESS) {
			throw std::runtime_error("failed to create render pass!");
		}


	}

	~RenderPass() {
		vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
		vkDestroyRenderPass(device, renderPass, nullptr);
		vkDestroyDescriptorSetLayout(device, descriptorSetLayout, nullptr);
	}
};

class Buffer
{
	VkDevice device;
	VkPhysicalDevice physicalDevice;
public: 
	VkBuffer buffer = VK_NULL_HANDLE;
	VkDeviceMemory bufferMemory = VK_NULL_HANDLE;

	Buffer() {

	}

	Buffer(VkDevice device, VkPhysicalDevice physical, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties) : device(device), physicalDevice(physical) {
		VkBufferCreateInfo bufferInfo = {};
		bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferInfo.size = size;
		bufferInfo.usage = usage;
		bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		if (vkCreateBuffer(device, &bufferInfo, nullptr, &buffer) != VK_SUCCESS) {
			throw std::runtime_error("failed to create buffer!");
		}

		VkMemoryRequirements memRequirements;
		vkGetBufferMemoryRequirements(device, buffer, &memRequirements);

		VkMemoryAllocateInfo allocInfo = {};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = memRequirements.size;
		allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties);

		if (vkAllocateMemory(device, &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS) {
			throw std::runtime_error("failed to allocate buffer memory!");
		}

		vkBindBufferMemory(device, buffer, bufferMemory, 0);
	}

	uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) {
		VkPhysicalDeviceMemoryProperties memProperties;
		vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);

		for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
			if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
				return i;
			}
		}

		throw std::runtime_error("failed to find suitable memory type!");
	}

	~Buffer()
	{
		if (buffer != VK_NULL_HANDLE) {
			vkDestroyBuffer(device, buffer, nullptr);
			buffer = VK_NULL_HANDLE;
		}
		if (bufferMemory != VK_NULL_HANDLE) {
			vkFreeMemory(device, bufferMemory, nullptr);
			bufferMemory = VK_NULL_HANDLE;
		}
	}

};


class SwapChainFramebuffers
{
public:
	std::vector<VkFramebuffer> swapChainFramebuffers;
	VkDevice device;

	SwapChainFramebuffers(VulkanSwapChain& swap, RenderPass& renderPass) : device(renderPass.device) {
		auto& swapChainImageViews = swap.swapChainImageViews;
		auto& swapChainExtent = swap.extent;
		swapChainFramebuffers.resize(swapChainImageViews.size());
		for (size_t i = 0; i < swapChainImageViews.size(); i++) {
			VkImageView attachments[] = {
				swapChainImageViews[i]
			};

			VkFramebufferCreateInfo framebufferInfo = {};
			framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			framebufferInfo.renderPass = renderPass.renderPass;
			framebufferInfo.attachmentCount = 1;
			framebufferInfo.pAttachments = attachments;
			framebufferInfo.width = swapChainExtent.width;
			framebufferInfo.height = swapChainExtent.height;
			framebufferInfo.layers = 1;

			if (vkCreateFramebuffer(device, &framebufferInfo, nullptr, &swapChainFramebuffers[i]) != VK_SUCCESS) {
				throw std::runtime_error("failed to create framebuffer!");
			}
		}
	}

	~SwapChainFramebuffers() {
		for (auto& framebuffer : swapChainFramebuffers)
		{
			vkDestroyFramebuffer(device, framebuffer, nullptr);
		}
	}
};

class DescriptorPool
{
	VkDevice device;
public: 
	VkDescriptorPool descriptorPool;

	DescriptorPool(VkDevice device, uint32_t count) : device(device) {
		VkDescriptorPoolSize poolSize = {};
		poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		poolSize.descriptorCount = count;

		VkDescriptorPoolCreateInfo poolInfo = {};
		poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		poolInfo.poolSizeCount = 1;
		poolInfo.pPoolSizes = &poolSize;
		poolInfo.maxSets = count;

		if (vkCreateDescriptorPool(device, &poolInfo, nullptr, &descriptorPool) != VK_SUCCESS) {
			throw std::runtime_error("failed to create descriptor pool!");
		}
	}

	~DescriptorPool() {
		vkDestroyDescriptorPool(device, descriptorPool, nullptr);
	}
};

class CommandPool
{
public:

	VkDevice device;
	VkCommandPool commandPool;

	CommandPool(VkDevice device, int queueFamilyIndex) : device(device){
		VkCommandPoolCreateInfo poolInfo = {};
		poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		poolInfo.queueFamilyIndex = queueFamilyIndex;
		poolInfo.flags = 0; // Optional
		// VK_COMMAND_POOL_CREATE_TRANSIENT_BIT // command buffers are rerecorded with new commands very often 
		// VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT // allow command buffers to be rerecorded individually,
		if (vkCreateCommandPool(device, &poolInfo, nullptr, &commandPool) != VK_SUCCESS) {
			throw std::runtime_error("failed to create command pool!");
		}
	}

	~CommandPool() {
		vkDestroyCommandPool(device, commandPool, nullptr);
	}
};

static void configure(ApplicationServices& app, int argc, char** argv)
{
	auto& deps = app.dependency;

	deps.For<Win32DefaultConsoleDriver>().UseDefaultConstructor();
	deps.For<IConsoleDriver>().UseType<Win32DefaultConsoleDriver>();
	app.console.UseDriver(app.dependency.GetInstance<IConsoleDriver>());
	deps.For<Console>().UseSharedInstance(&app.console);

	auto configuration = ConfigurationBuilder()
		.UseConfigurationFile("config.ini")
		.UseConsoleArgs(argc, argv)
		.Build(app);

	deps.For<Configuration>().UseInstanceTransferOwnership(configuration);
	auto& config = *configuration;

	deps.For<MusicPlaybackDevice>().UseConstructor<Configuration>();
	deps.For<ClockPlaybackDevice>().UseDefaultConstructor();
	deps.For<OfflinePlaybackDevice>().UseConstructor<Configuration>();

	if (config.offline) {
		deps.For<IPlaybackDevice>().UseType<OfflinePlaybackDevice>();
	}
	else if (config.musicEnabled) {
		deps.For<IPlaybackDevice>().UseType<MusicPlaybackDevice>();
	}
	else {
		deps.For<IPlaybackDevice>().UseType<ClockPlaybackDevice>();
	}

}

static void printProgress(Console& console, bool isPlaying, double positionInSeconds);
void runApplication(ApplicationServices& app);

int main(int argc, char** argv)
{
	ApplicationServices app;

	try
	{
		app.console.Open().Output << "Configuring application!\n";
		configure(app, argc, argv);
		app.console.Open().Output << "Starting application!\n";
		runApplication(app);
	}
	catch (const char* message) {
		app.console.Open().Error << "Fail: " << message << "\n";
	}
	catch (std::exception exception) {
		app.console.Open().Error << "Fail: " << exception.what() << "\n";
	}
}

struct UniformBufferObject
{
	float width;
	float height;
	float time;
	float _reserved;
};

void runApplication(ApplicationServices& app) {

	Configuration& config = *app.dependency.GetInstance<Configuration>();
	bool liveReload = config.liveReload;
	bool fullscreen = config.fullscreen;
	bool borderless = config.borderless;
	int xres = config.xres;
	int yres = config.yres;

	if (config.liveReload) {
		rebuildShaders();
	}

	ApplyEnvVarChanges();

	AppVulkanLogger logger{app};

	VulkanApplication vulkan = VulkanApplicationBuilder()
		.ApiVersion(VK_API_VERSION_1_0)
		.ApplicationInfo("N/A", VK_MAKE_VERSION(1, 0, 0))
		.EngineInfo("N/A", VK_MAKE_VERSION(1, 0, 0))
		.EnableValidationLayer(config.vulkanValidation)
		.UseLogger(logger)
		.EnableWindowSupport();

	// the actual window
	bool running = true; // true while window is open
	bool resize = true; // true if window was resized
	bool shouldPaint = true; // tells renderer to render
	int shouldPaintCount = 0;
	int width = 0, height = 0;

	InitWindowInfo windowInitInfo = {};

	windowInitInfo.fullscreen = fullscreen;
	windowInitInfo.borderless = borderless;
	windowInitInfo.xres = xres;
	windowInitInfo.yres = yres;
	windowInitInfo.title = config.windowTitle;

	windowInitInfo.onCloseWindow = [&]() {
		running = false;
		app.console.Open().Output << "Closed window.\n";
	};

	windowInitInfo.onWindowResize = [&](int x, int y) {
		resize = true;
		width = x; 
		height = y;
		app.console.ProgressPrinter("resize").Output 
			<< "New window size " << width << "x" << height << "  ";
	};

	IPlaybackDevice& playback = *app.dependency.GetInstance<IPlaybackDevice>();
	double currentPlaybackPosition = 0.0;

	bool didAutostartMusic = false;
	if (config.allowControls) {
		windowInitInfo.onKeystateChange = [&](int key, bool state) {
			if (state == false) return;
			double nextMusicPosition = currentPlaybackPosition;
			if (key == VK_SPACE) {
				if (playback.IsPlaying())
					playback.Pause();
				else
				{
					currentPlaybackPosition = 0;
					playback.Play();
				}
			}
			else if (key == VK_LEFT)
				nextMusicPosition -= 1.0;
			else if (key == VK_RIGHT)
				nextMusicPosition += 1.0;
			else if (key == VK_UP)
				nextMusicPosition -= 10.0;
			else if (key == VK_DOWN)
				nextMusicPosition += 10.0;

			if (nextMusicPosition < 0) 
			{ 
				nextMusicPosition = 0;
			}
			if (currentPlaybackPosition != nextMusicPosition) 
			{ 
				playback.SetPosition(nextMusicPosition);
				shouldPaint = true;
			}
		};
	}

	windowInitInfo.onWindowPaint = [&]() {
		shouldPaint = true;
	};

	app.console.Open().Output << "Creating window.\n";
	auto hwnd = InitWindow(windowInitInfo);

	// create window for vulkan
	app.console.Open().Output << "Creating surface.\n";
	VulkanWindow vulkanWindow(vulkan.instance, hwnd);

	// devices
	VulkanPhysicalDeviceEnumeration physicalDevices(vulkan.instance, vulkanWindow.surface);
	if (physicalDevices.anyDevice == false) throw std::runtime_error("No device was found with Vulkan support. Check drivers!");
	if (physicalDevices.anySuitable == false) throw std::runtime_error("No device was found with all required features!");
	auto& physical = *physicalDevices.top;

	// print info about physical device
	app.console.Open().Output << "Using " << physical.properties.deviceName << " for rendering purposes...\n";

	// directory change
	auto watchPath = "./";
	DirectoryChangeService* directory = nullptr;
	if (liveReload) {
		directory = new DirectoryChangeService(watchPath);
		app.console.Open().Output << "Watching " << watchPath << " for changes\n";
	}

	// logical vulkan device
	app.console.Open().Output << "Initializing Vulkan Device.\n";
	VulkanDevice device(vulkan, physical);

	app.console.Open().Output << "Graphics queue index "  << physical.graphicsFamilyIndex << "\n";
	app.console.Open().Output << "Present queue index "  << physical.presentFamilyIndex << "\n";

	while (running) {


		app.console.Open().Output << "Reading shader binaries.\n";
		VulkanShader vertShader = VulkanShader(device.device, readShader("vert"));
		VulkanShader fragShader = VulkanShader(device.device, readShader("frag"));

		app.console.Open().Output << "Creating swap chain.\n";
		physical.resetSwapChain();
		VulkanSwapChain swap(physical, device, width, height);

		resize = false;

		VkPipelineShaderStageCreateInfo vertShaderStageInfo = {};
		vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
		vertShaderStageInfo.module = vertShader.shaderModule;
		vertShaderStageInfo.pName = "main";
		vertShaderStageInfo.pSpecializationInfo = nullptr; // useful for constants

		VkPipelineShaderStageCreateInfo fragShaderStageInfo = {};
		fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
		fragShaderStageInfo.module = fragShader.shaderModule;
		fragShaderStageInfo.pName = "main";

		VkPipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo, fragShaderStageInfo };

		// vertex input
		VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
		vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		vertexInputInfo.vertexBindingDescriptionCount = 0;
		vertexInputInfo.pVertexBindingDescriptions = nullptr; // Optional
		vertexInputInfo.vertexAttributeDescriptionCount = 0;
		vertexInputInfo.pVertexAttributeDescriptions = nullptr; // Optional

		// vertex assembly
		VkPipelineInputAssemblyStateCreateInfo inputAssembly = {};
		inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		inputAssembly.primitiveRestartEnable = VK_FALSE;

		// viretport
		VkViewport viewport = {};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = (float)swap.extent.width; // match buffer sizes
		viewport.height = (float)swap.extent.height;
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;

		// scrissor 
		VkRect2D scissor = {};
		scissor.offset = { 0, 0 };
		scissor.extent = swap.extent; // please draw on whole screen

									  // setup the viewport
		VkPipelineViewportStateCreateInfo viewportState = {};
		viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		viewportState.viewportCount = 1;
		viewportState.pViewports = &viewport;
		viewportState.scissorCount = 1;
		viewportState.pScissors = &scissor;

		// setup the rasterizer
		VkPipelineRasterizationStateCreateInfo rasterizer = {};
		rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		rasterizer.depthClampEnable = VK_FALSE; // can be useful for shadow maps
		rasterizer.rasterizerDiscardEnable = VK_FALSE; // setting to true will make no output
													   // mode (other modes need extensions)
		rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
		rasterizer.lineWidth = 1.0f;
		// culling mode
		rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
		rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
		// depth bias
		rasterizer.depthBiasEnable = VK_FALSE;
		rasterizer.depthBiasConstantFactor = 0.0f; // Optional
		rasterizer.depthBiasClamp = 0.0f; // Optional
		rasterizer.depthBiasSlopeFactor = 0.0f; // Optional

												// multisample state
		VkPipelineMultisampleStateCreateInfo multisampling = {};
		multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		multisampling.sampleShadingEnable = VK_FALSE;
		multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
		multisampling.minSampleShading = 1.0f; // Optional
		multisampling.pSampleMask = nullptr; // Optional
		multisampling.alphaToCoverageEnable = VK_FALSE; // Optional
		multisampling.alphaToOneEnable = VK_FALSE; // Optional

		// color blend attachement
		VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
		colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		colorBlendAttachment.blendEnable = VK_TRUE;
		colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_DST_COLOR; // Optional
		colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA; // Optional
		colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD; // Optional
		colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
		colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
		colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD; // Optional
		// alpha blend colorBlendAttachment.blendEnable = VK_TRUE;
		// alpha blend colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
		// alpha blend colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
		// alpha blend colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
		// alpha blend colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
		// alpha blend colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
		// alpha blend colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

															 // color blend state
		VkPipelineColorBlendStateCreateInfo colorBlending = {};
		colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		colorBlending.logicOpEnable = VK_FALSE;
		colorBlending.logicOp = VK_LOGIC_OP_COPY; // Optional
		colorBlending.attachmentCount = 1;
		colorBlending.pAttachments = &colorBlendAttachment;
		colorBlending.blendConstants[0] = 0.0f; // Optional
		colorBlending.blendConstants[1] = 0.0f; // Optional
		colorBlending.blendConstants[2] = 0.0f; // Optional
		colorBlending.blendConstants[3] = 0.0f; // Optional

												// dynamic state
		VkDynamicState dynamicStates[] = {
			VK_DYNAMIC_STATE_VIEWPORT,
			VK_DYNAMIC_STATE_LINE_WIDTH
		};
		VkPipelineDynamicStateCreateInfo dynamicState = {};
		dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		dynamicState.dynamicStateCount = 2;
		dynamicState.pDynamicStates = dynamicStates;

		// render pass begins
		VkAttachmentDescription colorAttachment = {};
		colorAttachment.format = swap.surfaceFormat.format;
		colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
		colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

		VkAttachmentReference colorAttachmentRef = {};
		colorAttachmentRef.attachment = 0;
		colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkSubpassDescription subpass = {};
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.colorAttachmentCount = 1;
		subpass.pColorAttachments = &colorAttachmentRef;

		RenderPass renderPass(device.device, colorAttachment, subpass);

		VkGraphicsPipelineCreateInfo pipelineInfo = {};
		pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		pipelineInfo.stageCount = 2;
		pipelineInfo.pStages = shaderStages;

		pipelineInfo.pVertexInputState = &vertexInputInfo;
		pipelineInfo.pInputAssemblyState = &inputAssembly;
		pipelineInfo.pViewportState = &viewportState;
		pipelineInfo.pRasterizationState = &rasterizer;
		pipelineInfo.pMultisampleState = &multisampling;
		pipelineInfo.pDepthStencilState = nullptr; // Optional
		pipelineInfo.pColorBlendState = &colorBlending;
		pipelineInfo.pDynamicState = nullptr; // Optional

		pipelineInfo.layout = renderPass.pipelineLayout;
		pipelineInfo.renderPass = renderPass.renderPass;
		pipelineInfo.subpass = 0;

		pipelineInfo.basePipelineHandle = VK_NULL_HANDLE; // Optional
		pipelineInfo.basePipelineIndex = -1; // Optional


		VkPipeline graphicsPipeline;
		if (vkCreateGraphicsPipelines(device.device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &graphicsPipeline) != VK_SUCCESS) {
			throw std::runtime_error("failed to create graphics pipeline!");
		}

		SwapChainFramebuffers framebuffers(swap, renderPass);

		CommandPool graphicsCommandPool(device.device, physical.graphicsFamilyIndex);
		DescriptorPool descriptorPool(device.device, swap.swapChainImages.size());



		// uniform buffers
		std::vector<Buffer> uniformBuffers;

		VkDeviceSize bufferSize = sizeof(UniformBufferObject);
		uniformBuffers.reserve(swap.swapChainImages.size());

		for (size_t i = 0; i < swap.swapChainImages.size(); i++) {
			uniformBuffers.emplace_back(device.device, physical.physicalDevice, bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
		}

		// command buffers
		std::vector<VkCommandBuffer> commandBuffers;
		commandBuffers.resize(swap.swapChainImageViews.size());
		VkCommandBufferAllocateInfo allocInfo = {};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.commandPool = graphicsCommandPool.commandPool;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandBufferCount = (uint32_t)commandBuffers.size();

		if (vkAllocateCommandBuffers(device.device, &allocInfo, commandBuffers.data()) != VK_SUCCESS) {
			throw std::runtime_error("failed to allocate command buffers!");
		}

		std::vector<VkDescriptorSet> descriptorSets;
		{
			std::vector<VkDescriptorSetLayout> layouts(swap.swapChainImages.size(), renderPass.descriptorSetLayout);
			VkDescriptorSetAllocateInfo allocInfo = {};
			allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
			allocInfo.descriptorPool = descriptorPool.descriptorPool;
			allocInfo.descriptorSetCount = static_cast<uint32_t>(swap.swapChainImages.size());
			allocInfo.pSetLayouts = layouts.data();

			descriptorSets.resize(swap.swapChainImages.size());
			if (vkAllocateDescriptorSets(device.device, &allocInfo, &descriptorSets[0]) != VK_SUCCESS) {
				throw std::runtime_error("failed to allocate descriptor sets!");
			}

			for (size_t i = 0; i < swap.swapChainImages.size(); i++) {
				VkDescriptorBufferInfo bufferInfo = {};
				bufferInfo.buffer = uniformBuffers[i].buffer;
				bufferInfo.offset = 0;
				bufferInfo.range = sizeof(UniformBufferObject);
				VkWriteDescriptorSet descriptorWrite = {};
				descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				descriptorWrite.dstSet = descriptorSets[i];
				descriptorWrite.dstBinding = 0;
				descriptorWrite.dstArrayElement = 0;
				descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
				descriptorWrite.descriptorCount = 1;
				descriptorWrite.pBufferInfo = &bufferInfo;
				descriptorWrite.pImageInfo = nullptr; // Optional
				descriptorWrite.pTexelBufferView = nullptr; // Optional
				vkUpdateDescriptorSets(device.device, 1, &descriptorWrite, 0, nullptr);
			}
		}

		// recording 
		for (size_t i = 0; i < commandBuffers.size(); i++) {
			VkCommandBufferBeginInfo beginInfo = {};
			beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
			beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
			beginInfo.pInheritanceInfo = nullptr; // Optional

			if (vkBeginCommandBuffer(commandBuffers[i], &beginInfo) != VK_SUCCESS) {
				throw std::runtime_error("failed to begin recording command buffer!");
			}

			VkRenderPassBeginInfo renderPassInfo = {};
			renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
			renderPassInfo.renderPass = renderPass.renderPass;
			renderPassInfo.framebuffer = framebuffers.swapChainFramebuffers[i];
			renderPassInfo.renderArea.offset = { 0, 0 };
			renderPassInfo.renderArea.extent = swap.extent;

			VkClearValue clearColor = { 0.0f, 0.0f, 0.0f, 1.0f };
			renderPassInfo.clearValueCount = 1;
			renderPassInfo.pClearValues = &clearColor;

			vkCmdBeginRenderPass(commandBuffers[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
			vkCmdBindPipeline(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);
			vkCmdBindDescriptorSets(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, renderPass.pipelineLayout, 0, 1, &descriptorSets[i], 0, nullptr);
			vkCmdDraw(commandBuffers[i], 6000000, 1, 0, 0);
			vkCmdEndRenderPass(commandBuffers[i]);

			if (vkEndCommandBuffer(commandBuffers[i]) != VK_SUCCESS) {
				throw std::runtime_error("failed to record command buffer!");
			}
		}

		// actual rendering

		VkSemaphore imageAvailableSemaphore;
		VkSemaphore renderFinishedSemaphore;

		{
			VkSemaphoreCreateInfo semaphoreInfo = {};
			semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

			if (vkCreateSemaphore(device.device, &semaphoreInfo, nullptr, &imageAvailableSemaphore) != VK_SUCCESS ||
				vkCreateSemaphore(device.device, &semaphoreInfo, nullptr, &renderFinishedSemaphore) != VK_SUCCESS) {

				throw std::runtime_error("failed to create semaphores!");
			}
		}

		UniformBufferObject ubo;


		// process win32 message loop
		while (running) {
			ProcessWindowMessagesNonBlocking();

			bool rebuild = false;
			if (directory != nullptr) {
				while (directory->HasChange())
				{
					shouldPaint = true;
					auto diff = directory->ReadChange();
					if (diff.filename == "shader.vert" || diff.filename == "shader.frag")
					{
						rebuild = true;
					}
				}
			}

			if (rebuild && liveReload) {
				if (rebuildShaders()) {
					resize = true;
				}
			}

			if (shouldPaint || playback.IsPlaying()) {
				shouldPaintCount = swap.swapChainImages.size();
				shouldPaint = false;
			}

			if (shouldPaintCount <= 0)
			{
				Sleep(10); // save battery
				continue; // skip painting
			}
			
			shouldPaintCount--; // paint this frame

			uint32_t imageIndex;
			auto result = vkAcquireNextImageKHR(device.device, swap.swapChain, std::numeric_limits<uint64_t>::max(), imageAvailableSemaphore, VK_NULL_HANDLE, &imageIndex);

			if (result == VK_ERROR_OUT_OF_DATE_KHR || resize == true) {
				break; // break inner while to force re-creation of swap chain
			}
			else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
				throw std::runtime_error("failed to acquire swap chain image!");
			}
			
			// handle music
			if (!didAutostartMusic){
				playback.Play();
				didAutostartMusic = true;
			}

			playback.NextFrame();
			currentPlaybackPosition = playback.GetPosition();
			auto seconds = currentPlaybackPosition;

			printProgress(app.console, playback.IsPlaying(), seconds);

			ubo.width = static_cast<float>(width);
			ubo.height = static_cast<float>(height);
			ubo.time = static_cast<float>(seconds);
			ubo._reserved = 0.0f;

			void* data;
			vkMapMemory(device.device, uniformBuffers[imageIndex].bufferMemory, 0, sizeof(ubo), 0, &data);
			memcpy(data, &ubo, sizeof(ubo));
			vkUnmapMemory(device.device, uniformBuffers[imageIndex].bufferMemory);

			VkSubmitInfo submitInfo = {};
			submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

			VkSemaphore imageAvailable[] = { imageAvailableSemaphore };
			VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
			submitInfo.waitSemaphoreCount = 1;
			submitInfo.pWaitSemaphores = imageAvailable;
			submitInfo.pWaitDstStageMask = waitStages;

			submitInfo.commandBufferCount = 1;
			submitInfo.pCommandBuffers = &commandBuffers[imageIndex];

			VkSemaphore renderFinish[] = { renderFinishedSemaphore };
			submitInfo.signalSemaphoreCount = 1;
			submitInfo.pSignalSemaphores = renderFinish;

			if (vkQueueSubmit(device.graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE) != VK_SUCCESS) {
				throw std::runtime_error("failed to submit draw command buffer!");
			}

			VkPresentInfoKHR presentInfo = {};
			presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

			presentInfo.waitSemaphoreCount = 1;
			presentInfo.pWaitSemaphores = renderFinish;

			VkSwapchainKHR swapChains[] = { swap.swapChain };
			presentInfo.swapchainCount = 1;
			presentInfo.pSwapchains = swapChains;
			presentInfo.pImageIndices = &imageIndex;
			presentInfo.pResults = nullptr; // Optional

			vkQueuePresentKHR(device.presentQueue, &presentInfo);
		}

		vkDeviceWaitIdle(device.device);

		vkDestroySemaphore(device.device, renderFinishedSemaphore, nullptr);
		vkDestroySemaphore(device.device, imageAvailableSemaphore, nullptr);

		vkDestroyPipeline(device.device, graphicsPipeline, nullptr);
	}

}

static void printProgress(Console& console, bool isPlaying, double positionInSeconds) {
	auto con = console.ProgressPrinter("playback progress");
	auto& out = con.Output;

	// print status
	out << (isPlaying ? "PLAYING " : "PAUSED ");

	// split position into components
	int seconds = static_cast<int>(positionInSeconds);
	int fract = static_cast<int>((positionInSeconds - seconds)*1000);
	int minutes = seconds / 60;
	seconds -= minutes * 60;
	int hours = minutes / 60;
	minutes -= hours * 60;

	// print
	using namespace std;
	out << setw(2) << setfill('0') << hours
		<< ":" << setw(2) << setfill('0') << minutes
		<< ":" << setw(2) << setfill('0') << seconds
		<< "." << setw(3) << setfill('0') << fract
		<< "  ";
}
