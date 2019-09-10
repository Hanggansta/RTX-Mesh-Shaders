#include "descriptor_heap.hpp"

#include "context.hpp"
#include "gfx_settings.hpp"
#include "root_signature.hpp"
#include "gpu_buffers.hpp"

gfx::DescriptorHeap::DescriptorHeap(Context* context, RootSignature* root_signature, Desc desc)
	: m_context(context), m_desc(desc), m_descriptor_pool_create_info()
{
	auto logical_device = m_context->m_logical_device;

	m_descriptor_pools.resize(desc.m_versions);

	// Create the descriptor pool
	VkDescriptorPoolSize pool_size = {};
	pool_size.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	pool_size.descriptorCount = desc.m_num_descriptors;

	m_descriptor_pool_create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	m_descriptor_pool_create_info.poolSizeCount = 1;
	m_descriptor_pool_create_info.pPoolSizes = &pool_size;
	m_descriptor_pool_create_info.maxSets = desc.m_num_descriptors;

	m_descriptor_sets.resize(desc.m_versions);

	for (auto version = 0; version < desc.m_versions; version++)
	{
		if (vkCreateDescriptorPool(logical_device, &m_descriptor_pool_create_info, nullptr, &m_descriptor_pools[version]) !=
		    VK_SUCCESS)
		{
			throw std::runtime_error("failed to create descriptor pool!");
		}

		if (!root_signature) continue;

		// Create the descriptor tables
		std::vector<VkDescriptorSetLayout> layouts(desc.m_num_descriptors);
		for (auto i = 0; i < desc.m_num_descriptors; i++)
		{
			layouts[i] = root_signature->m_descriptor_set_layouts[i];
		}

		VkDescriptorSetAllocateInfo alloc_info = {};
		alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		alloc_info.descriptorPool = m_descriptor_pools[version];
		alloc_info.descriptorSetCount = desc.m_num_descriptors;
		alloc_info.pSetLayouts = layouts.data();

		m_descriptor_sets[version].resize(desc.m_num_descriptors);
		if (vkAllocateDescriptorSets(logical_device, &alloc_info, m_descriptor_sets[version].data()) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to allocate descriptor sets!");
		}
	}
}

gfx::DescriptorHeap::~DescriptorHeap()
{
	auto logical_device = m_context->m_logical_device;

	for (auto& pool : m_descriptor_pools)
	{
		vkDestroyDescriptorPool(logical_device, pool, nullptr);
	}
}

void gfx::DescriptorHeap::CreateSRVFromCB(GPUBuffer* buffer, std::uint32_t handle, std::uint32_t frame_idx)
{
	auto logical_device = m_context->m_logical_device;

	VkDescriptorBufferInfo buffer_info = {};
	buffer_info.buffer = buffer->m_buffer;
	buffer_info.offset = 0;
	buffer_info.range = buffer->m_size;

	VkWriteDescriptorSet descriptor_write = {};
	descriptor_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	descriptor_write.dstSet = m_descriptor_sets[frame_idx][handle];
	descriptor_write.dstBinding = 0;
	descriptor_write.dstArrayElement = 0;
	descriptor_write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	descriptor_write.descriptorCount = 1;
	descriptor_write.pBufferInfo = &buffer_info;
	descriptor_write.pImageInfo = nullptr;
	descriptor_write.pTexelBufferView = nullptr;
	vkUpdateDescriptorSets(logical_device, 1, &descriptor_write, 0, nullptr);
}