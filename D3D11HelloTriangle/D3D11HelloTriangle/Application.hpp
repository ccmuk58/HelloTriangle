#pragma once

#include <iostream>
#include <string>

struct GLFWwindow;

class Application
{
public:
	Application(const std::string& title);
	virtual ~Application();
	void Run();

protected:
	virtual void CleanUp();
	virtual bool Initialize();
	virtual bool Load() = 0;
	virtual void Render() = 0;
	virtual void Update() = 0;

private:
	GLFWwindow* _window = nullptr;
	int32_t _width = 0;
	int32_t _height = 0;
	std::string _title;
};