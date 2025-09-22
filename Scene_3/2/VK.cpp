
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"



#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan.h>


#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE   
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"

#include "VK.h"
#include "Sphere.h"

#pragma comment(lib, "user32.lib")
#pragma comment(lib, "gdi32.lib")

#pragma comment(lib, "vulkan-1.lib")
#pragma comment(lib, "Sphere.lib")



#define WIN_WIDTH  800
#define WIN_HEIGHT 600
#define PLANET_SPEED_BREAKER 2.0f


LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

void Update(void);

const char* gpszAppName = "ARTR";


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



uint32_t enabledInstanceExtensionCount = 0;



const char* enabledInstanceExtensionNames_array[3];


VkInstance vkInstance = VK_NULL_HANDLE;


VkSurfaceKHR vkSurfaceKHR = VK_NULL_HANDLE;


VkPhysicalDevice vkPhysicalDevice_selected = VK_NULL_HANDLE;
uint32_t graphicsQueueFamilyIndex_selected = UINT32_MAX;
VkPhysicalDeviceMemoryProperties vkPhysicalDeviceMemoryProperties;


VkPhysicalDevice *vkPhysicalDevice_array = NULL; 
uint32_t physicalDeviceCount = 0;


uint32_t enabledDeviceExtensionCount = 0;

const char* enabledDeviceExtensionNames_array[1];


VkDevice vkDevice = VK_NULL_HANDLE;


VkQueue vkQueue = VK_NULL_HANDLE;


VkFormat vkFormat_color = VK_FORMAT_UNDEFINED;
VkColorSpaceKHR vkColorSpaceKHR = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;


VkPresentModeKHR vkPresentModeKHR = VK_PRESENT_MODE_FIFO_KHR;


int winWidth = WIN_WIDTH;
int winHeight = WIN_HEIGHT;
VkSwapchainKHR vkSwapchainKHR = VK_NULL_HANDLE;
VkExtent2D vkExtent2D_swapchain;



uint32_t swapchainImageCount = UINT32_MAX;
VkImage* swapchainImage_array = NULL;
VkImageView* swapchainImageView_array = NULL;


VkFormat vkFormat_depth = VK_FORMAT_UNDEFINED;
VkImage vkImage_depth = VK_NULL_HANDLE;
VkDeviceMemory vkDeviceMemory_depth = VK_NULL_HANDLE;
VkImageView vkImageView_depth = VK_NULL_HANDLE;



VkCommandPool vkCommandPool = VK_NULL_HANDLE;


VkCommandBuffer* vkCommandBuffer_array = NULL;


VkRenderPass vkRenderPass = VK_NULL_HANDLE;


VkFramebuffer *vkFrameBuffer_array = NULL;


VkSemaphore vkSemaphore_backBuffer = VK_NULL_HANDLE;
VkSemaphore vkSemaphore_renderComplete = VK_NULL_HANDLE;


VkFence *vkFence_array = NULL;



VkClearColorValue vkClearColorValue;  
VkClearDepthStencilValue vkClearDepthStencilValue;  


VkBool32 bInitialized = FALSE;
uint32_t currentImageIndex = UINT32_MAX;


BOOL bValidation = TRUE;
uint32_t enabledValidationLayerCount = 0;
const char* enabledValidationLayerNames_array[1]; 
VkDebugReportCallbackEXT vkDebugReportCallbackEXT = VK_NULL_HANDLE;
PFN_vkDestroyDebugReportCallbackEXT vkDestroyDebugReportCallbackEXT_fnptr = NULL;



unsigned int numVertices;
unsigned int numElements;

float sphere_vertices[1146];
float sphere_normals[1146];
float sphere_textures[764];
unsigned short sphere_elements[2280];

typedef struct 
{
    VkBuffer vkBuffer;
    VkDeviceMemory vkDeviceMemory;
}VertexData;


















struct MyUniformData
{
    glm::mat4 modelMatrix;
    glm::mat4 viewMatrix;
    glm::mat4 projectionMatrix;
};

struct UniformData
{
    VkBuffer vkBuffer;
    VkDeviceMemory vkDeviceMemory;
};
enum Planet {
    PLANET_SUN = 0,
    PLANET_MERCURY,
    PLANET_VENUS,
    PLANET_EARTH,
    PLANET_MARS,
    PLANET_JUPITOR,
    PLANET_SATURN,
    PLANET_URANUS,
    PLANET_NEPTUNE,
    PLANET_COUNT
};

VertexData vertexData_position[PLANET_COUNT];
VertexData vertexData_texcoord[PLANET_COUNT];
VertexData vertexData_normal[PLANET_COUNT];
VertexData vertexData_index[PLANET_COUNT];

UniformData uniformData[PLANET_COUNT];

VkDescriptorSet vkDescriptorSet_planet[PLANET_COUNT] = { VK_NULL_HANDLE, VK_NULL_HANDLE, VK_NULL_HANDLE, VK_NULL_HANDLE, VK_NULL_HANDLE, VK_NULL_HANDLE, VK_NULL_HANDLE, VK_NULL_HANDLE, VK_NULL_HANDLE };

VkImageView vkImageView_texture_planet[PLANET_COUNT] = { VK_NULL_HANDLE, VK_NULL_HANDLE, VK_NULL_HANDLE, VK_NULL_HANDLE, VK_NULL_HANDLE, VK_NULL_HANDLE, VK_NULL_HANDLE, VK_NULL_HANDLE, VK_NULL_HANDLE };
VkSampler vkSampler_texture_planet[PLANET_COUNT] = { VK_NULL_HANDLE, VK_NULL_HANDLE, VK_NULL_HANDLE, VK_NULL_HANDLE, VK_NULL_HANDLE, VK_NULL_HANDLE, VK_NULL_HANDLE, VK_NULL_HANDLE, VK_NULL_HANDLE };

float angle_planet[PLANET_COUNT] = { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f };

#define vertexData_position_sun vertexData_position[PLANET_SUN]
#define vertexData_position_mercury vertexData_position[PLANET_MERCURY]
#define vertexData_position_venus vertexData_position[PLANET_VENUS]
#define vertexData_position_earth vertexData_position[PLANET_EARTH]
#define vertexData_position_mars vertexData_position[PLANET_MARS]
#define vertexData_position_jupitor vertexData_position[PLANET_JUPITOR]
#define vertexData_position_saturn vertexData_position[PLANET_SATURN]
#define vertexData_position_uranus vertexData_position[PLANET_URANUS]
#define vertexData_position_neptune vertexData_position[PLANET_NEPTUNE]
#define vertexData_texcoord_sun vertexData_texcoord[PLANET_SUN]
#define vertexData_texcoord_mercury vertexData_texcoord[PLANET_MERCURY]
#define vertexData_texcoord_venus vertexData_texcoord[PLANET_VENUS]
#define vertexData_texcoord_earth vertexData_texcoord[PLANET_EARTH]
#define vertexData_texcoord_mars vertexData_texcoord[PLANET_MARS]
#define vertexData_texcoord_jupitor vertexData_texcoord[PLANET_JUPITOR]
#define vertexData_texcoord_saturn vertexData_texcoord[PLANET_SATURN]
#define vertexData_texcoord_uranus vertexData_texcoord[PLANET_URANUS]
#define vertexData_texcoord_neptune vertexData_texcoord[PLANET_NEPTUNE]
#define vertexData_normal_sun vertexData_normal[PLANET_SUN]
#define vertexData_normal_mercury vertexData_normal[PLANET_MERCURY]
#define vertexData_normal_venus vertexData_normal[PLANET_VENUS]
#define vertexData_normal_earth vertexData_normal[PLANET_EARTH]
#define vertexData_normal_mars vertexData_normal[PLANET_MARS]
#define vertexData_normal_jupitor vertexData_normal[PLANET_JUPITOR]
#define vertexData_normal_saturn vertexData_normal[PLANET_SATURN]
#define vertexData_normal_uranus vertexData_normal[PLANET_URANUS]
#define vertexData_normal_neptune vertexData_normal[PLANET_NEPTUNE]
#define vertexData_index_sun vertexData_index[PLANET_SUN]
#define vertexData_index_mercury vertexData_index[PLANET_MERCURY]
#define vertexData_index_venus vertexData_index[PLANET_VENUS]
#define vertexData_index_earth vertexData_index[PLANET_EARTH]
#define vertexData_index_mars vertexData_index[PLANET_MARS]
#define vertexData_index_jupitor vertexData_index[PLANET_JUPITOR]
#define vertexData_index_saturn vertexData_index[PLANET_SATURN]
#define vertexData_index_uranus vertexData_index[PLANET_URANUS]
#define vertexData_index_neptune vertexData_index[PLANET_NEPTUNE]

#define uniformData_sun uniformData[PLANET_SUN]
#define uniformData_mercury uniformData[PLANET_MERCURY]
#define uniformData_venus uniformData[PLANET_VENUS]
#define uniformData_earth uniformData[PLANET_EARTH]
#define uniformData_mars uniformData[PLANET_MARS]
#define uniformData_jupitor uniformData[PLANET_JUPITOR]
#define uniformData_saturn uniformData[PLANET_SATURN]
#define uniformData_uranus uniformData[PLANET_URANUS]
#define uniformData_neptune uniformData[PLANET_NEPTUNE]

#define vkDescriptorSet_sun vkDescriptorSet_planet[PLANET_SUN]
#define vkDescriptorSet_mercury vkDescriptorSet_planet[PLANET_MERCURY]
#define vkDescriptorSet_venus vkDescriptorSet_planet[PLANET_VENUS]
#define vkDescriptorSet_earth vkDescriptorSet_planet[PLANET_EARTH]
#define vkDescriptorSet_mars vkDescriptorSet_planet[PLANET_MARS]
#define vkDescriptorSet_jupitor vkDescriptorSet_planet[PLANET_JUPITOR]
#define vkDescriptorSet_saturn vkDescriptorSet_planet[PLANET_SATURN]
#define vkDescriptorSet_uranus vkDescriptorSet_planet[PLANET_URANUS]
#define vkDescriptorSet_neptune vkDescriptorSet_planet[PLANET_NEPTUNE]

#define vkImageView_texture_sun vkImageView_texture_planet[PLANET_SUN]
#define vkImageView_texture_mercury vkImageView_texture_planet[PLANET_MERCURY]
#define vkImageView_texture_venus vkImageView_texture_planet[PLANET_VENUS]
#define vkImageView_texture_earth vkImageView_texture_planet[PLANET_EARTH]
#define vkImageView_texture_mars vkImageView_texture_planet[PLANET_MARS]
#define vkImageView_texture_jupitor vkImageView_texture_planet[PLANET_JUPITOR]
#define vkImageView_texture_saturn vkImageView_texture_planet[PLANET_SATURN]
#define vkImageView_texture_uranus vkImageView_texture_planet[PLANET_URANUS]
#define vkImageView_texture_neptune vkImageView_texture_planet[PLANET_NEPTUNE]
#define vkSampler_texture_sun vkSampler_texture_planet[PLANET_SUN]
#define vkSampler_texture_mercury vkSampler_texture_planet[PLANET_MERCURY]
#define vkSampler_texture_venus vkSampler_texture_planet[PLANET_VENUS]
#define vkSampler_texture_earth vkSampler_texture_planet[PLANET_EARTH]
#define vkSampler_texture_mars vkSampler_texture_planet[PLANET_MARS]
#define vkSampler_texture_jupitor vkSampler_texture_planet[PLANET_JUPITOR]
#define vkSampler_texture_saturn vkSampler_texture_planet[PLANET_SATURN]
#define vkSampler_texture_uranus vkSampler_texture_planet[PLANET_URANUS]
#define vkSampler_texture_neptune vkSampler_texture_planet[PLANET_NEPTUNE]

#define angle angle_planet[PLANET_SUN]
#define angle_mercury angle_planet[PLANET_MERCURY]
#define angle_venus angle_planet[PLANET_VENUS]
#define angle_earth angle_planet[PLANET_EARTH]
#define angle_mars angle_planet[PLANET_MARS]
#define angle_jupitor angle_planet[PLANET_JUPITOR]
#define angle_saturn angle_planet[PLANET_SATURN]
#define angle_uranus angle_planet[PLANET_URANUS]
#define angle_neptune angle_planet[PLANET_NEPTUNE]






VkShaderModule vkShaderModule_vertex_shader = VK_NULL_HANDLE;
VkShaderModule vkShaderModule_fragment_shader = VK_NULL_HANDLE;


VkDescriptorSetLayout vkDescriptorSetLayout = VK_NULL_HANDLE;


VkPipelineLayout vkPipelineLayout = VK_NULL_HANDLE;


VkDescriptorPool vkDescriptorPool = VK_NULL_HANDLE;






VkViewport vkViewport;
VkRect2D vkRect2D_scissor; 
VkPipeline vkPipeline = VK_NULL_HANDLE;



VkImage vkImage_texture = VK_NULL_HANDLE;
VkDeviceMemory vkDeviceMemory_texture = VK_NULL_HANDLE;

VkImageView vkImageView_texture = VK_NULL_HANDLE;



VkSampler vkSampler_texture = VK_NULL_HANDLE;

// Per-planet texture image + memory (track ALL textures, not just the last one)
VkImage        vkImage_texture_planet[PLANET_COUNT]        = {
    VK_NULL_HANDLE, VK_NULL_HANDLE, VK_NULL_HANDLE, VK_NULL_HANDLE, VK_NULL_HANDLE,
    VK_NULL_HANDLE, VK_NULL_HANDLE, VK_NULL_HANDLE, VK_NULL_HANDLE
};
VkDeviceMemory vkDeviceMemory_texture_planet[PLANET_COUNT] = {
    VK_NULL_HANDLE, VK_NULL_HANDLE, VK_NULL_HANDLE, VK_NULL_HANDLE, VK_NULL_HANDLE,
    VK_NULL_HANDLE, VK_NULL_HANDLE, VK_NULL_HANDLE, VK_NULL_HANDLE
};

static int gRevealStage = PLANET_MERCURY;  // who is currently moving
static const float kTargetAngle = 90.0f;   // "aligned to the right of the Sun"
static const float kRevealDPS   = 0.8f;    // degrees-per-frame for the staged move (tune)

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdLine, int iCmdShow)
{
	
	VkResult Initialize(void);
	void Uninitialize(void);
	VkResult Display(void);

	
	WNDCLASSEX wndclass;
	HWND hwnd;
	MSG msg;
	TCHAR szAppName[255];
	int bDone = 0;
    VkResult vkResult = VK_SUCCESS;


	
    gpFile = fopen("SSA_Log.txt", "w+");
	if (gpFile == NULL)
	{
		MessageBox(NULL, TEXT("Cannot create/open SSA_Log.txt file"), TEXT("FILE IO ERROR"), MB_ICONERROR);
		exit(0);
	}
	else
	{
		fprintf(gpFile, "WinMain() --> Program started successfully\n");
        fflush(gpFile);
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
		NULL, 
		NULL, 
		hInstance,
		NULL);

	ghwnd = hwnd;

	vkResult = Initialize();
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "WinMain() --> Initialize() is failed\n");
        fflush(gpFile);
        DestroyWindow(hwnd);
        hwnd = NULL;
    }
    else
    {
        fprintf(gpFile, "WinMain() --> Initialize() is succedded\n");
        fflush(gpFile);
    }
    

	ShowWindow(hwnd, iCmdShow);
    Update();
	SetForegroundWindow(hwnd);
	SetFocus(hwnd);

	
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
				
                Update();
				
                if(gbWindowMinimized == FALSE)
                {
                    vkResult = Display();
                    if((vkResult != VK_FALSE) && (vkResult != VK_SUCCESS) && (vkResult != VK_ERROR_OUT_OF_DATE_KHR) && (vkResult != VK_SUBOPTIMAL_KHR))
                    {
                        fprintf(gpFile, "WinMain() --> call to Display() failed\n");
                        fflush(gpFile);
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
	
	MONITORINFO mi = { sizeof(MONITORINFO) };

	
	if (gbFullscreen == 0) 
	{
		dwStyle = GetWindowLong(ghwnd, GWL_STYLE); 
		if (dwStyle & WS_OVERLAPPEDWINDOW)
		{
			if (GetWindowPlacement(ghwnd, &wpPrev) && GetMonitorInfo(MonitorFromWindow(ghwnd, MONITORINFOF_PRIMARY), &mi))
			{
				SetWindowLong(ghwnd, GWL_STYLE, (dwStyle & ~WS_OVERLAPPEDWINDOW)); 
				SetWindowPos(ghwnd,
					HWND_TOP, 
					mi.rcMonitor.left, 
					mi.rcMonitor.top, 
					mi.rcMonitor.right - mi.rcMonitor.left, 
					mi.rcMonitor.bottom - mi.rcMonitor.top, 
					SWP_NOZORDER | 
					SWP_FRAMECHANGED); 
			}
		}
		ShowCursor(FALSE);  
		gbFullscreen = 1;
	}
	else
	{
		SetWindowLong(ghwnd, GWL_STYLE, (dwStyle | WS_OVERLAPPEDWINDOW));
		SetWindowPlacement(ghwnd, &wpPrev); 
		SetWindowPos(ghwnd,
			HWND_TOP,
			0,  
			0,  
			0,  
			0,  
			SWP_NOMOVE | 
			SWP_NOSIZE | 
			SWP_NOOWNERZORDER | 
			SWP_NOZORDER | 
			SWP_FRAMECHANGED); 
		ShowCursor(TRUE); 
		gbFullscreen = 0;
	}
}


VkResult Initialize(void)
{
    
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
	VkResult createIndexBuffer(void);
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
        
    
    VkResult vkResult = VK_SUCCESS;

    
    vkResult = createVulkanInstance();
    
    
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "Initialize() --> createVulkanInstance() is failed %d\n", vkResult);
        fflush(gpFile);
        return vkResult;
    }
    else
    {
        fprintf(gpFile, "Initialize() --> createVulkanInstance() is succedded\n");
        fflush(gpFile);
    }

    
    vkResult = getSupportedSurface();
    
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "Initialize() --> getSupportedSurface() is failed %d\n", vkResult);
        fflush(gpFile);
        return vkResult;
    }
    else
    {
        fprintf(gpFile, "Initialize() --> getSupportedSurface() is succedded\n");
        fflush(gpFile);
    }

    
    vkResult = getPhysicalDevice();
    
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "Initialize() --> getPhysicalDevice() is failed %d\n", vkResult);
        fflush(gpFile);
        return vkResult;
    }
    else
    {
        fprintf(gpFile, "Initialize() --> getPhysicalDevice() is succedded\n");
        fflush(gpFile);
    }
    
    
    vkResult = printVkInfo();
    
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "Initialize() --> printVkInfo() is failed %d\n", vkResult);
        fflush(gpFile);
        return vkResult;
    }
    else
    {
        fprintf(gpFile, "Initialize() --> printVkInfo() is succedded\n");
        fflush(gpFile);
    }
    
    vkResult = createVulkanDevice();
    
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "Initialize() --> createVulkanDevice() is failed %d\n", vkResult);
        fflush(gpFile);
        return vkResult;
    }
    else
    {
        fprintf(gpFile, "Initialize() --> createVulkanDevice() is succedded\n");
        fflush(gpFile);
    }
    
    
    getDeviceQueue();
    

        
    
    vkResult = createSwapchain(VK_FALSE);
    
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "Initialize() --> createSwapchain() is failed %d\n", vkResult);
        fflush(gpFile);
        return VK_ERROR_INITIALIZATION_FAILED;
    }
    else
    {
        fprintf(gpFile, "Initialize() --> createSwapchain() is succedded\n");
        fflush(gpFile);
    }
    
    
    vkResult = createImagesAndImageViews();
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "Initialize() --> createImagesAndImageViews() is failed %d\n", vkResult);
        fflush(gpFile);
    }
    else
    {
        fprintf(gpFile, "Initialize() --> createImagesAndImageViews() is succedded\n");
        fflush(gpFile);
    }
    
    
    vkResult = createCommandPool();
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "Initialize() --> createCommandPool() is failed %d\n", vkResult);
        fflush(gpFile);
    }
    else
    {
        fprintf(gpFile, "Initialize() --> createCommandPool() is succedded\n");
        fflush(gpFile);
    }
    
    
    vkResult = createCommandBuffers();
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "Initialize() --> createCommandBuffers() is failed %d\n", vkResult);
        fflush(gpFile);
    }
    else
    {
        fprintf(gpFile, "Initialize() --> createCommandBuffers() is succedded\n");
        fflush(gpFile);
    }
    
    getSphereVertexData(sphere_vertices, sphere_normals, sphere_textures, sphere_elements);
    numVertices = getNumberOfSphereVertices();
    numElements = getNumberOfSphereElements();
    
    
    vkResult = createVertexBuffer();
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "Initialize() --> createVertexBuffer() is failed %d\n", vkResult);
        fflush(gpFile);
    }
    else
    {
        fprintf(gpFile, "Initialize() --> createVertexBuffer() is succedded\n");
        fflush(gpFile);
    }
    
	
    vkResult = createIndexBuffer();
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "Initialize() --> createIndexBuffer() is failed %d\n", vkResult);
        fflush(gpFile);
    }
    else
    {
        fprintf(gpFile, "Initialize() --> createIndexBuffer() is succedded\n");
        fflush(gpFile);
    }
	
    
    vkResult = createTexture("sun.png");
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "Initialize() --> createTexture() is failed for sun texture %d\n", vkResult);
        fflush(gpFile);
    }
    else
    {
        fprintf(gpFile, "Initialize() --> createTexture() is succedded for sun texture\n");
        fflush(gpFile);
    }
    
    vkResult = createTexture("mercury.png");
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "Initialize() --> createTexture() is failed for mercury texture %d\n", vkResult);
        fflush(gpFile);
    }
    else
    {
        fprintf(gpFile, "Initialize() --> createTexture() is succedded for mercury texture\n");
        fflush(gpFile);
    }
    
    vkResult = createTexture("venus.png");
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "Initialize() --> createTexture() is failed for venus texture %d\n", vkResult);
        fflush(gpFile);
    }
    else
    {
        fprintf(gpFile, "Initialize() --> createTexture() is succedded for venus texture\n");
        fflush(gpFile);
    }
    
    vkResult = createTexture("earth.png");
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "Initialize() --> createTexture() is failed for earth texture %d\n", vkResult);
        fflush(gpFile);
    }
    else
    {
        fprintf(gpFile, "Initialize() --> createTexture() is succedded for earth texture\n");
        fflush(gpFile);
    }
    
    vkResult = createTexture("mars.png");
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "Initialize() --> createTexture() is failed for mars texture %d\n", vkResult);
        fflush(gpFile);
    }
    else
    {
        fprintf(gpFile, "Initialize() --> createTexture() is succedded for mars texture\n");
        fflush(gpFile);
    }
    
    vkResult = createTexture("jupitor.png");
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "Initialize() --> createTexture() is failed for jupitor texture %d\n", vkResult);
        fflush(gpFile);
    }
    else
    {
        fprintf(gpFile, "Initialize() --> createTexture() is succedded for jupitor texture\n");
        fflush(gpFile);  
    }
    
    vkResult = createTexture("saturn.png");
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "Initialize() --> createTexture() is failed for saturn texture %d\n", vkResult);
        fflush(gpFile);
    }
    else
    {
        fprintf(gpFile, "Initialize() --> createTexture() is succedded for saturn texture\n");
        fflush(gpFile);
    }
    
    vkResult = createTexture("uranus.png");
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "Initialize() --> createTexture() is failed for uranus texture %d\n", vkResult);
        fflush(gpFile);
    }
    else
    {
        fprintf(gpFile, "Initialize() --> createTexture() is succedded for uranus texture\n");
        fflush(gpFile);
    }
    
    vkResult = createTexture("neptune.png");
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "Initialize() --> createTexture() is failed for neptune texture %d\n", vkResult);
        fflush(gpFile);
    }
    else
    {
        fprintf(gpFile, "Initialize() --> createTexture() is succedded for neptune texture\n");
        fflush(gpFile);
    }
    
    
    vkResult = createUniformBuffer();
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "Initialize() --> createUniformBuffer() is failed %d\n", vkResult);
        fflush(gpFile);
    }
    else
    {
        fprintf(gpFile, "Initialize() --> createUniformBuffer() is succedded\n");
        fflush(gpFile);
    }
    
    
    vkResult = createShaders();
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "Initialize() --> createShaders() is failed %d\n", vkResult);
        fflush(gpFile);
    }
    else
    {
        fprintf(gpFile, "Initialize() --> createShaders() is succedded\n");
        fflush(gpFile);
    }
    
    
    
    
    vkResult = createDescriptorSetLayout();
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "Initialize() --> createDescriptorSetLayout() is failed %d\n", vkResult);
        fflush(gpFile);
    }
    else
    {
        fprintf(gpFile, "Initialize() --> createDescriptorSetLayout() is succedded\n");
        fflush(gpFile);
    }
    
    
    vkResult = createPipelineLayout();
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "Initialize() --> createPipelineLayout() is failed %d\n", vkResult);
        fflush(gpFile);
    }
    else
    {
        fprintf(gpFile, "Initialize() --> createPipelineLayout() is succedded\n");
        fflush(gpFile);
    }
    
    
    vkResult = createDescriptorPool();
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "Initialize() --> createDescriptorPool() is failed %d\n", vkResult);
        fflush(gpFile);
    }
    else
    {
        fprintf(gpFile, "Initialize() --> createDescriptorPool() is succedded\n");
        fflush(gpFile);
    }
    
    
    vkResult = createDescriptorSet();
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "Initialize() --> createDescriptorSet() is failed %d\n", vkResult);
        fflush(gpFile);
    }
    else
    {
        fprintf(gpFile, "Initialize() --> createDescriptorPool() is succedded\n");
        fflush(gpFile);
    }
    
    
    vkResult = createRenderPass();
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "Initialize() --> createRenderPass() is failed %d\n", vkResult);
        fflush(gpFile);
    }
    else
    {
        fprintf(gpFile, "Initialize() --> createRenderPass() is succedded\n");
        fflush(gpFile);
    }
    
    
    vkResult = createPipeline();
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "Initialize() --> createPipeline() is failed %d\n", vkResult);
        fflush(gpFile);
    }
    else
    {
        fprintf(gpFile, "Initialize() --> createPipeline() is succedded\n");
        fflush(gpFile);
    }
    
    
    
    vkResult = createFrameBuffers();
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "Initialize() --> createFrameBuffers() is failed %d\n", vkResult);
        fflush(gpFile);
    }
    else
    {
        fprintf(gpFile, "Initialize() --> createFrameBuffers() is succedded\n");
        fflush(gpFile);
    }
    
    
    vkResult = createSemaphores();
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "Initialize() --> createSemaphores() is failed %d\n", vkResult);
        fflush(gpFile);
    }
    else
    {
        fprintf(gpFile, "Initialize() --> createSemaphores() is succedded\n");
        fflush(gpFile);
    }
    
    
    vkResult = createFences();
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "Initialize() --> createFences() is failed %d\n", vkResult);
        fflush(gpFile);
    }
    else
    {
        fprintf(gpFile, "Initialize() --> createFences() is succedded\n");
        fflush(gpFile);
    }
    
    
    memset((void*)&vkClearColorValue, 0, sizeof(VkClearColorValue));
    vkClearColorValue.float32[0] = 0.0f;
    vkClearColorValue.float32[1] = 0.0f;
    vkClearColorValue.float32[2] = 0.0f;
    vkClearColorValue.float32[3] = 1.0f;
    
    memset((void*)&vkClearDepthStencilValue, 0, sizeof(VkClearDepthStencilValue));
    
    vkClearDepthStencilValue.depth = 1.0f; 
    
    vkClearDepthStencilValue.stencil = 0; 
    
    
    vkResult = buildCommandBuffers();
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "Initialize() --> buildCommandBuffers() is failed %d\n", vkResult);
        fflush(gpFile);
    }
    else
    {
        fprintf(gpFile, "Initialize() --> buildCommandBuffers() is succedded\n");
        fflush(gpFile);
    }
    
    
    bInitialized = TRUE;
    fprintf(gpFile, "Initialize() --> Initialization is completed successfully\n");
    fflush(gpFile);
  
    return vkResult;
}


VkResult Resize(int width, int height)
{
    
    VkResult createSwapchain(VkBool32);
    VkResult createImagesAndImageViews(void);
    VkResult createCommandBuffers(void);
    VkResult createPipelineLayout(void);
    VkResult createPipeline(void);
    VkResult createRenderPass(void);
    VkResult createFrameBuffers(void);
    VkResult buildCommandBuffers(void);
    
    
    
    VkResult vkResult = VK_SUCCESS;
	
	if (height == 0)
		height = 1;
    
    giHeight = height;
    giWidth = width;
    

    
    if(bInitialized == FALSE)
    {
        fprintf(gpFile, "Resize() --> Initialization yet not completed or failed\n");
        fflush(gpFile);
        vkResult = VK_ERROR_INITIALIZATION_FAILED;
        return vkResult;
    }
    
    
    bInitialized = FALSE;

    
    winWidth = width;
    winHeight = height;
    
    
    vkDeviceWaitIdle(vkDevice);
    fprintf(gpFile, "Resize() --> vkDeviceWaitIdle() is done\n");
        fflush(gpFile);
    
    
    if(vkSwapchainKHR == VK_NULL_HANDLE)
    {
        fprintf(gpFile, "Resize() --> swapchain is already NULL cannot proceed\n");
        fflush(gpFile);
        vkResult = VK_ERROR_INITIALIZATION_FAILED;
    }
    
    
    for(uint32_t i = 0; i < swapchainImageCount ; i++)
    {
        vkDestroyFramebuffer(vkDevice, vkFrameBuffer_array[i], NULL);
    }
    if(vkFrameBuffer_array)
    {
        free(vkFrameBuffer_array);
        vkFrameBuffer_array = NULL;
        fprintf(gpFile, "Resize() --> vkFrameBuffer_array() is done\n");
        fflush(gpFile);
    }
    
    
    for(uint32_t i = 0; i < swapchainImageCount; i++)
    {
        vkFreeCommandBuffers(vkDevice, vkCommandPool, 1, &vkCommandBuffer_array[i]);
        fprintf(gpFile, "Resize() --> vkFreeCommandBuffers() is done\n");
        fflush(gpFile);
    }
    
    if(vkCommandBuffer_array)
    {
        free(vkCommandBuffer_array);
        vkCommandBuffer_array = NULL;
        fprintf(gpFile, "Resize() --> vkCommandBuffer_array is freed\n");
        fflush(gpFile);
    }
    
    
    
    if(vkPipeline)
    {
        vkDestroyPipeline(vkDevice, vkPipeline, NULL);
        vkPipeline = VK_NULL_HANDLE;
        fprintf(gpFile, "Resize() --> vkDestroyPipeline() is done\n");
        fflush(gpFile);
    }
    
    
    if(vkPipelineLayout)
    {
        vkDestroyPipelineLayout(vkDevice, vkPipelineLayout, NULL);
        vkPipelineLayout = VK_NULL_HANDLE;
        fprintf(gpFile, "Resize() --> vkPipelineLayout() is done\n");
        fflush(gpFile);
    }
    
    
    if(vkRenderPass)
    {
        vkDestroyRenderPass(vkDevice, vkRenderPass, NULL);
        vkRenderPass = VK_NULL_HANDLE;
    }
    
    
    if(vkImageView_depth)
    {
        vkDestroyImageView(vkDevice, vkImageView_depth, NULL);
        vkImageView_depth = VK_NULL_HANDLE;
    }
    
    
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
    
    for(uint32_t i = 0; i < swapchainImageCount; i++)
    {
        vkDestroyImageView(vkDevice, swapchainImageView_array[i], NULL);
        fprintf(gpFile, "Resize() --> vkDestoryImageView() is done\n");
        fflush(gpFile);
    }
    
    if(swapchainImageView_array)
    {
        free(swapchainImageView_array);
        swapchainImageView_array = NULL;
        fprintf(gpFile, "Resize() --> swapchainImageView_array is freed\n");
        fflush(gpFile);
    }
    
    
    
    
        
        
    
    
    
    if(swapchainImage_array)
    {
        free(swapchainImage_array);
        swapchainImage_array = NULL;
        fprintf(gpFile, "Resize() --> swapchainImage_array is freed\n");
        fflush(gpFile);
    }
    
    
    if(vkSwapchainKHR)
    {
        vkDestroySwapchainKHR(vkDevice, vkSwapchainKHR, NULL);
        vkSwapchainKHR = VK_NULL_HANDLE;
        fprintf(gpFile, "Resize() --> vkSwapchainCreateInfoKHR() is done\n");
        fflush(gpFile);
    }
    
    
    
    
    vkResult = createSwapchain(VK_FALSE);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "Resize() --> createSwapchain() is failed %d\n", vkResult);
        fflush(gpFile);
        return VK_ERROR_INITIALIZATION_FAILED;
    }
    
    
    vkResult = createImagesAndImageViews();
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "Resize() --> createImagesAndImageViews() is failed %d\n", vkResult);
        fflush(gpFile);
    }
    
    
    vkResult = createRenderPass();
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "Resize() --> createRenderPass() is failed %d\n", vkResult);
        fflush(gpFile);
    }
    
    
    vkResult = createPipelineLayout();
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "Resize() --> createPipelineLayout() is failed %d\n", vkResult);
        fflush(gpFile);
    }
    
    
    vkResult = createPipeline();
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "Resize() --> createPipeline() is failed %d\n", vkResult);
        fflush(gpFile);
    }
      
    
    vkResult = createFrameBuffers();
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "Resize() --> createFrameBuffers() is failed %d\n", vkResult);
        fflush(gpFile);
    }
    
    
    vkResult = createCommandBuffers();
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "Resize() --> createCommandBuffers() is failed %d\n", vkResult);
        fflush(gpFile);
    }
    
    
    vkResult = buildCommandBuffers();
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "Resize() --> buildCommandBuffers() is failed %d\n", vkResult);
        fflush(gpFile);
    }

    bInitialized = TRUE;

    return vkResult;
}


VkResult Display(void)
{
    
    VkResult Resize(int, int);
    VkResult updateUniformBuffer(void);
    
    
    VkResult vkResult = VK_SUCCESS; 

    
    
    if(bInitialized == FALSE)
    {
        fprintf(gpFile, "Display() --> Initialization yet not completed\n");
        fflush(gpFile);
        return (VkResult)VK_FALSE;
    }
    
    
    
    vkResult = vkAcquireNextImageKHR(vkDevice,
                                     vkSwapchainKHR,
                                     UINT64_MAX, 
                                     vkSemaphore_backBuffer, 
                                     VK_NULL_HANDLE, 
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
            fflush(gpFile);
            return vkResult;   
        }
    }
    
    
    vkResult = vkWaitForFences(vkDevice,
                               1, 
                               &vkFence_array[currentImageIndex], 
                               VK_TRUE, 
                               UINT64_MAX); 
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "Display() --> vkWaitForFences() is failed errorcode = %d\n", vkResult);
        fflush(gpFile);
        return vkResult;
    }

    
    vkResult = vkResetFences(vkDevice,
                             1, 
                             &vkFence_array[currentImageIndex]);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "Display() --> vkResetFences() is failed errorcode = %d\n", vkResult);
        fflush(gpFile);
        return vkResult;
    }
    
    
    const VkPipelineStageFlags waitDstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

    
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
    
    
    vkResult = vkQueueSubmit(vkQueue,
                             1,
                             &vkSubmitInfo,
                             vkFence_array[currentImageIndex]);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "Display() --> vkQueueSubmit() is failed errorcode = %d\n", vkResult);
        fflush(gpFile);
        return vkResult;
    }
    
    
    VkPresentInfoKHR vkPresentInfoKHR;
    memset((void*)&vkPresentInfoKHR, 0, sizeof(VkPresentInfoKHR));
    
    vkPresentInfoKHR.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    vkPresentInfoKHR.pNext = NULL;
    vkPresentInfoKHR.swapchainCount = 1;
    vkPresentInfoKHR.pSwapchains = &vkSwapchainKHR;
    vkPresentInfoKHR.pImageIndices = &currentImageIndex;
    vkPresentInfoKHR.waitSemaphoreCount = 1;
    vkPresentInfoKHR.pWaitSemaphores = &vkSemaphore_renderComplete;
    
    
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
            fflush(gpFile);
            return vkResult;
        }
    }
    
    vkResult = updateUniformBuffer();
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "Display() --> updateUniformBuffer() is failed errorcode = %d\n", vkResult);
        fflush(gpFile);
    }
        
    
    vkDeviceWaitIdle(vkDevice);    
    
    return vkResult;
}
    

void Update(void)
{
    // Keep the Sun spinning slowly if you like (purely cosmetic)
    angle += 0.3f; 
    if (angle >= 360.0f) angle -= 360.0f;

    // Clamp safety for any planet that is already placed
    for (int i = PLANET_MERCURY; i < PLANET_COUNT; ++i)
    {
        if (i < gRevealStage)         // already placed
            angle_planet[i] = kTargetAngle;
        else if (i > gRevealStage)    // not yet revealed -> keep hidden behind Sun
            angle_planet[i] = 0.0f;
    }

    // Advance the currently revealing planet toward the right-of-sun pose
    if (gRevealStage >= PLANET_MERCURY && gRevealStage < PLANET_COUNT)
    {
        angle_planet[gRevealStage] += kRevealDPS;      // move along its orbit
        if (angle_planet[gRevealStage] >= kTargetAngle)
        {
            angle_planet[gRevealStage] = kTargetAngle; // snap exactly to 90°
            ++gRevealStage;                             // start the next planet on the next frame
        }
    }
}

void Uninitialize(void)
{
    // Restore window state
    if (gbFullscreen == 1)
    {
        dwStyle = GetWindowLong(ghwnd, GWL_STYLE);
        SetWindowLong(ghwnd, GWL_STYLE, (dwStyle | WS_OVERLAPPEDWINDOW));
        SetWindowPlacement(ghwnd, &wpPrev);
        SetWindowPos(ghwnd,
            HWND_TOP,
            0, 0, 0, 0,
            SWP_NOMOVE |
            SWP_NOSIZE |
            SWP_NOOWNERZORDER |
            SWP_NOZORDER |
            SWP_FRAMECHANGED);
        ShowCursor(TRUE);
    }

    if (ghdc)
    {
        ReleaseDC(ghwnd, ghdc);
        ghdc = NULL;
    }

    // Make sure GPU is idle
    if (vkDevice)
    {
        vkDeviceWaitIdle(vkDevice);
        fprintf(gpFile, "Uninitialize() --> vkDeviceWaitIdle() is done\n"); fflush(gpFile);
    }

    // --- Sync primitives
    if (vkFence_array && swapchainImageCount != UINT32_MAX)
    {
        for (uint32_t i = 0; i < swapchainImageCount; ++i)
        {
            if (vkFence_array[i])
            {
                vkDestroyFence(vkDevice, vkFence_array[i], NULL);
                vkFence_array[i] = VK_NULL_HANDLE;
            }
        }
        free(vkFence_array);
        vkFence_array = NULL;
        fprintf(gpFile, "Uninitialize() --> vkDestroyFence() is done\n"); fflush(gpFile);
    }

    if (vkSemaphore_renderComplete)
    {
        vkDestroySemaphore(vkDevice, vkSemaphore_renderComplete, NULL);
        vkSemaphore_renderComplete = VK_NULL_HANDLE;
        fprintf(gpFile, "Uninitialize() --> vkDestroySemaphore() for vkSemaphore_renderComplete is done\n"); fflush(gpFile);
    }
    if (vkSemaphore_backBuffer)
    {
        vkDestroySemaphore(vkDevice, vkSemaphore_backBuffer, NULL);
        vkSemaphore_backBuffer = VK_NULL_HANDLE;
        fprintf(gpFile, "Uninitialize() --> vkDestroySemaphore() for vkSemaphore_backBuffer is done\n"); fflush(gpFile);
    }

    // --- Descriptor pool & layout
    if (vkDescriptorPool)
    {
        vkDestroyDescriptorPool(vkDevice, vkDescriptorPool, NULL);
        vkDescriptorPool = VK_NULL_HANDLE;
        fprintf(gpFile, "Uninitialize() --> vkDestroyDescriptorPool() is done\n"); fflush(gpFile);
    }
    if (vkDescriptorSetLayout)
    {
        vkDestroyDescriptorSetLayout(vkDevice, vkDescriptorSetLayout, NULL);
        vkDescriptorSetLayout = VK_NULL_HANDLE;
        fprintf(gpFile, "Uninitialize() --> vkDestroyDescriptorSetLayout() is done\n"); fflush(gpFile);
    }

    // --- Uniform buffers (per planet)
    for (int p = 0; p < PLANET_COUNT; ++p)
    {
        if (uniformData[p].vkBuffer)
        {
            vkDestroyBuffer(vkDevice, uniformData[p].vkBuffer, NULL);
            uniformData[p].vkBuffer = VK_NULL_HANDLE;
            fprintf(gpFile, "Uninitialize() --> vkDestroyBuffer() for uniformData[%d]\n", p); fflush(gpFile);
        }
        if (uniformData[p].vkDeviceMemory)
        {
            vkFreeMemory(vkDevice, uniformData[p].vkDeviceMemory, NULL);
            uniformData[p].vkDeviceMemory = VK_NULL_HANDLE;
            fprintf(gpFile, "Uninitialize() --> vkFreeMemory() for uniformData[%d]\n", p); fflush(gpFile);
        }
    }

    // --- TEXTURES (per-planet samplers & image views)
    for (int p = 0; p < PLANET_COUNT; ++p)
    {
        if (vkSampler_texture_planet[p])
        {
            vkDestroySampler(vkDevice, vkSampler_texture_planet[p], NULL);
            vkSampler_texture_planet[p] = VK_NULL_HANDLE;
            fprintf(gpFile, "Uninitialize() --> vkDestroySampler() planet[%d]\n", p); fflush(gpFile);
        }
        if (vkImageView_texture_planet[p])
        {
            vkDestroyImageView(vkDevice, vkImageView_texture_planet[p], NULL);
            vkImageView_texture_planet[p] = VK_NULL_HANDLE;
            fprintf(gpFile, "Uninitialize() --> vkDestroyImageView() planet[%d]\n", p); fflush(gpFile);
        }
    }
    // clear singletons to avoid double-destroy later
    vkSampler_texture = VK_NULL_HANDLE;
    vkImageView_texture = VK_NULL_HANDLE;

    // --- TEXTURES (per-planet images & device memory)
    for (int p = 0; p < PLANET_COUNT; ++p)
    {
        if (vkImage_texture_planet[p])
        {
            vkDestroyImage(vkDevice, vkImage_texture_planet[p], NULL);
            vkImage_texture_planet[p] = VK_NULL_HANDLE;
            fprintf(gpFile, "Uninitialize() --> vkDestroyImage() planet[%d]\n", p); fflush(gpFile);
        }
        if (vkDeviceMemory_texture_planet[p])
        {
            vkFreeMemory(vkDevice, vkDeviceMemory_texture_planet[p], NULL);
            vkDeviceMemory_texture_planet[p] = VK_NULL_HANDLE;
            fprintf(gpFile, "Uninitialize() --> vkFreeMemory() planet[%d]\n", p); fflush(gpFile);
        }
    }
    // IMPORTANT: the singletons reflected the last planet's handles; since we've already destroyed them
    // via the arrays above, set the singletons to NULL and DO NOT free/destroy them again.
    vkImage_texture = VK_NULL_HANDLE;
    vkDeviceMemory_texture = VK_NULL_HANDLE;

    // --- Vertex / index buffers (per planet)
    auto destroyVertexDataArray = [&](VertexData* arr, const char* label)
    {
        for (int p = 0; p < PLANET_COUNT; ++p)
        {
            if (arr[p].vkBuffer)
            {
                vkDestroyBuffer(vkDevice, arr[p].vkBuffer, NULL);
                arr[p].vkBuffer = VK_NULL_HANDLE;
                fprintf(gpFile, "Uninitialize() --> vkDestroyBuffer() for %s[%d]\n", label, p); fflush(gpFile);
            }
            if (arr[p].vkDeviceMemory)
            {
                vkFreeMemory(vkDevice, arr[p].vkDeviceMemory, NULL);
                arr[p].vkDeviceMemory = VK_NULL_HANDLE;
                fprintf(gpFile, "Uninitialize() --> vkFreeMemory() for %s[%d]\n", label, p); fflush(gpFile);
            }
        }
    };
    destroyVertexDataArray(vertexData_position, "vertexData_position");
    destroyVertexDataArray(vertexData_texcoord, "vertexData_texcoord");
    destroyVertexDataArray(vertexData_normal,   "vertexData_normal");
    destroyVertexDataArray(vertexData_index,    "vertexData_index");

    // --- Pipeline objects
    if (vkPipeline)
    {
        vkDestroyPipeline(vkDevice, vkPipeline, NULL);
        vkPipeline = VK_NULL_HANDLE;
        fprintf(gpFile, "Uninitialize() --> vkDestroyPipeline() is done\n"); fflush(gpFile);
    }
    if (vkPipelineLayout)
    {
        vkDestroyPipelineLayout(vkDevice, vkPipelineLayout, NULL);
        vkPipelineLayout = VK_NULL_HANDLE;
        fprintf(gpFile, "Uninitialize() --> vkDestroyPipelineLayout() is done\n"); fflush(gpFile);
    }

    // --- Shader modules
    if (vkShaderModule_fragment_shader)
    {
        vkDestroyShaderModule(vkDevice, vkShaderModule_fragment_shader, NULL);
        vkShaderModule_fragment_shader = VK_NULL_HANDLE;
        fprintf(gpFile, "Uninitialize() --> vkDestroyShaderModule() (fragment) is done\n"); fflush(gpFile);
    }
    if (vkShaderModule_vertex_shader)
    {
        vkDestroyShaderModule(vkDevice, vkShaderModule_vertex_shader, NULL);
        vkShaderModule_vertex_shader = VK_NULL_HANDLE;
        fprintf(gpFile, "Uninitialize() --> vkDestroyShaderModule() (vertex) is done\n"); fflush(gpFile);
    }

    // --- Framebuffers
    if (vkFrameBuffer_array && swapchainImageCount != UINT32_MAX)
    {
        for (uint32_t i = 0; i < swapchainImageCount; ++i)
        {
            if (vkFrameBuffer_array[i])
            {
                vkDestroyFramebuffer(vkDevice, vkFrameBuffer_array[i], NULL);
                vkFrameBuffer_array[i] = VK_NULL_HANDLE;
            }
        }
        free(vkFrameBuffer_array);
        vkFrameBuffer_array = NULL;
        fprintf(gpFile, "Uninitialize() --> vkDestroyFramebuffer() for array is done\n"); fflush(gpFile);
    }

    // --- Render pass
    if (vkRenderPass)
    {
        vkDestroyRenderPass(vkDevice, vkRenderPass, NULL);
        vkRenderPass = VK_NULL_HANDLE;
        fprintf(gpFile, "Uninitialize() --> vkDestroyRenderPass() is done\n"); fflush(gpFile);
    }

    // --- Depth resources
    if (vkImageView_depth)
    {
        vkDestroyImageView(vkDevice, vkImageView_depth, NULL);
        vkImageView_depth = VK_NULL_HANDLE;
        fprintf(gpFile, "Uninitialize() --> vkDestroyImageView() for depth is done\n"); fflush(gpFile);
    }
    if (vkImage_depth)
    {
        vkDestroyImage(vkDevice, vkImage_depth, NULL);
        vkImage_depth = VK_NULL_HANDLE;
        fprintf(gpFile, "Uninitialize() --> vkDestroyImage() for depth is done\n"); fflush(gpFile);
    }
    if (vkDeviceMemory_depth)
    {
        vkFreeMemory(vkDevice, vkDeviceMemory_depth, NULL);
        vkDeviceMemory_depth = VK_NULL_HANDLE;
        fprintf(gpFile, "Uninitialize() --> vkFreeMemory() for depth is done\n"); fflush(gpFile);
    }

    // --- Swapchain image views & arrays
    if (swapchainImageView_array && swapchainImageCount != UINT32_MAX)
    {
        for (uint32_t i = 0; i < swapchainImageCount; ++i)
        {
            if (swapchainImageView_array[i])
            {
                vkDestroyImageView(vkDevice, swapchainImageView_array[i], NULL);
                swapchainImageView_array[i] = VK_NULL_HANDLE;
            }
        }
        free(swapchainImageView_array);
        swapchainImageView_array = NULL;
        fprintf(gpFile, "Uninitialize() --> swapchainImageView_array is freed\n"); fflush(gpFile);
    }
    if (swapchainImage_array)
    {
        free(swapchainImage_array);
        swapchainImage_array = NULL;
        fprintf(gpFile, "Uninitialize() --> swapchainImage_array is freed\n"); fflush(gpFile);
    }

    // --- Swapchain
    if (vkSwapchainKHR)
    {
        vkDestroySwapchainKHR(vkDevice, vkSwapchainKHR, NULL);
        vkSwapchainKHR = VK_NULL_HANDLE;
        fprintf(gpFile, "Uninitialize() --> vkDestroySwapchainKHR() is done\n"); fflush(gpFile);
    }

    // --- Command buffers + pool
    if (vkCommandBuffer_array && swapchainImageCount != UINT32_MAX)
    {
        vkFreeCommandBuffers(vkDevice, vkCommandPool, swapchainImageCount, vkCommandBuffer_array);
        for (uint32_t i = 0; i < swapchainImageCount; ++i)
            vkCommandBuffer_array[i] = VK_NULL_HANDLE;
        free(vkCommandBuffer_array);
        vkCommandBuffer_array = NULL;
        fprintf(gpFile, "Uninitialize() --> vkFreeCommandBuffers() is done\n"); fflush(gpFile);
    }
    if (vkCommandPool)
    {
        vkDestroyCommandPool(vkDevice, vkCommandPool, NULL);
        vkCommandPool = VK_NULL_HANDLE;
        fprintf(gpFile, "Uninitialize() --> vkDestroyCommandPool() is done\n"); fflush(gpFile);
    }

    // --- Device
    if (vkDevice)
    {
        vkDestroyDevice(vkDevice, NULL);
        vkDevice = VK_NULL_HANDLE;
        fprintf(gpFile, "Uninitialize() --> vkDestroyDevice() is done\n"); fflush(gpFile);
    }

    // --- Surface
    if (vkSurfaceKHR)
    {
        vkDestroySurfaceKHR(vkInstance, vkSurfaceKHR, NULL);
        vkSurfaceKHR = VK_NULL_HANDLE;
        fprintf(gpFile, "Uninitialize() --> vkDestroySurfaceKHR() is done\n"); fflush(gpFile);
    }

    // --- Debug callback (EXT)
    if (vkDebugReportCallbackEXT && vkDestroyDebugReportCallbackEXT_fnptr)
    {
        vkDestroyDebugReportCallbackEXT_fnptr(vkInstance, vkDebugReportCallbackEXT, NULL);
        vkDebugReportCallbackEXT = VK_NULL_HANDLE;
        vkDestroyDebugReportCallbackEXT_fnptr = NULL;
        fprintf(gpFile, "Uninitialize() --> vkDestroyDebugReportCallbackEXT() is done\n"); fflush(gpFile);
    }

    // --- Instance
    if (vkInstance)
    {
        vkDestroyInstance(vkInstance, NULL);
        vkInstance = VK_NULL_HANDLE;
        fprintf(gpFile, "Uninitialize() --> vkDestroyInstance() is done\n"); fflush(gpFile);
    }

    if (gpFile)
    {
        fprintf(gpFile, "Uninitialize() --> Program terminated successfully\n");
        fflush(gpFile);
        fclose(gpFile);
        gpFile = NULL;
    }
}

VkResult createVulkanInstance(void)
{
    
    VkResult fillInstanceExtensionNames(void);
    VkResult fillValidationLayerNames(void);
    VkResult createValidationCallbackFunction(void);

    
    VkResult vkResult = VK_SUCCESS;
    
    
    
    vkResult = fillInstanceExtensionNames();
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "createVulkanInstance() --> fillInstanceExtensionNames() is failed\n");
        fflush(gpFile);
    }
    else
    {
        fprintf(gpFile, "createVulkanInstance() --> fillInstanceExtensionNames() is succedded\n");
        fflush(gpFile);
    }

    if(bValidation == TRUE)
    {
        
        vkResult = fillValidationLayerNames();
        if(vkResult != VK_SUCCESS)
        {
            fprintf(gpFile, "createVulkanInstance() --> fillValidationLayerNames() is failed\n");
            fflush(gpFile);
        }
        else
        {
            fprintf(gpFile, "createVulkanInstance() --> fillValidationLayerNames() is succedded\n");
            fflush(gpFile);
        }
    }


    
    VkApplicationInfo vkApplicationInfo;
    memset((void*)&vkApplicationInfo, 0, sizeof(VkApplicationInfo));

    vkApplicationInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO; 
    vkApplicationInfo.pNext = NULL; 
    vkApplicationInfo.pApplicationName = gpszAppName;
    vkApplicationInfo.applicationVersion = 1;
    vkApplicationInfo.pEngineName = gpszAppName;
    vkApplicationInfo.engineVersion = 1;
    vkApplicationInfo.apiVersion = VK_API_VERSION_1_4;


    
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

    
    
    vkResult = vkCreateInstance(&vkInstanceCreateInfo,
                                 NULL,  
                                 &vkInstance);
    if(vkResult == VK_ERROR_INCOMPATIBLE_DRIVER)
    {
        fprintf(gpFile, "createVulkanInstance() --> vkCreateInstance:: vkCreateInstance failed due to incompatible driver %d\n", vkResult);
        fflush(gpFile);
        return vkResult;
    }
    else if(vkResult == VK_ERROR_EXTENSION_NOT_PRESENT)
    {
        fprintf(gpFile, "createVulkanInstance() --> vkCreateInstance:: vkCreateInstance failed due to required extension not present %d\n", vkResult);
        fflush(gpFile);
        return vkResult;
    }
    else if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "createVulkanInstance() --> vkCreateInstance:: vkCreateInstance failed due to unknown reason %d\n", vkResult);
        fflush(gpFile);
        return vkResult;
    }
    else
    {
        fprintf(gpFile, "createVulkanInstance() --> vkCreateInstance:: vkCreateInstance succedded\n");
        fflush(gpFile);
    }
    
    
    if(bValidation == TRUE)
    {
        vkResult = createValidationCallbackFunction();
        if(vkResult != VK_SUCCESS)
        {
            fprintf(gpFile, "createVulkanInstance() --> createValidationCallbackFunction() is failed\n");
            fflush(gpFile);
        }
        else
        {
            fprintf(gpFile, "createVulkanInstance() --> createValidationCallbackFunction() is succedded\n");
            fflush(gpFile);
        }
    }


   

    return vkResult;
}


VkResult fillInstanceExtensionNames(void)
{
    
    VkResult vkResult = VK_SUCCESS;

    
    uint32_t instanceExtensionCount = 0;

    

    vkResult = vkEnumerateInstanceExtensionProperties(NULL, 
                                                      &instanceExtensionCount,
                                                      NULL); 
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "fillInstanceExtensionNames() --> 1st call to vkEnumerateInstanceExtensionProperties() is failed\n");
        fflush(gpFile);
    }
    else
    {
        fprintf(gpFile, "fillInstanceExtensionNames() --> 1st call to vkEnumerateInstanceExtensionProperties() is succedded\n");
        fflush(gpFile);
    }



    
    VkExtensionProperties* vkExtensionProperties_array = NULL;
    vkExtensionProperties_array = (VkExtensionProperties*) malloc (sizeof(VkExtensionProperties) * instanceExtensionCount);
    

    vkResult = vkEnumerateInstanceExtensionProperties(NULL, 
                                                      &instanceExtensionCount,
                                                      vkExtensionProperties_array); 
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "fillInstanceExtensionNames() --> 2nd call to vkEnumerateInstanceExtensionProperties() is failed\n");
        fflush(gpFile);
    }
    else
    {
        fprintf(gpFile, "fillInstanceExtensionNames() --> 2nd call to vkEnumerateInstanceExtensionProperties() is succedded\n");
        fflush(gpFile);
    }



    
    char** instanceExtensionNames_array = NULL;

    instanceExtensionNames_array = (char**)malloc(sizeof(char*) * instanceExtensionCount);
    
    for(uint32_t i = 0; i < instanceExtensionCount; i++)
    {
        instanceExtensionNames_array[i] = (char*)malloc(sizeof(char) * strlen(vkExtensionProperties_array[i].extensionName) + 1);
        memcpy(instanceExtensionNames_array[i], vkExtensionProperties_array[i].extensionName, strlen(vkExtensionProperties_array[i].extensionName) + 1);
        fprintf(gpFile, "fillInstanceExtensionNames() --> Vulkan Instance Extension names = %s\n", instanceExtensionNames_array[i]);
        fflush(gpFile);
    }



   
   free(vkExtensionProperties_array);
   vkExtensionProperties_array = NULL;



   
   
   
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
                
            }
        }
    }



    
    
    for(uint32_t i = 0; i < instanceExtensionCount; i++)
    {
        free(instanceExtensionNames_array[i]);
    }
    free(instanceExtensionNames_array);



    
    if(vulkanSurfaceExtensionFound == VK_FALSE)
    {
        
        vkResult = VK_ERROR_INITIALIZATION_FAILED;
        fprintf(gpFile, "fillInstanceExtensionNames() --> VK_KHR_SURFACE_EXTENSION_NAME not found\n");
        fflush(gpFile);
        return vkResult;
    }
    else
    {
        fprintf(gpFile, "fillInstanceExtensionNames() --> VK_KHR_SURFACE_EXTENSION_NAME found\n");
        fflush(gpFile);
    }

    if(vulkanWin32SurfaceExtensionFound == VK_FALSE)
    {
        
        vkResult = VK_ERROR_INITIALIZATION_FAILED;
        fprintf(gpFile, "fillInstanceExtensionNames() --> VK_KHR_WIN32_SURFACE_EXTENSION_NAME not found\n");
        fflush(gpFile);
        return vkResult;
    }
    else
    {
        fprintf(gpFile, "fillInstanceExtensionNames() --> VK_KHR_WIN32_SURFACE_EXTENSION_NAME found\n");
        fflush(gpFile);
    }
    
    if(vulkanDebugReportExtensionFound == VK_FALSE)
    {
        if(bValidation == TRUE)
        {
            
            vkResult = VK_ERROR_INITIALIZATION_FAILED;
            fprintf(gpFile, "fillInstanceExtensionNames() --> VK_EXT_DEBUG_REPORT_EXTENSION_NAME not found:: Validation is ON But required VK_EXT_DEBUG_REPORT_EXTENSION_NAME is not supported\n");
            fflush(gpFile);
            return vkResult;
        }
        else
        {
            fprintf(gpFile, "fillInstanceExtensionNames() --> VK_EXT_DEBUG_REPORT_EXTENSION_NAME not found:: Validation is OFF But required VK_EXT_DEBUG_REPORT_EXTENSION_NAME is not supported\n");
            fflush(gpFile);
        }
    }
    else
    {
        if(bValidation == TRUE)
        {
            fprintf(gpFile, "fillInstanceExtensionNames() --> VK_EXT_DEBUG_REPORT_EXTENSION_NAME found:: Validation is ON and required VK_EXT_DEBUG_REPORT_EXTENSION_NAME is supported\n");
            fflush(gpFile);
        }
        else
        {
            fprintf(gpFile, "fillInstanceExtensionNames() --> VK_EXT_DEBUG_REPORT_EXTENSION_NAME found:: Validation is OFF and required VK_EXT_DEBUG_REPORT_EXTENSION_NAME is supported\n");
            fflush(gpFile);
        }
    }


    
    for(uint32_t i = 0; i < enabledInstanceExtensionCount; i++)
    {
        fprintf(gpFile, "fillInstanceExtensionNames() --> Enabled vulkan Instance extension Names = %s\n", enabledInstanceExtensionNames_array[i]);
        fflush(gpFile);
    }

    return vkResult;
}


VkResult fillValidationLayerNames(void)
{
    
    
    VkResult vkResult = VK_SUCCESS;
    uint32_t validationLayerCount = 0;
    
    vkResult = vkEnumerateInstanceLayerProperties(&validationLayerCount,
                                                  NULL); 
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "fillValidationLayerNames() --> 1st call to vkEnumerateInstanceLayerProperties() is failed: error code %d\n", vkResult);
        fflush(gpFile);
    }
    else
    {
        fprintf(gpFile, "fillValidationLayerNames() --> 1st call to vkEnumerateInstanceLayerProperties() is succedded\n");
        fflush(gpFile);
    }
    
    VkLayerProperties* vkLayerProperties_array = NULL;
    vkLayerProperties_array = (VkLayerProperties*) malloc (sizeof(VkLayerProperties) * validationLayerCount);
    

    vkResult = vkEnumerateInstanceLayerProperties(&validationLayerCount,
                                                  vkLayerProperties_array); 
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "fillValidationLayerNames() --> 2nd call to vkEnumerateInstanceLayerProperties() is failed: error code %d\n", vkResult);
        fflush(gpFile);
    }
    else
    {
        fprintf(gpFile, "fillValidationLayerNames() --> 2nd call to vkEnumerateInstanceLayerProperties() is succedded\n");
        fflush(gpFile);
    }
    
    char** validationLayerNames_array = NULL;
    validationLayerNames_array = (char**) malloc(sizeof(char*) * validationLayerCount);
    
    for(uint32_t i = 0; i < validationLayerCount; i++)
    {
        validationLayerNames_array[i] = (char*)malloc(sizeof(char) * strlen(vkLayerProperties_array[i].layerName) + 1);
        
        memcpy(validationLayerNames_array[i], vkLayerProperties_array[i].layerName, strlen(vkLayerProperties_array[i].layerName) + 1);
        fprintf(gpFile, "fillValidationLayerNames() --> Vulkan Validation Layer names = %s\n", vkLayerProperties_array[i].layerName);
        fflush(gpFile);
    }
    
    if(vkLayerProperties_array) 
        free(vkLayerProperties_array);
    vkLayerProperties_array = NULL;

    
    
    VkBool32 vulkanValidationLayerFound = VK_FALSE;
    for(uint32_t i = 0; i < validationLayerCount; i++)
    {
        if(strcmp(validationLayerNames_array[i], "VK_LAYER_KHRONOS_validation") == 0)
        {
            vulkanValidationLayerFound = VK_TRUE;
            enabledValidationLayerNames_array[enabledValidationLayerCount++] = "VK_LAYER_KHRONOS_validation";
        }
    }
    
     
    for(uint32_t i = 0; i < validationLayerCount; i++)
    {
        free(validationLayerNames_array[i]);
    }
    free(validationLayerNames_array);
    
    if(bValidation == TRUE)
    {
        
        if(vulkanValidationLayerFound == VK_FALSE)
        {
            
            vkResult = VK_ERROR_INITIALIZATION_FAILED;
            fprintf(gpFile, "fillValidationLayerNames() --> VK_LAYER_KHRONOS_validation not supported\n");
            fflush(gpFile);
            return vkResult;
        }
        else
        {
            fprintf(gpFile, "fillValidationLayerNames() --> VK_LAYER_KHRONOS_validation is supported\n");
            fflush(gpFile);
        }
    }
    
    
    for(uint32_t i = 0; i < enabledValidationLayerCount; i++)
    {
        fprintf(gpFile, "fillValidationLayerNames() --> Enabled vulkan validation layer Names = %s\n", enabledValidationLayerNames_array[i]);
        fflush(gpFile);
    }
    
    return (vkResult);    
}


VkResult createValidationCallbackFunction(void)
{
    VKAPI_ATTR VkBool32 VKAPI_CALL debugReportCallback(VkDebugReportFlagsEXT,
                                                       VkDebugReportObjectTypeEXT,
                                                       uint64_t,
                                                       size_t,
                                                       int32_t,
                                                       const char*,
                                                       const char*,
                                                       void*);

    VkResult vkResult = VK_SUCCESS;

    // Get function pointers from the instance
    PFN_vkCreateDebugReportCallbackEXT vkCreateDebugReportCallbackEXT_fnptr = NULL;
    vkCreateDebugReportCallbackEXT_fnptr =
        (PFN_vkCreateDebugReportCallbackEXT)vkGetInstanceProcAddr(
            vkInstance, "vkCreateDebugReportCallbackEXT");
    if (vkCreateDebugReportCallbackEXT_fnptr == NULL)
    {
        vkResult = VK_ERROR_INITIALIZATION_FAILED;
        fprintf(gpFile, "createValidationCallbackFunction() --> vkGetInstanceProcAddr() failed for vkCreateDebugReportCallbackEXT\n");
        fflush(gpFile);
        return vkResult;
    }
    else
    {
        fprintf(gpFile, "createValidationCallbackFunction() --> vkGetInstanceProcAddr() got vkCreateDebugReportCallbackEXT\n");
        fflush(gpFile);
    }

    // *** IMPORTANT: use the GLOBAL destroy pointer (do not redeclare a local!) ***
    vkDestroyDebugReportCallbackEXT_fnptr =
        (PFN_vkDestroyDebugReportCallbackEXT)vkGetInstanceProcAddr(
            vkInstance, "vkDestroyDebugReportCallbackEXT");
    if (vkDestroyDebugReportCallbackEXT_fnptr == NULL)
    {
        vkResult = VK_ERROR_INITIALIZATION_FAILED;
        fprintf(gpFile, "createValidationCallbackFunction() --> vkGetInstanceProcAddr() failed for vkDestroyDebugReportCallbackEXT\n");
        fflush(gpFile);
        return vkResult;
    }
    else
    {
        fprintf(gpFile, "createValidationCallbackFunction() --> vkGetInstanceProcAddr() got vkDestroyDebugReportCallbackEXT\n");
        fflush(gpFile);
    }

    VkDebugReportCallbackCreateInfoEXT createInfo;
    memset(&createInfo, 0, sizeof(createInfo));
    createInfo.sType       = VK_STRUCTURE_TYPE_DEBUG_REPORT_CREATE_INFO_EXT;
    createInfo.flags       = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT;
    createInfo.pfnCallback = debugReportCallback;
    createInfo.pUserData   = NULL;

    vkResult = vkCreateDebugReportCallbackEXT_fnptr(vkInstance, &createInfo, NULL, &vkDebugReportCallbackEXT);
    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "createValidationCallbackFunction() --> vkCreateDebugReportCallbackEXT_fnptr() failed: %d\n", vkResult);
        fflush(gpFile);
    }
    else
    {
        fprintf(gpFile, "createValidationCallbackFunction() --> vkCreateDebugReportCallbackEXT_fnptr() succeeded\n");
        fflush(gpFile);
    }

    return vkResult;
}

VkResult getSupportedSurface(void)
{
    
    VkResult vkResult = VK_SUCCESS;

    
    VkWin32SurfaceCreateInfoKHR vkWin32SurfaceCreateInfoKHR;
    memset((void*)&vkWin32SurfaceCreateInfoKHR, 0, sizeof(VkWin32SurfaceCreateInfoKHR));
    vkWin32SurfaceCreateInfoKHR.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
    vkWin32SurfaceCreateInfoKHR.pNext = NULL;
    vkWin32SurfaceCreateInfoKHR.flags = 0;
    
    
    
    vkWin32SurfaceCreateInfoKHR.hinstance = (HINSTANCE)GetWindowLongPtr(ghwnd, GWLP_HINSTANCE);
    vkWin32SurfaceCreateInfoKHR.hwnd = ghwnd;

    
    vkResult = vkCreateWin32SurfaceKHR(vkInstance,
                                       &vkWin32SurfaceCreateInfoKHR,
                                       NULL, 
                                       &vkSurfaceKHR);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "getSupportedSurface() --> vkCreateWin32SurfaceKHR() is failed %d\n", vkResult);
        fflush(gpFile);
    }
    else
    {
        fprintf(gpFile, "getSupportedSurface() --> vkCreateWin32SurfaceKHR() is succedded\n");
        fflush(gpFile);
    }

    return vkResult;
}


VkResult getPhysicalDevice(void)
{
    
    VkResult vkResult = VK_SUCCESS;
    

    
    vkResult = vkEnumeratePhysicalDevices(vkInstance,
                                          &physicalDeviceCount,
                                          NULL);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "getPhysicalDevice() --> 1st call to vkEnumeratePhysicalDevices() is failed %d\n", vkResult);
        fflush(gpFile);
        return vkResult;
    }
    else if(physicalDeviceCount == 0)
    {
        fprintf(gpFile, "getPhysicalDevice() --> 1st call to vkEnumeratePhysicalDevices() resulted in zero devices\n");
        fflush(gpFile);
        vkResult = VK_ERROR_INITIALIZATION_FAILED;
        return vkResult;
    }
    else
    {
        fprintf(gpFile, "getPhysicalDevice() --> 1st call to vkEnumeratePhysicalDevices() is succedded\n");
        fflush(gpFile);
    }

    vkPhysicalDevice_array = (VkPhysicalDevice*)malloc(sizeof(VkPhysicalDevice) * physicalDeviceCount);
    
    
    vkResult = vkEnumeratePhysicalDevices(vkInstance, 
                                          &physicalDeviceCount,
                                          vkPhysicalDevice_array);
     if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "getPhysicalDevice() --> 2nd call to vkEnumeratePhysicalDevices() is failed %d\n", vkResult);
        fflush(gpFile);
        return vkResult;
    }
    else
    {
        fprintf(gpFile, "getPhysicalDevice() --> 2nd call to vkEnumeratePhysicalDevices() is succedded\n");
        fflush(gpFile);
    }

    VkBool32 bFound = VK_FALSE;
    for(uint32_t i = 0; i < physicalDeviceCount; i++)
    {
        uint32_t qCount = UINT32_MAX;
        
        
        vkGetPhysicalDeviceQueueFamilyProperties(vkPhysicalDevice_array[i], 
                                               &qCount, 
                                               NULL);
        VkQueueFamilyProperties *vkQueueFamilyProperties_array = NULL;
        vkQueueFamilyProperties_array = (VkQueueFamilyProperties*)malloc(sizeof(VkQueueFamilyProperties) * qCount);
        
        
        vkGetPhysicalDeviceQueueFamilyProperties(vkPhysicalDevice_array[i], 
                                               &qCount, 
                                               vkQueueFamilyProperties_array);
        
        VkBool32* isQueueSurfaceSupported_array = NULL;
        isQueueSurfaceSupported_array = (VkBool32*)malloc(sizeof(VkBool32) * qCount);
        
        
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
            fflush(gpFile);
        }
        
        if(vkQueueFamilyProperties_array)
        {
            free(vkQueueFamilyProperties_array);
            vkQueueFamilyProperties_array = NULL;
            fprintf(gpFile, "getPhysicalDevice() --> vkQueueFamilyProperties_array succedded to free\n");
            fflush(gpFile);
        }
        
        if(bFound == VK_TRUE)
        {
            break;
        }
    }
    
    if(bFound == VK_TRUE)
    {
        fprintf(gpFile, "getPhysicalDevice() -->is succedded to select the required device with graphics enabled\n");
        fflush(gpFile);
    }
    else
    {
        if(vkPhysicalDevice_array)
        {
            free(vkPhysicalDevice_array);
            vkPhysicalDevice_array = NULL;
            fprintf(gpFile, "getPhysicalDevice() --> vkPhysicalDevice_array succedded to free\n");
            fflush(gpFile);
        }
        vkResult = VK_ERROR_INITIALIZATION_FAILED;
        fprintf(gpFile, "getPhysicalDevice() -->is failed to select the required device with graphics enabled\n");
        fflush(gpFile);
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
        fflush(gpFile);
    }
    else
    {
        fprintf(gpFile, "getPhysicalDevice() -->selected device not supports tessellationShader\n");
        fflush(gpFile);
    }
    
    if(vkPhysicalDeviceFeatures.geometryShader)
    {
        fprintf(gpFile, "getPhysicalDevice() -->selected device supports geometryShader\n");
        fflush(gpFile);
    }
    else
    {
        fprintf(gpFile, "getPhysicalDevice() -->selected device not supports geometryShader\n");
        fflush(gpFile);
    }
    
    return vkResult;
}


VkResult printVkInfo(void)
{
    
    VkResult vkResult = VK_SUCCESS;
    
    
    fprintf(gpFile, "*******************VULKAN INFORMATION*********************\n");
    fflush(gpFile);
    for(uint32_t i = 0; i < physicalDeviceCount; i++)    
    {
        fprintf(gpFile, "Infomration of Device = %d\n", i);
        fflush(gpFile);
        
        VkPhysicalDeviceProperties vkPhysicalDeviceProperties;
        memset((void*)&vkPhysicalDeviceProperties, 0, sizeof(VkPhysicalDeviceProperties));
        
        vkGetPhysicalDeviceProperties(vkPhysicalDevice_array[i], &vkPhysicalDeviceProperties);
        
        uint32_t majorVersion = VK_API_VERSION_MAJOR(vkPhysicalDeviceProperties.apiVersion);
        uint32_t minorVersion = VK_API_VERSION_MINOR(vkPhysicalDeviceProperties.apiVersion);;
        uint32_t patchVersion = VK_API_VERSION_PATCH(vkPhysicalDeviceProperties.apiVersion);;
        
        fprintf(gpFile, "apiVersion = %d.%d.%d\n", majorVersion, minorVersion, patchVersion);
        fflush(gpFile);
        
        
        fprintf(gpFile, "DeviceName = %s\n", vkPhysicalDeviceProperties.deviceName);
        fflush(gpFile);
        
        
        switch(vkPhysicalDeviceProperties.deviceType)
        {
            case(VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU):
                fprintf(gpFile, "DeviceType = Integrated GPU(iGPU)\n");
                fflush(gpFile);
                break;
            
            case(VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU):
                fprintf(gpFile, "DeviceType = Discrete GPU(dGPU)\n");
                fflush(gpFile);
                break;
                
            case(VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU):
                fprintf(gpFile, "DeviceType = Virtual GPU(vGPU)\n");
                fflush(gpFile);
                break;
                
            case(VK_PHYSICAL_DEVICE_TYPE_CPU):
                fprintf(gpFile, "DeviceType = CPU\n");
                fflush(gpFile);
                break;    
                
            case(VK_PHYSICAL_DEVICE_TYPE_OTHER):
                fprintf(gpFile, "DeviceType = Other\n");
                fflush(gpFile);
                break; 
                
            default: 
                fprintf(gpFile, "DeviceType = UNKNOWN\n");  
                fflush(gpFile);              
        }
        
        
        fprintf(gpFile, "VendorId = 0x%04x\n", vkPhysicalDeviceProperties.vendorID);
        fflush(gpFile);
        
        
        fprintf(gpFile, "DeviceId = 0x%04x\n\n", vkPhysicalDeviceProperties.deviceID);
        fflush(gpFile);
   }
   
   fprintf(gpFile, "****************END OF VULKAN INFORMATION********************\n");
   fflush(gpFile);
   
    
    if(vkPhysicalDevice_array)
    {
        free(vkPhysicalDevice_array);
        vkPhysicalDevice_array = NULL;
        fprintf(gpFile, "printVkInfo() --> vkPhysicalDevice_array succedded to free\n");
        fflush(gpFile);
    }
    
    return vkResult;
}


VkResult fillDeviceExtensionNames(void)
{
    
    VkResult vkResult = VK_SUCCESS;

    
    uint32_t deviceExtensionCount = 0;

    vkResult = vkEnumerateDeviceExtensionProperties(vkPhysicalDevice_selected,
                                                    NULL,  
                                                    &deviceExtensionCount,
                                                    NULL); 
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "fillDeviceExtensionNames() --> 1st call to vkEnumerateDeviceExtensionProperties() is failed: %d\n", vkResult);
        fflush(gpFile);
        return vkResult;
    }
    else
    {
        fprintf(gpFile, "fillDeviceExtensionNames() --> 1st call to vkEnumerateDeviceExtensionProperties() is succedded\n");
        fprintf(gpFile, "deviceExtensionCount is %u\n", deviceExtensionCount);
        fflush(gpFile);
    }


    
    VkExtensionProperties* vkExtensionProperties_array = NULL;
    vkExtensionProperties_array = (VkExtensionProperties*) malloc (sizeof(VkExtensionProperties) * deviceExtensionCount);
    

    vkResult = vkEnumerateDeviceExtensionProperties(vkPhysicalDevice_selected,
                                                    NULL, 
                                                    &deviceExtensionCount,
                                                    vkExtensionProperties_array); 
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "fillDeviceExtensionNames() --> 2nd call to vkEnumerateDeviceExtensionProperties() is failed: %d\n", vkResult);
        fflush(gpFile);
        return vkResult;
    }
    else
    {
        fprintf(gpFile, "fillDeviceExtensionNames() --> 2nd call to vkEnumerateDeviceExtensionProperties() is succedded\n");
        fflush(gpFile);
    }



    
    char** deviceExtensionNames_array = NULL;

    deviceExtensionNames_array = (char**)malloc(sizeof(char*) * deviceExtensionCount);
    
    for(uint32_t i = 0; i < deviceExtensionCount; i++)
    {
        deviceExtensionNames_array[i] = (char*)malloc(sizeof(char) * strlen(vkExtensionProperties_array[i].extensionName) + 1);
        memcpy(deviceExtensionNames_array[i], vkExtensionProperties_array[i].extensionName, strlen(vkExtensionProperties_array[i].extensionName) + 1);
        fprintf(gpFile, "fillDeviceExtensionNames() --> Vulkan Device Extension names = %s\n", deviceExtensionNames_array[i]);
        fflush(gpFile);
    }



   
   free(vkExtensionProperties_array);
   vkExtensionProperties_array = NULL;



   
   
   VkBool32 vulkanSwapChainExtensionFound = VK_FALSE;
  
    for(uint32_t i = 0; i < deviceExtensionCount; i++)
    {
        if(strcmp(deviceExtensionNames_array[i], VK_KHR_SWAPCHAIN_EXTENSION_NAME) == 0)
        {
            vulkanSwapChainExtensionFound = VK_TRUE;
            enabledDeviceExtensionNames_array[enabledDeviceExtensionCount++] = VK_KHR_SWAPCHAIN_EXTENSION_NAME;
        }
    }



    
    
    for(uint32_t i = 0; i < deviceExtensionCount; i++)
    {
        free(deviceExtensionNames_array[i]);
    }
    free(deviceExtensionNames_array);



    
    if(vulkanSwapChainExtensionFound == VK_FALSE)
    {
        
        vkResult = VK_ERROR_INITIALIZATION_FAILED;
        fprintf(gpFile, "fillDeviceExtensionNames() --> VK_KHR_SWAPCHAIN_EXTENSION_NAME not found\n");
        fflush(gpFile);
        return vkResult;
    }
    else
    {
        fprintf(gpFile, "fillDeviceExtensionNames() --> VK_KHR_SWAPCHAIN_EXTENSION_NAME found\n");
        fflush(gpFile);
    }


    
    for(uint32_t i = 0; i < enabledDeviceExtensionCount; i++)
    {
        fprintf(gpFile, "fillDeviceExtensionNames() --> Enabled vulkan Device extension Names = %s\n", enabledDeviceExtensionNames_array[i]);
        fflush(gpFile);
    }

    return vkResult;
}


VkResult createVulkanDevice(void)
{  
    
    VkResult fillDeviceExtensionNames(void);
    
    
    VkResult vkResult = VK_SUCCESS;
    
    
    vkResult = fillDeviceExtensionNames();
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "createVulkanDevice() --> fillDeviceExtensionNames() is failed %d\n", vkResult);
        fflush(gpFile);
        return vkResult;
    }
    else
    {
        fprintf(gpFile, "createVulkanDevice() --> fillDeviceExtensionNames() is succedded\n");
        fflush(gpFile);
    }
    
    
    
    float QueuePriorities[] = {1.0};
    VkDeviceQueueCreateInfo vkDeviceQueueCreateInfo;
    memset((void*)&vkDeviceQueueCreateInfo, 0, sizeof(VkDeviceQueueCreateInfo));
    vkDeviceQueueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    vkDeviceQueueCreateInfo.pNext = NULL;
    vkDeviceQueueCreateInfo.flags = 0;
    vkDeviceQueueCreateInfo.queueFamilyIndex = graphicsQueueFamilyIndex_selected;
    vkDeviceQueueCreateInfo.queueCount = 1;
    vkDeviceQueueCreateInfo.pQueuePriorities = QueuePriorities;
    
    
    VkDeviceCreateInfo vkDeviceCreateInfo;
    memset((void*)&vkDeviceCreateInfo, 0, sizeof(VkDeviceCreateInfo));
    
    vkDeviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    vkDeviceCreateInfo.pNext = NULL;
    vkDeviceCreateInfo.flags = 0;
    vkDeviceCreateInfo.enabledExtensionCount = enabledDeviceExtensionCount;
    vkDeviceCreateInfo.ppEnabledExtensionNames = enabledDeviceExtensionNames_array;
    vkDeviceCreateInfo.enabledLayerCount = 0;  
    vkDeviceCreateInfo.ppEnabledLayerNames = NULL;  
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
        fflush(gpFile);
        return vkResult;
    }
    else
    {
        fprintf(gpFile, "createVulkanDevice() --> vkCreateDevice() is succedded\n");
        fflush(gpFile);
    }                     
    
    return vkResult;
}


void getDeviceQueue(void)
{
    
    vkGetDeviceQueue(vkDevice, 
                     graphicsQueueFamilyIndex_selected,
                     0, 
                     &vkQueue);
    if(vkQueue == VK_NULL_HANDLE) 
    {
        fprintf(gpFile, "getDeviceQueue() --> vkGetDeviceQueue() returned NULL for vkQueue\n");
        fflush(gpFile);
        return;
    }
    else
    {
        fprintf(gpFile, "getDeviceQueue() --> vkGetDeviceQueue() is succedded\n");
        fflush(gpFile);
    }
}


VkResult getPhysicalDeviceSurfaceFormatAndColorSpace(void)
{
    
    VkResult vkResult = VK_SUCCESS;   
    uint32_t formatCount = 0;
    
    
    
    vkResult = vkGetPhysicalDeviceSurfaceFormatsKHR(vkPhysicalDevice_selected,
                                                    vkSurfaceKHR,
                                                    &formatCount,
                                                    NULL);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "getPhysicalDeviceSurfaceFormatAndColorSpace() --> 1st call to vkGetPhysicalDeviceSurfaceFormatsKHR() is failed %d\n", vkResult);
        fflush(gpFile);
        return vkResult;
    }
    else if(formatCount == 0)
    {
        fprintf(gpFile, "getPhysicalDeviceSurfaceFormatAndColorSpace() --> 1st call to vkGetPhysicalDeviceSurfaceFormatsKHR() is failed as formatCount is zero:: %d\n", vkResult);
        fflush(gpFile);
        return VK_ERROR_INITIALIZATION_FAILED;
    }
    else
    {
        fprintf(gpFile, "getPhysicalDeviceSurfaceFormatAndColorSpace() --> 1st call to vkGetPhysicalDeviceSurfaceFormatsKHR() is succedded\n");
        fflush(gpFile);
    }
    
    fprintf(gpFile, "getPhysicalDeviceSurfaceFormatAndColorSpace() --> total formatCount are:: %d\n", formatCount);
    fflush(gpFile);
    
    VkSurfaceFormatKHR* vkSurfaceFormatKHR_array = (VkSurfaceFormatKHR*) malloc(formatCount * sizeof(VkSurfaceFormatKHR));
    
    
    
    vkResult = vkGetPhysicalDeviceSurfaceFormatsKHR(vkPhysicalDevice_selected,
                                                    vkSurfaceKHR,
                                                    &formatCount,
                                                    vkSurfaceFormatKHR_array);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "getPhysicalDeviceSurfaceFormatAndColorSpace() --> 2nd call to vkGetPhysicalDeviceSurfaceFormatsKHR() is failed %d\n", vkResult);
        fflush(gpFile);
        return vkResult;
    }
    else
    {
        fprintf(gpFile, "getPhysicalDeviceSurfaceFormatAndColorSpace() --> 2nd call to vkGetPhysicalDeviceSurfaceFormatsKHR() is succedded\n");
        fflush(gpFile);
    }
    
    if(formatCount == 1 && vkSurfaceFormatKHR_array[0].format == VK_FORMAT_UNDEFINED) 
    {
        vkFormat_color = VK_FORMAT_B8G8R8A8_UNORM;
        fprintf(gpFile, "getPhysicalDeviceSurfaceFormatAndColorSpace() --> vkFormat_color is VK_FORMAT_B8G8R8A8_UNORM\n");
        fflush(gpFile);
    }
    else
    {
        vkFormat_color = vkSurfaceFormatKHR_array[0].format;
        fprintf(gpFile, "getPhysicalDeviceSurfaceFormatAndColorSpace() --> vkFormat_color is %d\n", vkFormat_color);
        fflush(gpFile);
    }
    
    
    vkColorSpaceKHR = vkSurfaceFormatKHR_array[0].colorSpace;
    
    if(vkSurfaceFormatKHR_array)
    {
        free(vkSurfaceFormatKHR_array);
        vkSurfaceFormatKHR_array = NULL;
        fprintf(gpFile, "getPhysicalDeviceSurfaceFormatAndColorSpace() --> vkSurfaceFormatKHR_array is freed\n");
        fflush(gpFile);
    }
    
    return vkResult;
}


VkResult getPhysicalDevicePresentMode(void)
{
    
    VkResult vkResult = VK_SUCCESS;   
    
    uint32_t presentModeCount = 0;
    
    
    vkResult = vkGetPhysicalDeviceSurfacePresentModesKHR(vkPhysicalDevice_selected,
                                                         vkSurfaceKHR,
                                                         &presentModeCount,
                                                         NULL);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "getPhysicalDevicePresentMode() --> 1st call to vkGetPhysicalDeviceSurfacePresentModesKHR() is failed %d\n", vkResult);
        fflush(gpFile);
        return vkResult;
    }
    else if(presentModeCount == 0)
    {
        fprintf(gpFile, "getPhysicalDevicePresentMode() --> 1st call to vkGetPhysicalDeviceSurfacePresentModesKHR() is failed as formatCount is zero:: %d\n", vkResult);
        fflush(gpFile);
        return VK_ERROR_INITIALIZATION_FAILED;
    }
    else
    {
        fprintf(gpFile, "getPhysicalDevicePresentMode() --> 1st call to vkGetPhysicalDeviceSurfacePresentModesKHR() is succedded\n");
        fflush(gpFile);
    }   

    fprintf(gpFile, "getPhysicalDevicePresentMode() --> total presentModeCount are:: %d\n", presentModeCount);
    fflush(gpFile);

    VkPresentModeKHR* vkPresentModeKHR_array = (VkPresentModeKHR*) malloc(presentModeCount * sizeof(VkPresentModeKHR));
    
    
    vkResult = vkGetPhysicalDeviceSurfacePresentModesKHR(vkPhysicalDevice_selected,
                                                         vkSurfaceKHR,
                                                         &presentModeCount,
                                                         vkPresentModeKHR_array);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "getPhysicalDevicePresentMode() --> 2nd call to vkGetPhysicalDeviceSurfacePresentModesKHR() is failed %d\n", vkResult);
        fflush(gpFile);
        return vkResult;
    }
    else
    {
        fprintf(gpFile, "getPhysicalDevicePresentMode() --> 2nd call to vkGetPhysicalDeviceSurfacePresentModesKHR() is succedded\n");
        fflush(gpFile);
    }
    
    
    for(uint32_t i = 0; i < presentModeCount; i++)
    {
        if(vkPresentModeKHR_array[i] == VK_PRESENT_MODE_MAILBOX_KHR)
        {
            vkPresentModeKHR = VK_PRESENT_MODE_MAILBOX_KHR;
            fprintf(gpFile, "getPhysicalDevicePresentMode() --> vkPresentModeKHR is VK_PRESENT_MODE_MAILBOX_KHR\n");
            fflush(gpFile);
            break;
        }
    }
    
    if(vkPresentModeKHR != VK_PRESENT_MODE_MAILBOX_KHR)
    {
        vkPresentModeKHR = VK_PRESENT_MODE_FIFO_KHR;
        fprintf(gpFile, "getPhysicalDevicePresentMode() --> vkPresentModeKHR is VK_PRESENT_MODE_FIFO_KHR\n");   
        fflush(gpFile);
    }
  
    
    if(vkPresentModeKHR_array)
    {
        free(vkPresentModeKHR_array);
        vkPresentModeKHR_array = NULL;
        fprintf(gpFile, "getPhysicalDevicePresentMode() --> vkPresentModeKHR_array is freed\n");
        fflush(gpFile);
    }
    
    return vkResult;
  
}


VkResult createSwapchain(VkBool32 vsync)  
{
    
    VkResult getPhysicalDeviceSurfaceFormatAndColorSpace(void);
    VkResult getPhysicalDevicePresentMode(void);
    
    
    VkResult vkResult = VK_SUCCESS;
        
    
    
    vkResult = getPhysicalDeviceSurfaceFormatAndColorSpace();
    
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "createSwapchain() --> getPhysicalDeviceSurfaceFormatAndColorSpace() is failed %d\n", vkResult);
        fflush(gpFile);
        return vkResult;
    }
    else
    {
        fprintf(gpFile, "createSwapchain() --> getPhysicalDeviceSurfaceFormatAndColorSpace() is succedded\n");
        fflush(gpFile);
    }
   
   
    
    VkSurfaceCapabilitiesKHR vkSurfaceCapabilitiesKHR;
    memset((void*)&vkSurfaceCapabilitiesKHR, 0, sizeof(VkSurfaceCapabilitiesKHR));
    
    vkResult = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(vkPhysicalDevice_selected,
                                                         vkSurfaceKHR,
                                                         &vkSurfaceCapabilitiesKHR);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "createSwapchain() --> vkGetPhysicalDeviceSurfaceCapabilitiesKHR() is failed %d\n", vkResult);
        fflush(gpFile);
        return vkResult;
    }
    else
    {
        fprintf(gpFile, "createSwapchain() --> vkGetPhysicalDeviceSurfaceCapabilitiesKHR() is succedded\n");
        fflush(gpFile);
    }
   
    
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
    
    
    memset((void*)&vkExtent2D_swapchain, 0, sizeof(VkExtent2D));
    if(vkSurfaceCapabilitiesKHR.currentExtent.width != UINT32_MAX)
    {
        vkExtent2D_swapchain.width = vkSurfaceCapabilitiesKHR.currentExtent.width;
        vkExtent2D_swapchain.height = vkSurfaceCapabilitiesKHR.currentExtent.height;
        
        fprintf(gpFile, "createSwapchain() --> Swapchain image width X height = %d X %d \n", vkExtent2D_swapchain.width, vkExtent2D_swapchain.height);
        fflush(gpFile);
    }
    else
    {
        
        VkExtent2D vkExtent2D;
        memset((void*)&vkExtent2D, 0 , sizeof(VkExtent2D));
        vkExtent2D.width = (uint32_t)winWidth;
        vkExtent2D.height = (uint32_t)winHeight;
        
        vkExtent2D_swapchain.width = glm::max(vkSurfaceCapabilitiesKHR.minImageExtent.width, glm::min(vkSurfaceCapabilitiesKHR.maxImageExtent.width, vkExtent2D.width));
        vkExtent2D_swapchain.height = glm::max(vkSurfaceCapabilitiesKHR.minImageExtent.height, glm::min(vkSurfaceCapabilitiesKHR.maxImageExtent.height, vkExtent2D.height));
        
        fprintf(gpFile, "createSwapchain() --> Swapchain image width X height = %d X %d \n", vkExtent2D_swapchain.width, vkExtent2D_swapchain.height);
        fflush(gpFile);
    }
    
    
    VkImageUsageFlags vkImageUsageFlags = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT; 
    
    
    VkSurfaceTransformFlagBitsKHR vkSurfaceTransformFlagBitsKHR;
    if(vkSurfaceCapabilitiesKHR.supportedTransforms & VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR)
    {
        vkSurfaceTransformFlagBitsKHR = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
    }
    else
    {
        vkSurfaceTransformFlagBitsKHR = vkSurfaceCapabilitiesKHR.currentTransform;
    }
   
    
    
    vkResult = getPhysicalDevicePresentMode();
    
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "createSwapchain() --> getPhysicalDevicePresentMode() is failed %d\n", vkResult);
        fflush(gpFile);
        return vkResult;
    }
    else
    {
        fprintf(gpFile, "createSwapchain() --> getPhysicalDevicePresentMode() is succedded\n");
        fflush(gpFile);
    }
    
    
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
    
    
    vkResult = vkCreateSwapchainKHR(vkDevice,
                                    &vkSwapchainCreateInfoKHR,
                                    NULL,
                                    &vkSwapchainKHR);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "createSwapchain() --> vkCreateSwapchainKHR() is failed %d\n", vkResult);
        fflush(gpFile);
        return vkResult;
    }
    else
    {
        fprintf(gpFile, "createSwapchain() --> vkCreateSwapchainKHR() is succedded\n");
        fflush(gpFile);
    }
    
    return vkResult;
}

VkResult GetSupportedDepthFormat(VkPhysicalDevice physicalDevice,
                                 VkImageTiling tiling,
                                 VkFormat* outFormat)
{
    // Validate pointer
    if (!outFormat) return VK_ERROR_INITIALIZATION_FAILED;

    // From most to least precise
    const VkFormat candidates[] = {
        VK_FORMAT_D32_SFLOAT_S8_UINT,
        VK_FORMAT_D32_SFLOAT,
        VK_FORMAT_D24_UNORM_S8_UINT,
        VK_FORMAT_D16_UNORM_S8_UINT,
        VK_FORMAT_D16_UNORM
    };

    for (uint32_t i = 0; i < (uint32_t)(sizeof(candidates)/sizeof(candidates[0])); ++i)
    {
        VkFormatProperties props;
        memset((void*)&props, 0, sizeof(VkFormatProperties));
        vkGetPhysicalDeviceFormatProperties(physicalDevice, candidates[i], &props);

        const VkFormatFeatureFlags need = VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT;
        const VkBool32 okLinear  = (tiling == VK_IMAGE_TILING_LINEAR)  && (props.linearTilingFeatures  & need);
        const VkBool32 okOptimal = (tiling == VK_IMAGE_TILING_OPTIMAL) && (props.optimalTilingFeatures & need);

        if (okLinear || okOptimal)
        {
            *outFormat = candidates[i];        // caller passes &vkFormat_depth
            return VK_SUCCESS;
        }
    }

    return VK_ERROR_FORMAT_NOT_SUPPORTED;
}

VkResult createImagesAndImageViews(void)
{
    // Updated forward declaration to match new 3‑arg signature
    VkResult GetSupportedDepthFormat(VkPhysicalDevice physicalDevice,
                                     VkImageTiling tiling,
                                     VkFormat* outFormat);

    VkResult vkResult = VK_SUCCESS;

    // 1) Query swapchain images (count)
    vkResult = vkGetSwapchainImagesKHR(vkDevice,
                                       vkSwapchainKHR,
                                       &swapchainImageCount,
                                       NULL);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "createImagesAndImageViews() --> 1st call to vkGetSwapchainImagesKHR() is failed %d\n", vkResult);
        fflush(gpFile);
        return vkResult;
    }
    else if(0 == swapchainImageCount)
    {
        fprintf(gpFile, "createImagesAndImageViews() --> 1st call to vkGetSwapchainImagesKHR() is failed %d\n", vkResult);
        fflush(gpFile);
        vkResult = VK_ERROR_INITIALIZATION_FAILED;
        return vkResult;
    }
    else
    {
        fprintf(gpFile, "createImagesAndImageViews() --> gives swapchainImagecount = %d\n", swapchainImageCount);
        fprintf(gpFile, "createImagesAndImageViews() --> vkGetSwapchainImagesKHR() is succedded\n");
        fflush(gpFile);
    }

    // 2) Get swapchain images (handles)
    swapchainImage_array = (VkImage*)malloc(sizeof(VkImage) * swapchainImageCount);

    vkResult = vkGetSwapchainImagesKHR(vkDevice,
                                       vkSwapchainKHR,
                                       &swapchainImageCount,
                                       swapchainImage_array);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "createImagesAndImageViews() --> 2nd call to vkGetSwapchainImagesKHR() is failed %d\n", vkResult);
        fflush(gpFile);
        return vkResult;
    }
    else
    {
        fprintf(gpFile, "createImagesAndImageViews() --> 2nd call to vkGetSwapchainImagesKHR() is succedded\n");
        fflush(gpFile);
    }

    // 3) Create swapchain image views (color)
    swapchainImageView_array = (VkImageView*)malloc(sizeof(VkImageView) * swapchainImageCount);

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
    vkImageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    vkImageViewCreateInfo.subresourceRange.baseMipLevel = 0;
    vkImageViewCreateInfo.subresourceRange.levelCount = 1;
    vkImageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
    vkImageViewCreateInfo.subresourceRange.layerCount = 1;
    vkImageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;

    for(uint32_t i = 0; i < swapchainImageCount; i++)
    {
        vkImageViewCreateInfo.image = swapchainImage_array[i];

        vkResult = vkCreateImageView(vkDevice,
                                     &vkImageViewCreateInfo,
                                     NULL,
                                     &swapchainImageView_array[i]);
        if(vkResult != VK_SUCCESS)
        {
            fprintf(gpFile, "createImagesAndImageViews() --> vkCreateImageViews() is failed for iteration %d and error code is %d\n", i, vkResult);
            fflush(gpFile);
            return vkResult;
        }
        else
        {
            fprintf(gpFile, "createImagesAndImageViews() --> vkCreateImageViews() is succedded for iteration for %d\n", i);
            fflush(gpFile);
        }
    }

    // 4) Pick a supported depth format (UPDATED call)
    vkResult = GetSupportedDepthFormat(vkPhysicalDevice_selected,
                                       VK_IMAGE_TILING_OPTIMAL,
                                       &vkFormat_depth);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "createImagesAndImageViews() --> GetSupportedDepthFormat() is failed error code is %d\n", vkResult);
        fflush(gpFile);
        return vkResult;
    }
    else
    {
        fprintf(gpFile, "createImagesAndImageViews() --> GetSupportedDepthFormat() is succedded\n");
        fflush(gpFile);
    }

    // 5) Create depth image
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
        fprintf(gpFile, "createImagesAndImageViews() --> vkCreateImage() is failed error code is %d\n", vkResult);
        fflush(gpFile);
        return vkResult;
    }
    else
    {
        fprintf(gpFile, "createImagesAndImageViews() --> vkCreateImage() is succedded\n");
        fflush(gpFile);
    }

    // 6) Allocate & bind memory for depth image
    VkMemoryRequirements vkMemoryRequirements;
    memset((void*)&vkMemoryRequirements, 0, sizeof(VkMemoryRequirements));
    vkGetImageMemoryRequirements(vkDevice, vkImage_depth, &vkMemoryRequirements);

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
        fflush(gpFile);
        return vkResult;
    }
    else
    {
        fprintf(gpFile, "createImagesAndImageViews() --> vkAllocateMemory() is succedded\n");
        fflush(gpFile);
    }

    vkResult = vkBindImageMemory(vkDevice, vkImage_depth, vkDeviceMemory_depth, 0);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "createImagesAndImageViews() --> vkBindBufferMemory() is failed and error code is %d\n", vkResult);
        fflush(gpFile);
        return vkResult;
    }
    else
    {
        fprintf(gpFile, "createImagesAndImageViews() --> vkBindBufferMemory() is succedded\n");
        fflush(gpFile);
    }

    // 7) Create depth image view (aspect mask conditioned on stencil support)
    memset((void*)&vkImageViewCreateInfo, 0, sizeof(VkImageViewCreateInfo));
    vkImageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    vkImageViewCreateInfo.pNext = NULL;
    vkImageViewCreateInfo.flags = 0;
    vkImageViewCreateInfo.format = vkFormat_depth;

    VkImageAspectFlags depthAspect = VK_IMAGE_ASPECT_DEPTH_BIT;
    if (vkFormat_depth == VK_FORMAT_D16_UNORM_S8_UINT ||
        vkFormat_depth == VK_FORMAT_D24_UNORM_S8_UINT ||
        vkFormat_depth == VK_FORMAT_D32_SFLOAT_S8_UINT)
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
        fprintf(gpFile, "createImagesAndImageViews() --> vkCreateImageView() is failed error code is %d\n", vkResult);
        fflush(gpFile);
        return vkResult;
    }
    else
    {
        fprintf(gpFile, "createImagesAndImageViews() --> vkCreateImageView() is succedded\n");
        fflush(gpFile);
    }

    return vkResult;
}

VkResult createCommandPool(void)
{
    
   VkResult vkResult = VK_SUCCESS;
    
   
   VkCommandPoolCreateInfo vkCommandPoolCreateInfo;
   memset((void*)&vkCommandPoolCreateInfo, 0, sizeof(VkCommandPoolCreateInfo));
   
   vkCommandPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
   vkCommandPoolCreateInfo.pNext = NULL;
   vkCommandPoolCreateInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT; 
   vkCommandPoolCreateInfo.queueFamilyIndex = graphicsQueueFamilyIndex_selected;
   
   vkResult = vkCreateCommandPool(vkDevice, 
                                  &vkCommandPoolCreateInfo,
                                  NULL,
                                  &vkCommandPool);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "createCommandPool() --> vkCreateCommandPool() is failed and error code is %d\n",vkResult);
        fflush(gpFile);
        return vkResult;
    }
    else
    {
        fprintf(gpFile, "createCommandPool() --> vkCreateCommandPool() is succedded \n");
        fflush(gpFile);
    }

   return vkResult;   
}


VkResult createCommandBuffers(void)
{
   
   VkResult vkResult = VK_SUCCESS;
   
   
   
   VkCommandBufferAllocateInfo vkCommandBufferAllocateInfo; 
   memset((void*)&vkCommandBufferAllocateInfo, 0, sizeof(VkCommandBufferAllocateInfo));
   
   vkCommandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
   vkCommandBufferAllocateInfo.pNext = NULL;
   vkCommandBufferAllocateInfo.commandPool = vkCommandPool;
   vkCommandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
   vkCommandBufferAllocateInfo.commandBufferCount = 1;
   
   vkCommandBuffer_array = (VkCommandBuffer*)malloc(sizeof(VkCommandBuffer) * swapchainImageCount);
   
   
   for(uint32_t i = 0; i < swapchainImageCount; i++)
    {
        vkResult = vkAllocateCommandBuffers(vkDevice, &vkCommandBufferAllocateInfo, &vkCommandBuffer_array[i]);
        if(vkResult != VK_SUCCESS)
        {
            fprintf(gpFile, "createCommandBuffers() --> vkAllocateCommandBuffers() is failed for %d iteration and error code is %d\n",i, vkResult);
            fflush(gpFile);
            return vkResult;
        }
        else
        {
            fprintf(gpFile, "createCommandBuffers() --> vkAllocateCommandBuffers() is succedded for iteration %d\n", i);
            fflush(gpFile);
        }
    }
   
   return vkResult;   
}

VkResult createVertexBuffer(void)
{
    
    VkResult vkResult = VK_SUCCESS; 
    
    
    memset((void*)&vertexData_position_sun, 0, sizeof(VertexData));
    
    
    VkBufferCreateInfo vkBufferCreateInfo;
    memset((void*)&vkBufferCreateInfo , 0, sizeof(VkBufferCreateInfo));
    vkBufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    vkBufferCreateInfo.pNext = NULL;
    
    vkBufferCreateInfo.flags = 0;
    vkBufferCreateInfo.size = sizeof(sphere_vertices);
    vkBufferCreateInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
    
    
    vkResult = vkCreateBuffer(vkDevice, 
                              &vkBufferCreateInfo,
                              NULL,
                              &vertexData_position_sun.vkBuffer);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "createVertexBuffer() --> vkCreateBuffer() is failed for position and error code is %d\n", vkResult);
        fflush(gpFile);
        return vkResult;
    }
    else
    {
        fprintf(gpFile, "createVertexBuffer() --> vkCreateBuffer() is succedded for position \n");
        fflush(gpFile);
    }
    
    VkMemoryRequirements vkMemoryRequirements;
    memset((void*)&vkMemoryRequirements, 0, sizeof(VkMemoryRequirements));
   
    vkGetBufferMemoryRequirements(vkDevice, vertexData_position_sun.vkBuffer, &vkMemoryRequirements);
    
    
    VkMemoryAllocateInfo vkMemoryAllocateInfo;
    memset((void*)&vkMemoryAllocateInfo , 0, sizeof(VkMemoryAllocateInfo));
    vkMemoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    vkMemoryAllocateInfo.pNext = NULL; 
    vkMemoryAllocateInfo.allocationSize = vkMemoryRequirements.size;
    
    vkMemoryAllocateInfo.memoryTypeIndex = 0;
   
    for(int i = 0; i < vkPhysicalDeviceMemoryProperties.memoryTypeCount; i++)
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
    
    
    vkResult = vkAllocateMemory(vkDevice, &vkMemoryAllocateInfo, NULL, &vertexData_position_sun.vkDeviceMemory);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "createVertexBuffer() --> vkAllocateMemory() is failed for position and error code is %d\n", vkResult);
        fflush(gpFile);
        return vkResult;
    }
    else
    {
        fprintf(gpFile, "createVertexBuffer() --> vkAllocateMemory() is succedded for position \n");
        fflush(gpFile);
    }
    
    
    vkResult = vkBindBufferMemory(vkDevice, vertexData_position_sun.vkBuffer, vertexData_position_sun.vkDeviceMemory, 0);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "createVertexBuffer() --> vkBindBufferMemory() is failed for position and error code is %d\n", vkResult);
        fflush(gpFile);
        return vkResult;
    }
    else
    {
        fprintf(gpFile, "createVertexBuffer() --> vkBindBufferMemory() is succedded for position \n");
        fflush(gpFile);
    }
    
    
    void* data = NULL;
    vkResult = vkMapMemory(vkDevice, vertexData_position_sun.vkDeviceMemory, 0, vkMemoryAllocateInfo.allocationSize, 0, &data);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "createVertexBuffer() --> vkMapMemory() is failed for position and error code is %d\n", vkResult);
        fflush(gpFile);
        return vkResult;
    }
    else
    {
        fprintf(gpFile, "createVertexBuffer() --> vkMapMemory() is succedded for position \n");
        fflush(gpFile);
    }
   
    
    memcpy(data, sphere_vertices, sizeof(sphere_vertices));
    
    vkUnmapMemory(vkDevice, vertexData_position_sun.vkDeviceMemory);
   
    
    
    memset((void*)&vertexData_texcoord_sun, 0, sizeof(VertexData));
    
    
    memset((void*)&vkBufferCreateInfo , 0, sizeof(VkBufferCreateInfo));
    vkBufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    vkBufferCreateInfo.pNext = NULL;
    
    vkBufferCreateInfo.flags = 0;
    vkBufferCreateInfo.size = sizeof(sphere_textures);
    vkBufferCreateInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
    
    
    vkResult = vkCreateBuffer(vkDevice, 
                              &vkBufferCreateInfo,
                              NULL,
                              &vertexData_texcoord_sun.vkBuffer);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "createVertexBuffer() --> vkCreateBuffer() is failed for texcoord and error code is %d\n", vkResult);
        fflush(gpFile);
        return vkResult;
    }
    else
    {
        fprintf(gpFile, "createVertexBuffer() --> vkCreateBuffer() is succedded for texcoord\n");
        fflush(gpFile);
    }
    
    memset((void*)&vkMemoryRequirements, 0, sizeof(VkMemoryRequirements));
   
    vkGetBufferMemoryRequirements(vkDevice, vertexData_texcoord_sun.vkBuffer, &vkMemoryRequirements);
    
    
    memset((void*)&vkMemoryAllocateInfo , 0, sizeof(VkMemoryAllocateInfo));
    vkMemoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    vkMemoryAllocateInfo.pNext = NULL; 
    vkMemoryAllocateInfo.allocationSize = vkMemoryRequirements.size;
    
    vkMemoryAllocateInfo.memoryTypeIndex = 0;
   
    for(int i = 0; i < vkPhysicalDeviceMemoryProperties.memoryTypeCount; i++)
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
    
    
    vkResult = vkAllocateMemory(vkDevice, &vkMemoryAllocateInfo, NULL, &vertexData_texcoord_sun.vkDeviceMemory);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "createVertexBuffer() --> vkAllocateMemory() is failed for texcoord and error code is %d\n", vkResult);
        fflush(gpFile);
        return vkResult;
    }
    else
    {
        fprintf(gpFile, "createVertexBuffer() --> vkAllocateMemory() is succedded for texcoord\n");
        fflush(gpFile);
    }
    
    
    vkResult = vkBindBufferMemory(vkDevice, vertexData_texcoord_sun.vkBuffer, vertexData_texcoord_sun.vkDeviceMemory, 0);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "createVertexBuffer() --> vkBindBufferMemory() is failed for texcoord and error code is %d\n", vkResult);
        fflush(gpFile);
        return vkResult;
    }
    else
    {
        fprintf(gpFile, "createVertexBuffer() --> vkBindBufferMemory() is succedded for texcoord\n");
        fflush(gpFile);
    }
    
    
    data = NULL;
    vkResult = vkMapMemory(vkDevice, vertexData_texcoord_sun.vkDeviceMemory, 0, vkMemoryAllocateInfo.allocationSize, 0, &data);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "createVertexBuffer() --> vkMapMemory() is failed for texcoord and error code is %d\n", vkResult);
        fflush(gpFile);
        return vkResult;
    }
    else
    {
        fprintf(gpFile, "createVertexBuffer() --> vkMapMemory() is succedded for texcoord\n");
        fflush(gpFile);
    }
   
    
    memcpy(data, sphere_textures, sizeof(sphere_textures));
    
    vkUnmapMemory(vkDevice, vertexData_texcoord_sun.vkDeviceMemory);
   
   
    
    
   
    
    
    memset((void*)&vertexData_normal_sun, 0, sizeof(VertexData));
    
    
    memset((void*)&vkBufferCreateInfo , 0, sizeof(VkBufferCreateInfo));
    vkBufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    vkBufferCreateInfo.pNext = NULL;
    
    vkBufferCreateInfo.flags = 0;
    vkBufferCreateInfo.size = sizeof(sphere_normals);
    vkBufferCreateInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
    
    
    vkResult = vkCreateBuffer(vkDevice, 
                              &vkBufferCreateInfo,
                              NULL,
                              &vertexData_normal_sun.vkBuffer);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "createVertexBuffer() --> vkCreateBuffer() is failed for normal and error code is %d\n", vkResult);
        fflush(gpFile);
        return vkResult;
    }
    else
    {
        fprintf(gpFile, "createVertexBuffer() --> vkCreateBuffer() is succedded for normal\n");
        fflush(gpFile);
    }
    
    memset((void*)&vkMemoryRequirements, 0, sizeof(VkMemoryRequirements));
   
    vkGetBufferMemoryRequirements(vkDevice, vertexData_normal_sun.vkBuffer, &vkMemoryRequirements);
    
    
    memset((void*)&vkMemoryAllocateInfo , 0, sizeof(VkMemoryAllocateInfo));
    vkMemoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    vkMemoryAllocateInfo.pNext = NULL; 
    vkMemoryAllocateInfo.allocationSize = vkMemoryRequirements.size;
    
    vkMemoryAllocateInfo.memoryTypeIndex = 0;
   
    for(int i = 0; i < vkPhysicalDeviceMemoryProperties.memoryTypeCount; i++)
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
    
    
    vkResult = vkAllocateMemory(vkDevice, &vkMemoryAllocateInfo, NULL, &vertexData_normal_sun.vkDeviceMemory);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "createVertexBuffer() --> vkAllocateMemory() is failed for normal and error code is %d\n", vkResult);
        fflush(gpFile);
        return vkResult;
    }
    else
    {
        fprintf(gpFile, "createVertexBuffer() --> vkAllocateMemory() is succedded for normal\n");
        fflush(gpFile);
    }
    
    
    vkResult = vkBindBufferMemory(vkDevice, vertexData_normal_sun.vkBuffer, vertexData_normal_sun.vkDeviceMemory, 0);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "createVertexBuffer() --> vkBindBufferMemory() is failed for normal and error code is %d\n", vkResult);
        fflush(gpFile);
        return vkResult;
    }
    else
    {
        fprintf(gpFile, "createVertexBuffer() --> vkBindBufferMemory() is succedded for normal\n");
        fflush(gpFile);
    }
    
    
    data = NULL;
    vkResult = vkMapMemory(vkDevice, vertexData_normal_sun.vkDeviceMemory, 0, vkMemoryAllocateInfo.allocationSize, 0, &data);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "createVertexBuffer() --> vkMapMemory() is failed for normal and error code is %d\n", vkResult);
        fflush(gpFile);
        return vkResult;
    }
    else
    {
        fprintf(gpFile, "createVertexBuffer() --> vkMapMemory() is succedded for normal\n");
        fflush(gpFile);
    }
   
    
    memcpy(data, sphere_normals, sizeof(sphere_normals));
    
    vkUnmapMemory(vkDevice, vertexData_normal_sun.vkDeviceMemory);
    
    
    
    
    memset((void*)&vertexData_position_mercury, 0, sizeof(VertexData));
    
    
    memset((void*)&vkBufferCreateInfo , 0, sizeof(VkBufferCreateInfo));
    vkBufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    vkBufferCreateInfo.pNext = NULL;
    
    vkBufferCreateInfo.flags = 0;
    vkBufferCreateInfo.size = sizeof(sphere_vertices);
    vkBufferCreateInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
    
    
    vkResult = vkCreateBuffer(vkDevice, 
                              &vkBufferCreateInfo,
                              NULL,
                              &vertexData_position_mercury.vkBuffer);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "createVertexBuffer() --> vkCreateBuffer() is failed for position and error code is %d\n", vkResult);
        fflush(gpFile);
        return vkResult;
    }
    else
    {
        fprintf(gpFile, "createVertexBuffer() --> vkCreateBuffer() is succedded for position \n");
        fflush(gpFile);
    }
    
    memset((void*)&vkMemoryRequirements, 0, sizeof(VkMemoryRequirements));
   
    vkGetBufferMemoryRequirements(vkDevice, vertexData_position_mercury.vkBuffer, &vkMemoryRequirements);
    
    
    memset((void*)&vkMemoryAllocateInfo , 0, sizeof(VkMemoryAllocateInfo));
    vkMemoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    vkMemoryAllocateInfo.pNext = NULL; 
    vkMemoryAllocateInfo.allocationSize = vkMemoryRequirements.size;
    
    vkMemoryAllocateInfo.memoryTypeIndex = 0;
   
    for(int i = 0; i < vkPhysicalDeviceMemoryProperties.memoryTypeCount; i++)
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
    
    
    vkResult = vkAllocateMemory(vkDevice, &vkMemoryAllocateInfo, NULL, &vertexData_position_mercury.vkDeviceMemory);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "createVertexBuffer() --> vkAllocateMemory() is failed for position and error code is %d\n", vkResult);
        fflush(gpFile);
        return vkResult;
    }
    else
    {
        fprintf(gpFile, "createVertexBuffer() --> vkAllocateMemory() is succedded for position \n");
        fflush(gpFile);
    }
    
    
    vkResult = vkBindBufferMemory(vkDevice, vertexData_position_mercury.vkBuffer, vertexData_position_mercury.vkDeviceMemory, 0);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "createVertexBuffer() --> vkBindBufferMemory() is failed for position and error code is %d\n", vkResult);
        fflush(gpFile);
        return vkResult;
    }
    else
    {
        fprintf(gpFile, "createVertexBuffer() --> vkBindBufferMemory() is succedded for position \n");
        fflush(gpFile);
    }
    
    
    data = NULL;
    vkResult = vkMapMemory(vkDevice, vertexData_position_mercury.vkDeviceMemory, 0, vkMemoryAllocateInfo.allocationSize, 0, &data);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "createVertexBuffer() --> vkMapMemory() is failed for position and error code is %d\n", vkResult);
        fflush(gpFile);
        return vkResult;
    }
    else
    {
        fprintf(gpFile, "createVertexBuffer() --> vkMapMemory() is succedded for position \n");
        fflush(gpFile);
    }
   
    
    memcpy(data, sphere_vertices, sizeof(sphere_vertices));
    
    vkUnmapMemory(vkDevice, vertexData_position_mercury.vkDeviceMemory);
   
    
    
    memset((void*)&vertexData_texcoord_mercury, 0, sizeof(VertexData));
    
    
    memset((void*)&vkBufferCreateInfo , 0, sizeof(VkBufferCreateInfo));
    vkBufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    vkBufferCreateInfo.pNext = NULL;
    
    vkBufferCreateInfo.flags = 0;
    vkBufferCreateInfo.size = sizeof(sphere_textures);
    vkBufferCreateInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
    
    
    vkResult = vkCreateBuffer(vkDevice, 
                              &vkBufferCreateInfo,
                              NULL,
                              &vertexData_texcoord_mercury.vkBuffer);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "createVertexBuffer() --> vkCreateBuffer() is failed for texcoord and error code is %d\n", vkResult);
        fflush(gpFile);
        return vkResult;
    }
    else
    {
        fprintf(gpFile, "createVertexBuffer() --> vkCreateBuffer() is succedded for texcoord\n");
        fflush(gpFile);
    }
    
    memset((void*)&vkMemoryRequirements, 0, sizeof(VkMemoryRequirements));
   
    vkGetBufferMemoryRequirements(vkDevice, vertexData_texcoord_mercury.vkBuffer, &vkMemoryRequirements);
    
    
    memset((void*)&vkMemoryAllocateInfo , 0, sizeof(VkMemoryAllocateInfo));
    vkMemoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    vkMemoryAllocateInfo.pNext = NULL; 
    vkMemoryAllocateInfo.allocationSize = vkMemoryRequirements.size;
    
    vkMemoryAllocateInfo.memoryTypeIndex = 0;
   
    for(int i = 0; i < vkPhysicalDeviceMemoryProperties.memoryTypeCount; i++)
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
    
    
    vkResult = vkAllocateMemory(vkDevice, &vkMemoryAllocateInfo, NULL, &vertexData_texcoord_mercury.vkDeviceMemory);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "createVertexBuffer() --> vkAllocateMemory() is failed for texcoord and error code is %d\n", vkResult);
        fflush(gpFile);
        return vkResult;
    }
    else
    {
        fprintf(gpFile, "createVertexBuffer() --> vkAllocateMemory() is succedded for texcoord\n");
        fflush(gpFile);
    }
    
    
    vkResult = vkBindBufferMemory(vkDevice, vertexData_texcoord_mercury.vkBuffer, vertexData_texcoord_mercury.vkDeviceMemory, 0);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "createVertexBuffer() --> vkBindBufferMemory() is failed for texcoord and error code is %d\n", vkResult);
        fflush(gpFile);
        return vkResult;
    }
    else
    {
        fprintf(gpFile, "createVertexBuffer() --> vkBindBufferMemory() is succedded for texcoord\n");
        fflush(gpFile);
    }
    
    
    data = NULL;
    vkResult = vkMapMemory(vkDevice, vertexData_texcoord_mercury.vkDeviceMemory, 0, vkMemoryAllocateInfo.allocationSize, 0, &data);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "createVertexBuffer() --> vkMapMemory() is failed for texcoord and error code is %d\n", vkResult);
        fflush(gpFile);
        return vkResult;
    }
    else
    {
        fprintf(gpFile, "createVertexBuffer() --> vkMapMemory() is succedded for texcoord\n");
        fflush(gpFile);
    }
   
    
    memcpy(data, sphere_textures, sizeof(sphere_textures));
    
    vkUnmapMemory(vkDevice, vertexData_texcoord_mercury.vkDeviceMemory);
   
   
    
    
   
    
    
    memset((void*)&vertexData_normal_mercury, 0, sizeof(VertexData));
    
    
    memset((void*)&vkBufferCreateInfo , 0, sizeof(VkBufferCreateInfo));
    vkBufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    vkBufferCreateInfo.pNext = NULL;
    
    vkBufferCreateInfo.flags = 0;
    vkBufferCreateInfo.size = sizeof(sphere_normals);
    vkBufferCreateInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
    
    
    vkResult = vkCreateBuffer(vkDevice, 
                              &vkBufferCreateInfo,
                              NULL,
                              &vertexData_normal_mercury.vkBuffer);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "createVertexBuffer() --> vkCreateBuffer() is failed for normal and error code is %d\n", vkResult);
        fflush(gpFile);
        return vkResult;
    }
    else
    {
        fprintf(gpFile, "createVertexBuffer() --> vkCreateBuffer() is succedded for normal\n");
        fflush(gpFile);
    }
    
    memset((void*)&vkMemoryRequirements, 0, sizeof(VkMemoryRequirements));
   
    vkGetBufferMemoryRequirements(vkDevice, vertexData_normal_mercury.vkBuffer, &vkMemoryRequirements);
    
    
    memset((void*)&vkMemoryAllocateInfo , 0, sizeof(VkMemoryAllocateInfo));
    vkMemoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    vkMemoryAllocateInfo.pNext = NULL; 
    vkMemoryAllocateInfo.allocationSize = vkMemoryRequirements.size;
    
    vkMemoryAllocateInfo.memoryTypeIndex = 0;
   
    for(int i = 0; i < vkPhysicalDeviceMemoryProperties.memoryTypeCount; i++)
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
    
    
    vkResult = vkAllocateMemory(vkDevice, &vkMemoryAllocateInfo, NULL, &vertexData_normal_mercury.vkDeviceMemory);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "createVertexBuffer() --> vkAllocateMemory() is failed for normal and error code is %d\n", vkResult);
        fflush(gpFile);
        return vkResult;
    }
    else
    {
        fprintf(gpFile, "createVertexBuffer() --> vkAllocateMemory() is succedded for normal\n");
        fflush(gpFile);
    }
    
    
    vkResult = vkBindBufferMemory(vkDevice, vertexData_normal_mercury.vkBuffer, vertexData_normal_mercury.vkDeviceMemory, 0);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "createVertexBuffer() --> vkBindBufferMemory() is failed for normal and error code is %d\n", vkResult);
        fflush(gpFile);
        return vkResult;
    }
    else
    {
        fprintf(gpFile, "createVertexBuffer() --> vkBindBufferMemory() is succedded for normal\n");
        fflush(gpFile);
    }
    
    
    data = NULL;
    vkResult = vkMapMemory(vkDevice, vertexData_normal_mercury.vkDeviceMemory, 0, vkMemoryAllocateInfo.allocationSize, 0, &data);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "createVertexBuffer() --> vkMapMemory() is failed for normal and error code is %d\n", vkResult);
        fflush(gpFile);
        return vkResult;
    }
    else
    {
        fprintf(gpFile, "createVertexBuffer() --> vkMapMemory() is succedded for normal\n");
        fflush(gpFile);
    }
   
    
    memcpy(data, sphere_normals, sizeof(sphere_normals));
    
    vkUnmapMemory(vkDevice, vertexData_normal_mercury.vkDeviceMemory);
    
    
    
    
    memset((void*)&vertexData_position_venus, 0, sizeof(VertexData));
    
    
    memset((void*)&vkBufferCreateInfo , 0, sizeof(VkBufferCreateInfo));
    vkBufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    vkBufferCreateInfo.pNext = NULL;
    
    vkBufferCreateInfo.flags = 0;
    vkBufferCreateInfo.size = sizeof(sphere_vertices);
    vkBufferCreateInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
    
    
    vkResult = vkCreateBuffer(vkDevice, 
                              &vkBufferCreateInfo,
                              NULL,
                              &vertexData_position_venus.vkBuffer);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "createVertexBuffer() --> vkCreateBuffer() is failed for position and error code is %d\n", vkResult);
        fflush(gpFile);
        return vkResult;
    }
    else
    {
        fprintf(gpFile, "createVertexBuffer() --> vkCreateBuffer() is succedded for position \n");
        fflush(gpFile);
    }
    
    memset((void*)&vkMemoryRequirements, 0, sizeof(VkMemoryRequirements));
   
    vkGetBufferMemoryRequirements(vkDevice, vertexData_position_venus.vkBuffer, &vkMemoryRequirements);
    
    
    memset((void*)&vkMemoryAllocateInfo , 0, sizeof(VkMemoryAllocateInfo));
    vkMemoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    vkMemoryAllocateInfo.pNext = NULL; 
    vkMemoryAllocateInfo.allocationSize = vkMemoryRequirements.size;
    
    vkMemoryAllocateInfo.memoryTypeIndex = 0;
   
    for(int i = 0; i < vkPhysicalDeviceMemoryProperties.memoryTypeCount; i++)
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
    
    
    vkResult = vkAllocateMemory(vkDevice, &vkMemoryAllocateInfo, NULL, &vertexData_position_venus.vkDeviceMemory);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "createVertexBuffer() --> vkAllocateMemory() is failed for position and error code is %d\n", vkResult);
        fflush(gpFile);
        return vkResult;
    }
    else
    {
        fprintf(gpFile, "createVertexBuffer() --> vkAllocateMemory() is succedded for position \n");
        fflush(gpFile);
    }
    
    
    vkResult = vkBindBufferMemory(vkDevice, vertexData_position_venus.vkBuffer, vertexData_position_venus.vkDeviceMemory, 0);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "createVertexBuffer() --> vkBindBufferMemory() is failed for position and error code is %d\n", vkResult);
        fflush(gpFile);
        return vkResult;
    }
    else
    {
        fprintf(gpFile, "createVertexBuffer() --> vkBindBufferMemory() is succedded for position \n");
        fflush(gpFile);
    }
    
    
    data = NULL;
    vkResult = vkMapMemory(vkDevice, vertexData_position_venus.vkDeviceMemory, 0, vkMemoryAllocateInfo.allocationSize, 0, &data);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "createVertexBuffer() --> vkMapMemory() is failed for position and error code is %d\n", vkResult);
        fflush(gpFile);
        return vkResult;
    }
    else
    {
        fprintf(gpFile, "createVertexBuffer() --> vkMapMemory() is succedded for position \n");
        fflush(gpFile);
    }
   
    
    memcpy(data, sphere_vertices, sizeof(sphere_vertices));
    
    vkUnmapMemory(vkDevice, vertexData_position_venus.vkDeviceMemory);
   
    
    
    memset((void*)&vertexData_texcoord_venus, 0, sizeof(VertexData));
    
    
    memset((void*)&vkBufferCreateInfo , 0, sizeof(VkBufferCreateInfo));
    vkBufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    vkBufferCreateInfo.pNext = NULL;
    
    vkBufferCreateInfo.flags = 0;
    vkBufferCreateInfo.size = sizeof(sphere_textures);
    vkBufferCreateInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
    
    
    vkResult = vkCreateBuffer(vkDevice, 
                              &vkBufferCreateInfo,
                              NULL,
                              &vertexData_texcoord_venus.vkBuffer);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "createVertexBuffer() --> vkCreateBuffer() is failed for texcoord and error code is %d\n", vkResult);
        fflush(gpFile);
        return vkResult;
    }
    else
    {
        fprintf(gpFile, "createVertexBuffer() --> vkCreateBuffer() is succedded for texcoord\n");
        fflush(gpFile);
    }
    
    memset((void*)&vkMemoryRequirements, 0, sizeof(VkMemoryRequirements));
   
    vkGetBufferMemoryRequirements(vkDevice, vertexData_texcoord_venus.vkBuffer, &vkMemoryRequirements);
    
    
    memset((void*)&vkMemoryAllocateInfo , 0, sizeof(VkMemoryAllocateInfo));
    vkMemoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    vkMemoryAllocateInfo.pNext = NULL; 
    vkMemoryAllocateInfo.allocationSize = vkMemoryRequirements.size;
    
    vkMemoryAllocateInfo.memoryTypeIndex = 0;
   
    for(int i = 0; i < vkPhysicalDeviceMemoryProperties.memoryTypeCount; i++)
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
    
    
    vkResult = vkAllocateMemory(vkDevice, &vkMemoryAllocateInfo, NULL, &vertexData_texcoord_venus.vkDeviceMemory);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "createVertexBuffer() --> vkAllocateMemory() is failed for texcoord and error code is %d\n", vkResult);
        fflush(gpFile);
        return vkResult;
    }
    else
    {
        fprintf(gpFile, "createVertexBuffer() --> vkAllocateMemory() is succedded for texcoord\n");
        fflush(gpFile);
    }
    
    
    vkResult = vkBindBufferMemory(vkDevice, vertexData_texcoord_venus.vkBuffer, vertexData_texcoord_venus.vkDeviceMemory, 0);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "createVertexBuffer() --> vkBindBufferMemory() is failed for texcoord and error code is %d\n", vkResult);
        fflush(gpFile);
        return vkResult;
    }
    else
    {
        fprintf(gpFile, "createVertexBuffer() --> vkBindBufferMemory() is succedded for texcoord\n");
        fflush(gpFile); 
    }
    
    
    data = NULL;
    vkResult = vkMapMemory(vkDevice, vertexData_texcoord_venus.vkDeviceMemory, 0, vkMemoryAllocateInfo.allocationSize, 0, &data);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "createVertexBuffer() --> vkMapMemory() is failed for texcoord and error code is %d\n", vkResult);
        fflush(gpFile);
        return vkResult;
    }
    else
    {
        fprintf(gpFile, "createVertexBuffer() --> vkMapMemory() is succedded for texcoord\n");
        fflush(gpFile);
    }
   
    
    memcpy(data, sphere_textures, sizeof(sphere_textures));
    
    vkUnmapMemory(vkDevice, vertexData_texcoord_venus.vkDeviceMemory);
   
   
    
    
   
    
    
    memset((void*)&vertexData_normal_venus, 0, sizeof(VertexData));
    
    
    memset((void*)&vkBufferCreateInfo , 0, sizeof(VkBufferCreateInfo));
    vkBufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    vkBufferCreateInfo.pNext = NULL;
    
    vkBufferCreateInfo.flags = 0;
    vkBufferCreateInfo.size = sizeof(sphere_normals);
    vkBufferCreateInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
    
    
    vkResult = vkCreateBuffer(vkDevice, 
                              &vkBufferCreateInfo,
                              NULL,
                              &vertexData_normal_venus.vkBuffer);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "createVertexBuffer() --> vkCreateBuffer() is failed for normal and error code is %d\n", vkResult);
        fflush(gpFile);
        return vkResult;
    }
    else
    {
        fprintf(gpFile, "createVertexBuffer() --> vkCreateBuffer() is succedded for normal\n");
        fflush(gpFile);
    }
    
    memset((void*)&vkMemoryRequirements, 0, sizeof(VkMemoryRequirements));
   
    vkGetBufferMemoryRequirements(vkDevice, vertexData_normal_venus.vkBuffer, &vkMemoryRequirements);
    
    
    memset((void*)&vkMemoryAllocateInfo , 0, sizeof(VkMemoryAllocateInfo));
    vkMemoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    vkMemoryAllocateInfo.pNext = NULL; 
    vkMemoryAllocateInfo.allocationSize = vkMemoryRequirements.size;
    
    vkMemoryAllocateInfo.memoryTypeIndex = 0;
   
    for(int i = 0; i < vkPhysicalDeviceMemoryProperties.memoryTypeCount; i++)
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
    
    
    vkResult = vkAllocateMemory(vkDevice, &vkMemoryAllocateInfo, NULL, &vertexData_normal_venus.vkDeviceMemory);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "createVertexBuffer() --> vkAllocateMemory() is failed for normal and error code is %d\n", vkResult);
        fflush(gpFile);
        return vkResult;
    }
    else
    {
        fprintf(gpFile, "createVertexBuffer() --> vkAllocateMemory() is succedded for normal\n");
        fflush(gpFile);
    }
    
    
    vkResult = vkBindBufferMemory(vkDevice, vertexData_normal_venus.vkBuffer, vertexData_normal_venus.vkDeviceMemory, 0);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "createVertexBuffer() --> vkBindBufferMemory() is failed for normal and error code is %d\n", vkResult);
        fflush(gpFile);
        return vkResult;
    }
    else
    {
        fprintf(gpFile, "createVertexBuffer() --> vkBindBufferMemory() is succedded for normal\n");
        fflush(gpFile);
    }
    
    
    data = NULL;
    vkResult = vkMapMemory(vkDevice, vertexData_normal_venus.vkDeviceMemory, 0, vkMemoryAllocateInfo.allocationSize, 0, &data);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "createVertexBuffer() --> vkMapMemory() is failed for normal and error code is %d\n", vkResult);
        fflush(gpFile);
        return vkResult;
    }
    else
    {
        fprintf(gpFile, "createVertexBuffer() --> vkMapMemory() is succedded for normal\n");
        fflush(gpFile);
    }
   
    
    memcpy(data, sphere_normals, sizeof(sphere_normals));
    
    vkUnmapMemory(vkDevice, vertexData_normal_venus.vkDeviceMemory);
   
   
    
    
    memset((void*)&vertexData_position_earth, 0, sizeof(VertexData));
    
    
    memset((void*)&vkBufferCreateInfo , 0, sizeof(VkBufferCreateInfo));
    vkBufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    vkBufferCreateInfo.pNext = NULL;
    
    vkBufferCreateInfo.flags = 0;
    vkBufferCreateInfo.size = sizeof(sphere_vertices);
    vkBufferCreateInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
    
    
    vkResult = vkCreateBuffer(vkDevice, 
                              &vkBufferCreateInfo,
                              NULL,
                              &vertexData_position_earth.vkBuffer);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "createVertexBuffer() --> vkCreateBuffer() is failed for position and error code is %d\n", vkResult);
        fflush(gpFile);
        return vkResult;
    }
    else
    {
        fprintf(gpFile, "createVertexBuffer() --> vkCreateBuffer() is succedded for position \n");
        fflush(gpFile);
    }
    
    memset((void*)&vkMemoryRequirements, 0, sizeof(VkMemoryRequirements));
   
    vkGetBufferMemoryRequirements(vkDevice, vertexData_position_earth.vkBuffer, &vkMemoryRequirements);
    
    
    memset((void*)&vkMemoryAllocateInfo , 0, sizeof(VkMemoryAllocateInfo));
    vkMemoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    vkMemoryAllocateInfo.pNext = NULL; 
    vkMemoryAllocateInfo.allocationSize = vkMemoryRequirements.size;
    
    vkMemoryAllocateInfo.memoryTypeIndex = 0;
   
    for(int i = 0; i < vkPhysicalDeviceMemoryProperties.memoryTypeCount; i++)
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
    
    
    vkResult = vkAllocateMemory(vkDevice, &vkMemoryAllocateInfo, NULL, &vertexData_position_earth.vkDeviceMemory);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "createVertexBuffer() --> vkAllocateMemory() is failed for position and error code is %d\n", vkResult);
        fflush(gpFile);
        return vkResult;
    }
    else
    {
        fprintf(gpFile, "createVertexBuffer() --> vkAllocateMemory() is succedded for position \n");
        fflush(gpFile);
    }
    
    
    vkResult = vkBindBufferMemory(vkDevice, vertexData_position_earth.vkBuffer, vertexData_position_earth.vkDeviceMemory, 0);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "createVertexBuffer() --> vkBindBufferMemory() is failed for position and error code is %d\n", vkResult);
        fflush(gpFile);
        return vkResult;
    }
    else
    {
        fprintf(gpFile, "createVertexBuffer() --> vkBindBufferMemory() is succedded for position \n");
        fflush(gpFile);
    }
    
    
    data = NULL;
    vkResult = vkMapMemory(vkDevice, vertexData_position_earth.vkDeviceMemory, 0, vkMemoryAllocateInfo.allocationSize, 0, &data);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "createVertexBuffer() --> vkMapMemory() is failed for position and error code is %d\n", vkResult);
        fflush(gpFile);
        return vkResult;
    }
    else
    {
        fprintf(gpFile, "createVertexBuffer() --> vkMapMemory() is succedded for position \n");
        fflush(gpFile);
    }
   
    
    memcpy(data, sphere_vertices, sizeof(sphere_vertices));
    
    vkUnmapMemory(vkDevice, vertexData_position_earth.vkDeviceMemory);
   
    
    
    memset((void*)&vertexData_texcoord_earth, 0, sizeof(VertexData));
    
    
    memset((void*)&vkBufferCreateInfo , 0, sizeof(VkBufferCreateInfo));
    vkBufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    vkBufferCreateInfo.pNext = NULL;
    
    vkBufferCreateInfo.flags = 0;
    vkBufferCreateInfo.size = sizeof(sphere_textures);
    vkBufferCreateInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
    
    
    vkResult = vkCreateBuffer(vkDevice, 
                              &vkBufferCreateInfo,
                              NULL,
                              &vertexData_texcoord_earth.vkBuffer);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "createVertexBuffer() --> vkCreateBuffer() is failed for texcoord and error code is %d\n", vkResult);
        fflush(gpFile);
        return vkResult;
    }
    else
    {
        fprintf(gpFile, "createVertexBuffer() --> vkCreateBuffer() is succedded for texcoord\n");
        fflush(gpFile);
    }
    
    memset((void*)&vkMemoryRequirements, 0, sizeof(VkMemoryRequirements));
   
    vkGetBufferMemoryRequirements(vkDevice, vertexData_texcoord_earth.vkBuffer, &vkMemoryRequirements);
    
    
    memset((void*)&vkMemoryAllocateInfo , 0, sizeof(VkMemoryAllocateInfo));
    vkMemoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    vkMemoryAllocateInfo.pNext = NULL; 
    vkMemoryAllocateInfo.allocationSize = vkMemoryRequirements.size;
    
    vkMemoryAllocateInfo.memoryTypeIndex = 0;
   
    for(int i = 0; i < vkPhysicalDeviceMemoryProperties.memoryTypeCount; i++)
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
    
    
    vkResult = vkAllocateMemory(vkDevice, &vkMemoryAllocateInfo, NULL, &vertexData_texcoord_earth.vkDeviceMemory);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "createVertexBuffer() --> vkAllocateMemory() is failed for texcoord and error code is %d\n", vkResult);
        fflush(gpFile);
        return vkResult;
    }
    else
    {
        fprintf(gpFile, "createVertexBuffer() --> vkAllocateMemory() is succedded for texcoord\n");
        fflush(gpFile);
    }
    
    
    vkResult = vkBindBufferMemory(vkDevice, vertexData_texcoord_earth.vkBuffer, vertexData_texcoord_earth.vkDeviceMemory, 0);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "createVertexBuffer() --> vkBindBufferMemory() is failed for texcoord and error code is %d\n", vkResult);
        fflush(gpFile);
        return vkResult;
    }
    else
    {
        fprintf(gpFile, "createVertexBuffer() --> vkBindBufferMemory() is succedded for texcoord\n");
        fflush(gpFile);
    }
    
    
    data = NULL;
    vkResult = vkMapMemory(vkDevice, vertexData_texcoord_earth.vkDeviceMemory, 0, vkMemoryAllocateInfo.allocationSize, 0, &data);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "createVertexBuffer() --> vkMapMemory() is failed for texcoord and error code is %d\n", vkResult);
        fflush(gpFile);
        return vkResult;
    }
    else
    {
        fprintf(gpFile, "createVertexBuffer() --> vkMapMemory() is succedded for texcoord\n");
        fflush(gpFile);
    }
   
    
    memcpy(data, sphere_textures, sizeof(sphere_textures));
    
    vkUnmapMemory(vkDevice, vertexData_texcoord_earth.vkDeviceMemory);
   
   
    
    
   
    
    
    memset((void*)&vertexData_normal_earth, 0, sizeof(VertexData));
    
    
    memset((void*)&vkBufferCreateInfo , 0, sizeof(VkBufferCreateInfo));
    vkBufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    vkBufferCreateInfo.pNext = NULL;
    
    vkBufferCreateInfo.flags = 0;
    vkBufferCreateInfo.size = sizeof(sphere_normals);
    vkBufferCreateInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
    
    
    vkResult = vkCreateBuffer(vkDevice, 
                              &vkBufferCreateInfo,
                              NULL,
                              &vertexData_normal_earth.vkBuffer);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "createVertexBuffer() --> vkCreateBuffer() is failed for normal and error code is %d\n", vkResult);
        fflush(gpFile);
        return vkResult;
    }
    else
    {
        fprintf(gpFile, "createVertexBuffer() --> vkCreateBuffer() is succedded for normal\n");
        fflush(gpFile);
    }
    
    memset((void*)&vkMemoryRequirements, 0, sizeof(VkMemoryRequirements));
   
    vkGetBufferMemoryRequirements(vkDevice, vertexData_normal_earth.vkBuffer, &vkMemoryRequirements);
    
    
    memset((void*)&vkMemoryAllocateInfo , 0, sizeof(VkMemoryAllocateInfo));
    vkMemoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    vkMemoryAllocateInfo.pNext = NULL; 
    vkMemoryAllocateInfo.allocationSize = vkMemoryRequirements.size;
    
    vkMemoryAllocateInfo.memoryTypeIndex = 0;
   
    for(int i = 0; i < vkPhysicalDeviceMemoryProperties.memoryTypeCount; i++)
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
    
    
    vkResult = vkAllocateMemory(vkDevice, &vkMemoryAllocateInfo, NULL, &vertexData_normal_earth.vkDeviceMemory);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "createVertexBuffer() --> vkAllocateMemory() is failed for normal and error code is %d\n", vkResult);
        fflush(gpFile);
        return vkResult;
    }
    else
    {
        fprintf(gpFile, "createVertexBuffer() --> vkAllocateMemory() is succedded for normal\n");
        fflush(gpFile);
    }
    
    
    vkResult = vkBindBufferMemory(vkDevice, vertexData_normal_earth.vkBuffer, vertexData_normal_earth.vkDeviceMemory, 0);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "createVertexBuffer() --> vkBindBufferMemory() is failed for normal and error code is %d\n", vkResult);
        fflush(gpFile);
        return vkResult;
    }
    else
    {
        fprintf(gpFile, "createVertexBuffer() --> vkBindBufferMemory() is succedded for normal\n");
        fflush(gpFile);
    }
    
    
    data = NULL;
    vkResult = vkMapMemory(vkDevice, vertexData_normal_earth.vkDeviceMemory, 0, vkMemoryAllocateInfo.allocationSize, 0, &data);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "createVertexBuffer() --> vkMapMemory() is failed for normal and error code is %d\n", vkResult);
        fflush(gpFile);
        return vkResult;
    }
    else
    {
        fprintf(gpFile, "createVertexBuffer() --> vkMapMemory() is succedded for normal\n");
        fflush(gpFile);
    }
   
    
    memcpy(data, sphere_normals, sizeof(sphere_normals));
    
    vkUnmapMemory(vkDevice, vertexData_normal_earth.vkDeviceMemory);
    
     
    
    memset((void*)&vertexData_position_mars, 0, sizeof(VertexData));
    
    
    memset((void*)&vkBufferCreateInfo , 0, sizeof(VkBufferCreateInfo));
    vkBufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    vkBufferCreateInfo.pNext = NULL;
    
    vkBufferCreateInfo.flags = 0;
    vkBufferCreateInfo.size = sizeof(sphere_vertices);
    vkBufferCreateInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
    
    
    vkResult = vkCreateBuffer(vkDevice, 
                              &vkBufferCreateInfo,
                              NULL,
                              &vertexData_position_mars.vkBuffer);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "createVertexBuffer() --> vkCreateBuffer() is failed for position and error code is %d\n", vkResult);
        fflush(gpFile);
        return vkResult;
    }
    else
    {
        fprintf(gpFile, "createVertexBuffer() --> vkCreateBuffer() is succedded for position \n");
        fflush(gpFile);
    }
    
    memset((void*)&vkMemoryRequirements, 0, sizeof(VkMemoryRequirements));
   
    vkGetBufferMemoryRequirements(vkDevice, vertexData_position_mars.vkBuffer, &vkMemoryRequirements);
    
    
    memset((void*)&vkMemoryAllocateInfo , 0, sizeof(VkMemoryAllocateInfo));
    vkMemoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    vkMemoryAllocateInfo.pNext = NULL; 
    vkMemoryAllocateInfo.allocationSize = vkMemoryRequirements.size;
    
    vkMemoryAllocateInfo.memoryTypeIndex = 0;
   
    for(int i = 0; i < vkPhysicalDeviceMemoryProperties.memoryTypeCount; i++)
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
    
    
    vkResult = vkAllocateMemory(vkDevice, &vkMemoryAllocateInfo, NULL, &vertexData_position_mars.vkDeviceMemory);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "createVertexBuffer() --> vkAllocateMemory() is failed for position and error code is %d\n", vkResult);
        fflush(gpFile);
        return vkResult;
    }
    else
    {
        fprintf(gpFile, "createVertexBuffer() --> vkAllocateMemory() is succedded for position \n");
        fflush(gpFile);
    }
    
    
    vkResult = vkBindBufferMemory(vkDevice, vertexData_position_mars.vkBuffer, vertexData_position_mars.vkDeviceMemory, 0);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "createVertexBuffer() --> vkBindBufferMemory() is failed for position and error code is %d\n", vkResult);
        fflush(gpFile);
        return vkResult;
    }
    else
    {
        fprintf(gpFile, "createVertexBuffer() --> vkBindBufferMemory() is succedded for position \n");
        fflush(gpFile);
    }
    
    
    data = NULL;
    vkResult = vkMapMemory(vkDevice, vertexData_position_mars.vkDeviceMemory, 0, vkMemoryAllocateInfo.allocationSize, 0, &data);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "createVertexBuffer() --> vkMapMemory() is failed for position and error code is %d\n", vkResult);
        fflush(gpFile);
        return vkResult;
    }
    else
    {
        fprintf(gpFile, "createVertexBuffer() --> vkMapMemory() is succedded for position \n");
        fflush(gpFile);
    }
   
    
    memcpy(data, sphere_vertices, sizeof(sphere_vertices));
    
    vkUnmapMemory(vkDevice, vertexData_position_mars.vkDeviceMemory);
   
    
    
    memset((void*)&vertexData_texcoord_mars, 0, sizeof(VertexData));
    
    
    memset((void*)&vkBufferCreateInfo , 0, sizeof(VkBufferCreateInfo));
    vkBufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    vkBufferCreateInfo.pNext = NULL;
    
    vkBufferCreateInfo.flags = 0;
    vkBufferCreateInfo.size = sizeof(sphere_textures);
    vkBufferCreateInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
    
    
    vkResult = vkCreateBuffer(vkDevice, 
                              &vkBufferCreateInfo,
                              NULL,
                              &vertexData_texcoord_mars.vkBuffer);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "createVertexBuffer() --> vkCreateBuffer() is failed for texcoord and error code is %d\n", vkResult);
        fflush(gpFile);
        return vkResult;
    }
    else
    {
        fprintf(gpFile, "createVertexBuffer() --> vkCreateBuffer() is succedded for texcoord\n");
        fflush(gpFile);
    }
    
    memset((void*)&vkMemoryRequirements, 0, sizeof(VkMemoryRequirements));
   
    vkGetBufferMemoryRequirements(vkDevice, vertexData_texcoord_mars.vkBuffer, &vkMemoryRequirements);
    
    
    memset((void*)&vkMemoryAllocateInfo , 0, sizeof(VkMemoryAllocateInfo));
    vkMemoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    vkMemoryAllocateInfo.pNext = NULL; 
    vkMemoryAllocateInfo.allocationSize = vkMemoryRequirements.size;
    
    vkMemoryAllocateInfo.memoryTypeIndex = 0;
   
    for(int i = 0; i < vkPhysicalDeviceMemoryProperties.memoryTypeCount; i++)
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
    
    
    vkResult = vkAllocateMemory(vkDevice, &vkMemoryAllocateInfo, NULL, &vertexData_texcoord_mars.vkDeviceMemory);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "createVertexBuffer() --> vkAllocateMemory() is failed for texcoord and error code is %d\n", vkResult);
        fflush(gpFile);
        return vkResult;
    }
    else
    {
        fprintf(gpFile, "createVertexBuffer() --> vkAllocateMemory() is succedded for texcoord\n");
        fflush(gpFile);
    }
    
    
    vkResult = vkBindBufferMemory(vkDevice, vertexData_texcoord_mars.vkBuffer, vertexData_texcoord_mars.vkDeviceMemory, 0);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "createVertexBuffer() --> vkBindBufferMemory() is failed for texcoord and error code is %d\n", vkResult);
        fflush(gpFile);
        return vkResult;
    }
    else
    {
        fprintf(gpFile, "createVertexBuffer() --> vkBindBufferMemory() is succedded for texcoord\n");
        fflush(gpFile);
    }
    
    
    data = NULL;
    vkResult = vkMapMemory(vkDevice, vertexData_texcoord_mars.vkDeviceMemory, 0, vkMemoryAllocateInfo.allocationSize, 0, &data);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "createVertexBuffer() --> vkMapMemory() is failed for texcoord and error code is %d\n", vkResult);
        fflush(gpFile);
        return vkResult;
    }
    else
    {
        fprintf(gpFile, "createVertexBuffer() --> vkMapMemory() is succedded for texcoord\n");
        fflush(gpFile);
    }
   
    
    memcpy(data, sphere_textures, sizeof(sphere_textures));
    
    vkUnmapMemory(vkDevice, vertexData_texcoord_mars.vkDeviceMemory);
   
   
    
    
   
    
    
    memset((void*)&vertexData_normal_mars, 0, sizeof(VertexData));
    
    
    memset((void*)&vkBufferCreateInfo , 0, sizeof(VkBufferCreateInfo));
    vkBufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    vkBufferCreateInfo.pNext = NULL;
    
    vkBufferCreateInfo.flags = 0;
    vkBufferCreateInfo.size = sizeof(sphere_normals);
    vkBufferCreateInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
    
    
    vkResult = vkCreateBuffer(vkDevice, 
                              &vkBufferCreateInfo,
                              NULL,
                              &vertexData_normal_mars.vkBuffer);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "createVertexBuffer() --> vkCreateBuffer() is failed for normal and error code is %d\n", vkResult);
        fflush(gpFile);
        return vkResult;
    }
    else
    {
        fprintf(gpFile, "createVertexBuffer() --> vkCreateBuffer() is succedded for normal\n");
        fflush(gpFile);
    }
    
    memset((void*)&vkMemoryRequirements, 0, sizeof(VkMemoryRequirements));
   
    vkGetBufferMemoryRequirements(vkDevice, vertexData_normal_mars.vkBuffer, &vkMemoryRequirements);
    
    
    memset((void*)&vkMemoryAllocateInfo , 0, sizeof(VkMemoryAllocateInfo));
    vkMemoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    vkMemoryAllocateInfo.pNext = NULL; 
    vkMemoryAllocateInfo.allocationSize = vkMemoryRequirements.size;
    
    vkMemoryAllocateInfo.memoryTypeIndex = 0;
   
    for(int i = 0; i < vkPhysicalDeviceMemoryProperties.memoryTypeCount; i++)
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
    
    
    vkResult = vkAllocateMemory(vkDevice, &vkMemoryAllocateInfo, NULL, &vertexData_normal_mars.vkDeviceMemory);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "createVertexBuffer() --> vkAllocateMemory() is failed for normal and error code is %d\n", vkResult);
        fflush(gpFile);
        return vkResult;
    }
    else
    {
        fprintf(gpFile, "createVertexBuffer() --> vkAllocateMemory() is succedded for normal\n");
        fflush(gpFile);
    }
    
    
    vkResult = vkBindBufferMemory(vkDevice, vertexData_normal_mars.vkBuffer, vertexData_normal_mars.vkDeviceMemory, 0);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "createVertexBuffer() --> vkBindBufferMemory() is failed for normal and error code is %d\n", vkResult);
        fflush(gpFile);
        return vkResult;
    }
    else
    {
        fprintf(gpFile, "createVertexBuffer() --> vkBindBufferMemory() is succedded for normal\n");
        fflush(gpFile);
    }
    
    
    data = NULL;
    vkResult = vkMapMemory(vkDevice, vertexData_normal_mars.vkDeviceMemory, 0, vkMemoryAllocateInfo.allocationSize, 0, &data);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "createVertexBuffer() --> vkMapMemory() is failed for normal and error code is %d\n", vkResult);
        fflush(gpFile);
        return vkResult;
    }
    else
    {
        fprintf(gpFile, "createVertexBuffer() --> vkMapMemory() is succedded for normal\n");
        fflush(gpFile);
    }
   
    
    memcpy(data, sphere_normals, sizeof(sphere_normals));
    
    vkUnmapMemory(vkDevice, vertexData_normal_mars.vkDeviceMemory);
   
   
    
    
    memset((void*)&vertexData_position_jupitor, 0, sizeof(VertexData));
    
    
    memset((void*)&vkBufferCreateInfo , 0, sizeof(VkBufferCreateInfo));
    vkBufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    vkBufferCreateInfo.pNext = NULL;
    
    vkBufferCreateInfo.flags = 0;
    vkBufferCreateInfo.size = sizeof(sphere_vertices);
    vkBufferCreateInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
    
    
    vkResult = vkCreateBuffer(vkDevice, 
                              &vkBufferCreateInfo,
                              NULL,
                              &vertexData_position_jupitor.vkBuffer);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "createVertexBuffer() --> vkCreateBuffer() is failed for position and error code is %d\n", vkResult);
        fflush(gpFile);
        return vkResult;
    }
    else
    {
        fprintf(gpFile, "createVertexBuffer() --> vkCreateBuffer() is succedded for position \n");
        fflush(gpFile);
    }
    
    memset((void*)&vkMemoryRequirements, 0, sizeof(VkMemoryRequirements));
   
    vkGetBufferMemoryRequirements(vkDevice, vertexData_position_jupitor.vkBuffer, &vkMemoryRequirements);
    
    
    memset((void*)&vkMemoryAllocateInfo , 0, sizeof(VkMemoryAllocateInfo));
    vkMemoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    vkMemoryAllocateInfo.pNext = NULL; 
    vkMemoryAllocateInfo.allocationSize = vkMemoryRequirements.size;
    
    vkMemoryAllocateInfo.memoryTypeIndex = 0;
   
    for(int i = 0; i < vkPhysicalDeviceMemoryProperties.memoryTypeCount; i++)
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
    
    
    vkResult = vkAllocateMemory(vkDevice, &vkMemoryAllocateInfo, NULL, &vertexData_position_jupitor.vkDeviceMemory);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "createVertexBuffer() --> vkAllocateMemory() is failed for position and error code is %d\n", vkResult);
        fflush(gpFile);
        return vkResult;
    }
    else
    {
        fprintf(gpFile, "createVertexBuffer() --> vkAllocateMemory() is succedded for position \n");
        fflush(gpFile);
    }
    
    
    vkResult = vkBindBufferMemory(vkDevice, vertexData_position_jupitor.vkBuffer, vertexData_position_jupitor.vkDeviceMemory, 0);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "createVertexBuffer() --> vkBindBufferMemory() is failed for position and error code is %d\n", vkResult);
        fflush(gpFile);
        return vkResult;
    }
    else
    {
        fprintf(gpFile, "createVertexBuffer() --> vkBindBufferMemory() is succedded for position \n");
        fflush(gpFile);
    }
    
    
    data = NULL;
    vkResult = vkMapMemory(vkDevice, vertexData_position_jupitor.vkDeviceMemory, 0, vkMemoryAllocateInfo.allocationSize, 0, &data);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "createVertexBuffer() --> vkMapMemory() is failed for position and error code is %d\n", vkResult);
        fflush(gpFile);
        return vkResult;
    }
    else
    {
        fprintf(gpFile, "createVertexBuffer() --> vkMapMemory() is succedded for position \n");
        fflush(gpFile);
    }
   
    
    memcpy(data, sphere_vertices, sizeof(sphere_vertices));
    
    vkUnmapMemory(vkDevice, vertexData_position_jupitor.vkDeviceMemory);
   
    
    
    memset((void*)&vertexData_texcoord_jupitor, 0, sizeof(VertexData));
    
    
    memset((void*)&vkBufferCreateInfo , 0, sizeof(VkBufferCreateInfo));
    vkBufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    vkBufferCreateInfo.pNext = NULL;
    
    vkBufferCreateInfo.flags = 0;
    vkBufferCreateInfo.size = sizeof(sphere_textures);
    vkBufferCreateInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
    
    
    vkResult = vkCreateBuffer(vkDevice, 
                              &vkBufferCreateInfo,
                              NULL,
                              &vertexData_texcoord_jupitor.vkBuffer);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "createVertexBuffer() --> vkCreateBuffer() is failed for texcoord and error code is %d\n", vkResult);
        fflush(gpFile);
        return vkResult;
    }
    else
    {
        fprintf(gpFile, "createVertexBuffer() --> vkCreateBuffer() is succedded for texcoord\n");
        fflush(gpFile);
    }
    
    memset((void*)&vkMemoryRequirements, 0, sizeof(VkMemoryRequirements));
   
    vkGetBufferMemoryRequirements(vkDevice, vertexData_texcoord_jupitor.vkBuffer, &vkMemoryRequirements);
    
    
    memset((void*)&vkMemoryAllocateInfo , 0, sizeof(VkMemoryAllocateInfo));
    vkMemoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    vkMemoryAllocateInfo.pNext = NULL; 
    vkMemoryAllocateInfo.allocationSize = vkMemoryRequirements.size;
    
    vkMemoryAllocateInfo.memoryTypeIndex = 0;
   
    for(int i = 0; i < vkPhysicalDeviceMemoryProperties.memoryTypeCount; i++)
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
    
    
    vkResult = vkAllocateMemory(vkDevice, &vkMemoryAllocateInfo, NULL, &vertexData_texcoord_jupitor.vkDeviceMemory);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "createVertexBuffer() --> vkAllocateMemory() is failed for texcoord and error code is %d\n", vkResult);
        fflush(gpFile);
        return vkResult;
    }
    else
    {
        fprintf(gpFile, "createVertexBuffer() --> vkAllocateMemory() is succedded for texcoord\n");
        fflush(gpFile);
    }
    
    
    vkResult = vkBindBufferMemory(vkDevice, vertexData_texcoord_jupitor.vkBuffer, vertexData_texcoord_jupitor.vkDeviceMemory, 0);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "createVertexBuffer() --> vkBindBufferMemory() is failed for texcoord and error code is %d\n", vkResult);
        fflush(gpFile);
        return vkResult;
    }
    else
    {
        fprintf(gpFile, "createVertexBuffer() --> vkBindBufferMemory() is succedded for texcoord\n");
        fflush(gpFile);
    }
    
    
    data = NULL;
    vkResult = vkMapMemory(vkDevice, vertexData_texcoord_jupitor.vkDeviceMemory, 0, vkMemoryAllocateInfo.allocationSize, 0, &data);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "createVertexBuffer() --> vkMapMemory() is failed for texcoord and error code is %d\n", vkResult);
        fflush(gpFile);
        return vkResult;
    }
    else
    {
        fprintf(gpFile, "createVertexBuffer() --> vkMapMemory() is succedded for texcoord\n");
        fflush(gpFile);
    }
   
    
    memcpy(data, sphere_textures, sizeof(sphere_textures));
    
    vkUnmapMemory(vkDevice, vertexData_texcoord_jupitor.vkDeviceMemory);
   
   
    
    
   
    
    
    memset((void*)&vertexData_normal_jupitor, 0, sizeof(VertexData));
    
    
    memset((void*)&vkBufferCreateInfo , 0, sizeof(VkBufferCreateInfo));
    vkBufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    vkBufferCreateInfo.pNext = NULL;
    
    vkBufferCreateInfo.flags = 0;
    vkBufferCreateInfo.size = sizeof(sphere_normals);
    vkBufferCreateInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
    
    
    vkResult = vkCreateBuffer(vkDevice, 
                              &vkBufferCreateInfo,
                              NULL,
                              &vertexData_normal_jupitor.vkBuffer);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "createVertexBuffer() --> vkCreateBuffer() is failed for normal and error code is %d\n", vkResult);
        fflush(gpFile);
        return vkResult;
    }
    else
    {
        fprintf(gpFile, "createVertexBuffer() --> vkCreateBuffer() is succedded for normal\n");
        fflush(gpFile);
    }
    
    memset((void*)&vkMemoryRequirements, 0, sizeof(VkMemoryRequirements));
   
    vkGetBufferMemoryRequirements(vkDevice, vertexData_normal_jupitor.vkBuffer, &vkMemoryRequirements);
    
    
    memset((void*)&vkMemoryAllocateInfo , 0, sizeof(VkMemoryAllocateInfo));
    vkMemoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    vkMemoryAllocateInfo.pNext = NULL; 
    vkMemoryAllocateInfo.allocationSize = vkMemoryRequirements.size;
    
    vkMemoryAllocateInfo.memoryTypeIndex = 0;
   
    for(int i = 0; i < vkPhysicalDeviceMemoryProperties.memoryTypeCount; i++)
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
    
    
    vkResult = vkAllocateMemory(vkDevice, &vkMemoryAllocateInfo, NULL, &vertexData_normal_jupitor.vkDeviceMemory);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "createVertexBuffer() --> vkAllocateMemory() is failed for normal and error code is %d\n", vkResult);
        fflush(gpFile);
        return vkResult;
    }
    else
    {
        fprintf(gpFile, "createVertexBuffer() --> vkAllocateMemory() is succedded for normal\n");
        fflush(gpFile);
    }
    
    
    vkResult = vkBindBufferMemory(vkDevice, vertexData_normal_jupitor.vkBuffer, vertexData_normal_jupitor.vkDeviceMemory, 0);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "createVertexBuffer() --> vkBindBufferMemory() is failed for normal and error code is %d\n", vkResult);
        fflush(gpFile);
        return vkResult;
    }
    else
    {
        fprintf(gpFile, "createVertexBuffer() --> vkBindBufferMemory() is succedded for normal\n");
        fflush(gpFile);
    }
    
    
    data = NULL;
    vkResult = vkMapMemory(vkDevice, vertexData_normal_jupitor.vkDeviceMemory, 0, vkMemoryAllocateInfo.allocationSize, 0, &data);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "createVertexBuffer() --> vkMapMemory() is failed for normal and error code is %d\n", vkResult);
        fflush(gpFile);
        return vkResult;
    }
    else
    {
        fprintf(gpFile, "createVertexBuffer() --> vkMapMemory() is succedded for normal\n");
        fflush(gpFile);
    }
   
    
    memcpy(data, sphere_normals, sizeof(sphere_normals));
    
    vkUnmapMemory(vkDevice, vertexData_normal_jupitor.vkDeviceMemory);
   
    
    
    memset((void*)&vertexData_position_saturn, 0, sizeof(VertexData));
    
    
    memset((void*)&vkBufferCreateInfo , 0, sizeof(VkBufferCreateInfo));
    vkBufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    vkBufferCreateInfo.pNext = NULL;
    
    vkBufferCreateInfo.flags = 0;
    vkBufferCreateInfo.size = sizeof(sphere_vertices);
    vkBufferCreateInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
    
    
    vkResult = vkCreateBuffer(vkDevice, 
                              &vkBufferCreateInfo,
                              NULL,
                              &vertexData_position_saturn.vkBuffer);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "createVertexBuffer() --> vkCreateBuffer() is failed for position and error code is %d\n", vkResult);
        fflush(gpFile);
        return vkResult;
    }
    else
    {
        fprintf(gpFile, "createVertexBuffer() --> vkCreateBuffer() is succedded for position \n");
        fflush(gpFile);
    }
    
    memset((void*)&vkMemoryRequirements, 0, sizeof(VkMemoryRequirements));
   
    vkGetBufferMemoryRequirements(vkDevice, vertexData_position_saturn.vkBuffer, &vkMemoryRequirements);
    
    
    memset((void*)&vkMemoryAllocateInfo , 0, sizeof(VkMemoryAllocateInfo));
    vkMemoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    vkMemoryAllocateInfo.pNext = NULL; 
    vkMemoryAllocateInfo.allocationSize = vkMemoryRequirements.size;
    
    vkMemoryAllocateInfo.memoryTypeIndex = 0;
   
    for(int i = 0; i < vkPhysicalDeviceMemoryProperties.memoryTypeCount; i++)
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
    
    
    vkResult = vkAllocateMemory(vkDevice, &vkMemoryAllocateInfo, NULL, &vertexData_position_saturn.vkDeviceMemory);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "createVertexBuffer() --> vkAllocateMemory() is failed for position and error code is %d\n", vkResult);
        fflush(gpFile);
        return vkResult;
    }
    else
    {
        fprintf(gpFile, "createVertexBuffer() --> vkAllocateMemory() is succedded for position \n");
        fflush(gpFile);
    }
    
    
    vkResult = vkBindBufferMemory(vkDevice, vertexData_position_saturn.vkBuffer, vertexData_position_saturn.vkDeviceMemory, 0);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "createVertexBuffer() --> vkBindBufferMemory() is failed for position and error code is %d\n", vkResult);
        fflush(gpFile);
        return vkResult;
    }
    else
    {
        fprintf(gpFile, "createVertexBuffer() --> vkBindBufferMemory() is succedded for position \n");
        fflush(gpFile);
    }
    
    
    data = NULL;
    vkResult = vkMapMemory(vkDevice, vertexData_position_saturn.vkDeviceMemory, 0, vkMemoryAllocateInfo.allocationSize, 0, &data);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "createVertexBuffer() --> vkMapMemory() is failed for position and error code is %d\n", vkResult);
        fflush(gpFile);
        return vkResult;
    }
    else
    {
        fprintf(gpFile, "createVertexBuffer() --> vkMapMemory() is succedded for position \n");
        fflush(gpFile);
    }
   
    
    memcpy(data, sphere_vertices, sizeof(sphere_vertices));
    
    vkUnmapMemory(vkDevice, vertexData_position_saturn.vkDeviceMemory);
   
    
    
    memset((void*)&vertexData_texcoord_saturn, 0, sizeof(VertexData));
    
    
    memset((void*)&vkBufferCreateInfo , 0, sizeof(VkBufferCreateInfo));
    vkBufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    vkBufferCreateInfo.pNext = NULL;
    
    vkBufferCreateInfo.flags = 0;
    vkBufferCreateInfo.size = sizeof(sphere_textures);
    vkBufferCreateInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
    
    
    vkResult = vkCreateBuffer(vkDevice, 
                              &vkBufferCreateInfo,
                              NULL,
                              &vertexData_texcoord_saturn.vkBuffer);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "createVertexBuffer() --> vkCreateBuffer() is failed for texcoord and error code is %d\n", vkResult);
        fflush(gpFile);
        return vkResult;
    }
    else
    {
        fprintf(gpFile, "createVertexBuffer() --> vkCreateBuffer() is succedded for texcoord\n");
        fflush(gpFile);
    }
    
    memset((void*)&vkMemoryRequirements, 0, sizeof(VkMemoryRequirements));
   
    vkGetBufferMemoryRequirements(vkDevice, vertexData_texcoord_saturn.vkBuffer, &vkMemoryRequirements);
    
    
    memset((void*)&vkMemoryAllocateInfo , 0, sizeof(VkMemoryAllocateInfo));
    vkMemoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    vkMemoryAllocateInfo.pNext = NULL; 
    vkMemoryAllocateInfo.allocationSize = vkMemoryRequirements.size;
    
    vkMemoryAllocateInfo.memoryTypeIndex = 0;
   
    for(int i = 0; i < vkPhysicalDeviceMemoryProperties.memoryTypeCount; i++)
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
    
    
    vkResult = vkAllocateMemory(vkDevice, &vkMemoryAllocateInfo, NULL, &vertexData_texcoord_saturn.vkDeviceMemory);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "createVertexBuffer() --> vkAllocateMemory() is failed for texcoord and error code is %d\n", vkResult);
        fflush(gpFile);
        return vkResult;
    }
    else
    {
        fprintf(gpFile, "createVertexBuffer() --> vkAllocateMemory() is succedded for texcoord\n");
        fflush(gpFile);
    }
    
    
    vkResult = vkBindBufferMemory(vkDevice, vertexData_texcoord_saturn.vkBuffer, vertexData_texcoord_saturn.vkDeviceMemory, 0);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "createVertexBuffer() --> vkBindBufferMemory() is failed for texcoord and error code is %d\n", vkResult);
        fflush(gpFile);
        return vkResult;
    }
    else
    {
        fprintf(gpFile, "createVertexBuffer() --> vkBindBufferMemory() is succedded for texcoord\n");
        fflush(gpFile);
    }
    
    
    data = NULL;
    vkResult = vkMapMemory(vkDevice, vertexData_texcoord_saturn.vkDeviceMemory, 0, vkMemoryAllocateInfo.allocationSize, 0, &data);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "createVertexBuffer() --> vkMapMemory() is failed for texcoord and error code is %d\n", vkResult);
        fflush(gpFile);
        return vkResult;
    }
    else
    {
        fprintf(gpFile, "createVertexBuffer() --> vkMapMemory() is succedded for texcoord\n");
        fflush(gpFile);
    }
   
    
    memcpy(data, sphere_textures, sizeof(sphere_textures));
    
    vkUnmapMemory(vkDevice, vertexData_texcoord_saturn.vkDeviceMemory);
   
   
    
    
   
    
    
    memset((void*)&vertexData_normal_saturn, 0, sizeof(VertexData));
    
    
    memset((void*)&vkBufferCreateInfo , 0, sizeof(VkBufferCreateInfo));
    vkBufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    vkBufferCreateInfo.pNext = NULL;
    
    vkBufferCreateInfo.flags = 0;
    vkBufferCreateInfo.size = sizeof(sphere_normals);
    vkBufferCreateInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
    
    
    vkResult = vkCreateBuffer(vkDevice, 
                              &vkBufferCreateInfo,
                              NULL,
                              &vertexData_normal_saturn.vkBuffer);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "createVertexBuffer() --> vkCreateBuffer() is failed for normal and error code is %d\n", vkResult);
        fflush(gpFile);
        return vkResult;
    }
    else
    {
        fprintf(gpFile, "createVertexBuffer() --> vkCreateBuffer() is succedded for normal\n");
        fflush(gpFile);
    }
    
    memset((void*)&vkMemoryRequirements, 0, sizeof(VkMemoryRequirements));
   
    vkGetBufferMemoryRequirements(vkDevice, vertexData_normal_saturn.vkBuffer, &vkMemoryRequirements);
    
    
    memset((void*)&vkMemoryAllocateInfo , 0, sizeof(VkMemoryAllocateInfo));
    vkMemoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    vkMemoryAllocateInfo.pNext = NULL; 
    vkMemoryAllocateInfo.allocationSize = vkMemoryRequirements.size;
    
    vkMemoryAllocateInfo.memoryTypeIndex = 0;
   
    for(int i = 0; i < vkPhysicalDeviceMemoryProperties.memoryTypeCount; i++)
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
    
    
    vkResult = vkAllocateMemory(vkDevice, &vkMemoryAllocateInfo, NULL, &vertexData_normal_saturn.vkDeviceMemory);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "createVertexBuffer() --> vkAllocateMemory() is failed for normal and error code is %d\n", vkResult);
        fflush(gpFile);
        return vkResult;
    }
    else
    {
        fprintf(gpFile, "createVertexBuffer() --> vkAllocateMemory() is succedded for normal\n");
        fflush(gpFile);
    }
    
    
    vkResult = vkBindBufferMemory(vkDevice, vertexData_normal_saturn.vkBuffer, vertexData_normal_saturn.vkDeviceMemory, 0);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "createVertexBuffer() --> vkBindBufferMemory() is failed for normal and error code is %d\n", vkResult);
        fflush(gpFile);
        return vkResult;
    }
    else
    {
        fprintf(gpFile, "createVertexBuffer() --> vkBindBufferMemory() is succedded for normal\n");
        fflush(gpFile);
    }
    
    
    data = NULL;
    vkResult = vkMapMemory(vkDevice, vertexData_normal_saturn.vkDeviceMemory, 0, vkMemoryAllocateInfo.allocationSize, 0, &data);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "createVertexBuffer() --> vkMapMemory() is failed for normal and error code is %d\n", vkResult);
        fflush(gpFile);
        return vkResult;
    }
    else
    {
        fprintf(gpFile, "createVertexBuffer() --> vkMapMemory() is succedded for normal\n");
        fflush(gpFile);
    }
   
    
    memcpy(data, sphere_normals, sizeof(sphere_normals));
    
    vkUnmapMemory(vkDevice, vertexData_normal_saturn.vkDeviceMemory);
   
    
    
    memset((void*)&vertexData_position_uranus, 0, sizeof(VertexData));
    
    
    memset((void*)&vkBufferCreateInfo , 0, sizeof(VkBufferCreateInfo));
    vkBufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    vkBufferCreateInfo.pNext = NULL;
    
    vkBufferCreateInfo.flags = 0;
    vkBufferCreateInfo.size = sizeof(sphere_vertices);
    vkBufferCreateInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
    
    
    vkResult = vkCreateBuffer(vkDevice, 
                              &vkBufferCreateInfo,
                              NULL,
                              &vertexData_position_uranus.vkBuffer);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "createVertexBuffer() --> vkCreateBuffer() is failed for position and error code is %d\n", vkResult);
        fflush(gpFile);
        return vkResult;
    }
    else
    {
        fprintf(gpFile, "createVertexBuffer() --> vkCreateBuffer() is succedded for position \n");
        fflush(gpFile);
    }
    
    memset((void*)&vkMemoryRequirements, 0, sizeof(VkMemoryRequirements));
   
    vkGetBufferMemoryRequirements(vkDevice, vertexData_position_uranus.vkBuffer, &vkMemoryRequirements);
    
    
    memset((void*)&vkMemoryAllocateInfo , 0, sizeof(VkMemoryAllocateInfo));
    vkMemoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    vkMemoryAllocateInfo.pNext = NULL; 
    vkMemoryAllocateInfo.allocationSize = vkMemoryRequirements.size;
    
    vkMemoryAllocateInfo.memoryTypeIndex = 0;
   
    for(int i = 0; i < vkPhysicalDeviceMemoryProperties.memoryTypeCount; i++)
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
    
    
    vkResult = vkAllocateMemory(vkDevice, &vkMemoryAllocateInfo, NULL, &vertexData_position_uranus.vkDeviceMemory);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "createVertexBuffer() --> vkAllocateMemory() is failed for position and error code is %d\n", vkResult);
        fflush(gpFile);
        return vkResult;
    }
    else
    {
        fprintf(gpFile, "createVertexBuffer() --> vkAllocateMemory() is succedded for position \n");
        fflush(gpFile);
    }
    
    
    vkResult = vkBindBufferMemory(vkDevice, vertexData_position_uranus.vkBuffer, vertexData_position_uranus.vkDeviceMemory, 0);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "createVertexBuffer() --> vkBindBufferMemory() is failed for position and error code is %d\n", vkResult);
        fflush(gpFile);
        return vkResult;
    }
    else
    {
        fprintf(gpFile, "createVertexBuffer() --> vkBindBufferMemory() is succedded for position \n");
        fflush(gpFile);
    }
    
    
    data = NULL;
    vkResult = vkMapMemory(vkDevice, vertexData_position_uranus.vkDeviceMemory, 0, vkMemoryAllocateInfo.allocationSize, 0, &data);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "createVertexBuffer() --> vkMapMemory() is failed for position and error code is %d\n", vkResult);
        fflush(gpFile);
        return vkResult;
    }
    else
    {
        fprintf(gpFile, "createVertexBuffer() --> vkMapMemory() is succedded for position \n");
        fflush(gpFile);
    }
   
    
    memcpy(data, sphere_vertices, sizeof(sphere_vertices));
    
    vkUnmapMemory(vkDevice, vertexData_position_uranus.vkDeviceMemory);
   
    
    
    memset((void*)&vertexData_texcoord_uranus, 0, sizeof(VertexData));
    
    
    memset((void*)&vkBufferCreateInfo , 0, sizeof(VkBufferCreateInfo));
    vkBufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    vkBufferCreateInfo.pNext = NULL;
    
    vkBufferCreateInfo.flags = 0;
    vkBufferCreateInfo.size = sizeof(sphere_textures);
    vkBufferCreateInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
    
    
    vkResult = vkCreateBuffer(vkDevice, 
                              &vkBufferCreateInfo,
                              NULL,
                              &vertexData_texcoord_uranus.vkBuffer);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "createVertexBuffer() --> vkCreateBuffer() is failed for texcoord and error code is %d\n", vkResult);
        fflush(gpFile);
        return vkResult;
    }
    else
    {
        fprintf(gpFile, "createVertexBuffer() --> vkCreateBuffer() is succedded for texcoord\n");
        fflush(gpFile);
    }
    
    memset((void*)&vkMemoryRequirements, 0, sizeof(VkMemoryRequirements));
   
    vkGetBufferMemoryRequirements(vkDevice, vertexData_texcoord_uranus.vkBuffer, &vkMemoryRequirements);
    
    
    memset((void*)&vkMemoryAllocateInfo , 0, sizeof(VkMemoryAllocateInfo));
    vkMemoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    vkMemoryAllocateInfo.pNext = NULL; 
    vkMemoryAllocateInfo.allocationSize = vkMemoryRequirements.size;
    
    vkMemoryAllocateInfo.memoryTypeIndex = 0;
   
    for(int i = 0; i < vkPhysicalDeviceMemoryProperties.memoryTypeCount; i++)
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
    
    
    vkResult = vkAllocateMemory(vkDevice, &vkMemoryAllocateInfo, NULL, &vertexData_texcoord_uranus.vkDeviceMemory);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "createVertexBuffer() --> vkAllocateMemory() is failed for texcoord and error code is %d\n", vkResult);
        fflush(gpFile);
        return vkResult;
    }
    else
    {
        fprintf(gpFile, "createVertexBuffer() --> vkAllocateMemory() is succedded for texcoord\n");
        fflush(gpFile);
    }
    
    
    vkResult = vkBindBufferMemory(vkDevice, vertexData_texcoord_uranus.vkBuffer, vertexData_texcoord_uranus.vkDeviceMemory, 0);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "createVertexBuffer() --> vkBindBufferMemory() is failed for texcoord and error code is %d\n", vkResult);
        fflush(gpFile);
        return vkResult;
    }
    else
    {
        fprintf(gpFile, "createVertexBuffer() --> vkBindBufferMemory() is succedded for texcoord\n");
        fflush(gpFile);
    }
    
    
    data = NULL;
    vkResult = vkMapMemory(vkDevice, vertexData_texcoord_uranus.vkDeviceMemory, 0, vkMemoryAllocateInfo.allocationSize, 0, &data);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "createVertexBuffer() --> vkMapMemory() is failed for texcoord and error code is %d\n", vkResult);
        fflush(gpFile);
        return vkResult;
    }
    else
    {
        fprintf(gpFile, "createVertexBuffer() --> vkMapMemory() is succedded for texcoord\n");
        fflush(gpFile);
    }
   
    
    memcpy(data, sphere_textures, sizeof(sphere_textures));
    
    vkUnmapMemory(vkDevice, vertexData_texcoord_uranus.vkDeviceMemory);
   
   
    
    
   
    
    
    memset((void*)&vertexData_normal_uranus, 0, sizeof(VertexData));
    
    
    memset((void*)&vkBufferCreateInfo , 0, sizeof(VkBufferCreateInfo));
    vkBufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    vkBufferCreateInfo.pNext = NULL;
    
    vkBufferCreateInfo.flags = 0;
    vkBufferCreateInfo.size = sizeof(sphere_normals);
    vkBufferCreateInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
    
    
    vkResult = vkCreateBuffer(vkDevice, 
                              &vkBufferCreateInfo,
                              NULL,
                              &vertexData_normal_uranus.vkBuffer);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "createVertexBuffer() --> vkCreateBuffer() is failed for normal and error code is %d\n", vkResult);
        fflush(gpFile);
        return vkResult;
    }
    else
    {
        fprintf(gpFile, "createVertexBuffer() --> vkCreateBuffer() is succedded for normal\n");
        fflush(gpFile);
    }
    
    memset((void*)&vkMemoryRequirements, 0, sizeof(VkMemoryRequirements));
   
    vkGetBufferMemoryRequirements(vkDevice, vertexData_normal_uranus.vkBuffer, &vkMemoryRequirements);
    
    
    memset((void*)&vkMemoryAllocateInfo , 0, sizeof(VkMemoryAllocateInfo));
    vkMemoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    vkMemoryAllocateInfo.pNext = NULL; 
    vkMemoryAllocateInfo.allocationSize = vkMemoryRequirements.size;
    
    vkMemoryAllocateInfo.memoryTypeIndex = 0;
   
    for(int i = 0; i < vkPhysicalDeviceMemoryProperties.memoryTypeCount; i++)
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
    
    
    vkResult = vkAllocateMemory(vkDevice, &vkMemoryAllocateInfo, NULL, &vertexData_normal_uranus.vkDeviceMemory);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "createVertexBuffer() --> vkAllocateMemory() is failed for normal and error code is %d\n", vkResult);
        fflush(gpFile);
        return vkResult;
    }
    else
    {
        fprintf(gpFile, "createVertexBuffer() --> vkAllocateMemory() is succedded for normal\n");
        fflush(gpFile);
    }
    
    
    vkResult = vkBindBufferMemory(vkDevice, vertexData_normal_uranus.vkBuffer, vertexData_normal_uranus.vkDeviceMemory, 0);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "createVertexBuffer() --> vkBindBufferMemory() is failed for normal and error code is %d\n", vkResult);
        fflush(gpFile);
        return vkResult;
    }
    else
    {
        fprintf(gpFile, "createVertexBuffer() --> vkBindBufferMemory() is succedded for normal\n");
        fflush(gpFile);
    }
    
    
    data = NULL;
    vkResult = vkMapMemory(vkDevice, vertexData_normal_uranus.vkDeviceMemory, 0, vkMemoryAllocateInfo.allocationSize, 0, &data);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "createVertexBuffer() --> vkMapMemory() is failed for normal and error code is %d\n", vkResult);
        fflush(gpFile);
        return vkResult;
    }
    else
    {
        fprintf(gpFile, "createVertexBuffer() --> vkMapMemory() is succedded for normal\n");
        fflush(gpFile);
    }
   
    
    memcpy(data, sphere_normals, sizeof(sphere_normals));
    
    vkUnmapMemory(vkDevice, vertexData_normal_uranus.vkDeviceMemory);
   
    
    
    memset((void*)&vertexData_position_neptune, 0, sizeof(VertexData));
    
    
    memset((void*)&vkBufferCreateInfo , 0, sizeof(VkBufferCreateInfo));
    vkBufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    vkBufferCreateInfo.pNext = NULL;
    
    vkBufferCreateInfo.flags = 0;
    vkBufferCreateInfo.size = sizeof(sphere_vertices);
    vkBufferCreateInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
    
    
    vkResult = vkCreateBuffer(vkDevice, 
                              &vkBufferCreateInfo,
                              NULL,
                              &vertexData_position_neptune.vkBuffer);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "createVertexBuffer() --> vkCreateBuffer() is failed for position and error code is %d\n", vkResult);
        fflush(gpFile);
        return vkResult;
    }
    else
    {
        fprintf(gpFile, "createVertexBuffer() --> vkCreateBuffer() is succedded for position \n");
        fflush(gpFile);
    }
    
    memset((void*)&vkMemoryRequirements, 0, sizeof(VkMemoryRequirements));
   
    vkGetBufferMemoryRequirements(vkDevice, vertexData_position_neptune.vkBuffer, &vkMemoryRequirements);
    
    
    memset((void*)&vkMemoryAllocateInfo , 0, sizeof(VkMemoryAllocateInfo));
    vkMemoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    vkMemoryAllocateInfo.pNext = NULL; 
    vkMemoryAllocateInfo.allocationSize = vkMemoryRequirements.size;
    
    vkMemoryAllocateInfo.memoryTypeIndex = 0;
   
    for(int i = 0; i < vkPhysicalDeviceMemoryProperties.memoryTypeCount; i++)
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
    
    
    vkResult = vkAllocateMemory(vkDevice, &vkMemoryAllocateInfo, NULL, &vertexData_position_neptune.vkDeviceMemory);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "createVertexBuffer() --> vkAllocateMemory() is failed for position and error code is %d\n", vkResult);
        fflush(gpFile);
        return vkResult;
    }
    else
    {
        fprintf(gpFile, "createVertexBuffer() --> vkAllocateMemory() is succedded for position \n");
        fflush(gpFile);
    }
    
    
    vkResult = vkBindBufferMemory(vkDevice, vertexData_position_neptune.vkBuffer, vertexData_position_neptune.vkDeviceMemory, 0);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "createVertexBuffer() --> vkBindBufferMemory() is failed for position and error code is %d\n", vkResult);
        fflush(gpFile);
        return vkResult;
    }
    else
    {
        fprintf(gpFile, "createVertexBuffer() --> vkBindBufferMemory() is succedded for position \n");
        fflush(gpFile);
    }
    
    
    data = NULL;
    vkResult = vkMapMemory(vkDevice, vertexData_position_neptune.vkDeviceMemory, 0, vkMemoryAllocateInfo.allocationSize, 0, &data);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "createVertexBuffer() --> vkMapMemory() is failed for position and error code is %d\n", vkResult);
        fflush(gpFile);
        return vkResult;
    }
    else
    {
        fprintf(gpFile, "createVertexBuffer() --> vkMapMemory() is succedded for position \n");
        fflush(gpFile);
    }
   
    
    memcpy(data, sphere_vertices, sizeof(sphere_vertices));
    
    vkUnmapMemory(vkDevice, vertexData_position_neptune.vkDeviceMemory);
   
    
    
    memset((void*)&vertexData_texcoord_neptune, 0, sizeof(VertexData));
    
    
    memset((void*)&vkBufferCreateInfo , 0, sizeof(VkBufferCreateInfo));
    vkBufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    vkBufferCreateInfo.pNext = NULL;
    
    vkBufferCreateInfo.flags = 0;
    vkBufferCreateInfo.size = sizeof(sphere_textures);
    vkBufferCreateInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
    
    
    vkResult = vkCreateBuffer(vkDevice, 
                              &vkBufferCreateInfo,
                              NULL,
                              &vertexData_texcoord_neptune.vkBuffer);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "createVertexBuffer() --> vkCreateBuffer() is failed for texcoord and error code is %d\n", vkResult);
        fflush(gpFile);
        return vkResult;
    }
    else
    {
        fprintf(gpFile, "createVertexBuffer() --> vkCreateBuffer() is succedded for texcoord\n");
        fflush(gpFile);
    }
    
    memset((void*)&vkMemoryRequirements, 0, sizeof(VkMemoryRequirements));
   
    vkGetBufferMemoryRequirements(vkDevice, vertexData_texcoord_neptune.vkBuffer, &vkMemoryRequirements);
    
    
    memset((void*)&vkMemoryAllocateInfo , 0, sizeof(VkMemoryAllocateInfo));
    vkMemoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    vkMemoryAllocateInfo.pNext = NULL; 
    vkMemoryAllocateInfo.allocationSize = vkMemoryRequirements.size;
    
    vkMemoryAllocateInfo.memoryTypeIndex = 0;
   
    for(int i = 0; i < vkPhysicalDeviceMemoryProperties.memoryTypeCount; i++)
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
    
    
    vkResult = vkAllocateMemory(vkDevice, &vkMemoryAllocateInfo, NULL, &vertexData_texcoord_neptune.vkDeviceMemory);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "createVertexBuffer() --> vkAllocateMemory() is failed for texcoord and error code is %d\n", vkResult);
        fflush(gpFile);
        return vkResult;
    }
    else
    {
        fprintf(gpFile, "createVertexBuffer() --> vkAllocateMemory() is succedded for texcoord\n");
        fflush(gpFile);
    }
    
    
    vkResult = vkBindBufferMemory(vkDevice, vertexData_texcoord_neptune.vkBuffer, vertexData_texcoord_neptune.vkDeviceMemory, 0);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "createVertexBuffer() --> vkBindBufferMemory() is failed for texcoord and error code is %d\n", vkResult);
        fflush(gpFile);
        return vkResult;
    }
    else
    {
        fprintf(gpFile, "createVertexBuffer() --> vkBindBufferMemory() is succedded for texcoord\n");
        fflush(gpFile);
    }
    
    
    data = NULL;
    vkResult = vkMapMemory(vkDevice, vertexData_texcoord_neptune.vkDeviceMemory, 0, vkMemoryAllocateInfo.allocationSize, 0, &data);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "createVertexBuffer() --> vkMapMemory() is failed for texcoord and error code is %d\n", vkResult);
        fflush(gpFile);
        return vkResult;
    }
    else
    {
        fprintf(gpFile, "createVertexBuffer() --> vkMapMemory() is succedded for texcoord\n");
        fflush(gpFile);
    }
   
    
    memcpy(data, sphere_textures, sizeof(sphere_textures));
    
    vkUnmapMemory(vkDevice, vertexData_texcoord_neptune.vkDeviceMemory);
   
   
    
    
   
    
    
    memset((void*)&vertexData_normal_neptune, 0, sizeof(VertexData));
    
    
    memset((void*)&vkBufferCreateInfo , 0, sizeof(VkBufferCreateInfo));
    vkBufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    vkBufferCreateInfo.pNext = NULL;
    
    vkBufferCreateInfo.flags = 0;
    vkBufferCreateInfo.size = sizeof(sphere_normals);
    vkBufferCreateInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
    
    
    vkResult = vkCreateBuffer(vkDevice, 
                              &vkBufferCreateInfo,
                              NULL,
                              &vertexData_normal_neptune.vkBuffer);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "createVertexBuffer() --> vkCreateBuffer() is failed for normal and error code is %d\n", vkResult);
        fflush(gpFile);
        return vkResult;
    }
    else
    {
        fprintf(gpFile, "createVertexBuffer() --> vkCreateBuffer() is succedded for normal\n");
        fflush(gpFile);
    }
    
    memset((void*)&vkMemoryRequirements, 0, sizeof(VkMemoryRequirements));
   
    vkGetBufferMemoryRequirements(vkDevice, vertexData_normal_neptune.vkBuffer, &vkMemoryRequirements);
    
    
    memset((void*)&vkMemoryAllocateInfo , 0, sizeof(VkMemoryAllocateInfo));
    vkMemoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    vkMemoryAllocateInfo.pNext = NULL; 
    vkMemoryAllocateInfo.allocationSize = vkMemoryRequirements.size;
    
    vkMemoryAllocateInfo.memoryTypeIndex = 0;
   
    for(int i = 0; i < vkPhysicalDeviceMemoryProperties.memoryTypeCount; i++)
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
    
    
    vkResult = vkAllocateMemory(vkDevice, &vkMemoryAllocateInfo, NULL, &vertexData_normal_neptune.vkDeviceMemory);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "createVertexBuffer() --> vkAllocateMemory() is failed for normal and error code is %d\n", vkResult);
        fflush(gpFile);
        return vkResult;
    }
    else
    {
        fprintf(gpFile, "createVertexBuffer() --> vkAllocateMemory() is succedded for normal\n");
        fflush(gpFile);
    }
    
    
    vkResult = vkBindBufferMemory(vkDevice, vertexData_normal_neptune.vkBuffer, vertexData_normal_neptune.vkDeviceMemory, 0);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "createVertexBuffer() --> vkBindBufferMemory() is failed for normal and error code is %d\n", vkResult);
        fflush(gpFile);
        return vkResult;
    }
    else
    {
        fprintf(gpFile, "createVertexBuffer() --> vkBindBufferMemory() is succedded for normal\n");
        fflush(gpFile);
    }
    
    
    data = NULL;
    vkResult = vkMapMemory(vkDevice, vertexData_normal_neptune.vkDeviceMemory, 0, vkMemoryAllocateInfo.allocationSize, 0, &data);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "createVertexBuffer() --> vkMapMemory() is failed for normal and error code is %d\n", vkResult);
        fflush(gpFile);
        return vkResult;
    }
    else
    {
        fprintf(gpFile, "createVertexBuffer() --> vkMapMemory() is succedded for normal\n");
        fflush(gpFile);
    }
   
    
    memcpy(data, sphere_normals, sizeof(sphere_normals));
    
    vkUnmapMemory(vkDevice, vertexData_normal_neptune.vkDeviceMemory);
   
    return (vkResult);
}

VkResult createIndexBuffer(void)
{
    
    VkResult vkResult = VK_SUCCESS;
    
    
    
    memset((void*)&vertexData_index_sun, 0, sizeof(VertexData));
    
    
    VkBufferCreateInfo vkBufferCreateInfo;
    memset((void*)&vkBufferCreateInfo , 0, sizeof(VkBufferCreateInfo));
    vkBufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    vkBufferCreateInfo.pNext = NULL;
    
    vkBufferCreateInfo.flags = 0;
    vkBufferCreateInfo.size = sizeof(sphere_elements);
    vkBufferCreateInfo.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
    
    
    vkResult = vkCreateBuffer(vkDevice, 
                              &vkBufferCreateInfo,
                              NULL,
                              &vertexData_index_sun.vkBuffer);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "createIndexBuffer() --> vkCreateBuffer() of index buffer is failed and error code is %d\n", vkResult);
        fflush(gpFile);
        return vkResult;
    }
    else
    {
        fprintf(gpFile, "createIndexBuffer() --> vkCreateBuffer() of index buffer is succedded\n");
        fflush(gpFile);
    }
    
    VkMemoryRequirements vkMemoryRequirements;
    memset((void*)&vkMemoryRequirements, 0, sizeof(VkMemoryRequirements));
   
    vkGetBufferMemoryRequirements(vkDevice, vertexData_index_sun.vkBuffer, &vkMemoryRequirements);
    
    
    VkMemoryAllocateInfo vkMemoryAllocateInfo;
    memset((void*)&vkMemoryAllocateInfo , 0, sizeof(VkMemoryAllocateInfo));
    vkMemoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    vkMemoryAllocateInfo.pNext = NULL; 
    vkMemoryAllocateInfo.allocationSize = vkMemoryRequirements.size;
    
    vkMemoryAllocateInfo.memoryTypeIndex = 0;
   
    for(int i = 0; i < vkPhysicalDeviceMemoryProperties.memoryTypeCount; i++)
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
    
    
    vkResult = vkAllocateMemory(vkDevice, &vkMemoryAllocateInfo, NULL, &vertexData_index_sun.vkDeviceMemory);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "createIndexBuffer() --> vkAllocateMemory() is failed for rindex buffer and error code is %d\n", vkResult);
        fflush(gpFile);
        return vkResult;
    }
    else
    {
        fprintf(gpFile, "createIndexBuffer() --> vkAllocateMemory() is succedded for index buffer\n");
        fflush(gpFile);
    }
    
    
    vkResult = vkBindBufferMemory(vkDevice, vertexData_index_sun.vkBuffer, vertexData_index_sun.vkDeviceMemory, 0);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "createIndexBuffer() --> vkBindBufferMemory() is failed for index buffer and error code is %d\n", vkResult);
        fflush(gpFile);
        return vkResult;
    }
    else
    {
        fprintf(gpFile, "createIndexBuffer() --> vkBindBufferMemory() is succedded for index buffer\n");
        fflush(gpFile);
    }
    
    
    void* data = NULL;
    vkResult = vkMapMemory(vkDevice, vertexData_index_sun.vkDeviceMemory, 0, vkMemoryAllocateInfo.allocationSize, 0, &data);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "createIndexBuffer() --> vkMapMemory() is failed for index buffer and error code is %d\n", vkResult);
        fflush(gpFile);
        return vkResult;
    }
    else
    {
        fprintf(gpFile, "createIndexBuffer() --> vkMapMemory() is succedded for index buffer\n");
        fflush(gpFile);
    }
   
    
    memcpy(data, sphere_elements, sizeof(sphere_elements));
    
    vkUnmapMemory(vkDevice, vertexData_index_sun.vkDeviceMemory);
    
    
    
    
    
    
    memset((void*)&vertexData_index_mercury, 0, sizeof(VertexData));
    
    
    memset((void*)&vkBufferCreateInfo , 0, sizeof(VkBufferCreateInfo));
    vkBufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    vkBufferCreateInfo.pNext = NULL;
    
    vkBufferCreateInfo.flags = 0;
    vkBufferCreateInfo.size = sizeof(sphere_elements);
    vkBufferCreateInfo.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
    
    
    vkResult = vkCreateBuffer(vkDevice, 
                              &vkBufferCreateInfo,
                              NULL,
                              &vertexData_index_mercury.vkBuffer);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "createIndexBuffer() --> vkCreateBuffer() of index buffer is failed and error code is %d\n", vkResult);
        fflush(gpFile);
        return vkResult;
    }
    else
    {
        fprintf(gpFile, "createIndexBuffer() --> vkCreateBuffer() of index buffer is succedded\n");
        fflush(gpFile);
    }
    

    memset((void*)&vkMemoryRequirements, 0, sizeof(VkMemoryRequirements));
   
    vkGetBufferMemoryRequirements(vkDevice, vertexData_index_mercury.vkBuffer, &vkMemoryRequirements);
    
    
    memset((void*)&vkMemoryAllocateInfo , 0, sizeof(VkMemoryAllocateInfo));
    vkMemoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    vkMemoryAllocateInfo.pNext = NULL; 
    vkMemoryAllocateInfo.allocationSize = vkMemoryRequirements.size;
    
    vkMemoryAllocateInfo.memoryTypeIndex = 0;
   
    for(int i = 0; i < vkPhysicalDeviceMemoryProperties.memoryTypeCount; i++)
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
    
    
    vkResult = vkAllocateMemory(vkDevice, &vkMemoryAllocateInfo, NULL, &vertexData_index_mercury.vkDeviceMemory);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "createIndexBuffer() --> vkAllocateMemory() is failed for rindex buffer and error code is %d\n", vkResult);
        fflush(gpFile);
        return vkResult;
    }
    else
    {
        fprintf(gpFile, "createIndexBuffer() --> vkAllocateMemory() is succedded for index buffer\n");
    }
    
    
    vkResult = vkBindBufferMemory(vkDevice, vertexData_index_mercury.vkBuffer, vertexData_index_mercury.vkDeviceMemory, 0);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "createIndexBuffer() --> vkBindBufferMemory() is failed for index buffer and error code is %d\n", vkResult);
        fflush(gpFile);
        return vkResult;
    }
    else
    {
        fprintf(gpFile, "createIndexBuffer() --> vkBindBufferMemory() is succedded for index buffer\n");
    }
    
    
    data = NULL;
    vkResult = vkMapMemory(vkDevice, vertexData_index_mercury.vkDeviceMemory, 0, vkMemoryAllocateInfo.allocationSize, 0, &data);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "createIndexBuffer() --> vkMapMemory() is failed for index buffer and error code is %d\n", vkResult);
        fflush(gpFile);
        return vkResult;
    }
    else
    {
        fprintf(gpFile, "createIndexBuffer() --> vkMapMemory() is succedded for index buffer\n");
    }
   
    
    memcpy(data, sphere_elements, sizeof(sphere_elements));
    
    vkUnmapMemory(vkDevice, vertexData_index_mercury.vkDeviceMemory);
    
    
    
    
    
    
    memset((void*)&vertexData_index_venus, 0, sizeof(VertexData));
    
    
    memset((void*)&vkBufferCreateInfo , 0, sizeof(VkBufferCreateInfo));
    vkBufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    vkBufferCreateInfo.pNext = NULL;
    
    vkBufferCreateInfo.flags = 0;
    vkBufferCreateInfo.size = sizeof(sphere_elements);
    vkBufferCreateInfo.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
    
    
    vkResult = vkCreateBuffer(vkDevice, 
                              &vkBufferCreateInfo,
                              NULL,
                              &vertexData_index_venus.vkBuffer);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "createIndexBuffer() --> vkCreateBuffer() of index buffer is failed and error code is %d\n", vkResult);
        fflush(gpFile);
        return vkResult;
    }
    else
    {
        fprintf(gpFile, "createIndexBuffer() --> vkCreateBuffer() of index buffer is succedded\n");
    }
    

    memset((void*)&vkMemoryRequirements, 0, sizeof(VkMemoryRequirements));
   
    vkGetBufferMemoryRequirements(vkDevice, vertexData_index_venus.vkBuffer, &vkMemoryRequirements);
    
    
    memset((void*)&vkMemoryAllocateInfo , 0, sizeof(VkMemoryAllocateInfo));
    vkMemoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    vkMemoryAllocateInfo.pNext = NULL; 
    vkMemoryAllocateInfo.allocationSize = vkMemoryRequirements.size;
    
    vkMemoryAllocateInfo.memoryTypeIndex = 0;
   
    for(int i = 0; i < vkPhysicalDeviceMemoryProperties.memoryTypeCount; i++)
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
    
    
    vkResult = vkAllocateMemory(vkDevice, &vkMemoryAllocateInfo, NULL, &vertexData_index_venus.vkDeviceMemory);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "createIndexBuffer() --> vkAllocateMemory() is failed for rindex buffer and error code is %d\n", vkResult);
        fflush(gpFile);
        return vkResult;
    }
    else
    {
        fprintf(gpFile, "createIndexBuffer() --> vkAllocateMemory() is succedded for index buffer\n");
    }
    
    
    vkResult = vkBindBufferMemory(vkDevice, vertexData_index_venus.vkBuffer, vertexData_index_venus.vkDeviceMemory, 0);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "createIndexBuffer() --> vkBindBufferMemory() is failed for index buffer and error code is %d\n", vkResult);
        fflush(gpFile);
        return vkResult;
    }
    else
    {
        fprintf(gpFile, "createIndexBuffer() --> vkBindBufferMemory() is succedded for index buffer\n");
    }
    
    
    data = NULL;
    vkResult = vkMapMemory(vkDevice, vertexData_index_venus.vkDeviceMemory, 0, vkMemoryAllocateInfo.allocationSize, 0, &data);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "createIndexBuffer() --> vkMapMemory() is failed for index buffer and error code is %d\n", vkResult);
        fflush(gpFile);
        return vkResult;
    }
    else
    {
        fprintf(gpFile, "createIndexBuffer() --> vkMapMemory() is succedded for index buffer\n");
        fflush(gpFile);
    }
   
    
    memcpy(data, sphere_elements, sizeof(sphere_elements));
    
    vkUnmapMemory(vkDevice, vertexData_index_venus.vkDeviceMemory);
    
    
    
    
    
    memset((void*)&vertexData_index_earth, 0, sizeof(VertexData));
    
    
    memset((void*)&vkBufferCreateInfo , 0, sizeof(VkBufferCreateInfo));
    vkBufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    vkBufferCreateInfo.pNext = NULL;
    
    vkBufferCreateInfo.flags = 0;
    vkBufferCreateInfo.size = sizeof(sphere_elements);
    vkBufferCreateInfo.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
    
    
    vkResult = vkCreateBuffer(vkDevice, 
                              &vkBufferCreateInfo,
                              NULL,
                              &vertexData_index_earth.vkBuffer);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "createIndexBuffer() --> vkCreateBuffer() of index buffer is failed and error code is %d\n", vkResult);
        fflush(gpFile);
        return vkResult;
    }
    else
    {
        fprintf(gpFile, "createIndexBuffer() --> vkCreateBuffer() of index buffer is succedded\n");
    }
    

    memset((void*)&vkMemoryRequirements, 0, sizeof(VkMemoryRequirements));
   
    vkGetBufferMemoryRequirements(vkDevice, vertexData_index_earth.vkBuffer, &vkMemoryRequirements);
    
    
    memset((void*)&vkMemoryAllocateInfo , 0, sizeof(VkMemoryAllocateInfo));
    vkMemoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    vkMemoryAllocateInfo.pNext = NULL; 
    vkMemoryAllocateInfo.allocationSize = vkMemoryRequirements.size;
    
    vkMemoryAllocateInfo.memoryTypeIndex = 0;
   
    for(int i = 0; i < vkPhysicalDeviceMemoryProperties.memoryTypeCount; i++)
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
    
    
    vkResult = vkAllocateMemory(vkDevice, &vkMemoryAllocateInfo, NULL, &vertexData_index_earth.vkDeviceMemory);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "createIndexBuffer() --> vkAllocateMemory() is failed for rindex buffer and error code is %d\n", vkResult);
        fflush(gpFile);
        return vkResult;
    }
    else
    {
        fprintf(gpFile, "createIndexBuffer() --> vkAllocateMemory() is succedded for index buffer\n");
    }
    
    
    vkResult = vkBindBufferMemory(vkDevice, vertexData_index_earth.vkBuffer, vertexData_index_earth.vkDeviceMemory, 0);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "createIndexBuffer() --> vkBindBufferMemory() is failed for index buffer and error code is %d\n", vkResult);
        fflush(gpFile);
        return vkResult;
    }
    else
    {
        fprintf(gpFile, "createIndexBuffer() --> vkBindBufferMemory() is succedded for index buffer\n");
    }
    
    
    data = NULL;
    vkResult = vkMapMemory(vkDevice, vertexData_index_earth.vkDeviceMemory, 0, vkMemoryAllocateInfo.allocationSize, 0, &data);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "createIndexBuffer() --> vkMapMemory() is failed for index buffer and error code is %d\n", vkResult);
        fflush(gpFile);
        return vkResult;
    }
    else
    {
        fprintf(gpFile, "createIndexBuffer() --> vkMapMemory() is succedded for index buffer\n");
    }
   
    
    memcpy(data, sphere_elements, sizeof(sphere_elements));
    
    vkUnmapMemory(vkDevice, vertexData_index_earth.vkDeviceMemory);


    
    
    
    memset((void*)&vertexData_index_mars, 0, sizeof(VertexData));
    
    
    memset((void*)&vkBufferCreateInfo , 0, sizeof(VkBufferCreateInfo));
    vkBufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    vkBufferCreateInfo.pNext = NULL;
    
    vkBufferCreateInfo.flags = 0;
    vkBufferCreateInfo.size = sizeof(sphere_elements);
    vkBufferCreateInfo.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
    
    
    vkResult = vkCreateBuffer(vkDevice, 
                              &vkBufferCreateInfo,
                              NULL,
                              &vertexData_index_mars.vkBuffer);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "createIndexBuffer() --> vkCreateBuffer() of index buffer is failed and error code is %d\n", vkResult);
        fflush(gpFile);
        return vkResult;
    }
    else
    {
        fprintf(gpFile, "createIndexBuffer() --> vkCreateBuffer() of index buffer is succedded\n");
    }
    

    memset((void*)&vkMemoryRequirements, 0, sizeof(VkMemoryRequirements));
   
    vkGetBufferMemoryRequirements(vkDevice, vertexData_index_mars.vkBuffer, &vkMemoryRequirements);
    
    
    memset((void*)&vkMemoryAllocateInfo , 0, sizeof(VkMemoryAllocateInfo));
    vkMemoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    vkMemoryAllocateInfo.pNext = NULL; 
    vkMemoryAllocateInfo.allocationSize = vkMemoryRequirements.size;
    
    vkMemoryAllocateInfo.memoryTypeIndex = 0;
   
    for(int i = 0; i < vkPhysicalDeviceMemoryProperties.memoryTypeCount; i++)
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
    
    
    vkResult = vkAllocateMemory(vkDevice, &vkMemoryAllocateInfo, NULL, &vertexData_index_mars.vkDeviceMemory);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "createIndexBuffer() --> vkAllocateMemory() is failed for rindex buffer and error code is %d\n", vkResult);
        fflush(gpFile);
        return vkResult;
    }
    else
    {
        fprintf(gpFile, "createIndexBuffer() --> vkAllocateMemory() is succedded for index buffer\n");
    }
    
    
    vkResult = vkBindBufferMemory(vkDevice, vertexData_index_mars.vkBuffer, vertexData_index_mars.vkDeviceMemory, 0);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "createIndexBuffer() --> vkBindBufferMemory() is failed for index buffer and error code is %d\n", vkResult);
        fflush(gpFile);
        return vkResult;
    }
    else
    {
        fprintf(gpFile, "createIndexBuffer() --> vkBindBufferMemory() is succedded for index buffer\n");
    }
    
    
    data = NULL;
    vkResult = vkMapMemory(vkDevice, vertexData_index_mars.vkDeviceMemory, 0, vkMemoryAllocateInfo.allocationSize, 0, &data);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "createIndexBuffer() --> vkMapMemory() is failed for index buffer and error code is %d\n", vkResult);
        fflush(gpFile);
        return vkResult;
    }
    else
    {
        fprintf(gpFile, "createIndexBuffer() --> vkMapMemory() is succedded for index buffer\n");
    }
   
    
    memcpy(data, sphere_elements, sizeof(sphere_elements));
    
    vkUnmapMemory(vkDevice, vertexData_index_mars.vkDeviceMemory);
    
    
    
    
    
    memset((void*)&vertexData_index_jupitor, 0, sizeof(VertexData));
    
    
    memset((void*)&vkBufferCreateInfo , 0, sizeof(VkBufferCreateInfo));
    vkBufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    vkBufferCreateInfo.pNext = NULL;
    
    vkBufferCreateInfo.flags = 0;
    vkBufferCreateInfo.size = sizeof(sphere_elements);
    vkBufferCreateInfo.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
    
    
    vkResult = vkCreateBuffer(vkDevice, 
                              &vkBufferCreateInfo,
                              NULL,
                              &vertexData_index_jupitor.vkBuffer);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "createIndexBuffer() --> vkCreateBuffer() of index buffer is failed and error code is %d\n", vkResult);
        fflush(gpFile);
        return vkResult;
    }
    else
    {
        fprintf(gpFile, "createIndexBuffer() --> vkCreateBuffer() of index buffer is succedded\n");
    }
    

    memset((void*)&vkMemoryRequirements, 0, sizeof(VkMemoryRequirements));
   
    vkGetBufferMemoryRequirements(vkDevice, vertexData_index_jupitor.vkBuffer, &vkMemoryRequirements);
    
    
    memset((void*)&vkMemoryAllocateInfo , 0, sizeof(VkMemoryAllocateInfo));
    vkMemoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    vkMemoryAllocateInfo.pNext = NULL; 
    vkMemoryAllocateInfo.allocationSize = vkMemoryRequirements.size;
    
    vkMemoryAllocateInfo.memoryTypeIndex = 0;
   
    for(int i = 0; i < vkPhysicalDeviceMemoryProperties.memoryTypeCount; i++)
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
    
    
    vkResult = vkAllocateMemory(vkDevice, &vkMemoryAllocateInfo, NULL, &vertexData_index_jupitor.vkDeviceMemory);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "createIndexBuffer() --> vkAllocateMemory() is failed for rindex buffer and error code is %d\n", vkResult);
        fflush(gpFile);
        return vkResult;
    }
    else
    {
        fprintf(gpFile, "createIndexBuffer() --> vkAllocateMemory() is succedded for index buffer\n");
    }
    
    
    vkResult = vkBindBufferMemory(vkDevice, vertexData_index_jupitor.vkBuffer, vertexData_index_jupitor.vkDeviceMemory, 0);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "createIndexBuffer() --> vkBindBufferMemory() is failed for index buffer and error code is %d\n", vkResult);
        fflush(gpFile);
        return vkResult;
    }
    else
    {
        fprintf(gpFile, "createIndexBuffer() --> vkBindBufferMemory() is succedded for index buffer\n");
    }
    
    
    data = NULL;
    vkResult = vkMapMemory(vkDevice, vertexData_index_jupitor.vkDeviceMemory, 0, vkMemoryAllocateInfo.allocationSize, 0, &data);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "createIndexBuffer() --> vkMapMemory() is failed for index buffer and error code is %d\n", vkResult);
        fflush(gpFile);
        return vkResult;
    }
    else
    {
        fprintf(gpFile, "createIndexBuffer() --> vkMapMemory() is succedded for index buffer\n");
    }
   
    
    memcpy(data, sphere_elements, sizeof(sphere_elements));
    
    vkUnmapMemory(vkDevice, vertexData_index_jupitor.vkDeviceMemory);
    
    
    
    
    
    memset((void*)&vertexData_index_saturn, 0, sizeof(VertexData));
    
    
    memset((void*)&vkBufferCreateInfo , 0, sizeof(VkBufferCreateInfo));
    vkBufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    vkBufferCreateInfo.pNext = NULL;
    
    vkBufferCreateInfo.flags = 0;
    vkBufferCreateInfo.size = sizeof(sphere_elements);
    vkBufferCreateInfo.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
    
    
    vkResult = vkCreateBuffer(vkDevice, 
                              &vkBufferCreateInfo,
                              NULL,
                              &vertexData_index_saturn.vkBuffer);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "createIndexBuffer() --> vkCreateBuffer() of index buffer is failed and error code is %d\n", vkResult);
        fflush(gpFile);
        return vkResult;
    }
    else
    {
        fprintf(gpFile, "createIndexBuffer() --> vkCreateBuffer() of index buffer is succedded\n");
    }
    

    memset((void*)&vkMemoryRequirements, 0, sizeof(VkMemoryRequirements));
   
    vkGetBufferMemoryRequirements(vkDevice, vertexData_index_saturn.vkBuffer, &vkMemoryRequirements);
    
    
    memset((void*)&vkMemoryAllocateInfo , 0, sizeof(VkMemoryAllocateInfo));
    vkMemoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    vkMemoryAllocateInfo.pNext = NULL; 
    vkMemoryAllocateInfo.allocationSize = vkMemoryRequirements.size;
    
    vkMemoryAllocateInfo.memoryTypeIndex = 0;
   
    for(int i = 0; i < vkPhysicalDeviceMemoryProperties.memoryTypeCount; i++)
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
    
    
    vkResult = vkAllocateMemory(vkDevice, &vkMemoryAllocateInfo, NULL, &vertexData_index_saturn.vkDeviceMemory);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "createIndexBuffer() --> vkAllocateMemory() is failed for rindex buffer and error code is %d\n", vkResult);
        fflush(gpFile);
        return vkResult;
    }
    else
    {
        fprintf(gpFile, "createIndexBuffer() --> vkAllocateMemory() is succedded for index buffer\n");
    }
    
    
    vkResult = vkBindBufferMemory(vkDevice, vertexData_index_saturn.vkBuffer, vertexData_index_saturn.vkDeviceMemory, 0);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "createIndexBuffer() --> vkBindBufferMemory() is failed for index buffer and error code is %d\n", vkResult);
        fflush(gpFile);
        return vkResult;
    }
    else
    {
        fprintf(gpFile, "createIndexBuffer() --> vkBindBufferMemory() is succedded for index buffer\n");
    }
    
    
    data = NULL;
    vkResult = vkMapMemory(vkDevice, vertexData_index_saturn.vkDeviceMemory, 0, vkMemoryAllocateInfo.allocationSize, 0, &data);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "createIndexBuffer() --> vkMapMemory() is failed for index buffer and error code is %d\n", vkResult);
        fflush(gpFile);
        return vkResult;
    }
    else
    {
        fprintf(gpFile, "createIndexBuffer() --> vkMapMemory() is succedded for index buffer\n");
    }
   
    
    memcpy(data, sphere_elements, sizeof(sphere_elements));
    
    vkUnmapMemory(vkDevice, vertexData_index_saturn.vkDeviceMemory);
    
    
    
    
    
    
    memset((void*)&vertexData_index_uranus, 0, sizeof(VertexData));
    
    
    memset((void*)&vkBufferCreateInfo , 0, sizeof(VkBufferCreateInfo));
    vkBufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    vkBufferCreateInfo.pNext = NULL;
    
    vkBufferCreateInfo.flags = 0;
    vkBufferCreateInfo.size = sizeof(sphere_elements);
    vkBufferCreateInfo.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
    
    
    vkResult = vkCreateBuffer(vkDevice, 
                              &vkBufferCreateInfo,
                              NULL,
                              &vertexData_index_uranus.vkBuffer);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "createIndexBuffer() --> vkCreateBuffer() of index buffer is failed and error code is %d\n", vkResult);
        fflush(gpFile);
        return vkResult;
    }
    else
    {
        fprintf(gpFile, "createIndexBuffer() --> vkCreateBuffer() of index buffer is succedded\n");
    }
    

    memset((void*)&vkMemoryRequirements, 0, sizeof(VkMemoryRequirements));
   
    vkGetBufferMemoryRequirements(vkDevice, vertexData_index_uranus.vkBuffer, &vkMemoryRequirements);
    
    
    memset((void*)&vkMemoryAllocateInfo , 0, sizeof(VkMemoryAllocateInfo));
    vkMemoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    vkMemoryAllocateInfo.pNext = NULL; 
    vkMemoryAllocateInfo.allocationSize = vkMemoryRequirements.size;
    
    vkMemoryAllocateInfo.memoryTypeIndex = 0;
   
    for(int i = 0; i < vkPhysicalDeviceMemoryProperties.memoryTypeCount; i++)
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
    
    
    vkResult = vkAllocateMemory(vkDevice, &vkMemoryAllocateInfo, NULL, &vertexData_index_uranus.vkDeviceMemory);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "createIndexBuffer() --> vkAllocateMemory() is failed for rindex buffer and error code is %d\n", vkResult);
        fflush(gpFile);
        return vkResult;
    }
    else
    {
        fprintf(gpFile, "createIndexBuffer() --> vkAllocateMemory() is succedded for index buffer\n");
    }
    
    
    vkResult = vkBindBufferMemory(vkDevice, vertexData_index_uranus.vkBuffer, vertexData_index_uranus.vkDeviceMemory, 0);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "createIndexBuffer() --> vkBindBufferMemory() is failed for index buffer and error code is %d\n", vkResult);
        fflush(gpFile);
        return vkResult;
    }
    else
    {
        fprintf(gpFile, "createIndexBuffer() --> vkBindBufferMemory() is succedded for index buffer\n");
    }
    
    
    data = NULL;
    vkResult = vkMapMemory(vkDevice, vertexData_index_uranus.vkDeviceMemory, 0, vkMemoryAllocateInfo.allocationSize, 0, &data);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "createIndexBuffer() --> vkMapMemory() is failed for index buffer and error code is %d\n", vkResult);
        fflush(gpFile);
        return vkResult;
    }
    else
    {
        fprintf(gpFile, "createIndexBuffer() --> vkMapMemory() is succedded for index buffer\n");
    }
   
    
    memcpy(data, sphere_elements, sizeof(sphere_elements));
    
    vkUnmapMemory(vkDevice, vertexData_index_uranus.vkDeviceMemory);
    
    
    
    
    
    memset((void*)&vertexData_index_neptune, 0, sizeof(VertexData));
    
    
    memset((void*)&vkBufferCreateInfo , 0, sizeof(VkBufferCreateInfo));
    vkBufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    vkBufferCreateInfo.pNext = NULL;
    
    vkBufferCreateInfo.flags = 0;
    vkBufferCreateInfo.size = sizeof(sphere_elements);
    vkBufferCreateInfo.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
    
    
    vkResult = vkCreateBuffer(vkDevice, 
                              &vkBufferCreateInfo,
                              NULL,
                              &vertexData_index_neptune.vkBuffer);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "createIndexBuffer() --> vkCreateBuffer() of index buffer is failed and error code is %d\n", vkResult);
        fflush(gpFile);
        return vkResult;
    }
    else
    {
        fprintf(gpFile, "createIndexBuffer() --> vkCreateBuffer() of index buffer is succedded\n");
    }
    

    memset((void*)&vkMemoryRequirements, 0, sizeof(VkMemoryRequirements));
   
    vkGetBufferMemoryRequirements(vkDevice, vertexData_index_neptune.vkBuffer, &vkMemoryRequirements);
    
    
    memset((void*)&vkMemoryAllocateInfo , 0, sizeof(VkMemoryAllocateInfo));
    vkMemoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    vkMemoryAllocateInfo.pNext = NULL; 
    vkMemoryAllocateInfo.allocationSize = vkMemoryRequirements.size;
    
    vkMemoryAllocateInfo.memoryTypeIndex = 0;
   
    for(int i = 0; i < vkPhysicalDeviceMemoryProperties.memoryTypeCount; i++)
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
    
    
    vkResult = vkAllocateMemory(vkDevice, &vkMemoryAllocateInfo, NULL, &vertexData_index_neptune.vkDeviceMemory);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "createIndexBuffer() --> vkAllocateMemory() is failed for rindex buffer and error code is %d\n", vkResult);
        fflush(gpFile);
        return vkResult;
    }
    else
    {
        fprintf(gpFile, "createIndexBuffer() --> vkAllocateMemory() is succedded for index buffer\n");
    }
    
    
    vkResult = vkBindBufferMemory(vkDevice, vertexData_index_neptune.vkBuffer, vertexData_index_neptune.vkDeviceMemory, 0);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "createIndexBuffer() --> vkBindBufferMemory() is failed for index buffer and error code is %d\n", vkResult);
        fflush(gpFile);
        return vkResult;
    }
    else
    {
        fprintf(gpFile, "createIndexBuffer() --> vkBindBufferMemory() is succedded for index buffer\n");
    }
    
    
    data = NULL;
    vkResult = vkMapMemory(vkDevice, vertexData_index_neptune.vkDeviceMemory, 0, vkMemoryAllocateInfo.allocationSize, 0, &data);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "createIndexBuffer() --> vkMapMemory() is failed for index buffer and error code is %d\n", vkResult);
        fflush(gpFile);
        return vkResult;
    }
    else
    {
        fprintf(gpFile, "createIndexBuffer() --> vkMapMemory() is succedded for index buffer\n");
    }
   
    
    memcpy(data, sphere_elements, sizeof(sphere_elements));
    
    vkUnmapMemory(vkDevice, vertexData_index_neptune.vkDeviceMemory);

    return (vkResult);
}


VkResult createTexture(const char* textureFileName)
{
    VkResult vkResult = VK_SUCCESS;

    // Identify which planet this texture is for (your existing booleans & logs preserved)
    bool bSun_texture     = VK_FALSE;
    bool bMercury_texture = VK_FALSE;
    bool bVenus_texture   = VK_FALSE;
    bool bEarth_texture   = VK_FALSE;
    bool bMars_texture    = VK_FALSE;
    bool bJupitor_texture = VK_FALSE;
    bool bSaturn_texture  = VK_FALSE;
    bool bUranus_texture  = VK_FALSE;
    bool bNeptune_texture = VK_FALSE;

    FILE* fp = fopen(textureFileName, "rb");
    if (fp == NULL)
    {
        fprintf(gpFile, "createTexture() --> fopen() failed to open %s\n", textureFileName);
        return VK_ERROR_INITIALIZATION_FAILED;
    }

    if (textureFileName[0] == 's' && textureFileName[1] == 'u' && textureFileName[2] == 'n')
    { bSun_texture = VK_TRUE;     fprintf(gpFile, "createTexture() --> bSun_texture is VK_TRUE\n"); }
    if (textureFileName[0] == 'm' && textureFileName[1] == 'e' && textureFileName[2] == 'r')
    { bMercury_texture = VK_TRUE; fprintf(gpFile, "createTexture() --> bMercury_texture is VK_TRUE\n"); }
    if (textureFileName[0] == 'v' && textureFileName[1] == 'e' && textureFileName[2] == 'n')
    { bVenus_texture = VK_TRUE;   fprintf(gpFile, "createTexture() --> bVenus_texture is VK_TRUE\n"); }
    if (textureFileName[0] == 'e' && textureFileName[1] == 'a' && textureFileName[2] == 'r')
    { bEarth_texture = VK_TRUE;   fprintf(gpFile, "createTexture() --> bEarth_texture is VK_TRUE\n"); }
    if (textureFileName[0] == 'm' && textureFileName[1] == 'a' && textureFileName[2] == 'r')
    { bMars_texture = VK_TRUE;    fprintf(gpFile, "createTexture() --> bMars_texture is VK_TRUE\n"); }
    if (textureFileName[0] == 'j' && textureFileName[1] == 'u' && textureFileName[2] == 'p')
    { bJupitor_texture = VK_TRUE; fprintf(gpFile, "createTexture() --> bJupitor_texture is VK_TRUE\n"); }
    if (textureFileName[0] == 's' && textureFileName[1] == 'a' && textureFileName[2] == 't')
    { bSaturn_texture = VK_TRUE;  fprintf(gpFile, "createTexture() --> bSaturn_texture is VK_TRUE\n"); }
    if (textureFileName[0] == 'u' && textureFileName[1] == 'r' && textureFileName[2] == 'a')
    { bUranus_texture = VK_TRUE;  fprintf(gpFile, "createTexture() --> bUranus_texture is VK_TRUE\n"); }
    if (textureFileName[0] == 'n' && textureFileName[1] == 'e' && textureFileName[2] == 'p')
    { bNeptune_texture = VK_TRUE; fprintf(gpFile, "createTexture() --> bNeptune_texture is VK_TRUE\n"); }

    // Load RGBA8 with stb_image (matches VK_FORMAT_R8G8B8A8_UNORM used below)
    uint8_t* imageData = NULL; int texture_width = 0, texture_height = 0, texture_channels = 0;
    imageData = stbi_load_from_file(fp, &texture_width, &texture_height, &texture_channels, STBI_rgb_alpha);
    fclose(fp);
    if (imageData == NULL || texture_width <= 0 || texture_height <= 0)
    {
        fprintf(gpFile, "createTexture() --> stbi_load_from_file() failed for %s\n", textureFileName);
        return VK_ERROR_INITIALIZATION_FAILED;
    }

    const VkDeviceSize image_size = (VkDeviceSize)texture_width * (VkDeviceSize)texture_height * 4;

    // Stage buffer (device-local images cannot be host-mapped directly)
    VkBuffer       vkBuffer_stagingBuffer         = VK_NULL_HANDLE;
    VkDeviceMemory vkDeviceMemory_stagingBuffer   = VK_NULL_HANDLE;

    VkBufferCreateInfo vkBufferCreateInfo_stagingBuffer;
    memset(&vkBufferCreateInfo_stagingBuffer, 0, sizeof(vkBufferCreateInfo_stagingBuffer));
    vkBufferCreateInfo_stagingBuffer.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    vkBufferCreateInfo_stagingBuffer.size  = image_size;
    vkBufferCreateInfo_stagingBuffer.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    vkBufferCreateInfo_stagingBuffer.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    vkResult = vkCreateBuffer(vkDevice, &vkBufferCreateInfo_stagingBuffer, NULL, &vkBuffer_stagingBuffer);
    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "createTexture() --> vkCreateBuffer(staging) failed: %d\n", vkResult);
        stbi_image_free(imageData);
        return vkResult;
    }
    else
    {
        fprintf(gpFile, "createTexture() --> vkCreateBuffer() is succedded\n");
    }

    VkMemoryRequirements vkMemReq_staging;
    vkGetBufferMemoryRequirements(vkDevice, vkBuffer_stagingBuffer, &vkMemReq_staging);

    VkMemoryAllocateInfo vkAlloc_staging;
    memset(&vkAlloc_staging, 0, sizeof(vkAlloc_staging));
    vkAlloc_staging.sType          = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    vkAlloc_staging.allocationSize = vkMemReq_staging.size;
    vkAlloc_staging.memoryTypeIndex = 0;

    for (uint32_t i = 0; i < vkPhysicalDeviceMemoryProperties.memoryTypeCount; i++)
    {
        if ((vkMemReq_staging.memoryTypeBits & 1) == 1)
        {
            if ((vkPhysicalDeviceMemoryProperties.memoryTypes[i].propertyFlags &
                 (VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)) ==
                (VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT))
            {
                vkAlloc_staging.memoryTypeIndex = i;
                break;
            }
        }
        vkMemReq_staging.memoryTypeBits >>= 1;
    }

    vkResult = vkAllocateMemory(vkDevice, &vkAlloc_staging, NULL, &vkDeviceMemory_stagingBuffer);
    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "createTexture() --> vkAllocateMemory(staging) failed: %d\n", vkResult);
        stbi_image_free(imageData);
        return vkResult;
    }
    else
    {
        fprintf(gpFile, "createTexture() --> vkAllocateMemory() is succedded\n");
    }

    vkResult = vkBindBufferMemory(vkDevice, vkBuffer_stagingBuffer, vkDeviceMemory_stagingBuffer, 0);
    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "createTexture() --> vkBindBufferMemory(staging) failed: %d\n", vkResult);
        stbi_image_free(imageData);
        return vkResult;
    }
    else
    {
        fprintf(gpFile, "createTexture() --> vkBindBufferMemory() is succedded\n");
    }

    // Upload pixels
    void* data = NULL;
    vkResult = vkMapMemory(vkDevice, vkDeviceMemory_stagingBuffer, 0, image_size, 0, &data);
    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "createTexture() --> vkMapMemory(staging) failed: %d\n", vkResult);
        stbi_image_free(imageData);
        return vkResult;
    }
    else
    {
        fprintf(gpFile, "createTexture() --> vkMapMemory() is succedded\n");
    }

    memcpy(data, imageData, (size_t)image_size);
    vkUnmapMemory(vkDevice, vkDeviceMemory_stagingBuffer);
    stbi_image_free(imageData);
    fprintf(gpFile, "createTexture() --> stbi_image_free() Freeing of image data is succedded\n");

    // Device-local image (RGBA8)
    VkImageCreateInfo vkImageCreateInfo;
    memset(&vkImageCreateInfo, 0, sizeof(vkImageCreateInfo));
    vkImageCreateInfo.sType         = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    vkImageCreateInfo.imageType     = VK_IMAGE_TYPE_2D;
    vkImageCreateInfo.format        = VK_FORMAT_R8G8B8A8_UNORM;
    vkImageCreateInfo.extent.width  = (uint32_t)texture_width;
    vkImageCreateInfo.extent.height = (uint32_t)texture_height;
    vkImageCreateInfo.extent.depth  = 1;
    vkImageCreateInfo.mipLevels     = 1;
    vkImageCreateInfo.arrayLayers   = 1;
    vkImageCreateInfo.samples       = VK_SAMPLE_COUNT_1_BIT;
    vkImageCreateInfo.tiling        = VK_IMAGE_TILING_OPTIMAL;
    vkImageCreateInfo.usage         = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    vkImageCreateInfo.sharingMode   = VK_SHARING_MODE_EXCLUSIVE;
    vkImageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

    vkResult = vkCreateImage(vkDevice, &vkImageCreateInfo, NULL, &vkImage_texture);
    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "createTexture() --> vkCreateImage() failed: %d\n", vkResult);
        return vkResult;
    }
    else
    {
        fprintf(gpFile, "createTexture() --> vkCreateImage() is succedded\n");
    }

    VkMemoryRequirements vkMemReq_image;
    vkGetImageMemoryRequirements(vkDevice, vkImage_texture, &vkMemReq_image);

    VkMemoryAllocateInfo vkAlloc_image;
    memset(&vkAlloc_image, 0, sizeof(vkAlloc_image));
    vkAlloc_image.sType          = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    vkAlloc_image.allocationSize = vkMemReq_image.size;
    vkAlloc_image.memoryTypeIndex = 0;

    for (uint32_t i = 0; i < vkPhysicalDeviceMemoryProperties.memoryTypeCount; i++)
    {
        if ((vkMemReq_image.memoryTypeBits & 1) == 1)
        {
            if (vkPhysicalDeviceMemoryProperties.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
            {
                vkAlloc_image.memoryTypeIndex = i;
                break;
            }
        }
        vkMemReq_image.memoryTypeBits >>= 1;
    }

    vkResult = vkAllocateMemory(vkDevice, &vkAlloc_image, NULL, &vkDeviceMemory_texture);
    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "createTexture() --> vkAllocateMemory(image) failed: %d\n", vkResult);
        return vkResult;
    }
    else
    {
        fprintf(gpFile, "createTexture() --> vkAllocateMemory() is succedded\n");
    }

    vkResult = vkBindImageMemory(vkDevice, vkImage_texture, vkDeviceMemory_texture, 0);
    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "createTexture() --> vkBindImageMemory(image) failed: %d\n", vkResult);
        return vkResult;
    }
    else
    {
        fprintf(gpFile, "createTexture() --> vkBindImageMemory() is succedded\n");
    }

    // 1st one-time command: transition to TRANSFER_DST_OPTIMAL
    VkCommandBuffer cmd1 = VK_NULL_HANDLE;
    VkCommandBufferAllocateInfo cbAllocInfo1;
    memset(&cbAllocInfo1, 0, sizeof(cbAllocInfo1));
    cbAllocInfo1.sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    cbAllocInfo1.commandPool        = vkCommandPool;
    cbAllocInfo1.level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    cbAllocInfo1.commandBufferCount = 1;
    vkAllocateCommandBuffers(vkDevice, &cbAllocInfo1, &cmd1);
    fprintf(gpFile, "createTexture() --> vkAllocateCommandBuffers() is succedded\n");

    VkCommandBufferBeginInfo beginInfo;
    memset(&beginInfo, 0, sizeof(beginInfo));
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    vkBeginCommandBuffer(cmd1, &beginInfo);
    fprintf(gpFile, "createTexture() --> vkBeginCommandBuffer() is succedded\n");

    VkImageMemoryBarrier barrier1;
    memset(&barrier1, 0, sizeof(barrier1));
    barrier1.sType               = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier1.srcAccessMask       = 0;
    barrier1.dstAccessMask       = VK_ACCESS_TRANSFER_WRITE_BIT;
    barrier1.oldLayout           = VK_IMAGE_LAYOUT_UNDEFINED;
    barrier1.newLayout           = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    barrier1.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier1.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier1.image               = vkImage_texture;
    barrier1.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier1.subresourceRange.baseMipLevel   = 0;
    barrier1.subresourceRange.levelCount     = 1;
    barrier1.subresourceRange.baseArrayLayer = 0;
    barrier1.subresourceRange.layerCount     = 1;

    vkCmdPipelineBarrier(cmd1,
                         VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
                         0, 0, NULL, 0, NULL, 1, &barrier1);
    vkEndCommandBuffer(cmd1);
    fprintf(gpFile, "createTexture() --> vkEndCommandBuffer() is succedded\n");

    VkSubmitInfo submit1;
    memset(&submit1, 0, sizeof(submit1));
    submit1.sType              = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit1.commandBufferCount = 1;
    submit1.pCommandBuffers    = &cmd1;
    vkQueueSubmit(vkQueue, 1, &submit1, VK_NULL_HANDLE);
    fprintf(gpFile, "createTexture() --> vkQueueSubmit() succeded\n");
    vkQueueWaitIdle(vkQueue);
    fprintf(gpFile, "createTexture() --> vkQueueWaitIdle() succeded\n");

    // 2nd one-time command: copy buffer -> image
    VkCommandBuffer cmd2 = VK_NULL_HANDLE;
    VkCommandBufferAllocateInfo cbAllocInfo2 = cbAllocInfo1;
    vkAllocateCommandBuffers(vkDevice, &cbAllocInfo2, &cmd2);
    fprintf(gpFile, "createTexture() --> vkAllocateCommandBuffers() is succedded for buffer_to_image_copy\n");
    vkBeginCommandBuffer(cmd2, &beginInfo);
    fprintf(gpFile, "createTexture() --> vkBeginCommandBuffer() is succedded for buffer_to_image_copy\n");

    VkBufferImageCopy region;
    memset(&region, 0, sizeof(region));
    region.imageSubresource.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.mipLevel       = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount     = 1;
    region.imageExtent.width  = (uint32_t)texture_width;
    region.imageExtent.height = (uint32_t)texture_height;
    region.imageExtent.depth  = 1;

    vkCmdCopyBufferToImage(cmd2,
                           vkBuffer_stagingBuffer,
                           vkImage_texture,
                           VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                           1, &region);

    vkEndCommandBuffer(cmd2);
    fprintf(gpFile, "createTexture() --> vkEndCommandBuffer() is succedded for buffer_to_image_copy\n");
    VkSubmitInfo submit2 = submit1; submit2.pCommandBuffers = &cmd2;
    vkQueueSubmit(vkQueue, 1, &submit2, VK_NULL_HANDLE);
    fprintf(gpFile, "createTexture() --> vkQueueSubmit() is succedded for buffer_to_image_copy");
    vkQueueWaitIdle(vkQueue);
    fprintf(gpFile, "createTexture() --> vkQueueWaitIdle() succeded for buffer_to_image_copy\n");

    // 3rd one-time command: transition to SHADER_READ_ONLY_OPTIMAL
    VkCommandBuffer cmd3 = VK_NULL_HANDLE;
    VkCommandBufferAllocateInfo cbAllocInfo3 = cbAllocInfo1;
    vkAllocateCommandBuffers(vkDevice, &cbAllocInfo3, &cmd3);
    fprintf(gpFile, "createTexture() --> vkAllocateCommandBuffers() is succedded\n");
    vkBeginCommandBuffer(cmd3, &beginInfo);
    fprintf(gpFile, "createTexture() --> vkBeginCommandBuffer() is succedded\n");

    VkImageMemoryBarrier barrier2 = barrier1;
    barrier2.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    barrier2.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
    barrier2.oldLayout     = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    barrier2.newLayout     = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    vkCmdPipelineBarrier(cmd3,
                         VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                         0, 0, NULL, 0, NULL, 1, &barrier2);
    vkEndCommandBuffer(cmd3);
    fprintf(gpFile, "createTexture() --> vkEndCommandBuffer() is succedded\n");
    VkSubmitInfo submit3 = submit1; submit3.pCommandBuffers = &cmd3;
    vkQueueSubmit(vkQueue, 1, &submit3, VK_NULL_HANDLE);
    fprintf(gpFile, "createTexture() --> vkQueueSubmit() succeded  for 2nd time in step 6 \n");
    vkQueueWaitIdle(vkQueue);
    fprintf(gpFile, "createTexture() --> vkQueueWaitIdle() succeded for 2nd time in step 6 \n");

    // Free staging resources
    vkDestroyBuffer(vkDevice, vkBuffer_stagingBuffer, NULL);
    fprintf(gpFile, "createTexture() --> vkDestroyBuffer() is done for vkBuffer_stagingBuffer of setp 7\n");
    vkFreeMemory(vkDevice, vkDeviceMemory_stagingBuffer, NULL);
    fprintf(gpFile, "createTexture() --> vkFreeMemory() is done for vkBuffer_stagingBuffer of setp 7\n");

    // Image view (*** FIX: COLOR aspect only for color image ***)
    VkImageViewCreateInfo vkImageViewCreateInfo;
    memset(&vkImageViewCreateInfo, 0, sizeof(vkImageViewCreateInfo));
    vkImageViewCreateInfo.sType                           = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    vkImageViewCreateInfo.image                           = vkImage_texture;
    vkImageViewCreateInfo.viewType                        = VK_IMAGE_VIEW_TYPE_2D;
    vkImageViewCreateInfo.format                          = VK_FORMAT_R8G8B8A8_UNORM;
    vkImageViewCreateInfo.components.r                    = VK_COMPONENT_SWIZZLE_R;
    vkImageViewCreateInfo.components.g                    = VK_COMPONENT_SWIZZLE_G;
    vkImageViewCreateInfo.components.b                    = VK_COMPONENT_SWIZZLE_B;
    vkImageViewCreateInfo.components.a                    = VK_COMPONENT_SWIZZLE_A;
    vkImageViewCreateInfo.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;   // <-- FIX (was COLOR|STENCIL)
    vkImageViewCreateInfo.subresourceRange.baseMipLevel   = 0;
    vkImageViewCreateInfo.subresourceRange.levelCount     = 1;
    vkImageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
    vkImageViewCreateInfo.subresourceRange.layerCount     = 1;

    vkResult = vkCreateImageView(vkDevice, &vkImageViewCreateInfo, NULL, &vkImageView_texture);
    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "createTexture() --> vkCreateImageView() is failed error code is %d\n", vkResult);
        fflush(gpFile);
        return vkResult;
    }
    else
    {
        fprintf(gpFile, "createTexture() --> vkCreateImageView() is succedded\n");
        fflush(gpFile);
    }

    // Sampler
    VkSamplerCreateInfo vkSamplerCreateInfo;
    memset(&vkSamplerCreateInfo, 0, sizeof(vkSamplerCreateInfo));
    vkSamplerCreateInfo.sType        = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    vkSamplerCreateInfo.magFilter    = VK_FILTER_LINEAR;
    vkSamplerCreateInfo.minFilter    = VK_FILTER_LINEAR;
    vkSamplerCreateInfo.mipmapMode   = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    vkSamplerCreateInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    vkSamplerCreateInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    vkSamplerCreateInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    vkSamplerCreateInfo.anisotropyEnable      = VK_FALSE;
    vkSamplerCreateInfo.maxAnisotropy         = 16;
    vkSamplerCreateInfo.borderColor           = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    vkSamplerCreateInfo.unnormalizedCoordinates = VK_FALSE;
    vkSamplerCreateInfo.compareEnable         = VK_FALSE;
    vkSamplerCreateInfo.compareOp             = VK_COMPARE_OP_ALWAYS;

    vkResult = vkCreateSampler(vkDevice, &vkSamplerCreateInfo, NULL, &vkSampler_texture);
    if (vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "createTexture() --> vkCreateSampler() is failed error code is %d\n", vkResult);
        fflush(gpFile);
        return vkResult;
    }
    else
    {
        fprintf(gpFile, "createTexture() --> vkCreateSampler() is succedded\n");
        fflush(gpFile);
    }

    // Record per-planet handles (image + memory + view + sampler)
    if (bSun_texture)
    {
        vkImage_texture_planet[PLANET_SUN]        = vkImage_texture;
        vkDeviceMemory_texture_planet[PLANET_SUN] = vkDeviceMemory_texture;
        vkImageView_texture_sun                   = vkImageView_texture;
        vkSampler_texture_sun                     = vkSampler_texture;
    }
    if (bMercury_texture)
    {
        vkImage_texture_planet[PLANET_MERCURY]        = vkImage_texture;
        vkDeviceMemory_texture_planet[PLANET_MERCURY] = vkDeviceMemory_texture;
        vkImageView_texture_mercury                    = vkImageView_texture;
        vkSampler_texture_mercury                      = vkSampler_texture;
    }
    if (bVenus_texture)
    {
        vkImage_texture_planet[PLANET_VENUS]        = vkImage_texture;
        vkDeviceMemory_texture_planet[PLANET_VENUS] = vkDeviceMemory_texture;
        vkImageView_texture_venus                    = vkImageView_texture;
        vkSampler_texture_venus                      = vkSampler_texture;
    }
    if (bEarth_texture)
    {
        vkImage_texture_planet[PLANET_EARTH]        = vkImage_texture;
        vkDeviceMemory_texture_planet[PLANET_EARTH] = vkDeviceMemory_texture;
        vkImageView_texture_earth                    = vkImageView_texture;
        vkSampler_texture_earth                      = vkSampler_texture;
    }
    if (bMars_texture)
    {
        vkImage_texture_planet[PLANET_MARS]        = vkImage_texture;
        vkDeviceMemory_texture_planet[PLANET_MARS] = vkDeviceMemory_texture;
        vkImageView_texture_mars                    = vkImageView_texture;
        vkSampler_texture_mars                      = vkSampler_texture;
    }
    if (bJupitor_texture)
    {
        vkImage_texture_planet[PLANET_JUPITOR]        = vkImage_texture;
        vkDeviceMemory_texture_planet[PLANET_JUPITOR] = vkDeviceMemory_texture;
        vkImageView_texture_jupitor                    = vkImageView_texture;
        vkSampler_texture_jupitor                      = vkSampler_texture;
    }
    if (bSaturn_texture)
    {
        vkImage_texture_planet[PLANET_SATURN]        = vkImage_texture;
        vkDeviceMemory_texture_planet[PLANET_SATURN] = vkDeviceMemory_texture;
        vkImageView_texture_saturn                    = vkImageView_texture;
        vkSampler_texture_saturn                      = vkSampler_texture;
    }
    if (bUranus_texture)
    {
        vkImage_texture_planet[PLANET_URANUS]        = vkImage_texture;
        vkDeviceMemory_texture_planet[PLANET_URANUS] = vkDeviceMemory_texture;
        vkImageView_texture_uranus                    = vkImageView_texture;
        vkSampler_texture_uranus                      = vkSampler_texture;
    }
    if (bNeptune_texture)
    {
        vkImage_texture_planet[PLANET_NEPTUNE]        = vkImage_texture;
        vkDeviceMemory_texture_planet[PLANET_NEPTUNE] = vkDeviceMemory_texture;
        vkImageView_texture_neptune                    = vkImageView_texture;
        vkSampler_texture_neptune                      = vkSampler_texture;
    }

    return vkResult;
}

VkResult createUniformBuffer(void)
{
    
    VkResult updateUniformBuffer(void);
    
    
    VkResult vkResult = VK_SUCCESS;
    
    
    
    
    memset((void*)&uniformData_sun, 0, sizeof(UniformData));
    
    
    VkBufferCreateInfo vkBufferCreateInfo;
    memset((void*)&vkBufferCreateInfo , 0, sizeof(VkBufferCreateInfo));
    vkBufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    vkBufferCreateInfo.pNext = NULL;
    
    vkBufferCreateInfo.flags = 0;
    vkBufferCreateInfo.size = sizeof(struct MyUniformData);
    vkBufferCreateInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
    
    
    vkResult = vkCreateBuffer(vkDevice, 
                              &vkBufferCreateInfo,
                              NULL,
                              &uniformData_sun.vkBuffer);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "createUniformBuffer() --> vkCreateBuffer() is failed and error code is %d\n", vkResult);
        fflush(gpFile);
        return vkResult;
    }
    else
    {
        fprintf(gpFile, "createUniformBuffer() --> vkCreateBuffer() is succedded\n");
        fflush(gpFile);
    }
    
    VkMemoryRequirements vkMemoryRequirements;
    memset((void*)&vkMemoryRequirements, 0, sizeof(VkMemoryRequirements));
   
    vkGetBufferMemoryRequirements(vkDevice, uniformData_sun.vkBuffer, &vkMemoryRequirements);
    
    
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
            if(vkPhysicalDeviceMemoryProperties.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)
            {
                vkMemoryAllocateInfo.memoryTypeIndex = i;
                break;
            }
        }
        vkMemoryRequirements.memoryTypeBits >>= 1;
    }
    
    
    vkResult = vkAllocateMemory(vkDevice, &vkMemoryAllocateInfo, NULL, &uniformData_sun.vkDeviceMemory);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "createUniformBuffer() --> vkAllocateMemory() is failed and error code is %d\n", vkResult);
        fflush(gpFile);
        return vkResult;
    }
    else
    {
        fprintf(gpFile, "createUniformBuffer() --> vkAllocateMemory() is succedded\n");
        fflush(gpFile);
    }
    
    
    vkResult = vkBindBufferMemory(vkDevice, uniformData_sun.vkBuffer, uniformData_sun.vkDeviceMemory, 0);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "createUniformBuffer() --> vkBindBufferMemory() is failed and error code is %d\n", vkResult);
        fflush(gpFile);
        return vkResult;
    }
    else
    {
        fprintf(gpFile, "createUniformBuffer() --> vkBindBufferMemory() is succedded\n");
        fflush(gpFile);
    }
    
    
    
    
    
    
    memset((void*)&uniformData_mercury, 0, sizeof(UniformData));
    
    
    memset((void*)&vkBufferCreateInfo , 0, sizeof(VkBufferCreateInfo));
    vkBufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    vkBufferCreateInfo.pNext = NULL;
    
    vkBufferCreateInfo.flags = 0;
    vkBufferCreateInfo.size = sizeof(struct MyUniformData);
    vkBufferCreateInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
    
    
    vkResult = vkCreateBuffer(vkDevice, 
                              &vkBufferCreateInfo,
                              NULL,
                              &uniformData_mercury.vkBuffer);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "createUniformBuffer() --> vkCreateBuffer() is failed and error code is %d\n", vkResult);
        fflush(gpFile);
        return vkResult;
    }
    else
    {
        fprintf(gpFile, "createUniformBuffer() --> vkCreateBuffer() is succedded\n");
        fflush(gpFile);
    }
    
    memset((void*)&vkMemoryRequirements, 0, sizeof(VkMemoryRequirements));
   
    vkGetBufferMemoryRequirements(vkDevice, uniformData_mercury.vkBuffer, &vkMemoryRequirements);
    
    
    memset((void*)&vkMemoryAllocateInfo , 0, sizeof(VkMemoryAllocateInfo));
    vkMemoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    vkMemoryAllocateInfo.pNext = NULL; 
    vkMemoryAllocateInfo.allocationSize = vkMemoryRequirements.size;
    
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
    
    
    vkResult = vkAllocateMemory(vkDevice, &vkMemoryAllocateInfo, NULL, &uniformData_mercury.vkDeviceMemory);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "createUniformBuffer() --> vkAllocateMemory() is failed and error code is %d\n", vkResult);
        fflush(gpFile);
        return vkResult;
    }
    else
    {
        fprintf(gpFile, "createUniformBuffer() --> vkAllocateMemory() is succedded\n");
        fflush(gpFile);
    }
    
    
    vkResult = vkBindBufferMemory(vkDevice, uniformData_mercury.vkBuffer, uniformData_mercury.vkDeviceMemory, 0);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "createUniformBuffer() --> vkBindBufferMemory() is failed and error code is %d\n", vkResult);
        fflush(gpFile);
        return vkResult;
    }
    else
    {
        fprintf(gpFile, "createUniformBuffer() --> vkBindBufferMemory() is succedded\n");
        fflush(gpFile);
    }
    
    
    
    
    
    memset((void*)&uniformData_venus, 0, sizeof(UniformData));
    
    
    memset((void*)&vkBufferCreateInfo , 0, sizeof(VkBufferCreateInfo));
    vkBufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    vkBufferCreateInfo.pNext = NULL;
    
    vkBufferCreateInfo.flags = 0;
    vkBufferCreateInfo.size = sizeof(struct MyUniformData);
    vkBufferCreateInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
    
    
    vkResult = vkCreateBuffer(vkDevice, 
                              &vkBufferCreateInfo,
                              NULL,
                              &uniformData_venus.vkBuffer);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "createUniformBuffer() --> vkCreateBuffer() is failed and error code is %d\n", vkResult);
        fflush(gpFile);
        return vkResult;
    }
    else
    {
        fprintf(gpFile, "createUniformBuffer() --> vkCreateBuffer() is succedded\n");
        fflush(gpFile);
    }
    
    memset((void*)&vkMemoryRequirements, 0, sizeof(VkMemoryRequirements));
   
    vkGetBufferMemoryRequirements(vkDevice, uniformData_venus.vkBuffer, &vkMemoryRequirements);
    
    
    memset((void*)&vkMemoryAllocateInfo , 0, sizeof(VkMemoryAllocateInfo));
    vkMemoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    vkMemoryAllocateInfo.pNext = NULL; 
    vkMemoryAllocateInfo.allocationSize = vkMemoryRequirements.size;
    
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
    
    
    vkResult = vkAllocateMemory(vkDevice, &vkMemoryAllocateInfo, NULL, &uniformData_venus.vkDeviceMemory);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "createUniformBuffer() --> vkAllocateMemory() is failed and error code is %d\n", vkResult);
        fflush(gpFile);
        return vkResult;
    }
    else
    {
        fprintf(gpFile, "createUniformBuffer() --> vkAllocateMemory() is succedded\n");
        fflush(gpFile);
    }
    
    
    vkResult = vkBindBufferMemory(vkDevice, uniformData_venus.vkBuffer, uniformData_venus.vkDeviceMemory, 0);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "createUniformBuffer() --> vkBindBufferMemory() is failed and error code is %d\n", vkResult);
        fflush(gpFile);
        return vkResult;
    }
    else
    {
        fprintf(gpFile, "createUniformBuffer() --> vkBindBufferMemory() is succedded\n");
        fflush(gpFile);
    }
   
   
    
    
    
    memset((void*)&uniformData_earth, 0, sizeof(UniformData));
    
    
    memset((void*)&vkBufferCreateInfo , 0, sizeof(VkBufferCreateInfo));
    vkBufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    vkBufferCreateInfo.pNext = NULL;
    
    vkBufferCreateInfo.flags = 0;
    vkBufferCreateInfo.size = sizeof(struct MyUniformData);
    vkBufferCreateInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
    
    
    vkResult = vkCreateBuffer(vkDevice, 
                              &vkBufferCreateInfo,
                              NULL,
                              &uniformData_earth.vkBuffer);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "createUniformBuffer() --> vkCreateBuffer() is failed and error code is %d\n", vkResult);
        fflush(gpFile);
        return vkResult;
    }
    else
    {
        fprintf(gpFile, "createUniformBuffer() --> vkCreateBuffer() is succedded\n");
        fflush(gpFile);
    }
    
    memset((void*)&vkMemoryRequirements, 0, sizeof(VkMemoryRequirements));
   
    vkGetBufferMemoryRequirements(vkDevice, uniformData_earth.vkBuffer, &vkMemoryRequirements);
    
    
    memset((void*)&vkMemoryAllocateInfo , 0, sizeof(VkMemoryAllocateInfo));
    vkMemoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    vkMemoryAllocateInfo.pNext = NULL; 
    vkMemoryAllocateInfo.allocationSize = vkMemoryRequirements.size;
    
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
    
    
    vkResult = vkAllocateMemory(vkDevice, &vkMemoryAllocateInfo, NULL, &uniformData_earth.vkDeviceMemory);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "createUniformBuffer() --> vkAllocateMemory() is failed and error code is %d\n", vkResult);
        fflush(gpFile);
        return vkResult;
    }
    else
    {
        fprintf(gpFile, "createUniformBuffer() --> vkAllocateMemory() is succedded\n");
        fflush(gpFile);
    }
    
    
    vkResult = vkBindBufferMemory(vkDevice, uniformData_earth.vkBuffer, uniformData_earth.vkDeviceMemory, 0);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "createUniformBuffer() --> vkBindBufferMemory() is failed and error code is %d\n", vkResult);
        fflush(gpFile);
        return vkResult;
    }
    else
    {
        fprintf(gpFile, "createUniformBuffer() --> vkBindBufferMemory() is succedded\n");
        fflush(gpFile);
    }
    
    
    
    
    
    memset((void*)&uniformData_mars, 0, sizeof(UniformData));
    
    
    memset((void*)&vkBufferCreateInfo , 0, sizeof(VkBufferCreateInfo));
    vkBufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    vkBufferCreateInfo.pNext = NULL;
    
    vkBufferCreateInfo.flags = 0;
    vkBufferCreateInfo.size = sizeof(struct MyUniformData);
    vkBufferCreateInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
    
    
    vkResult = vkCreateBuffer(vkDevice, 
                              &vkBufferCreateInfo,
                              NULL,
                              &uniformData_mars.vkBuffer);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "createUniformBuffer() --> vkCreateBuffer() is failed and error code is %d\n", vkResult);
        fflush(gpFile);
        return vkResult;
    }
    else
    {
        fprintf(gpFile, "createUniformBuffer() --> vkCreateBuffer() is succedded\n");
        fflush(gpFile);
    }
    
    memset((void*)&vkMemoryRequirements, 0, sizeof(VkMemoryRequirements));
   
    vkGetBufferMemoryRequirements(vkDevice, uniformData_mars.vkBuffer, &vkMemoryRequirements);
    
    
    memset((void*)&vkMemoryAllocateInfo , 0, sizeof(VkMemoryAllocateInfo));
    vkMemoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    vkMemoryAllocateInfo.pNext = NULL; 
    vkMemoryAllocateInfo.allocationSize = vkMemoryRequirements.size;
    
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
    
    
    vkResult = vkAllocateMemory(vkDevice, &vkMemoryAllocateInfo, NULL, &uniformData_mars.vkDeviceMemory);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "createUniformBuffer() --> vkAllocateMemory() is failed and error code is %d\n", vkResult);
        fflush(gpFile);
        return vkResult;
    }
    else
    {
        fprintf(gpFile, "createUniformBuffer() --> vkAllocateMemory() is succedded\n");
        fflush(gpFile);
    }
    
    
    vkResult = vkBindBufferMemory(vkDevice, uniformData_mars.vkBuffer, uniformData_mars.vkDeviceMemory, 0);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "createUniformBuffer() --> vkBindBufferMemory() is failed and error code is %d\n", vkResult);
        fflush(gpFile);
        return vkResult;
    }
    else
    {
        fprintf(gpFile, "createUniformBuffer() --> vkBindBufferMemory() is succedded\n");
        fflush(gpFile);
    }
    
    
    
    
    
    memset((void*)&uniformData_jupitor, 0, sizeof(UniformData));
    
    
    memset((void*)&vkBufferCreateInfo , 0, sizeof(VkBufferCreateInfo));
    vkBufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    vkBufferCreateInfo.pNext = NULL;
    
    vkBufferCreateInfo.flags = 0;
    vkBufferCreateInfo.size = sizeof(struct MyUniformData);
    vkBufferCreateInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
    
    
    vkResult = vkCreateBuffer(vkDevice, 
                              &vkBufferCreateInfo,
                              NULL,
                              &uniformData_jupitor.vkBuffer);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "createUniformBuffer() --> vkCreateBuffer() is failed and error code is %d\n", vkResult);
        fflush(gpFile);
        return vkResult;
    }
    else
    {
        fprintf(gpFile, "createUniformBuffer() --> vkCreateBuffer() is succedded\n");
        fflush(gpFile);
    }
    
    memset((void*)&vkMemoryRequirements, 0, sizeof(VkMemoryRequirements));
   
    vkGetBufferMemoryRequirements(vkDevice, uniformData_jupitor.vkBuffer, &vkMemoryRequirements);
    
    
    memset((void*)&vkMemoryAllocateInfo , 0, sizeof(VkMemoryAllocateInfo));
    vkMemoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    vkMemoryAllocateInfo.pNext = NULL; 
    vkMemoryAllocateInfo.allocationSize = vkMemoryRequirements.size;
    
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
    
    
    vkResult = vkAllocateMemory(vkDevice, &vkMemoryAllocateInfo, NULL, &uniformData_jupitor.vkDeviceMemory);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "createUniformBuffer() --> vkAllocateMemory() is failed and error code is %d\n", vkResult);
        fflush(gpFile);
        return vkResult;
    }
    else
    {
        fprintf(gpFile, "createUniformBuffer() --> vkAllocateMemory() is succedded\n");
        fflush(gpFile);
    }
    
    
    vkResult = vkBindBufferMemory(vkDevice, uniformData_jupitor.vkBuffer, uniformData_jupitor.vkDeviceMemory, 0);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "createUniformBuffer() --> vkBindBufferMemory() is failed and error code is %d\n", vkResult);
        fflush(gpFile);
        return vkResult;
    }
    else
    {
        fprintf(gpFile, "createUniformBuffer() --> vkBindBufferMemory() is succedded\n");
        fflush(gpFile);
    }
    
    
    
    
    
    memset((void*)&uniformData_saturn, 0, sizeof(UniformData));
    
    
    memset((void*)&vkBufferCreateInfo , 0, sizeof(VkBufferCreateInfo));
    vkBufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    vkBufferCreateInfo.pNext = NULL;
    
    vkBufferCreateInfo.flags = 0;
    vkBufferCreateInfo.size = sizeof(struct MyUniformData);
    vkBufferCreateInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
    
    
    vkResult = vkCreateBuffer(vkDevice, 
                              &vkBufferCreateInfo,
                              NULL,
                              &uniformData_saturn.vkBuffer);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "createUniformBuffer() --> vkCreateBuffer() is failed and error code is %d\n", vkResult);
        fflush(gpFile);
        return vkResult;
    }
    else
    {
        fprintf(gpFile, "createUniformBuffer() --> vkCreateBuffer() is succedded\n");
        fflush(gpFile);
    }
    
    memset((void*)&vkMemoryRequirements, 0, sizeof(VkMemoryRequirements));
   
    vkGetBufferMemoryRequirements(vkDevice, uniformData_saturn.vkBuffer, &vkMemoryRequirements);
    
    
    memset((void*)&vkMemoryAllocateInfo , 0, sizeof(VkMemoryAllocateInfo));
    vkMemoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    vkMemoryAllocateInfo.pNext = NULL; 
    vkMemoryAllocateInfo.allocationSize = vkMemoryRequirements.size;
    
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
    
    
    vkResult = vkAllocateMemory(vkDevice, &vkMemoryAllocateInfo, NULL, &uniformData_saturn.vkDeviceMemory);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "createUniformBuffer() --> vkAllocateMemory() is failed and error code is %d\n", vkResult);
        fflush(gpFile);
        return vkResult;
    }
    else
    {
        fprintf(gpFile, "createUniformBuffer() --> vkAllocateMemory() is succedded\n");
        fflush(gpFile);
    }
    
    
    vkResult = vkBindBufferMemory(vkDevice, uniformData_saturn.vkBuffer, uniformData_saturn.vkDeviceMemory, 0);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "createUniformBuffer() --> vkBindBufferMemory() is failed and error code is %d\n", vkResult);
        fflush(gpFile);
        return vkResult;
    }
    else
    {
        fprintf(gpFile, "createUniformBuffer() --> vkBindBufferMemory() is succedded\n");
        fflush(gpFile);
    }
    
    
    
    
    
    memset((void*)&uniformData_uranus, 0, sizeof(UniformData));
    
    
    memset((void*)&vkBufferCreateInfo , 0, sizeof(VkBufferCreateInfo));
    vkBufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    vkBufferCreateInfo.pNext = NULL;
    
    vkBufferCreateInfo.flags = 0;
    vkBufferCreateInfo.size = sizeof(struct MyUniformData);
    vkBufferCreateInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
    
    
    vkResult = vkCreateBuffer(vkDevice, 
                              &vkBufferCreateInfo,
                              NULL,
                              &uniformData_uranus.vkBuffer);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "createUniformBuffer() --> vkCreateBuffer() is failed and error code is %d\n", vkResult);
        fflush(gpFile);
        return vkResult;
    }
    else
    {
        fprintf(gpFile, "createUniformBuffer() --> vkCreateBuffer() is succedded\n");
        fflush(gpFile);
    }
    
    memset((void*)&vkMemoryRequirements, 0, sizeof(VkMemoryRequirements));
   
    vkGetBufferMemoryRequirements(vkDevice, uniformData_uranus.vkBuffer, &vkMemoryRequirements);
    
    
    memset((void*)&vkMemoryAllocateInfo , 0, sizeof(VkMemoryAllocateInfo));
    vkMemoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    vkMemoryAllocateInfo.pNext = NULL; 
    vkMemoryAllocateInfo.allocationSize = vkMemoryRequirements.size;
    
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
    
    
    vkResult = vkAllocateMemory(vkDevice, &vkMemoryAllocateInfo, NULL, &uniformData_uranus.vkDeviceMemory);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "createUniformBuffer() --> vkAllocateMemory() is failed and error code is %d\n", vkResult);
        fflush(gpFile);
        return vkResult;
    }
    else
    {
        fprintf(gpFile, "createUniformBuffer() --> vkAllocateMemory() is succedded\n");
        fflush(gpFile);
    }
    
    
    vkResult = vkBindBufferMemory(vkDevice, uniformData_uranus.vkBuffer, uniformData_uranus.vkDeviceMemory, 0);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "createUniformBuffer() --> vkBindBufferMemory() is failed and error code is %d\n", vkResult);
        fflush(gpFile);
        return vkResult;
    }
    else
    {
        fprintf(gpFile, "createUniformBuffer() --> vkBindBufferMemory() is succedded\n");
        fflush(gpFile);
    }
    
    
    
    
    
    memset((void*)&uniformData_neptune, 0, sizeof(UniformData));
    
    
    memset((void*)&vkBufferCreateInfo , 0, sizeof(VkBufferCreateInfo));
    vkBufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    vkBufferCreateInfo.pNext = NULL;
    
    vkBufferCreateInfo.flags = 0;
    vkBufferCreateInfo.size = sizeof(struct MyUniformData);
    vkBufferCreateInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
    
    
    vkResult = vkCreateBuffer(vkDevice, 
                              &vkBufferCreateInfo,
                              NULL,
                              &uniformData_neptune.vkBuffer);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "createUniformBuffer() --> vkCreateBuffer() is failed and error code is %d\n", vkResult);
        fflush(gpFile);
        return vkResult;
    }
    else
    {
        fprintf(gpFile, "createUniformBuffer() --> vkCreateBuffer() is succedded\n");
        fflush(gpFile);
    }
    
    memset((void*)&vkMemoryRequirements, 0, sizeof(VkMemoryRequirements));
   
    vkGetBufferMemoryRequirements(vkDevice, uniformData_neptune.vkBuffer, &vkMemoryRequirements);
    
    
    memset((void*)&vkMemoryAllocateInfo , 0, sizeof(VkMemoryAllocateInfo));
    vkMemoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    vkMemoryAllocateInfo.pNext = NULL; 
    vkMemoryAllocateInfo.allocationSize = vkMemoryRequirements.size;
    
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
    
    
    vkResult = vkAllocateMemory(vkDevice, &vkMemoryAllocateInfo, NULL, &uniformData_neptune.vkDeviceMemory);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "createUniformBuffer() --> vkAllocateMemory() is failed and error code is %d\n", vkResult);
        fflush(gpFile);
        return vkResult;
    }
    else
    {
        fprintf(gpFile, "createUniformBuffer() --> vkAllocateMemory() is succedded\n");
        fflush(gpFile);
    }
    
    
    vkResult = vkBindBufferMemory(vkDevice, uniformData_neptune.vkBuffer, uniformData_neptune.vkDeviceMemory, 0);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "createUniformBuffer() --> vkBindBufferMemory() is failed and error code is %d\n", vkResult);
        fflush(gpFile);
        return vkResult;
    }
    else
    {
        fprintf(gpFile, "createUniformBuffer() --> vkBindBufferMemory() is succedded\n");
        fflush(gpFile);
    }
    
    
    
    vkResult = updateUniformBuffer();
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "createUniformBuffer() --> updateUniformBuffer() is failed and error code is %d\n", vkResult);
        fflush(gpFile);
        return vkResult;
    }
    else
    {
        fprintf(gpFile, "createUniformBuffer() --> updateUniformBuffer() is succedded\n");
        fflush(gpFile);
    }
    
    return vkResult;
}

VkResult updateUniformBuffer(void)
{
    VkResult vkResult = VK_SUCCESS;

    // ===================== READABILITY TUNABLES =====================
    // Freeze orbits to keep the equal spacing layout stable (set false to re-enable orbiting).
    const bool  kFreezeOrbits   = true;

    // Visual sizes (independent of any "real physics"):
    const float kSunBase        = 420.0f;     // world units
    const float kSunScaleGlobal = 1.0f;       // scale Sun only
    const float kPlanetScaleGlobal = 1.0f;    // scale all planets together

    // Designer planet radii in world units (feel free to tweak these).
    // Using similar large sizes for clarity; you can individualize.
    const float planetBase[8] = { 260.0f, 260.0f, 260.0f, 260.0f, 260.0f, 260.0f, 260.0f, 260.0f };

    // How much horizontal width the Sun should occupy (in NDC radius).
    // r_ndc = 0.35 means Sun takes 70% of the viewport height if centered,
    // but here we anchor its LEFT rim at -1 so it "hugs" the left edge.
    const float kSunNdcRadius   = 0.35f;

    // Target equal gap between silhouettes (edge-to-edge) in NDC units.
    // 0.06 ≈ leave 6% of the screen width as each gap, including margins.
    const float kTargetGapNDC   = 0.06f;

    // ===================== PROJECTION (P00 only depends on fovy/aspect) =====================
    const float fovy   = glm::radians(45.0f);
    const float aspect = (float)winWidth / (float)winHeight;

    // Far plane will be refined later; P00 doesn't depend on far.
    glm::mat4 projection = glm::perspective(fovy, aspect, 0.1f, 10000.0f);
    projection[1][1] *= -1.0f;  // GLM/Vulkan fix
    const float P00 = projection[0][0];

    // ===================== SUN SETUP (anchored to the left edge) =====================
    const float kSunScale = kSunBase * kSunScaleGlobal;

    // Choose sunZ so its projected radius equals kSunNdcRadius.
    const float sunZ = -(kSunScale * P00) / kSunNdcRadius;

    // Sun center in NDC = -1 + r_ndc; convert to world X.
    const float sunCenterNdc = -1.0f + kSunNdcRadius;
    const float sunX = sunCenterNdc * (-sunZ) / P00;

    const glm::vec3 SUN_POS(sunX, 0.0f, sunZ);

    // Right edge of the Sun in NDC:
    const float sunRightNdc = -1.0f + 2.0f * kSunNdcRadius;

    // NDC <-> world X scale at z = sunZ:
    const float ndc_to_world = (-sunZ) / P00;
    const float world_to_ndc = 1.0f / ndc_to_world; // == P00/(-sunZ)

    // ===================== PLANET SIZE AUTO-FIT ACROSS VIEWPORT =====================
    // Raw world radii before auto-fit.
    float kScale[8];
    for (int i = 0; i < 8; ++i) kScale[i] = planetBase[i] * kPlanetScaleGlobal;

    // Convert those radii to NDC radii (at z = sunZ) to plan layout.
    float r_ndc[8];
    float sumDiamNdc = 0.0f;
    for (int i = 0; i < 8; ++i) {
        r_ndc[i] = kScale[i] * world_to_ndc;
        sumDiamNdc += 2.0f * r_ndc[i];
    }

    // Available NDC width for planets = from Sun's right rim to +1.0.
    const float xStart = sunRightNdc;
    const float xEnd   = 1.0f;
    const float regionWidth = xEnd - xStart;

    // Use equal gaps including the left margin after the Sun and the right border.
    // There are N+1 gaps for N planets.
    const int   N = 8;
    float       g = kTargetGapNDC;

    // Auto-scale all planets uniformly so they exactly fill the region with equal gaps.
    // Available width for diameters:
    float availForPlanets = regionWidth - (N + 1) * g;

    // If sizes are too big/small, adjust them.
    if (availForPlanets <= 0.0f) {
        // If even gaps alone exceed the region, reduce gap first.
        g = glm::max(0.0f, regionWidth / float(N + 1));
        availForPlanets = 0.0f; // planets will be scaled to nearly zero if needed
    }

    float scaleNdc = (sumDiamNdc > 0.0f) ? (availForPlanets / sumDiamNdc) : 1.0f;
    // Clamp to something sane
    scaleNdc = glm::clamp(scaleNdc, 0.05f, 10.0f);

    // Apply the NDC scale to planet world radii
    for (int i = 0; i < 8; ++i) {
        r_ndc[i] *= scaleNdc;
        kScale[i] = r_ndc[i] * ndc_to_world; // back to world
    }

    // Recompute the exact gap so the whole strip fits perfectly.
    float sumDiamNdcScaled = 0.0f;
    for (int i = 0; i < 8; ++i) sumDiamNdcScaled += 2.0f * r_ndc[i];
    g = (regionWidth - sumDiamNdcScaled) / float(N + 1);

    // ===================== COMPUTE PLANET CENTERS (equal edge-to-edge gaps) =====================
    float centerNdc[8];
    float x = xStart + g + r_ndc[0]; // left margin gap, then first radius
    centerNdc[0] = x;
    for (int i = 1; i < 8; ++i) {
        x += r_ndc[i - 1] + g + r_ndc[i];   // previous radius + gap + current radius
        centerNdc[i] = x;
    }

    // Convert those NDC centers to world centers; derive orbit radii about SUN_POS.
    float orbitRadius[8];
    for (int i = 0; i < 8; ++i) {
        const float cx_world = centerNdc[i] * ndc_to_world;
        orbitRadius[i] = cx_world - SUN_POS.x; // so angle=0 puts the planet at that X
    }

    // ===================== FINAL PROJECTION WITH SAFE FAR =====================
    // Keep far comfortably beyond the Sun center depth (|sunZ|) and any size.
    const float farPlane = fabsf(sunZ) + 4.0f * (kSunScale + *std::max_element(kScale, kScale + 8)) + 500.0f;
    projection = glm::perspective(fovy, aspect, 0.1f, farPlane);
    projection[1][1] *= -1.0f;

    // ===================== UBO WRITE HELPERS =====================
    auto writeUBO = [&](const UniformData& ubo, const MyUniformData& dataObj)->VkResult {
        void* data = nullptr;
        VkResult rmap = vkMapMemory(vkDevice, ubo.vkDeviceMemory, 0, sizeof(MyUniformData), 0, &data);
        if (rmap != VK_SUCCESS) return rmap;
        memcpy(data, &dataObj, sizeof(MyUniformData));
        vkUnmapMemory(vkDevice, ubo.vkDeviceMemory);
        return VK_SUCCESS;
    };

    auto planetModel = [&](float thetaRad, float spinDeg, float radius, float scale)->glm::mat4 {
        const glm::vec3 off = { radius * cosf(thetaRad), 0.0f, radius * sinf(thetaRad) };
        const glm::mat4 T0  = glm::translate(glm::mat4(1.0f), SUN_POS);
        const glm::mat4 Tor = glm::translate(glm::mat4(1.0f), off);
        const glm::mat4 S   = glm::scale   (glm::mat4(1.0f), glm::vec3(scale));
        const glm::mat4 Ry  = glm::rotate  (glm::mat4(1.0f), glm::radians(spinDeg), glm::vec3(0.0f, 1.0f, 0.0f));
        return T0 * Tor * S * Ry;
    };

    // ===================== SUN =====================
    {
        MyUniformData u{};
        const glm::mat4 T  = glm::translate(glm::mat4(1.0f), SUN_POS);
        const glm::mat4 S  = glm::scale   (glm::mat4(1.0f), glm::vec3(kSunScale));
        const glm::mat4 Ry = glm::rotate  (glm::mat4(1.0f), glm::radians(angle), glm::vec3(0.0f, 1.0f, 0.0f));
        u.modelMatrix      = T * S * Ry;
        u.viewMatrix       = glm::mat4(1.0f);
        u.projectionMatrix = projection;
        VkResult r = writeUBO(uniformData_sun, u); if (r != VK_SUCCESS) return r;
    }

    // ===================== PLANETS =====================
    // Orbit angle: keep 0 when frozen (so they stay lined up and equally spaced).
    const float thetaFrozen = 0.0f;

    {
        MyUniformData u{};
        u.viewMatrix       = glm::mat4(1.0f);
        u.projectionMatrix = projection;

        float theta, spin;

        // Mercury
        theta = kFreezeOrbits ? thetaFrozen : glm::radians(angle_mercury);
        spin  = angle_mercury;
        u.modelMatrix = planetModel(theta, spin, orbitRadius[0], kScale[0]);
        if ((vkResult = writeUBO(uniformData_mercury, u)) != VK_SUCCESS) return vkResult;

        // Venus
        theta = kFreezeOrbits ? thetaFrozen : glm::radians(angle_venus);
        spin  = angle_venus;
        u.modelMatrix = planetModel(theta, spin, orbitRadius[1], kScale[1]);
        if ((vkResult = writeUBO(uniformData_venus, u)) != VK_SUCCESS) return vkResult;

        // Earth
        theta = kFreezeOrbits ? thetaFrozen : glm::radians(angle_earth);
        spin  = angle_earth;
        u.modelMatrix = planetModel(theta, spin, orbitRadius[2], kScale[2]);
        if ((vkResult = writeUBO(uniformData_earth, u)) != VK_SUCCESS) return vkResult;

        // Mars
        theta = kFreezeOrbits ? thetaFrozen : glm::radians(angle_mars);
        spin  = angle_mars;
        u.modelMatrix = planetModel(theta, spin, orbitRadius[3], kScale[3]);
        if ((vkResult = writeUBO(uniformData_mars, u)) != VK_SUCCESS) return vkResult;

        // Jupiter
        theta = kFreezeOrbits ? thetaFrozen : glm::radians(angle_jupitor);
        spin  = angle_jupitor;
        u.modelMatrix = planetModel(theta, spin, orbitRadius[4], kScale[4]);
        if ((vkResult = writeUBO(uniformData_jupitor, u)) != VK_SUCCESS) return vkResult;

        // Saturn
        theta = kFreezeOrbits ? thetaFrozen : glm::radians(angle_saturn);
        spin  = angle_saturn;
        u.modelMatrix = planetModel(theta, spin, orbitRadius[5], kScale[5]);
        if ((vkResult = writeUBO(uniformData_saturn, u)) != VK_SUCCESS) return vkResult;

        // Uranus
        theta = kFreezeOrbits ? thetaFrozen : glm::radians(angle_uranus);
        spin  = angle_uranus;
        u.modelMatrix = planetModel(theta, spin, orbitRadius[6], kScale[6]);
        if ((vkResult = writeUBO(uniformData_uranus, u)) != VK_SUCCESS) return vkResult;

        // Neptune
        theta = kFreezeOrbits ? thetaFrozen : glm::radians(angle_neptune);
        spin  = angle_neptune;
        u.modelMatrix = planetModel(theta, spin, orbitRadius[7], kScale[7]);
        if ((vkResult = writeUBO(uniformData_neptune, u)) != VK_SUCCESS) return vkResult;
    }

    return VK_SUCCESS;
}

VkResult createShaders(void)
{
    
    VkResult vkResult = VK_SUCCESS;
    
    
    
    const char* szFileName = "shader.vert.spv";
    FILE* fp = NULL;
    size_t size;
    
    
    fp = fopen(szFileName, "rb"); 
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
    
    
    fseek(fp, 0L, SEEK_END);
    
    
    size = ftell(fp);
    if(size == 0)
    {
        fprintf(gpFile, "createShaders() --> ftell() failed to provide size of shader.vert.spv\n");
        vkResult = VK_ERROR_INITIALIZATION_FAILED;
        return vkResult;
    }
   
    
    fseek(fp, 0L, SEEK_SET); 
    
    
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
    
    
    fclose(fp);
    
    
    VkShaderModuleCreateInfo vkShaderModuleCreateInfo;
    memset((void*)&vkShaderModuleCreateInfo, 0, sizeof(VkShaderModuleCreateInfo));
    vkShaderModuleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    vkShaderModuleCreateInfo.pNext = NULL;
    vkShaderModuleCreateInfo.flags = 0; 
    vkShaderModuleCreateInfo.codeSize = size;
    vkShaderModuleCreateInfo.pCode = (uint32_t*)shaderData;

    
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
    
    
    if(shaderData)
    {
        free(shaderData);
        shaderData = NULL;
    }
    
    fprintf(gpFile, "createShaders() --> vertex Shader module successfully created\n");
    
    
    
    szFileName = "shader.frag.spv";
    fp = NULL;
    size = 0;
    
    
    fp = fopen(szFileName, "rb"); 
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
    
    
    fseek(fp, 0L, SEEK_END);
    
    
    size = ftell(fp);
    if(size == 0)
    {
        fprintf(gpFile, "createShaders() --> ftell() failed to provide size of shader.frag.spv\n");
        vkResult = VK_ERROR_INITIALIZATION_FAILED;
        return vkResult;
    }
   
    
    fseek(fp, 0L, SEEK_SET); 
    
    
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
    
    
    fclose(fp);
    
    
    memset((void*)&vkShaderModuleCreateInfo, 0, sizeof(VkShaderModuleCreateInfo));
    vkShaderModuleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    vkShaderModuleCreateInfo.pNext = NULL;
    vkShaderModuleCreateInfo.flags = 0; 
    vkShaderModuleCreateInfo.codeSize = size;
    vkShaderModuleCreateInfo.pCode = (uint32_t*)shaderData;

    
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
    
    VkResult vkResult = VK_SUCCESS;
    
    
    
    
    
    VkDescriptorSetLayoutBinding vkDescriptorSetLayoutBinding_array[2];
    memset((void*)vkDescriptorSetLayoutBinding_array, 0, sizeof(VkDescriptorSetLayoutBinding) * _ARRAYSIZE(vkDescriptorSetLayoutBinding_array));
    
    
    vkDescriptorSetLayoutBinding_array[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    vkDescriptorSetLayoutBinding_array[0].binding = 0;  
    vkDescriptorSetLayoutBinding_array[0].descriptorCount = 1;
    vkDescriptorSetLayoutBinding_array[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;  
    vkDescriptorSetLayoutBinding_array[0].pImmutableSamplers = NULL;
    
    
    vkDescriptorSetLayoutBinding_array[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    vkDescriptorSetLayoutBinding_array[1].binding = 1;  
    vkDescriptorSetLayoutBinding_array[1].descriptorCount = 1;
    vkDescriptorSetLayoutBinding_array[1].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;  
    vkDescriptorSetLayoutBinding_array[1].pImmutableSamplers = NULL;
    
    
    VkDescriptorSetLayoutCreateInfo vkDescriptorSetLayoutCreateInfo;
    memset((void*)&vkDescriptorSetLayoutCreateInfo, 0, sizeof(VkDescriptorSetLayoutCreateInfo));
    vkDescriptorSetLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    vkDescriptorSetLayoutCreateInfo.pNext = NULL;
    vkDescriptorSetLayoutCreateInfo.flags = 0; 
    vkDescriptorSetLayoutCreateInfo.bindingCount = _ARRAYSIZE(vkDescriptorSetLayoutBinding_array); 
    vkDescriptorSetLayoutCreateInfo.pBindings = vkDescriptorSetLayoutBinding_array;
    
    vkResult = vkCreateDescriptorSetLayout(vkDevice, &vkDescriptorSetLayoutCreateInfo, NULL, &vkDescriptorSetLayout);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "createDescriptorSetLayout() --> vkCreateDescriptorSetLayour() is failed & error code is %d\n", vkResult);
        fflush(gpFile);
        return vkResult;
    }
    else
    {
        fprintf(gpFile, "createDescriptorSetLayout() --> vkCreateDescriptorSetLayour() is succedded\n");
        fflush(gpFile);
    }
    
    return (vkResult);
}

VkResult createPipelineLayout(void)
{
    
    VkResult vkResult = VK_SUCCESS;
    
    
    VkPipelineLayoutCreateInfo vkPipelineLayoutCreateInfo;
    memset((void*)&vkPipelineLayoutCreateInfo, 0, sizeof(VkPipelineLayoutCreateInfo));
    vkPipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    vkPipelineLayoutCreateInfo.pNext = NULL;
    vkPipelineLayoutCreateInfo.flags = 0; 
    vkPipelineLayoutCreateInfo.setLayoutCount = 1;
    vkPipelineLayoutCreateInfo.pSetLayouts = &vkDescriptorSetLayout;
    vkPipelineLayoutCreateInfo.pushConstantRangeCount = 0;
    vkPipelineLayoutCreateInfo.pPushConstantRanges = NULL;
    
    vkResult = vkCreatePipelineLayout(vkDevice, &vkPipelineLayoutCreateInfo, NULL, &vkPipelineLayout);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "createPipelineLayout() --> vkCreatePipelineLayout() is failed & error code is %d\n", vkResult);
        fflush(gpFile);
        return vkResult;
    }
    else
    {
        fprintf(gpFile, "createPipelineLayout() --> vkCreatePipelineLayout() is succedded\n");
        fflush(gpFile);
    }
    
    return (vkResult);
}

VkResult createDescriptorPool(void)
{
    
    VkResult vkResult = VK_SUCCESS;
    
    
    
    
    
    VkDescriptorPoolSize vkDescriptorPoolSize_array[2];
    memset((void*)vkDescriptorPoolSize_array, 0, sizeof(VkDescriptorPoolSize) * _ARRAYSIZE(vkDescriptorPoolSize_array));
    
    
    vkDescriptorPoolSize_array[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    vkDescriptorPoolSize_array[0].descriptorCount = 1;
    
    
    vkDescriptorPoolSize_array[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    vkDescriptorPoolSize_array[1].descriptorCount = 1;
    
    
    
    VkDescriptorPoolCreateInfo vkDescriptorPoolCreateInfo;
    memset((void*)&vkDescriptorPoolCreateInfo, 0, sizeof(VkDescriptorPoolCreateInfo));
    vkDescriptorPoolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    vkDescriptorPoolCreateInfo.pNext = NULL;
    vkDescriptorPoolCreateInfo.flags = 0;
    vkDescriptorPoolCreateInfo.poolSizeCount = _ARRAYSIZE(vkDescriptorPoolSize_array);
    vkDescriptorPoolCreateInfo.pPoolSizes = vkDescriptorPoolSize_array;
    vkDescriptorPoolCreateInfo.maxSets = 9;
    
    vkResult = vkCreateDescriptorPool(vkDevice, &vkDescriptorPoolCreateInfo, NULL, &vkDescriptorPool);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "createDescriptorPool() --> vkCreateDescriptorPool() is failed & error code is %d\n", vkResult);
        fflush(gpFile);
        return vkResult;
    }
    else
    {
        fprintf(gpFile, "createDescriptorPool() --> vkCreateDescriptorPool() is succedded\n");
        fflush(gpFile);
    }
    
    return (vkResult);
}


VkResult createDescriptorSet(void)
{
    
    VkResult vkResult = VK_SUCCESS;
    
    
    
    VkDescriptorSetAllocateInfo vkDescriptorSetAllocateInfo;
    memset((void*)&vkDescriptorSetAllocateInfo, 0, sizeof(VkDescriptorSetAllocateInfo));
    vkDescriptorSetAllocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    vkDescriptorSetAllocateInfo.pNext = NULL;
    vkDescriptorSetAllocateInfo.descriptorPool = vkDescriptorPool;
    vkDescriptorSetAllocateInfo.descriptorSetCount = 1;  
    vkDescriptorSetAllocateInfo.pSetLayouts = &vkDescriptorSetLayout;
    
    
    
    vkResult = vkAllocateDescriptorSets(vkDevice, &vkDescriptorSetAllocateInfo, &vkDescriptorSet_sun);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "createDescriptorSet() --> vkAllocateDescriptorSets() is failed for sun & error code is %d\n", vkResult);
        fflush(gpFile);
        return vkResult;
    }
    else
    {
        fprintf(gpFile, "createDescriptorSet() --> vkAllocateDescriptorSets() is succedded for sun\n");
        fflush(gpFile);
    }
    
    
    
    VkDescriptorBufferInfo vkDescriptorBufferInfo;
    memset((void*)&vkDescriptorBufferInfo, 0, sizeof(VkDescriptorBufferInfo));
    vkDescriptorBufferInfo.buffer = uniformData_sun.vkBuffer;
    vkDescriptorBufferInfo.offset = 0;
    vkDescriptorBufferInfo.range = sizeof(MyUniformData);
    
    
    
    VkDescriptorImageInfo vkDescriptorImageInfo;
    memset((void*)&vkDescriptorImageInfo, 0, sizeof(VkDescriptorImageInfo));
    vkDescriptorImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    vkDescriptorImageInfo.imageView = vkImageView_texture_sun;
    vkDescriptorImageInfo.sampler = vkSampler_texture_sun;
    
   
    
    
    
    
    VkWriteDescriptorSet vkWriteDescriptorSet_array[2];
    
    memset((void*)vkWriteDescriptorSet_array, 0, sizeof(VkWriteDescriptorSet) * _ARRAYSIZE(vkWriteDescriptorSet_array));

    
    vkWriteDescriptorSet_array[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    vkWriteDescriptorSet_array[0].dstSet = vkDescriptorSet_sun;
    vkWriteDescriptorSet_array[0].dstArrayElement = 0;
    vkWriteDescriptorSet_array[0].descriptorCount = 1;
    vkWriteDescriptorSet_array[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    vkWriteDescriptorSet_array[0].pBufferInfo = &vkDescriptorBufferInfo;
    vkWriteDescriptorSet_array[0].pImageInfo = NULL;
    vkWriteDescriptorSet_array[0].pTexelBufferView = NULL;
    vkWriteDescriptorSet_array[0].dstBinding = 0; 
    
    
    vkWriteDescriptorSet_array[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    vkWriteDescriptorSet_array[1].dstSet = vkDescriptorSet_sun;
    vkWriteDescriptorSet_array[1].dstArrayElement = 0;
    vkWriteDescriptorSet_array[1].descriptorCount = 1;
    vkWriteDescriptorSet_array[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    vkWriteDescriptorSet_array[1].pBufferInfo = NULL;
    vkWriteDescriptorSet_array[1].pImageInfo = &vkDescriptorImageInfo;
    vkWriteDescriptorSet_array[1].pTexelBufferView = NULL;
    vkWriteDescriptorSet_array[1].dstBinding = 1; 
    
    
    vkUpdateDescriptorSets(vkDevice, _ARRAYSIZE(vkWriteDescriptorSet_array), vkWriteDescriptorSet_array, 0, NULL);    
    fprintf(gpFile, "createDescriptorSet() --> vkUpdateDescriptorSets() is succedded\n");
    fflush(gpFile);
    
    
    

    
    vkResult = vkAllocateDescriptorSets(vkDevice, &vkDescriptorSetAllocateInfo, &vkDescriptorSet_mercury);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "createDescriptorSet() --> vkAllocateDescriptorSets() is failed for mercury & error code is %d\n", vkResult);
        fflush(gpFile);
        return vkResult;
    }
    else
    {
        fprintf(gpFile, "createDescriptorSet() --> vkAllocateDescriptorSets() is succedded for mercury\n");
        fflush(gpFile);
    }
    
    
    
    memset((void*)&vkDescriptorBufferInfo, 0, sizeof(VkDescriptorBufferInfo));
    vkDescriptorBufferInfo.buffer = uniformData_mercury.vkBuffer;
    vkDescriptorBufferInfo.offset = 0;
    vkDescriptorBufferInfo.range = sizeof(MyUniformData);
    
    
    
    memset((void*)&vkDescriptorImageInfo, 0, sizeof(VkDescriptorImageInfo));
    vkDescriptorImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    vkDescriptorImageInfo.imageView = vkImageView_texture_mercury;
    vkDescriptorImageInfo.sampler = vkSampler_texture_mercury;
    
   
    
    
    
    
    
    memset((void*)vkWriteDescriptorSet_array, 0, sizeof(VkWriteDescriptorSet) * _ARRAYSIZE(vkWriteDescriptorSet_array));

    
    vkWriteDescriptorSet_array[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    vkWriteDescriptorSet_array[0].dstSet = vkDescriptorSet_mercury;
    vkWriteDescriptorSet_array[0].dstArrayElement = 0;
    vkWriteDescriptorSet_array[0].descriptorCount = 1;
    vkWriteDescriptorSet_array[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    vkWriteDescriptorSet_array[0].pBufferInfo = &vkDescriptorBufferInfo;
    vkWriteDescriptorSet_array[0].pImageInfo = NULL;
    vkWriteDescriptorSet_array[0].pTexelBufferView = NULL;
    vkWriteDescriptorSet_array[0].dstBinding = 0; 
    
    
    vkWriteDescriptorSet_array[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    vkWriteDescriptorSet_array[1].dstSet = vkDescriptorSet_mercury;
    vkWriteDescriptorSet_array[1].dstArrayElement = 0;
    vkWriteDescriptorSet_array[1].descriptorCount = 1;
    vkWriteDescriptorSet_array[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    vkWriteDescriptorSet_array[1].pBufferInfo = NULL;
    vkWriteDescriptorSet_array[1].pImageInfo = &vkDescriptorImageInfo;
    vkWriteDescriptorSet_array[1].pTexelBufferView = NULL;
    vkWriteDescriptorSet_array[1].dstBinding = 1; 
    
    
    vkUpdateDescriptorSets(vkDevice, _ARRAYSIZE(vkWriteDescriptorSet_array), vkWriteDescriptorSet_array, 0, NULL);    
    fprintf(gpFile, "createDescriptorSet() --> vkUpdateDescriptorSets() is succedded\n");
    fflush(gpFile);
    
    
    
    

    
    vkResult = vkAllocateDescriptorSets(vkDevice, &vkDescriptorSetAllocateInfo, &vkDescriptorSet_venus);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "createDescriptorSet() --> vkAllocateDescriptorSets() is failed for venus & error code is %d\n", vkResult);
        fflush(gpFile);
        return vkResult;
    }
    else
    {
        fprintf(gpFile, "createDescriptorSet() --> vkAllocateDescriptorSets() is succedded venus\n");
        fflush(gpFile);
    }
    
    
    
    memset((void*)&vkDescriptorBufferInfo, 0, sizeof(VkDescriptorBufferInfo));
    vkDescriptorBufferInfo.buffer = uniformData_venus.vkBuffer;
    vkDescriptorBufferInfo.offset = 0;
    vkDescriptorBufferInfo.range = sizeof(MyUniformData);
    
    
    
    memset((void*)&vkDescriptorImageInfo, 0, sizeof(VkDescriptorImageInfo));
    vkDescriptorImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    vkDescriptorImageInfo.imageView = vkImageView_texture_venus;
    vkDescriptorImageInfo.sampler = vkSampler_texture_venus;
    
   
    
    
    
    
    
    memset((void*)vkWriteDescriptorSet_array, 0, sizeof(VkWriteDescriptorSet) * _ARRAYSIZE(vkWriteDescriptorSet_array));

    
    vkWriteDescriptorSet_array[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    vkWriteDescriptorSet_array[0].dstSet = vkDescriptorSet_venus;
    vkWriteDescriptorSet_array[0].dstArrayElement = 0;
    vkWriteDescriptorSet_array[0].descriptorCount = 1;
    vkWriteDescriptorSet_array[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    vkWriteDescriptorSet_array[0].pBufferInfo = &vkDescriptorBufferInfo;
    vkWriteDescriptorSet_array[0].pImageInfo = NULL;
    vkWriteDescriptorSet_array[0].pTexelBufferView = NULL;
    vkWriteDescriptorSet_array[0].dstBinding = 0; 
    
    
    vkWriteDescriptorSet_array[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    vkWriteDescriptorSet_array[1].dstSet = vkDescriptorSet_venus;
    vkWriteDescriptorSet_array[1].dstArrayElement = 0;
    vkWriteDescriptorSet_array[1].descriptorCount = 1;
    vkWriteDescriptorSet_array[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    vkWriteDescriptorSet_array[1].pBufferInfo = NULL;
    vkWriteDescriptorSet_array[1].pImageInfo = &vkDescriptorImageInfo;
    vkWriteDescriptorSet_array[1].pTexelBufferView = NULL;
    vkWriteDescriptorSet_array[1].dstBinding = 1; 
    
    
    vkUpdateDescriptorSets(vkDevice, _ARRAYSIZE(vkWriteDescriptorSet_array), vkWriteDescriptorSet_array, 0, NULL);    
    fprintf(gpFile, "createDescriptorSet() --> vkUpdateDescriptorSets() is succedded\n");
    fflush(gpFile);
    
    
    

    
    vkResult = vkAllocateDescriptorSets(vkDevice, &vkDescriptorSetAllocateInfo, &vkDescriptorSet_earth);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "createDescriptorSet() --> vkAllocateDescriptorSets() is failed for earth & error code is %d\n", vkResult);
        fflush(gpFile);
        return vkResult;
    }
    else
    {
        fprintf(gpFile, "createDescriptorSet() --> vkAllocateDescriptorSets() is succedded for earth\n");
        fflush(gpFile);
    }
    
    
    
    memset((void*)&vkDescriptorBufferInfo, 0, sizeof(VkDescriptorBufferInfo));
    vkDescriptorBufferInfo.buffer = uniformData_earth.vkBuffer;
    vkDescriptorBufferInfo.offset = 0;
    vkDescriptorBufferInfo.range = sizeof(MyUniformData);
    
    
    
    memset((void*)&vkDescriptorImageInfo, 0, sizeof(VkDescriptorImageInfo));
    vkDescriptorImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    vkDescriptorImageInfo.imageView = vkImageView_texture_earth;
    vkDescriptorImageInfo.sampler = vkSampler_texture_earth;
    
   
    
    
    
    
    
    memset((void*)vkWriteDescriptorSet_array, 0, sizeof(VkWriteDescriptorSet) * _ARRAYSIZE(vkWriteDescriptorSet_array));

    
    vkWriteDescriptorSet_array[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    vkWriteDescriptorSet_array[0].dstSet = vkDescriptorSet_earth;
    vkWriteDescriptorSet_array[0].dstArrayElement = 0;
    vkWriteDescriptorSet_array[0].descriptorCount = 1;
    vkWriteDescriptorSet_array[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    vkWriteDescriptorSet_array[0].pBufferInfo = &vkDescriptorBufferInfo;
    vkWriteDescriptorSet_array[0].pImageInfo = NULL;
    vkWriteDescriptorSet_array[0].pTexelBufferView = NULL;
    vkWriteDescriptorSet_array[0].dstBinding = 0; 
    
    
    vkWriteDescriptorSet_array[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    vkWriteDescriptorSet_array[1].dstSet = vkDescriptorSet_earth;
    vkWriteDescriptorSet_array[1].dstArrayElement = 0;
    vkWriteDescriptorSet_array[1].descriptorCount = 1;
    vkWriteDescriptorSet_array[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    vkWriteDescriptorSet_array[1].pBufferInfo = NULL;
    vkWriteDescriptorSet_array[1].pImageInfo = &vkDescriptorImageInfo;
    vkWriteDescriptorSet_array[1].pTexelBufferView = NULL;
    vkWriteDescriptorSet_array[1].dstBinding = 1; 
    
    
    vkUpdateDescriptorSets(vkDevice, _ARRAYSIZE(vkWriteDescriptorSet_array), vkWriteDescriptorSet_array, 0, NULL);    
    fprintf(gpFile, "createDescriptorSet() --> vkUpdateDescriptorSets() is succedded\n");
    fflush(gpFile);
    
    
    

    
    vkResult = vkAllocateDescriptorSets(vkDevice, &vkDescriptorSetAllocateInfo, &vkDescriptorSet_mars);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "createDescriptorSet() --> vkAllocateDescriptorSets() is failed for mars & error code is %d\n", vkResult);
        fflush(gpFile);
        return vkResult;
    }
    else
    {
        fprintf(gpFile, "createDescriptorSet() --> vkAllocateDescriptorSets() is succedded for mars\n");
        fflush(gpFile);
    }
    
    
    
    memset((void*)&vkDescriptorBufferInfo, 0, sizeof(VkDescriptorBufferInfo));
    vkDescriptorBufferInfo.buffer = uniformData_mars.vkBuffer;
    vkDescriptorBufferInfo.offset = 0;
    vkDescriptorBufferInfo.range = sizeof(MyUniformData);
    
    
    
    memset((void*)&vkDescriptorImageInfo, 0, sizeof(VkDescriptorImageInfo));
    vkDescriptorImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    vkDescriptorImageInfo.imageView = vkImageView_texture_mars;
    vkDescriptorImageInfo.sampler = vkSampler_texture_mars;
    
   
    
    
    
    
    
    memset((void*)vkWriteDescriptorSet_array, 0, sizeof(VkWriteDescriptorSet) * _ARRAYSIZE(vkWriteDescriptorSet_array));

    
    vkWriteDescriptorSet_array[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    vkWriteDescriptorSet_array[0].dstSet = vkDescriptorSet_mars;
    vkWriteDescriptorSet_array[0].dstArrayElement = 0;
    vkWriteDescriptorSet_array[0].descriptorCount = 1;
    vkWriteDescriptorSet_array[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    vkWriteDescriptorSet_array[0].pBufferInfo = &vkDescriptorBufferInfo;
    vkWriteDescriptorSet_array[0].pImageInfo = NULL;
    vkWriteDescriptorSet_array[0].pTexelBufferView = NULL;
    vkWriteDescriptorSet_array[0].dstBinding = 0; 
    
    
    vkWriteDescriptorSet_array[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    vkWriteDescriptorSet_array[1].dstSet = vkDescriptorSet_mars;
    vkWriteDescriptorSet_array[1].dstArrayElement = 0;
    vkWriteDescriptorSet_array[1].descriptorCount = 1;
    vkWriteDescriptorSet_array[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    vkWriteDescriptorSet_array[1].pBufferInfo = NULL;
    vkWriteDescriptorSet_array[1].pImageInfo = &vkDescriptorImageInfo;
    vkWriteDescriptorSet_array[1].pTexelBufferView = NULL;
    vkWriteDescriptorSet_array[1].dstBinding = 1; 
    
    
    vkUpdateDescriptorSets(vkDevice, _ARRAYSIZE(vkWriteDescriptorSet_array), vkWriteDescriptorSet_array, 0, NULL);    
    fprintf(gpFile, "createDescriptorSet() --> vkUpdateDescriptorSets() is succedded\n");
        fflush(gpFile);
    
    
    

    
    vkResult = vkAllocateDescriptorSets(vkDevice, &vkDescriptorSetAllocateInfo, &vkDescriptorSet_jupitor);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "createDescriptorSet() --> vkAllocateDescriptorSets() is failed for jupitor & error code is %d\n", vkResult);
        fflush(gpFile);
        return vkResult;
    }
    else
    {
        fprintf(gpFile, "createDescriptorSet() --> vkAllocateDescriptorSets() is succedded for jupitor\n");
        fflush(gpFile);
    }
    
    
    
    memset((void*)&vkDescriptorBufferInfo, 0, sizeof(VkDescriptorBufferInfo));
    vkDescriptorBufferInfo.buffer = uniformData_jupitor.vkBuffer;
    vkDescriptorBufferInfo.offset = 0;
    vkDescriptorBufferInfo.range = sizeof(MyUniformData);
    
    
    
    memset((void*)&vkDescriptorImageInfo, 0, sizeof(VkDescriptorImageInfo));
    vkDescriptorImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    vkDescriptorImageInfo.imageView = vkImageView_texture_jupitor;
    vkDescriptorImageInfo.sampler = vkSampler_texture_jupitor;
    
   
    
    
    
    
    
    memset((void*)vkWriteDescriptorSet_array, 0, sizeof(VkWriteDescriptorSet) * _ARRAYSIZE(vkWriteDescriptorSet_array));

    
    vkWriteDescriptorSet_array[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    vkWriteDescriptorSet_array[0].dstSet = vkDescriptorSet_jupitor;
    vkWriteDescriptorSet_array[0].dstArrayElement = 0;
    vkWriteDescriptorSet_array[0].descriptorCount = 1;
    vkWriteDescriptorSet_array[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    vkWriteDescriptorSet_array[0].pBufferInfo = &vkDescriptorBufferInfo;
    vkWriteDescriptorSet_array[0].pImageInfo = NULL;
    vkWriteDescriptorSet_array[0].pTexelBufferView = NULL;
    vkWriteDescriptorSet_array[0].dstBinding = 0; 
    
    
    vkWriteDescriptorSet_array[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    vkWriteDescriptorSet_array[1].dstSet = vkDescriptorSet_jupitor;
    vkWriteDescriptorSet_array[1].dstArrayElement = 0;
    vkWriteDescriptorSet_array[1].descriptorCount = 1;
    vkWriteDescriptorSet_array[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    vkWriteDescriptorSet_array[1].pBufferInfo = NULL;
    vkWriteDescriptorSet_array[1].pImageInfo = &vkDescriptorImageInfo;
    vkWriteDescriptorSet_array[1].pTexelBufferView = NULL;
    vkWriteDescriptorSet_array[1].dstBinding = 1; 
    
    
    vkUpdateDescriptorSets(vkDevice, _ARRAYSIZE(vkWriteDescriptorSet_array), vkWriteDescriptorSet_array, 0, NULL);    
    fprintf(gpFile, "createDescriptorSet() --> vkUpdateDescriptorSets() is succedded\n");
        fflush(gpFile);
    
    
    

    
    vkResult = vkAllocateDescriptorSets(vkDevice, &vkDescriptorSetAllocateInfo, &vkDescriptorSet_saturn);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "createDescriptorSet() --> vkAllocateDescriptorSets() is failed for saturn & error code is %d\n", vkResult);
        fflush(gpFile);
        return vkResult;
    }
    else
    {
        fprintf(gpFile, "createDescriptorSet() --> vkAllocateDescriptorSets() is succedded\n");
        fflush(gpFile);
    }
    
    
    
    memset((void*)&vkDescriptorBufferInfo, 0, sizeof(VkDescriptorBufferInfo));
    vkDescriptorBufferInfo.buffer = uniformData_saturn.vkBuffer;
    vkDescriptorBufferInfo.offset = 0;
    vkDescriptorBufferInfo.range = sizeof(MyUniformData);
    
    
    
    memset((void*)&vkDescriptorImageInfo, 0, sizeof(VkDescriptorImageInfo));
    vkDescriptorImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    vkDescriptorImageInfo.imageView = vkImageView_texture_saturn;
    vkDescriptorImageInfo.sampler = vkSampler_texture_saturn;
    
   
    
    
    
    
    
    memset((void*)vkWriteDescriptorSet_array, 0, sizeof(VkWriteDescriptorSet) * _ARRAYSIZE(vkWriteDescriptorSet_array));

    
    vkWriteDescriptorSet_array[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    vkWriteDescriptorSet_array[0].dstSet = vkDescriptorSet_saturn;
    vkWriteDescriptorSet_array[0].dstArrayElement = 0;
    vkWriteDescriptorSet_array[0].descriptorCount = 1;
    vkWriteDescriptorSet_array[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    vkWriteDescriptorSet_array[0].pBufferInfo = &vkDescriptorBufferInfo;
    vkWriteDescriptorSet_array[0].pImageInfo = NULL;
    vkWriteDescriptorSet_array[0].pTexelBufferView = NULL;
    vkWriteDescriptorSet_array[0].dstBinding = 0; 
    
    
    vkWriteDescriptorSet_array[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    vkWriteDescriptorSet_array[1].dstSet = vkDescriptorSet_saturn;
    vkWriteDescriptorSet_array[1].dstArrayElement = 0;
    vkWriteDescriptorSet_array[1].descriptorCount = 1;
    vkWriteDescriptorSet_array[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    vkWriteDescriptorSet_array[1].pBufferInfo = NULL;
    vkWriteDescriptorSet_array[1].pImageInfo = &vkDescriptorImageInfo;
    vkWriteDescriptorSet_array[1].pTexelBufferView = NULL;
    vkWriteDescriptorSet_array[1].dstBinding = 1; 
    
    
    vkUpdateDescriptorSets(vkDevice, _ARRAYSIZE(vkWriteDescriptorSet_array), vkWriteDescriptorSet_array, 0, NULL);    
    fprintf(gpFile, "createDescriptorSet() --> vkUpdateDescriptorSets() is succedded\n");
        fflush(gpFile);
    
    
    

    
    vkResult = vkAllocateDescriptorSets(vkDevice, &vkDescriptorSetAllocateInfo, &vkDescriptorSet_uranus);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "createDescriptorSet() --> vkAllocateDescriptorSets() is failed for uranus & error code is %d\n", vkResult);
        fflush(gpFile);
        return vkResult;
    }
    else
    {
        fprintf(gpFile, "createDescriptorSet() --> vkAllocateDescriptorSets() is succedded\n");
        fflush(gpFile);
    }
    
    
    
    memset((void*)&vkDescriptorBufferInfo, 0, sizeof(VkDescriptorBufferInfo));
    vkDescriptorBufferInfo.buffer = uniformData_uranus.vkBuffer;
    vkDescriptorBufferInfo.offset = 0;
    vkDescriptorBufferInfo.range = sizeof(MyUniformData);
    
    
    
    memset((void*)&vkDescriptorImageInfo, 0, sizeof(VkDescriptorImageInfo));
    vkDescriptorImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    vkDescriptorImageInfo.imageView = vkImageView_texture_uranus;
    vkDescriptorImageInfo.sampler = vkSampler_texture_uranus;
    
   
    
    
    
    
    
    memset((void*)vkWriteDescriptorSet_array, 0, sizeof(VkWriteDescriptorSet) * _ARRAYSIZE(vkWriteDescriptorSet_array));

    
    vkWriteDescriptorSet_array[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    vkWriteDescriptorSet_array[0].dstSet = vkDescriptorSet_uranus;
    vkWriteDescriptorSet_array[0].dstArrayElement = 0;
    vkWriteDescriptorSet_array[0].descriptorCount = 1;
    vkWriteDescriptorSet_array[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    vkWriteDescriptorSet_array[0].pBufferInfo = &vkDescriptorBufferInfo;
    vkWriteDescriptorSet_array[0].pImageInfo = NULL;
    vkWriteDescriptorSet_array[0].pTexelBufferView = NULL;
    vkWriteDescriptorSet_array[0].dstBinding = 0; 
    
    
    vkWriteDescriptorSet_array[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    vkWriteDescriptorSet_array[1].dstSet = vkDescriptorSet_uranus;
    vkWriteDescriptorSet_array[1].dstArrayElement = 0;
    vkWriteDescriptorSet_array[1].descriptorCount = 1;
    vkWriteDescriptorSet_array[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    vkWriteDescriptorSet_array[1].pBufferInfo = NULL;
    vkWriteDescriptorSet_array[1].pImageInfo = &vkDescriptorImageInfo;
    vkWriteDescriptorSet_array[1].pTexelBufferView = NULL;
    vkWriteDescriptorSet_array[1].dstBinding = 1; 
    
    
    vkUpdateDescriptorSets(vkDevice, _ARRAYSIZE(vkWriteDescriptorSet_array), vkWriteDescriptorSet_array, 0, NULL);    
    fprintf(gpFile, "createDescriptorSet() --> vkUpdateDescriptorSets() is succedded\n");
        fflush(gpFile);
    
    
    

    
    vkResult = vkAllocateDescriptorSets(vkDevice, &vkDescriptorSetAllocateInfo, &vkDescriptorSet_neptune);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "createDescriptorSet() --> vkAllocateDescriptorSets() is failed for neptune & error code is %d\n", vkResult);
        fflush(gpFile);
        return vkResult;
    }
    else
    {
        fprintf(gpFile, "createDescriptorSet() --> vkAllocateDescriptorSets() is succedded\n");
        fflush(gpFile);
    }
    
    
    
    memset((void*)&vkDescriptorBufferInfo, 0, sizeof(VkDescriptorBufferInfo));
    vkDescriptorBufferInfo.buffer = uniformData_neptune.vkBuffer;
    vkDescriptorBufferInfo.offset = 0;
    vkDescriptorBufferInfo.range = sizeof(MyUniformData);
    
    
    
    memset((void*)&vkDescriptorImageInfo, 0, sizeof(VkDescriptorImageInfo));
    vkDescriptorImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    vkDescriptorImageInfo.imageView = vkImageView_texture_neptune;
    vkDescriptorImageInfo.sampler = vkSampler_texture_neptune;
    
   
    
    
    
    
    
    memset((void*)vkWriteDescriptorSet_array, 0, sizeof(VkWriteDescriptorSet) * _ARRAYSIZE(vkWriteDescriptorSet_array));

    
    vkWriteDescriptorSet_array[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    vkWriteDescriptorSet_array[0].dstSet = vkDescriptorSet_neptune;
    vkWriteDescriptorSet_array[0].dstArrayElement = 0;
    vkWriteDescriptorSet_array[0].descriptorCount = 1;
    vkWriteDescriptorSet_array[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    vkWriteDescriptorSet_array[0].pBufferInfo = &vkDescriptorBufferInfo;
    vkWriteDescriptorSet_array[0].pImageInfo = NULL;
    vkWriteDescriptorSet_array[0].pTexelBufferView = NULL;
    vkWriteDescriptorSet_array[0].dstBinding = 0; 
    
    
    vkWriteDescriptorSet_array[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    vkWriteDescriptorSet_array[1].dstSet = vkDescriptorSet_neptune;
    vkWriteDescriptorSet_array[1].dstArrayElement = 0;
    vkWriteDescriptorSet_array[1].descriptorCount = 1;
    vkWriteDescriptorSet_array[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    vkWriteDescriptorSet_array[1].pBufferInfo = NULL;
    vkWriteDescriptorSet_array[1].pImageInfo = &vkDescriptorImageInfo;
    vkWriteDescriptorSet_array[1].pTexelBufferView = NULL;
    vkWriteDescriptorSet_array[1].dstBinding = 1; 
    
    
    vkUpdateDescriptorSets(vkDevice, _ARRAYSIZE(vkWriteDescriptorSet_array), vkWriteDescriptorSet_array, 0, NULL);    
    fprintf(gpFile, "createDescriptorSet() --> vkUpdateDescriptorSets() is succedded\n");
        fflush(gpFile);
    
    return (vkResult);
}

VkResult createRenderPass(void)
{
    
    VkResult vkResult = VK_SUCCESS;
    
    
    
    VkAttachmentDescription vkAttachmentDescription_array[2];  
    memset((void*)vkAttachmentDescription_array, 0, sizeof(VkAttachmentDescription) * _ARRAYSIZE(vkAttachmentDescription_array));
    
    
    vkAttachmentDescription_array[0].flags = 0; 
    vkAttachmentDescription_array[0].format = vkFormat_color;
    vkAttachmentDescription_array[0].samples = VK_SAMPLE_COUNT_1_BIT;
    vkAttachmentDescription_array[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    vkAttachmentDescription_array[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    vkAttachmentDescription_array[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    vkAttachmentDescription_array[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    vkAttachmentDescription_array[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    vkAttachmentDescription_array[0].finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    
    
    vkAttachmentDescription_array[1].flags = 0; 
    vkAttachmentDescription_array[1].format = vkFormat_depth;
    vkAttachmentDescription_array[1].samples = VK_SAMPLE_COUNT_1_BIT;
    vkAttachmentDescription_array[1].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    vkAttachmentDescription_array[1].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    vkAttachmentDescription_array[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    vkAttachmentDescription_array[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    vkAttachmentDescription_array[1].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    vkAttachmentDescription_array[1].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    

    
    
    VkAttachmentReference vkAttachmentReference_color;
    memset((void*)&vkAttachmentReference_color, 0, sizeof(VkAttachmentReference));
    
    vkAttachmentReference_color.attachment = 0;  
    vkAttachmentReference_color.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL; 
    
    
    VkAttachmentReference vkAttachmentReference_depth;
    memset((void*)&vkAttachmentReference_depth, 0, sizeof(VkAttachmentReference));
    
    vkAttachmentReference_depth.attachment = 1;  
    vkAttachmentReference_depth.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL; 
    
    
    
    VkSubpassDescription vkSubpassDescription;
    memset((void*)&vkSubpassDescription, 0, sizeof(VkSubpassDescription));
    
    vkSubpassDescription.flags = 0;
    vkSubpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    vkSubpassDescription.inputAttachmentCount = 0;
    vkSubpassDescription.pInputAttachments = NULL;
    
    vkSubpassDescription.colorAttachmentCount = 1;  
    vkSubpassDescription.pColorAttachments = &vkAttachmentReference_color;
    vkSubpassDescription.pResolveAttachments = NULL;
    vkSubpassDescription.pDepthStencilAttachment = &vkAttachmentReference_depth;
    vkSubpassDescription.preserveAttachmentCount = 0;
    vkSubpassDescription.pPreserveAttachments = NULL;
    
    
    VkRenderPassCreateInfo vkRenderPassCreateInfo;
    memset((void*)&vkRenderPassCreateInfo, 0, sizeof(VkRenderPassCreateInfo));
    
    vkRenderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    vkRenderPassCreateInfo.pNext = NULL;
    vkRenderPassCreateInfo.flags = 0;
    vkRenderPassCreateInfo.attachmentCount = _ARRAYSIZE(vkAttachmentDescription_array);
    vkRenderPassCreateInfo.pAttachments = vkAttachmentDescription_array;
    vkRenderPassCreateInfo.subpassCount = 1;
    vkRenderPassCreateInfo.pSubpasses = &vkSubpassDescription;
    vkRenderPassCreateInfo.dependencyCount = 0;
    vkRenderPassCreateInfo.pDependencies = NULL;
    
    
    
    vkResult = vkCreateRenderPass(vkDevice, 
                                  &vkRenderPassCreateInfo,
                                  NULL,
                                  &vkRenderPass);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "createRenderPass() --> vkCreateRenderPass() is failed & error code is %d\n", vkResult);
        fflush(gpFile);
        return vkResult;
    }
    else
    {
        fprintf(gpFile, "createRenderPass() --> vkCreateRenderPass() is succedded\n");
        fflush(gpFile);
    }
    
    return (vkResult);
}

VkResult createPipeline(void)
{
    
    VkResult vkResult = VK_SUCCESS;
    
    
    
    VkVertexInputBindingDescription vkVertexInputBindingDescription_array[3];  
    memset((void*)vkVertexInputBindingDescription_array, 0, sizeof(VkVertexInputBindingDescription) * _ARRAYSIZE(vkVertexInputBindingDescription_array));
    
    vkVertexInputBindingDescription_array[0].binding = 0;
    vkVertexInputBindingDescription_array[0].stride = sizeof(float) * 3;
    vkVertexInputBindingDescription_array[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
    
    
    vkVertexInputBindingDescription_array[1].binding = 1;
    vkVertexInputBindingDescription_array[1].stride = sizeof(float) * 3;
    vkVertexInputBindingDescription_array[1].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
    
    
    vkVertexInputBindingDescription_array[2].binding = 2;
    vkVertexInputBindingDescription_array[2].stride = sizeof(float) * 2;
    vkVertexInputBindingDescription_array[2].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    
    VkVertexInputAttributeDescription vkVertexInputAttributeDescription_array[3]; 
    memset((void*)vkVertexInputAttributeDescription_array, 0, sizeof(VkVertexInputAttributeDescription) * _ARRAYSIZE(vkVertexInputAttributeDescription_array));
    
    
    vkVertexInputAttributeDescription_array[0].binding = 0;
    vkVertexInputAttributeDescription_array[0].location = 0;
    vkVertexInputAttributeDescription_array[0].format = VK_FORMAT_R32G32B32_SFLOAT;
    vkVertexInputAttributeDescription_array[0].offset = 0;
    
    
    vkVertexInputAttributeDescription_array[1].binding = 1;
    vkVertexInputAttributeDescription_array[1].location = 1;
    vkVertexInputAttributeDescription_array[1].format = VK_FORMAT_R32G32B32_SFLOAT;
    vkVertexInputAttributeDescription_array[1].offset = 0;
    
    
    vkVertexInputAttributeDescription_array[2].binding = 2;
    vkVertexInputAttributeDescription_array[2].location = 2;
    vkVertexInputAttributeDescription_array[2].format = VK_FORMAT_R32G32_SFLOAT;
    vkVertexInputAttributeDescription_array[2].offset = 0;
    
    
    
    
    VkPipelineVertexInputStateCreateInfo vkPipelineVertexInputStateCreateInfo;
    memset((void*)&vkPipelineVertexInputStateCreateInfo, 0, sizeof(VkPipelineVertexInputStateCreateInfo));
    vkPipelineVertexInputStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vkPipelineVertexInputStateCreateInfo.pNext = NULL;
    vkPipelineVertexInputStateCreateInfo.flags = 0;
    vkPipelineVertexInputStateCreateInfo.vertexBindingDescriptionCount = _ARRAYSIZE(vkVertexInputBindingDescription_array);
    vkPipelineVertexInputStateCreateInfo.pVertexBindingDescriptions = vkVertexInputBindingDescription_array;
    vkPipelineVertexInputStateCreateInfo.vertexAttributeDescriptionCount = _ARRAYSIZE(vkVertexInputAttributeDescription_array);
    vkPipelineVertexInputStateCreateInfo.pVertexAttributeDescriptions = vkVertexInputAttributeDescription_array;
    
    
    
    VkPipelineInputAssemblyStateCreateInfo vkPipelineInputAssemblyStateCreateInfo;
    memset((void*)&vkPipelineInputAssemblyStateCreateInfo, 0, sizeof(VkPipelineInputAssemblyStateCreateInfo));
    vkPipelineInputAssemblyStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    vkPipelineInputAssemblyStateCreateInfo.pNext = NULL;
    vkPipelineInputAssemblyStateCreateInfo.flags = 0;
    vkPipelineInputAssemblyStateCreateInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    vkPipelineInputAssemblyStateCreateInfo.primitiveRestartEnable = 0;
    
    
    
    VkPipelineRasterizationStateCreateInfo vkPipelineRasterizationStateCreateInfo;
    memset((void*)&vkPipelineRasterizationStateCreateInfo, 0, sizeof(VkPipelineRasterizationStateCreateInfo));
    vkPipelineRasterizationStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    vkPipelineRasterizationStateCreateInfo.pNext = NULL;
    vkPipelineRasterizationStateCreateInfo.flags = 0;
    vkPipelineRasterizationStateCreateInfo.polygonMode = VK_POLYGON_MODE_FILL;
    vkPipelineRasterizationStateCreateInfo.cullMode = VK_CULL_MODE_NONE;
    vkPipelineRasterizationStateCreateInfo.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    vkPipelineRasterizationStateCreateInfo.lineWidth = 1.0f;
    
    
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
    
    
    VkPipelineViewportStateCreateInfo vkPipelineViewportStateCreateInfo;
    memset((void*)&vkPipelineViewportStateCreateInfo, 0, sizeof(VkPipelineViewportStateCreateInfo));
    vkPipelineViewportStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    vkPipelineViewportStateCreateInfo.pNext = NULL;
    vkPipelineViewportStateCreateInfo.flags = 0;
    vkPipelineViewportStateCreateInfo.viewportCount = 1; 
    
    memset((void*)&vkViewport, 0, sizeof(VkViewport));
    vkViewport.x = 0;
    vkViewport.y = 0;
    vkViewport.width = (float)vkExtent2D_swapchain.width;
    vkViewport.height = (float)vkExtent2D_swapchain.height;
    vkViewport.minDepth = 0.0f;
    vkViewport.maxDepth = 1.0f;
    
    vkPipelineViewportStateCreateInfo.pViewports = &vkViewport;
    vkPipelineViewportStateCreateInfo.scissorCount = 1;
    
    memset((void*)&vkRect2D_scissor, 0, sizeof(VkRect2D));
    vkRect2D_scissor.offset.x = 0;
    vkRect2D_scissor.offset.y = 0;
    vkRect2D_scissor.extent.width = (float)vkExtent2D_swapchain.width;
    vkRect2D_scissor.extent.height = (float)vkExtent2D_swapchain.height;
    
    vkPipelineViewportStateCreateInfo.pScissors = &vkRect2D_scissor;
    
    
    
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
    
    
    
    
    
    VkPipelineMultisampleStateCreateInfo vkPipelineMultisampleStateCreateInfo;
    memset((void*)&vkPipelineMultisampleStateCreateInfo, 0, sizeof(VkPipelineMultisampleStateCreateInfo));
    vkPipelineMultisampleStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    vkPipelineMultisampleStateCreateInfo.pNext = NULL;
    vkPipelineMultisampleStateCreateInfo.flags = 0;
    vkPipelineMultisampleStateCreateInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    
    
    VkPipelineShaderStageCreateInfo vkPipelineShaderStageCreateInfo_array[2];
    memset((void*)vkPipelineShaderStageCreateInfo_array, 0, sizeof(VkPipelineShaderStageCreateInfo) * _ARRAYSIZE(vkPipelineShaderStageCreateInfo_array));
    
    vkPipelineShaderStageCreateInfo_array[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vkPipelineShaderStageCreateInfo_array[0].pNext = NULL;
    vkPipelineShaderStageCreateInfo_array[0].flags = 0;
    vkPipelineShaderStageCreateInfo_array[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
    vkPipelineShaderStageCreateInfo_array[0].module = vkShaderModule_vertex_shader;
    vkPipelineShaderStageCreateInfo_array[0].pName = "main";
    vkPipelineShaderStageCreateInfo_array[0].pSpecializationInfo = NULL;
    
    
    vkPipelineShaderStageCreateInfo_array[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vkPipelineShaderStageCreateInfo_array[1].pNext = NULL;
    vkPipelineShaderStageCreateInfo_array[1].flags = 0;
    vkPipelineShaderStageCreateInfo_array[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    vkPipelineShaderStageCreateInfo_array[1].module = vkShaderModule_fragment_shader;
    vkPipelineShaderStageCreateInfo_array[1].pName = "main";
    vkPipelineShaderStageCreateInfo_array[1].pSpecializationInfo = NULL;
    
    
    
    
    
    
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
        fflush(gpFile);
        return vkResult;
    }
    else
    {
        fprintf(gpFile, "createPipeline() --> vkCreatePipelineCache() is succedded\n");
        fflush(gpFile);
    }
    
    
    VkGraphicsPipelineCreateInfo vkGraphicsPipelineCreateInfo;
    memset((void*)&vkGraphicsPipelineCreateInfo, 0, sizeof(VkGraphicsPipelineCreateInfo));
    vkGraphicsPipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    vkGraphicsPipelineCreateInfo.pNext = NULL;
    vkGraphicsPipelineCreateInfo.flags = 0;
    vkGraphicsPipelineCreateInfo.pVertexInputState = &vkPipelineVertexInputStateCreateInfo;
    vkGraphicsPipelineCreateInfo.pInputAssemblyState = &vkPipelineInputAssemblyStateCreateInfo;
    vkGraphicsPipelineCreateInfo.pRasterizationState = &vkPipelineRasterizationStateCreateInfo;
    vkGraphicsPipelineCreateInfo.pColorBlendState = &vkPipelineColorBlendStateCreateInfo;
    vkGraphicsPipelineCreateInfo.pViewportState = &vkPipelineViewportStateCreateInfo;
    vkGraphicsPipelineCreateInfo.pDepthStencilState = &vkPipelineDepthStencilStateCreateInfo;
    vkGraphicsPipelineCreateInfo.pDynamicState = NULL;
    vkGraphicsPipelineCreateInfo.pMultisampleState = &vkPipelineMultisampleStateCreateInfo;
    vkGraphicsPipelineCreateInfo.stageCount = _ARRAYSIZE(vkPipelineShaderStageCreateInfo_array);
    vkGraphicsPipelineCreateInfo.pStages = vkPipelineShaderStageCreateInfo_array;
    vkGraphicsPipelineCreateInfo.pTessellationState = NULL;
    vkGraphicsPipelineCreateInfo.layout = vkPipelineLayout;
    vkGraphicsPipelineCreateInfo.renderPass = vkRenderPass;
    vkGraphicsPipelineCreateInfo.subpass = 0; 
    vkGraphicsPipelineCreateInfo.basePipelineHandle = VK_NULL_HANDLE;
    vkGraphicsPipelineCreateInfo.basePipelineIndex = 0;
    
    
    vkResult = vkCreateGraphicsPipelines(vkDevice,
                                         vkPipelineCache,
                                         1,
                                         &vkGraphicsPipelineCreateInfo,
                                         NULL,
                                         &vkPipeline);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "createPipeline() --> vkCreateGraphicsPipelines() is failed error code is %d\n", vkResult);
        fflush(gpFile);
        vkDestroyPipelineCache(vkDevice, vkPipelineCache, NULL);
        vkPipelineCache = VK_NULL_HANDLE;
        return vkResult;
    }
    else
    {
        fprintf(gpFile, "createPipeline() --> vkCreateGraphicsPipelines() is succedded\n");
        fflush(gpFile);
    }
    
    
    vkDestroyPipelineCache(vkDevice, vkPipelineCache, NULL);
    vkPipelineCache = VK_NULL_HANDLE;
    
    return (vkResult);
}



VkResult createFrameBuffers(void)
{
    
    VkResult vkResult = VK_SUCCESS;
    
    
    
    VkImageView vkImageView_attachment_array[1];
    memset((void*)vkImageView_attachment_array, 0, sizeof(VkImageView) * _ARRAYSIZE(vkImageView_attachment_array));
    
    
    VkFramebufferCreateInfo vkFramebufferCreateInfo;
    memset((void*)&vkFramebufferCreateInfo, 0, sizeof(VkFramebufferCreateInfo));
    
    vkFramebufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    vkFramebufferCreateInfo.pNext = NULL;
    vkFramebufferCreateInfo.flags = 0;
    vkFramebufferCreateInfo.renderPass = vkRenderPass;
    vkFramebufferCreateInfo.attachmentCount = _ARRAYSIZE(vkImageView_attachment_array);
    vkFramebufferCreateInfo.pAttachments = vkImageView_attachment_array;
    vkFramebufferCreateInfo.width = vkExtent2D_swapchain.width;
    vkFramebufferCreateInfo.height = vkExtent2D_swapchain.height;
    vkFramebufferCreateInfo.layers = 1;
    
    
    vkFrameBuffer_array = (VkFramebuffer*)malloc(sizeof(VkFramebuffer) * swapchainImageCount);
    
    
    
    
    for(uint32_t i = 0; i < swapchainImageCount; i++)
    {
        
        
        VkImageView vkImageView_attachment_array[2]; 
        memset((void*)vkImageView_attachment_array, 0, sizeof(VkImageView) * _ARRAYSIZE(vkImageView_attachment_array));
        
        
        VkFramebufferCreateInfo vkFramebufferCreateInfo;
        memset((void*)&vkFramebufferCreateInfo, 0, sizeof(VkFramebufferCreateInfo));
        
        vkFramebufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        vkFramebufferCreateInfo.pNext = NULL;
        vkFramebufferCreateInfo.flags = 0;
        vkFramebufferCreateInfo.renderPass = vkRenderPass;
        vkFramebufferCreateInfo.attachmentCount = _ARRAYSIZE(vkImageView_attachment_array);
        vkFramebufferCreateInfo.pAttachments = vkImageView_attachment_array;
        vkFramebufferCreateInfo.width = vkExtent2D_swapchain.width;
        vkFramebufferCreateInfo.height = vkExtent2D_swapchain.height;
        vkFramebufferCreateInfo.layers = 1;
        
        
        
        vkImageView_attachment_array[0] = swapchainImageView_array[i];
        vkImageView_attachment_array[1] = vkImageView_depth;
        
        vkResult = vkCreateFramebuffer(vkDevice, 
                                       &vkFramebufferCreateInfo,
                                       NULL,
                                       &vkFrameBuffer_array[i]);
        if(vkResult != VK_SUCCESS)
        {
            fprintf(gpFile, "createFrameBuffers() --> vkCreateFramebuffer() is failed for %d iteration and error code is %d\n",i, vkResult);
        fflush(gpFile);
            return vkResult;
        }
        else
        {
            fprintf(gpFile, "createFrameBuffers() --> vkCreateFramebuffer() is succedded for iteration %d\n", i);
        fflush(gpFile);
        }
    }
    
    return vkResult;
}


VkResult createSemaphores(void)
{
    
    VkResult vkResult = VK_SUCCESS;
    
    
    
    VkSemaphoreCreateInfo vkSemaphoreCreateInfo;
    memset((void*)&vkSemaphoreCreateInfo, 0, sizeof(VkSemaphoreCreateInfo));
    
    vkSemaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    vkSemaphoreCreateInfo.pNext = NULL;  
    vkSemaphoreCreateInfo.flags = 0; 
    
    
    vkResult = vkCreateSemaphore(vkDevice, 
                                 &vkSemaphoreCreateInfo,
                                 NULL,
                                 &vkSemaphore_backBuffer);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "createSemaphores() --> vkCreateSemaphore() is failed for vkSemaphore_backBuffer and error code is %d\n", vkResult);
        fflush(gpFile);
        return vkResult;
    }
    else
    {
        fprintf(gpFile, "createSemaphores() --> vkCreateSemaphore() is succedded for vkSemaphore_backBuffer\n");
        fflush(gpFile);
    }
    
    
    
    vkResult = vkCreateSemaphore(vkDevice, 
                                 &vkSemaphoreCreateInfo,
                                 NULL,
                                 &vkSemaphore_renderComplete);
    if(vkResult != VK_SUCCESS)
    {
        fprintf(gpFile, "createSemaphores() --> vkCreateSemaphore() is failed for vkSemaphore_renderComplete and error code is %d\n", vkResult);
        fflush(gpFile);
        return vkResult;
    }
    else
    {
        fprintf(gpFile, "createSemaphores() --> vkCreateSemaphore() is succedded for vkSemaphore_renderComplete\n");
        fflush(gpFile);
    }
    
    return vkResult;
    
}

VkResult createFences(void)
{
    VkResult vkResult = VK_SUCCESS;

    VkFenceCreateInfo vkFenceCreateInfo;
    memset(&vkFenceCreateInfo, 0, sizeof(vkFenceCreateInfo));
    vkFenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    vkFenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    // FIX: allocate for swapchainImageCount, not sizeof(swapchainImageCount)
    vkFence_array = (VkFence*)malloc(sizeof(VkFence) * swapchainImageCount);

    for (uint32_t i = 0; i < swapchainImageCount; ++i)
    {
        vkResult = vkCreateFence(vkDevice, &vkFenceCreateInfo, NULL, &vkFence_array[i]);
        if (vkResult != VK_SUCCESS)
        {
            fprintf(gpFile, "createFences() --> vkCreateFence() failed at %u, err=%d\n", i, vkResult); fflush(gpFile);
            return vkResult;
        }
        else
        {
            fprintf(gpFile, "createFences() --> vkCreateFence() is succedded for %u iteration\n", i); fflush(gpFile);
        }
    }

    return vkResult;
}

VkResult buildCommandBuffers(void)
{
    
    VkResult vkResult = VK_SUCCESS;
    
    
    
    
    for(uint32_t i = 0; i < swapchainImageCount; i++)
    {
        
        vkResult = vkResetCommandBuffer(vkCommandBuffer_array[i], 0);  
        if(vkResult != VK_SUCCESS)
        {
            fprintf(gpFile, "buildCommandBuffers() --> vkResetCommandBuffer() is failed for %d iteration and error code is %d\n", i, vkResult);
        fflush(gpFile);
            return vkResult;
        }
        else
        {
            fprintf(gpFile, "buildCommandBuffers() --> vkResetCommandBuffer() is succedded for %d iteration\n", i);
        fflush(gpFile);
        }
        
        VkCommandBufferBeginInfo vkCommandBufferBeginInfo;
        memset((void*)&vkCommandBufferBeginInfo, 0, sizeof(VkCommandBufferBeginInfo));
        
        vkCommandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        vkCommandBufferBeginInfo.pNext = NULL;
        vkCommandBufferBeginInfo.flags = 0;  
        
        vkResult = vkBeginCommandBuffer(vkCommandBuffer_array[i], &vkCommandBufferBeginInfo);
        if(vkResult != VK_SUCCESS)
        {
            fprintf(gpFile, "buildCommandBuffers() --> vkBeginCommandBuffer() is failed for %d iteration and error code is %d\n", i, vkResult);
        fflush(gpFile);
            return vkResult;
        }
        else
        {
            fprintf(gpFile, "buildCommandBuffers() --> vkBeginCommandBuffer() is succedded for %d iteration\n", i);
        fflush(gpFile);
        }
        
        
        VkClearValue vkClearValue_array[2];
        memset((void*)vkClearValue_array, 0, (sizeof(VkClearValue) * _ARRAYSIZE(vkClearValue_array)));
        
        vkClearValue_array[0].color = vkClearColorValue;
        vkClearValue_array[1].depthStencil = vkClearDepthStencilValue;
        
        
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
        
        
        vkCmdBeginRenderPass(vkCommandBuffer_array[i], 
                             &vkRenderPassBeginInfo, 
                             VK_SUBPASS_CONTENTS_INLINE);

        
        vkCmdBindPipeline(vkCommandBuffer_array[i],
                          VK_PIPELINE_BIND_POINT_GRAPHICS,
                          vkPipeline);
                          
        
        
        vkCmdBindDescriptorSets(vkCommandBuffer_array[i],
                                VK_PIPELINE_BIND_POINT_GRAPHICS,
                                vkPipelineLayout,
                                0,
                                1,
                                &vkDescriptorSet_sun,
                                0, 
                                NULL);
        
        
        VkDeviceSize vkDeviceSize_offset_position[1];
        memset((void*)vkDeviceSize_offset_position, 0, sizeof(VkDeviceSize) * _ARRAYSIZE(vkDeviceSize_offset_position));
        vkCmdBindVertexBuffers(vkCommandBuffer_array[i], 
                               0, 
                               1, 
                               &vertexData_position_sun.vkBuffer, 
                               vkDeviceSize_offset_position);
        
        
        VkDeviceSize vkDeviceSize_offset_normal[1];
        memset((void*)vkDeviceSize_offset_normal, 0, sizeof(VkDeviceSize) * _ARRAYSIZE(vkDeviceSize_offset_normal));
        vkCmdBindVertexBuffers(vkCommandBuffer_array[i], 
                               1,
                               1, 
                               &vertexData_normal_sun.vkBuffer, 
                               vkDeviceSize_offset_normal);

        
        VkDeviceSize vkDeviceSize_offset_texcoord[1];
        memset((void*)vkDeviceSize_offset_texcoord, 0, sizeof(VkDeviceSize) * _ARRAYSIZE(vkDeviceSize_offset_texcoord));
        vkCmdBindVertexBuffers(vkCommandBuffer_array[i], 
                               2,
                               1, 
                               &vertexData_texcoord_sun.vkBuffer, 
                               vkDeviceSize_offset_texcoord);                               
                              
        
        vkCmdBindIndexBuffer(vkCommandBuffer_array[i],
                             vertexData_index_sun.vkBuffer,
                             0,
                             VK_INDEX_TYPE_UINT16 
                             );
                             
        
        vkCmdDrawIndexed(vkCommandBuffer_array[i],
                         numElements,
                         1,
                         0,
                         0,
                         0);
                         
                         
        
        
        vkCmdBindDescriptorSets(vkCommandBuffer_array[i],
                                VK_PIPELINE_BIND_POINT_GRAPHICS,
                                vkPipelineLayout,
                                0,
                                1,
                                &vkDescriptorSet_mercury,
                                0, 
                                NULL);
        
        
        
        memset((void*)vkDeviceSize_offset_position, 0, sizeof(VkDeviceSize) * _ARRAYSIZE(vkDeviceSize_offset_position));
        vkCmdBindVertexBuffers(vkCommandBuffer_array[i], 
                               0, 
                               1, 
                               &vertexData_position_mercury.vkBuffer, 
                               vkDeviceSize_offset_position);
        
        
        
        memset((void*)vkDeviceSize_offset_normal, 0, sizeof(VkDeviceSize) * _ARRAYSIZE(vkDeviceSize_offset_normal));
        vkCmdBindVertexBuffers(vkCommandBuffer_array[i], 
                               1,
                               1, 
                               &vertexData_normal_mercury.vkBuffer, 
                               vkDeviceSize_offset_normal);

        
        
        memset((void*)vkDeviceSize_offset_texcoord, 0, sizeof(VkDeviceSize) * _ARRAYSIZE(vkDeviceSize_offset_texcoord));
        vkCmdBindVertexBuffers(vkCommandBuffer_array[i], 
                               2,
                               1, 
                               &vertexData_texcoord_mercury.vkBuffer, 
                               vkDeviceSize_offset_texcoord);                               
                              
        
        vkCmdBindIndexBuffer(vkCommandBuffer_array[i],
                             vertexData_index_mercury.vkBuffer,
                             0,
                             VK_INDEX_TYPE_UINT16 
                             );
                             
        
        vkCmdDrawIndexed(vkCommandBuffer_array[i],
                         numElements,
                         1,
                         0,
                         0,
                         0);
        
        
        
        vkCmdBindDescriptorSets(vkCommandBuffer_array[i],
                                VK_PIPELINE_BIND_POINT_GRAPHICS,
                                vkPipelineLayout,
                                0,
                                1,
                                &vkDescriptorSet_venus,
                                0, 
                                NULL);
        
        
        
        memset((void*)vkDeviceSize_offset_position, 0, sizeof(VkDeviceSize) * _ARRAYSIZE(vkDeviceSize_offset_position));
        vkCmdBindVertexBuffers(vkCommandBuffer_array[i], 
                               0, 
                               1, 
                               &vertexData_position_venus.vkBuffer, 
                               vkDeviceSize_offset_position);
        
        
        
        memset((void*)vkDeviceSize_offset_normal, 0, sizeof(VkDeviceSize) * _ARRAYSIZE(vkDeviceSize_offset_normal));
        vkCmdBindVertexBuffers(vkCommandBuffer_array[i], 
                               1,
                               1, 
                               &vertexData_normal_venus.vkBuffer, 
                               vkDeviceSize_offset_normal);

        
        
        memset((void*)vkDeviceSize_offset_texcoord, 0, sizeof(VkDeviceSize) * _ARRAYSIZE(vkDeviceSize_offset_texcoord));
        vkCmdBindVertexBuffers(vkCommandBuffer_array[i], 
                               2,
                               1, 
                               &vertexData_texcoord_venus.vkBuffer, 
                               vkDeviceSize_offset_texcoord);                               
                              
        
        vkCmdBindIndexBuffer(vkCommandBuffer_array[i],
                             vertexData_index_venus.vkBuffer,
                             0,
                             VK_INDEX_TYPE_UINT16 
                             );
                             
        
        vkCmdDrawIndexed(vkCommandBuffer_array[i],
                         numElements,
                         1,
                         0,
                         0,
                         0);
                         
        
        
        vkCmdBindDescriptorSets(vkCommandBuffer_array[i],
                                VK_PIPELINE_BIND_POINT_GRAPHICS,
                                vkPipelineLayout,
                                0,
                                1,
                                &vkDescriptorSet_earth,
                                0, 
                                NULL);
        
        
        
        memset((void*)vkDeviceSize_offset_position, 0, sizeof(VkDeviceSize) * _ARRAYSIZE(vkDeviceSize_offset_position));
        vkCmdBindVertexBuffers(vkCommandBuffer_array[i], 
                               0, 
                               1, 
                               &vertexData_position_earth.vkBuffer, 
                               vkDeviceSize_offset_position);
        
        
        
        memset((void*)vkDeviceSize_offset_normal, 0, sizeof(VkDeviceSize) * _ARRAYSIZE(vkDeviceSize_offset_normal));
        vkCmdBindVertexBuffers(vkCommandBuffer_array[i], 
                               1,
                               1, 
                               &vertexData_normal_earth.vkBuffer, 
                               vkDeviceSize_offset_normal);

        
        
        memset((void*)vkDeviceSize_offset_texcoord, 0, sizeof(VkDeviceSize) * _ARRAYSIZE(vkDeviceSize_offset_texcoord));
        vkCmdBindVertexBuffers(vkCommandBuffer_array[i], 
                               2,
                               1, 
                               &vertexData_texcoord_earth.vkBuffer, 
                               vkDeviceSize_offset_texcoord);                               
                              
        
        vkCmdBindIndexBuffer(vkCommandBuffer_array[i],
                             vertexData_index_earth.vkBuffer,
                             0,
                             VK_INDEX_TYPE_UINT16 
                             );
                             
        
        vkCmdDrawIndexed(vkCommandBuffer_array[i],
                         numElements,
                         1,
                         0,
                         0,
                         0);
                         
        
        
        vkCmdBindDescriptorSets(vkCommandBuffer_array[i],
                                VK_PIPELINE_BIND_POINT_GRAPHICS,
                                vkPipelineLayout,
                                0,
                                1,
                                &vkDescriptorSet_mars,
                                0, 
                                NULL);
        
        
        
        memset((void*)vkDeviceSize_offset_position, 0, sizeof(VkDeviceSize) * _ARRAYSIZE(vkDeviceSize_offset_position));
        vkCmdBindVertexBuffers(vkCommandBuffer_array[i], 
                               0, 
                               1, 
                               &vertexData_position_mars.vkBuffer, 
                               vkDeviceSize_offset_position);
        
        
        
        memset((void*)vkDeviceSize_offset_normal, 0, sizeof(VkDeviceSize) * _ARRAYSIZE(vkDeviceSize_offset_normal));
        vkCmdBindVertexBuffers(vkCommandBuffer_array[i], 
                               1,
                               1, 
                               &vertexData_normal_mars.vkBuffer, 
                               vkDeviceSize_offset_normal);

        
        
        memset((void*)vkDeviceSize_offset_texcoord, 0, sizeof(VkDeviceSize) * _ARRAYSIZE(vkDeviceSize_offset_texcoord));
        vkCmdBindVertexBuffers(vkCommandBuffer_array[i], 
                               2,
                               1, 
                               &vertexData_texcoord_mars.vkBuffer, 
                               vkDeviceSize_offset_texcoord);                               
                              
        
        vkCmdBindIndexBuffer(vkCommandBuffer_array[i],
                             vertexData_index_mars.vkBuffer,
                             0,
                             VK_INDEX_TYPE_UINT16 
                             );
                             
        
        vkCmdDrawIndexed(vkCommandBuffer_array[i],
                         numElements,
                         1,
                         0,
                         0,
                         0);
                         
        
        
        vkCmdBindDescriptorSets(vkCommandBuffer_array[i],
                                VK_PIPELINE_BIND_POINT_GRAPHICS,
                                vkPipelineLayout,
                                0,
                                1,
                                &vkDescriptorSet_jupitor,
                                0, 
                                NULL);
        
        
        
        memset((void*)vkDeviceSize_offset_position, 0, sizeof(VkDeviceSize) * _ARRAYSIZE(vkDeviceSize_offset_position));
        vkCmdBindVertexBuffers(vkCommandBuffer_array[i], 
                               0, 
                               1, 
                               &vertexData_position_jupitor.vkBuffer, 
                               vkDeviceSize_offset_position);
        
        
        
        memset((void*)vkDeviceSize_offset_normal, 0, sizeof(VkDeviceSize) * _ARRAYSIZE(vkDeviceSize_offset_normal));
        vkCmdBindVertexBuffers(vkCommandBuffer_array[i], 
                               1,
                               1, 
                               &vertexData_normal_jupitor.vkBuffer, 
                               vkDeviceSize_offset_normal);

        
        
        memset((void*)vkDeviceSize_offset_texcoord, 0, sizeof(VkDeviceSize) * _ARRAYSIZE(vkDeviceSize_offset_texcoord));
        vkCmdBindVertexBuffers(vkCommandBuffer_array[i], 
                               2,
                               1, 
                               &vertexData_texcoord_jupitor.vkBuffer, 
                               vkDeviceSize_offset_texcoord);                               
                              
        
        vkCmdBindIndexBuffer(vkCommandBuffer_array[i],
                             vertexData_index_jupitor.vkBuffer,
                             0,
                             VK_INDEX_TYPE_UINT16 
                             );
                             
        
        vkCmdDrawIndexed(vkCommandBuffer_array[i],
                         numElements,
                         1,
                         0,
                         0,
                         0);
                         
        
        
        vkCmdBindDescriptorSets(vkCommandBuffer_array[i],
                                VK_PIPELINE_BIND_POINT_GRAPHICS,
                                vkPipelineLayout,
                                0,
                                1,
                                &vkDescriptorSet_saturn,
                                0, 
                                NULL);
        
        
        
        memset((void*)vkDeviceSize_offset_position, 0, sizeof(VkDeviceSize) * _ARRAYSIZE(vkDeviceSize_offset_position));
        vkCmdBindVertexBuffers(vkCommandBuffer_array[i], 
                               0, 
                               1, 
                               &vertexData_position_saturn.vkBuffer, 
                               vkDeviceSize_offset_position);
        
        
        
        memset((void*)vkDeviceSize_offset_normal, 0, sizeof(VkDeviceSize) * _ARRAYSIZE(vkDeviceSize_offset_normal));
        vkCmdBindVertexBuffers(vkCommandBuffer_array[i], 
                               1,
                               1, 
                               &vertexData_normal_saturn.vkBuffer, 
                               vkDeviceSize_offset_normal);

        
        
        memset((void*)vkDeviceSize_offset_texcoord, 0, sizeof(VkDeviceSize) * _ARRAYSIZE(vkDeviceSize_offset_texcoord));
        vkCmdBindVertexBuffers(vkCommandBuffer_array[i], 
                               2,
                               1, 
                               &vertexData_texcoord_saturn.vkBuffer, 
                               vkDeviceSize_offset_texcoord);                               
                              
        
        vkCmdBindIndexBuffer(vkCommandBuffer_array[i],
                             vertexData_index_saturn.vkBuffer,
                             0,
                             VK_INDEX_TYPE_UINT16 
                             );
                             
        
        vkCmdDrawIndexed(vkCommandBuffer_array[i],
                         numElements,
                         1,
                         0,
                         0,
                         0);
                         
        
        
        vkCmdBindDescriptorSets(vkCommandBuffer_array[i],
                                VK_PIPELINE_BIND_POINT_GRAPHICS,
                                vkPipelineLayout,
                                0,
                                1,
                                &vkDescriptorSet_uranus,
                                0, 
                                NULL);
        
        
        
        memset((void*)vkDeviceSize_offset_position, 0, sizeof(VkDeviceSize) * _ARRAYSIZE(vkDeviceSize_offset_position));
        vkCmdBindVertexBuffers(vkCommandBuffer_array[i], 
                               0, 
                               1, 
                               &vertexData_position_uranus.vkBuffer, 
                               vkDeviceSize_offset_position);
        
        
        
        memset((void*)vkDeviceSize_offset_normal, 0, sizeof(VkDeviceSize) * _ARRAYSIZE(vkDeviceSize_offset_normal));
        vkCmdBindVertexBuffers(vkCommandBuffer_array[i], 
                               1,
                               1, 
                               &vertexData_normal_uranus.vkBuffer, 
                               vkDeviceSize_offset_normal);

        
        
        memset((void*)vkDeviceSize_offset_texcoord, 0, sizeof(VkDeviceSize) * _ARRAYSIZE(vkDeviceSize_offset_texcoord));
        vkCmdBindVertexBuffers(vkCommandBuffer_array[i], 
                               2,
                               1, 
                               &vertexData_texcoord_uranus.vkBuffer, 
                               vkDeviceSize_offset_texcoord);                               
                              
        
        vkCmdBindIndexBuffer(vkCommandBuffer_array[i],
                             vertexData_index_uranus.vkBuffer,
                             0,
                             VK_INDEX_TYPE_UINT16 
                             );
                             
        
        vkCmdDrawIndexed(vkCommandBuffer_array[i],
                         numElements,
                         1,
                         0,
                         0,
                         0);
                         
        
        
        vkCmdBindDescriptorSets(vkCommandBuffer_array[i],
                                VK_PIPELINE_BIND_POINT_GRAPHICS,
                                vkPipelineLayout,
                                0,
                                1,
                                &vkDescriptorSet_neptune,
                                0, 
                                NULL);
        
        
        
        memset((void*)vkDeviceSize_offset_position, 0, sizeof(VkDeviceSize) * _ARRAYSIZE(vkDeviceSize_offset_position));
        vkCmdBindVertexBuffers(vkCommandBuffer_array[i], 
                               0, 
                               1, 
                               &vertexData_position_neptune.vkBuffer, 
                               vkDeviceSize_offset_position);
        
        
        
        memset((void*)vkDeviceSize_offset_normal, 0, sizeof(VkDeviceSize) * _ARRAYSIZE(vkDeviceSize_offset_normal));
        vkCmdBindVertexBuffers(vkCommandBuffer_array[i], 
                               1,
                               1, 
                               &vertexData_normal_neptune.vkBuffer, 
                               vkDeviceSize_offset_normal);

        
        
        memset((void*)vkDeviceSize_offset_texcoord, 0, sizeof(VkDeviceSize) * _ARRAYSIZE(vkDeviceSize_offset_texcoord));
        vkCmdBindVertexBuffers(vkCommandBuffer_array[i], 
                               2,
                               1, 
                               &vertexData_texcoord_neptune.vkBuffer, 
                               vkDeviceSize_offset_texcoord);                               
                              
        
        vkCmdBindIndexBuffer(vkCommandBuffer_array[i],
                             vertexData_index_neptune.vkBuffer,
                             0,
                             VK_INDEX_TYPE_UINT16 
                             );
                             
        
        vkCmdDrawIndexed(vkCommandBuffer_array[i],
                         numElements,
                         1,
                         0,
                         0,
                         0);
                         
                         
        
        
        
        vkCmdEndRenderPass(vkCommandBuffer_array[i]);
        
        
        vkResult = vkEndCommandBuffer(vkCommandBuffer_array[i]);
        if(vkResult != VK_SUCCESS)
        {
            fprintf(gpFile, "buildCommandBuffers() --> vkEndCommandBuffer() is failed for %d iteration and error code is %d\n", i, vkResult);
        fflush(gpFile);
            return vkResult;
        }
        else
        {
            fprintf(gpFile, "buildCommandBuffers() --> vkEndCommandBuffer() is succedded for %d iteration\n", i);
        fflush(gpFile);
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
    
    fprintf(gpFile, "SSA_Validation: debugReportCallback() --> %s (%d) = %s\n", pLayerPrefix, messageCode, pMessage);
        fflush(gpFile);
    
    return(VK_FALSE);
}