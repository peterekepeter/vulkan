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
#include "VulkanShaderModule.h"
#include "VulkanSwapChain.h"
#include "VulkanGraphicsPipeline.h"
#include "VulkanImageMemoryAllocator.h"
#include "VulkanImage.h"
#include "VulkanImageView.h"
#include "VulkanRenderPass.h"
#include "VulkanDescriptorSetLayout.h"
#include "VulkanPipelineLayout.h"
#include "VulkanObjectBuilder.h"
#include "BuiltinShaders.h"

// TODO: refactor move to library
static void make_sure_dir_exists(const std::string dirPath)
{
	if (CreateDirectoryA(dirPath.c_str(), NULL) ||
		ERROR_ALREADY_EXISTS == GetLastError())
	{
		return; /// ok
	}
	else
	{
		throw std::runtime_error("failed to create dir");
	}
}

static bool rebuild_shaders(Console& console, Configuration& config)
{
	if (config.vulkan_sdk == nullptr) {
		throw std::runtime_error("rebuilding shaders requires Vulkan SDK!\n" 
			"Download from: https://vulkan.lunarg.com/sdk/home#windows");
	}
	console.Open().Output << "Rebuilding shaders\n";
	bool success = true;
	std::string toolsPath = (*config.vulkan_sdk) + "/Bin";
	std::string compiler = toolsPath + "/glslangValidator.exe";
	std::string optimizer = toolsPath + "/spirv-opt.exe";
	make_sure_dir_exists("spv");
	auto result1 = ProcessCommand::Execute(compiler 
		+ " -V shader.frag -o spv/frag.spv");
	auto result2 = ProcessCommand::Execute(compiler 
		+ " -V shader.vert -o spv/vert.spv");
	if (result1.exitCode == 0) {
		auto result3 = ProcessCommand::Execute(optimizer
			+ " -O spv/frag.spv -o spv/frag.spv");
	}
	else {
		console.Open().Error << result1.output;
		success = false;
	}
	if (result2.exitCode == 0) {
		auto result4 = ProcessCommand::Execute(optimizer
			+ " -O spv/vert.spv -o spv/vert.spv");
	}
	else {
		console.Open().Error << result2.output;
		success = false;
	}
	return success;
}

static std::vector<char> read_file(const std::string& filename) {
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

static std::vector<char> read_shader(const std::string& filename) {
	std::string newname = filename; // copy;
	newname = "spv\\" + filename + ".spv";
	return read_file(newname);
}


class Buffer
{
	VkDevice device;
	VkPhysicalDevice physicalDevice;
public: 
	VkBuffer buffer = VK_NULL_HANDLE;
	VkDeviceMemory bufferMemory = VK_NULL_HANDLE;

	Buffer() 
		: device(nullptr)
		, physicalDevice(nullptr) {}

	Buffer(
		VkDevice device, 
		VkPhysicalDevice physical, 
		VkDeviceSize size, 
		VkBufferUsageFlags usage, 
		VkMemoryPropertyFlags properties) 
		: device(device)
		, physicalDevice(physical) 
	{
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

	SwapChainFramebuffers(VulkanSwapChain& swap, VkDevice device, VkRenderPass render_pass) : device(device) {
		auto& swapChainImageViews = swap.swapChainImageViews;
		auto& swapChainExtent = swap.extent;
		swapChainFramebuffers.resize(swapChainImageViews.size());
		for (size_t i = 0; i < swapChainImageViews.size(); i++) {
			VkImageView attachments[] = {
				swapChainImageViews[i]
			};

			VkFramebufferCreateInfo framebufferInfo = {};
			framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			framebufferInfo.renderPass = render_pass;
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

static void configure(ApplicationServices& app, int argc, char** argv, char** env)
{
	auto& di = app.dependency;

	di.For<IConsoleDriver>().UseType<Win32DefaultConsoleDriver>();
	app.console.UseDriver(app.dependency.GetInstance<IConsoleDriver>());
	di.For<Console>().UseSharedInstance(&app.console);

	di.For<Configuration>().UseFactory([&] {	
		return ConfigurationBuilder()
			.UseConfigurationFile("config.ini")
			.UseConsoleArgs(argc, argv)
			.UseEnvironment(env)
			.Build();
	});

	auto config = di.GetInstance<Configuration>();

	if (config->offline) {
		di.For<IPlaybackDevice>().UseType<OfflinePlaybackDevice>();
	}
	else if (config->musicEnabled) {
		di.For<IPlaybackDevice>().UseType<MusicPlaybackDevice>();
	}
	else {
		di.For<IPlaybackDevice>().UseType<ClockPlaybackDevice>();
	}

}

static void printProgress(Console& console, bool isPlaying, double positionInSeconds);
void runApplication(ApplicationServices& app);

int main(int argc, char** argv, char** env)
{
	ApplicationServices app;
	int exitCode = 0;

	try
	{
		app.console.Open().Output << "Configuring application!\n";
		configure(app, argc, argv, env);
		app.console.Open().Output << "Starting application!\n";
		runApplication(app);
	}
	catch (const char* message) {
		app.console.Open().Error << "Fail: " << message << "\n";
		exitCode = 1;
	}
	catch (std::exception exception) {
		app.console.Open().Error << "Fail: " << exception.what() << "\n";
		exitCode = 1;
	}
	if (exitCode != 0) {
		app.console.Open().Output << "Shutting down due to failiure!";
	}
	return exitCode;
}

struct UniformBufferObject
{
	float width;
	float height;
	float time;
	float _reserved;
};

VulkanPhysicalDevice ChoosePhysicalDevice(
	Console& console,
	VulkanApplication& vulkan, 
	VulkanWindow& vulkanWindow,
	Configuration& config) {
	// devices
	VulkanPhysicalDeviceEnumeration enumeration(vulkan.instance, vulkanWindow.surface);
	if (enumeration.anyDevice == false) throw std::runtime_error("No device was found with Vulkan support. Check drivers!");
	if (enumeration.anySuitable == false) throw std::runtime_error("No device was found with all required features!");
	
	console.Open().Output << "available Devices:\n";
	int index = 0;
	for (auto& physicalDevice : enumeration.physicalDevices)
	{
		console.Open().Output
			<< "  " << index << " - "
			<< physicalDevice.properties.deviceName << " - "
			<< (physicalDevice.suitable ? "compatible" : "not compatible!")
			<< "\n";
		index++;
	}

	if (config.device_index == -1) {
		// print info about physical device
		auto& physical = *enumeration.top;
		console.Open().Output 
			<< "Use - " 
			<< physical.properties.deviceName 
			<< " - auto selected best device\n";
		return physical;
	}
	else if (config.device_index >= 0 && config.device_index < enumeration.physicalDevices.size())
	{
		auto& chosen = enumeration.physicalDevices[config.device_index];
		console.Open().Output
			<< "Use - "
			<< chosen.properties.deviceName
			<< " - as configured\n";
		return chosen;
	}
	else {
		throw std::runtime_error("Invalid device index in configuration.");
	}

}

void runApplication(ApplicationServices& app) {

	Configuration& config = *app.dependency.GetInstance<Configuration>();
	bool liveReload = config.liveReload;
	bool fullscreen = config.fullscreen;
	bool borderless = config.borderless;
	int xres = config.xres;
	int yres = config.yres;
	
	if (config.liveReload) {
		rebuild_shaders(app.console, config);
	}

	ApplyEnvVarChanges();

	AppVulkanLogger logger{app};

	VulkanApplication vulkan = VulkanApplicationBuilder(app.console)
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

	VulkanPhysicalDevice physical = ChoosePhysicalDevice(app.console, vulkan, vulkanWindow, config);

	// logical vulkan device
	app.console.Open().Output << "Initializing Vulkan Device.\n";
	VulkanDevice device(vulkan, physical);

	// directory change
	auto watchPath = "./";
	DirectoryChangeService* directory = nullptr;
	if (liveReload) {
		directory = new DirectoryChangeService(watchPath);
		app.console.Open().Output << "Watching " << watchPath << " for changes\n";
	}

	app.console.Open().Output << "Graphics queue index "  << physical.graphicsFamilyIndex << "\n";
	app.console.Open().Output << "Present queue index "  << physical.presentFamilyIndex << "\n";

	VulkanObjectBuilder builder(device);

	if (config.offline) {

		VulkanImageMemoryAllocator allocator = VulkanImageMemoryAllocator(physical.physicalDevice, device.device);
		VulkanImage image = VulkanImage::Builder(allocator).image_2d(256, 256).format_R32G32B32A32_SFLOAT().usage_color_attachment().usage_transfer_src();
		VulkanImageView imageView = VulkanImageView::Builder(image);

		VulkanRenderPass render_pass = builder.render_pass()
			.attachment(0).from(image).initial_layout_color_attachment().final_layout_color_attachment().store_final_color_depth()
			.subpass(0).writes_color_attachment(0);

		app.console.Open().Output << "Reading shader binaries.\n";
		VulkanShaderModule vertex_shader = builder.shader_module(read_shader("vert"));
		VulkanShaderModule fragment_shader = builder.shader_module(read_shader("frag"));

		VulkanDescriptorSetLayout uniform_descriptor = builder.descriptor_set_layout()
			.add_uniform_buffer(0).with_both_vertex_fragment_stage_access();

		VulkanPipelineLayout pipeline_layout = builder.pipeline_layout()
			.add(uniform_descriptor.m_vk_descriptor_set_layout);

		VulkanGraphicsPipeline pipeline = VulkanGraphicsPipelineBuilder(device.device)
			.no_vertex_input()
			.add_vertex_shader(vertex_shader)
			.topology_triangles()
			.viewport(256, 256)
			.add_fragment_shader(fragment_shader)
			.blend_soft_additive()
			.set_pipeline_layot(pipeline_layout.m_vk_pipeline_layout)
			.set_render_pass(render_pass.m_vk_render_pass)
			.set_subpass_index(0)
			.add_dynamic_state(VK_DYNAMIC_STATE_VIEWPORT)
			;

		VulkanDescriptorPool descriptor_pool = builder.descriptor_pool()
			.uniform_buffers(1);

		VulkanCommandPool command_pool = builder.command_pool()
			.queue_family_index(physical.graphicsFamilyIndex);
		
		VulkanCommandBuffer command_buffer = command_pool.allocate_command_buffer();

		BuiltinShaders builtin_shaders;

		auto shader_info = builtin_shaders.get_fullscreen_triangle_vert_shader();

		// hacky: disable the while after this if
		running = false;
	}

	while (running) {

		app.console.Open().Output << "Reading shader binaries.\n";
		auto vertShader = builder.shader_module(read_shader("vert"));
		auto fragShader = builder.shader_module(read_shader("frag"));

		app.console.Open().Output << "Creating swap chain.\n";
		physical.resetSwapChain();
		VulkanSwapChain swap(physical, device, width, height);

		resize = false;

		VulkanRenderPass render_pass = builder.render_pass()
			.attachment(0).set_format(swap.surfaceFormat.format).dont_care_initial_color_depth().store_final_color_depth().final_layout_present_src()
			.subpass(0).writes_color_attachment(0)
			;

		VulkanDescriptorSetLayout ubo_descriptor_set_layout = builder.descriptor_set_layout()
			.add_uniform_buffer(0).with_both_vertex_fragment_stage_access();

		VulkanPipelineLayout pipeline_layout = builder.pipeline_layout()
			.add(ubo_descriptor_set_layout.m_vk_descriptor_set_layout);

		VulkanGraphicsPipeline pipeline = builder.graphics_pipeline()
			.no_vertex_input()
			.topology_triangles()
			.add_vertex_shader(vertShader)
			.add_fragment_shader(fragShader)
			.viewport(swap.extent.width, swap.extent.height)
			.polygon_mode_fill()
			.blend_soft_additive()
			.set_pipeline_layot(pipeline_layout.m_vk_pipeline_layout)
			.set_render_pass(render_pass.m_vk_render_pass)
			.set_subpass_index(0)
			.add_dynamic_state(VK_DYNAMIC_STATE_VIEWPORT)
			;

		VkPipeline vk_pipeline = pipeline.m_vk_pipeline;

		SwapChainFramebuffers framebuffers(swap, device.device, render_pass.m_vk_render_pass);

		VulkanCommandPool command_pool = builder.command_pool()
			.queue_family_index(physical.graphicsFamilyIndex);


		VulkanDescriptorPool descriptor_pool = builder.descriptor_pool()
			.uniform_buffers(swap.swapChainImages.size());

		// uniform buffers
		std::vector<Buffer> uniformBuffers;


		uniformBuffers.reserve(swap.swapChainImages.size());

		for (size_t i = 0; i < swap.swapChainImages.size(); i++) {
			uniformBuffers.emplace_back(device.device, physical.physicalDevice, sizeof(UniformBufferObject), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
		}

		// command buffers
		auto command_buffers = command_pool.allocate_command_buffers(swap.swapChainImageViews.size());
		
		std::vector<VulkanDescriptorSet> descriptor_sets;
		
		for (size_t i = 0; i < swap.swapChainImages.size(); i++) {
			descriptor_sets.push_back(descriptor_pool.allocate_descriptor_set(ubo_descriptor_set_layout));
			descriptor_sets[i].update_to_uniform_buffer(uniformBuffers[i].buffer);
		}

		// recording 

		for (size_t i = 0; i < command_buffers.size(); i++) {
			auto& command_buffer = command_buffers[i];

			VkRect2D render_area = { 0, 0, swap.extent.width, swap.extent.height };
			VkClearValue clear_color = { 0.0f, 0.0f, 0.0f, 1.0f };
			VkViewport viewport = { 0, 0, swap.extent.width, swap.extent.height, 0, 1 };

			command_buffer.begin_recording_simultaneous()
				.begin_render_pass(render_pass.m_vk_render_pass, framebuffers.swapChainFramebuffers[i], render_area, 1, &clear_color)
				.bind_graphics_pipeline(pipeline)
				.bind_graphics_descriptor_set(pipeline_layout, descriptor_sets[i])
				.set_viewport(viewport)
				.draw(3)
				.end_render_pass()
				.end_recording();
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
				if (rebuild_shaders(app.console, config)) {
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
			submitInfo.pCommandBuffers = &command_buffers[imageIndex].m_vk_command_buffer;

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

		// wait before freeing resources
		vkDeviceWaitIdle(device.device);

		vkDestroySemaphore(device.device, renderFinishedSemaphore, nullptr);
		vkDestroySemaphore(device.device, imageAvailableSemaphore, nullptr);
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
