#include <GLAD/glad.h>
#include <gtest/gtest.h>

#include "camera.cpp"
#include "window_management.cpp"

TEST(CameraTest, Simple)
{
    // std::shared_ptr<GLFWwindow> p(msb::initializeWindow(), [](GLFWwindow* p) { glfwTerminate(); });
    auto cam = CameraState(nullptr);

    EXPECT_EQ(cam.fov, 45.0);
}