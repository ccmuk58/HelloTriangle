#include "Application.hpp"

class HelloWindowApplication final : public Application
{
public:
	HelloWindowApplication(const std::string& title);

protected:
	bool Load() override;
	void Render() override;
	void Update() override;
};