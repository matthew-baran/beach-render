#include <GLAD/glad.h>
#include <gtest/gtest.h>

#include "camera.cpp"
#include "window_management.cpp"

testing::AssertionResult Vec3Equal(const char* a_expr,
                                             const char* b_expr,
                                             glm::vec3 a,
                                             glm::vec3 b) {
  if (a == b) return testing::AssertionSuccess();

  return testing::AssertionFailure() << a_expr << " != " << b_expr << "\n"
      << a_expr << " = (" << a.x << ", " << a.y << ", " << a.z << ")\n"
      << b_expr << " = (" << b.x << ", " << b.y << ", " << b.z << ")";
}

TEST(CameraTest, Simple)
{
    auto cam = CameraState(nullptr);

    EXPECT_EQ(cam.fov, 45.0);
}

TEST(CameraTest, CameraMotion)
{
    auto cam = CameraState(nullptr);
    cam.moveForward();

    EXPECT_PRED_FORMAT2(Vec3Equal, cam.cameraPosition(), glm::vec3(0.0, 0.0, 2.95));

    cam.moveBackward();
    EXPECT_PRED_FORMAT2(Vec3Equal, cam.cameraPosition(), glm::vec3(0.0, 0.0, 3.0));

    cam.moveLeft();
    EXPECT_PRED_FORMAT2(Vec3Equal, cam.cameraPosition(), glm::vec3(-0.05, 0.0, 3.0));

    cam.moveRight();
    EXPECT_PRED_FORMAT2(Vec3Equal, cam.cameraPosition(), glm::vec3(0.0, 0.0, 3.0));
}