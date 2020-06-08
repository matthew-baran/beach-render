#include "wave.hpp"

namespace msb
{

Wave::Wave()
{
    resetWave();
    _reset_time = _reset_time - _att_offset * duration;
    _start_time = _reset_time;
}

Wave::Wave(float amp, float wl, float chop, float dur, glm::vec2 dir, double att_offset)
    : avg_amplitude(amp), avg_wavelength(wl), chop(chop), duration(dur), avg_direction(dir),
      _att_offset(att_offset)
{
    resetWave();
    _reset_time = _reset_time - _att_offset * duration;
    _start_time = _reset_time;
}

void Wave::resetWave()
{
    //_amplitude = avg_amplitude;
    //_direction = avg_direction;

    _amplitude = float(0.5 * avg_amplitude + (float(std::rand()) / float(RAND_MAX)) *
                                                 (1.5 * avg_amplitude)); // 10% amplitude variation
    _wavelength = _amplitude * avg_wavelength / avg_amplitude;           // maintain A/L ratio
    _freq = float(2. * 3.14159 / _wavelength);
    _phi = float(std::sqrt(9.8 * 3.14159 * 2 / _wavelength));
    //_phi = float(std::sqrt(9.8 * 0.5 / (3.14159 * 2)));  // hard-code water depth...
    _phase_offset = (float(std::rand()) / RAND_MAX) * 2.f * 3.14159f;

    auto angle = glm::radians(5. * (2. * (float(std::rand()) / RAND_MAX) - 1.));
    auto rot = glm::mat2(cos(angle), -sin(angle), sin(angle), cos(angle));
    _direction = glm::normalize(rot * avg_direction);

    _reset_time = glfwGetTime();
}

float Wave::amplitude()
{
    auto t = glfwGetTime();
    auto global_elapsed = t - _start_time;
    auto instance_elapsed = t - _reset_time;

    if (instance_elapsed > duration)
    {
        return 0.f;
    }

    if (duration - instance_elapsed < _fade_in_time)
    {
        return _amplitude *
               float(0.5 - 0.5 * std::cos(3.14159 * (duration - instance_elapsed) / _fade_in_time));
    }

    if (instance_elapsed > _fade_in_time)
    {
        return _amplitude;
    }

    return _amplitude * float(0.5 - 0.5 * std::cos(3.14159 * instance_elapsed / _fade_in_time));
}

void initWaves(Shader& shader, std::vector<Wave>& waves, std::string var_name, float total_chop)
{
    for (size_t i = 0; i < waves.size(); ++i)
    {
        std::string idx = "[" + std::to_string(i) + "]";

        // auto chop = 1 / (waves[i].freq() * waves[i].amplitude() * waves.size());
        // chop = std::min(waves[i].chop, chop);

        auto chop = total_chop / (waves[i].freq() * waves[i].amplitude() * waves.size());

        shader.setVec2(var_name + idx + ".wave_dirs", waves[i].direction());
        shader.setFloat(var_name + idx + ".freq", waves[i].freq());
        shader.setFloat(var_name + idx + ".phase", waves[i].phase());
        shader.setFloat(var_name + idx + ".phase_offset", waves[i].phase_offset());
        shader.setFloat(var_name + idx + ".amplitude", waves[i].amplitude());
        shader.setFloat(var_name + idx + ".chop", chop);
    }
}

void updateWaves(Shader& shader, std::vector<Wave>& waves, std::string var_name, float total_chop)
{
    for (size_t i = 0; i < waves.size(); ++i)
    {
        std::string idx = "[" + std::to_string(i) + "]";

        auto amplitude = waves[i].amplitude();

        if (!amplitude)
        {
            waves[i].resetWave();
            amplitude = waves[i].amplitude();
            shader.setVec2(var_name + idx + ".wave_dirs", waves[i].direction());
            shader.setFloat(var_name + idx + ".freq", waves[i].freq());
        }

        // auto chop = 1 / (waves[i].freq() * waves[i].amplitude() * waves.size());
        // chop = std::min(waves[i].chop, chop);

        auto chop = total_chop / (waves[i].freq() * waves[i].amplitude() * waves.size());

        shader.setFloat(var_name + idx + ".amplitude", amplitude);
        shader.setFloat(var_name + idx + ".phase", waves[i].phase());
        shader.setFloat(var_name + idx + ".phase_offset", waves[i].phase_offset());
        shader.setFloat(var_name + idx + ".chop", chop);
    }
}

std::vector<Wave> makeGeomWaves()
{
    std::vector<Wave> waves;
    waves.push_back(Wave(1., 35., 8, 1000., {1, 0.5}, 0.1));

    // waves.push_back(Wave(.5f, 35., 16., 100., { 1,0 }, 0.));
    // waves.push_back(Wave(.2f, 30., 2., 100., { 1,.25 }, 0.25));
    // waves.push_back(Wave(.2f, 25., 1., 100., { 1,-.25 }, 0.5));
    // waves.push_back(Wave(.2f, 15., 0., 100., { 1,.1 }, 0.75));

    return waves;
}

std::vector<Wave> makeTexWaves(size_t num_waves)
{
    std::vector<Wave> tx_waves;

    // for (size_t i = 0; i < 16; ++i)
    //{
    //	auto lambda = i * .4 + 0.3;
    //	tx_waves.push_back(Wave(.005 * lambda, lambda, 2, 1600., { 1, 0.5 + pow(-1, i)*i/15. },
    //1/16.));
    //}

    // for (auto i = 0; i < 1; ++i)
    //{
    //	tx_waves.push_back(Wave(1, 5, 0, 1600, { 1, 0 }, 1 / 16));
    //}

    for (size_t i = 0; i < num_waves; ++i)
    {
        // auto lambda = i * .1 + .3;
        // auto lambda = i * i * .05 + 0.3;
        auto r1 = (2 * float(std::rand()) / float(RAND_MAX) - 1.0) * .5;
        auto r2 = (2 * float(std::rand()) / float(RAND_MAX) - 1.0) * .5;
        auto r3 = (2 * float(std::rand()) / float(RAND_MAX) - 1.0) * 3;
        // auto amp = float(std::rand()) / float(RAND_MAX) * 0.02 + .01;
        auto lambda = float(std::rand()) / float(RAND_MAX) * 6.f + 0.3f;
        tx_waves.push_back(Wave(.005 * lambda, lambda, 0, 1600., {1 + r1, 0.5 + r2}, 1 / 16.));
        // tx_waves.push_back(Wave(amp, lambda, r3, 1600., { 1 + r1, 0.5+r2 }, 1 / 16.));
    }

    // for (size_t i = 0; i < 4; ++i)
    //{
    //	auto lambda = .5 * i  + 0.1;
    //	tx_waves.push_back(Wave(.005 * lambda, lambda, 4, 2500., { 1, .5 }, .2));
    //	tx_waves.push_back(Wave(.005 * lambda, lambda, 4, 2500., { 1, .65 }, .2));
    //	tx_waves.push_back(Wave(.005 * lambda, lambda, 4, 2500., { 1, .35 }, .2));
    //	tx_waves.push_back(Wave(.005 * lambda, lambda, 4, 2500., { 1, .4 }, .2));
    //}

    return tx_waves;
}

} // namespace msb