#include "camera.hpp"
#include "geometry.hpp"
#include "gl_helpers.hpp"
#include "model.hpp"
#include "shader.hpp"
#include "terrain.hpp"
#include "wave.hpp"
#include "window_management.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

int main()
{
	auto window = msb::initializeWindow();
	auto [vertices, faces] = msb::getPlane(65, 50, .1, 10);

	std::vector<msb::Texture> ocean_tex = {
		msb::initTexture("resources/bathy.png", "texture_diffuse", GL_CLAMP_TO_EDGE, GL_LINEAR, GL_RGB),
		msb::initTexture("resources/foam2.png", "texture_diffuse", GL_MIRRORED_REPEAT, GL_LINEAR, GL_RGBA)
	};

	auto mesh = msb::Mesh(vertices, faces, ocean_tex);
	msb::Model model(std::move(mesh));
	Shader shader("shaders/ocean.vert", "shaders/ocean_pbr2.frag");
	shader.setFloat("avg_water_ht", 0.f);
	shader.setInt("env_map", 0);
	shader.setInt("brdf_map", 2);

	// auto [v_beach, f_beach] = getQuad(50, 50, 10);
	auto [v_beach, f_beach] = msb::getTerrain("resources/bathy.png", "resources/bathy_norms.png", 50, 50);

	std::vector<msb::Texture> beach_tex = {
		msb::initTexture("resources/sand.png", "texture_diffuse", GL_MIRRORED_REPEAT, GL_LINEAR, GL_SRGB)
	};

	auto mesh_beach = msb::Mesh(v_beach, f_beach, beach_tex);
	msb::Model model_beach(std::move(mesh_beach));
	Shader shader_beach("shaders/model_test.vert", "shaders/model_test.frag");

	//auto [v_cube, f_cube] = makeSkybox();
	//auto skybox_vao = fillBuffers(v_cube);
	//auto cubemap_tex = loadCubemap({ "skybox2/right.jpg","skybox2/left.jpg","skybox2/top.jpg",
	//								 "skybox2/bottom.jpg","skybox2/front.jpg","skybox2/back.jpg" });
	auto hdr_cube = msb::setHdrTexture("resources/Malibu/Malibu_Overlook_env.hdr");
	auto [cube_tex, cube_vao] = msb::renderCubeMap(hdr_cube);
	Shader shader_cubemap("shaders/cube.vert", "shaders/cube.frag");
	shader_cubemap.setInt("skybox", 0);

	auto brdf_map_id = msb::renderBrdfQuad();

	float geom_chop = 0.5;
	auto waves = msb::makeGeomWaves();
	msb::initWaves(shader, waves, "geom_waves", geom_chop);
	
	float tex_chop = 0.8;
	auto tx_waves = msb::makeTexWaves(32);
	msb::initWaves(shader, tx_waves, "tex_waves", tex_chop);

	// Directional
	//auto dir_light_vec = glm::vec3(-0.2f, -1.0f, -0.3f);
	auto dir_light_vec = glm::vec3(1.f, -.25f, 0.f);
	shader.setVec3("dir_light.direction", dir_light_vec);
	shader.setVec3("dir_light.ambient", 0.4f, 0.4f, 0.4f);
	shader.setVec3("dir_light.diffuse", .3f, .3f, .3f);
	shader.setVec3("dir_light.specular", .8f, .8f, .8f);

	shader_beach.setVec3("dir_light.direction", dir_light_vec);
	shader_beach.setVec3("dir_light.ambient", 0.2f, 0.2f, 0.2f);
	shader_beach.setVec3("dir_light.diffuse", .8f, .8f, .8f);
	shader_beach.setVec3("dir_light.specular", .0f, .0f, .0f);

	CameraState state(window);
	glfwSetWindowUserPointer(window, &state);

	state.setCameraPosition(glm::vec3(0.0, 3.0, 3.0));

	//glEnable(GL_FRAMEBUFFER_SRGB);
	glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_DEPTH_TEST);
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	glViewport(0, 0, 800, 600);

	float delta_time = 0.0f;
	float last_frame = 0.0f;

	while (!glfwWindowShouldClose(window))
	{
		auto current_frame = static_cast<float>(glfwGetTime());
		delta_time = current_frame - last_frame;
		last_frame = current_frame;
		state.setCameraSpeed(5.f * delta_time);
		msb::processInput(state);

		glClearColor(0.0, 0.0, 0.0, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		auto model_mat = glm::mat4(1.0f);

		// Beach
		shader_beach.setVec3("dir_light.direction", glm::vec3(state.viewMatrix() * glm::vec4(dir_light_vec, 0.0)));
		shader_beach.setMat4("model", model_mat);
		shader_beach.setMat4("view", state.viewMatrix());
		shader_beach.setMat4("projection", state.projectionMatrix());
		shader_beach.setMat3("invmodel", glm::mat3(glm::transpose(glm::inverse(state.viewMatrix() * model_mat))));
		model_beach.Draw(shader_beach);

		// Waves
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, brdf_map_id);
		shader.setVec3("dir_light.direction", dir_light_vec);
		shader.setMat4("model", model_mat);
		shader.setMat4("view", state.viewMatrix());
		shader.setMat4("projection", state.projectionMatrix());
		shader.setVec3("cam_pos", state.cameraPosition());
		updateWaves(shader, waves, "geom_waves", geom_chop);
		updateWaves(shader, tx_waves, "tex_waves", tex_chop);
		model.Draw(shader);

		// Cube map
		//glDepthFunc(GL_LEQUAL);
		//shader_cubemap.setMat4("view", glm::mat4(glm::mat3(state.viewMatrix())));
		//shader_cubemap.setMat4("projection", state.projectionMatrix());
		//glBindVertexArray(cube_vao);
		//glActiveTexture(GL_TEXTURE0);
		//glBindTexture(GL_TEXTURE_CUBE_MAP, cube_tex);
		//glDrawArrays(GL_TRIANGLES, 0, 36);
		//glBindVertexArray(0);
		//glDepthFunc(GL_LESS);

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glfwTerminate();

	return 0;
}