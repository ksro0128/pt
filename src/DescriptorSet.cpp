#include "include/DescriptorSet.h"

DescriptorSet::~DescriptorSet() {
	cleanup();
}

void DescriptorSet::cleanup() {
	std::cout << "DescriptorSet::cleanup" << std::endl;

	if (m_descriptorSet != VK_NULL_HANDLE) {
		vkFreeDescriptorSets(context->getDevice(), context->getDescriptorPool(), 1, &m_descriptorSet);
		m_descriptorSet = VK_NULL_HANDLE;
	}
}
