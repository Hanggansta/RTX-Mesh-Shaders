/*!
 *  \author    Viktor Zoutman
 *  \date      2019-2020
 *  \copyright GNU General Public License v3.0
 */

#pragma once

#include <vulkan/vulkan.h>
#include <vector>
#include "render_target.hpp"


namespace gfx
{

	class Context;
	class Viewport;
	class RootSignature;
	class Shader;
	class RenderTarget;

	class PipelineState
	{
		friend class CommandList;
	public:
		explicit PipelineState(Context* context);
		~PipelineState();

		void SetViewport(Viewport* viewport);
		void SetRootSignature(RootSignature* root_signature);
		void AddShader(Shader* shader);
		void SetRenderTarget(RenderTarget* target);

		void Compile();

	private:
		std::vector<VkPipelineShaderStageCreateInfo> m_shader_info;
		VkPipelineVertexInputStateCreateInfo m_vertex_input_info;
		VkPipelineInputAssemblyStateCreateInfo m_ia_info;
		VkPipelineViewportStateCreateInfo m_viewport_info;
		VkPipelineRasterizationStateCreateInfo m_raster_info;
		VkPipelineMultisampleStateCreateInfo m_ms_info;
		VkPipelineColorBlendAttachmentState m_color_blend_attachment_info;
		VkPipelineColorBlendStateCreateInfo m_color_blend_info;
		VkPipelineLayout m_layout;
		RootSignature* m_root_signature;
		RenderTarget* m_render_target;
		VkGraphicsPipelineCreateInfo m_create_info;
		VkPipeline m_pipeline;

		Context* m_context;
	};

} /* gfx */