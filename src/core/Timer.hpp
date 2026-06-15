#pragma once

#include <chrono>
#include <string>
#include <thread>
#include <cmath>

namespace Core {

    class Timer {
    public:
        Timer();

        void update();

        void tick();

        float getDeltaTime() const { return m_deltaTime; }

        float getDeltaTimeMs() const { return m_deltaTime * 1000.0f; }

        float getTotalTime() const;

        long long getTotalTimeMs() const;

        float getFPS() const { return m_fps; }

        std::string getFPSString() const;

        float getSmoothedFPS() const { return m_smoothedFPS; }

        void reset();

        void setPaused(bool paused) { m_paused = paused; }
        bool isPaused() const { return m_paused; }

        static void sleepSeconds(float seconds);

        static void sleepMillis(uint32_t milliseconds);

        void delayForTargetFPS(float targetFPS);

    private:
        void updateFPS();

        std::chrono::steady_clock::time_point m_startTime;
        std::chrono::steady_clock::time_point m_lastFrameTime;
        std::chrono::steady_clock::time_point m_lastFPSTime;

        float m_deltaTime = 0.0f;
        float m_totalTime = 0.0f;

        int m_frameCount = 0;
        float m_fps = 0.0f;
        float m_smoothedFPS = 0.0f;

        bool m_paused = false;

        static constexpr float SMOOTHING_FACTOR = 0.1f;
        static constexpr float TARGET_FPS = 60.0f;
    };

}