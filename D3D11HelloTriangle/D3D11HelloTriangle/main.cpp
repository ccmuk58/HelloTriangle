#include <iostream>
#include <cstdint>
#include <GLFW/glfw3.h>


int main()
{
	if (!glfwInit())
	{
		std::cout << "GLFW: Unable to initialize\n";
		return -1;
	}

	// get monitor info
	GLFWmonitor* primaryMonitor = glfwGetPrimaryMonitor();
	const GLFWvidmode* videoMode = glfwGetVideoMode(primaryMonitor);
	const int32_t width = static_cast<int32_t>(videoMode->width * 0.5f);
	const int32_t height = static_cast<int32_t>(videoMode->height * 0.5f);

	// glfw setting
	glfwWindowHint(GLFW_SCALE_TO_MONITOR, GLFW_FALSE);
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

	GLFWwindow* window = 
		glfwCreateWindow(width, height, "D3D11 Hello Triangle", nullptr, nullptr);
	if (window == nullptr)
	{
		std::cout << "GLFW: Unable to create window\n";
		glfwTerminate();
		return -1;
	}

	const int32_t windowLeft = videoMode->width / 2 - width / 2;
	const int32_t windowTop = videoMode->height/ 2 - height/ 2;
	glfwSetWindowPos(window, windowLeft, windowTop);

	while (!glfwWindowShouldClose(window))
	{
		glfwPollEvents();
		// update
		// render
	}
	glfwDestroyWindow(window);
	glfwTerminate();
	return 0;
}