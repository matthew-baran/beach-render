#pragma once

#include "shader.hpp"

#include <GLAD/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <iostream>
#include <string>
#include <tuple>
#include <vector>

namespace msb
{

class Wave
{
  private:
    float _amplitude;
    float _wavelength;
    float _freq;
    float _phi;
    float _phase_offset;
    double _start_time;
    double _reset_time;
    double _att_offset;
    double _fade_in_time = 5; // seconds
    glm::vec2 _direction;

  public:
    float avg_amplitude = 0.2;
    float avg_wavelength = 20;
    float chop = 1;
    float duration = 5;
    glm::vec2 avg_direction = {1, 1};

    Wave();
    Wave(float amp, float wl, float chop, float dur, glm::vec2 dir, double att_offset);

    void resetWave();
    float amplitude();
    float wavelength() { return _wavelength; }
    float freq() { return _freq; }
    float phase() { return float(_phi * (glfwGetTime() - _reset_time)) + _phase_offset; }
    float phase_offset() { return _phase_offset; }
    glm::vec2 direction() { return _direction; }
};

void initWaves(Shader& shader, std::vector<Wave>& waves, std::string var_name, float total_chop);
void updateWaves(Shader& shader, std::vector<Wave>& waves, std::string var_name, float total_chop);
std::vector<Wave> makeGeomWaves();
std::vector<Wave> makeTexWaves(size_t num_waves);

} // namespace msb