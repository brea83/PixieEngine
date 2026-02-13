#pragma once
#include "ImGuiPanel.h"
#include "Log.h"
#include <shared_mutex>
#include <chrono>
#include <ctime>

namespace Pixie
{
    struct LogData
    {
        LogData() = default;

        LogLevel Level{LogLevel::Off};
        std::string_view LoggerName;
        std::time_t Time{ 0 };
        //std::string Source;
        std::string Message{ "" };
        //std::size_t Color_range_start{ 0 };
        //std::size_t Color_range_end{ 0 };
        ImVec4 Color{ 0.9F, 0.7F, 0.0F, 1.0F };
        bool Emphasis{ false };
    };

    class ConsoleWindow : public ImGuiPanel
    {
    public:
        ConsoleWindow() {}
        virtual ~ConsoleWindow() {}

        bool IsTimeShown() const { return m_ShowTime; }
        bool IsLevelShown() const { return m_ShowLevel; }
        bool IsLoggerShown() const { return m_ShowLogger; }
        void ToggleWrap() { m_WrapText = !m_WrapText; }
        void ToggleScrollLock() { m_scrollLock = !m_scrollLock; }

        void DrawLogLevelsPopup();
        void DrawLogFormatPopup();

        // Inherited via ImGuiPanel
        bool Draw() override;
        // do not use, not implemented
        int DrawReturnsInt() override { return 0; };


        void RecieveLog(LogData& log);
        void ClearLogs();
    protected:

        std::vector<LogData> m_LogHistory;
        mutable std::shared_timed_mutex m_LogHistory_mutex;

        ImGuiTextFilter m_DisplayFilter;
        std::string_view m_CoreLogName;
        std::string_view m_SandboxLogName;


        std::vector<std::pair<std::string_view, bool>> m_CoreLogLevel{ {"Trace", true}, {"Info", true}, {"Debug", true}, {"Warning", true}, {"Error", true}, {"Critical", true} };
        std::vector<std::pair<std::string_view, bool>> m_SandboxLogLevel{ {"Trace", true}, {"Info", true}, {"Debug", true}, {"Warning", true}, {"Error", true}, {"Critical", true} };

        float m_WrapWidth{ 200.0f };
        int m_FontSize{ 15 };

        std::string m_TimeFormat{"%Y-%m-%d %H:%M:%S"};

        bool m_scrollToBottom{ false };
        bool m_WrapText{ false };
        bool m_scrollLock{ false };

        bool m_ShowTime{ false };
        bool m_ShowLevel{ true };
        bool m_ShowLogger{ true };

    };

}