/*!
 *  \author    Viktor Zoutman
 *  \date      2019-2020
 *  \copyright GNU General Public License v3.0
 */

#include "vk_material_pool.hpp"

#include "../texture_pool.hpp"
#include "gpu_buffers.hpp"
#include "descriptor_heap.hpp"
#include "../util/log.hpp"
#include "context.hpp"

gfx::VkMaterialPool::VkMaterialPool(gfx::Context* context)
	: m_context(context),
	m_material_set_layout(VK_NULL_HANDLE),
	m_desc_heap(nullptr)
{
	auto logical_device = context->m_logical_device;

	gfx::DescriptorHeap::Desc desc;
	desc.m_versions = 1;
	desc.m_num_descriptors = 100;
	m_desc_heap = new gfx::DescriptorHeap(m_context, desc);

	// TODO: make this entire layout static and use it when creating root signatures.
	std::vector<VkDescriptorSetLayoutBinding> parameters(1);
	parameters[0].binding = 1; // root parameter 1
	parameters[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	parameters[0].descriptorCount = 3;
	parameters[0].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
	parameters[0].pImmutableSamplers = nullptr;

	VkDescriptorSetLayoutCreateInfo descriptor_set_create_info = {};
	descriptor_set_create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	descriptor_set_create_info.bindingCount = parameters.size();
	descriptor_set_create_info.pBindings = parameters.data();

	if (vkCreateDescriptorSetLayout(logical_device, &descriptor_set_create_info, nullptr, &m_material_set_layout) != VK_SUCCESS)
	{
		LOGC("failed to create descriptor set layout!");
	}
}

gfx::VkMaterialPool::~VkMaterialPool()
{
	auto logical_device = m_context->m_logical_device;

	vkDestroyDescriptorSetLayout(logical_device, m_material_set_layout, nullptr);

	delete m_desc_heap;
}

std::uint32_t gfx::VkMaterialPool::GetDescriptorSetID(MaterialHandle handle)
{
	if (auto it = m_descriptor_sets.find(handle.m_material_id); it != m_descriptor_sets.end())
	{
		return it->second;
	}

	LOGE("Failed to get descriptor set id from material handle");
	return 0;
}

gfx::DescriptorHeap* gfx::VkMaterialPool::GetDescriptorHeap()
{
	return m_desc_heap;
}

void gfx::VkMaterialPool::Load_Impl(MaterialHandle& handle, MaterialData const & data, TexturePool* texture_pool)
{
	// Textures
	auto textures = texture_pool->GetTextures({ handle.m_albedo_texture_handle, handle.m_normal_texture_handle, handle.m_roughness_texture_handle });
	auto descriptor_set_id = m_desc_heap->CreateSRVSetFromTexture(textures, m_material_set_layout, 1, 0);
	handle.m_material_set_id = descriptor_set_id;
	m_descriptor_sets.insert({ handle.m_material_id, descriptor_set_id }); //TODO: Unhardcode this handle (1). We want this to be a global static. see line 25
}