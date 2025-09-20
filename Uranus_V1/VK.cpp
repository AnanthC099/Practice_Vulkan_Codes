//Header file
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <cmath>
//header file for texture
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "VK.h"
//This inclusion of <vulkan/vulkan.h> is needed in all platforms
//to distinguish the platfrom, below macro is needed
#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan.h>

//glm related macros and header files
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE   //clipping values between 0 and 1 for depth
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"

#pragma comment(lib, "user32.lib")
#pragma comment(lib, "gdi32.lib")
// Vulkan related library
#pragma comment(lib, "vulkan-1.lib")

//Macros
/* History of Graphics adapter
Aspect ratio od 4:3 is supported by all monitors
Monochrome
CGA -> Color Graphics Adapter
EGA -> Extended Graphics Adapter
VGA -> Video Graphics Adapter
SVGA -> Super Video Graphics Adapter (4:3 Aspect ratio)
*/
#define WIN_WIDTH  800
#define WIN_HEIGHT 600

//Callback function declaration
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

void Update(void);

const char* gpszAppName = "ARTR";

//Global variables
HWND ghwnd = NULL;
HDC ghdc = NULL;
HGLRC ghrc = NULL;

DWORD dwStyle;
WINDOWPLACEMENT wpPrev;

int gbActiveWindow = 0;
int gbFullscreen = 0;
int gbWindowMinimized = FALSE;

int giHeight = 0;
int giWidth = 0;

FILE *gpFile = NULL;

//Vulkan related global variables
//Instance Extension releated variables
uint32_t enabledInstanceExtensionCount = 0;
//VK_KHR_SURFACE_EXTENSION_NAME
//VK_KHR_WIN32_SURFACE_EXTENSION_NAME
//VK_EXT_DEBUG_REPORT_EXTENSION_NAME
const char* enabledInstanceExtensionNames_array[3];

//Vulkan Instance
VkInstance vkInstance = VK_NULL_HANDLE;

//Vulkan Presentaiton Surface
VkSurfaceKHR vkSurfaceKHR = VK_NULL_HANDLE;

//vulkan Physical Device related
VkPhysicalDevice vkPhysicalDevice_selected = VK_NULL_HANDLE;
uint32_t graphicsQueueFamilyIndex_selected = UINT32_MAX;
VkPhysicalDeviceMemoryProperties vkPhysicalDeviceMemoryProperties;

//vulkan Printing VkInfo related 
VkPhysicalDevice *vkPhysicalDevice_array = NULL; 
uint32_t physicalDeviceCount = 0;

//Device Extension releated variables
uint32_t enabledDeviceExtensionCount = 0;
//VK_KHR_SWAPCHAIN_EXTENSION_NAME
const char* enabledDeviceExtensionNames_array[1];

//Vulkan Device
VkDevice vkDevice = VK_NULL_HANDLE;

//DeviceQueue
VkQueue vkQueue = VK_NULL_HANDLE;

//Color format and color Space
VkFormat vkFormat_color = VK_FORMAT_UNDEFINED;
VkColorSpaceKHR vkColorSpaceKHR = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;

//Presentation Mode
VkPresentModeKHR vkPresentModeKHR = VK_PRESENT_MODE_FIFO_KHR;

//Swapchain related
int winWidth = WIN_WIDTH;
int winHeight = WIN_HEIGHT;
VkSwapchainKHR vkSwapchainKHR = VK_NULL_HANDLE;
VkExtent2D vkExtent2D_swapchain;

//for color images
//SwapchainImage and swapchainImagesViews releated
uint32_t swapchainImageCount = UINT32_MAX;
VkImage* swapchainImage_array = NULL;
VkImageView* swapchainImageView_array = NULL;

//for depth image
VkFormat vkFormat_depth = VK_FORMAT_UNDEFINED;
VkImage vkImage_depth = VK_NULL_HANDLE;
VkDeviceMemory vkDeviceMemory_depth = VK_NULL_HANDLE;
VkImageView vkImageView_depth = VK_NULL_HANDLE;


//commandPool
VkCommandPool vkCommandPool = VK_NULL_HANDLE;

//Command Buffer
VkCommandBuffer* vkCommandBuffer_array = NULL;

//RenderPass
VkRenderPass vkRenderPass = VK_NULL_HANDLE;

//FrameBuffers
VkFramebuffer *vkFrameBuffer_array = NULL;

//semaphore
VkSemaphore vkSemaphore_backBuffer = VK_NULL_HANDLE;
VkSemaphore vkSemaphore_renderComplete = VK_NULL_HANDLE;

//Fence
VkFence *vkFence_array = NULL;

//BuildCommandBuffers
//clear color values
VkClearColorValue vkClearColorValue;  // 3 arrays
VkClearDepthStencilValue vkClearDepthStencilValue;  //for depth

//Render
VkBool32 bInitialized = FALSE;
uint32_t currentImageIndex = UINT32_MAX;

//Validation
BOOL bValidation = TRUE;
uint32_t enabledValidationLayerCount = 0;
const char* enabledValidationLayerNames_array[1]; //for VK_LAYER_KHRONOS_validation
VkDebugReportCallbackEXT vkDebugReportCallbackEXT = VK_NULL_HANDLE;
PFN_vkDestroyDebugReportCallbackEXT vkDestroyDebugReportCallbackEXT_fnptr = NULL;

//VertexBuffer related variables
typedef struct 
{
    VkBuffer vkBuffer;
    VkDeviceMemory vkDeviceMemory;
}VertexData;

//position
VertexData vertexData_position;
VertexData vertexData_texcoord;

//Uniform related declarations
struct MyUniformData
{
    glm::mat4 modelMatrix;
    glm::mat4 viewMatrix;
    glm::mat4 projectionMatrix;
	
	glm::vec4 lightDir;      // world-space, normalized (xyz), w unused
    glm::vec4 lightColor;    // rgb, a unused
    glm::vec4 ambientColor;  // rgb, a unused
    glm::vec4 params;        // x = displacementScale (object units), y,z,w unused
	
	// L2 Spherical Harmonics irradiance coefficients, RGB in xyz of each vec4
    // (9 basis * 3 channels) stored as 9 vec4s for std140 alignment; .w unused
    glm::vec4 sh[9];
};

struct UniformData
{
    VkBuffer vkBuffer;
    VkDeviceMemory vkDeviceMemory;
};

UniformData uniformData;

//Shader related variables
VkShaderModule vkShaderModule_vertex_shader = VK_NULL_HANDLE;
VkShaderModule vkShaderModule_fragment_shader = VK_NULL_HANDLE;

//DescriptorSetLayout realted variables
VkDescriptorSetLayout vkDescriptorSetLayout = VK_NULL_HANDLE;

//PipelineLayout realted variables
VkPipelineLayout vkPipelineLayout = VK_NULL_HANDLE;

//Descriptor pool
VkDescriptorPool vkDescriptorPool = VK_NULL_HANDLE;

//Descriptor set
VkDescriptorSet vkDescriptorSet = VK_NULL_HANDLE;

//Pipeline
VkViewport vkViewport;
VkRect2D vkRect2D_scissor; //mostly viewport and scissor dimensions are same
VkPipeline vkPipeline = VK_NULL_HANDLE;

float angle = 0.0f;

//Texture related global data
VkImage vkImage_texture = VK_NULL_HANDLE;
VkDeviceMemory vkDeviceMemory_texture = VK_NULL_HANDLE;
VkImageView vkImageView_texture = VK_NULL_HANDLE;
VkSampler vkSampler_texture = VK_NULL_HANDLE;

uint32_t gSphereVertexCount = 0; // set in createVertexBuffer(), used in buildCommandBuffers()

// --- Mars textures (albedo, normal, height) ---
VkImage        vkImage_albedo        = VK_NULL_HANDLE;
VkDeviceMemory vkDeviceMemory_albedo = VK_NULL_HANDLE;
VkImageView    vkImageView_albedo    = VK_NULL_HANDLE;
VkSampler      vkSampler_albedo      = VK_NULL_HANDLE;

VkImage        vkImage_normal        = VK_NULL_HANDLE;
VkDeviceMemory vkDeviceMemory_normal = VK_NULL_HANDLE;
VkImageView    vkImageView_normal    = VK_NULL_HANDLE;
VkSampler      vkSampler_normal      = VK_NULL_HANDLE;

VkImage        vkImage_height        = VK_NULL_HANDLE;
VkDeviceMemory vkDeviceMemory_height = VK_NULL_HANDLE;
VkImageView    vkImageView_height    = VK_NULL_HANDLE;
VkSampler      vkSampler_height      = VK_NULL_HANDLE;

// Load a single-channel 16-bit image as VK_FORMAT_R16_UNORM (ideal for height/displacement).
VkResult createTexture2D_R16(const char* filename,
                             VkImage* outImage,
                             VkDeviceMemory* outMemory,
                             VkImageView* outView,
                             VkSampler* outSampler)
{
    VkResult vkResult = VK_SUCCESS;

    // --- Load 16-bit grayscale with stb_image (path-based) ---
    int w = 0, h = 0, ch = 0;
    unsigned short* pixels16 = stbi_load_16(filename, &w, &h, &ch, 1); // force 1 channel (R16)
    if (!pixels16 || w <= 0 || h <= 0) {
        fprintf(gpFile, "createTexture2D_R16() -> stbi_load_16 failed: %s\n", filename);
        if (pixels16) stbi_image_free(pixels16);
        return VK_ERROR_INITIALIZATION_FAILED;
    }
    VkDeviceSize image_size = (VkDeviceSize)w * (VkDeviceSize)h * 2; // 2 bytes per texel

    // --- Staging buffer (HOST_VISIBLE | HOST_COHERENT) ---
    VkBuffer staging = VK_NULL_HANDLE;
    VkDeviceMemory stagingMem = VK_NULL_HANDLE;

    VkBufferCreateInfo bci{}; bci.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bci.size = image_size;
    bci.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    vkResult = vkCreateBuffer(vkDevice, &bci, NULL, &staging);
    if (vkResult != VK_SUCCESS) { stbi_image_free(pixels16); return vkResult; }

    VkMemoryRequirements req{};
    vkGetBufferMemoryRequirements(vkDevice, staging, &req);

    VkMemoryAllocateInfo mai{}; mai.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    mai.allocationSize = req.size;
    for (uint32_t i = 0; i < vkPhysicalDeviceMemoryProperties.memoryTypeCount; ++i) {
        if ((req.memoryTypeBits & 1) == 1) {
            VkMemoryPropertyFlags f = vkPhysicalDeviceMemoryProperties.memoryTypes[i].propertyFlags;
            if ((f & (VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)) ==
                 (VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)) {
                mai.memoryTypeIndex = i; break;
            }
        }
        req.memoryTypeBits >>= 1;
    }
    vkResult = vkAllocateMemory(vkDevice, &mai, NULL, &stagingMem);
    if (vkResult != VK_SUCCESS) { vkDestroyBuffer(vkDevice, staging, NULL); stbi_image_free(pixels16); return vkResult; }
    vkBindBufferMemory(vkDevice, staging, stagingMem, 0);

    void* map = nullptr;
    vkResult = vkMapMemory(vkDevice, stagingMem, 0, image_size, 0, &map);
    if (vkResult != VK_SUCCESS) { vkDestroyBuffer(vkDevice, staging, NULL); vkFreeMemory(vkDevice, stagingMem, NULL); stbi_image_free(pixels16); return vkResult; }
    memcpy(map, pixels16, (size_t)image_size);
    vkUnmapMemory(vkDevice, stagingMem);
    stbi_image_free(pixels16);

    // --- Device-local R16 image ---
    VkImageCreateInfo ici{}; ici.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    ici.imageType = VK_IMAGE_TYPE_2D;
    ici.format = VK_FORMAT_R16_UNORM;
    ici.extent = { (uint32_t)w, (uint32_t)h, 1 };
    ici.mipLevels = 1; ici.arrayLayers = 1;
    ici.samples = VK_SAMPLE_COUNT_1_BIT;
    ici.tiling = VK_IMAGE_TILING_OPTIMAL;
    ici.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    ici.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    ici.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

    vkResult = vkCreateImage(vkDevice, &ici, NULL, outImage);
    if (vkResult != VK_SUCCESS) { vkDestroyBuffer(vkDevice, staging, NULL); vkFreeMemory(vkDevice, stagingMem, NULL); return vkResult; }

    VkMemoryRequirements imgReq{};
    vkGetImageMemoryRequirements(vkDevice, *outImage, &imgReq);

    VkMemoryAllocateInfo imgAlloc{}; imgAlloc.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    imgAlloc.allocationSize = imgReq.size;
    for (uint32_t i = 0; i < vkPhysicalDeviceMemoryProperties.memoryTypeCount; ++i) {
        if ((imgReq.memoryTypeBits & 1) == 1) {
            VkMemoryPropertyFlags f = vkPhysicalDeviceMemoryProperties.memoryTypes[i].propertyFlags;
            if (f & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) { imgAlloc.memoryTypeIndex = i; break; }
        }
        imgReq.memoryTypeBits >>= 1;
    }
    vkResult = vkAllocateMemory(vkDevice, &imgAlloc, NULL, outMemory);
    if (vkResult != VK_SUCCESS) { vkDestroyImage(vkDevice, *outImage, NULL); vkDestroyBuffer(vkDevice, staging, NULL); vkFreeMemory(vkDevice, stagingMem, NULL); return vkResult; }
    vkBindImageMemory(vkDevice, *outImage, *outMemory, 0);

    // --- One-time command buffer: transitions + copy ---
    VkCommandBufferAllocateInfo cai{}; cai.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    cai.commandPool = vkCommandPool; cai.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY; cai.commandBufferCount = 1;
    VkCommandBuffer cmd = VK_NULL_HANDLE;
    vkResult = vkAllocateCommandBuffers(vkDevice, &cai, &cmd);
    if (vkResult != VK_SUCCESS) { return vkResult; }

    VkCommandBufferBeginInfo cbi{}; cbi.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    cbi.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    vkBeginCommandBuffer(cmd, &cbi);

    // UNDEFINED -> TRANSFER_DST
    VkImageMemoryBarrier toDst{}; toDst.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    toDst.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    toDst.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    toDst.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    toDst.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    toDst.image = *outImage;
    toDst.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    toDst.subresourceRange.baseMipLevel = 0;
    toDst.subresourceRange.levelCount = 1;
    toDst.subresourceRange.baseArrayLayer = 0;
    toDst.subresourceRange.layerCount = 1;
    toDst.srcAccessMask = 0;
    toDst.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    vkCmdPipelineBarrier(cmd,
        VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
        0, 0, NULL, 0, NULL, 1, &toDst);

    // Copy buffer -> image
    VkBufferImageCopy bic{}; 
    bic.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    bic.imageSubresource.mipLevel = 0;
    bic.imageSubresource.baseArrayLayer = 0;
    bic.imageSubresource.layerCount = 1;
    bic.imageOffset = {0,0,0};
    bic.imageExtent = { (uint32_t)w, (uint32_t)h, 1 };
    vkCmdCopyBufferToImage(cmd, staging, *outImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &bic);

    // TRANSFER_DST -> SHADER_READ_ONLY
    VkImageMemoryBarrier toRead{}; toRead.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    toRead.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    toRead.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    toRead.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    toRead.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    toRead.image = *outImage;
    toRead.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    toRead.subresourceRange.levelCount = 1;
    toRead.subresourceRange.layerCount = 1;
    toRead.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    toRead.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
    vkCmdPipelineBarrier(cmd,
        VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
        0, 0, NULL, 0, NULL, 1, &toRead);

    vkEndCommandBuffer(cmd);
    VkSubmitInfo si{}; si.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO; si.commandBufferCount = 1; si.pCommandBuffers = &cmd;
    vkQueueSubmit(vkQueue, 1, &si, VK_NULL_HANDLE);
    vkQueueWaitIdle(vkQueue);
    vkFreeCommandBuffers(vkDevice, vkCommandPool, 1, &cmd);

    // --- View (R16_UNORM) ---
    VkImageViewCreateInfo vci{}; vci.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    vci.image = *outImage;
    vci.viewType = VK_IMAGE_VIEW_TYPE_2D;
    vci.format = VK_FORMAT_R16_UNORM;
    vci.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    vci.subresourceRange.baseMipLevel = 0;
    vci.subresourceRange.levelCount = 1;
    vci.subresourceRange.baseArrayLayer = 0;
    vci.subresourceRange.layerCount = 1;
    vkResult = vkCreateImageView(vkDevice, &vci, NULL, outView);
    if (vkResult != VK_SUCCESS) { return vkResult; }

    // --- Sampler (single mip level) ---
    VkSamplerCreateInfo sci{}; sci.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    sci.magFilter = VK_FILTER_LINEAR; sci.minFilter = VK_FILTER_LINEAR;
    sci.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
    sci.addressModeU = sci.addressModeV = sci.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    sci.anisotropyEnable = VK_FALSE; sci.unnormalizedCoordinates = VK_FALSE;
    vkResult = vkCreateSampler(vkDevice, &sci, NULL, outSampler);
    if (vkResult != VK_SUCCESS) { return vkResult; }

    // Cleanup staging
    vkDestroyBuffer(vkDevice, staging, NULL);
    vkFreeMemory(vkDevice, stagingMem, NULL);
    return VK_SUCCESS;
}

// Create a 2D sampled image (no mipmaps) from file into out* resources.
// format should be VK_FORMAT_R8G8B8A8_SRGB for albedo, VK_FORMAT_R8G8B8A8_UNORM for normal/height.
VkResult createTexture2D(const char* filename,
                         VkFormat format,
                         VkImage* outImage,
                         VkDeviceMemory* outMemory,
                         VkImageView* outView,
                         VkSampler* outSampler)
{
    VkResult vkResult = VK_SUCCESS;

    // --- Load with stb_image as RGBA ---
    FILE* fp = fopen(filename, "rb");
    if (!fp) { fprintf(gpFile, "createTexture2D() -> fopen failed: %s\n", filename); return VK_ERROR_INITIALIZATION_FAILED; }

    int w = 0, h = 0, ch = 0;
    uint8_t* pixels = stbi_load_from_file(fp, &w, &h, &ch, STBI_rgb_alpha);
    fclose(fp); fp = NULL;

    if (!pixels || w <= 0 || h <= 0) {
        fprintf(gpFile, "createTexture2D() -> stb load failed: %s\n", filename);
        if (pixels) stbi_image_free(pixels);
        return VK_ERROR_INITIALIZATION_FAILED;
    }

    VkDeviceSize image_size = (VkDeviceSize)w * (VkDeviceSize)h * 4;

    // --- Staging buffer ---
    VkBuffer staging = VK_NULL_HANDLE; VkDeviceMemory stagingMem = VK_NULL_HANDLE;

    VkBufferCreateInfo bci{}; bci.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bci.size = image_size; bci.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    vkResult = vkCreateBuffer(vkDevice, &bci, NULL, &staging); if (vkResult) return vkResult;

    VkMemoryRequirements req{}; vkGetBufferMemoryRequirements(vkDevice, staging, &req);

    VkMemoryAllocateInfo mai{}; mai.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    mai.allocationSize = req.size;
    for (uint32_t i = 0; i < vkPhysicalDeviceMemoryProperties.memoryTypeCount; ++i) {
        if ((req.memoryTypeBits & 1) == 1) {
            if ((vkPhysicalDeviceMemoryProperties.memoryTypes[i].propertyFlags &
                 (VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)) ==
                 (VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)) {
                mai.memoryTypeIndex = i; break;
            }
        }
        req.memoryTypeBits >>= 1;
    }
    vkResult = vkAllocateMemory(vkDevice, &mai, NULL, &stagingMem); if (vkResult) return vkResult;
    vkBindBufferMemory(vkDevice, staging, stagingMem, 0);

    void* map = nullptr;
    vkMapMemory(vkDevice, stagingMem, 0, image_size, 0, &map);
    memcpy(map, pixels, (size_t)image_size);
    vkUnmapMemory(vkDevice, stagingMem);

    stbi_image_free(pixels);

    // --- Device-local image ---
    VkImageCreateInfo ici{}; ici.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    ici.imageType = VK_IMAGE_TYPE_2D; ici.format = format;
    ici.extent = { (uint32_t)w, (uint32_t)h, 1 };
    ici.mipLevels = 1; ici.arrayLayers = 1;
    ici.samples = VK_SAMPLE_COUNT_1_BIT;
    ici.tiling = VK_IMAGE_TILING_OPTIMAL;
    ici.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    ici.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    ici.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

    vkResult = vkCreateImage(vkDevice, &ici, NULL, outImage); if (vkResult) return vkResult;

    VkMemoryRequirements imgReq{}; vkGetImageMemoryRequirements(vkDevice, *outImage, &imgReq);
    VkMemoryAllocateInfo imgAlloc{}; imgAlloc.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    imgAlloc.allocationSize = imgReq.size;
    for (uint32_t i = 0; i < vkPhysicalDeviceMemoryProperties.memoryTypeCount; ++i) {
        if ((imgReq.memoryTypeBits & 1) == 1) {
            if (vkPhysicalDeviceMemoryProperties.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) {
                imgAlloc.memoryTypeIndex = i; break;
            }
        }
        imgReq.memoryTypeBits >>= 1;
    }
    vkResult = vkAllocateMemory(vkDevice, &imgAlloc, NULL, outMemory); if (vkResult) return vkResult;
    vkBindImageMemory(vkDevice, *outImage, *outMemory, 0);

    // --- One-time command buffer ---
    VkCommandBufferAllocateInfo cai{}; cai.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    cai.commandPool = vkCommandPool; cai.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY; cai.commandBufferCount = 1;

    VkCommandBuffer cmd = VK_NULL_HANDLE;
    vkAllocateCommandBuffers(vkDevice, &cai, &cmd);

    VkCommandBufferBeginInfo cbi{}; cbi.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    cbi.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    vkBeginCommandBuffer(cmd, &cbi);

    // Layout: UNDEFINED -> TRANSFER_DST
    VkImageMemoryBarrier toDst{}; toDst.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    toDst.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    toDst.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    toDst.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    toDst.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    toDst.image = *outImage;
    toDst.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    toDst.subresourceRange.levelCount = 1; toDst.subresourceRange.layerCount = 1;
    toDst.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

    vkCmdPipelineBarrier(cmd,
        VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
        0, 0, NULL, 0, NULL, 1, &toDst);

    // Copy buffer->image
    VkBufferImageCopy bic{}; bic.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    bic.imageSubresource.layerCount = 1; bic.imageExtent = { (uint32_t)w, (uint32_t)h, 1 };
    vkCmdCopyBufferToImage(cmd, staging, *outImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &bic);

    // TRANSFER_DST -> SHADER_READ_ONLY
    VkImageMemoryBarrier toRead{}; toRead.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    toRead.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    toRead.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    toRead.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    toRead.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    toRead.image = *outImage;
    toRead.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    toRead.subresourceRange.levelCount = 1; toRead.subresourceRange.layerCount = 1;
    toRead.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    toRead.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

    vkCmdPipelineBarrier(cmd,
        VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
        0, 0, NULL, 0, NULL, 1, &toRead);

    vkEndCommandBuffer(cmd);

    VkSubmitInfo si{}; si.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO; si.commandBufferCount = 1; si.pCommandBuffers = &cmd;
    vkQueueSubmit(vkQueue, 1, &si, VK_NULL_HANDLE);
    vkQueueWaitIdle(vkQueue);
    vkFreeCommandBuffers(vkDevice, vkCommandPool, 1, &cmd);

    // --- View ---
    VkImageViewCreateInfo vci{}; vci.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    vci.viewType = VK_IMAGE_VIEW_TYPE_2D; vci.format = format; vci.image = *outImage;
    vci.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    vci.subresourceRange.levelCount = 1; vci.subresourceRange.layerCount = 1;

    vkResult = vkCreateImageView(vkDevice, &vci, NULL, outView); if (vkResult) return vkResult;

    // --- Sampler ---
    VkSamplerCreateInfo sci{}; sci.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    sci.magFilter = VK_FILTER_LINEAR; sci.minFilter = VK_FILTER_LINEAR;
    sci.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR; // 1 mip; textureLod(level 0) in VS for height
    sci.addressModeU = sci.addressModeV = sci.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    sci.anisotropyEnable = VK_FALSE; sci.unnormalizedCoordinates = VK_FALSE;
    vkResult = vkCreateSampler(vkDevice, &sci, NULL, outSampler);

    // cleanup staging
    vkDestroyBuffer(vkDevice, staging, NULL);
    vkFreeMemory(vkDevice, stagingMem, NULL);
    return vkResult;
}

VkResult createMarsTextures(const char* albedoFile,
                            const char* normalFile,
                            const char* heightFile)
{
    VkResult r = VK_SUCCESS;

    // Albedo in sRGB
    r = createTexture2D(albedoFile, VK_FORMAT_R8G8B8A8_SRGB,
                        &vkImage_albedo, &vkDeviceMemory_albedo, &vkImageView_albedo, &vkSampler_albedo);
    if (r != VK_SUCCESS) return r;

    // Normal in UNORM
    r = createTexture2D(normalFile, VK_FORMAT_R8G8B8A8_UNORM,
                        &vkImage_normal, &vkDeviceMemory_normal, &vkImageView_normal, &vkSampler_normal);
    if (r != VK_SUCCESS) return r;

    // Height: true 16-bit R16_UNORM
    r = createTexture2D_R16(heightFile,
                            &vkImage_height, &vkDeviceMemory_height, &vkImageView_height, &vkSampler_height);
    return r;
}

//WinMain
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdLine, int iCmdShow)
{
	//function declarations
	VkResult Initialize(void);
	void Uninitialize(void);
	VkResult Display(void);

	//local variable declarations
	WNDCLASSEX wndclass;
	HWND hwnd;
	MSG msg;
	TCHAR szAppName[255];
	int bDone = 0;
    VkResult vkResult = VK_SUCCESS;


	//code
    gpFile = fopen("SSA_Log.txt", "w+");
	if (gpFile == NULL)
	{
		MessageBox(NULL, TEXT("Cannot create/open SSA_Log.txt file"), TEXT("FILE IO ERROR"), MB_ICONERROR);
		exit(0);
	}
	else
	{
		fprintf(gpFile, "WinMain() --> Program started successfully\n");
	}

    wsprintf(szAppName, TEXT("%s"), gpszAppName);

	wndclass.cbSize = sizeof(WNDCLASSEX);
	wndclass.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	wndclass.cbClsExtra = 0;
	wndclass.cbWndExtra = 0;
	wndclass.lpfnWndProc = WndProc;
	wndclass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	wndclass.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(MYICON));
	wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);
	wndclass.hInstance = hInstance;
	wndclass.lpszMenuName = NULL;
	wndclass.lpszClassName = szAppName;
	wndclass.hIconSm = LoadIcon(hInstance, MAKEINTRESOURCE(MYICON));

	RegisterClassEx(&wndclass);

	hwnd = CreateWindowEx(WS_EX_APPWINDOW,
		szAppName,
		TEXT("SSA_VULKAN"),
		WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | WS_VISIBLE,
		GetSystemMetrics(SM_CXSCREEN)/2 - WIN_WIDTH/2,
		GetSystemMetrics(SM_CYSCREEN)/2 - WIN_HEIGHT/2,
		WIN_WIDTH,
		WIN_HEIGHT,
		NULL, //Parent Window Handle
		NULL, //Menu Handle
		hInstance,
		NULL);

	ghwnd = hwnd;

	vkResult = Initialize();
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "WinMain() --> Initialize() is failed\n");
        DestroyWindow(hwnd);
        hwnd = NULL;
    }
    else
    {
        fprintf(gpFile, "WinMain() --> Initialize() is succedded\n");
    }
    //Error checking of Initialize

	ShowWindow(hwnd, iCmdShow);
    Update();
	SetForegroundWindow(hwnd);
	SetFocus(hwnd);

	//Game loop
	while (bDone == 0)
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			if (msg.message == WM_QUIT)
				bDone = 1;
			else
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}
		else
		{
			if (gbActiveWindow == 1)
			{
				//Here you should call update() for OpenGL rendering
                Update();
				//Here you should call display() for OpenGL rendering
                if(gbWindowMinimized == FALSE)
                {
                    vkResult = Display();
                    if((vkResult != VK_FALSE) && (vkResult != VK_SUCCESS) && (vkResult != VK_ERROR_OUT_OF_DATE_KHR) && (vkResult != VK_SUBOPTIMAL_KHR))
                    {
                        fprintf(gpFile, "WinMain() --> call to Display() failed\n");
                        bDone = TRUE;
                    }
                }
			}
		}
	}

	Uninitialize();

	return((int)(msg.wParam));
}


LRESULT CALLBACK WndProc(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
	//Local function declaration
	void ToggleFullscreen(void);
	VkResult Resize(int, int);
	void Uninitialize(void);

	int height = 0;
    int width = 0;
    int length = sizeof(WINDOWPLACEMENT);
    
    switch (iMsg)
	{

    case WM_CREATE:
        memset(&wpPrev, 0, length);
        break;

	case WM_SETFOCUS:
		gbActiveWindow = 1;
		break;

	case WM_KILLFOCUS:
		gbActiveWindow = 0;
		break;

	case WM_ERASEBKGND:
		return(0);

	case WM_SIZE:
        if(wParam == SIZE_MINIMIZED)
        {
            gbWindowMinimized = TRUE;
        }
        else
        {
            gbWindowMinimized = FALSE;
            Resize(LOWORD(lParam), HIWORD(lParam));
        }
        
		break;

	case WM_KEYDOWN:
		switch (wParam)
		{
		case VK_ESCAPE:
			DestroyWindow(hwnd);
			break;

		case 0x46:
		case 0x66:
			ToggleFullscreen();
			break;

		default:
			break;
		}
		break;

	case WM_CLOSE:
		DestroyWindow(hwnd);
		break;

	case WM_DESTROY:
		Uninitialize();
		PostQuitMessage(0);
		break;

	default: break;
	}

	return(DefWindowProc(hwnd, iMsg, wParam, lParam));
}


void ToggleFullscreen(void)
{
	//Local variable declaration
	MONITORINFO mi = { sizeof(MONITORINFO) };

	//Code
	if (gbFullscreen == 0) //If current window is normal(Not Fullscreen)
	{
		dwStyle = GetWindowLong(ghwnd, GWL_STYLE); //dwStyle will get WS_OVERLAPPEDWINDOW information
		if (dwStyle & WS_OVERLAPPEDWINDOW)
		{
			if (GetWindowPlacement(ghwnd, &wpPrev) && GetMonitorInfo(MonitorFromWindow(ghwnd, MONITORINFOF_PRIMARY), &mi))
			{
				SetWindowLong(ghwnd, GWL_STYLE, (dwStyle & ~WS_OVERLAPPEDWINDOW)); //Removing flags of WS_OVERLAPPEDWINDOW
				SetWindowPos(ghwnd,
					HWND_TOP, //HWND_TOP -> for making it on top order
					mi.rcMonitor.left, // left coordinate of monitor
					mi.rcMonitor.top, // top coordinate of monitor
					mi.rcMonitor.right - mi.rcMonitor.left, // width coordinate of monitor
					mi.rcMonitor.bottom - mi.rcMonitor.top, // height coordinate of monitor
					SWP_NOZORDER | //Window flag --> don't change the Z order
					SWP_FRAMECHANGED); //Window flag --> WM_NCCALCSIZE (Window message calculate Non Client area)
			}
		}
		ShowCursor(FALSE);  //disappear the cursor in full screen or Game mode
		gbFullscreen = 1;
	}
	else
	{
		SetWindowLong(ghwnd, GWL_STYLE, (dwStyle | WS_OVERLAPPEDWINDOW));//Restoring flags of WS_OVERLAPPEDWINDOW
		SetWindowPlacement(ghwnd, &wpPrev); //setting placement of window
		SetWindowPos(ghwnd,
			HWND_TOP,
			0,  //already set in wpPrev
			0,  //already set in wpPrev
			0,  //already set in wpPrev
			0,  //already set in wpPrev
			SWP_NOMOVE | // do not change x, y cordinates for starting position
			SWP_NOSIZE | // do not change height and width cordinates
			SWP_NOOWNERZORDER | //do not change the position even if its parent window is changed
			SWP_NOZORDER | //Window flag --> don't change the Z order
			SWP_FRAMECHANGED); //Window flag --> WM_NCCALCSIZE (Window message calculate Non Client area)
		ShowCursor(TRUE); //Appear the cursor in full screen or Game mode
		gbFullscreen = 0;
	}
}


VkResult Initialize(void)
{
    //function declaration
    VkResult createVulkanInstance(void);
    VkResult getSupportedSurface(void);
    VkResult getPhysicalDevice(void);
    VkResult printVkInfo(void);
    VkResult createVulkanDevice(void);
    void getDeviceQueue(void);
    VkResult createSwapchain(VkBool32);
    VkResult createImagesAndImageViews(void);
    VkResult createCommandPool(void);
    VkResult createCommandBuffers(void);
    VkResult createVertexBuffer(void);
    VkResult createTexture(const char*);
    VkResult createUniformBuffer(void);
    VkResult createShaders(void);
    VkResult createDescriptorSetLayout(void);
    VkResult createPipelineLayout(void);
    VkResult createDescriptorPool(void);
    VkResult createDescriptorSet(void);
    VkResult createRenderPass(void);
    VkResult createPipeline(void);
    VkResult createFrameBuffers(void);
    VkResult createSemaphores(void);
    VkResult createFences(void);
    VkResult buildCommandBuffers(void);
        
    //variable delaration
    VkResult vkResult = VK_SUCCESS;

    //code
    vkResult = createVulkanInstance();
    /*Main points:
        vkEnumerateInstanceExtensionProperties()
        
        struct --> vkApplicationInfo
        struct --> vkInstanceCreateInfo
        vkCreateInstance()
    */
    
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "Initialize() --> createVulkanInstance() is failed %d\n", vkResult);
        return vkResult;
    }
    else
    {
        fprintf(gpFile, "Initialize() --> createVulkanInstance() is succedded\n");
    }

    //CreateVulkan Presntation Surface
    vkResult = getSupportedSurface();
    /* Main points:
        struct --> vkWin32SurfaceCreateInfoKHR
        vkCreateWin32SurfaceKHR()
    */
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "Initialize() --> getSupportedSurface() is failed %d\n", vkResult);
        return vkResult;
    }
    else
    {
        fprintf(gpFile, "Initialize() --> getSupportedSurface() is succedded\n");
    }

    //Get required Physical Device and its Queue family index
    vkResult = getPhysicalDevice();
    /* Main points:
        vkEnumeratePhysicalDevices()
        vkGetPhysicalDeviceQueueFamilyProperties()
        vkGetPhysicalDeviceSurfaceSupportKHR()
        vkGetPhysicalDeviceMemoryProperties()
        vkGetPhysicalDeviceFeatures()
    */
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "Initialize() --> getPhysicalDevice() is failed %d\n", vkResult);
        return vkResult;
    }
    else
    {
        fprintf(gpFile, "Initialize() --> getPhysicalDevice() is succedded\n");
    }
    
    //Printing Vulkan Infomration
    vkResult = printVkInfo();
    /* Main points:
        vkGetPhysicalDeviceProperties()
    */
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "Initialize() --> printVkInfo() is failed %d\n", vkResult);
        return vkResult;
    }
    else
    {
        fprintf(gpFile, "Initialize() --> printVkInfo() is succedded\n");
    }
    
    vkResult = createVulkanDevice();
    /* Main Points:
        vkEnumerateDeviceExtensionProperties()
        struct --> vkDeviceQueueCreateInfo
        struct --> vkDeviceCreateInfo
        vkCreateDevice()
        
    */
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "Initialize() --> createVulkanDevice() is failed %d\n", vkResult);
        return vkResult;
    }
    else
    {
        fprintf(gpFile, "Initialize() --> createVulkanDevice() is succedded\n");
    }
    
    //getDeviceQueue
    getDeviceQueue();
    /* Main Points
        vkGetDeviceQueue()
    */

        
    //Swapchain
    vkResult = createSwapchain(VK_FALSE);
    /*Main Points:
        vkGetPhysicalDeviceSurfaceFormatsKHR()
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR()
        vkGetPhysicalDeviceSurfacePresentModesKHR()
        
    */
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "Initialize() --> createSwapchain() is failed %d\n", vkResult);
        return VK_ERROR_INITIALIZATION_FAILED;
    }
    else
    {
        fprintf(gpFile, "Initialize() --> createSwapchain() is succedded\n");
    }
    
    //Create Vulkan Images and ImageViews
    vkResult = createImagesAndImageViews();
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "Initialize() --> createImagesAndImageViews() is failed %d\n", vkResult);
    }
    else
    {
        fprintf(gpFile, "Initialize() --> createImagesAndImageViews() is succedded\n");
    }
    
    //Create Command Pool
    vkResult = createCommandPool();
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "Initialize() --> createCommandPool() is failed %d\n", vkResult);
    }
    else
    {
        fprintf(gpFile, "Initialize() --> createCommandPool() is succedded\n");
    }
    
    //Command Buffers
    vkResult = createCommandBuffers();
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "Initialize() --> createCommandBuffers() is failed %d\n", vkResult);
    }
    else
    {
        fprintf(gpFile, "Initialize() --> createCommandBuffers() is succedded\n");
    }
    
    //create VertexBUffer
    vkResult = createVertexBuffer();
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "Initialize() --> createVertexBuffer() is failed %d\n", vkResult);
    }
    else
    {
        fprintf(gpFile, "Initialize() --> createVertexBuffer() is succedded\n");
    }
    
    //create texture
    vkResult = createMarsTextures("Uranus_Color_Stylized_8k.png", "Uranus_Normal_Banded_8k.png", "Uranus_Displacement_Banded16_8k.png");
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "Initialize() --> createMarsTextures() failed %d\n", vkResult);
	} 
	else
	{
		fprintf(gpFile, "Initialize() --> createMarsTextures() succeeded\n");
	}
    
    //create Uniform Buffer
    vkResult = createUniformBuffer();
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "Initialize() --> createUniformBuffer() is failed %d\n", vkResult);
    }
    else
    {
        fprintf(gpFile, "Initialize() --> createUniformBuffer() is succedded\n");
    }
    
    //Create RenderPass
    vkResult = createShaders();
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "Initialize() --> createShaders() is failed %d\n", vkResult);
    }
    else
    {
        fprintf(gpFile, "Initialize() --> createShaders() is succedded\n");
    }
    
    
    
    //Create Dewscriptor Set Layout
    vkResult = createDescriptorSetLayout();
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "Initialize() --> createDescriptorSetLayout() is failed %d\n", vkResult);
    }
    else
    {
        fprintf(gpFile, "Initialize() --> createDescriptorSetLayout() is succedded\n");
    }
    
    //Create Descriptor Set Layout
    vkResult = createPipelineLayout();
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "Initialize() --> createPipelineLayout() is failed %d\n", vkResult);
    }
    else
    {
        fprintf(gpFile, "Initialize() --> createPipelineLayout() is succedded\n");
    }
    
    //create Descriptor pool
    vkResult = createDescriptorPool();
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "Initialize() --> createDescriptorPool() is failed %d\n", vkResult);
    }
    else
    {
        fprintf(gpFile, "Initialize() --> createDescriptorPool() is succedded\n");
    }
    
    //create Descriptor set
    vkResult = createDescriptorSet();
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "Initialize() --> createDescriptorSet() is failed %d\n", vkResult);
    }
    else
    {
        fprintf(gpFile, "Initialize() --> createDescriptorPool() is succedded\n");
    }
    
    //Create RenderPass
    vkResult = createRenderPass();
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "Initialize() --> createRenderPass() is failed %d\n", vkResult);
    }
    else
    {
        fprintf(gpFile, "Initialize() --> createRenderPass() is succedded\n");
    }
    
    //Create Pipeline
    vkResult = createPipeline();
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "Initialize() --> createPipeline() is failed %d\n", vkResult);
    }
    else
    {
        fprintf(gpFile, "Initialize() --> createPipeline() is succedded\n");
    }
    
    
    //CreateBuffer
    vkResult = createFrameBuffers();
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "Initialize() --> createFrameBuffers() is failed %d\n", vkResult);
    }
    else
    {
        fprintf(gpFile, "Initialize() --> createFrameBuffers() is succedded\n");
    }
    
    //CreateSemaphores
    vkResult = createSemaphores();
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "Initialize() --> createSemaphores() is failed %d\n", vkResult);
    }
    else
    {
        fprintf(gpFile, "Initialize() --> createSemaphores() is succedded\n");
    }
    
    //CreateFences
    vkResult = createFences();
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "Initialize() --> createFences() is failed %d\n", vkResult);
    }
    else
    {
        fprintf(gpFile, "Initialize() --> createFences() is succedded\n");
    }
    
    //Initlaize vkClearColorValue
    memset((void*)&vkClearColorValue, 0, sizeof(VkClearColorValue));
    vkClearColorValue.float32[0] = 0.0f;// R
    vkClearColorValue.float32[1] = 0.0f;// G
    vkClearColorValue.float32[2] = 0.0f;// B
    vkClearColorValue.float32[3] = 1.0f;//Analogous to glClearColor()
    
    memset((void*)&vkClearDepthStencilValue, 0, sizeof(VkClearDepthStencilValue));
    //setDefaultClearDepth
    vkClearDepthStencilValue.depth = 1.0f; //flaot value
    //setDeafaultStencilValue
    vkClearDepthStencilValue.stencil = 0; //uint32_t value
    
    //BuildCommandBuffers
    vkResult = buildCommandBuffers();
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "Initialize() --> buildCommandBuffers() is failed %d\n", vkResult);
    }
    else
    {
        fprintf(gpFile, "Initialize() --> buildCommandBuffers() is succedded\n");
    }
    
    //Initialization is completed
    bInitialized = TRUE;
    fprintf(gpFile, "Initialize() --> Initialization is completed successfully\n");
  
    return vkResult;
}


VkResult Resize(int width, int height)
{
    //Function declarations
    VkResult createSwapchain(VkBool32);
    VkResult createImagesAndImageViews(void);
    VkResult createCommandBuffers(void);
    VkResult createPipelineLayout(void);
    VkResult createPipeline(void);
    VkResult createRenderPass(void);
    VkResult createFrameBuffers(void);
    VkResult buildCommandBuffers(void);
    
    
    //variables
    VkResult vkResult = VK_SUCCESS;
	//code
	if (height == 0)
		height = 1;
    
    giHeight = height;
    giWidth = width;
    

    //check the bInitialized variable
    if(bInitialized == FALSE)
    {
        fprintf(gpFile, "Resize() --> Initialization yet not completed or failed\n");  
        vkResult = VK_ERROR_INITIALIZATION_FAILED;
        return vkResult;
    }
    
    //As recreation of swapchain is needed we are goring to repeat many steps of initialized again, hence set bINitialized to \False again
    bInitialized = FALSE;

    //Set gloabl winwidth and winheight variable
    winWidth = width;
    winHeight = height;
    
    //wait for device to complete inhand tasks
    vkDeviceWaitIdle(vkDevice);
    fprintf(gpFile, "Resize() --> vkDeviceWaitIdle() is done\n");
    
    //check presence of swapchain
    if(vkSwapchainKHR == VK_NULL_HANDLE)
    {
        fprintf(gpFile, "Resize() --> swapchain is already NULL cannot proceed\n");
        vkResult = VK_ERROR_INITIALIZATION_FAILED;
    }
    
    //Destroy vkframebuffer
    for(uint32_t i = 0; i < swapchainImageCount ; i++)
    {
        vkDestroyFramebuffer(vkDevice, vkFrameBuffer_array[i], NULL);
    }
    if(vkFrameBuffer_array)
    {
        free(vkFrameBuffer_array);
        vkFrameBuffer_array = NULL;
        fprintf(gpFile, "Resize() --> vkFrameBuffer_array() is done\n");
    }
    
    //Destroy Commandbuffer
    for(uint32_t i = 0; i < swapchainImageCount; i++)
    {
        vkFreeCommandBuffers(vkDevice, vkCommandPool, 1, &vkCommandBuffer_array[i]);
        fprintf(gpFile, "Resize() --> vkFreeCommandBuffers() is done\n");
    }
    
    if(vkCommandBuffer_array)
    {
        free(vkCommandBuffer_array);
        vkCommandBuffer_array = NULL;
        fprintf(gpFile, "Resize() --> vkCommandBuffer_array is freed\n");
    }
    
    
    //Destroy Pipeline
    if(vkPipeline)
    {
        vkDestroyPipeline(vkDevice, vkPipeline, NULL);
        vkPipeline = VK_NULL_HANDLE;
        fprintf(gpFile, "Resize() --> vkDestroyPipeline() is done\n");
    }
    
    //Destroy vkPipelineLayout
    if(vkPipelineLayout)
    {
        vkDestroyPipelineLayout(vkDevice, vkPipelineLayout, NULL);
        vkPipelineLayout = VK_NULL_HANDLE;
        fprintf(gpFile, "Resize() --> vkPipelineLayout() is done\n");
    }
    
    //Destroy Renderpass
    if(vkRenderPass)
    {
        vkDestroyRenderPass(vkDevice, vkRenderPass, NULL);
        vkRenderPass = VK_NULL_HANDLE;
    }
    
    //destroy depth image view
    if(vkImageView_depth)
    {
        vkDestroyImageView(vkDevice, vkImageView_depth, NULL);
        vkImageView_depth = VK_NULL_HANDLE;
    }
    
    //destroy device memory for depth image
    if(vkDeviceMemory_depth)
    {
        vkFreeMemory(vkDevice, vkDeviceMemory_depth, NULL);
        vkDeviceMemory_depth = VK_NULL_HANDLE;
    }
    
    if(vkImage_depth)
    {
        vkDestroyImage(vkDevice, vkImage_depth, NULL);
        vkImage_depth = VK_NULL_HANDLE;
    }
    //destory image views
    for(uint32_t i = 0; i < swapchainImageCount; i++)
    {
        vkDestroyImageView(vkDevice, swapchainImageView_array[i], NULL);
        fprintf(gpFile, "Resize() --> vkDestoryImageView() is done\n");
    }
    
    if(swapchainImageView_array)
    {
        free(swapchainImageView_array);
        swapchainImageView_array = NULL;
        fprintf(gpFile, "Resize() --> swapchainImageView_array is freed\n");
    }
    
    //free swapchainImages
    // for(uint32_t i = 0; i < swapchainImageCount; i++)
    // {
        // vkDestroyImage(vkDevice, swapchainImage_array[i], NULL);
        // fprintf(gpFile, "Resize() --> vkDestroyImage() is done\n");
    // }
    
    
    if(swapchainImage_array)
    {
        free(swapchainImage_array);
        swapchainImage_array = NULL;
        fprintf(gpFile, "Resize() --> swapchainImage_array is freed\n");
    }
    
    //Destory Swapchain
    if(vkSwapchainKHR)
    {
        vkDestroySwapchainKHR(vkDevice, vkSwapchainKHR, NULL);
        vkSwapchainKHR = VK_NULL_HANDLE;
        fprintf(gpFile, "Resize() --> vkSwapchainCreateInfoKHR() is done\n");
    }
    
    //RECREATE FOR RESIZE
    
    //Swapchain
    vkResult = createSwapchain(VK_FALSE);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "Resize() --> createSwapchain() is failed %d\n", vkResult);
        return VK_ERROR_INITIALIZATION_FAILED;
    }
    
    //Create Vulkan Images and ImageViews
    vkResult = createImagesAndImageViews();
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "Resize() --> createImagesAndImageViews() is failed %d\n", vkResult);
    }
    
    //Create RenderPass
    vkResult = createRenderPass();
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "Resize() --> createRenderPass() is failed %d\n", vkResult);
    }
    
    //Create Descriptor Set Layout
    vkResult = createPipelineLayout();
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "Resize() --> createPipelineLayout() is failed %d\n", vkResult);
    }
    
    //Create Pipeline
    vkResult = createPipeline();
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "Resize() --> createPipeline() is failed %d\n", vkResult);
    }
      
    //CreateBuffer
    vkResult = createFrameBuffers();
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "Resize() --> createFrameBuffers() is failed %d\n", vkResult);
    }
    
    //Command Buffers
    vkResult = createCommandBuffers();
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "Resize() --> createCommandBuffers() is failed %d\n", vkResult);
    }
    
    //BuildCommandBuffers
    vkResult = buildCommandBuffers();
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "Resize() --> buildCommandBuffers() is failed %d\n", vkResult);
    }

    bInitialized = TRUE;

    return vkResult;
}


VkResult Display(void)
{
    //Function Declarations
    VkResult Resize(int, int);
    VkResult updateUniformBuffer(void);
    
    //variable declarations
    VkResult vkResult = VK_SUCCESS; 

    //code
    // if control comes here before initlaization is completed, return FALSE
    if(bInitialized == FALSE)
    {
        fprintf(gpFile, "Display() --> Initialization yet not completed\n");
        return (VkResult)VK_FALSE;
    }
    
    //acquire index of next swapchin image
    //if timour occurs, then function returns VK_NOT_READY
    vkResult = vkAcquireNextImageKHR(vkDevice,
                                     vkSwapchainKHR,
                                     UINT64_MAX, //waiting time in nanaoseconds for swapchain to get the image
                                     vkSemaphore_backBuffer, //semaphore, waiting for another queue to relaease the image held by another queue demanded by swapchain, (InterQueue semaphore)
                                     VK_NULL_HANDLE, //Fence, when you want to halt host also, for device::: (Use Semaphore and fences exclusively, using both is not recommended(Redbook)
                                     &currentImageIndex);
    if(vkResult != VK_SUCCESS)
    {
        if((vkResult == VK_ERROR_OUT_OF_DATE_KHR) || (vkResult == VK_SUBOPTIMAL_KHR))
        {
            Resize(winWidth, winHeight);
        }
        else
        {
            fprintf(gpFile, "Display() --> vkAcquireNextImageKHR() is failed errorcode = %d\n", vkResult);
            return vkResult;   
        }
    }
    
    //use fence to allow host to wait for completion of execution of previous commandbuffer
    vkResult = vkWaitForFences(vkDevice,
                               1, //waiting for how many fences
                               &vkFence_array[currentImageIndex], //Which fence
                               VK_TRUE, // wait till all fences get signalled(Blocking and unblocking function)
                               UINT64_MAX); //waiting time in nanaoseconds
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "Display() --> vkWaitForFences() is failed errorcode = %d\n", vkResult);
        return vkResult;
    }

    //Now make Fences execution of next command buffer
    vkResult = vkResetFences(vkDevice,
                             1, //How many fences to reset
                             &vkFence_array[currentImageIndex]);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "Display() --> vkResetFences() is failed errorcode = %d\n", vkResult);
        return vkResult;
    }
    
    //One of the mmeber of vkSubmitinfo structure requires array of pipeline stages, we have only one of the completion of color attachment output, still we need 1 member array
    const VkPipelineStageFlags waitDstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

    //Declare memset and initialize VkSubmitInfo structure
    VkSubmitInfo vkSubmitInfo;
    memset((void*)&vkSubmitInfo, 0, sizeof(VkSubmitInfo));
    
    vkSubmitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    vkSubmitInfo.pNext = NULL;
    vkSubmitInfo.pWaitDstStageMask = &waitDstStageMask;
    vkSubmitInfo.waitSemaphoreCount = 1;
    vkSubmitInfo.pWaitSemaphores = &vkSemaphore_backBuffer;
    vkSubmitInfo.commandBufferCount = 1;
    vkSubmitInfo.pCommandBuffers = &vkCommandBuffer_array[currentImageIndex];
    vkSubmitInfo.signalSemaphoreCount = 1;
    vkSubmitInfo.pSignalSemaphores = &vkSemaphore_renderComplete;
    
    //Now submit our work to the Queue
    vkResult = vkQueueSubmit(vkQueue,
                             1,
                             &vkSubmitInfo,
                             vkFence_array[currentImageIndex]);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "Display() --> vkQueueSubmit() is failed errorcode = %d\n", vkResult);
        return vkResult;
    }
    
    //We are going to present rendered image after declaring and initlaizing VkPresentInfoKHR structure
    VkPresentInfoKHR vkPresentInfoKHR;
    memset((void*)&vkPresentInfoKHR, 0, sizeof(VkPresentInfoKHR));
    
    vkPresentInfoKHR.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    vkPresentInfoKHR.pNext = NULL;
    vkPresentInfoKHR.swapchainCount = 1;
    vkPresentInfoKHR.pSwapchains = &vkSwapchainKHR;
    vkPresentInfoKHR.pImageIndices = &currentImageIndex;
    vkPresentInfoKHR.waitSemaphoreCount = 1;
    vkPresentInfoKHR.pWaitSemaphores = &vkSemaphore_renderComplete;
    
    //Now present the Queue
    vkResult = vkQueuePresentKHR(vkQueue, &vkPresentInfoKHR);
    if(vkResult != VK_SUCCESS)
    {
        if((vkResult == VK_ERROR_OUT_OF_DATE_KHR) || (vkResult == VK_SUBOPTIMAL_KHR))
        {
            Resize(winWidth, winHeight);
        }
        else
        {
            fprintf(gpFile, "Display() --> vkQueuePresentKHR() is failed errorcode = %d\n", vkResult);
            return vkResult;
        }
    }
    
    vkResult = updateUniformBuffer();
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "Display() --> updateUniformBuffer() is failed errorcode = %d\n", vkResult);
    }
        
    //validation
    vkDeviceWaitIdle(vkDevice);    
    
    return vkResult;
}
    

void Update(void)
{
    angle = angle + 0.01f;
}


void Uninitialize(void)
{
	//code
	if (gbFullscreen == 1)
	{
		dwStyle = GetWindowLong(ghwnd, GWL_STYLE);
		SetWindowLong(ghwnd, GWL_STYLE, (dwStyle | WS_OVERLAPPEDWINDOW));//Restoring flags of WS_OVERLAPPEDWINDOW
		SetWindowPlacement(ghwnd, &wpPrev); //setting placement of window
		SetWindowPos(ghwnd,
			HWND_TOP,
			0,  //already set in wpPrev
			0,  //already set in wpPrev
			0,  //already set in wpPrev
			0,  //already set in wpPrev
			SWP_NOMOVE | // do not change x, y cordinates for starting position
			SWP_NOSIZE | // do not change height and width cordinates
			SWP_NOOWNERZORDER | //do not change the position even if its parent window is changed
			SWP_NOZORDER | //Window flag --> don't change the Z order
			SWP_FRAMECHANGED); //Window flag --> WM_NCCALCSIZE (Window message calculate Non Client area)
		ShowCursor(TRUE); //Appear the cursor in full screen or Game mode
	}

	/**************Shader removal code*******************/


	if (ghdc)
	{
		ReleaseDC(ghwnd, ghdc);
		ghdc = NULL;
	}

    //synchronisation function
    vkDeviceWaitIdle(vkDevice);
    fprintf(gpFile, "Uninitialize() --> vkDeviceWaitIdle() is done\n");
	
	// Albedo
	if (vkSampler_albedo)      { vkDestroySampler(vkDevice, vkSampler_albedo, NULL); vkSampler_albedo = VK_NULL_HANDLE; }
	if (vkImageView_albedo)    { vkDestroyImageView(vkDevice, vkImageView_albedo, NULL); vkImageView_albedo = VK_NULL_HANDLE; }
	if (vkDeviceMemory_albedo) { vkFreeMemory(vkDevice, vkDeviceMemory_albedo, NULL); vkDeviceMemory_albedo = VK_NULL_HANDLE; }
	if (vkImage_albedo)        { vkDestroyImage(vkDevice, vkImage_albedo, NULL); vkImage_albedo = VK_NULL_HANDLE; }

	// Normal
	if (vkSampler_normal)      { vkDestroySampler(vkDevice, vkSampler_normal, NULL); vkSampler_normal = VK_NULL_HANDLE; }
	if (vkImageView_normal)    { vkDestroyImageView(vkDevice, vkImageView_normal, NULL); vkImageView_normal = VK_NULL_HANDLE; }
	if (vkDeviceMemory_normal) { vkFreeMemory(vkDevice, vkDeviceMemory_normal, NULL); vkDeviceMemory_normal = VK_NULL_HANDLE; }
	if (vkImage_normal)        { vkDestroyImage(vkDevice, vkImage_normal, NULL); vkImage_normal = VK_NULL_HANDLE; }

	// Height
	if (vkSampler_height)      { vkDestroySampler(vkDevice, vkSampler_height, NULL); vkSampler_height = VK_NULL_HANDLE; }
	if (vkImageView_height)    { vkDestroyImageView(vkDevice, vkImageView_height, NULL); vkImageView_height = VK_NULL_HANDLE; }
	if (vkDeviceMemory_height) { vkFreeMemory(vkDevice, vkDeviceMemory_height, NULL); vkDeviceMemory_height = VK_NULL_HANDLE; }
	if (vkImage_height)        { vkDestroyImage(vkDevice, vkImage_height, NULL); vkImage_height = VK_NULL_HANDLE; }
    
    //DestroyFences
    for(uint32_t i = 0; i < swapchainImageCount; i++)
    {
        vkDestroyFence(vkDevice, vkFence_array[i], NULL);
    }
    
    if(vkFence_array)
    {
        free(vkFence_array);
        vkFence_array = NULL;
        fprintf(gpFile, "Uninitialize() --> vkDestroyFence() is done\n");
    }
    
    //DestroySemaphore
    if(vkSemaphore_renderComplete)
    {
        vkDestroySemaphore(vkDevice, vkSemaphore_renderComplete, NULL);
        vkSemaphore_renderComplete = VK_NULL_HANDLE;
        fprintf(gpFile, "Uninitialize() --> vkDestroySemaphore() for vkSemaphore_renderComplete is done\n");
    }
    
    if(vkSemaphore_backBuffer)
    {
        vkDestroySemaphore(vkDevice, vkSemaphore_backBuffer, NULL);
        vkSemaphore_backBuffer = VK_NULL_HANDLE;
        fprintf(gpFile, "Uninitialize() --> vkDestroySemaphore() for vkSemaphore_backBuffer is done\n");
    }
    
    //vkframebuffer
    for(uint32_t i = 0; i < swapchainImageCount ; i++)
    {
        vkDestroyFramebuffer(vkDevice, vkFrameBuffer_array[i], NULL);
    }
    if(vkFrameBuffer_array)
    {
        free(vkFrameBuffer_array);
        vkFrameBuffer_array = NULL;
        fprintf(gpFile, "Uninitialize() --> vkFrameBuffer_array() is done\n");
    }
    
    //Destroy Descriptor Pool
    //When Descriptor pool is destroyed, descriptor set created  by that pool get destroyed implicitly
    if(vkDescriptorPool)
    {
        vkDestroyDescriptorPool(vkDevice, vkDescriptorPool, NULL);        
        vkDescriptorPool = VK_NULL_HANDLE;
        vkDescriptorSet = VK_NULL_HANDLE;
        fprintf(gpFile, "Uninitialize() --> vkDescriptorPool and DescriptorSet() is done\n");
    }
    
    //VkDescritporSetLayout
    if(vkPipelineLayout)
    {
        vkDestroyPipelineLayout(vkDevice, vkPipelineLayout, NULL);
        vkPipelineLayout = VK_NULL_HANDLE;
        fprintf(gpFile, "Uninitialize() --> vkPipelineLayout() is done\n");
    }
    
    //VkDescritporSetLayout
    if(vkDescriptorSetLayout)
    {
        vkDestroyDescriptorSetLayout(vkDevice, vkDescriptorSetLayout, NULL);
        vkDescriptorSetLayout = VK_NULL_HANDLE;
        fprintf(gpFile, "Uninitialize() --> vkDescriptorSetLayout() is done\n");
    }
    
    if(vkPipeline)
    {
        vkDestroyPipeline(vkDevice, vkPipeline, NULL);
        vkPipeline = VK_NULL_HANDLE;
        fprintf(gpFile, "Uninitialize() --> vkDestroyPipeline() is done\n");
    }
    
    //Renderpass
    if(vkRenderPass)
    {
        vkDestroyRenderPass(vkDevice, vkRenderPass, NULL);
        vkRenderPass = VK_NULL_HANDLE;
        fprintf(gpFile, "Uninitialize() --> vkDestroyRenderPass() is done\n");
    }
    
    //destroy shader modules
    if(vkShaderModule_fragment_shader)
    {
        vkDestroyShaderModule(vkDevice, vkShaderModule_fragment_shader, NULL);
        vkShaderModule_fragment_shader = VK_NULL_HANDLE;
        fprintf(gpFile, "Uninitialize() --> vkDestroyShaderModule() is done\n");
    }
    
    if(vkShaderModule_vertex_shader)
    {
        vkDestroyShaderModule(vkDevice, vkShaderModule_vertex_shader, NULL);
        vkShaderModule_vertex_shader = VK_NULL_HANDLE;
        fprintf(gpFile, "Uninitialize() --> vkDestroyShaderModule() is done\n");
    }
    
    //Destroy uniform buffer
    if(uniformData.vkBuffer)
    {
        vkDestroyBuffer(vkDevice, uniformData.vkBuffer, NULL);
        uniformData.vkBuffer = VK_NULL_HANDLE;
        fprintf(gpFile, "Uninitialize() --> vkDestroyBuffer()  for uniformData.vkBuffer is done\n");
    }
    
    if(uniformData.vkDeviceMemory)
    {
        vkFreeMemory(vkDevice, uniformData.vkDeviceMemory, NULL);
        uniformData.vkDeviceMemory = VK_NULL_HANDLE;
        fprintf(gpFile, "Uninitialize() --> vkFreeMemory()  for uniformData.vkDeviceMemory is done\n");
        
    }
    
    if(vkSampler_texture)
    {
        vkDestroySampler(vkDevice, vkSampler_texture, NULL);
        vkSampler_texture = VK_NULL_HANDLE;
        fprintf(gpFile, "Uninitialize() --> vkDestroySampler() for vkSampler_texture is done\n");
    }
    
    if(vkImageView_texture)
    {
        vkDestroyImageView(vkDevice, vkImageView_texture, NULL);
        vkImageView_texture = VK_NULL_HANDLE;
        fprintf(gpFile, "Uninitialize() --> vkDestroySampler() for vkImageView_texture is done\n");
    }
    
    if(vkDeviceMemory_texture)
    {
        vkFreeMemory(vkDevice, vkDeviceMemory_texture, NULL);
        vkDeviceMemory_texture = VK_NULL_HANDLE;
        fprintf(gpFile, "Uninitialize() --> vkFreeMemory() for vkDeviceMemory_texture is done\n");
    }
    
    if(vkImage_texture)
    {
        vkDestroyImage(vkDevice, vkImage_texture, NULL);
        vkImage_texture = VK_NULL_HANDLE;
        fprintf(gpFile, "Uninitialize() --> vkDestroyImage() for vkImage_texture is done\n");
    }

    
    //Vertex Tecoord buffer
    if(vertexData_texcoord.vkDeviceMemory)
    {
        vkFreeMemory(vkDevice, vertexData_texcoord.vkDeviceMemory, NULL);
        vertexData_texcoord.vkDeviceMemory = VK_NULL_HANDLE;
        fprintf(gpFile, "Uninitialize() --> vkFreeMemory() is done\n");
    }
    
    if(vertexData_texcoord.vkBuffer)
    {
        vkDestroyBuffer(vkDevice, vertexData_texcoord.vkBuffer, NULL);
        vertexData_texcoord.vkBuffer = VK_NULL_HANDLE;
        fprintf(gpFile, "Uninitialize() --> vkDestroyBuffer() is done\n");
    }

    //Vertex position BUffer
    if(vertexData_position.vkDeviceMemory)
    {
        vkFreeMemory(vkDevice, vertexData_position.vkDeviceMemory, NULL);
        vertexData_position.vkDeviceMemory = VK_NULL_HANDLE;
        fprintf(gpFile, "Uninitialize() --> vkFreeMemory() is done\n");
    }
    
    if(vertexData_position.vkBuffer)
    {
        vkDestroyBuffer(vkDevice, vertexData_position.vkBuffer, NULL);
        vertexData_position.vkBuffer = VK_NULL_HANDLE;
        fprintf(gpFile, "Uninitialize() --> vkDestroyBuffer() is done\n");
    }

    
    for(uint32_t i = 0; i < swapchainImageCount; i++)
    {
        vkFreeCommandBuffers(vkDevice, vkCommandPool, 1, &vkCommandBuffer_array[i]);
        fprintf(gpFile, "Uninitialize() --> vkFreeCommandBuffers() is done\n");
    }
    
    if(vkCommandBuffer_array)
    {
        free(vkCommandBuffer_array);
        vkCommandBuffer_array = NULL;
        fprintf(gpFile, "Uninitialize() --> vkCommandBuffer_array is freed\n");
    }
    
    
    if(vkCommandPool)
    {
        vkDestroyCommandPool(vkDevice, vkCommandPool, NULL);
        vkCommandPool = NULL;
        fprintf(gpFile, "Uninitialize() --> vkDestroyCommandPool() is done\n");
    }
    
    //destroy depth image view
    if(vkImageView_depth)
    {
        vkDestroyImageView(vkDevice, vkImageView_depth, NULL);
        vkImageView_depth = VK_NULL_HANDLE;
        fprintf(gpFile, "Uninitialize() --> vkDestoryImageView() is done for depth\n");
    }
    
    //destroy device memory for depth image
    if(vkDeviceMemory_depth)
    {
        vkFreeMemory(vkDevice, vkDeviceMemory_depth, NULL);
        vkDeviceMemory_depth = VK_NULL_HANDLE;
        fprintf(gpFile, "Uninitialize() --> vkFreeMemory() is done for depth\n");
    }
    
    if(vkImage_depth)
    {
        vkDestroyImage(vkDevice, vkImage_depth, NULL);
        vkImage_depth = VK_NULL_HANDLE;
        fprintf(gpFile, "Uninitialize() --> vkDestoryImage() is done for depth\n");
    }
    
    //destory image views
    for(uint32_t i = 0; i < swapchainImageCount; i++)
    {
        vkDestroyImageView(vkDevice, swapchainImageView_array[i], NULL);
        fprintf(gpFile, "Uninitialize() --> vkDestoryImageView() is done for color\n");
    }
    
    if(swapchainImageView_array)
    {
        free(swapchainImageView_array);
        swapchainImageView_array = NULL;
        fprintf(gpFile, "Uninitialize() --> swapchainImageView_array is freed\n");
    }
    
    //free swapchainImages
    // for(uint32_t i = 0; i < swapchainImageCount; i++)
    // {
        // vkDestroyImage(vkDevice, swapchainImage_array[i], NULL);
        // fprintf(gpFile, "Uninitialize() --> vkDestroyImage() is done\n");
    // }
    
    if(swapchainImage_array)
    {
        free(swapchainImage_array);
        swapchainImage_array = NULL;
        fprintf(gpFile, "Uninitialize() --> swapchainImage_array is freed\n");
    }
    
  
    
    //Destory Swapchain
    if(vkSwapchainKHR)
    {
        vkDestroySwapchainKHR(vkDevice, vkSwapchainKHR, NULL);
        vkSwapchainKHR = VK_NULL_HANDLE;
        fprintf(gpFile, "Uninitialize() --> vkSwapchainCreateInfoKHR() is done\n");
    }

    //No need to Destroy/Uninitialize the DeviceQueue

    //Destroy vulkan device
    if(vkDevice)
    {
        vkDestroyDevice(vkDevice, NULL);
        vkDevice = VK_NULL_HANDLE;
        fprintf(gpFile, "Uninitialize() --> vkDestoryDevice() is done\n");
    }

    //No need to free slected physical device

    //Destroy vkSurfaceKHR:This function is generic and not platform specific
    if(vkSurfaceKHR)
    {
        vkDestroySurfaceKHR(vkInstance, vkSurfaceKHR, NULL);
        vkSurfaceKHR = VK_NULL_HANDLE;
        fprintf(gpFile, "Uninitialize() --> vkDestroySurfaceKHR() is done\n");
    }
    
    //Validation destroying
    if(vkDebugReportCallbackEXT && vkDestroyDebugReportCallbackEXT_fnptr)
    {
        vkDestroyDebugReportCallbackEXT_fnptr(vkInstance, 
                                              vkDebugReportCallbackEXT,
                                              NULL);
        vkDebugReportCallbackEXT = VK_NULL_HANDLE;
        vkDestroyDebugReportCallbackEXT_fnptr = NULL;
    }
    
    
    //Destroy Vulkan Instance
    if(vkInstance)
    {
        vkDestroyInstance(vkInstance, NULL);
        vkInstance = VK_NULL_HANDLE;
        fprintf(gpFile, "Uninitialize() --> vkDestroyInstance() is done\n");
    }
    
    if (gpFile)
	{
		fprintf(gpFile, "Uninitialize() --> Program terminated successfully\n");
		fclose(gpFile);
		gpFile = NULL;
	}
}

/***************Definition of Vulkan functions***********************/
/********************************************************************/

VkResult createVulkanInstance(void)
{
    //Function declarations
    VkResult fillInstanceExtensionNames(void);
    VkResult fillValidationLayerNames(void);
    VkResult createValidationCallbackFunction(void);

    //variable declarations
    VkResult vkResult = VK_SUCCESS;

    //code
    //Step1: Fill and initialize required extension names and count in global variable
    vkResult = fillInstanceExtensionNames();
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "createVulkanInstance() --> fillInstanceExtensionNames() is failed\n");
    }
    else
    {
        fprintf(gpFile, "createVulkanInstance() --> fillInstanceExtensionNames() is succedded\n");
    }

    if(bValidation == TRUE)
    {
        //Fill validation Layer Names
        vkResult = fillValidationLayerNames();
        if(vkResult != VK_SUCCESS)
        {
            fprintf(gpFile, "createVulkanInstance() --> fillValidationLayerNames() is failed\n");
        }
        else
        {
            fprintf(gpFile, "createVulkanInstance() --> fillValidationLayerNames() is succedded\n");
        }
    }


    //Step1: Initialize struct VkApplicationInfo
    VkApplicationInfo vkApplicationInfo;
    memset((void*)&vkApplicationInfo, 0, sizeof(VkApplicationInfo));

    vkApplicationInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO; //type safety, generic names
    vkApplicationInfo.pNext = NULL; //Linked List
    vkApplicationInfo.pApplicationName = gpszAppName;
    vkApplicationInfo.applicationVersion = 1;
    vkApplicationInfo.pEngineName = gpszAppName;
    vkApplicationInfo.engineVersion = 1;
    vkApplicationInfo.apiVersion = VK_API_VERSION_1_4;


    //Step3: Initialize struct VkInstanceCreateInfo by using information in Step1 and Step2
    VkInstanceCreateInfo vkInstanceCreateInfo;
    memset((void*)&vkInstanceCreateInfo, 0, sizeof(VkInstanceCreateInfo));
    vkInstanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    vkInstanceCreateInfo.pNext = NULL;
    vkInstanceCreateInfo.pApplicationInfo = &vkApplicationInfo;
    vkInstanceCreateInfo.enabledExtensionCount = enabledInstanceExtensionCount;
    vkInstanceCreateInfo.ppEnabledExtensionNames = enabledInstanceExtensionNames_array;
    
    if(bValidation == TRUE)
    {
        vkInstanceCreateInfo.enabledLayerCount = enabledValidationLayerCount;
        vkInstanceCreateInfo.ppEnabledLayerNames = enabledValidationLayerNames_array;
    }
    else
    {
        vkInstanceCreateInfo.enabledLayerCount = 0;
        vkInstanceCreateInfo.ppEnabledLayerNames = NULL;
    }

    /*
	// Provided by VK_VERSION_1_0
		VkResult vkCreateInstance(
		const VkInstanceCreateInfo* pCreateInfo,
		const VkAllocationCallbacks* pAllocator,
		VkInstance* pInstance);

		• pCreateInfo is a pointer to a VkInstanceCreateInfo structure controlling creation of the instance.
		• pAllocator controls host memory allocation as described in the Memory Allocation chapter.
		• pInstance points a VkInstance handle in which the resulting instance is returned.
	 */
    //Step4: Call VkCreateInstance() to get vkInstance in a global variable and do error checking
    vkResult = vkCreateInstance(&vkInstanceCreateInfo,
                                 NULL,  //no custom Memory allocater
                                 &vkInstance);
    if(vkResult == VK_ERROR_INCOMPATIBLE_DRIVER)
    {
        fprintf(gpFile, "createVulkanInstance() --> vkCreateInstance:: vkCreateInstance failed due to incompatible driver %d\n", vkResult);
        return vkResult;
    }
    else if(vkResult == VK_ERROR_EXTENSION_NOT_PRESENT)
    {
        fprintf(gpFile, "createVulkanInstance() --> vkCreateInstance:: vkCreateInstance failed due to required extension not present %d\n", vkResult);
        return vkResult;
    }
    else if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "createVulkanInstance() --> vkCreateInstance:: vkCreateInstance failed due to unknown reason %d\n", vkResult);
        return vkResult;
    }
    else
    {
        fprintf(gpFile, "createVulkanInstance() --> vkCreateInstance:: vkCreateInstance succedded\n");
    }
    
    //do for validation callbacks
    if(bValidation == TRUE)
    {
        vkResult = createValidationCallbackFunction();
        if(vkResult != VK_SUCCESS)
        {
            fprintf(gpFile, "createVulkanInstance() --> createValidationCallbackFunction() is failed\n");
        }
        else
        {
            fprintf(gpFile, "createVulkanInstance() --> createValidationCallbackFunction() is succedded\n");
        }
    }


   //Step5: Destroy

    return vkResult;
}


VkResult fillInstanceExtensionNames(void)
{
    //variable declaration
    VkResult vkResult = VK_SUCCESS;

    //Step1: Find how many Instacne Extension are supported by the vulkan driver of this version and keep the count in local variable
    uint32_t instanceExtensionCount = 0;

    /*
    // Provided by VK_VERSION_1_0
        VkResult vkEnumerateInstanceExtensionProperties(
        const char* pLayerName,
        uint32_t* pPropertyCount,
        VkExtensionProperties* pProperties);
    
        • pLayerName is either NULL or a pointer to a null-terminated UTF-8 string naming the layer to
          retrieve extensions from.
        • pPropertyCount is a pointer to an integer related to the number of extension properties available
          or queried, as described below.
        • pProperties is either NULL or a pointer to an array of VkExtensionProperties structures
    */

    vkResult = vkEnumerateInstanceExtensionProperties(NULL, //Which layer's extenion is needed: Mention extension name: For all driver's extension use NULL
                                                      &instanceExtensionCount,
                                                      NULL); // Instance Extensions Properties array: As we dont have count, so its NULL
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "fillInstanceExtensionNames() --> 1st call to vkEnumerateInstanceExtensionProperties() is failed\n");
    }
    else
    {
        fprintf(gpFile, "fillInstanceExtensionNames() --> 1st call to vkEnumerateInstanceExtensionProperties() is succedded\n");
    }



    //Step2: Allocate and fill VkExtensionProperties corresponding to above count
    VkExtensionProperties* vkExtensionProperties_array = NULL;
    vkExtensionProperties_array = (VkExtensionProperties*) malloc (sizeof(VkExtensionProperties) * instanceExtensionCount);
    //Should be error checking for malloc: assert() can also be used

    vkResult = vkEnumerateInstanceExtensionProperties(NULL, //Which layer's extenion is needed: Mention extension name: For all driver's extension use NULL
                                                      &instanceExtensionCount,
                                                      vkExtensionProperties_array); // Instance Extensions Properties array: As we have count, so it is value
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "fillInstanceExtensionNames() --> 2nd call to vkEnumerateInstanceExtensionProperties() is failed\n");
    }
    else
    {
        fprintf(gpFile, "fillInstanceExtensionNames() --> 2nd call to vkEnumerateInstanceExtensionProperties() is succedded\n");
    }



    //Step3: Fill and Display a local string array of extension names obtained from vkExtensionProperties
    char** instanceExtensionNames_array = NULL;

    instanceExtensionNames_array = (char**)malloc(sizeof(char*) * instanceExtensionCount);
    //Should be error checking for malloc: assert() can also be used
    for(uint32_t i = 0; i < instanceExtensionCount; i++)
    {
        instanceExtensionNames_array[i] = (char*)malloc(sizeof(char) * strlen(vkExtensionProperties_array[i].extensionName) + 1);
        memcpy(instanceExtensionNames_array[i], vkExtensionProperties_array[i].extensionName, strlen(vkExtensionProperties_array[i].extensionName) + 1);
        fprintf(gpFile, "fillInstanceExtensionNames() --> Vulkan Instance Extension names = %s\n", instanceExtensionNames_array[i]);
    }



   //Step4: As not required henceforth, free the vkExtensionProperties_array;
   free(vkExtensionProperties_array);
   vkExtensionProperties_array = NULL;



   // Step5: Find whether below extension names contains our required two extensions
   //VK_KHR_SURFACE_EXTENSION_NAME
   //VK_KHR_WIN32_SURFACE_EXTENSION_NAME
   VkBool32 vulkanSurfaceExtensionFound = VK_FALSE;
   VkBool32 vulkanWin32SurfaceExtensionFound = VK_FALSE;
   VkBool32 vulkanDebugReportExtensionFound = VK_FALSE;

    for(uint32_t i = 0; i < instanceExtensionCount; i++)
    {
        if(strcmp(instanceExtensionNames_array[i], VK_KHR_SURFACE_EXTENSION_NAME) == 0)
        {
            vulkanSurfaceExtensionFound = VK_TRUE;
            enabledInstanceExtensionNames_array[enabledInstanceExtensionCount++] = VK_KHR_SURFACE_EXTENSION_NAME;
        }
        
        if(strcmp(instanceExtensionNames_array[i], VK_KHR_WIN32_SURFACE_EXTENSION_NAME) == 0)
        {
            vulkanWin32SurfaceExtensionFound = VK_TRUE;
            enabledInstanceExtensionNames_array[enabledInstanceExtensionCount++] = VK_KHR_WIN32_SURFACE_EXTENSION_NAME;
        }
        
        if(strcmp(instanceExtensionNames_array[i], VK_EXT_DEBUG_REPORT_EXTENSION_NAME) == 0)
        {
            vulkanDebugReportExtensionFound = VK_TRUE;
            if(bValidation == TRUE)
            {
                enabledInstanceExtensionNames_array[enabledInstanceExtensionCount++] = VK_EXT_DEBUG_REPORT_EXTENSION_NAME;
            }
            else
            {
                //array will not have entry of VK_EXT_DEBUG_REPORT_EXTENSION_NAME;
            }
        }
    }



    //Step 6:
    //As not required henceforth, free the local string array
    for(uint32_t i = 0; i < instanceExtensionCount; i++)
    {
        free(instanceExtensionNames_array[i]);
    }
    free(instanceExtensionNames_array);



    //Step7:Print whether our vulkan driver supports our required extension names or not
    if(vulkanSurfaceExtensionFound == VK_FALSE)
    {
        // return hardcoded failure
        vkResult = VK_ERROR_INITIALIZATION_FAILED;
        fprintf(gpFile, "fillInstanceExtensionNames() --> VK_KHR_SURFACE_EXTENSION_NAME not found\n");
        return vkResult;
    }
    else
    {
        fprintf(gpFile, "fillInstanceExtensionNames() --> VK_KHR_SURFACE_EXTENSION_NAME found\n");
    }

    if(vulkanWin32SurfaceExtensionFound == VK_FALSE)
    {
        // return hardcoded failure
        vkResult = VK_ERROR_INITIALIZATION_FAILED;
        fprintf(gpFile, "fillInstanceExtensionNames() --> VK_KHR_WIN32_SURFACE_EXTENSION_NAME not found\n");
        return vkResult;
    }
    else
    {
        fprintf(gpFile, "fillInstanceExtensionNames() --> VK_KHR_WIN32_SURFACE_EXTENSION_NAME found\n");
    }
    
    if(vulkanDebugReportExtensionFound == VK_FALSE)
    {
        if(bValidation == TRUE)
        {
            // return hardcoded failure
            vkResult = VK_ERROR_INITIALIZATION_FAILED;
            fprintf(gpFile, "fillInstanceExtensionNames() --> VK_EXT_DEBUG_REPORT_EXTENSION_NAME not found:: Validation is ON But required VK_EXT_DEBUG_REPORT_EXTENSION_NAME is not supported\n");
            return vkResult;
        }
        else
        {
            fprintf(gpFile, "fillInstanceExtensionNames() --> VK_EXT_DEBUG_REPORT_EXTENSION_NAME not found:: Validation is OFF But required VK_EXT_DEBUG_REPORT_EXTENSION_NAME is not supported\n");
        }
    }
    else
    {
        if(bValidation == TRUE)
        {
            fprintf(gpFile, "fillInstanceExtensionNames() --> VK_EXT_DEBUG_REPORT_EXTENSION_NAME found:: Validation is ON and required VK_EXT_DEBUG_REPORT_EXTENSION_NAME is supported\n");
        }
        else
        {
            fprintf(gpFile, "fillInstanceExtensionNames() --> VK_EXT_DEBUG_REPORT_EXTENSION_NAME found:: Validation is OFF and required VK_EXT_DEBUG_REPORT_EXTENSION_NAME is supported\n");
        }
    }


    //Step8: Print only Enabled Extension Names 
    for(uint32_t i = 0; i < enabledInstanceExtensionCount; i++)
    {
         fprintf(gpFile, "fillInstanceExtensionNames() --> Enabled vulkan Instance extension Names = %s\n", enabledInstanceExtensionNames_array[i]);
    }

    return vkResult;
}


VkResult fillValidationLayerNames(void)
{
    //code
    //variables
    VkResult vkResult = VK_SUCCESS;
    uint32_t validationLayerCount = 0;
    
    vkResult = vkEnumerateInstanceLayerProperties(&validationLayerCount,
                                                  NULL); // Instance Validation Properties array: As we dont have count, so its NULL
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "fillValidationLayerNames() --> 1st call to vkEnumerateInstanceLayerProperties() is failed: error code %d\n", vkResult);
    }
    else
    {
        fprintf(gpFile, "fillValidationLayerNames() --> 1st call to vkEnumerateInstanceLayerProperties() is succedded\n");
    }
    
    VkLayerProperties* vkLayerProperties_array = NULL;
    vkLayerProperties_array = (VkLayerProperties*) malloc (sizeof(VkLayerProperties) * validationLayerCount);
    //Should be error checking for malloc: assert() can also be used

    vkResult = vkEnumerateInstanceLayerProperties(&validationLayerCount,
                                                  vkLayerProperties_array); // Instance Validation Properties array: As we dont have count, so its NULL
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "fillValidationLayerNames() --> 2nd call to vkEnumerateInstanceLayerProperties() is failed: error code %d\n", vkResult);
    }
    else
    {
        fprintf(gpFile, "fillValidationLayerNames() --> 2nd call to vkEnumerateInstanceLayerProperties() is succedded\n");
    }
    
    char** validationLayerNames_array = NULL;
    validationLayerNames_array = (char**) malloc(sizeof(char*) * validationLayerCount);
    //Should be error checking for malloc: assert() can also be used
    for(uint32_t i = 0; i < validationLayerCount; i++)
    {
        validationLayerNames_array[i] = (char*)malloc(sizeof(char) * strlen(vkLayerProperties_array[i].layerName) + 1);
        //Should be error checking for malloc: assert() can also be used
        memcpy(validationLayerNames_array[i], vkLayerProperties_array[i].layerName, strlen(vkLayerProperties_array[i].layerName) + 1);
        fprintf(gpFile, "fillValidationLayerNames() --> Vulkan Validation Layer names = %s\n", vkLayerProperties_array[i].layerName);
    }
    
    if(vkLayerProperties_array) 
        free(vkLayerProperties_array);
    vkLayerProperties_array = NULL;

    // Step5: Find whether below layer names contains our required two extensions
    //VK_KHR_SURFACE_EXTENSION_NAME
    VkBool32 vulkanValidationLayerFound = VK_FALSE;
    for(uint32_t i = 0; i < validationLayerCount; i++)
    {
        if(strcmp(validationLayerNames_array[i], "VK_LAYER_KHRONOS_validation") == 0)
        {
            vulkanValidationLayerFound = VK_TRUE;
            enabledValidationLayerNames_array[enabledValidationLayerCount++] = "VK_LAYER_KHRONOS_validation";
        }
    }
    
     //As not required henceforth, free the local string array
    for(uint32_t i = 0; i < validationLayerCount; i++)
    {
        free(validationLayerNames_array[i]);
    }
    free(validationLayerNames_array);
    
    if(bValidation == TRUE)
    {
        //Step7:Print whether our vulkan driver supports our required extension names or not
        if(vulkanValidationLayerFound == VK_FALSE)
        {
            // return hardcoded failure
            vkResult = VK_ERROR_INITIALIZATION_FAILED;
            fprintf(gpFile, "fillValidationLayerNames() --> VK_LAYER_KHRONOS_validation not supported\n");
            return vkResult;
        }
        else
        {
            fprintf(gpFile, "fillValidationLayerNames() --> VK_LAYER_KHRONOS_validation is supported\n");
        }
    }
    
    //Step8: Print only Enabled validation layer Names
    for(uint32_t i = 0; i < enabledValidationLayerCount; i++)
    {
         fprintf(gpFile, "fillValidationLayerNames() --> Enabled vulkan validation layer Names = %s\n", enabledValidationLayerNames_array[i]);
    }
    
    return (vkResult);    
}

VkResult createValidationCallbackFunction(void)
{
    //code
    //function declarations
    VKAPI_ATTR VkBool32 VKAPI_CALL debugReportCallback(VkDebugReportFlagsEXT, 
                                                       VkDebugReportObjectTypeEXT,
                                                       uint64_t,
                                                       size_t,
                                                       int32_t,
                                                       const char*,
                                                       const char*,
                                                       void*);
    
    //variables
    VkResult vkResult = VK_SUCCESS;
    
    // Get the required function pointer for CREATE (local is fine)
    PFN_vkCreateDebugReportCallbackEXT vkCreateDebugReportCallbackEXT_fnptr = NULL;
    vkCreateDebugReportCallbackEXT_fnptr =
        (PFN_vkCreateDebugReportCallbackEXT) vkGetInstanceProcAddr(
            vkInstance, "vkCreateDebugReportCallbackEXT");
    if(vkCreateDebugReportCallbackEXT_fnptr == NULL)
    {
        vkResult = VK_ERROR_INITIALIZATION_FAILED;
        fprintf(gpFile, "createValidationCallbackFunction() --> vkGetInstanceProcAddr() failed to get vkCreateDebugReportCallbackEXT\n");
        return vkResult;
    }
    else
    {
        fprintf(gpFile, "createValidationCallbackFunction() --> vkGetInstanceProcAddr() succeeded for vkCreateDebugReportCallbackEXT\n");
    }
    
    // ***** IMPORTANT FIX *****
    // Do NOT redeclare a local variable here; assign to the GLOBAL pointer.
    // (Previously a local variable shadowed the global, leaving the global NULL.)
    vkDestroyDebugReportCallbackEXT_fnptr =
        (PFN_vkDestroyDebugReportCallbackEXT) vkGetInstanceProcAddr(
            vkInstance, "vkDestroyDebugReportCallbackEXT");
    if(vkDestroyDebugReportCallbackEXT_fnptr == NULL)
    {
        vkResult = VK_ERROR_INITIALIZATION_FAILED;
        fprintf(gpFile, "createValidationCallbackFunction() --> vkGetInstanceProcAddr() failed to get vkDestroyDebugReportCallbackEXT\n");
        return vkResult;
    }
    else
    {
        fprintf(gpFile, "createValidationCallbackFunction() --> vkGetInstanceProcAddr() succeeded for vkDestroyDebugReportCallbackEXT\n");
    }
    // ***** END FIX *****

    // Create the debug report callback
    VkDebugReportCallbackCreateInfoEXT vkDebugReportCallbackCreateInfoEXT;
    memset((void*)&vkDebugReportCallbackCreateInfoEXT, 0, sizeof(VkDebugReportCallbackCreateInfoEXT));
    vkDebugReportCallbackCreateInfoEXT.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CREATE_INFO_EXT;
    vkDebugReportCallbackCreateInfoEXT.pNext = NULL;
    vkDebugReportCallbackCreateInfoEXT.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT;
    vkDebugReportCallbackCreateInfoEXT.pfnCallback = debugReportCallback;
    vkDebugReportCallbackCreateInfoEXT.pUserData = NULL;
    
    vkResult = vkCreateDebugReportCallbackEXT_fnptr(vkInstance,
                                                    &vkDebugReportCallbackCreateInfoEXT,
                                                    NULL,
                                                    &vkDebugReportCallbackEXT);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "createValidationCallbackFunction() --> vkCreateDebugReportCallbackEXT() failed: error %d\n", vkResult);
    }
    else
    {
        fprintf(gpFile, "createValidationCallbackFunction() --> vkCreateDebugReportCallbackEXT() succeeded\n");
    }
    
    return vkResult;
}

//Create Vulkan Presentation Surface
VkResult getSupportedSurface(void)
{
    //local variable declaration
    VkResult vkResult = VK_SUCCESS;

    //Step2
    VkWin32SurfaceCreateInfoKHR vkWin32SurfaceCreateInfoKHR;
    memset((void*)&vkWin32SurfaceCreateInfoKHR, 0, sizeof(VkWin32SurfaceCreateInfoKHR));
    vkWin32SurfaceCreateInfoKHR.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
    vkWin32SurfaceCreateInfoKHR.pNext = NULL;
    vkWin32SurfaceCreateInfoKHR.flags = 0;
    //one way
    // vkWin32SurfaceCreateInfoKHR.hinstance = (HINSTANCE)GetModuleHandle(NULL);
    //another way for 64bit
    vkWin32SurfaceCreateInfoKHR.hinstance = (HINSTANCE)GetWindowLongPtr(ghwnd, GWLP_HINSTANCE);
    vkWin32SurfaceCreateInfoKHR.hwnd = ghwnd;

    //Step3:
    vkResult = vkCreateWin32SurfaceKHR(vkInstance,
                                       &vkWin32SurfaceCreateInfoKHR,
                                       NULL, //Memory mamnagement function is default
                                       &vkSurfaceKHR);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "getSupportedSurface() --> vkCreateWin32SurfaceKHR() is failed %d\n", vkResult);
    }
    else
    {
        fprintf(gpFile, "getSupportedSurface() --> vkCreateWin32SurfaceKHR() is succedded\n");
    }

    return vkResult;
}


VkResult getPhysicalDevice(void)
{
    //local variable declaration
    VkResult vkResult = VK_SUCCESS;
    

    //code
    vkResult = vkEnumeratePhysicalDevices(vkInstance,
                                          &physicalDeviceCount,
                                          NULL);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "getPhysicalDevice() --> 1st call to vkEnumeratePhysicalDevices() is failed %d\n", vkResult);
        return vkResult;
    }
    else if(physicalDeviceCount == 0)
    {
        fprintf(gpFile, "getPhysicalDevice() --> 1st call to vkEnumeratePhysicalDevices() resulted in zero devices\n");
        vkResult = VK_ERROR_INITIALIZATION_FAILED;
        return vkResult;
    }
    else
    {
        fprintf(gpFile, "getPhysicalDevice() --> 1st call to vkEnumeratePhysicalDevices() is succedded\n");
    }

    vkPhysicalDevice_array = (VkPhysicalDevice*)malloc(sizeof(VkPhysicalDevice) * physicalDeviceCount);
    //error checking to be done
    
    vkResult = vkEnumeratePhysicalDevices(vkInstance, 
                                          &physicalDeviceCount,
                                          vkPhysicalDevice_array);
     if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "getPhysicalDevice() --> 2nd call to vkEnumeratePhysicalDevices() is failed %d\n", vkResult);
        return vkResult;
    }
    else
    {
        fprintf(gpFile, "getPhysicalDevice() --> 2nd call to vkEnumeratePhysicalDevices() is succedded\n");
    }

    VkBool32 bFound = VK_FALSE;
    for(uint32_t i = 0; i < physicalDeviceCount; i++)
    {
        uint32_t qCount = UINT32_MAX;
        
        //If physical device is present then it must support at least 1 queue family
        vkGetPhysicalDeviceQueueFamilyProperties(vkPhysicalDevice_array[i], 
                                               &qCount, 
                                               NULL);
        VkQueueFamilyProperties *vkQueueFamilyProperties_array = NULL;
        vkQueueFamilyProperties_array = (VkQueueFamilyProperties*)malloc(sizeof(VkQueueFamilyProperties) * qCount);
        //error checking to be done
        
        vkGetPhysicalDeviceQueueFamilyProperties(vkPhysicalDevice_array[i], 
                                               &qCount, 
                                               vkQueueFamilyProperties_array);
        
        VkBool32* isQueueSurfaceSupported_array = NULL;
        isQueueSurfaceSupported_array = (VkBool32*)malloc(sizeof(VkBool32) * qCount);
        //error checking to be done
        
        for(uint32_t j = 0; j < qCount; j++)
        {
            vkGetPhysicalDeviceSurfaceSupportKHR(vkPhysicalDevice_array[i], 
                                                 j,
                                                 vkSurfaceKHR,
                                                 &isQueueSurfaceSupported_array[j]);
        }
        
        for(uint32_t j = 0; j < qCount; j++)
        {
            if(vkQueueFamilyProperties_array[j].queueFlags & VK_QUEUE_GRAPHICS_BIT)
            {
                if(isQueueSurfaceSupported_array[j] == VK_TRUE)
                {
                    vkPhysicalDevice_selected = vkPhysicalDevice_array[i];
                    graphicsQueueFamilyIndex_selected = j;
                    bFound = VK_TRUE;
                    break;
                }
            }
        }
        
        if(isQueueSurfaceSupported_array)
        {
            free(isQueueSurfaceSupported_array);
            isQueueSurfaceSupported_array = NULL;
            fprintf(gpFile, "getPhysicalDevice() --> isQueueSurfaceSupported_array succedded to free\n");
        }
        
        if(vkQueueFamilyProperties_array)
        {
            free(vkQueueFamilyProperties_array);
            vkQueueFamilyProperties_array = NULL;
            fprintf(gpFile, "getPhysicalDevice() --> vkQueueFamilyProperties_array succedded to free\n");
        }
        
        if(bFound == VK_TRUE)
        {
            break;
        }
    }
    
    if(bFound == VK_TRUE)
    {
        fprintf(gpFile, "getPhysicalDevice() -->is succedded to select the required device with graphics enabled\n");
    }
    else
    {
        if(vkPhysicalDevice_array)
        {
            free(vkPhysicalDevice_array);
            vkPhysicalDevice_array = NULL;
            fprintf(gpFile, "getPhysicalDevice() --> vkPhysicalDevice_array succedded to free\n");
        }
        vkResult = VK_ERROR_INITIALIZATION_FAILED;
        fprintf(gpFile, "getPhysicalDevice() -->is failed to select the required device with graphics enabled\n");
    }
    
    
    memset((void*)&vkPhysicalDeviceMemoryProperties, 0, sizeof(VkPhysicalDeviceMemoryProperties));
    vkGetPhysicalDeviceMemoryProperties(vkPhysicalDevice_selected, 
                                        &vkPhysicalDeviceMemoryProperties);
                                        
    VkPhysicalDeviceFeatures vkPhysicalDeviceFeatures;
    memset((void*)&vkPhysicalDeviceFeatures, 0, sizeof(VkPhysicalDeviceFeatures));
    
    vkGetPhysicalDeviceFeatures(vkPhysicalDevice_selected, 
                                &vkPhysicalDeviceFeatures);
                                
    if(vkPhysicalDeviceFeatures.tessellationShader)
    {
        fprintf(gpFile, "getPhysicalDevice() -->selected device supports tessellationShader\n");
    }
    else
    {
          fprintf(gpFile, "getPhysicalDevice() -->selected device not supports tessellationShader\n");
    }
    
    if(vkPhysicalDeviceFeatures.geometryShader)
    {
        fprintf(gpFile, "getPhysicalDevice() -->selected device supports geometryShader\n");
    }
    else
    {
        fprintf(gpFile, "getPhysicalDevice() -->selected device not supports geometryShader\n");
    }
    
    return vkResult;
}


VkResult printVkInfo(void)
{
    //local variable declaration
    VkResult vkResult = VK_SUCCESS;
    
    //code
    fprintf(gpFile, "*******************VULKAN INFORMATION*********************\n");
    for(uint32_t i = 0; i < physicalDeviceCount; i++)    
    {
        fprintf(gpFile, "Infomration of Device = %d\n", i);
        
        VkPhysicalDeviceProperties vkPhysicalDeviceProperties;
        memset((void*)&vkPhysicalDeviceProperties, 0, sizeof(VkPhysicalDeviceProperties));
        
        vkGetPhysicalDeviceProperties(vkPhysicalDevice_array[i], &vkPhysicalDeviceProperties);
        
        uint32_t majorVersion = VK_API_VERSION_MAJOR(vkPhysicalDeviceProperties.apiVersion);
        uint32_t minorVersion = VK_API_VERSION_MINOR(vkPhysicalDeviceProperties.apiVersion);;
        uint32_t patchVersion = VK_API_VERSION_PATCH(vkPhysicalDeviceProperties.apiVersion);;
        //API Version
        fprintf(gpFile, "apiVersion = %d.%d.%d\n", majorVersion, minorVersion, patchVersion);
        
        //Device Name
        fprintf(gpFile, "DeviceName = %s\n", vkPhysicalDeviceProperties.deviceName);
        
        //DeviceType
        switch(vkPhysicalDeviceProperties.deviceType)
        {
            case(VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU):
                fprintf(gpFile, "DeviceType = Integrated GPU(iGPU)\n");
                break;
            
            case(VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU):
                fprintf(gpFile, "DeviceType = Discrete GPU(dGPU)\n");
                break;
                
            case(VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU):
                fprintf(gpFile, "DeviceType = Virtual GPU(vGPU)\n");
                break;
                
            case(VK_PHYSICAL_DEVICE_TYPE_CPU):
                fprintf(gpFile, "DeviceType = CPU\n");
                break;    
                
            case(VK_PHYSICAL_DEVICE_TYPE_OTHER):
                fprintf(gpFile, "DeviceType = Other\n");
                break; 
                
            default: 
                fprintf(gpFile, "DeviceType = UNKNOWN\n");                
        }
        
        //Vendor Id
        fprintf(gpFile, "VendorId = 0x%04x\n", vkPhysicalDeviceProperties.vendorID);
        
        //DeviceId
        fprintf(gpFile, "DeviceId = 0x%04x\n\n", vkPhysicalDeviceProperties.deviceID);
   }
   
   fprintf(gpFile, "****************END OF VULKAN INFORMATION********************\n");
   
    //Freephysical device array
    if(vkPhysicalDevice_array)
    {
        free(vkPhysicalDevice_array);
        vkPhysicalDevice_array = NULL;
        fprintf(gpFile, "printVkInfo() --> vkPhysicalDevice_array succedded to free\n");
    }
    
    return vkResult;
}


VkResult fillDeviceExtensionNames(void)
{
    //variable declaration
    VkResult vkResult = VK_SUCCESS;

    //Step1: Find how many Device Extension are supported by the vulkan driver of this version and keep the count in local variable
    uint32_t deviceExtensionCount = 0;

    vkResult = vkEnumerateDeviceExtensionProperties(vkPhysicalDevice_selected,
                                                    NULL,  //Layer name: All layers
                                                    &deviceExtensionCount,
                                                    NULL); // Device Extensions Properties array: As we dont have count, so its NULL
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "fillDeviceExtensionNames() --> 1st call to vkEnumerateDeviceExtensionProperties() is failed: %d\n", vkResult);
        return vkResult;
    }
    else
    {
        fprintf(gpFile, "fillDeviceExtensionNames() --> 1st call to vkEnumerateDeviceExtensionProperties() is succedded\n");
        fprintf(gpFile, "deviceExtensionCount is %u\n", deviceExtensionCount);
    }


    //Step2: Allocate and fill VkExtensionProperties corresponding to above count
    VkExtensionProperties* vkExtensionProperties_array = NULL;
    vkExtensionProperties_array = (VkExtensionProperties*) malloc (sizeof(VkExtensionProperties) * deviceExtensionCount);
    //Should be error checking for malloc: assert() can also be used

    vkResult = vkEnumerateDeviceExtensionProperties(vkPhysicalDevice_selected,
                                                    NULL, //Which layer's extenion is needed: Mention extension name: For all driver's extension use NULL
                                                    &deviceExtensionCount,
                                                    vkExtensionProperties_array); // Instance Extensions Properties array: As we have count, so it is value
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "fillDeviceExtensionNames() --> 2nd call to vkEnumerateDeviceExtensionProperties() is failed: %d\n", vkResult);
        return vkResult;
    }
    else
    {
        fprintf(gpFile, "fillDeviceExtensionNames() --> 2nd call to vkEnumerateDeviceExtensionProperties() is succedded\n");
    }



    //Step3: Fill and Display a local string array of extension names obtained from vkExtensionProperties
    char** deviceExtensionNames_array = NULL;

    deviceExtensionNames_array = (char**)malloc(sizeof(char*) * deviceExtensionCount);
    //Should be error checking for malloc: assert() can also be used
    for(uint32_t i = 0; i < deviceExtensionCount; i++)
    {
        deviceExtensionNames_array[i] = (char*)malloc(sizeof(char) * strlen(vkExtensionProperties_array[i].extensionName) + 1);
        memcpy(deviceExtensionNames_array[i], vkExtensionProperties_array[i].extensionName, strlen(vkExtensionProperties_array[i].extensionName) + 1);
        fprintf(gpFile, "fillDeviceExtensionNames() --> Vulkan Device Extension names = %s\n", deviceExtensionNames_array[i]);
    }



   //Step4: As not required henceforth, free the vkExtensionProperties_array;
   free(vkExtensionProperties_array);
   vkExtensionProperties_array = NULL;



   // Step5: Find whether below extension names contains our required two extensions
   //VK_KHR_SWAPCHAIN_EXTENSION_NAME
   VkBool32 vulkanSwapChainExtensionFound = VK_FALSE;
  
    for(uint32_t i = 0; i < deviceExtensionCount; i++)
    {
        if(strcmp(deviceExtensionNames_array[i], VK_KHR_SWAPCHAIN_EXTENSION_NAME) == 0)
        {
            vulkanSwapChainExtensionFound = VK_TRUE;
            enabledDeviceExtensionNames_array[enabledDeviceExtensionCount++] = VK_KHR_SWAPCHAIN_EXTENSION_NAME;
        }
    }



    //Step 6:
    //As not required henceforth, free the local string array
    for(uint32_t i = 0; i < deviceExtensionCount; i++)
    {
        free(deviceExtensionNames_array[i]);
    }
    free(deviceExtensionNames_array);



    //Step7:Print whether our vulkan driver supports our required extension names or not
    if(vulkanSwapChainExtensionFound == VK_FALSE)
    {
        // return hardcoded failure
        vkResult = VK_ERROR_INITIALIZATION_FAILED;
        fprintf(gpFile, "fillDeviceExtensionNames() --> VK_KHR_SWAPCHAIN_EXTENSION_NAME not found\n");
        return vkResult;
    }
    else
    {
        fprintf(gpFile, "fillDeviceExtensionNames() --> VK_KHR_SWAPCHAIN_EXTENSION_NAME found\n");
    }


    //Step8: Print only Enabled Extension Names
    for(uint32_t i = 0; i < enabledDeviceExtensionCount; i++)
    {
         fprintf(gpFile, "fillDeviceExtensionNames() --> Enabled vulkan Device extension Names = %s\n", enabledDeviceExtensionNames_array[i]);
    }

    return vkResult;
}


VkResult createVulkanDevice(void)
{  
    //Function declarations
    VkResult fillDeviceExtensionNames(void);
    
    //variable declaration
    VkResult vkResult = VK_SUCCESS;
    
    //Fill Device Extensions
    vkResult = fillDeviceExtensionNames();
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "createVulkanDevice() --> fillDeviceExtensionNames() is failed %d\n", vkResult);
        return vkResult;
    }
    else
    {
        fprintf(gpFile, "createVulkanDevice() --> fillDeviceExtensionNames() is succedded\n");
    }
    
    /////Newly added code//////
    
    float QueuePriorities[] = {1.0};
    VkDeviceQueueCreateInfo vkDeviceQueueCreateInfo;
    memset((void*)&vkDeviceQueueCreateInfo, 0, sizeof(VkDeviceQueueCreateInfo));
    vkDeviceQueueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    vkDeviceQueueCreateInfo.pNext = NULL;
    vkDeviceQueueCreateInfo.flags = 0;
    vkDeviceQueueCreateInfo.queueFamilyIndex = graphicsQueueFamilyIndex_selected;
    vkDeviceQueueCreateInfo.queueCount = 1;
    vkDeviceQueueCreateInfo.pQueuePriorities = QueuePriorities;
    
    //Initialize VkDeviceCreateinfo structure
    VkDeviceCreateInfo vkDeviceCreateInfo;
    memset((void*)&vkDeviceCreateInfo, 0, sizeof(VkDeviceCreateInfo));
    
    vkDeviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    vkDeviceCreateInfo.pNext = NULL;
    vkDeviceCreateInfo.flags = 0;
    vkDeviceCreateInfo.enabledExtensionCount = enabledDeviceExtensionCount;
    vkDeviceCreateInfo.ppEnabledExtensionNames = enabledDeviceExtensionNames_array;
    vkDeviceCreateInfo.enabledLayerCount = 0;  // Deprecated
    vkDeviceCreateInfo.ppEnabledLayerNames = NULL;  // Deprecated
    vkDeviceCreateInfo.pEnabledFeatures = NULL;
    vkDeviceCreateInfo.queueCreateInfoCount = 1;
    vkDeviceCreateInfo.pQueueCreateInfos = &vkDeviceQueueCreateInfo;
        
    vkResult = vkCreateDevice(vkPhysicalDevice_selected,
                              &vkDeviceCreateInfo,
                              NULL,
                              &vkDevice);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "createVulkanDevice() --> vkCreateDevice() is failed %d\n", vkResult);
        return vkResult;
    }
    else
    {
        fprintf(gpFile, "createVulkanDevice() --> vkCreateDevice() is succedded\n");
    }                     
    
    return vkResult;
}


void getDeviceQueue(void)
{
    //code
    vkGetDeviceQueue(vkDevice, 
                     graphicsQueueFamilyIndex_selected,
                     0, //0th Queue index in that family queue
                     &vkQueue);
    if(vkQueue == VK_NULL_HANDLE) //rarest possibility
    {
        fprintf(gpFile, "getDeviceQueue() --> vkGetDeviceQueue() returned NULL for vkQueue\n");
        return;
    }
    else
    {
        fprintf(gpFile, "getDeviceQueue() --> vkGetDeviceQueue() is succedded\n");
    }
}

// VK.cpp
VkResult getPhysicalDeviceSurfaceFormatAndColorSpace(void)
{
    VkResult vkResult = VK_SUCCESS;
    uint32_t formatCount = 0;

    // Query formats
    vkResult = vkGetPhysicalDeviceSurfaceFormatsKHR(
        vkPhysicalDevice_selected, vkSurfaceKHR, &formatCount, NULL);
    if (vkResult != VK_SUCCESS || formatCount == 0) {
        fprintf(gpFile, "getPhysicalDeviceSurfaceFormatAndColorSpace() --> failed, count=%u, err=%d\n",
                formatCount, vkResult);
        return (vkResult == VK_SUCCESS) ? VK_ERROR_INITIALIZATION_FAILED : vkResult;
    }
    fprintf(gpFile, "getPhysicalDeviceSurfaceFormatAndColorSpace() --> total formatCount are:: %u\n",
            formatCount);

    // Fetch formats
    VkSurfaceFormatKHR* fmts = (VkSurfaceFormatKHR*)malloc(sizeof(VkSurfaceFormatKHR) * formatCount);
    vkResult = vkGetPhysicalDeviceSurfaceFormatsKHR(
        vkPhysicalDevice_selected, vkSurfaceKHR, &formatCount, fmts);
    if (vkResult != VK_SUCCESS) {
        fprintf(gpFile, "getPhysicalDeviceSurfaceFormatAndColorSpace() --> 2nd call failed %d\n",
                vkResult);
        free(fmts);
        return vkResult;
    }

    // Choose sRGB if possible
    VkSurfaceFormatKHR chosen = fmts[0];

    // Special-case: UNDEFINED means "you can choose"
    if (formatCount == 1 && fmts[0].format == VK_FORMAT_UNDEFINED) {
        chosen.format     = VK_FORMAT_B8G8R8A8_SRGB;
        chosen.colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
    } else {
        for (uint32_t i = 0; i < formatCount; ++i) {
            const VkFormat f = fmts[i].format;
            const VkColorSpaceKHR cs = fmts[i].colorSpace;
            if ((f == VK_FORMAT_B8G8R8A8_SRGB || f == VK_FORMAT_R8G8B8A8_SRGB) &&
                 cs == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
            {
                chosen = fmts[i];
                break;
            }
        }
    }

    vkFormat_color  = chosen.format;
    vkColorSpaceKHR = chosen.colorSpace;

    fprintf(gpFile, "getPhysicalDeviceSurfaceFormatAndColorSpace() --> Chosen format=%d, colorspace=%d\n",
            (int)vkFormat_color, (int)vkColorSpaceKHR);

    free(fmts);
    fprintf(gpFile, "getPhysicalDeviceSurfaceFormatAndColorSpace() --> vkSurfaceFormatKHR_array is freed\n");
    return VK_SUCCESS;
}

VkResult getPhysicalDevicePresentMode(void)
{
    //variable declarations
    VkResult vkResult = VK_SUCCESS;   
    
    uint32_t presentModeCount = 0;
    
    //code
    vkResult = vkGetPhysicalDeviceSurfacePresentModesKHR(vkPhysicalDevice_selected,
                                                         vkSurfaceKHR,
                                                         &presentModeCount,
                                                         NULL);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "getPhysicalDevicePresentMode() --> 1st call to vkGetPhysicalDeviceSurfacePresentModesKHR() is failed %d\n", vkResult);
        return vkResult;
    }
    else if(presentModeCount == 0)
    {
        fprintf(gpFile, "getPhysicalDevicePresentMode() --> 1st call to vkGetPhysicalDeviceSurfacePresentModesKHR() is failed as formatCount is zero:: %d\n", vkResult);
        return VK_ERROR_INITIALIZATION_FAILED;
    }
    else
    {
        fprintf(gpFile, "getPhysicalDevicePresentMode() --> 1st call to vkGetPhysicalDeviceSurfacePresentModesKHR() is succedded\n");
    }   

    fprintf(gpFile, "getPhysicalDevicePresentMode() --> total presentModeCount are:: %d\n", presentModeCount);

    VkPresentModeKHR* vkPresentModeKHR_array = (VkPresentModeKHR*) malloc(presentModeCount * sizeof(VkPresentModeKHR));
    //Malloc error checking
    
    vkResult = vkGetPhysicalDeviceSurfacePresentModesKHR(vkPhysicalDevice_selected,
                                                         vkSurfaceKHR,
                                                         &presentModeCount,
                                                         vkPresentModeKHR_array);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "getPhysicalDevicePresentMode() --> 2nd call to vkGetPhysicalDeviceSurfacePresentModesKHR() is failed %d\n", vkResult);
        return vkResult;
    }
    else
    {
        fprintf(gpFile, "getPhysicalDevicePresentMode() --> 2nd call to vkGetPhysicalDeviceSurfacePresentModesKHR() is succedded\n");
    }
    
    //Decide Presentation mode
    for(uint32_t i = 0; i < presentModeCount; i++)
    {
        if(vkPresentModeKHR_array[i] == VK_PRESENT_MODE_MAILBOX_KHR)
        {
            vkPresentModeKHR = VK_PRESENT_MODE_MAILBOX_KHR;
            fprintf(gpFile, "getPhysicalDevicePresentMode() --> vkPresentModeKHR is VK_PRESENT_MODE_MAILBOX_KHR\n");
            break;
        }
    }
    
    if(vkPresentModeKHR != VK_PRESENT_MODE_MAILBOX_KHR)
    {
        vkPresentModeKHR = VK_PRESENT_MODE_FIFO_KHR;
        fprintf(gpFile, "getPhysicalDevicePresentMode() --> vkPresentModeKHR is VK_PRESENT_MODE_FIFO_KHR\n");
    }
  
    
    if(vkPresentModeKHR_array)
    {
        free(vkPresentModeKHR_array);
        vkPresentModeKHR_array = NULL;
        fprintf(gpFile, "getPhysicalDevicePresentMode() --> vkPresentModeKHR_array is freed\n");
    }
    
    return vkResult;
  
}


VkResult createSwapchain(VkBool32 vsync)  // vertical sync
{
    //fucntion Declarations
    VkResult getPhysicalDeviceSurfaceFormatAndColorSpace(void);
    VkResult getPhysicalDevicePresentMode(void);
    
    //variables
    VkResult vkResult = VK_SUCCESS;
        
    //code
    //Color Format and ColorSpace
    vkResult = getPhysicalDeviceSurfaceFormatAndColorSpace();
    /*Main Points
        vkGetPhysicalDeviceSurfaceFormatsKHR()
    */
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "createSwapchain() --> getPhysicalDeviceSurfaceFormatAndColorSpace() is failed %d\n", vkResult);
        return vkResult;
    }
    else
    {
        fprintf(gpFile, "createSwapchain() --> getPhysicalDeviceSurfaceFormatAndColorSpace() is succedded\n");
    }
   
   
    //Step 2:
    VkSurfaceCapabilitiesKHR vkSurfaceCapabilitiesKHR;
    memset((void*)&vkSurfaceCapabilitiesKHR, 0, sizeof(VkSurfaceCapabilitiesKHR));
    
    vkResult = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(vkPhysicalDevice_selected,
                                                         vkSurfaceKHR,
                                                         &vkSurfaceCapabilitiesKHR);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "createSwapchain() --> vkGetPhysicalDeviceSurfaceCapabilitiesKHR() is failed %d\n", vkResult);
        return vkResult;
    }
    else
    {
        fprintf(gpFile, "createSwapchain() --> vkGetPhysicalDeviceSurfaceCapabilitiesKHR() is succedded\n");
    }
   
    //Step3: Find out desired swapchain image count
    uint32_t testingNumberOfSwapchainImages = vkSurfaceCapabilitiesKHR.minImageCount + 1;
    uint32_t desiredNumbeOfSwapchainImages = 0;
    
    
    if(vkSurfaceCapabilitiesKHR.maxImageCount > 0 && vkSurfaceCapabilitiesKHR.maxImageCount < testingNumberOfSwapchainImages)
    {
        desiredNumbeOfSwapchainImages = vkSurfaceCapabilitiesKHR.maxImageCount;
    }
    else
    {
        desiredNumbeOfSwapchainImages = vkSurfaceCapabilitiesKHR.minImageCount;
    }
    
    //Step4: Choose size of swapchain image
    memset((void*)&vkExtent2D_swapchain, 0, sizeof(VkExtent2D));
    if(vkSurfaceCapabilitiesKHR.currentExtent.width != UINT32_MAX)
    {
        vkExtent2D_swapchain.width = vkSurfaceCapabilitiesKHR.currentExtent.width;
        vkExtent2D_swapchain.height = vkSurfaceCapabilitiesKHR.currentExtent.height;
        
        fprintf(gpFile, "createSwapchain() --> Swapchain image width X height = %d X %d \n", vkExtent2D_swapchain.width, vkExtent2D_swapchain.height);
    }
    else
    {
        // if surface is already defined then swapchain image size must match with it
        VkExtent2D vkExtent2D;
        memset((void*)&vkExtent2D, 0 , sizeof(VkExtent2D));
        vkExtent2D.width = (uint32_t)winWidth;
        vkExtent2D.height = (uint32_t)winHeight;
        
        vkExtent2D_swapchain.width = glm::max(vkSurfaceCapabilitiesKHR.minImageExtent.width, glm::min(vkSurfaceCapabilitiesKHR.maxImageExtent.width, vkExtent2D.width));
        vkExtent2D_swapchain.height = glm::max(vkSurfaceCapabilitiesKHR.minImageExtent.height, glm::min(vkSurfaceCapabilitiesKHR.maxImageExtent.height, vkExtent2D.height));
        
        fprintf(gpFile, "createSwapchain() --> Swapchain image width X height = %d X %d \n", vkExtent2D_swapchain.width, vkExtent2D_swapchain.height);
    }
    
    //step5: Set SwapchainImageUsageFlag
    VkImageUsageFlags vkImageUsageFlags = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT/*Texture, Compute, FBO*/; 
    
    //enum
    VkSurfaceTransformFlagBitsKHR vkSurfaceTransformFlagBitsKHR;
    if(vkSurfaceCapabilitiesKHR.supportedTransforms & VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR)
    {
        vkSurfaceTransformFlagBitsKHR = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
    }
    else
    {
        vkSurfaceTransformFlagBitsKHR = vkSurfaceCapabilitiesKHR.currentTransform;
    }
   
    
    //Step 7: Presentation mode
    vkResult = getPhysicalDevicePresentMode();
    /*Main Points
        
    */
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "createSwapchain() --> getPhysicalDevicePresentMode() is failed %d\n", vkResult);
        return vkResult;
    }
    else
    {
        fprintf(gpFile, "createSwapchain() --> getPhysicalDevicePresentMode() is succedded\n");
    }
    
    //Step 8: Initialie vkCreateSwapchinCreateInfoStructure
    VkSwapchainCreateInfoKHR vkSwapchainCreateInfoKHR;
    memset((void*)&vkSwapchainCreateInfoKHR, 0 , sizeof(VkSwapchainCreateInfoKHR));
    vkSwapchainCreateInfoKHR.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    vkSwapchainCreateInfoKHR.pNext = NULL;
    vkSwapchainCreateInfoKHR.flags = 0;
    vkSwapchainCreateInfoKHR.surface = vkSurfaceKHR;
    vkSwapchainCreateInfoKHR.minImageCount = desiredNumbeOfSwapchainImages;
    vkSwapchainCreateInfoKHR.imageFormat = vkFormat_color;
    vkSwapchainCreateInfoKHR.imageColorSpace = vkColorSpaceKHR;
    vkSwapchainCreateInfoKHR.imageExtent.width = vkExtent2D_swapchain.width;
    vkSwapchainCreateInfoKHR.imageExtent.height = vkExtent2D_swapchain.height;
    vkSwapchainCreateInfoKHR.imageUsage = vkImageUsageFlags;
    vkSwapchainCreateInfoKHR.preTransform = vkSurfaceTransformFlagBitsKHR;
    vkSwapchainCreateInfoKHR.imageArrayLayers = 1;
    vkSwapchainCreateInfoKHR.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    vkSwapchainCreateInfoKHR.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    vkSwapchainCreateInfoKHR.presentMode = vkPresentModeKHR;
    vkSwapchainCreateInfoKHR.clipped = VK_TRUE;
    
    //Step9:
    vkResult = vkCreateSwapchainKHR(vkDevice,
                                    &vkSwapchainCreateInfoKHR,
                                    NULL,
                                    &vkSwapchainKHR);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "createSwapchain() --> vkCreateSwapchainKHR() is failed %d\n", vkResult);
        return vkResult;
    }
    else
    {
        fprintf(gpFile, "createSwapchain() --> vkCreateSwapchainKHR() is succedded\n");
    }
    
    return vkResult;
}

VkResult createImagesAndImageViews(void)
{
    //fuction  declarations
    VkResult GetSupportedDepthFormat(void);
    
    //variables
    VkResult vkResult = VK_SUCCESS;
    
    //step1: Get desired SwapchainImage count
    vkResult = vkGetSwapchainImagesKHR(vkDevice, 
                                       vkSwapchainKHR,
                                       &swapchainImageCount,
                                       NULL);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "createImagesAndImageViews() --> 1st call to vkGetSwapchainImagesKHR() is failed %d\n", vkResult);
        return vkResult;
    }
    else if(0 == swapchainImageCount)
    {
        fprintf(gpFile, "createImagesAndImageViews() --> 1st call to vkGetSwapchainImagesKHR() returned zero images %d\n", vkResult);
        vkResult = VK_ERROR_INITIALIZATION_FAILED;
        return vkResult;
    }
    else
    {
        fprintf(gpFile, "createImagesAndImageViews() --> gives swapchainImagecount = %d\n", swapchainImageCount);
        fprintf(gpFile, "createImagesAndImageViews() --> vkGetSwapchainImagesKHR() is succedded\n");
    }
    
    //step2: Allocate the swapchain Image array
    swapchainImage_array = (VkImage*)malloc(sizeof(VkImage) * swapchainImageCount);
    //malloc check to be done

    //step3: fill this array by swapchain images
    vkResult = vkGetSwapchainImagesKHR(vkDevice, 
                                       vkSwapchainKHR,
                                       &swapchainImageCount,
                                       swapchainImage_array);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "createImagesAndImageViews() --> 2nd call to vkGetSwapchainImagesKHR() is failed %d\n", vkResult);
        return vkResult;
    }
    else
    {
        fprintf(gpFile, "createImagesAndImageViews() --> 2nd call to vkGetSwapchainImagesKHR() is succedded\n");
    }
    
    //step4: allocate array of swapchainImageViews   
    swapchainImageView_array = (VkImageView*)malloc(sizeof(VkImageView) * swapchainImageCount);
    //malloc check to be done
    
    //step5: Initialize vkImageViewCreateInfo structure
    VkImageViewCreateInfo vkImageViewCreateInfo;
    memset((void*)&vkImageViewCreateInfo, 0, sizeof(VkImageViewCreateInfo));

    vkImageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    vkImageViewCreateInfo.pNext = NULL;
    vkImageViewCreateInfo.flags = 0;
    vkImageViewCreateInfo.format = vkFormat_color;
    vkImageViewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_R;
    vkImageViewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_G;
    vkImageViewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_B;
    vkImageViewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_A;

    // ***** FIX: color images must use COLOR aspect (not depth/stencil) *****
    vkImageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

    vkImageViewCreateInfo.subresourceRange.baseMipLevel = 0;
    vkImageViewCreateInfo.subresourceRange.levelCount = 1;
    vkImageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
    vkImageViewCreateInfo.subresourceRange.layerCount = 1;
    vkImageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    
    //Step6: Fill Imageview Array by using above struct
    for(uint32_t i = 0; i < swapchainImageCount; i++)
    {
        vkImageViewCreateInfo.image = swapchainImage_array[i];
        
        vkResult = vkCreateImageView(vkDevice,
                                     &vkImageViewCreateInfo,
                                     NULL,
                                     &swapchainImageView_array[i]);
        if(vkResult != VK_SUCCESS)
        {
            fprintf(gpFile, "createImagesAndImageViews() --> vkCreateImageView() failed for swapchain image %d, error %d\n", i, vkResult);
            return vkResult;
        }
        else
        {
            fprintf(gpFile, "createImagesAndImageViews() --> vkCreateImageView() succedded for iteration %d\n", i);
        }
    }
    
    //for depth image
    vkResult = GetSupportedDepthFormat();
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "createImagesAndImageViews() --> GetSupportedDepthFormat() is failed error code is %d\n",vkResult);
        return vkResult;
    }
    else
    {
        fprintf(gpFile, "createImagesAndImageViews() --> GetSupportedDepthFormat() is succedded\n");
    }
    
    //for depth image Initialize VkImageCreateInfo
    VkImageCreateInfo vkImageCreateInfo;
    memset((void*)&vkImageCreateInfo, 0, sizeof(VkImageCreateInfo));
    vkImageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    vkImageCreateInfo.pNext = NULL;
    vkImageCreateInfo.flags = 0;
    vkImageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
    vkImageCreateInfo.format = vkFormat_depth;
    vkImageCreateInfo.extent.width = winWidth;
    vkImageCreateInfo.extent.height = winHeight;
    vkImageCreateInfo.extent.depth = 1;
    vkImageCreateInfo.mipLevels = 1;
    vkImageCreateInfo.arrayLayers = 1;
    vkImageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    vkImageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    vkImageCreateInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;

    vkResult = vkCreateImage(vkDevice, &vkImageCreateInfo, NULL, &vkImage_depth);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "createImagesAndImageViews() --> vkCreateImage() is failed error code is %d\n",vkResult);
        fflush(gpFile);
        return vkResult;
    }
    else
    {
        fprintf(gpFile, "createImagesAndImageViews() --> vkCreateImage() is succedded\n");
    }
    
    //Memory requirement for depth image
    VkMemoryRequirements vkMemoryRequirements;
    memset((void*)&vkMemoryRequirements, 0, sizeof(VkMemoryRequirements));
    vkGetImageMemoryRequirements(vkDevice, vkImage_depth, &vkMemoryRequirements);
    
    //Allocate
    VkMemoryAllocateInfo vkMemoryAllocateInfo;
    memset((void*)&vkMemoryAllocateInfo , 0, sizeof(VkMemoryAllocateInfo));
    vkMemoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    vkMemoryAllocateInfo.pNext = NULL; 
    vkMemoryAllocateInfo.allocationSize = vkMemoryRequirements.size;
    vkMemoryAllocateInfo.memoryTypeIndex = 0;
   
    for(uint32_t i = 0; i < vkPhysicalDeviceMemoryProperties.memoryTypeCount; i++)
    {
        if((vkMemoryRequirements.memoryTypeBits & 1) == 1)
        {
            if(vkPhysicalDeviceMemoryProperties.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
            {
                vkMemoryAllocateInfo.memoryTypeIndex = i;
                break;
            }
        }
        vkMemoryRequirements.memoryTypeBits >>= 1;
    }
    
    vkResult = vkAllocateMemory(vkDevice, &vkMemoryAllocateInfo, NULL, &vkDeviceMemory_depth);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "createImagesAndImageViews() --> vkAllocateMemory() is failed and error code is %d\n", vkResult);
        return vkResult;
    }
    else
    {
        fprintf(gpFile, "createImagesAndImageViews() --> vkAllocateMemory() is succedded\n");
    }
    
    // Bind device-local memory to the depth image
    vkResult = vkBindImageMemory(vkDevice, vkImage_depth, vkDeviceMemory_depth, 0);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "createImagesAndImageViews() --> vkBindImageMemory() is failed and error code is %d\n", vkResult);
        return vkResult;
    }
    else
    {
        fprintf(gpFile, "createImagesAndImageViews() --> vkBindImageMemory() is succedded\n");
    }
    
    //create image view for above depth image
    memset((void*)&vkImageViewCreateInfo, 0, sizeof(VkImageViewCreateInfo));
    vkImageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    vkImageViewCreateInfo.pNext = NULL;
    vkImageViewCreateInfo.flags = 0;
    vkImageViewCreateInfo.format = vkFormat_depth;

    // ***** FIX: depth aspect (add STENCIL only if depth format has stencil) *****
    VkImageAspectFlags depthAspect = VK_IMAGE_ASPECT_DEPTH_BIT;
    if (vkFormat_depth == VK_FORMAT_D32_SFLOAT_S8_UINT ||
        vkFormat_depth == VK_FORMAT_D24_UNORM_S8_UINT ||
        vkFormat_depth == VK_FORMAT_D16_UNORM_S8_UINT)
    {
        depthAspect |= VK_IMAGE_ASPECT_STENCIL_BIT;
    }
    vkImageViewCreateInfo.subresourceRange.aspectMask = depthAspect;

    vkImageViewCreateInfo.subresourceRange.baseMipLevel = 0;
    vkImageViewCreateInfo.subresourceRange.levelCount = 1;
    vkImageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
    vkImageViewCreateInfo.subresourceRange.layerCount = 1;
    vkImageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    vkImageViewCreateInfo.image = vkImage_depth;
    
    vkResult = vkCreateImageView(vkDevice,
                                 &vkImageViewCreateInfo,
                                 NULL,
                                 &vkImageView_depth);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "createImagesAndImageViews() --> vkCreateImageView() for depth failed, error %d\n",vkResult);
        return vkResult;
    }
    else
    {
        fprintf(gpFile, "createImagesAndImageViews() --> vkCreateImageView() for depth succedded\n");
    }
    
    return vkResult;
}

VkResult GetSupportedDepthFormat(void)
{
    //code
    //variables
    VkResult vkResult = VK_SUCCESS;
    VkFormat vkFormat_depth_array[] = { VK_FORMAT_D32_SFLOAT_S8_UINT, 
                                       VK_FORMAT_D32_SFLOAT,
                                       VK_FORMAT_D24_UNORM_S8_UINT,
                                       VK_FORMAT_D16_UNORM_S8_UINT,
                                       VK_FORMAT_D16_UNORM };
    
    for(uint32_t i = 0; i < (sizeof(vkFormat_depth_array)/sizeof(vkFormat_depth_array[0])); i++)
    {
        VkFormatProperties vkFormatProperties;
        memset((void*)&vkFormatProperties, 0, sizeof(VkFormatProperties));
        
        vkGetPhysicalDeviceFormatProperties(vkPhysicalDevice_selected, vkFormat_depth_array[i], &vkFormatProperties);
        
        if(vkFormatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT)
        {
           vkFormat_depth = vkFormat_depth_array[i];
           vkResult = VK_SUCCESS;
           break;
        }
    }
    
   return vkResult;
}


VkResult createCommandPool(void)
{
    //variables
   VkResult vkResult = VK_SUCCESS;
    
   //code
   VkCommandPoolCreateInfo vkCommandPoolCreateInfo;
   memset((void*)&vkCommandPoolCreateInfo, 0, sizeof(VkCommandPoolCreateInfo));
   
   vkCommandPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
   vkCommandPoolCreateInfo.pNext = NULL;
   vkCommandPoolCreateInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT; //such command buffers will be resetted and resatrted, and these command buffers are long lived
   vkCommandPoolCreateInfo.queueFamilyIndex = graphicsQueueFamilyIndex_selected;
   
   vkResult = vkCreateCommandPool(vkDevice, 
                                  &vkCommandPoolCreateInfo,
                                  NULL,
                                  &vkCommandPool);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "createCommandPool() --> vkCreateCommandPool() is failed and error code is %d\n",vkResult);
        return vkResult;
    }
    else
    {
        fprintf(gpFile, "createCommandPool() --> vkCreateCommandPool() is succedded \n");
    }

   return vkResult;   
}


VkResult createCommandBuffers(void)
{
   //variables
   VkResult vkResult = VK_SUCCESS;
   
   //code
   //vkCommandBuffer allocate info structure initliazation
   VkCommandBufferAllocateInfo vkCommandBufferAllocateInfo; 
   memset((void*)&vkCommandBufferAllocateInfo, 0, sizeof(VkCommandBufferAllocateInfo));
   
   vkCommandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
   vkCommandBufferAllocateInfo.pNext = NULL;
   vkCommandBufferAllocateInfo.commandPool = vkCommandPool;
   vkCommandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
   vkCommandBufferAllocateInfo.commandBufferCount = 1;
   
   vkCommandBuffer_array = (VkCommandBuffer*)malloc(sizeof(VkCommandBuffer) * swapchainImageCount);
   //malloc check to be done
   
   for(uint32_t i = 0; i < swapchainImageCount; i++)
    {
        vkResult = vkAllocateCommandBuffers(vkDevice, &vkCommandBufferAllocateInfo, &vkCommandBuffer_array[i]);
        if(vkResult != VK_SUCCESS)
        {
            fprintf(gpFile, "createCommandBuffers() --> vkAllocateCommandBuffers() is failed for %d iteration and error code is %d\n",i, vkResult);
            return vkResult;
        }
        else
        {
            fprintf(gpFile, "createCommandBuffers() --> vkAllocateCommandBuffers() is succedded for iteration %d\n", i);
        }
    }
   
   return vkResult;   
}

VkResult createVertexBuffer(void)
{
    //variable
    VkResult vkResult = VK_SUCCESS;

    // -------- Sphere parameters (adjust for smoothness if you like) --------
    const uint32_t STACKS  = 48;   // latitude  (>= 2)
    const uint32_t SLICES  = 64;   // longitude (>= 3)
    const float    RADIUS  = 1.0f;
    const float    PI      = 3.14159265358979323846f;
    const float    TWO_PI  = 6.28318530717958647692f;

    // We build a triangle list (two triangles per quad). No index buffer.
    std::vector<float> positions;
    std::vector<float> uvs;
    positions.reserve(STACKS * SLICES * 6 * 3);
    uvs.reserve      (STACKS * SLICES * 6 * 2);

    for (uint32_t i = 0; i < STACKS; ++i)
    {
        float theta0 = (float)i       * PI / (float)STACKS;     // [0, PI)
        float theta1 = (float)(i + 1) * PI / (float)STACKS;     // (0, PI]

        float sin0 = std::sin(theta0), cos0 = std::cos(theta0);
        float sin1 = std::sin(theta1), cos1 = std::cos(theta1);

        for (uint32_t j = 0; j < SLICES; ++j)
        {
            float phi0 = (float)j       * TWO_PI / (float)SLICES; // [0, 2PI)
            float phi1 = (float)(j + 1) * TWO_PI / (float)SLICES; // (0, 2PI]

            float c0 = std::cos(phi0), s0 = std::sin(phi0);
            float c1 = std::cos(phi1), s1 = std::sin(phi1);

            // Quad corners on the unit sphere (then scaled by RADIUS)
            // p0 ----- p3   (theta0)
            //  |     / |
            //  |   /   |
            //  | /     |
            // p1 ----- p2   (theta1)

            glm::vec3 p0(RADIUS * sin0 * c0, RADIUS * cos0, RADIUS * sin0 * s0);
            glm::vec3 p1(RADIUS * sin1 * c0, RADIUS * cos1, RADIUS * sin1 * s0);
            glm::vec3 p2(RADIUS * sin1 * c1, RADIUS * cos1, RADIUS * sin1 * s1);
            glm::vec3 p3(RADIUS * sin0 * c1, RADIUS * cos0, RADIUS * sin0 * s1);

            // UVs (wrap in U, north pole v=0, south pole v=1)
            glm::vec2 t0(phi0 / TWO_PI, 1.0f - theta0 / PI);
            glm::vec2 t1(phi0 / TWO_PI, 1.0f - theta1 / PI);
            glm::vec2 t2(phi1 / TWO_PI, 1.0f - theta1 / PI);
            glm::vec2 t3(phi1 / TWO_PI, 1.0f - theta0 / PI);

            // Tri 1: p0, p1, p2  (counter‑clockwise)
            positions.insert(positions.end(), { p0.x, p0.y, p0.z, p1.x, p1.y, p1.z, p2.x, p2.y, p2.z });
            uvs.insert(uvs.end(),           { t0.x, t0.y,        t1.x, t1.y,        t2.x, t2.y        });

            // Tri 2: p0, p2, p3
            positions.insert(positions.end(), { p0.x, p0.y, p0.z, p2.x, p2.y, p2.z, p3.x, p3.y, p3.z });
            uvs.insert(uvs.end(),           { t0.x, t0.y,        t2.x, t2.y,        t3.x, t3.y        });
        }
    }

    gSphereVertexCount = static_cast<uint32_t>(positions.size() / 3);

    // ---------------- create & fill POSITION buffer (binding 0) ----------------
    memset((void*)&vertexData_position, 0, sizeof(VertexData));

    VkBufferCreateInfo vkBufferCreateInfo;
    memset((void*)&vkBufferCreateInfo , 0, sizeof(VkBufferCreateInfo));
    vkBufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    vkBufferCreateInfo.pNext = NULL;
    vkBufferCreateInfo.flags = 0;
    vkBufferCreateInfo.size  = positions.size() * sizeof(float);
    vkBufferCreateInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;

    vkResult = vkCreateBuffer(vkDevice, &vkBufferCreateInfo, NULL, &vertexData_position.vkBuffer);
    if (vkResult != VK_SUCCESS) return vkResult;

    VkMemoryRequirements vkMemoryRequirements;
    vkGetBufferMemoryRequirements(vkDevice, vertexData_position.vkBuffer, &vkMemoryRequirements);

    VkMemoryAllocateInfo vkMemoryAllocateInfo;
    memset((void*)&vkMemoryAllocateInfo , 0, sizeof(VkMemoryAllocateInfo));
    vkMemoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    vkMemoryAllocateInfo.pNext = NULL;
    vkMemoryAllocateInfo.allocationSize = vkMemoryRequirements.size;
    vkMemoryAllocateInfo.memoryTypeIndex = 0;

    for (uint32_t i = 0; i < vkPhysicalDeviceMemoryProperties.memoryTypeCount; i++)
    {
        if ((vkMemoryRequirements.memoryTypeBits & 1) == 1)
        {
            if (vkPhysicalDeviceMemoryProperties.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)
            {
                vkMemoryAllocateInfo.memoryTypeIndex = i;
                break;
            }
        }
        vkMemoryRequirements.memoryTypeBits >>= 1;
    }

    vkResult = vkAllocateMemory(vkDevice, &vkMemoryAllocateInfo, NULL, &vertexData_position.vkDeviceMemory);
    if (vkResult != VK_SUCCESS) return vkResult;

    vkResult = vkBindBufferMemory(vkDevice, vertexData_position.vkBuffer, vertexData_position.vkDeviceMemory, 0);
    if (vkResult != VK_SUCCESS) return vkResult;

    void* data = NULL;
    vkResult = vkMapMemory(vkDevice, vertexData_position.vkDeviceMemory, 0,
                           vkBufferCreateInfo.size, 0, &data);
    if (vkResult != VK_SUCCESS) return vkResult;

    memcpy(data, positions.data(), vkBufferCreateInfo.size);
    vkUnmapMemory(vkDevice, vertexData_position.vkDeviceMemory);

    // ---------------- create & fill TEXCOORD buffer (binding 1) ----------------
    memset((void*)&vertexData_texcoord, 0, sizeof(VertexData));

    memset((void*)&vkBufferCreateInfo , 0, sizeof(VkBufferCreateInfo));
    vkBufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    vkBufferCreateInfo.size  = uvs.size() * sizeof(float);
    vkBufferCreateInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;

    vkResult = vkCreateBuffer(vkDevice, &vkBufferCreateInfo, NULL, &vertexData_texcoord.vkBuffer);
    if (vkResult != VK_SUCCESS) return vkResult;

    memset((void*)&vkMemoryRequirements, 0, sizeof(VkMemoryRequirements));
    vkGetBufferMemoryRequirements(vkDevice, vertexData_texcoord.vkBuffer, &vkMemoryRequirements);

    memset((void*)&vkMemoryAllocateInfo , 0, sizeof(VkMemoryAllocateInfo));
    vkMemoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    vkMemoryAllocateInfo.allocationSize = vkMemoryRequirements.size;
    vkMemoryAllocateInfo.memoryTypeIndex = 0;

    for (uint32_t i = 0; i < vkPhysicalDeviceMemoryProperties.memoryTypeCount; i++)
    {
        if ((vkMemoryRequirements.memoryTypeBits & 1) == 1)
        {
            if (vkPhysicalDeviceMemoryProperties.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)
            {
                vkMemoryAllocateInfo.memoryTypeIndex = i;
                break;
            }
        }
        vkMemoryRequirements.memoryTypeBits >>= 1;
    }

    vkResult = vkAllocateMemory(vkDevice, &vkMemoryAllocateInfo, NULL, &vertexData_texcoord.vkDeviceMemory);
    if (vkResult != VK_SUCCESS) return vkResult;

    vkResult = vkBindBufferMemory(vkDevice, vertexData_texcoord.vkBuffer, vertexData_texcoord.vkDeviceMemory, 0);
    if (vkResult != VK_SUCCESS) return vkResult;

    data = NULL;
    vkResult = vkMapMemory(vkDevice, vertexData_texcoord.vkDeviceMemory, 0,
                           vkBufferCreateInfo.size, 0, &data);
    if (vkResult != VK_SUCCESS) return vkResult;

    memcpy(data, uvs.data(), vkBufferCreateInfo.size);
    vkUnmapMemory(vkDevice, vertexData_texcoord.vkDeviceMemory);

    return vkResult;
}

VkResult createTexture(const char* textureFileName)
{
    //variable
    VkResult vkResult = VK_SUCCESS;
    
    //code
    //step 1
    FILE* fp = NULL;
    fp = fopen(textureFileName, "rb");
    if(fp == NULL)
    {
        fprintf(gpFile, "createTexture() --> fOpen() failed to open Stone.png texture file\n");
        return VK_ERROR_OUT_OF_HOST_MEMORY;
    }
    
    uint8_t* imageData = NULL;
    int texture_width, texture_height, texture_channels;

    imageData = stbi_load_from_file(fp, &texture_width, &texture_height, &texture_channels, STBI_rgb_alpha);
    if(imageData == NULL || texture_width <= 0 || texture_height <= 0 || texture_channels <= 0)
    {
        fprintf(gpFile, "createTexture() --> stbi_load_from_file() failed to read Stone.png texture file\n");
        return VK_ERROR_OUT_OF_HOST_MEMORY;
    }
    
    VkDeviceSize image_size = (VkDeviceSize)texture_width * (VkDeviceSize)texture_height * 4 /*RGBA*/ ;
    
    //step 2: staging buffer
    VkBuffer vkBuffer_stagingBuffer = VK_NULL_HANDLE;
    VkDeviceMemory vkDeviceMemory_stagingBuffer = VK_NULL_HANDLE;
    
    VkBufferCreateInfo vkBufferCreateInfo_stagingBuffer;
    memset((void*)&vkBufferCreateInfo_stagingBuffer, 0, sizeof(VkBufferCreateInfo));    
    vkBufferCreateInfo_stagingBuffer.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    vkBufferCreateInfo_stagingBuffer.pNext = NULL;
    vkBufferCreateInfo_stagingBuffer.flags = 0;
    vkBufferCreateInfo_stagingBuffer.size = image_size;
    vkBufferCreateInfo_stagingBuffer.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT; // this buffer is source
    vkBufferCreateInfo_stagingBuffer.sharingMode = VK_SHARING_MODE_EXCLUSIVE; 
    
    vkResult = vkCreateBuffer(vkDevice, 
                              &vkBufferCreateInfo_stagingBuffer,
                              NULL,
                              &vkBuffer_stagingBuffer);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "createTexture() --> vkCreateBuffer() is failed and error code is %d\n", vkResult);
        return vkResult;
    }
    else
    {
        fprintf(gpFile, "createTexture() --> vkCreateBuffer() is succedded\n");
    }
    
    VkMemoryRequirements vkMemoryRequirements_stagingBuffer;
    memset((void*)&vkMemoryRequirements_stagingBuffer, 0, sizeof(VkMemoryRequirements));
    vkGetBufferMemoryRequirements(vkDevice, vkBuffer_stagingBuffer, &vkMemoryRequirements_stagingBuffer);
    
    VkMemoryAllocateInfo vkMemoryAllocateInfo_stagingBuffer;
    memset((void*)&vkMemoryAllocateInfo_stagingBuffer , 0, sizeof(VkMemoryAllocateInfo));
    vkMemoryAllocateInfo_stagingBuffer.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    vkMemoryAllocateInfo_stagingBuffer.pNext = NULL; 
    vkMemoryAllocateInfo_stagingBuffer.allocationSize = vkMemoryRequirements_stagingBuffer.size;
    vkMemoryAllocateInfo_stagingBuffer.memoryTypeIndex = 0;
    
    for(uint32_t i = 0; i < vkPhysicalDeviceMemoryProperties.memoryTypeCount; i++)
    {
        if((vkMemoryRequirements_stagingBuffer.memoryTypeBits & 1) == 1)
        {
            if(vkPhysicalDeviceMemoryProperties.memoryTypes[i].propertyFlags & (VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT))
            {
                vkMemoryAllocateInfo_stagingBuffer.memoryTypeIndex = i;
                break;
            }
        }
        vkMemoryRequirements_stagingBuffer.memoryTypeBits >>= 1;
    }
    
    vkResult = vkAllocateMemory(vkDevice, &vkMemoryAllocateInfo_stagingBuffer, NULL, &vkDeviceMemory_stagingBuffer);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "createTexture() --> vkAllocateMemory() is failed and error code is %d\n", vkResult);
        return vkResult;
    }
    else
    {
        fprintf(gpFile, "createTexture() --> vkAllocateMemory() is succedded\n");
    }
    
    vkResult = vkBindBufferMemory(vkDevice, vkBuffer_stagingBuffer, vkDeviceMemory_stagingBuffer, 0);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "createTexture() --> vkBindBufferMemory() is failed and error code is %d\n", vkResult);
        return vkResult;
    }
    else
    {
        fprintf(gpFile, "createTexture() --> vkBindBufferMemory() is succedded\n");
    }
    
    void* data = NULL;
    vkResult = vkMapMemory(vkDevice, vkDeviceMemory_stagingBuffer, 0, image_size, 0, &data);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "createTexture() --> vkMapMemory() is failed and error code is %d\n", vkResult);
        return vkResult;
    }
    else
    {
        fprintf(gpFile, "createTexture() --> vkMapMemory() is succedded\n");
    }
    
    memcpy(data, imageData, (size_t)image_size);
    vkUnmapMemory(vkDevice, vkDeviceMemory_stagingBuffer);
    
    stbi_image_free(imageData);
    imageData = NULL;
    fprintf(gpFile, "createTexture() --> stbi_image_free() Freeing of image data is succedded\n");
    
    // Step#3: device‑local image
    VkImageCreateInfo vkImageCreateInfo;
    memset((void*)&vkImageCreateInfo, 0, sizeof(VkImageCreateInfo));
    vkImageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    vkImageCreateInfo.pNext = NULL;
    vkImageCreateInfo.flags = 0;
    vkImageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
    vkImageCreateInfo.format = VK_FORMAT_R8G8B8A8_UNORM;
    vkImageCreateInfo.extent.width = texture_width;
    vkImageCreateInfo.extent.height = texture_height;
    vkImageCreateInfo.extent.depth = 1;
    vkImageCreateInfo.mipLevels = 1;
    vkImageCreateInfo.arrayLayers = 1;
    vkImageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    vkImageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    vkImageCreateInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    vkImageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    vkImageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

    vkResult = vkCreateImage(vkDevice, &vkImageCreateInfo, NULL, &vkImage_texture);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "createTexture() --> vkCreateImage() is failed error code is %d\n",vkResult);
        fflush(gpFile);
        return vkResult;
    }
    else
    {
        fprintf(gpFile, "createTexture() --> vkCreateImage() is succedded\n");
        fflush(gpFile);
    }
    
    // Memory for image
    VkMemoryRequirements vkMemoryRequirements_image;
    memset((void*)&vkMemoryRequirements_image, 0, sizeof(VkMemoryRequirements));
    vkGetImageMemoryRequirements(vkDevice, vkImage_texture, &vkMemoryRequirements_image);
    
    VkMemoryAllocateInfo vkMemoryAllocateInfo_image;
    memset((void*)&vkMemoryAllocateInfo_image , 0, sizeof(VkMemoryAllocateInfo));
    vkMemoryAllocateInfo_image.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    vkMemoryAllocateInfo_image.pNext = NULL; 
    vkMemoryAllocateInfo_image.allocationSize = vkMemoryRequirements_image.size;
    vkMemoryAllocateInfo_image.memoryTypeIndex = 0;
   
    for(uint32_t i = 0; i < vkPhysicalDeviceMemoryProperties.memoryTypeCount; i++)
    {
        if((vkMemoryRequirements_image.memoryTypeBits & 1) == 1)
        {
            if(vkPhysicalDeviceMemoryProperties.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
            {
                vkMemoryAllocateInfo_image.memoryTypeIndex = i;
                break;
            }
        }
        vkMemoryRequirements_image.memoryTypeBits >>= 1;
    }
    
    vkResult = vkAllocateMemory(vkDevice, &vkMemoryAllocateInfo_image, NULL, &vkDeviceMemory_texture);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "createTexture() --> vkAllocateMemory() is failed and error code is %d\n", vkResult);
        fflush(gpFile);
        return vkResult;
    }
    else
    {
        fprintf(gpFile, "createTexture() --> vkAllocateMemory() is succedded\n");
        fflush(gpFile);
    }
    
    vkResult = vkBindImageMemory(vkDevice, vkImage_texture, vkDeviceMemory_texture, 0);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "createTexture() --> vkBindImageMemory() is failed and error code is %d\n", vkResult);
        fflush(gpFile);
        return vkResult;
    }
    else
    {
        fprintf(gpFile, "createTexture() --> vkBindImageMemory() is succedded\n");
        fflush(gpFile);
    }
    
    // Step#4: transition UNDEFINED -> TRANSFER_DST_OPTIMAL
    VkCommandBufferAllocateInfo vkCommandBufferAllocateInfo_transition_image_layout; 
    memset((void*)&vkCommandBufferAllocateInfo_transition_image_layout, 0, sizeof(VkCommandBufferAllocateInfo));
    vkCommandBufferAllocateInfo_transition_image_layout.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    vkCommandBufferAllocateInfo_transition_image_layout.pNext = NULL;
    vkCommandBufferAllocateInfo_transition_image_layout.commandPool = vkCommandPool;
    vkCommandBufferAllocateInfo_transition_image_layout.commandBufferCount = 1;
    vkCommandBufferAllocateInfo_transition_image_layout.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
   
    VkCommandBuffer vkCommandBuffer_transition_image_layout = VK_NULL_HANDLE;
    vkResult = vkAllocateCommandBuffers(vkDevice, &vkCommandBufferAllocateInfo_transition_image_layout, &vkCommandBuffer_transition_image_layout);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "createTexture() --> vkAllocateCommandBuffers() is failed and error code is %d\n", vkResult);
        fflush(gpFile);
        return vkResult;
    }
    else
    {
        fprintf(gpFile, "createTexture() --> vkAllocateCommandBuffers() is succedded\n");
        fflush(gpFile);
    }

    VkCommandBufferBeginInfo vkCommandBufferBeginInfo_transition_image_layout;
    memset((void*)&vkCommandBufferBeginInfo_transition_image_layout, 0, sizeof(VkCommandBufferBeginInfo));
    vkCommandBufferBeginInfo_transition_image_layout.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    vkCommandBufferBeginInfo_transition_image_layout.pNext = NULL;
    vkCommandBufferBeginInfo_transition_image_layout.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    
    vkResult = vkBeginCommandBuffer(vkCommandBuffer_transition_image_layout, &vkCommandBufferBeginInfo_transition_image_layout);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "createTexture() --> vkBeginCommandBuffer() is failed and error code is %d\n", vkResult);
        fflush(gpFile);
        return vkResult;
    }
    else
    {
        fprintf(gpFile, "createTexture() --> vkBeginCommandBuffer() is succedded \n");
        fflush(gpFile);
    }

    VkPipelineStageFlags vkPipelineStageFlags_source = 0;
    VkPipelineStageFlags vkPipelineStageFlags_destination = 0;
    VkImageMemoryBarrier vkImageMemoryBarrier;
    memset((void*)&vkImageMemoryBarrier, 0, sizeof(VkImageMemoryBarrier));
    vkImageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    vkImageMemoryBarrier.pNext = NULL;
    vkImageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    vkImageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    vkImageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    vkImageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    vkImageMemoryBarrier.image = vkImage_texture;
    vkImageMemoryBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT; // color
    vkImageMemoryBarrier.subresourceRange.baseArrayLayer = 0;
    vkImageMemoryBarrier.subresourceRange.baseMipLevel = 0;
    vkImageMemoryBarrier.subresourceRange.layerCount = 1;
    vkImageMemoryBarrier.subresourceRange.levelCount = 1;
    
    vkImageMemoryBarrier.srcAccessMask = 0;
    vkImageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    vkPipelineStageFlags_source = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
    vkPipelineStageFlags_destination = VK_PIPELINE_STAGE_TRANSFER_BIT;
        
    vkCmdPipelineBarrier(vkCommandBuffer_transition_image_layout,
                         vkPipelineStageFlags_source,
                         vkPipelineStageFlags_destination,
                         0, 0, NULL, 0, NULL, 1, &vkImageMemoryBarrier);
    
    vkResult = vkEndCommandBuffer(vkCommandBuffer_transition_image_layout);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "createTexture() --> vkEndCommandBuffer() is failed and error code is %d\n", vkResult);
        return vkResult;
    }
    else
    {
        fprintf(gpFile, "createTexture() --> vkEndCommandBuffer() is succedded\n");
    }
    
    VkSubmitInfo  vkSubmitInfo_transition_image_layout;
    memset((void*)&vkSubmitInfo_transition_image_layout, 0, sizeof(VkSubmitInfo));
    vkSubmitInfo_transition_image_layout.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    vkSubmitInfo_transition_image_layout.pNext = NULL;
    vkSubmitInfo_transition_image_layout.commandBufferCount = 1;
    vkSubmitInfo_transition_image_layout.pCommandBuffers = &vkCommandBuffer_transition_image_layout;
    
    vkResult = vkQueueSubmit(vkQueue, 1, &vkSubmitInfo_transition_image_layout, VK_NULL_HANDLE);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "createTexture() --> vkQueueSubmit() is failed errorcode = %d\n", vkResult);
        return vkResult;
    }
    else
    {
        fprintf(gpFile, "createTexture() --> vkQueueSubmit() succeded\n");
    }

    vkResult = vkQueueWaitIdle(vkQueue);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "createTexture() --> vkQueueWaitIdle() is failed errorcode = %d\n", vkResult);
        return vkResult;
    }
    else
    {
        fprintf(gpFile, "createTexture() --> vkQueueWaitIdle() succeded\n");
    }
    
    vkFreeCommandBuffers(vkDevice, vkCommandPool, 1, &vkCommandBuffer_transition_image_layout);
    vkCommandBuffer_transition_image_layout = VK_NULL_HANDLE;
    
    // Step #5: copy staging -> image
    VkCommandBufferAllocateInfo vkCommandBufferAllocateInfo_buffer_to_image_copy; 
    memset((void*)&vkCommandBufferAllocateInfo_buffer_to_image_copy, 0, sizeof(VkCommandBufferAllocateInfo));
    vkCommandBufferAllocateInfo_buffer_to_image_copy.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    vkCommandBufferAllocateInfo_buffer_to_image_copy.pNext = NULL;
    vkCommandBufferAllocateInfo_buffer_to_image_copy.commandPool = vkCommandPool;
    vkCommandBufferAllocateInfo_buffer_to_image_copy.commandBufferCount = 1;
    vkCommandBufferAllocateInfo_buffer_to_image_copy.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
   
    VkCommandBuffer vkCommandBuffer_buffer_to_image_copy = VK_NULL_HANDLE;
    vkResult = vkAllocateCommandBuffers(vkDevice, &vkCommandBufferAllocateInfo_buffer_to_image_copy, &vkCommandBuffer_buffer_to_image_copy);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "createTexture() --> vkAllocateCommandBuffers() is failed for buffer_to_image_copy and error code is %d \n", vkResult);
        fflush(gpFile);
        return vkResult;
    }
    else
    {
        fprintf(gpFile, "createTexture() --> vkAllocateCommandBuffers() is succedded for buffer_to_image_copy\n");
        fflush(gpFile);
    }

    VkCommandBufferBeginInfo vkCommandBufferBeginInfo_buffer_to_image_copy;
    memset((void*)&vkCommandBufferBeginInfo_buffer_to_image_copy, 0, sizeof(VkCommandBufferBeginInfo));
    vkCommandBufferBeginInfo_buffer_to_image_copy.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    vkCommandBufferBeginInfo_buffer_to_image_copy.pNext = NULL;
    vkCommandBufferBeginInfo_buffer_to_image_copy.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    
    vkResult = vkBeginCommandBuffer(vkCommandBuffer_buffer_to_image_copy, &vkCommandBufferBeginInfo_buffer_to_image_copy);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "createTexture() --> vkBeginCommandBuffer() is failed for buffer_to_image_copy and error code is %d\n", vkResult);
        fflush(gpFile);
        return vkResult;
    }
    else
    {
        fprintf(gpFile, "createTexture() --> vkBeginCommandBuffer() is succedded for buffer_to_image_copy \n");
        fflush(gpFile);
    }
    
    VkBufferImageCopy vkBufferImageCopy;
    memset((void*)&vkBufferImageCopy, 0, sizeof(VkBufferImageCopy));
    vkBufferImageCopy.bufferOffset = 0; 
    vkBufferImageCopy.bufferRowLength = 0;
    vkBufferImageCopy.bufferImageHeight = 0;
    vkBufferImageCopy.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    vkBufferImageCopy.imageSubresource.mipLevel = 0;
    vkBufferImageCopy.imageSubresource.baseArrayLayer = 0;
    vkBufferImageCopy.imageSubresource.layerCount = 1;
    vkBufferImageCopy.imageOffset.x = 0;
    vkBufferImageCopy.imageOffset.y = 0;
    vkBufferImageCopy.imageOffset.z = 0;
    vkBufferImageCopy.imageExtent.width = texture_width;
    vkBufferImageCopy.imageExtent.height = texture_height;
    vkBufferImageCopy.imageExtent.depth = 1;
    
    vkCmdCopyBufferToImage(vkCommandBuffer_buffer_to_image_copy, 
                           vkBuffer_stagingBuffer, 
                           vkImage_texture, 
                           VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 
                           1, 
                           &vkBufferImageCopy);
    
    vkResult = vkEndCommandBuffer(vkCommandBuffer_buffer_to_image_copy);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "createTexture() --> vkEndCommandBuffer() is failed for buffer_to_image_copy and error code is %d\n", vkResult);
        return vkResult;
    }
    else
    {
        fprintf(gpFile, "createTexture() --> vkEndCommandBuffer() is succedded for buffer_to_image_copy\n");
    }
    
    VkSubmitInfo  vkSubmitInfo_buffer_to_image_copy;
    memset((void*)&vkSubmitInfo_buffer_to_image_copy, 0, sizeof(VkSubmitInfo));
    vkSubmitInfo_buffer_to_image_copy.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    vkSubmitInfo_buffer_to_image_copy.pNext = NULL;
    vkSubmitInfo_buffer_to_image_copy.commandBufferCount = 1;
    vkSubmitInfo_buffer_to_image_copy.pCommandBuffers = &vkCommandBuffer_buffer_to_image_copy;
    
    vkResult = vkQueueSubmit(vkQueue, 1, &vkSubmitInfo_buffer_to_image_copy, VK_NULL_HANDLE);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "createTexture() --> vkQueueSubmit() is failed for buffer_to_image_copy errorcode = %d\n", vkResult);
        return vkResult;
    }
    else
    {
        fprintf(gpFile, "createTexture() --> vkQueueSubmit() is succedded for buffer_to_image_copy");
    }

    vkResult = vkQueueWaitIdle(vkQueue);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "createTexture() --> vkQueueWaitIdle() is failed for buffer_to_image_copy errorcode = %d\n", vkResult);
        return vkResult;
    }
    else
    {
        fprintf(gpFile, "createTexture() --> vkQueueWaitIdle() succeded for buffer_to_image_copy\n");
    }
    
    vkFreeCommandBuffers(vkDevice, vkCommandPool, 1, &vkCommandBuffer_buffer_to_image_copy);
    vkCommandBuffer_buffer_to_image_copy = VK_NULL_HANDLE;
    
    // Step 6: TRANSFER_DST_OPTIMAL -> SHADER_READ_ONLY_OPTIMAL
    memset((void*)&vkCommandBufferAllocateInfo_transition_image_layout, 0, sizeof(VkCommandBufferAllocateInfo));
    vkCommandBufferAllocateInfo_transition_image_layout.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    vkCommandBufferAllocateInfo_transition_image_layout.pNext = NULL;
    vkCommandBufferAllocateInfo_transition_image_layout.commandPool = vkCommandPool;
    vkCommandBufferAllocateInfo_transition_image_layout.commandBufferCount = 1;
    vkCommandBufferAllocateInfo_transition_image_layout.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
   
    vkCommandBuffer_transition_image_layout = VK_NULL_HANDLE;
    vkResult = vkAllocateCommandBuffers(vkDevice, &vkCommandBufferAllocateInfo_transition_image_layout, &vkCommandBuffer_transition_image_layout);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "createTexture() --> vkAllocateCommandBuffers() is failed and error code is %d\n", vkResult);
        fflush(gpFile);
        return vkResult;
    }
    else
    {
        fprintf(gpFile, "createTexture() --> vkAllocateCommandBuffers() is succedded\n");
        fflush(gpFile);
    }

    memset((void*)&vkCommandBufferBeginInfo_transition_image_layout, 0, sizeof(VkCommandBufferBeginInfo));
    vkCommandBufferBeginInfo_transition_image_layout.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    vkCommandBufferBeginInfo_transition_image_layout.pNext = NULL;
    vkCommandBufferBeginInfo_transition_image_layout.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    
    vkResult = vkBeginCommandBuffer(vkCommandBuffer_transition_image_layout, &vkCommandBufferBeginInfo_transition_image_layout);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "createTexture() --> vkBeginCommandBuffer() is failed and error code is %d\n", vkResult);
        fflush(gpFile);
        return vkResult;
    }
    else
    {
        fprintf(gpFile, "createTexture() --> vkBeginCommandBuffer() is succedded \n");
        fflush(gpFile);
    }

    vkPipelineStageFlags_source = 0;
    vkPipelineStageFlags_destination = 0;

    memset((void*)&vkImageMemoryBarrier, 0, sizeof(VkImageMemoryBarrier));
    vkImageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    vkImageMemoryBarrier.pNext = NULL;
    vkImageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    vkImageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    vkImageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    vkImageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    vkImageMemoryBarrier.image = vkImage_texture;
    vkImageMemoryBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT; // color
    vkImageMemoryBarrier.subresourceRange.baseArrayLayer = 0;
    vkImageMemoryBarrier.subresourceRange.baseMipLevel = 0;
    vkImageMemoryBarrier.subresourceRange.layerCount = 1;
    vkImageMemoryBarrier.subresourceRange.levelCount = 1;
    
    vkImageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    vkImageMemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
    vkPipelineStageFlags_source = VK_PIPELINE_STAGE_TRANSFER_BIT;
    vkPipelineStageFlags_destination = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    
    vkCmdPipelineBarrier(vkCommandBuffer_transition_image_layout,
                         vkPipelineStageFlags_source,
                         vkPipelineStageFlags_destination,
                         0, 0, NULL, 0, NULL, 1, &vkImageMemoryBarrier);
    
    vkResult = vkEndCommandBuffer(vkCommandBuffer_transition_image_layout);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "createTexture() --> vkEndCommandBuffer() is failed for 2nd transition and error code is %d\n", vkResult);
        return vkResult;
    }
    else
    {
        fprintf(gpFile, "createTexture() --> vkEndCommandBuffer() is succedded for 2nd transition\n");
    }
    
    memset((void*)&vkSubmitInfo_transition_image_layout, 0, sizeof(VkSubmitInfo));
    vkSubmitInfo_transition_image_layout.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    vkSubmitInfo_transition_image_layout.pNext = NULL;
    vkSubmitInfo_transition_image_layout.commandBufferCount = 1;
    vkSubmitInfo_transition_image_layout.pCommandBuffers = &vkCommandBuffer_transition_image_layout;
    
    vkResult = vkQueueSubmit(vkQueue, 1, &vkSubmitInfo_transition_image_layout, VK_NULL_HANDLE);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "createTexture() --> vkQueueSubmit() is failed  for 2nd transition errorcode = %d\n", vkResult);
        return vkResult;
    }
    else
    {
        fprintf(gpFile, "createTexture() --> vkQueueSubmit() succeded  for 2nd transition \n");
    }

    vkResult = vkQueueWaitIdle(vkQueue);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "createTexture() --> vkQueueWaitIdle() is failed  for 2nd transition errorcode = %d\n", vkResult);
        return vkResult;
    }
    else
    {
        fprintf(gpFile, "createTexture() --> vkQueueWaitIdle() succeded for 2nd transition \n");
    }
    
    vkFreeCommandBuffers(vkDevice, vkCommandPool, 1, &vkCommandBuffer_transition_image_layout);
    vkCommandBuffer_transition_image_layout = VK_NULL_HANDLE;
    
    // Step #7: release staging buffer
    if(vkBuffer_stagingBuffer)
    {
        vkDestroyBuffer(vkDevice, vkBuffer_stagingBuffer, NULL);
        vkBuffer_stagingBuffer = VK_NULL_HANDLE;
        fprintf(gpFile, "createTexture() --> vkDestroyBuffer() is done for vkBuffer_stagingBuffer of setp 7\n");
    }
    
    if(vkDeviceMemory_stagingBuffer)
    {
       vkFreeMemory(vkDevice, vkDeviceMemory_stagingBuffer, NULL);
       vkDeviceMemory_stagingBuffer = VK_NULL_HANDLE;
       fprintf(gpFile, "createTexture() --> vkFreeMemory() is done for vkBuffer_stagingBuffer of setp 7\n");
    }
    
    // Step#8: image view for the texture (***** FIX: COLOR aspect *****)
    VkImageViewCreateInfo vkImageViewCreateInfo;
    memset((void*)&vkImageViewCreateInfo, 0, sizeof(VkImageViewCreateInfo));
    vkImageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    vkImageViewCreateInfo.pNext = NULL;
    vkImageViewCreateInfo.flags = 0;
    vkImageViewCreateInfo.format = VK_FORMAT_R8G8B8A8_UNORM;
    vkImageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT; // FIXED
    vkImageViewCreateInfo.subresourceRange.baseMipLevel = 0;
    vkImageViewCreateInfo.subresourceRange.levelCount = 1;
    vkImageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
    vkImageViewCreateInfo.subresourceRange.layerCount = 1;
    vkImageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    vkImageViewCreateInfo.image = vkImage_texture;
    
    vkResult = vkCreateImageView(vkDevice,
                                 &vkImageViewCreateInfo,
                                 NULL,
                                 &vkImageView_texture);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "createTexture() --> vkCreateImageView() is failed error code is %d\n",vkResult);
        return vkResult;
    }
    else
    {
        fprintf(gpFile, "createTexture() --> vkCreateImageView() is succedded\n");
    }
    
    // Step #9: sampler (unchanged)
    VkSamplerCreateInfo vkSamplerCreateInfo;
    memset((void*)&vkSamplerCreateInfo, 0, sizeof(VkSamplerCreateInfo));
    vkSamplerCreateInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    vkSamplerCreateInfo.pNext = NULL;
    vkSamplerCreateInfo.magFilter = VK_FILTER_LINEAR;
    vkSamplerCreateInfo.minFilter = VK_FILTER_LINEAR;
    vkSamplerCreateInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    vkSamplerCreateInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    vkSamplerCreateInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    vkSamplerCreateInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    vkSamplerCreateInfo.anisotropyEnable = VK_FALSE;
    vkSamplerCreateInfo.maxAnisotropy = 16;
    vkSamplerCreateInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    vkSamplerCreateInfo.unnormalizedCoordinates = VK_FALSE;
    vkSamplerCreateInfo.compareEnable = VK_FALSE;
    vkSamplerCreateInfo.compareOp = VK_COMPARE_OP_ALWAYS;
    
    vkResult = vkCreateSampler(vkDevice, &vkSamplerCreateInfo, NULL, &vkSampler_texture);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "createTexture() --> vkCreateSampler() is failed error code is %d\n",vkResult);
        return vkResult;
    }
    else
    {
        fprintf(gpFile, "createTexture() --> vkCreateSampler() is succedded\n");
    }
     
    return vkResult; 
}

VkResult createUniformBuffer(void)
{
    //Function declarations
    VkResult updateUniformBuffer(void);
    
    //variable
    VkResult vkResult = VK_SUCCESS;
    
    //code
    
    //#4 memset the global strucure variable
    memset((void*)&uniformData, 0, sizeof(UniformData));
    
    //#5 VkBufferCreateInfo structure filling
    VkBufferCreateInfo vkBufferCreateInfo;
    memset((void*)&vkBufferCreateInfo , 0, sizeof(VkBufferCreateInfo));
    vkBufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    vkBufferCreateInfo.pNext = NULL;
    //valid flags are used in scatterred/sparse buffer
    vkBufferCreateInfo.flags = 0;
    vkBufferCreateInfo.size = sizeof(struct MyUniformData);
    vkBufferCreateInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
    
    //#6
    vkResult = vkCreateBuffer(vkDevice, 
                              &vkBufferCreateInfo,
                              NULL,
                              &uniformData.vkBuffer);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "createUniformBuffer() --> vkCreateBuffer() is failed and error code is %d\n", vkResult);
        return vkResult;
    }
    else
    {
        fprintf(gpFile, "createUniformBuffer() --> vkCreateBuffer() is succedded\n");
    }
    
    VkMemoryRequirements vkMemoryRequirements;
    memset((void*)&vkMemoryRequirements, 0, sizeof(VkMemoryRequirements));
   
    vkGetBufferMemoryRequirements(vkDevice, uniformData.vkBuffer, &vkMemoryRequirements);
    
    //8" Allocate
    VkMemoryAllocateInfo vkMemoryAllocateInfo;
    memset((void*)&vkMemoryAllocateInfo , 0, sizeof(VkMemoryAllocateInfo));
    vkMemoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    vkMemoryAllocateInfo.pNext = NULL; 
    vkMemoryAllocateInfo.allocationSize = vkMemoryRequirements.size;
    //initial value before entering inloop
    vkMemoryAllocateInfo.memoryTypeIndex = 0;
   
    for(uint32_t i = 0; i < vkPhysicalDeviceMemoryProperties.memoryTypeCount; i++)
    {
        if((vkMemoryRequirements.memoryTypeBits & 1) == 1)
        {
            if(vkPhysicalDeviceMemoryProperties.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)
            {
                vkMemoryAllocateInfo.memoryTypeIndex = i;
                break;
            }
        }
        vkMemoryRequirements.memoryTypeBits >>= 1;
    }
    
    //#9 vkAllocateMemory
    vkResult = vkAllocateMemory(vkDevice, &vkMemoryAllocateInfo, NULL, &uniformData.vkDeviceMemory);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "createUniformBuffer() --> vkAllocateMemory() is failed and error code is %d\n", vkResult);
        return vkResult;
    }
    else
    {
        fprintf(gpFile, "createUniformBuffer() --> vkAllocateMemory() is succedded\n");
    }
    
    //#10: Binds vulkan device memory object handle with vulkan buffer object handle
    vkResult = vkBindBufferMemory(vkDevice, uniformData.vkBuffer, uniformData.vkDeviceMemory, 0);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "createUniformBuffer() --> vkBindBufferMemory() is failed and error code is %d\n", vkResult);
        return vkResult;
    }
    else
    {
        fprintf(gpFile, "createUniformBuffer() --> vkBindBufferMemory() is succedded\n");
    }
    
    //call updateUniformBuffer()
    vkResult = updateUniformBuffer();
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "createUniformBuffer() --> updateUniformBuffer() is failed and error code is %d\n", vkResult);
        return vkResult;
    }
    else
    {
        fprintf(gpFile, "createUniformBuffer() --> updateUniformBuffer() is succedded\n");
    }
    
    return vkResult;
}

VkResult updateUniformBuffer(void)
{
    VkResult vkResult = VK_SUCCESS;

    MyUniformData myUniformData;
    memset((void*)&myUniformData, 0, sizeof(MyUniformData));

    // --- Transforms ---
    glm::mat4 translationMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -4.0f));
    glm::mat4 rotationMatrix    = glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(0.0f, 1.0f, 0.0f));
    myUniformData.modelMatrix   = translationMatrix * rotationMatrix;
    myUniformData.viewMatrix    = glm::mat4(1.0f);

    glm::mat4 perspectiveProjectionMatrix = glm::perspective(glm::radians(45.0f),
                                                             (float)winWidth / (float)winHeight,
                                                             0.1f, 100.0f);
    perspectiveProjectionMatrix[1][1] *= -1.0f; // GLM fix for Vulkan
    myUniformData.projectionMatrix = perspectiveProjectionMatrix;

    // Lighting (kept, but disabled if noShadow = 1)
    glm::vec3 L = glm::normalize(glm::vec3(-0.4f, 0.8f, 0.4f));
    myUniformData.lightDir     = glm::vec4(L, 0.0f);
    myUniformData.lightColor   = glm::vec4(1.0f, 1.0f, 1.0f, 0.0f);
    myUniformData.ambientColor = glm::vec4(0.03f, 0.03f, 0.03f, 0.0f);

    // params: x = displacementScale (object units), y = noShadow (1=flat), z = saturation, w unused
    // Use tiny displacement for gas-giant cloud relief and full saturation.
    myUniformData.params = glm::vec4(0.006f, 1.0f, 1.0f, 0.0f); // scale, noShadow, saturation=1.0
    //                     ^0.6% radius; tweak 0.003–0.01
    // If you want lighting, set y to 0.0f.

    // SH irradiance defaults (constant)
    const float Y00 = 0.282095f;
    glm::vec3 C(0.20f, 0.20f, 0.20f);
    glm::vec3 dc = C / Y00;
    myUniformData.sh[0] = glm::vec4(dc, 0.0f);
    for (int i = 1; i < 9; ++i) myUniformData.sh[i] = glm::vec4(0.0f);

    void* data = NULL;
    vkResult = vkMapMemory(vkDevice, uniformData.vkDeviceMemory, 0, VK_WHOLE_SIZE, 0, &data);
    if (vkResult != VK_SUCCESS) {
        fprintf(gpFile, "updateUniformBuffer() --> vkMapMemory() failed, err=%d\n", vkResult);
        return vkResult;
    }
    memcpy(data, &myUniformData, sizeof(MyUniformData));

    VkMappedMemoryRange rng{};
    rng.sType  = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
    rng.memory = uniformData.vkDeviceMemory;
    rng.offset = 0;
    rng.size   = VK_WHOLE_SIZE;
    vkFlushMappedMemoryRanges(vkDevice, 1, &rng);

    vkUnmapMemory(vkDevice, uniformData.vkDeviceMemory);
    return vkResult;
}

VkResult createShaders(void)
{
    //variable
    VkResult vkResult = VK_SUCCESS;
    
    //code
    //for vertex shader
    const char* szFileName = "shader.vert.spv";
    FILE* fp = NULL;
    size_t size;
    
    //#6a
    fp = fopen(szFileName, "rb"); //open for reading in binary format
    if(fp == NULL)
    {
        fprintf(gpFile, "createShaders() --> fopen() failed to open shader.vert.spv\n");
        vkResult = VK_ERROR_INITIALIZATION_FAILED;
        return vkResult;
    }
    else
    {
        fprintf(gpFile, "createShaders() --> fopen() succedded to open shader.vert.spv\n");
    }
    
    //#6b
    fseek(fp, 0L, SEEK_END);
    
    //#6c
    size = ftell(fp);
    if(size == 0)
    {
        fprintf(gpFile, "createShaders() --> ftell() failed to provide size of shader.vert.spv\n");
        vkResult = VK_ERROR_INITIALIZATION_FAILED;
        return vkResult;
    }
   
    //#6d
    fseek(fp, 0L, SEEK_SET); //reset to start
    
    //#6e
    char* shaderData = (char*)malloc(sizeof(char) * size);
    size_t retVal = fread(shaderData, size, 1, fp);
    if(retVal != 1)
    {
        fprintf(gpFile, "createShaders() --> fread() failed to read shader.vert.spv\n");
        vkResult = VK_ERROR_INITIALIZATION_FAILED;
        return vkResult;
    }
    else
    {
        fprintf(gpFile, "createShaders() --> fread() succedded to read shader.vert.spv\n");
    }
    
    //#6f
    fclose(fp);
    
    //#7
    VkShaderModuleCreateInfo vkShaderModuleCreateInfo;
    memset((void*)&vkShaderModuleCreateInfo, 0, sizeof(VkShaderModuleCreateInfo));
    vkShaderModuleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    vkShaderModuleCreateInfo.pNext = NULL;
    vkShaderModuleCreateInfo.flags = 0; // reserved, hence must be zero
    vkShaderModuleCreateInfo.codeSize = size;
    vkShaderModuleCreateInfo.pCode = (uint32_t*)shaderData;

    //8
    vkResult = vkCreateShaderModule(vkDevice, &vkShaderModuleCreateInfo, NULL, &vkShaderModule_vertex_shader);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "createShaders() --> vkCreateShaderModule() is failed & error code is %d\n", vkResult);
        return vkResult;
    }
    else
    {
        fprintf(gpFile, "createShaders() --> vkCreateShaderModule() is succedded\n");
    }
    
    //#9
    if(shaderData)
    {
        free(shaderData);
        shaderData = NULL;
    }
    
    fprintf(gpFile, "createShaders() --> vertex Shader module successfully created\n");
    
    
    //for fragment shader
    szFileName = "shader.frag.spv";
    fp = NULL;
    size = 0;
    
    //#6a
    fp = fopen(szFileName, "rb"); //open for reading in binary format
    if(fp == NULL)
    {
        fprintf(gpFile, "createShaders() --> fopen() failed to open shader.frag.spv\n");
        vkResult = VK_ERROR_INITIALIZATION_FAILED;
        return vkResult;
    }
    else
    {
        fprintf(gpFile, "createShaders() --> fopen() succedded to open shader.frag.spv\n");
    }
    
    //#6b
    fseek(fp, 0L, SEEK_END);
    
    //#6c
    size = ftell(fp);
    if(size == 0)
    {
        fprintf(gpFile, "createShaders() --> ftell() failed to provide size of shader.frag.spv\n");
        vkResult = VK_ERROR_INITIALIZATION_FAILED;
        return vkResult;
    }
   
    //#6d
    fseek(fp, 0L, SEEK_SET); //reset to start
    
    //#6e
    shaderData = (char*)malloc(sizeof(char) * size);
    retVal = fread(shaderData, size, 1, fp);
    if(retVal != 1)
    {
        fprintf(gpFile, "createShaders() --> fread() failed to read shader.frag.spv\n");
        vkResult = VK_ERROR_INITIALIZATION_FAILED;
        return vkResult;
    }
    else
    {
        fprintf(gpFile, "createShaders() --> fread() succedded to read shader.frag.spv\n");
    }
    
    //#6f
    fclose(fp);
    
    //#7
    memset((void*)&vkShaderModuleCreateInfo, 0, sizeof(VkShaderModuleCreateInfo));
    vkShaderModuleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    vkShaderModuleCreateInfo.pNext = NULL;
    vkShaderModuleCreateInfo.flags = 0; // reserved, hence must be zero
    vkShaderModuleCreateInfo.codeSize = size;
    vkShaderModuleCreateInfo.pCode = (uint32_t*)shaderData;

    //8
    vkResult = vkCreateShaderModule(vkDevice, &vkShaderModuleCreateInfo, NULL, &vkShaderModule_fragment_shader);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "createShaders() --> vkCreateShaderModule() is failed & error code is %d\n", vkResult);
        return vkResult;
    }
    else
    {
        fprintf(gpFile, "createShaders() --> vkCreateShaderModule() is succedded\n");
    }
    
    //#9
    if(shaderData)
    {
        free(shaderData);
        shaderData = NULL;
    }
    
    fprintf(gpFile, "createShaders() --> fragment Shader module successfully created\n");
     
    return (vkResult);
}

VkResult createDescriptorSetLayout(void)
{
    //variable
    VkResult vkResult = VK_SUCCESS;
    
    //code
    //Descriptor set binding 
    //0th index --> uniform (VS + FS)
    //1st index --> albedo texture image (FS)
    //2nd index --> normal map (FS)
    //3rd index --> height / displacement map (VS)
    VkDescriptorSetLayoutBinding vkDescriptorSetLayoutBinding_array[4];
    memset((void*)vkDescriptorSetLayoutBinding_array, 0,
           sizeof(VkDescriptorSetLayoutBinding) * _ARRAYSIZE(vkDescriptorSetLayoutBinding_array));
    
    // for MVP + lighting + params Uniform
    vkDescriptorSetLayoutBinding_array[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    vkDescriptorSetLayoutBinding_array[0].binding = 0;  // this zero related with zero binding in vertex/fragment shaders
    vkDescriptorSetLayoutBinding_array[0].descriptorCount = 1;
    vkDescriptorSetLayoutBinding_array[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;  // used in both stages
    vkDescriptorSetLayoutBinding_array[0].pImmutableSamplers = NULL;
    
    // for albedo texture image and sampler
    vkDescriptorSetLayoutBinding_array[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    vkDescriptorSetLayoutBinding_array[1].binding = 1;  // this one related with 1 binding in fragment shader
    vkDescriptorSetLayoutBinding_array[1].descriptorCount = 1;
    vkDescriptorSetLayoutBinding_array[1].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;  //shader stage
    vkDescriptorSetLayoutBinding_array[1].pImmutableSamplers = NULL;
    
    // for normal map image and sampler
    vkDescriptorSetLayoutBinding_array[2].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    vkDescriptorSetLayoutBinding_array[2].binding = 2;  // binding 2 in fragment shader
    vkDescriptorSetLayoutBinding_array[2].descriptorCount = 1;
    vkDescriptorSetLayoutBinding_array[2].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;  //shader stage
    vkDescriptorSetLayoutBinding_array[2].pImmutableSamplers = NULL;

    // for height/displacement map image and sampler (sampled in VS)
    vkDescriptorSetLayoutBinding_array[3].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    vkDescriptorSetLayoutBinding_array[3].binding = 3;  // binding 3 in vertex shader
    vkDescriptorSetLayoutBinding_array[3].descriptorCount = 1;
    vkDescriptorSetLayoutBinding_array[3].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    vkDescriptorSetLayoutBinding_array[3].pImmutableSamplers = NULL;
    
    
    VkDescriptorSetLayoutCreateInfo vkDescriptorSetLayoutCreateInfo;
    memset((void*)&vkDescriptorSetLayoutCreateInfo, 0, sizeof(VkDescriptorSetLayoutCreateInfo));
    vkDescriptorSetLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    vkDescriptorSetLayoutCreateInfo.pNext = NULL;
    vkDescriptorSetLayoutCreateInfo.flags = 0; //reserved
    vkDescriptorSetLayoutCreateInfo.bindingCount = _ARRAYSIZE(vkDescriptorSetLayoutBinding_array); // one DescriptorSet available
    vkDescriptorSetLayoutCreateInfo.pBindings = vkDescriptorSetLayoutBinding_array;
    
    vkResult = vkCreateDescriptorSetLayout(vkDevice, &vkDescriptorSetLayoutCreateInfo, NULL, &vkDescriptorSetLayout);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "createDescriptorSetLayout() --> vkCreateDescriptorSetLayout() is failed & error code is %d\n", vkResult);
        return vkResult;
    }
    else
    {
        fprintf(gpFile, "createDescriptorSetLayout() --> vkCreateDescriptorSetLayout() is succedded\n");
    }
    
    return (vkResult);
}

VkResult createPipelineLayout(void)
{
    //variable
    VkResult vkResult = VK_SUCCESS;
    
    //code
    VkPipelineLayoutCreateInfo vkPipelineLayoutCreateInfo;
    memset((void*)&vkPipelineLayoutCreateInfo, 0, sizeof(VkPipelineLayoutCreateInfo));
    vkPipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    vkPipelineLayoutCreateInfo.pNext = NULL;
    vkPipelineLayoutCreateInfo.flags = 0; //reserved
    vkPipelineLayoutCreateInfo.setLayoutCount = 1;
    vkPipelineLayoutCreateInfo.pSetLayouts = &vkDescriptorSetLayout;
    vkPipelineLayoutCreateInfo.pushConstantRangeCount = 0;
    vkPipelineLayoutCreateInfo.pPushConstantRanges = NULL;
    
    vkResult = vkCreatePipelineLayout(vkDevice, &vkPipelineLayoutCreateInfo, NULL, &vkPipelineLayout);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "createPipelineLayout() --> vkCreatePipelineLayout() is failed & error code is %d\n", vkResult);
        return vkResult;
    }
    else
    {
        fprintf(gpFile, "createPipelineLayout() --> vkCreatePipelineLayout() is succedded\n");
    }
    
    return (vkResult);
}

VkResult createDescriptorPool(void)
{
    //variable
    VkResult vkResult = VK_SUCCESS;
    
    //code
    //before creating actual descriptor pool, vulkan expects descriptor pool size
    //0th index --> uniform
    //1st index --> texture images (albedo + normal + height)
    VkDescriptorPoolSize vkDescriptorPoolSize_array[2];
    memset((void*)vkDescriptorPoolSize_array, 0, sizeof(VkDescriptorPoolSize) * _ARRAYSIZE(vkDescriptorPoolSize_array));
    
    //for MVP + lighting + params uniform (binding 0)
    vkDescriptorPoolSize_array[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    vkDescriptorPoolSize_array[0].descriptorCount = 1;
    
    //for image samplers: albedo (binding 1), normal (binding 2), height (binding 3)
    //Total descriptors in the pool must cover all images used by the set(s).
    vkDescriptorPoolSize_array[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    vkDescriptorPoolSize_array[1].descriptorCount = 3;
    
    
    //Create the pool
    VkDescriptorPoolCreateInfo vkDescriptorPoolCreateInfo;
    memset((void*)&vkDescriptorPoolCreateInfo, 0, sizeof(VkDescriptorPoolCreateInfo));
    vkDescriptorPoolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    vkDescriptorPoolCreateInfo.pNext = NULL;
    vkDescriptorPoolCreateInfo.flags = 0;
    vkDescriptorPoolCreateInfo.poolSizeCount = _ARRAYSIZE(vkDescriptorPoolSize_array);
    vkDescriptorPoolCreateInfo.pPoolSizes = vkDescriptorPoolSize_array;
    // We allocate exactly one descriptor set in this app.
    vkDescriptorPoolCreateInfo.maxSets = 1;
    
    vkResult = vkCreateDescriptorPool(vkDevice, &vkDescriptorPoolCreateInfo, NULL, &vkDescriptorPool);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "createDescriptorPool() --> vkCreateDescriptorPool() is failed & error code is %d\n", vkResult);
        return vkResult;
    }
    else
    {
        fprintf(gpFile, "createDescriptorPool() --> vkCreateDescriptorPool() is succedded\n");
    }
    
    return (vkResult);
}

VkResult createDescriptorSet(void)
{
    //variable
    VkResult vkResult = VK_SUCCESS;
    
    //code
    //Initialize descriptorset allocation info
    VkDescriptorSetAllocateInfo vkDescriptorSetAllocateInfo;
    memset((void*)&vkDescriptorSetAllocateInfo, 0, sizeof(VkDescriptorSetAllocateInfo));
    vkDescriptorSetAllocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    vkDescriptorSetAllocateInfo.pNext = NULL;
    vkDescriptorSetAllocateInfo.descriptorPool = vkDescriptorPool;
    vkDescriptorSetAllocateInfo.descriptorSetCount = 1;  // one descriptor set
    vkDescriptorSetAllocateInfo.pSetLayouts = &vkDescriptorSetLayout;
    
    vkResult = vkAllocateDescriptorSets(vkDevice, &vkDescriptorSetAllocateInfo, &vkDescriptorSet);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "createDescriptorSet() --> vkAllocateDescriptorSets() is failed & error code is %d\n", vkResult);
        return vkResult;
    }
    else
    {
        fprintf(gpFile, "createDescriptorSet() --> vkAllocateDescriptorSets() is succedded\n");
    }
    
    //Describe whether we want buffer as uniform or image as uniform

    //For Buffer Uniform (binding 0)
    VkDescriptorBufferInfo vkDescriptorBufferInfo;
    memset((void*)&vkDescriptorBufferInfo, 0, sizeof(VkDescriptorBufferInfo));
    vkDescriptorBufferInfo.buffer = uniformData.vkBuffer;
    vkDescriptorBufferInfo.offset = 0;
    vkDescriptorBufferInfo.range  = sizeof(MyUniformData);
    
    //for Albedo texture image & sampler (binding 1)
    VkDescriptorImageInfo vkDescriptorImageInfo_albedo;
    memset((void*)&vkDescriptorImageInfo_albedo, 0, sizeof(VkDescriptorImageInfo));
    vkDescriptorImageInfo_albedo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    vkDescriptorImageInfo_albedo.imageView   = vkImageView_albedo;
    vkDescriptorImageInfo_albedo.sampler     = vkSampler_albedo;

    //for Normal map image & sampler (binding 2)
    VkDescriptorImageInfo vkDescriptorImageInfo_normal;
    memset((void*)&vkDescriptorImageInfo_normal, 0, sizeof(VkDescriptorImageInfo));
    vkDescriptorImageInfo_normal.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    vkDescriptorImageInfo_normal.imageView   = vkImageView_normal;
    vkDescriptorImageInfo_normal.sampler     = vkSampler_normal;

    //for Height/Displacement map image & sampler (binding 3)
    VkDescriptorImageInfo vkDescriptorImageInfo_height;
    memset((void*)&vkDescriptorImageInfo_height, 0, sizeof(VkDescriptorImageInfo));
    vkDescriptorImageInfo_height.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    vkDescriptorImageInfo_height.imageView   = vkImageView_height;
    vkDescriptorImageInfo_height.sampler     = vkSampler_height;
    
    //now update the above descriptor set directly to shader
    VkWriteDescriptorSet vkWriteDescriptorSet_array[4];
    memset((void*)vkWriteDescriptorSet_array, 0, sizeof(VkWriteDescriptorSet) * _ARRAYSIZE(vkWriteDescriptorSet_array));

    // binding 0: UBO (VS + FS)
    vkWriteDescriptorSet_array[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    vkWriteDescriptorSet_array[0].dstSet = vkDescriptorSet;
    vkWriteDescriptorSet_array[0].dstBinding = 0;
    vkWriteDescriptorSet_array[0].dstArrayElement = 0;
    vkWriteDescriptorSet_array[0].descriptorCount = 1;
    vkWriteDescriptorSet_array[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    vkWriteDescriptorSet_array[0].pBufferInfo = &vkDescriptorBufferInfo;
    vkWriteDescriptorSet_array[0].pImageInfo = NULL;
    vkWriteDescriptorSet_array[0].pTexelBufferView = NULL;
    
    // binding 1: Albedo (FS)
    vkWriteDescriptorSet_array[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    vkWriteDescriptorSet_array[1].dstSet = vkDescriptorSet;
    vkWriteDescriptorSet_array[1].dstBinding = 1;
    vkWriteDescriptorSet_array[1].dstArrayElement = 0;
    vkWriteDescriptorSet_array[1].descriptorCount = 1;
    vkWriteDescriptorSet_array[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    vkWriteDescriptorSet_array[1].pBufferInfo = NULL;
    vkWriteDescriptorSet_array[1].pImageInfo = &vkDescriptorImageInfo_albedo;
    vkWriteDescriptorSet_array[1].pTexelBufferView = NULL;

    // binding 2: Normal (FS)
    vkWriteDescriptorSet_array[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    vkWriteDescriptorSet_array[2].dstSet = vkDescriptorSet;
    vkWriteDescriptorSet_array[2].dstBinding = 2;
    vkWriteDescriptorSet_array[2].dstArrayElement = 0;
    vkWriteDescriptorSet_array[2].descriptorCount = 1;
    vkWriteDescriptorSet_array[2].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    vkWriteDescriptorSet_array[2].pBufferInfo = NULL;
    vkWriteDescriptorSet_array[2].pImageInfo = &vkDescriptorImageInfo_normal;
    vkWriteDescriptorSet_array[2].pTexelBufferView = NULL;

    // binding 3: Height/Displacement (VS)
    vkWriteDescriptorSet_array[3].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    vkWriteDescriptorSet_array[3].dstSet = vkDescriptorSet;
    vkWriteDescriptorSet_array[3].dstBinding = 3;
    vkWriteDescriptorSet_array[3].dstArrayElement = 0;
    vkWriteDescriptorSet_array[3].descriptorCount = 1;
    vkWriteDescriptorSet_array[3].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    vkWriteDescriptorSet_array[3].pBufferInfo = NULL;
    vkWriteDescriptorSet_array[3].pImageInfo = &vkDescriptorImageInfo_height;
    vkWriteDescriptorSet_array[3].pTexelBufferView = NULL;
    
    vkUpdateDescriptorSets(vkDevice, _ARRAYSIZE(vkWriteDescriptorSet_array), vkWriteDescriptorSet_array, 0, NULL);    
    fprintf(gpFile, "createDescriptorSet() --> vkUpdateDescriptorSets() is succedded\n");
    
    return (vkResult);
}

VkResult createRenderPass(void)
{
    //variable
    VkResult vkResult = VK_SUCCESS;
    
    //code
    //step1:
    VkAttachmentDescription vkAttachmentDescription_array[2];  //for both color and depth
    memset((void*)vkAttachmentDescription_array, 0, sizeof(VkAttachmentDescription) * _ARRAYSIZE(vkAttachmentDescription_array));
    
    //for color
    vkAttachmentDescription_array[0].flags = 0; //For embedded devices
    vkAttachmentDescription_array[0].format = vkFormat_color;
    vkAttachmentDescription_array[0].samples = VK_SAMPLE_COUNT_1_BIT;
    vkAttachmentDescription_array[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    vkAttachmentDescription_array[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    vkAttachmentDescription_array[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    vkAttachmentDescription_array[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    vkAttachmentDescription_array[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    vkAttachmentDescription_array[0].finalLayout   = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;  // present after render
    
    //for depth
    vkAttachmentDescription_array[1].flags = 0; //For embedded devices
    vkAttachmentDescription_array[1].format = vkFormat_depth;
    vkAttachmentDescription_array[1].samples = VK_SAMPLE_COUNT_1_BIT;
    vkAttachmentDescription_array[1].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    vkAttachmentDescription_array[1].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    vkAttachmentDescription_array[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    vkAttachmentDescription_array[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    vkAttachmentDescription_array[1].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    vkAttachmentDescription_array[1].finalLayout   = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    

    //Step2: attachment refs
    VkAttachmentReference vkAttachmentReference_color;
    memset((void*)&vkAttachmentReference_color, 0, sizeof(VkAttachmentReference));
    vkAttachmentReference_color.attachment = 0;  //index number
    vkAttachmentReference_color.layout     = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    
    VkAttachmentReference vkAttachmentReference_depth;
    memset((void*)&vkAttachmentReference_depth, 0, sizeof(VkAttachmentReference));
    vkAttachmentReference_depth.attachment = 1;  //index number
    vkAttachmentReference_depth.layout     = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    
    //step3: subpass
    VkSubpassDescription vkSubpassDescription;
    memset((void*)&vkSubpassDescription, 0, sizeof(VkSubpassDescription));
    vkSubpassDescription.flags = 0;
    vkSubpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    vkSubpassDescription.inputAttachmentCount = 0;
    vkSubpassDescription.pInputAttachments = NULL;
    vkSubpassDescription.colorAttachmentCount = 1;  // only color attachments counted here
    vkSubpassDescription.pColorAttachments = &vkAttachmentReference_color;
    vkSubpassDescription.pResolveAttachments = NULL;
    vkSubpassDescription.pDepthStencilAttachment = &vkAttachmentReference_depth;
    vkSubpassDescription.preserveAttachmentCount = 0;
    vkSubpassDescription.pPreserveAttachments = NULL;
    
    // ***** NEW: explicit subpass dependencies to sync external <-> subpass *****
    VkSubpassDependency dependencies[2];
    memset((void*)dependencies, 0, sizeof(dependencies));

    // External -> Subpass (ensure we can write color/depth)
    dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
    dependencies[0].dstSubpass = 0;
    dependencies[0].srcStageMask  = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependencies[0].dstStageMask  = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependencies[0].srcAccessMask = 0;
    dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
    dependencies[0].dependencyFlags = 0;

    // Subpass -> External (ensure image is ready for PRESENT)
    dependencies[1].srcSubpass = 0;
    dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
    dependencies[1].srcStageMask  = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependencies[1].dstStageMask  = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
    dependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    dependencies[1].dstAccessMask = 0;
    dependencies[1].dependencyFlags = 0;

    //step4: render pass create info
    VkRenderPassCreateInfo vkRenderPassCreateInfo;
    memset((void*)&vkRenderPassCreateInfo, 0, sizeof(VkRenderPassCreateInfo));
    vkRenderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    vkRenderPassCreateInfo.pNext = NULL;
    vkRenderPassCreateInfo.flags = 0;
    vkRenderPassCreateInfo.attachmentCount = _ARRAYSIZE(vkAttachmentDescription_array);
    vkRenderPassCreateInfo.pAttachments    = vkAttachmentDescription_array;
    vkRenderPassCreateInfo.subpassCount    = 1;
    vkRenderPassCreateInfo.pSubpasses      = &vkSubpassDescription;

    // ***** Hook in the dependencies *****
    vkRenderPassCreateInfo.dependencyCount = _ARRAYSIZE(dependencies);
    vkRenderPassCreateInfo.pDependencies   = dependencies;
    
    //step5: create RP
    vkResult = vkCreateRenderPass(vkDevice, 
                                  &vkRenderPassCreateInfo,
                                  NULL,
                                  &vkRenderPass);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "createRenderPass() --> vkCreateRenderPass() is failed & error code is %d\n", vkResult);
        return vkResult;
    }
    else
    {
        fprintf(gpFile, "createRenderPass() --> vkCreateRenderPass() is succedded\n");
    }
    
    return (vkResult);
}

VkResult createPipeline(void)
{
    //variables
    VkResult vkResult = VK_SUCCESS;
    
    //code
    //#1: vertex input state
    VkVertexInputBindingDescription vkVertexInputBindingDescription_array[2]; 
    memset((void*)vkVertexInputBindingDescription_array, 0, sizeof(VkVertexInputBindingDescription) * _ARRAYSIZE(vkVertexInputBindingDescription_array));
    
    //for position
    vkVertexInputBindingDescription_array[0].binding = 0;//corresponding to location 0 in vertex shader
    vkVertexInputBindingDescription_array[0].stride = sizeof(float) * 3;
    vkVertexInputBindingDescription_array[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    //for texcoord
    vkVertexInputBindingDescription_array[1].binding = 1; //corresponding to location 1 in vertex shader
    vkVertexInputBindingDescription_array[1].stride = sizeof(float) * 2;
    vkVertexInputBindingDescription_array[1].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
  
    VkVertexInputAttributeDescription vkVertexInputAttributeDescription_array[2]; 
    memset((void*)vkVertexInputAttributeDescription_array, 0, sizeof(VkVertexInputAttributeDescription) * _ARRAYSIZE(vkVertexInputAttributeDescription_array));
    
    //for position
    vkVertexInputAttributeDescription_array[0].binding = 0;
    vkVertexInputAttributeDescription_array[0].location = 0;
    vkVertexInputAttributeDescription_array[0].format = VK_FORMAT_R32G32B32_SFLOAT;
    vkVertexInputAttributeDescription_array[0].offset = 0;
    
    //for texcoord
    vkVertexInputAttributeDescription_array[1].binding = 1;
    vkVertexInputAttributeDescription_array[1].location = 1;
    vkVertexInputAttributeDescription_array[1].format = VK_FORMAT_R32G32_SFLOAT;
    vkVertexInputAttributeDescription_array[1].offset = 0;
    
    VkPipelineVertexInputStateCreateInfo vkPipelineVertexInputStateCreateInfo;
    memset((void*)&vkPipelineVertexInputStateCreateInfo, 0, sizeof(VkPipelineVertexInputStateCreateInfo));
    vkPipelineVertexInputStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vkPipelineVertexInputStateCreateInfo.pNext = NULL;
    vkPipelineVertexInputStateCreateInfo.flags = 0;
    vkPipelineVertexInputStateCreateInfo.vertexBindingDescriptionCount = _ARRAYSIZE(vkVertexInputBindingDescription_array);
    vkPipelineVertexInputStateCreateInfo.pVertexBindingDescriptions = vkVertexInputBindingDescription_array;
    vkPipelineVertexInputStateCreateInfo.vertexAttributeDescriptionCount = _ARRAYSIZE(vkVertexInputAttributeDescription_array);
    vkPipelineVertexInputStateCreateInfo.pVertexAttributeDescriptions = vkVertexInputAttributeDescription_array;
    
    //#2: Input assembly State
    VkPipelineInputAssemblyStateCreateInfo vkPipelineInputAssemblyStateCreateInfo;
    memset((void*)&vkPipelineInputAssemblyStateCreateInfo, 0, sizeof(VkPipelineInputAssemblyStateCreateInfo));
    vkPipelineInputAssemblyStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    vkPipelineInputAssemblyStateCreateInfo.pNext = NULL;
    vkPipelineInputAssemblyStateCreateInfo.flags = 0;
    vkPipelineInputAssemblyStateCreateInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    vkPipelineInputAssemblyStateCreateInfo.primitiveRestartEnable = 0;
    
    //#3: Rasterizer state
    VkPipelineRasterizationStateCreateInfo vkPipelineRasterizationStateCreateInfo;
    memset((void*)&vkPipelineRasterizationStateCreateInfo, 0, sizeof(VkPipelineRasterizationStateCreateInfo));
    vkPipelineRasterizationStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    vkPipelineRasterizationStateCreateInfo.pNext = NULL;
    vkPipelineRasterizationStateCreateInfo.flags = 0;
    vkPipelineRasterizationStateCreateInfo.polygonMode = VK_POLYGON_MODE_FILL;
    vkPipelineRasterizationStateCreateInfo.cullMode = VK_CULL_MODE_NONE;
    vkPipelineRasterizationStateCreateInfo.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    vkPipelineRasterizationStateCreateInfo.lineWidth = 1.0f;
    
    //#4: Color Blend State
    VkPipelineColorBlendAttachmentState vkPipelineColorBlendAttachmentState_array[1];
    memset((void*)vkPipelineColorBlendAttachmentState_array, 0, sizeof(VkPipelineColorBlendAttachmentState) * _ARRAYSIZE(vkPipelineColorBlendAttachmentState_array));
    vkPipelineColorBlendAttachmentState_array[0].blendEnable = VK_FALSE;
    vkPipelineColorBlendAttachmentState_array[0].colorWriteMask = 0xF;
    
    VkPipelineColorBlendStateCreateInfo vkPipelineColorBlendStateCreateInfo;
    memset((void*)&vkPipelineColorBlendStateCreateInfo, 0, sizeof(VkPipelineColorBlendStateCreateInfo));
    vkPipelineColorBlendStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    vkPipelineColorBlendStateCreateInfo.pNext = NULL;
    vkPipelineColorBlendStateCreateInfo.flags = 0;
    vkPipelineColorBlendStateCreateInfo.attachmentCount = _ARRAYSIZE(vkPipelineColorBlendAttachmentState_array);
    vkPipelineColorBlendStateCreateInfo.pAttachments = vkPipelineColorBlendAttachmentState_array;
    
    //#5: viewport scissor state
    VkPipelineViewportStateCreateInfo vkPipelineViewportStateCreateInfo;
    memset((void*)&vkPipelineViewportStateCreateInfo, 0, sizeof(VkPipelineViewportStateCreateInfo));
    vkPipelineViewportStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    vkPipelineViewportStateCreateInfo.pNext = NULL;
    vkPipelineViewportStateCreateInfo.flags = 0;
    vkPipelineViewportStateCreateInfo.viewportCount = 1;
    
    memset((void*)&vkViewport, 0, sizeof(VkViewport));
    vkViewport.x = 0;
    vkViewport.y = 0;
    vkViewport.width  = (float)vkExtent2D_swapchain.width;
    vkViewport.height = (float)vkExtent2D_swapchain.height;
    vkViewport.minDepth = 0.0f;
    vkViewport.maxDepth = 1.0f;
    
    vkPipelineViewportStateCreateInfo.pViewports = &vkViewport;
    vkPipelineViewportStateCreateInfo.scissorCount = 1;
    
    memset((void*)&vkRect2D_scissor, 0, sizeof(VkRect2D));
    vkRect2D_scissor.offset.x = 0;
    vkRect2D_scissor.offset.y = 0;

    // ***** FIX: extent is uint32_t *****
    vkRect2D_scissor.extent.width  = vkExtent2D_swapchain.width;
    vkRect2D_scissor.extent.height = vkExtent2D_swapchain.height;
    
    vkPipelineViewportStateCreateInfo.pScissors = &vkRect2D_scissor;
    
    //#6: Depth Stencil state
    VkPipelineDepthStencilStateCreateInfo vkPipelineDepthStencilStateCreateInfo;
    memset((void*)&vkPipelineDepthStencilStateCreateInfo, 0, sizeof(VkPipelineDepthStencilStateCreateInfo));
    vkPipelineDepthStencilStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    vkPipelineDepthStencilStateCreateInfo.depthTestEnable = VK_TRUE;
    vkPipelineDepthStencilStateCreateInfo.depthWriteEnable = VK_TRUE;
    vkPipelineDepthStencilStateCreateInfo.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
    vkPipelineDepthStencilStateCreateInfo.depthBoundsTestEnable = VK_FALSE;
    vkPipelineDepthStencilStateCreateInfo.back.failOp = VK_STENCIL_OP_KEEP;
    vkPipelineDepthStencilStateCreateInfo.back.passOp = VK_STENCIL_OP_KEEP;
    vkPipelineDepthStencilStateCreateInfo.back.compareOp = VK_COMPARE_OP_ALWAYS;
    vkPipelineDepthStencilStateCreateInfo.front = vkPipelineDepthStencilStateCreateInfo.back;
    vkPipelineDepthStencilStateCreateInfo.stencilTestEnable = VK_FALSE;    
    
    //#8: Multi Sample State
    VkPipelineMultisampleStateCreateInfo vkPipelineMultisampleStateCreateInfo;
    memset((void*)&vkPipelineMultisampleStateCreateInfo, 0, sizeof(VkPipelineMultisampleStateCreateInfo));
    vkPipelineMultisampleStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    vkPipelineMultisampleStateCreateInfo.pNext = NULL;
    vkPipelineMultisampleStateCreateInfo.flags = 0;
    vkPipelineMultisampleStateCreateInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    
    //#9: Shader stages
    VkPipelineShaderStageCreateInfo vkPipelineShaderStageCreateInfo_array[2];
    memset((void*)vkPipelineShaderStageCreateInfo_array, 0, sizeof(VkPipelineShaderStageCreateInfo) * _ARRAYSIZE(vkPipelineShaderStageCreateInfo_array));
    //Vertex Shader
    vkPipelineShaderStageCreateInfo_array[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vkPipelineShaderStageCreateInfo_array[0].pNext = NULL;
    vkPipelineShaderStageCreateInfo_array[0].flags = 0;
    vkPipelineShaderStageCreateInfo_array[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
    vkPipelineShaderStageCreateInfo_array[0].module = vkShaderModule_vertex_shader;
    vkPipelineShaderStageCreateInfo_array[0].pName = "main";
    vkPipelineShaderStageCreateInfo_array[0].pSpecializationInfo = NULL;
    
    //fragment Shader
    vkPipelineShaderStageCreateInfo_array[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vkPipelineShaderStageCreateInfo_array[1].pNext = NULL;
    vkPipelineShaderStageCreateInfo_array[1].flags = 0;
    vkPipelineShaderStageCreateInfo_array[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    vkPipelineShaderStageCreateInfo_array[1].module = vkShaderModule_fragment_shader;
    vkPipelineShaderStageCreateInfo_array[1].pName = "main";
    vkPipelineShaderStageCreateInfo_array[1].pSpecializationInfo = NULL;
    
    // pipeline cache
    VkPipelineCacheCreateInfo vkPipelineCacheCreateInfo;
    memset((void*)&vkPipelineCacheCreateInfo, 0, sizeof(VkPipelineCacheCreateInfo));
    vkPipelineCacheCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
    vkPipelineCacheCreateInfo.pNext = NULL;
    vkPipelineCacheCreateInfo.flags = 0;
    
    VkPipelineCache vkPipelineCache = VK_NULL_HANDLE; 
    vkResult = vkCreatePipelineCache(vkDevice, &vkPipelineCacheCreateInfo, NULL, &vkPipelineCache);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "createPipeline() --> vkCreatePipelineCache() is failed error code is %d\n", vkResult);
        return vkResult;
    }
    else
    {
        fprintf(gpFile, "createPipeline() --> vkCreatePipelineCache() is succedded\n");
    }
    
    // graphics pipeline
    VkGraphicsPipelineCreateInfo vkGraphicsPipelineCreateInfo;
    memset((void*)&vkGraphicsPipelineCreateInfo, 0, sizeof(VkGraphicsPipelineCreateInfo));
    vkGraphicsPipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    vkGraphicsPipelineCreateInfo.pNext = NULL;
    vkGraphicsPipelineCreateInfo.flags = 0;
    vkGraphicsPipelineCreateInfo.pVertexInputState   = &vkPipelineVertexInputStateCreateInfo;
    vkGraphicsPipelineCreateInfo.pInputAssemblyState = &vkPipelineInputAssemblyStateCreateInfo;
    vkGraphicsPipelineCreateInfo.pRasterizationState = &vkPipelineRasterizationStateCreateInfo;
    vkGraphicsPipelineCreateInfo.pColorBlendState    = &vkPipelineColorBlendStateCreateInfo;
    vkGraphicsPipelineCreateInfo.pViewportState      = &vkPipelineViewportStateCreateInfo;
    vkGraphicsPipelineCreateInfo.pDepthStencilState  = &vkPipelineDepthStencilStateCreateInfo;
    vkGraphicsPipelineCreateInfo.pDynamicState       = NULL;
    vkGraphicsPipelineCreateInfo.pMultisampleState   = &vkPipelineMultisampleStateCreateInfo;
    vkGraphicsPipelineCreateInfo.stageCount          = _ARRAYSIZE(vkPipelineShaderStageCreateInfo_array);
    vkGraphicsPipelineCreateInfo.pStages             = vkPipelineShaderStageCreateInfo_array;
    vkGraphicsPipelineCreateInfo.pTessellationState  = NULL;
    vkGraphicsPipelineCreateInfo.layout              = vkPipelineLayout;
    vkGraphicsPipelineCreateInfo.renderPass          = vkRenderPass;
    vkGraphicsPipelineCreateInfo.subpass             = 0;
    vkGraphicsPipelineCreateInfo.basePipelineHandle  = VK_NULL_HANDLE;
    vkGraphicsPipelineCreateInfo.basePipelineIndex   = 0;
    
    vkResult = vkCreateGraphicsPipelines(vkDevice,
                                         vkPipelineCache,
                                         1,
                                         &vkGraphicsPipelineCreateInfo,
                                         NULL,
                                         &vkPipeline);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "createPipeline() --> vkCreateGraphicsPipelines() is failed error code is %d\n", vkResult);
        vkDestroyPipelineCache(vkDevice, vkPipelineCache, NULL);
        vkPipelineCache = VK_NULL_HANDLE;
        return vkResult;
    }
    else
    {
        fprintf(gpFile, "createPipeline() --> vkCreateGraphicsPipelines() is succedded\n");
    }
    
    vkDestroyPipelineCache(vkDevice, vkPipelineCache, NULL);
    vkPipelineCache = VK_NULL_HANDLE;
    
    return (vkResult);
}

VkResult createFrameBuffers(void)
{
    //variables
    VkResult vkResult = VK_SUCCESS;
    
    //code
    // allocate framebuffer array
    vkFrameBuffer_array = (VkFramebuffer*)malloc(sizeof(VkFramebuffer) * swapchainImageCount);
    //check for malloc
    
    for(uint32_t i = 0; i < swapchainImageCount; i++)
    {
        // Two attachments: [0]=color swapchain image view, [1]=depth view
        VkImageView vkImageView_attachment_array[2];
        memset((void*)vkImageView_attachment_array, 0, sizeof(VkImageView) * _ARRAYSIZE(vkImageView_attachment_array));
        vkImageView_attachment_array[0] = swapchainImageView_array[i];
        vkImageView_attachment_array[1] = vkImageView_depth;

        VkFramebufferCreateInfo vkFramebufferCreateInfo;
        memset((void*)&vkFramebufferCreateInfo, 0, sizeof(VkFramebufferCreateInfo));
        vkFramebufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        vkFramebufferCreateInfo.pNext = NULL;
        vkFramebufferCreateInfo.flags = 0;
        vkFramebufferCreateInfo.renderPass = vkRenderPass;
        vkFramebufferCreateInfo.attachmentCount = _ARRAYSIZE(vkImageView_attachment_array);
        vkFramebufferCreateInfo.pAttachments = vkImageView_attachment_array;
        vkFramebufferCreateInfo.width  = vkExtent2D_swapchain.width;
        vkFramebufferCreateInfo.height = vkExtent2D_swapchain.height;
        vkFramebufferCreateInfo.layers = 1;
        
        vkResult = vkCreateFramebuffer(vkDevice, 
                                       &vkFramebufferCreateInfo,
                                       NULL,
                                       &vkFrameBuffer_array[i]);
        if(vkResult != VK_SUCCESS)
        {
            fprintf(gpFile, "createFrameBuffers() --> vkCreateFramebuffer() is failed for %d iteration and error code is %d\n",i, vkResult);
            return vkResult;
        }
        else
        {
            fprintf(gpFile, "createFrameBuffers() --> vkCreateFramebuffer() is succedded for iteration %d\n", i);
        }
    }
    
    return vkResult;
}

VkResult createSemaphores(void)
{
    //variables
    VkResult vkResult = VK_SUCCESS;
    
    //code
    //step1
    VkSemaphoreCreateInfo vkSemaphoreCreateInfo;
    memset((void*)&vkSemaphoreCreateInfo, 0, sizeof(VkSemaphoreCreateInfo));
    
    vkSemaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    vkSemaphoreCreateInfo.pNext = NULL;  //Binary and Timeline Semaphore info, bydefault it is Binary
    vkSemaphoreCreateInfo.flags = 0; //RESERVED: must be zero
    
    //backBuffer Semaphore
    vkResult = vkCreateSemaphore(vkDevice, 
                                 &vkSemaphoreCreateInfo,
                                 NULL,
                                 &vkSemaphore_backBuffer);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "createSemaphores() --> vkCreateSemaphore() is failed for vkSemaphore_backBuffer and error code is %d\n", vkResult);
        return vkResult;
    }
    else
    {
        fprintf(gpFile, "createSemaphores() --> vkCreateSemaphore() is succedded for vkSemaphore_backBuffer\n");
    }
    
    
    //renderComplete Semaphore
    vkResult = vkCreateSemaphore(vkDevice, 
                                 &vkSemaphoreCreateInfo,
                                 NULL,
                                 &vkSemaphore_renderComplete);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "createSemaphores() --> vkCreateSemaphore() is failed for vkSemaphore_renderComplete and error code is %d\n", vkResult);
        return vkResult;
    }
    else
    {
        fprintf(gpFile, "createSemaphores() --> vkCreateSemaphore() is succedded for vkSemaphore_renderComplete\n");
    }
    
    return vkResult;
    
}

VkResult createFences(void)
{
    //variables
    VkResult vkResult = VK_SUCCESS;
    
    //code
    //step1
    VkFenceCreateInfo vkFenceCreateInfo;
    memset((void*)&vkFenceCreateInfo, 0, sizeof(VkFenceCreateInfo));
    
    vkFenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    vkFenceCreateInfo.pNext = NULL;
    vkFenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
    
    // ***** FIX: allocate swapchainImageCount fences (not sizeof(swapchainImageCount)) *****
    vkFence_array = (VkFence*)malloc(sizeof(VkFence) * swapchainImageCount);
    //malloc error checking to be done
    
    for(uint32_t i = 0; i < swapchainImageCount; i++)
    {
        vkResult = vkCreateFence(vkDevice, 
                                 &vkFenceCreateInfo,
                                 NULL,
                                 &vkFence_array[i]);
        if(vkResult != VK_SUCCESS)
        {
            fprintf(gpFile, "createFences() --> vkCreateFence() is failed for %d iteration and error code is %d\n", i, vkResult);
            return vkResult;
        }
        else
        {
            fprintf(gpFile, "createFences() --> vkCreateFence() is succedded for %d iteration\n", i);
        }
    }

    return vkResult;
}

VkResult buildCommandBuffers(void)
{
    //variables
    VkResult vkResult = VK_SUCCESS;
    
    //code
    //step1
    //Loop per swapchainImagecount
    for(uint32_t i = 0; i < swapchainImageCount; i++)
    {
        //ResetCommandBuffers
        vkResult = vkResetCommandBuffer(vkCommandBuffer_array[i], 0);  // second parameetr: this command buffer is created from command pool, so dont release the resouces created by this comand buffer
        if(vkResult != VK_SUCCESS)
        {
            fprintf(gpFile, "buildCommandBuffers() --> vkResetCommandBuffer() is failed for %d iteration and error code is %d\n", i, vkResult);
            return vkResult;
        }
        else
        {
            fprintf(gpFile, "buildCommandBuffers() --> vkResetCommandBuffer() is succedded for %d iteration\n", i);
        }
        
        VkCommandBufferBeginInfo vkCommandBufferBeginInfo;
        memset((void*)&vkCommandBufferBeginInfo, 0, sizeof(VkCommandBufferBeginInfo));
        
        vkCommandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        vkCommandBufferBeginInfo.pNext = NULL;
        vkCommandBufferBeginInfo.flags = 0;  //1. we will use only primary command buffers, 2. we are not going to use this command buffer simultaneoulsy between multipple threads
        
        vkResult = vkBeginCommandBuffer(vkCommandBuffer_array[i], &vkCommandBufferBeginInfo);
        if(vkResult != VK_SUCCESS)
        {
            fprintf(gpFile, "buildCommandBuffers() --> vkBeginCommandBuffer() is failed for %d iteration and error code is %d\n", i, vkResult);
            return vkResult;
        }
        else
        {
            fprintf(gpFile, "buildCommandBuffers() --> vkBeginCommandBuffer() is succedded for %d iteration\n", i);
        }
        
        //SetClearValues
        VkClearValue vkClearValue_array[2];
        memset((void*)vkClearValue_array, 0, (sizeof(VkClearValue) * _ARRAYSIZE(vkClearValue_array)));
        
        vkClearValue_array[0].color = vkClearColorValue;
        vkClearValue_array[1].depthStencil = vkClearDepthStencilValue;
        
        //Fill REnderpass 
        VkRenderPassBeginInfo vkRenderPassBeginInfo;
        memset((void*)&vkRenderPassBeginInfo, 0, sizeof(VkRenderPassBeginInfo));
        
        vkRenderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        vkRenderPassBeginInfo.pNext = NULL;
        vkRenderPassBeginInfo.renderPass = vkRenderPass;
        vkRenderPassBeginInfo.renderArea.offset.x = 0;
        vkRenderPassBeginInfo.renderArea.offset.y = 0;
        vkRenderPassBeginInfo.renderArea.extent.width = vkExtent2D_swapchain.width;
        vkRenderPassBeginInfo.renderArea.extent.height = vkExtent2D_swapchain.height;
        vkRenderPassBeginInfo.clearValueCount = _ARRAYSIZE(vkClearValue_array);
        vkRenderPassBeginInfo.pClearValues = vkClearValue_array;
        vkRenderPassBeginInfo.framebuffer = vkFrameBuffer_array[i];
        
        //BeginRenderPass
        vkCmdBeginRenderPass(vkCommandBuffer_array[i], 
                             &vkRenderPassBeginInfo, 
                             VK_SUBPASS_CONTENTS_INLINE);

        //Bind with the pipeline
        vkCmdBindPipeline(vkCommandBuffer_array[i],
                          VK_PIPELINE_BIND_POINT_GRAPHICS,
                          vkPipeline);
        //Bind our descriptor set with the pipeline
        vkCmdBindDescriptorSets(vkCommandBuffer_array[i],
                                VK_PIPELINE_BIND_POINT_GRAPHICS,
                                vkPipelineLayout,
                                0,
                                1,
                                &vkDescriptorSet,
                                0, 
                                NULL);
        
        //bind vertex position buffer
        VkDeviceSize vkDeviceSize_offset_position[1];
        memset((void*)vkDeviceSize_offset_position, 0, sizeof(VkDeviceSize) * _ARRAYSIZE(vkDeviceSize_offset_position));
        vkCmdBindVertexBuffers(vkCommandBuffer_array[i], 
                               0, //binding point
                               1, //how many buffer
                               &vertexData_position.vkBuffer, //which buffer
                               vkDeviceSize_offset_position);
        
        //bind vertex color buffer
        VkDeviceSize vkDeviceSize_offset_texcoord[1];
        memset((void*)vkDeviceSize_offset_texcoord, 0, sizeof(VkDeviceSize) * _ARRAYSIZE(vkDeviceSize_offset_texcoord));
        vkCmdBindVertexBuffers(vkCommandBuffer_array[i], 
                               1, //binding point
                               1, //how many buffer
                               &vertexData_texcoord.vkBuffer, //which buffer
                               vkDeviceSize_offset_texcoord);
                                     
        //Here we should call vulkan drawing functions
       vkCmdDraw(vkCommandBuffer_array[i],
          gSphereVertexCount,  // vertices
          1,                   // instances
          0,                   // firstVertex
          0);                  // firstInstance

        
        //EndRenderPass
        vkCmdEndRenderPass(vkCommandBuffer_array[i]);
        
        //End Command Buffer recording
        vkResult = vkEndCommandBuffer(vkCommandBuffer_array[i]);
        if(vkResult != VK_SUCCESS)
        {
            fprintf(gpFile, "buildCommandBuffers() --> vkEndCommandBuffer() is failed for %d iteration and error code is %d\n", i, vkResult);
            return vkResult;
        }
        else
        {
            fprintf(gpFile, "buildCommandBuffers() --> vkEndCommandBuffer() is succedded for %d iteration\n", i);
        }
    }
    
    return vkResult;
}


 VKAPI_ATTR VkBool32 VKAPI_CALL debugReportCallback(VkDebugReportFlagsEXT vkDebugReportFlagsEXT, 
                                                   VkDebugReportObjectTypeEXT vkDebugReportObjectTypeEXT,
                                                   uint64_t object,
                                                   size_t location,
                                                   int32_t messageCode,
                                                   const char* pLayerPrefix,
                                                   const char* pMessage,
                                                   void* pUserData)
{
    //code
    fprintf(gpFile, "SSA_Validation: debugReportCallback() --> %s (%d) = %s\n", pLayerPrefix, messageCode, pMessage);
    
    return(VK_FALSE);
}
