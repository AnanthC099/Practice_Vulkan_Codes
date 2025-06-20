/* =========================================================================
 *  Vk_SceneGraph_Cpp98.cpp
 *  ----------------------------------------------------
 *  C++98 implementation of a tiny scene graph renderer:
 *   ─ rotating pyramid and cube
 *   ─ one secondary CB per node
 *   ─ dynamic rendering + depth
 *  ========================================================================= */

#define WIN32_LEAN_AND_MEAN
#define VK_USE_PLATFORM_WIN32_KHR
#include <windows.h>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_win32.h>

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <vector>

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"

#pragma comment(lib, "vulkan-1.lib")

/* ---------------- very small logger ---------------- */
static FILE* gLOG = NULL;
#define LOG(fmt, ...)  do{ if(gLOG)fprintf(gLOG,"LOG  : " fmt "\n",##__VA_ARGS__);}while(0)
#define ERR(fmt, ...)  do{ if(gLOG)fprintf(gLOG,"ERROR: " fmt "\n",##__VA_ARGS__);}while(0)
#define VK_CHECK(x) \
    do{ VkResult _r=(x); if(_r!=VK_SUCCESS){ ERR(#x " failed (%d)",_r); return _r;} }while(0)

/* ============ Win32 globals ============ */
static const char* APP_NAME = "Vulkan Scene‑Graph (C++98)";
static const int   WIN_W = 800, WIN_H = 600;
static HWND        gHWND  = NULL;
static BOOL        gRun   = FALSE;

/* forward declaration */
static LRESULT CALLBACK WndProc(HWND,UINT,WPARAM,LPARAM);

/* ============ Vulkan handles ============ */
static bool             gUseValidation = true;
static VkInstance       gInst          = VK_NULL_HANDLE;
static VkSurfaceKHR     gSurface       = VK_NULL_HANDLE;
static VkPhysicalDevice gPhys          = VK_NULL_HANDLE;
static VkDevice         gDev           = VK_NULL_HANDLE;
static VkQueue          gGfxQ          = VK_NULL_HANDLE;
static uint32_t         gGfxFamily     = 0;

/* swap‑chain */
static VkSwapchainKHR                 gSwap        = VK_NULL_HANDLE;
static VkFormat                       gColorFormat = VK_FORMAT_UNDEFINED;
static VkExtent2D                     gExtent;
static std::vector<VkImage>           gImages;
static std::vector<VkImageView>       gViews;

/* depth */
static VkFormat        gDepthFormat = VK_FORMAT_UNDEFINED;
static VkImage         gDepthImg    = VK_NULL_HANDLE;
static VkDeviceMemory  gDepthMem    = VK_NULL_HANDLE;
static VkImageView     gDepthView   = VK_NULL_HANDLE;

/* command pools / buffers / sync */
static VkCommandPool                gCmdPool  = VK_NULL_HANDLE;
static std::vector<VkCommandBuffer> gPrimary;             /* 1 per swap image */
static VkSemaphore                  gImgAvail[2];
static VkSemaphore                  gRenderDone[2];
static VkFence                      gInFlight [2];
static uint32_t                     gFrame = 0;

/* dynamic rendering entry points */
static PFN_vkCmdBeginRendering  pfnBeginRendering = NULL;
static PFN_vkCmdEndRendering    pfnEndRendering   = NULL;

/* helper : memory properties */
static VkPhysicalDeviceMemoryProperties gMemProps;
static uint32_t FindMemType(uint32_t bits, VkMemoryPropertyFlags flags)
{
    for(uint32_t i=0;i<gMemProps.memoryTypeCount;++i)
        if( (bits&(1u<<i)) &&
            (gMemProps.memoryTypes[i].propertyFlags & flags) == flags)
            return i;
    return UINT32_MAX;
}

/* ---------------------------------------------------- */
/*                     Vertex data                      */
/* ---------------------------------------------------- */
struct Vertex { float pos[3]; float col[3]; };

/* pyramid (18 verts) */
static const Vertex PYR[] = {
    {{ 0,  0.5f, 0 },{1,0,0}}, {{ 0.5f,-0.5f, 0.5f},{1,0,0}}, {{-0.5f,-0.5f, 0.5f},{1,0,0}},
    {{ 0,  0.5f, 0 },{0,1,0}}, {{ 0.5f,-0.5f,-0.5f},{0,1,0}}, {{ 0.5f,-0.5f, 0.5f},{0,1,0}},
    {{ 0,  0.5f, 0 },{0.2f,0.5f,1}},{{-0.5f,-0.5f,-0.5f},{0.2f,0.5f,1}},{{ 0.5f,-0.5f,-0.5f},{0.2f,0.5f,1}},
    {{ 0,  0.5f, 0 },{1,1,0}},{{-0.5f,-0.5f, 0.5f},{1,1,0}},{{-0.5f,-0.5f,-0.5f},{1,1,0}},
    {{-0.5f,-0.5f, 0.5f},{0,1,1}},{{ 0.5f,-0.5f,-0.5f},{0,1,1}},{{ 0.5f,-0.5f, 0.5f},{0,1,1}},
    {{ 0.5f,-0.5f,-0.5f},{0,1,1}},{{-0.5f,-0.5f, 0.5f},{0,1,1}},{{-0.5f,-0.5f,-0.5f},{0,1,1}}
};
/* cube (36 verts) */
static const Vertex CUB[] = {
    {{-0.5f,-0.5f, 0.5f},{1,0,0}},{{ 0.5f,-0.5f, 0.5f},{1,0,0}},{{ 0.5f, 0.5f, 0.5f},{1,0,0}},
    {{ 0.5f, 0.5f, 0.5f},{1,0,0}},{{-0.5f, 0.5f, 0.5f},{1,0,0}},{{-0.5f,-0.5f, 0.5f},{1,0,0}},

    {{ 0.5f,-0.5f, 0.5f},{0,1,0}},{{ 0.5f,-0.5f,-0.5f},{0,1,0}},{{ 0.5f, 0.5f,-0.5f},{0,1,0}},
    {{ 0.5f, 0.5f,-0.5f},{0,1,0}},{{ 0.5f, 0.5f, 0.5f},{0,1,0}},{{ 0.5f,-0.5f, 0.5f},{0,1,0}},

    {{ 0.5f,-0.5f,-0.5f},{0.5f,0.5f,1}},{{-0.5f,-0.5f,-0.5f},{0.5f,0.5f,1}},{{-0.5f, 0.5f,-0.5f},{0.5f,0.5f,1}},
    {{-0.5f, 0.5f,-0.5f},{0.5f,0.5f,1}},{{ 0.5f, 0.5f,-0.5f},{0.5f,0.5f,1}},{{ 0.5f,-0.5f,-0.5f},{0.5f,0.5f,1}},

    {{-0.5f,-0.5f,-0.5f},{1,1,0}},{{-0.5f,-0.5f, 0.5f},{1,1,0}},{{-0.5f, 0.5f, 0.5f},{1,1,0}},
    {{-0.5f, 0.5f, 0.5f},{1,1,0}},{{-0.5f, 0.5f,-0.5f},{1,1,0}},{{-0.5f,-0.5f,-0.5f},{1,1,0}},

    {{-0.5f, 0.5f, 0.5f},{1,0,1}},{{ 0.5f, 0.5f, 0.5f},{1,0,1}},{{ 0.5f, 0.5f,-0.5f},{1,0,1}},
    {{ 0.5f, 0.5f,-0.5f},{1,0,1}},{{-0.5f, 0.5f,-0.5f},{1,0,1}},{{-0.5f, 0.5f, 0.5f},{1,0,1}},

    {{-0.5f,-0.5f,-0.5f},{0,1,1}},{{ 0.5f,-0.5f,-0.5f},{0,1,1}},{{ 0.5f,-0.5f, 0.5f},{0,1,1}},
    {{ 0.5f,-0.5f, 0.5f},{0,1,1}},{{-0.5f,-0.5f, 0.5f},{0,1,1}},{{-0.5f,-0.5f,-0.5f},{0,1,1}}
};

/* ---------------------------------------------------- */
/*                     Scene node                       */
/* ---------------------------------------------------- */
struct Node {
    const Vertex*                 verts;
    uint32_t                      vtxCount;
    VkBuffer                      vbo;
    VkDeviceMemory                vboMem;
    std::vector<VkCommandBuffer>  secCB;   /* one per swap image */
    glm::vec3                     pos;
    glm::vec3                     axis;
    float                         scale;
    /* constructor (C++98 style) */
    Node() :
        verts(NULL), vtxCount(0), vbo(VK_NULL_HANDLE), vboMem(VK_NULL_HANDLE),
        pos(0.0f), axis(0.0f,1.0f,0.0f), scale(1.0f) {}
};

#define NODE_COUNT 2
static Node gNodes[NODE_COUNT];

/* ---------------------------------------------------- */
/*         small helpers for timing (C API)             */
/* ---------------------------------------------------- */
static clock_t gStartClock = 0;
static float SecondsSinceStart()
{
    return (float)(clock() - gStartClock) / (float)CLOCKS_PER_SEC;
}

/* ---------------------------------------------------- */
/*          Vulkan instance / device creation           */
/* ---------------------------------------------------- */
static VkResult CreateInstance()
{
    const char* exts[3];
    exts[0] = VK_KHR_SURFACE_EXTENSION_NAME;
    exts[1] = VK_KHR_WIN32_SURFACE_EXTENSION_NAME;
    exts[2] = VK_EXT_DEBUG_UTILS_EXTENSION_NAME;  /* harmless if absent */

    VkApplicationInfo ai;
    memset(&ai,0,sizeof(ai));
    ai.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    ai.pApplicationName = APP_NAME;
    ai.apiVersion       = VK_API_VERSION_1_3;

    VkInstanceCreateInfo ci;
    memset(&ci,0,sizeof(ci));
    ci.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    ci.pApplicationInfo = &ai;
    ci.enabledExtensionCount = 3;
    ci.ppEnabledExtensionNames = exts;
    return vkCreateInstance(&ci,NULL,&gInst);
}
static VkResult CreateSurface()
{
    VkWin32SurfaceCreateInfoKHR sci;
    memset(&sci,0,sizeof(sci));
    sci.sType     = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
    sci.hinstance = GetModuleHandle(NULL);
    sci.hwnd      = gHWND;
    return vkCreateWin32SurfaceKHR(gInst,&sci,NULL,&gSurface);
}
static VkResult PickPhysical()
{
    uint32_t cnt = 0;
    vkEnumeratePhysicalDevices(gInst,&cnt,NULL);
    if(!cnt) return VK_ERROR_INITIALIZATION_FAILED;
    std::vector<VkPhysicalDevice> devs(cnt);
    vkEnumeratePhysicalDevices(gInst,&cnt,&devs[0]);

    for(uint32_t d=0; d<cnt; ++d)
    {
        uint32_t qc=0;
        vkGetPhysicalDeviceQueueFamilyProperties(devs[d],&qc,NULL);
        std::vector<VkQueueFamilyProperties> props(qc);
        vkGetPhysicalDeviceQueueFamilyProperties(devs[d],&qc,&props[0]);

        for(uint32_t i=0;i<qc;++i)
        {
            VkBool32 present = VK_FALSE;
            vkGetPhysicalDeviceSurfaceSupportKHR(devs[d],i,gSurface,&present);
            if( (props[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) && present )
            {
                gPhys      = devs[d];
                gGfxFamily = i;
                vkGetPhysicalDeviceMemoryProperties(gPhys,&gMemProps);
                return VK_SUCCESS;
            }
        }
    }
    return VK_ERROR_INITIALIZATION_FAILED;
}
static VkResult CreateDevice()
{
    float prio = 1.0f;
    VkDeviceQueueCreateInfo q;
    memset(&q,0,sizeof(q));
    q.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    q.queueFamilyIndex = gGfxFamily;
    q.queueCount = 1;
    q.pQueuePriorities = &prio;

    const char* exts[2];
    exts[0] = VK_KHR_SWAPCHAIN_EXTENSION_NAME;
    exts[1] = "VK_KHR_dynamic_rendering";

    VkPhysicalDeviceDynamicRenderingFeaturesKHR drf;
    memset(&drf,0,sizeof(drf));
    drf.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_FEATURES_KHR;
    drf.dynamicRendering = VK_TRUE;

    VkDeviceCreateInfo ci;
    memset(&ci,0,sizeof(ci));
    ci.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    ci.queueCreateInfoCount = 1;
    ci.pQueueCreateInfos    = &q;
    ci.enabledExtensionCount = 2;
    ci.ppEnabledExtensionNames = exts;
    ci.pNext = &drf;

    VK_CHECK(vkCreateDevice(gPhys,&ci,NULL,&gDev));
    vkGetDeviceQueue(gDev,gGfxFamily,0,&gGfxQ);

    /* load dynamic rendering functions (core first, then KHR) */
    pfnBeginRendering = (PFN_vkCmdBeginRendering)vkGetDeviceProcAddr(gDev,"vkCmdBeginRendering");
    pfnEndRendering   = (PFN_vkCmdEndRendering  )vkGetDeviceProcAddr(gDev,"vkCmdEndRendering");
    if(!pfnBeginRendering)
    {
        pfnBeginRendering = (PFN_vkCmdBeginRendering)vkGetDeviceProcAddr(gDev,"vkCmdBeginRenderingKHR");
        pfnEndRendering   = (PFN_vkCmdEndRendering  )vkGetDeviceProcAddr(gDev,"vkCmdEndRenderingKHR");
    }
    return VK_SUCCESS;
}

/* ---------------------------------------------------- */
/*              swap‑chain & colour targets             */
/* ---------------------------------------------------- */
static VkResult CreateSwap()
{
    /* colour format */
    uint32_t c=0;
    vkGetPhysicalDeviceSurfaceFormatsKHR(gPhys,gSurface,&c,NULL);
    std::vector<VkSurfaceFormatKHR> fmts(c);
    vkGetPhysicalDeviceSurfaceFormatsKHR(gPhys,gSurface,&c,&fmts[0]);
    if(c==1 && fmts[0].format==VK_FORMAT_UNDEFINED)
        gColorFormat = VK_FORMAT_B8G8R8A8_UNORM;
    else
        gColorFormat = fmts[0].format;

    /* extent + capabilities */
    VkSurfaceCapabilitiesKHR caps;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(gPhys,gSurface,&caps);
    gExtent = caps.currentExtent;
    if(gExtent.width==UINT32_MAX)
    {
        gExtent.width  = WIN_W;
        gExtent.height = WIN_H;
    }

    uint32_t desired = caps.minImageCount + 1;
    if(caps.maxImageCount && desired>caps.maxImageCount)
        desired = caps.maxImageCount;

    VkSwapchainCreateInfoKHR sc;
    memset(&sc,0,sizeof(sc));
    sc.sType   = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    sc.surface = gSurface;
    sc.minImageCount    = desired;
    sc.imageFormat      = gColorFormat;
    sc.imageColorSpace  = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
    sc.imageExtent      = gExtent;
    sc.imageArrayLayers = 1;
    sc.imageUsage       = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    sc.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    sc.preTransform     = (caps.supportedTransforms & VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR)?
                           VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR : caps.currentTransform;
    sc.compositeAlpha   = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    sc.presentMode      = VK_PRESENT_MODE_FIFO_KHR;
    sc.clipped          = VK_TRUE;

    VK_CHECK(vkCreateSwapchainKHR(gDev,&sc,NULL,&gSwap));

    uint32_t imgCnt=0;
    vkGetSwapchainImagesKHR(gDev,gSwap,&imgCnt,NULL);
    gImages.resize(imgCnt);
    gViews .resize(imgCnt);
    vkGetSwapchainImagesKHR(gDev,gSwap,&imgCnt,&gImages[0]);

    for(uint32_t i=0;i<imgCnt;++i)
    {
        VkImageViewCreateInfo iv;
        memset(&iv,0,sizeof(iv));
        iv.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        iv.image = gImages[i];
        iv.viewType = VK_IMAGE_VIEW_TYPE_2D;
        iv.format   = gColorFormat;
        iv.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        iv.subresourceRange.levelCount = 1;
        iv.subresourceRange.layerCount = 1;
        VK_CHECK(vkCreateImageView(gDev,&iv,NULL,&gViews[i]));
    }
    return VK_SUCCESS;
}

/* ---------------------------------------------------- */
/*                    depth buffer                      */
/* ---------------------------------------------------- */
static VkFormat ChooseDepthFormat()
{
    VkFormat cand[3] = { VK_FORMAT_D32_SFLOAT,
                         VK_FORMAT_D24_UNORM_S8_UINT,
                         VK_FORMAT_D16_UNORM };
    for(int i=0;i<3;++i)
    {
        VkFormatProperties p;
        vkGetPhysicalDeviceFormatProperties(gPhys,cand[i],&p);
        if(p.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT)
            return cand[i];
    }
    return VK_FORMAT_UNDEFINED;
}
static VkResult CreateDepth()
{
    gDepthFormat = ChooseDepthFormat();
    if(gDepthFormat==VK_FORMAT_UNDEFINED) return VK_ERROR_FORMAT_NOT_SUPPORTED;

    VkImageCreateInfo im;
    memset(&im,0,sizeof(im));
    im.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    im.imageType = VK_IMAGE_TYPE_2D;
    im.format    = gDepthFormat;
    im.extent.width  = gExtent.width;
    im.extent.height = gExtent.height;
    im.extent.depth  = 1;
    im.mipLevels   = 1;
    im.arrayLayers = 1;
    im.samples     = VK_SAMPLE_COUNT_1_BIT;
    im.tiling      = VK_IMAGE_TILING_OPTIMAL;
    im.usage       = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;

    VK_CHECK(vkCreateImage(gDev,&im,NULL,&gDepthImg));

    VkMemoryRequirements req;
    vkGetImageMemoryRequirements(gDev,gDepthImg,&req);
    uint32_t mt = FindMemType(req.memoryTypeBits,VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    VkMemoryAllocateInfo ai;
    memset(&ai,0,sizeof(ai));
    ai.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    ai.allocationSize  = req.size;
    ai.memoryTypeIndex = mt;
    VK_CHECK(vkAllocateMemory(gDev,&ai,NULL,&gDepthMem));
    vkBindImageMemory(gDev,gDepthImg,gDepthMem,0);

    VkImageViewCreateInfo iv;
    memset(&iv,0,sizeof(iv));
    iv.sType   = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    iv.image   = gDepthImg;
    iv.viewType= VK_IMAGE_VIEW_TYPE_2D;
    iv.format  = gDepthFormat;
    iv.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT |
        ((gDepthFormat==VK_FORMAT_D24_UNORM_S8_UINT)?VK_IMAGE_ASPECT_STENCIL_BIT:0);
    iv.subresourceRange.levelCount = 1;
    iv.subresourceRange.layerCount = 1;
    VK_CHECK(vkCreateImageView(gDev,&iv,NULL,&gDepthView));

    /* one‑time layout transition */
    VkCommandPool tp;
    VkCommandPoolCreateInfo pci;
    memset(&pci,0,sizeof(pci));
    pci.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    pci.queueFamilyIndex = gGfxFamily;
    pci.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
    VK_CHECK(vkCreateCommandPool(gDev,&pci,NULL,&tp));

    VkCommandBuffer cb;
    VkCommandBufferAllocateInfo cbai;
    memset(&cbai,0,sizeof(cbai));
    cbai.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    cbai.commandPool   = tp;
    cbai.level         = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    cbai.commandBufferCount = 1;
    vkAllocateCommandBuffers(gDev,&cbai,&cb);

    VkCommandBufferBeginInfo bi;
    memset(&bi,0,sizeof(bi));
    bi.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    bi.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    vkBeginCommandBuffer(cb,&bi);

    VkImageMemoryBarrier bar;
    memset(&bar,0,sizeof(bar));
    bar.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    bar.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    bar.newLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
    bar.image     = gDepthImg;
    bar.subresourceRange = iv.subresourceRange;
    bar.srcAccessMask = 0;
    bar.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

    vkCmdPipelineBarrier(cb,
        VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
        VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
        0,0,NULL,0,NULL,1,&bar);

    vkEndCommandBuffer(cb);
    VkSubmitInfo si;
    memset(&si,0,sizeof(si));
    si.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    si.commandBufferCount = 1;
    si.pCommandBuffers    = &cb;
    vkQueueSubmit(gGfxQ,1,&si,VK_NULL_HANDLE);
    vkQueueWaitIdle(gGfxQ);

    vkFreeCommandBuffers(gDev,tp,1,&cb);
    vkDestroyCommandPool(gDev,tp,NULL);
    return VK_SUCCESS;
}

/* ---------------------------------------------------- */
/*          command pool / primaries / secondaries      */
/* ---------------------------------------------------- */
static VkResult CreateCmdAndSync()
{
    VkCommandPoolCreateInfo pci;
    memset(&pci,0,sizeof(pci));
    pci.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    pci.queueFamilyIndex = gGfxFamily;
    pci.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    VK_CHECK(vkCreateCommandPool(gDev,&pci,NULL,&gCmdPool));

    /* primary CBs */
    gPrimary.resize(gImages.size());
    VkCommandBufferAllocateInfo cbai;
    memset(&cbai,0,sizeof(cbai));
    cbai.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    cbai.commandPool        = gCmdPool;
    cbai.level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    cbai.commandBufferCount = (uint32_t)gPrimary.size();
    VK_CHECK(vkAllocateCommandBuffers(gDev,&cbai,&gPrimary[0]));

    /* secondaries : allocate once per node */
    cbai.level = VK_COMMAND_BUFFER_LEVEL_SECONDARY;
    for(int n=0;n<NODE_COUNT;++n)
    {
        gNodes[n].secCB.resize(gImages.size());
        cbai.commandBufferCount = (uint32_t)gNodes[n].secCB.size();
        VK_CHECK(vkAllocateCommandBuffers(gDev,&cbai,&gNodes[n].secCB[0]));
    }

    /* semaphores / fences */
    VkSemaphoreCreateInfo sci;
    memset(&sci,0,sizeof(sci));
    sci.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    VkFenceCreateInfo fci;
    memset(&fci,0,sizeof(fci));
    fci.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fci.flags = VK_FENCE_CREATE_SIGNALED_BIT;
    for(int i=0;i<2;++i)
    {
        VK_CHECK(vkCreateSemaphore(gDev,&sci,NULL,&gImgAvail[i]));
        VK_CHECK(vkCreateSemaphore(gDev,&sci,NULL,&gRenderDone[i]));
        VK_CHECK(vkCreateFence    (gDev,&fci,NULL,&gInFlight [i]));
    }
    return VK_SUCCESS;
}

/* ---------------------------------------------------- */
/*                 graphics pipeline                    */
/* ---------------------------------------------------- */
static VkPipelineLayout gPipeLayout = VK_NULL_HANDLE;
static VkPipeline       gPipeline   = VK_NULL_HANDLE;

static VkShaderModule LoadModule(const char* path)
{
    FILE* fp = fopen(path,"rb");
    if(!fp){ ERR("shader %s not found",path); return VK_NULL_HANDLE; }
    fseek(fp,0,SEEK_END); long sz = ftell(fp); rewind(fp);
    std::vector<char> buf(sz);
    fread(&buf[0],1,sz,fp); fclose(fp);

    VkShaderModuleCreateInfo ci;
    memset(&ci,0,sizeof(ci));
    ci.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    ci.codeSize = sz;
    ci.pCode    = (const uint32_t*)&buf[0];
    VkShaderModule m;
    if(vkCreateShaderModule(gDev,&ci,NULL,&m)!=VK_SUCCESS) return VK_NULL_HANDLE;
    return m;
}
static VkResult CreatePipeline()
{
    VkShaderModule vert = LoadModule("pyramid.vert.spv");
    VkShaderModule frag = LoadModule("pyramid.frag.spv");
    if(!vert || !frag) return VK_ERROR_INITIALIZATION_FAILED;

    VkPipelineShaderStageCreateInfo stg[2];
    memset(stg,0,sizeof(stg));
    stg[0].sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    stg[0].stage  = VK_SHADER_STAGE_VERTEX_BIT;
    stg[0].module = vert; stg[0].pName = "main";
    stg[1] = stg[0];
    stg[1].stage  = VK_SHADER_STAGE_FRAGMENT_BIT;
    stg[1].module = frag;

    VkVertexInputBindingDescription   bind;
    bind.binding = 0; bind.stride = sizeof(Vertex); bind.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
    VkVertexInputAttributeDescription attr[2];
    attr[0].location=0; attr[0].binding=0; attr[0].format=VK_FORMAT_R32G32B32_SFLOAT; attr[0].offset=0;
    attr[1].location=1; attr[1].binding=0; attr[1].format=VK_FORMAT_R32G32B32_SFLOAT; attr[1].offset=sizeof(float)*3;

    VkPipelineVertexInputStateCreateInfo vi;
    memset(&vi,0,sizeof(vi));
    vi.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vi.vertexBindingDescriptionCount   = 1;
    vi.pVertexBindingDescriptions      = &bind;
    vi.vertexAttributeDescriptionCount = 2;
    vi.pVertexAttributeDescriptions    = attr;

    VkPipelineInputAssemblyStateCreateInfo ia;
    memset(&ia,0,sizeof(ia));
    ia.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    ia.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

    VkPipelineViewportStateCreateInfo vp;
    memset(&vp,0,sizeof(vp));
    vp.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    vp.viewportCount = 1; vp.scissorCount = 1;

    VkPipelineRasterizationStateCreateInfo rs;
    memset(&rs,0,sizeof(rs));
    rs.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rs.cullMode = VK_CULL_MODE_NONE;
    rs.frontFace= VK_FRONT_FACE_CLOCKWISE;
    rs.polygonMode = VK_POLYGON_MODE_FILL;
    rs.lineWidth = 1.0f;

    VkPipelineMultisampleStateCreateInfo ms;
    memset(&ms,0,sizeof(ms));
    ms.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    ms.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

    VkPipelineDepthStencilStateCreateInfo ds;
    memset(&ds,0,sizeof(ds));
    ds.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    ds.depthTestEnable  = VK_TRUE;
    ds.depthWriteEnable = VK_TRUE;
    ds.depthCompareOp   = VK_COMPARE_OP_LESS;

    VkPipelineColorBlendAttachmentState cba;
    memset(&cba,0,sizeof(cba));
    cba.colorWriteMask = VK_COLOR_COMPONENT_R_BIT|VK_COLOR_COMPONENT_G_BIT|
                         VK_COLOR_COMPONENT_B_BIT|VK_COLOR_COMPONENT_A_BIT;
    VkPipelineColorBlendStateCreateInfo cb;
    memset(&cb,0,sizeof(cb));
    cb.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    cb.attachmentCount = 1;
    cb.pAttachments    = &cba;

    VkDynamicState dynStates[2] = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
    VkPipelineDynamicStateCreateInfo dyn;
    memset(&dyn,0,sizeof(dyn));
    dyn.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dyn.dynamicStateCount = 2;
    dyn.pDynamicStates    = dynStates;

    VkPushConstantRange pc;
    pc.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    pc.offset     = 0;
    pc.size       = sizeof(glm::mat4);

    VkPipelineLayoutCreateInfo plci;
    memset(&plci,0,sizeof(plci));
    plci.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    plci.pushConstantRangeCount = 1;
    plci.pPushConstantRanges    = &pc;
    VK_CHECK(vkCreatePipelineLayout(gDev,&plci,NULL,&gPipeLayout));

    VkPipelineRenderingCreateInfo ri;
    memset(&ri,0,sizeof(ri));
    ri.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO;
    ri.colorAttachmentCount    = 1;
    ri.pColorAttachmentFormats = &gColorFormat;
    ri.depthAttachmentFormat   = gDepthFormat;

    VkGraphicsPipelineCreateInfo pi;
    memset(&pi,0,sizeof(pi));
    pi.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pi.stageCount          = 2;
    pi.pStages             = stg;
    pi.pVertexInputState   = &vi;
    pi.pInputAssemblyState = &ia;
    pi.pViewportState      = &vp;
    pi.pRasterizationState = &rs;
    pi.pMultisampleState   = &ms;
    pi.pDepthStencilState  = &ds;
    pi.pColorBlendState    = &cb;
    pi.pDynamicState       = &dyn;
    pi.layout              = gPipeLayout;
    pi.pNext               = &ri;

    VK_CHECK(vkCreateGraphicsPipelines(gDev,VK_NULL_HANDLE,1,&pi,NULL,&gPipeline));
    vkDestroyShaderModule(gDev,vert,NULL);
    vkDestroyShaderModule(gDev,frag,NULL);
    return VK_SUCCESS;
}

/* ---------------------------------------------------- */
/*          vertex buffers for every node               */
/* ---------------------------------------------------- */
static VkResult CreateVB()
{
    for(int n=0;n<NODE_COUNT;++n)
    {
        const Vertex* src;
        uint32_t cnt;
        if(n==0){ src = PYR; cnt = sizeof(PYR)/sizeof(Vertex); }
        else    { src = CUB; cnt = sizeof(CUB)/sizeof(Vertex); }

        gNodes[n].verts    = src;
        gNodes[n].vtxCount = cnt;
        gNodes[n].pos      = (n==0)? glm::vec3(-1.5f,0,-6) : glm::vec3( 1.5f,0,-6);
        gNodes[n].axis     = glm::vec3(0,1,0);
        gNodes[n].scale    = 0.75f;

        VkBufferCreateInfo bi;
        memset(&bi,0,sizeof(bi));
        bi.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bi.size  = cnt*sizeof(Vertex);
        bi.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
        VK_CHECK(vkCreateBuffer(gDev,&bi,NULL,&gNodes[n].vbo));

        VkMemoryRequirements req;
        vkGetBufferMemoryRequirements(gDev,gNodes[n].vbo,&req);
        uint32_t mt = FindMemType(req.memoryTypeBits,
                                  VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                                  VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

        VkMemoryAllocateInfo ai;
        memset(&ai,0,sizeof(ai));
        ai.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        ai.allocationSize  = req.size;
        ai.memoryTypeIndex = mt;
        VK_CHECK(vkAllocateMemory(gDev,&ai,NULL,&gNodes[n].vboMem));
        vkBindBufferMemory(gDev,gNodes[n].vbo,gNodes[n].vboMem,0);

        void* dst = NULL;
        vkMapMemory(gDev,gNodes[n].vboMem,0,bi.size,0,&dst);
        memcpy(dst,src,bi.size);
        vkUnmapMemory(gDev,gNodes[n].vboMem);
    }
    return VK_SUCCESS;
}

/* ---------------------------------------------------- */
/*          record one secondary CB for node            */
/* ---------------------------------------------------- */
static VkResult RecordNodeCB(int nodeIdx, uint32_t imgIdx, float angle)
{
    Node& node = gNodes[nodeIdx];
    VkCommandBuffer cb = node.secCB[imgIdx];
    vkResetCommandBuffer(cb,0);

    /* inheritance info */
    VkCommandBufferInheritanceRenderingInfo inhRI;
    memset(&inhRI,0,sizeof(inhRI));
    inhRI.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_RENDERING_INFO;
    inhRI.colorAttachmentCount    = 1;
    inhRI.pColorAttachmentFormats = &gColorFormat;
    inhRI.depthAttachmentFormat   = gDepthFormat;

    VkCommandBufferInheritanceInfo inh;
    memset(&inh,0,sizeof(inh));
    inh.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;
    inh.pNext = &inhRI;

    VkCommandBufferBeginInfo bi;
    memset(&bi,0,sizeof(bi));
    bi.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    bi.flags = VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
    bi.pInheritanceInfo = &inh;

    VK_CHECK(vkBeginCommandBuffer(cb,&bi));

    VkViewport vp; memset(&vp,0,sizeof(vp));
    vp.width  = (float)gExtent.width;
    vp.height = -(float)gExtent.height; /* negative to flip Y */
    vp.y      = (float)gExtent.height;
    vp.minDepth = 0.0f; vp.maxDepth = 1.0f;
    VkRect2D sc; sc.offset.x = sc.offset.y = 0; sc.extent = gExtent;
    vkCmdSetViewport(cb,0,1,&vp);
    vkCmdSetScissor (cb,0,1,&sc);

    vkCmdBindPipeline(cb,VK_PIPELINE_BIND_POINT_GRAPHICS,gPipeline);
    VkDeviceSize off = 0;
    vkCmdBindVertexBuffers(cb,0,1,&node.vbo,&off);

    float aspect = (float)gExtent.width / (float)gExtent.height;
    glm::mat4 proj = glm::perspective(glm::radians(45.0f),aspect,0.1f,100.0f);
    glm::mat4 model = glm::translate(glm::mat4(1.0f),node.pos);
    model = glm::rotate(model,angle,node.axis);
    model = glm::scale (model,glm::vec3(node.scale));
    glm::mat4 mvp = proj * model;

    vkCmdPushConstants(cb,gPipeLayout,VK_SHADER_STAGE_VERTEX_BIT,
                       0,sizeof(glm::mat4),&mvp[0][0]);

    vkCmdDraw(cb,node.vtxCount,1,0,0);

    return vkEndCommandBuffer(cb);
}

/* ---------------------------------------------------- */
/*          record primary CB for one image             */
/* ---------------------------------------------------- */
static VkClearColorValue        gClr = { {0,0,1,1} };
static VkClearDepthStencilValue gDep = { 1.0f, 0 };

static VkResult RecordPrimary(uint32_t imgIdx, float angle)
{
    /* record all secondaries first */
    for(int n=0; n<NODE_COUNT;++n)
        VK_CHECK(RecordNodeCB(n,imgIdx,angle));

    VkCommandBuffer cb = gPrimary[imgIdx];
    vkResetCommandBuffer(cb,0);

    VkCommandBufferBeginInfo bi;
    memset(&bi,0,sizeof(bi));
    bi.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    VK_CHECK(vkBeginCommandBuffer(cb,&bi));

    /* barrier colour UNDEFINED → COLOR_ATTACHMENT_OPTIMAL */
    VkImageMemoryBarrier toClr;
    memset(&toClr,0,sizeof(toClr));
    toClr.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    toClr.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    toClr.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    toClr.image     = gImages[imgIdx];
    toClr.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    toClr.subresourceRange.levelCount = 1;
    toClr.subresourceRange.layerCount = 1;
    toClr.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

    vkCmdPipelineBarrier(cb,
        VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
        0,0,NULL,0,NULL,1,&toClr);

    /* begin rendering */
    VkRenderingAttachmentInfo colAtt;
    memset(&colAtt,0,sizeof(colAtt));
    colAtt.sType       = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
    colAtt.imageView   = gViews[imgIdx];
    colAtt.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    colAtt.loadOp      = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colAtt.storeOp     = VK_ATTACHMENT_STORE_OP_STORE;
    colAtt.clearValue.color = gClr;

    VkRenderingAttachmentInfo depAtt;
    memset(&depAtt,0,sizeof(depAtt));
    depAtt.sType       = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
    depAtt.imageView   = gDepthView;
    depAtt.imageLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
    depAtt.loadOp      = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depAtt.storeOp     = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depAtt.clearValue.depthStencil = gDep;

    VkRenderingInfo ri;
    memset(&ri,0,sizeof(ri));
    ri.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
    ri.renderArea.extent = gExtent;
    ri.layerCount = 1;
    ri.colorAttachmentCount = 1;
    ri.pColorAttachments    = &colAtt;
    ri.pDepthAttachment     = &depAtt;
    pfnBeginRendering(cb,&ri);

    /* execute ALL secondary CBs */
    std::vector<VkCommandBuffer> secs(NODE_COUNT);
    for(int i=0;i<NODE_COUNT;++i) secs[i]=gNodes[i].secCB[imgIdx];
    vkCmdExecuteCommands(cb,(uint32_t)secs.size(),&secs[0]);

    pfnEndRendering(cb);

    /* colour → PRESENT_SRC */
    VkImageMemoryBarrier toPresent = toClr;
    toPresent.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    toPresent.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    toPresent.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    toPresent.dstAccessMask = 0;

    vkCmdPipelineBarrier(cb,
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
        VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
        0,0,NULL,0,NULL,1,&toPresent);

    return vkEndCommandBuffer(cb);
}

/* ---------------------------------------------------- */
/*                   draw frame                         */
/* ---------------------------------------------------- */
static VkResult DrawFrame()
{
    float angle = SecondsSinceStart();

    VK_CHECK(vkWaitForFences(gDev,1,&gInFlight[gFrame],VK_TRUE,UINT64_MAX));
    VK_CHECK(vkResetFences(gDev,1,&gInFlight[gFrame]));

    uint32_t imgIdx = 0;
    VkResult r = vkAcquireNextImageKHR(gDev,gSwap,UINT64_MAX,
                                       gImgAvail[gFrame],VK_NULL_HANDLE,&imgIdx);
    if(r==VK_ERROR_OUT_OF_DATE_KHR || r==VK_SUBOPTIMAL_KHR) return VK_SUCCESS;
    if(r!=VK_SUCCESS) return r;

    VK_CHECK(RecordPrimary(imgIdx,angle));

    VkPipelineStageFlags waitStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    VkSubmitInfo si;
    memset(&si,0,sizeof(si));
    si.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    si.waitSemaphoreCount = 1;
    si.pWaitSemaphores    = &gImgAvail[gFrame];
    si.pWaitDstStageMask  = &waitStage;
    si.commandBufferCount = 1;
    si.pCommandBuffers    = &gPrimary[imgIdx];
    si.signalSemaphoreCount = 1;
    si.pSignalSemaphores    = &gRenderDone[gFrame];
    VK_CHECK(vkQueueSubmit(gGfxQ,1,&si,gInFlight[gFrame]));

    VkPresentInfoKHR pi;
    memset(&pi,0,sizeof(pi));
    pi.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    pi.waitSemaphoreCount = 1;
    pi.pWaitSemaphores    = &gRenderDone[gFrame];
    pi.swapchainCount     = 1;
    pi.pSwapchains        = &gSwap;
    pi.pImageIndices      = &imgIdx;
    vkQueuePresentKHR(gGfxQ,&pi);

    gFrame = (gFrame+1)%2;
    return VK_SUCCESS;
}

/* ---------------------------------------------------- */
/*                    initialisation                    */
/* ---------------------------------------------------- */
static bool InitAll()
{
    if(CreateInstance()!=VK_SUCCESS) return false;
    if(CreateSurface ()!=VK_SUCCESS) return false;
    if(PickPhysical  ()!=VK_SUCCESS) return false;
    if(CreateDevice  ()!=VK_SUCCESS) return false;
    if(CreateSwap    ()!=VK_SUCCESS) return false;
    if(CreateDepth   ()!=VK_SUCCESS) return false;
    if(CreateCmdAndSync()!=VK_SUCCESS) return false;
    if(CreateVB()   !=VK_SUCCESS) return false;
    if(CreatePipeline()!=VK_SUCCESS) return false;
    gStartClock = clock();
    return true;
}

/* ---------------------------------------------------- */
/*                       cleanup                        */
/* ---------------------------------------------------- */
static void Cleanup()
{
    if(!gDev) return;
    vkDeviceWaitIdle(gDev);

    for(int i=0;i<2;++i){
        if(gImgAvail [i]) vkDestroySemaphore(gDev,gImgAvail [i],NULL);
        if(gRenderDone[i]) vkDestroySemaphore(gDev,gRenderDone[i],NULL);
        if(gInFlight  [i]) vkDestroyFence    (gDev,gInFlight  [i],NULL);
    }
    if(gPipeline)   vkDestroyPipeline      (gDev,gPipeline,NULL);
    if(gPipeLayout) vkDestroyPipelineLayout(gDev,gPipeLayout,NULL);

    for(int n=0;n<NODE_COUNT;++n){
        if(gNodes[n].vbo)    vkDestroyBuffer(gDev,gNodes[n].vbo,NULL);
        if(gNodes[n].vboMem) vkFreeMemory    (gDev,gNodes[n].vboMem,NULL);
    }

    if(gCmdPool)   vkDestroyCommandPool(gDev,gCmdPool,NULL);
    if(gDepthView) vkDestroyImageView  (gDev,gDepthView,NULL);
    if(gDepthImg)  vkDestroyImage      (gDev,gDepthImg ,NULL);
    if(gDepthMem)  vkFreeMemory        (gDev,gDepthMem ,NULL);

    for(size_t i=0;i<gViews.size();++i)
        vkDestroyImageView(gDev,gViews[i],NULL);
    if(gSwap) vkDestroySwapchainKHR(gDev,gSwap,NULL);
    if(gDev)  vkDestroyDevice(gDev,NULL);
    if(gSurface) vkDestroySurfaceKHR(gInst,gSurface,NULL);
    if(gInst)    vkDestroyInstance  (gInst,NULL);
    if(gHWND)    DestroyWindow      (gHWND);
    if(gLOG)     fclose             (gLOG);
}

/* ---------------------------------------------------- */
/*                   WinMain & WndProc                  */
/* ---------------------------------------------------- */
int WINAPI WinMain(HINSTANCE hi,HINSTANCE,LPSTR,int)
{
    gLOG = fopen("log.txt","w");

    WNDCLASSEX wc;
    memset(&wc,0,sizeof(wc));
    wc.cbSize        = sizeof(wc);
    wc.hInstance     = hi;
    wc.lpfnWndProc   = WndProc;
    wc.lpszClassName = APP_NAME;
    wc.style         = CS_HREDRAW|CS_VREDRAW|CS_OWNDC;
    RegisterClassEx(&wc);

    gHWND = CreateWindowEx(WS_EX_APPWINDOW,
                           APP_NAME,APP_NAME,
                           WS_OVERLAPPEDWINDOW|WS_VISIBLE,
                           CW_USEDEFAULT,CW_USEDEFAULT,
                           WIN_W,WIN_H,
                           NULL,NULL,hi,NULL);

    if(!InitAll()){ ERR("init failed"); Cleanup(); return 0; }

    gRun = TRUE;
    MSG msg;
    while(gRun)
    {
        while(PeekMessage(&msg,NULL,0,0,PM_REMOVE))
        {
            if(msg.message==WM_QUIT){ gRun=FALSE; }
            TranslateMessage(&msg); DispatchMessage(&msg);
        }
        if(gRun) DrawFrame();
    }
    Cleanup();
    return 0;
}
static LRESULT CALLBACK WndProc(HWND h,UINT m,WPARAM w,LPARAM l)
{
    switch(m)
    {
        case WM_DESTROY: PostQuitMessage(0); return 0;
        case WM_KEYDOWN:
            if(w==VK_ESCAPE) DestroyWindow(h);
            return 0;
    }
    return DefWindowProc(h,m,w,l);
}
