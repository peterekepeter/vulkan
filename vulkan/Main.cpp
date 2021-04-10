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
#include "VulkanFramebuffer.h"

#ifndef DEBUG
#include "renderdoc_app.h"
#include "../submodule/app-service-sandwich/AppServiceSandwich/AutoBuild.hpp"
#include "../submodule/curve-editor/curve-lib/io_binary.h"
#include "VulkanWindow.h"
RENDERDOC_API_1_1_2* rdoc_api = NULL;
#define DEBUG_FRAME_CAPTURE_INIT if (HMODULE mod = GetModuleHandleA("renderdoc.dll"))\
{\
	pRENDERDOC_GetAPI RENDERDOC_GetAPI =\
		(pRENDERDOC_GetAPI)GetProcAddress(mod, "RENDERDOC_GetAPI");\
	int ret = RENDERDOC_GetAPI(eRENDERDOC_API_Version_1_1_2, (void**)&rdoc_api);\
	if(ret != 1) throw std::runtime_error("oops!");\
}
#define DEBUG_FRAME_CAPTURE_START if(rdoc_api) rdoc_api->StartFrameCapture(NULL, NULL);
#define DEBUG_FRAME_CAPTURE_END if(rdoc_api) rdoc_api->EndFrameCapture(NULL, NULL);
#else // ! DEBUG
#define DEBUG_FRAME_CAPTURE_INIT
#define DEBUG_START_FRAME_CAPTURE
#define DEBUG_END_FRAME_CAPTURE
#endif


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

static bool rebuild_shaders(ApplicationServices& app)
{
	auto& config = *app.dependency.GetInstance<Configuration>();
	auto& console = *app.dependency.GetInstance<Console>();
	auto& atb = *app.dependency.GetInstance<AutoBuild>();

	if (config.vulkan_sdk == nullptr) {
		throw std::runtime_error("rebuilding shaders requires Vulkan SDK!\n" 
			"Download from: https://vulkan.lunarg.com/sdk/home#windows");
	}
	console.Open().Output << "Rebuilding shaders\n";
	bool success = true;

	std::string toolsPath = (*config.vulkan_sdk) + "/Bin";
	std::string compiler = toolsPath + "/glslangValidator.exe";
	std::string optimizer = toolsPath + "/spirv-opt.exe";

	atb.tool("glsl", [compiler, &console](const char* in, const char* out) { 
		auto cmd = compiler + " -V " + in + " -o " + out;
		auto result = ProcessCommand::Execute(cmd);
		if (result.exitCode != 0) {
			console.Open().Error << result.output;
		}
		return result.exitCode;
	});

	atb.tool("spv-opt", [optimizer, &console](const char* in, const char* out) {
		auto cmd = optimizer + " -O " + in + " -o " + out;
		auto result = ProcessCommand::Execute(cmd);
		if (result.exitCode != 0) {
			console.Open().Error << result.output;
		}
		return result.exitCode;
	});

	make_sure_dir_exists("build");
	atb.step("glsl", "shader.frag", "build/shader.frag.spv");
	atb.step("glsl", "shader.vert", "build/shader.vert.spv");
	atb.step("spv-opt", "build/shader.frag.spv", "build/shader.frag.opt.spv");
	atb.step("spv-opt", "build/shader.vert.spv", "build/shader.vert.opt.spv");
	atb.wait_idle();
	return success;
}

static std::vector<char> read_file(const std::string& filename) {
    std::ifstream file(filename, std::ios::ate | std::ios::binary);

    if (!file.is_open()) {
        throw std::runtime_error(std::string("failed to open file! ") + filename);
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
	newname = "build\\" + filename + ".opt.spv";
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
	di.For<AutoBuild>().UseDefaultConstructor();

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

struct RunApplicationResult
{
	bool requested_reload = false;
};

static void printProgress(Console& console, bool isPlaying, double positionInSeconds);
RunApplicationResult runApplication(ApplicationServices& app);


int main(int argc, char** argv, char** env)
{
	// some win32 function calls depend on this being defined
	expect(UNICODE == 1 && _UNICODE == 1, "compiled with unicode flag")

	ApplicationServices app;
	int exit_code = 0;
	bool first_run = true;


	try
	{
		RunApplicationResult result;
		while (first_run || result.requested_reload)
		{
			first_run = false;
			app.console.Open().Output << "Configuring application!\n";
			configure(app, argc, argv, env);
			app.console.Open().Output << "Starting application!\n";
			result = runApplication(app);
		}
	}
	catch (const char* message) {
		app.console.Open().Error << "Fail: " << message << "\n";
		exit_code = 1;
	}
	catch (std::exception exception) {
		app.console.Open().Error << "Fail: " << exception.what() << "\n";
		exit_code = 1;
	}
	if (exit_code != 0) {
		app.console.Open().Output << "Shutting down due to failiure!";
	}
	return exit_code;
}

void print_time_span(Console::Transaction& con, int64_t ms)
{
	int s = ms / 1000;
	int m = s / 60;
	int h = m / 60;
	int d = h / 24;

	if (d > 0) {
		con.Output << d << "d " << (h - d * 24) << "h";
	}
	else if (h > 0)
	{
		con.Output << h << "h " << (m - h * 60) << "m";
	}
	else if (m > 0)
	{
		con.Output << m << "m " << (s - m * 60) << "s";
	}
	else
	{
		con.Output << s << "s " << (ms - s * 1000) << "ms";
	}
}

class stopwatch
{
	std::chrono::steady_clock::time_point start_time;

public:
	stopwatch()
	{
		start_time = std::chrono::steady_clock::now();
	}

	int64_t elapsed_milliseconds() {
		auto elapsed = std::chrono::steady_clock::now() - start_time;
		auto elapsed_ms = std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count();
		return elapsed_ms;
	}
};

struct UniformBufferObject
{
	float width;
	float height;
	float time;
	float _reserved;
};

struct OutputUniformBufferObject
{
	float image_count;
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

RunApplicationResult runApplication(ApplicationServices& app) {

	Configuration& config = *app.dependency.GetInstance<Configuration>();
	bool liveReload = config.liveReload;
	bool fullscreen = config.fullscreen;
	bool borderless = config.borderless;
	int xres = config.xres;
	int yres = config.yres;
	
	if (config.liveReload) {
		rebuild_shaders(app);
	}

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
	windowInitInfo.x_res = xres;
	windowInitInfo.y_res = yres;
	windowInitInfo.title = config.windowTitle;

	windowInitInfo.on_close = [&]() {
		running = false;
		app.console.Open().Output << "Closed window.\n";
	};

	windowInitInfo.on_resize = [&](int x, int y) {
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
		windowInitInfo.on_key = [&](int key, bool state) {
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

	windowInitInfo.on_paint = [&]() {
		shouldPaint = true;
	};

	app.console.Open().Output << "Creating window.\n";
	Window win32_window(windowInitInfo);

	// create window for vulkan
	app.console.Open().Output << "Creating surface.\n";
	VulkanWindow vulkanWindow(vulkan.instance, win32_window.get_hwnd());

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

		// prevent system from going to sleep, but goes to away mode instead and can turn off the monitor
		SetThreadExecutionState(ES_CONTINUOUS | ES_SYSTEM_REQUIRED | ES_AWAYMODE_REQUIRED);

		DEBUG_FRAME_CAPTURE_INIT;
		DEBUG_FRAME_CAPTURE_START;

		VulkanImageMemoryAllocator allocator = VulkanImageMemoryAllocator(physical.physicalDevice, device.device);

		// image used as HDR renderbuffer for accumulating color information
		VulkanImage accumulator_image = VulkanImage::Builder(allocator)
			.image_2d(width, height)
			.format_R32G32B32A32_SFLOAT()
			.usage_color_attachment()
			.usage_sampled();

		VulkanImageView accumulator_image_view = VulkanImageView::Builder(accumulator_image);

		// image used as buffer to output SDR content
		VulkanImage render_output_image = VulkanImage::Builder(allocator)
			.image_2d(width, height)
			.format_R8G8B8A8_UNORM()
			.usage_color_attachment()
			.usage_transfer_src();

		// image which can be read by CPU
		VulkanImage host_visible_image = VulkanImage::Builder(allocator)
			.image_2d(width, height)
			.format_R8G8B8A8_UNORM()
			.usage_transfer_dst()
			.host_visible_and_coherent();

		VulkanImageView render_output_image_view = VulkanImageView::Builder(render_output_image);

		VulkanRenderPass render_pass_accumulate = builder.render_pass()
			.attachment(0).from(accumulator_image).initial_layout_color_attachment().final_layout_color_attachment().load_initial_color_depth().store_final_color_depth()
			.subpass(0).writes_color_attachment(0);

		VulkanRenderPass render_pass_output = builder.render_pass()
			.attachment(0).from(host_visible_image).initial_layout_color_attachment().final_layout_transfer_src().store_final_color_depth()
			.subpass(0).writes_color_attachment(0);

		app.console.Open().Output << "Reading shader binaries.\n";
		VulkanShaderModule vertex_shader = builder.shader_module(read_shader("shader.vert"));
		VulkanShaderModule fragment_shader = builder.shader_module(read_shader("shader.frag"));

		VulkanDescriptorSetLayout descriptor_set_layout = builder.descriptor_set_layout()
			.add_uniform_buffer(0).with_both_vertex_fragment_stage_access();

		VulkanPipelineLayout main_pipeline_layout = builder.pipeline_layout()
			.add(descriptor_set_layout.m_vk_descriptor_set_layout);


		VulkanGraphicsPipeline main_pipeline = VulkanGraphicsPipelineBuilder(device.device)
			.no_vertex_input()
			.add_vertex_shader(vertex_shader)
			.topology_triangles()
			.viewport(width, height)
			.add_fragment_shader(fragment_shader)
			.blend_additive()
			.set_pipeline_layot(main_pipeline_layout.m_vk_pipeline_layout)
			.set_render_pass(render_pass_accumulate.m_vk_render_pass)
			.set_subpass_index(0);


		BuiltinShaders builtin_shaders;
		auto fullscreen_triangle_shader_data = builtin_shaders.get_fullscreen_triangle_vert_shader();
		auto output_sdr_shader_data = builtin_shaders.get_sdr_output_frag_shader();

		VulkanShaderModule fullscreen_triangle_vert_shader = builder.shader_module(fullscreen_triangle_shader_data.data, fullscreen_triangle_shader_data.size);
		VulkanShaderModule sdr_output_frag_shader = builder.shader_module(output_sdr_shader_data.data, output_sdr_shader_data.size);

		VulkanDescriptorSetLayout output_descriptor_set_layout = builder.descriptor_set_layout()
			.add_uniform_buffer(0).with_fragment_stage_access()
			.add_combined_image_sampler(1).with_fragment_stage_access();

		VulkanPipelineLayout sdr_output_pipeline_layout = builder.pipeline_layout()
			.add(output_descriptor_set_layout.m_vk_descriptor_set_layout);

		VulkanGraphicsPipeline sdr_output_pipeline = builder.graphics_pipeline()
			.no_vertex_input()
			.add_vertex_shader(fullscreen_triangle_vert_shader)
			.topology_triangles()
			.viewport(width, height)
			.add_fragment_shader(sdr_output_frag_shader)
			.no_blend()
			.set_pipeline_layot(sdr_output_pipeline_layout.m_vk_pipeline_layout)
			.set_render_pass(render_pass_output.m_vk_render_pass)
			.set_subpass_index(0);

		VulkanDescriptorPool descriptor_pool = builder.descriptor_pool()
			.uniform_buffers(2)
			.combined_image_samplers(1);

		VulkanCommandPool command_pool = builder.command_pool()
			.queue_family_index(physical.graphicsFamilyIndex);
		
		VulkanCommandBuffer main_cmd_buffer = command_pool.allocate_command_buffer();


		VulkanDescriptorSet main_ubo_descriptor_set = descriptor_pool.allocate_descriptor_set(descriptor_set_layout);

		Buffer ubo_buffer(device.device, physical.physicalDevice, sizeof(UniformBufferObject), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

		main_ubo_descriptor_set.write_uniform_buffer(0, ubo_buffer.buffer);

		VulkanFramebuffer accumulator_framebuffer = builder.framebuffer().render_pass(render_pass_accumulate).size(width, height).attachment(accumulator_image_view);

		VkRect2D render_area = { 0, 0, width, height };
		VkClearValue clear_color = { 0.0f, 0.0f, 0.0f, 1.0f };
		VkClearColorValue clear_color_value = { 0.0f, 0.0f, 0.0f, 1.0f };
		VkViewport viewport = { 0, 0, width, height, 0, 1 };

		auto init_command_buffer = command_pool.allocate_command_buffer();

		init_command_buffer
			.begin_recording()
			.begin_barrier(VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, (VkDependencyFlagBits)0)
			.image(0, 0, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_QUEUE_FAMILY_IGNORED, VK_QUEUE_FAMILY_IGNORED, accumulator_image.m_vk_image, VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1)
			.end_barrier()
			.begin_render_pass(render_pass_accumulate.m_vk_render_pass, accumulator_framebuffer.m_vk_framebuffer, render_area, 0, nullptr)
			.clear_attachment(0, VK_IMAGE_ASPECT_COLOR_BIT, clear_color, width, height)
			.end_render_pass()
			.end_recording();

		main_cmd_buffer
			.begin_recording()
			.begin_render_pass(render_pass_accumulate.m_vk_render_pass, accumulator_framebuffer.m_vk_framebuffer, render_area, 0, nullptr)
			.bind_graphics_pipeline(main_pipeline)
			.bind_graphics_descriptor_set(main_pipeline_layout, main_ubo_descriptor_set)
			.draw(3)
			.end_render_pass()
			.end_recording();

		VulkanDescriptorSet output_descriptor_set = descriptor_pool.allocate_descriptor_set(output_descriptor_set_layout);
		Buffer output_ubo_buffer(device.device, physical.physicalDevice, sizeof(OutputUniformBufferObject), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

		VulkanSampler framebuffer_sampler = builder.sampler().unnormalized_coordinates().filter_nearest();

		output_descriptor_set
			.write_uniform_buffer(0, output_ubo_buffer.buffer)
			.write_image_sampler(1, framebuffer_sampler, accumulator_image_view);

		auto output_cmd_buffer = command_pool.allocate_command_buffer();

		VulkanFramebuffer output_framebuffer = builder.framebuffer()
			.render_pass(render_pass_output).size(width, height).attachment(render_output_image_view);

		output_cmd_buffer.begin_recording()
			.begin_barrier(VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, (VkDependencyFlagBits)0)
			.image(0, 0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_QUEUE_FAMILY_IGNORED, VK_QUEUE_FAMILY_IGNORED, accumulator_image.m_vk_image, VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1)
			.image(0, 0, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_QUEUE_FAMILY_IGNORED, VK_QUEUE_FAMILY_IGNORED, render_output_image.m_vk_image, VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1)
			.end_barrier()
			.begin_render_pass(render_pass_output, output_framebuffer, render_area, 1, &clear_color)
			.bind_graphics_pipeline(sdr_output_pipeline)
			.bind_graphics_descriptor_set(sdr_output_pipeline_layout, output_descriptor_set)
			.draw(3)
			.end_render_pass()
			.begin_barrier(VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, (VkDependencyFlagBits)0)
			.image(0, 0, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_QUEUE_FAMILY_IGNORED, VK_QUEUE_FAMILY_IGNORED, host_visible_image.m_vk_image, VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1)
			.end_barrier()
			.copy_image(render_output_image.m_vk_image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, host_visible_image.m_vk_image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, width, height)
			.end_recording();

		int sample_count = config.sample_count;
		int fps = config.fps;
		int frame_count = config.frame_count;
		float shutter_open = 0.5;

		std::ofstream outfile;
		outfile.open(config.outFile.c_str(), std::ios::out | std::ios::binary); 
		auto render_stopwatch = stopwatch();

		for (int frame = 0; frame < frame_count; frame++) 
		{
			{
				auto con = app.console.ProgressPrinter("playback progress");
				con.Output << "frame " << (frame + 1) << " / " << frame_count;
				
				auto elapsed_ms = render_stopwatch.elapsed_milliseconds();

				if (frame > 0 && elapsed_ms > 1000)
				{
					double ms_per_frame = elapsed_ms / (double)frame;
					auto remaining_frames = frame_count - frame;
					int remaining_ms = (int)(remaining_frames * ms_per_frame);
					con.Output << " about ";
					print_time_span(con, remaining_ms);
					con.Output << " left    ";
				}
			}

			VkSubmitInfo submit_info = {};
			submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
			submit_info.commandBufferCount = 1;
			submit_info.pCommandBuffers = &init_command_buffer.m_vk_command_buffer;
			vkQueueSubmit(device.graphicsQueue, 1, &submit_info, VK_NULL_HANDLE);

			vkDeviceWaitIdle(device.device);

			for (int i = 0; i < sample_count; i++)
			{
				UniformBufferObject ubo;
				ubo.width = static_cast<float>(width);
				ubo.height = static_cast<float>(height);
				ubo.time = (frame + (shutter_open * float(i) / sample_count)) * (1.0f / fps);
				ubo._reserved = 0.0f;

				void* data;
				vkMapMemory(device.device, ubo_buffer.bufferMemory, 0, sizeof(ubo), 0, &data);
				memcpy(data, &ubo, sizeof(ubo));
				vkUnmapMemory(device.device, ubo_buffer.bufferMemory);

				VkSubmitInfo submit_info = {};
				submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
				submit_info.commandBufferCount = 1;
				submit_info.pCommandBuffers = &main_cmd_buffer.m_vk_command_buffer;
				vkQueueSubmit(device.graphicsQueue, 1, &submit_info, VK_NULL_HANDLE);

				vkDeviceWaitIdle(device.device);
			}

			{
				OutputUniformBufferObject ubo;
				ubo.image_count = sample_count;

				void* data;
				vkMapMemory(device.device, output_ubo_buffer.bufferMemory, 0, sizeof(ubo), 0, &data);
				memcpy(data, &ubo, sizeof(ubo));
				vkUnmapMemory(device.device, output_ubo_buffer.bufferMemory);

				VkSubmitInfo submit_info = {};
				submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
				submit_info.commandBufferCount = 1;
				submit_info.pCommandBuffers = &output_cmd_buffer.m_vk_command_buffer;
				vkQueueSubmit(device.graphicsQueue, 1, &submit_info, VK_NULL_HANDLE);

				vkDeviceWaitIdle(device.device);
			}

			uint32_t image_data_size = width * height * 4;
			std::vector<unsigned char> cpu_image_buffer;
			cpu_image_buffer.resize(image_data_size);
			char* mapped_image_data;

			DEBUG_FRAME_CAPTURE_END

				vkMapMemory(device.device, host_visible_image.m_memory.m_vk_memory, 0, image_data_size, 0, (void**)&mapped_image_data);
			memcpy(cpu_image_buffer.data(), mapped_image_data, image_data_size);
			vkUnmapMemory(device.device, host_visible_image.m_memory.m_vk_memory);

			for (int i = 0; i < image_data_size; i += 4) {
				unsigned char r = cpu_image_buffer[i];
				unsigned char g = cpu_image_buffer[i + 1];
				unsigned char b = cpu_image_buffer[i + 2];
				outfile << r << g << b;
			}
		}

		// preventing sleep is no longer a requirement
		SetThreadExecutionState(ES_CONTINUOUS);

		// hacky: disable the while after this if
		running = false;
	}

	while (running) {

		app.console.Open().Output << "Reading shader binaries.\n";
		auto vertShader = builder.shader_module(read_shader("shader.vert"));
		auto fragShader = builder.shader_module(read_shader("shader.frag"));

		document_model curves;
		std::vector<float> curves_eval_result;

		if (config.curvesFile.length() > 0) {
			app.console.Open().Output << "Reading curves file '" << config.curvesFile << "'\n";
			std::ifstream file(config.curvesFile, std::ios::binary);
			io_binary::read(file, curves);
			curves_eval_result.resize(curves.curve_list.size());
		}

		app.console.Open().Output << "Creating swap chain.\n";
		physical.resetSwapChain();
		VulkanSwapChain swap(physical, device, width, height);

		resize = false;

		VulkanRenderPass render_pass = builder.render_pass()
			.attachment(0).set_format(swap.surfaceFormat.format).dont_care_initial_color_depth().store_final_color_depth().final_layout_present_src()
			.subpass(0).writes_color_attachment(0)
			;

		VulkanDescriptorSetLayout ubo_descriptor_set_layout = builder.descriptor_set_layout()
			.add_uniform_buffer(0).with_both_vertex_fragment_stage_access()
			.add_uniform_buffer(1).with_both_vertex_fragment_stage_access();

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
			.uniform_buffers(swap.swapChainImages.size() * 2);

		// uniform buffers
		std::vector<Buffer> uniformBuffers;
		std::vector<Buffer> uniformParamBuffers;

		uniformBuffers.reserve(swap.swapChainImages.size());
		uniformParamBuffers.reserve(swap.swapChainImages.size());

		for (size_t i = 0; i < swap.swapChainImages.size(); i++) {
			uniformBuffers.emplace_back(device.device, physical.physicalDevice, sizeof(UniformBufferObject), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
			uniformParamBuffers.emplace_back(device.device, physical.physicalDevice, curves.curve_list.size() * sizeof(float), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
		}

		// command buffers
		auto command_buffers = command_pool.allocate_command_buffers(swap.swapChainImageViews.size());
		
		std::vector<VulkanDescriptorSet> descriptor_sets;
		
		for (size_t i = 0; i < swap.swapChainImages.size(); i++) {
			descriptor_sets.push_back(descriptor_pool.allocate_descriptor_set(ubo_descriptor_set_layout));
			descriptor_sets[i].write_uniform_buffer(0, uniformBuffers[i].buffer);
			descriptor_sets[i].write_uniform_buffer(1, uniformParamBuffers[i].buffer);
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
			process_thread_message_queue_non_blocking();

			bool rebuild = false;
			if (liveReload && directory != nullptr) {

				auto atb = app.dependency.GetInstance<AutoBuild>();

				while (directory->HasChange())
				{
					shouldPaint = true;
					auto diff = directory->ReadChange();
					if (diff.filename == "config.ini") {
						app.console.Open().Output << "'" << diff.filename << "' changed, reloading everything...\n";
						vkDeviceWaitIdle(device.device);
						RunApplicationResult result;
						result.requested_reload = true;
						return result;
					}
					atb->notify_file_change(diff.filename.c_str());
				}

				if (!atb->m_is_idle) {
					atb->wait_idle();
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

			// eval curves and write to GPU buffer
			for (int i = 0; i < curves.curve_list.size(); i++) {
				curves_eval_result[i] = curves.curve_list[i].eval(ubo.time);
			}
			vkMapMemory(device.device, uniformParamBuffers[imageIndex].bufferMemory, 0, curves_eval_result.size() * sizeof(float), 0, &data);
			memcpy(data, curves_eval_result.data(), curves_eval_result.size() * sizeof(float));
			vkUnmapMemory(device.device, uniformParamBuffers[imageIndex].bufferMemory);

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

	return RunApplicationResult{};
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
