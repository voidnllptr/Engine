#include "Timer.hpp"

namespace Core {


    Timer::Timer() {
        reset();
    }

    void Timer::update() {
        tick();
    }

    void Timer::tick() {
        if (m_paused) {
            m_lastFrameTime = std::chrono::steady_clock::now();
            m_deltaTime = 0.0f;
            return;
        }

        auto currentTime = std::chrono::steady_clock::now();

        m_deltaTime = std::chrono::duration<float>(currentTime - m_lastFrameTime).count();

        const float MAX_DELTA_TIME = 0.1f;
        if (m_deltaTime > MAX_DELTA_TIME) {
            m_deltaTime = MAX_DELTA_TIME;
        }

        m_totalTime = std::chrono::duration<float>(currentTime - m_startTime).count();

        updateFPS();

        m_lastFrameTime = currentTime;
    }

    void Timer::updateFPS() {
        m_frameCount++;

        auto currentTime = std::chrono::steady_clock::now();
        float timeSinceLastFPS = std::chrono::duration<float>(currentTime - m_lastFPSTime).count();

        if (timeSinceLastFPS >= 1.0f) {
            m_fps = static_cast<float>(m_frameCount) / timeSinceLastFPS;

            if (m_smoothedFPS == 0.0f) {
                m_smoothedFPS = m_fps;
            }
            else {
                m_smoothedFPS = SMOOTHING_FACTOR * m_fps + (1.0f - SMOOTHING_FACTOR) * m_smoothedFPS;
            }

            m_frameCount = 0;
            m_lastFPSTime = currentTime;
        }
    }

    float Timer::getTotalTime() const {
        if (m_paused) {
            return m_totalTime;
        }

        auto currentTime = std::chrono::steady_clock::now();
        return std::chrono::duration<float>(currentTime - m_startTime).count();
    }

    long long Timer::getTotalTimeMs() const {
        if (m_paused) {
            return static_cast<long long>(m_totalTime * 1000.0f);
        }

        auto currentTime = std::chrono::steady_clock::now();
        return std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - m_startTime).count();
    }

    std::string Timer::getFPSString() const {
        int fpsInt = static_cast<int>(std::round(m_fps));
        return std::to_string(fpsInt);
    }

    void Timer::reset() {
        m_startTime = std::chrono::steady_clock::now();
        m_lastFrameTime = m_startTime;
        m_lastFPSTime = m_startTime;
        m_deltaTime = 0.0f;
        m_totalTime = 0.0f;
        m_frameCount = 0;
        m_fps = 0.0f;
        m_smoothedFPS = 0.0f;
        m_paused = false;
    }

    void Timer::sleepSeconds(float seconds) {
        std::this_thread::sleep_for(std::chrono::duration<float>(seconds));
    }

    void Timer::sleepMillis(uint32_t milliseconds) {
        std::this_thread::sleep_for(std::chrono::milliseconds(milliseconds));
    }

    void Timer::delayForTargetFPS(float targetFPS) {
        float targetFrameTime = 1.0f / targetFPS;

        if (m_deltaTime < targetFrameTime) {
            float sleepTime = targetFrameTime - m_deltaTime;
            sleepSeconds(sleepTime);
        }
    }

}