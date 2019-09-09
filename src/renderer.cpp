/*!
 *  \author    Viktor Zoutman
 *  \date      2019-2020
 *  \copyright GNU General Public License v3.0
 */

#include "renderer.hpp"

#include "application.hpp"
#include "vertex.hpp"
#include "graphics/context.hpp"
#include "graphics/command_queue.hpp"
#include "graphics/render_window.hpp"
#include "graphics/shader.hpp"
#include "graphics/pipeline_state.hpp"
#include "graphics/viewport.hpp"
#include "graphics/root_signature.hpp"
#include "graphics/command_list.hpp"
#include "graphics/gfx_settings.hpp"
#include "graphics/gfx_enums.hpp"
#include "graphics/staging_buffer.hpp"
#include "graphics/fence.hpp"

#include <iostream>

Renderer::~Renderer()
{
	WaitForAllPreviousWork();

	delete m_vertex_buffer;
	delete m_viewport;
	for (auto& fence : m_present_fences)
	{
		delete fence;
	}
	delete m_root_signature;
	delete m_pipeline;
	delete m_vs;
	delete m_ps;
	delete m_render_window;
	delete m_direct_cmd_list;
	delete m_direct_queue;
	delete m_context;
}

void Renderer::Init(Application* app)
{
	m_context = new gfx::Context(app);

	std::cout << "Initialized Vulkan" << std::endl;

	auto supported_extensions = m_context->GetSupportedExtensions();
	auto supported_device_extensions = m_context->GetSupportedDeviceExtensions();

	auto print_extensions_func = [](auto extensions)
	{
		for (auto extension : extensions)
		{
			std::cout << "\t- " << extension.extensionName << " (" << std::to_string(extension.specVersion) << ")" << std::endl;
		}
	};

	std::cout << "Supported Instance Extensions:" << std::endl;
	print_extensions_func(supported_extensions);

	std::cout << "Supported Device Extensions:" << std::endl;
	print_extensions_func(supported_device_extensions);

	m_render_window = new gfx::RenderWindow(m_context);
	m_direct_queue = new gfx::CommandQueue(m_context, gfx::CommandQueueType::DIRECT);
	m_direct_cmd_list = new gfx::CommandList(m_direct_queue);

	m_present_fences.resize(gfx::settings::num_back_buffers);
	for (auto& fence : m_present_fences)
	{
		fence = new gfx::Fence(m_context);
	}

	m_vs = new gfx::Shader(m_context);
	m_ps = new gfx::Shader(m_context);
	m_vs->LoadAndCompile("shaders/triangle.vert.spv", gfx::ShaderType::VERTEX);
	m_ps->LoadAndCompile("shaders/triangle.frag.spv", gfx::ShaderType::PIXEL);

	m_viewport = new gfx::Viewport(app->GetWidth(), app->GetHeight());

	m_root_signature = new gfx::RootSignature(m_context);
	m_root_signature->Compile();

	m_pipeline = new gfx::PipelineState(m_context);
	m_pipeline->SetViewport(m_viewport);
	m_pipeline->AddShader(m_vs);
	m_pipeline->AddShader(m_ps);
	m_pipeline->SetInputLayout(Vertex2D::GetInputLayout());
	m_pipeline->SetRenderTarget(m_render_window);
	m_pipeline->SetRootSignature(m_root_signature);
	m_pipeline->Compile();

	const std::vector<Vertex2D> vertices =
	{
		{{-1.0f, 1.0f}, {1.0f, 0.0f, 0.0f}},
		{{-1.0f, -1.0f}, {0.0f, 1.0f, 0.0f}},
		{{1.0f, 1.0f}, {0.0f, 1.0f, 0.0f}},
		{{1.0f, -1.0f}, {0.0f, 0.0f, 1.0f}}
	};

	m_vertex_buffer = new gfx::StagingBuffer(m_context, (void*)vertices.data(), vertices.size(), sizeof(Vertex2D), gfx::enums::BufferUsageFlag::VERTEX_BUFFER);

	// Upload
	m_direct_cmd_list->Begin(0);
	m_direct_cmd_list->StageBuffer(m_vertex_buffer, 0);
	m_direct_cmd_list->Close(0);
	m_direct_queue->Execute({ m_direct_cmd_list }, nullptr, 0);
	m_direct_queue->Wait();
	m_vertex_buffer->FreeStagingResources();
	std::cout << "Finished Uploading Resources" << std::endl;

	std::cout << "Finished Initializing Renderer" << std::endl;
}

void Renderer::Render()
{
	auto frame_idx = m_render_window->GetFrameIdx();

	m_present_fences[frame_idx]->Wait();

	m_render_window->AquireBackBuffer(m_present_fences[frame_idx]);

	m_direct_cmd_list->Begin(frame_idx);
	m_direct_cmd_list->BindRenderTargetVersioned(m_render_window, frame_idx);
	m_direct_cmd_list->BindPipelineState(m_pipeline, frame_idx);
	m_direct_cmd_list->BindVertexBuffer(m_vertex_buffer, frame_idx);
	m_direct_cmd_list->Draw(frame_idx, 4, 1);
	m_direct_cmd_list->Close(frame_idx);

	m_direct_queue->Execute({ m_direct_cmd_list }, m_present_fences[frame_idx], frame_idx);

	m_render_window->Present(m_direct_queue, m_present_fences[frame_idx]);
}

void Renderer::WaitForAllPreviousWork()
{
	for (auto& fence : m_present_fences)
	{
		fence->Wait();
	}
}

void Renderer::Resize(std::uint32_t width, std::uint32_t height)
{
	//WaitForAllPreviousWork();
	m_context->WaitForDevice();

	m_render_window->Resize(width, height);
	m_pipeline->SetRenderTarget(m_render_window);
	m_pipeline->Recompile();
}
