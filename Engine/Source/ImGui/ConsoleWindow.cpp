#include "BsPrecompileHeader.h"
#include "ConsoleWindow.h"
#include <iomanip>

namespace Pixie
{

    void ConsoleWindow::DrawLogLevelsPopup()
    {
        ImGui::MenuItem("Logging Levels", nullptr, false, false);
        //std::shared_ptr<spdlog::logger> coreLogger = Logger::GetCoreLogger();
        //std::shared_ptr<spdlog::logger> sandboxLogger = Logger::GetSandboxLogger();

        if (ImGui::BeginTable("##Logging Levels", 2))
        {
            float availableWidth = 180.0f;
            ImGui::TableSetupColumn(m_CoreLogName.data(), ImGuiTableColumnFlags_WidthFixed, availableWidth * 0.5);
            ImGui::TableSetupColumn(m_SandboxLogName.data(), ImGuiTableColumnFlags_WidthFixed, availableWidth * 0.5);
            
            for (int i = 0; i < m_CoreLogLevel.size(); i++)
            {
                ImGui::TableNextRow();
                ImGui::PushID(i);
                
                // core logger column
                ImGui::TableSetColumnIndex(0);

                ImGui::Text(m_CoreLogLevel[i].first.data());
                ImGui::SameLine(ImGui::GetContentRegionAvail().x - ImGui::GetFrameHeight());
                ImGui::Checkbox("##coreLevel", &(m_CoreLogLevel[i].second));

                // sandbox / game column
                ImGui::TableSetColumnIndex(1);

                ImGui::Text(m_SandboxLogLevel[i].first.data());
                ImGui::SameLine(ImGui::GetContentRegionAvail().x - ImGui::GetFrameHeight());
                ImGui::Checkbox("##sandboxLevel", &(m_SandboxLogLevel[i].second));
                
                ImGui::PopID();
            }

            ImGui::EndTable();
        }
    }
     
    void ConsoleWindow::DrawLogFormatPopup()
    {
        ImGui::MenuItem("Logging Format", nullptr, false, false);

        bool bFormatChanged = false;
        ImGui::Checkbox("Time", &m_ShowTime);
        
        //ImGui::SameLine();
        //ImGui::Checkbox("Thread", &m_ShowThread);
        ImGui::SameLine();
        ImGui::Checkbox("Level", &m_ShowLevel);
        
        ImGui::SameLine();
        ImGui::Checkbox("Logger", &m_ShowLogger);

    }

    bool ConsoleWindow::Draw()
    {
        ImGui::Begin("Console");
        
        if (Logger::GetCoreLogger() == nullptr || Logger::GetSandboxLogger() == nullptr)
        {
            ImGui::End();
            return false;
        }

        if (m_CoreLogName.empty())
        {
            m_CoreLogName = Logger::GetCoreLogger()->name();
        }

        if (m_SandboxLogName.empty())
        {
            m_SandboxLogName = Logger::GetSandboxLogger()->name();
        }

        // -------------------------------------------------------------------------
        // Toolbar
        // -------------------------------------------------------------------------

        {
            // Make all buttons transparent in the toolbar
            auto button_color = ImGui::GetStyleColorVec4(ImGuiCol_Button);
            button_color.w = 0.0F;
            ImGui::PushStyleColor(ImGuiCol_Button, button_color);

            if (ImGui::Button("Levels"))
            {
                ImGui::OpenPopup("LogLevelsPopup");
            }
            if (ImGui::IsItemHovered())
            {
                ImGui::SetTooltip("Change the logging levels");
            }
            if (ImGui::BeginPopup("LogLevelsPopup"))
            {
                DrawLogLevelsPopup();
                ImGui::EndPopup();
            }

            ImGui::SameLine();
            if (ImGui::Button("Format"))
            {
                ImGui::OpenPopup("LogFormatPopup");
            }
            if (ImGui::IsItemHovered())
            {
                ImGui::SetTooltip("Chose what information to show");
            }
            if (ImGui::BeginPopup("LogFormatPopup"))
            {
                DrawLogFormatPopup();
                ImGui::EndPopup();
            }

            ImGui::SameLine();
            if (ImGui::Button("Clear"))
            {
                ClearLogs();
            }
            if (ImGui::IsItemHovered())
            {
                ImGui::SetTooltip("Discard all messages");
            }

            ImGui::SameLine();
            bool need_pop_style_var = false;
            if (m_WrapText)
            {
                // Highlight the button
                ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 2.0F);
                ImGui::PushStyleColor(
                    ImGuiCol_Border, ImGui::GetStyleColorVec4(ImGuiCol_TextSelectedBg));
                need_pop_style_var = true;
            }
            if (ImGui::Button("Wrap Text"))
            {
                ToggleWrap();
            }
            if (ImGui::IsItemHovered())
            {
                ImGui::SetTooltip("Toggle soft wraping of text");
            }
            if (need_pop_style_var)
            {
                ImGui::PopStyleColor();
                ImGui::PopStyleVar();
            }

            ImGui::SameLine();
            need_pop_style_var = false;
            if (m_scrollLock)
            {
                // Highlight the button
                ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 2.0F);
                ImGui::PushStyleColor(
                    ImGuiCol_Border, ImGui::GetStyleColorVec4(ImGuiCol_TextSelectedBg));
                need_pop_style_var = true;
            }
            if (ImGui::Button("Lock"))
            {
                ToggleScrollLock();
            }
            if (ImGui::IsItemHovered())
            {
                ImGui::SetTooltip("Toggle automatic scrolling to the bottom");
            }
            if (need_pop_style_var)
            {
                ImGui::PopStyleColor();
                ImGui::PopStyleVar();
            }

            ImGui::SameLine();
            ImGui::Text("Search");
            ImGui::SameLine();
            m_DisplayFilter.Draw("##SearchFilter", 150.0F);

            ImGui::SameLine();
            ImGui::Text("Font Size");
            ImGui::SameLine();
            ImGui::PushItemWidth(75.0f);
            if (ImGui::InputInt("##FontSize", &m_FontSize))
            {
                m_FontSize = std::max(m_FontSize, 2);
            }
            ImGui::PopItemWidth();
        }
        // Restore the button color
        ImGui::PopStyleColor();

        // -------------------------------------------------------------------------
        // Log records
        // -------------------------------------------------------------------------

        ImGui::Separator();

        ImGui::BeginChild("scrolling", ImVec2(0, 0), false, ImGuiWindowFlags_HorizontalScrollbar);

        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 1));
        ImGui::PushFont(NULL, m_FontSize);

        std::shared_lock<std::shared_timed_mutex> lock(m_LogHistory_mutex);
        for (auto const& log : m_LogHistory)
        {
            if (log.Level == LogLevel::Off) continue;

            switch (log.Level)
            {
            case Pixie::LogLevel::Trace:
                if (log.LoggerName == m_CoreLogName && !m_CoreLogLevel[0].second) continue;
                if (log.LoggerName == m_SandboxLogName && !m_SandboxLogLevel[0].second) continue;
                break;
            case Pixie::LogLevel::Info:
                if (log.LoggerName == m_CoreLogName && !m_CoreLogLevel[1].second) continue;
                if (log.LoggerName == m_SandboxLogName && !m_SandboxLogLevel[1].second) continue;
                break;
            case Pixie::LogLevel::Debug:
                if (log.LoggerName == m_CoreLogName && !m_CoreLogLevel[2].second) continue;
                if (log.LoggerName == m_SandboxLogName && !m_SandboxLogLevel[2].second) continue;
                break;
            case Pixie::LogLevel::Warning:
                if (log.LoggerName == m_CoreLogName && !m_CoreLogLevel[3].second) continue;
                if (log.LoggerName == m_SandboxLogName && !m_SandboxLogLevel[3].second) continue;
                break;
            case Pixie::LogLevel::Error:
                if (log.LoggerName == m_CoreLogName && !m_CoreLogLevel[4].second) continue;
                if (log.LoggerName == m_SandboxLogName && !m_SandboxLogLevel[4].second) continue;
                break;
            case Pixie::LogLevel::Critical:
                if (log.LoggerName == m_CoreLogName && !m_CoreLogLevel[5].second) continue;
                if (log.LoggerName == m_SandboxLogName && !m_SandboxLogLevel[5].second) continue;
                break;
            case Pixie::LogLevel::Off:
                continue;
                break;
            default:
                break;
            }

            bool logPassesFilter = !m_DisplayFilter.IsActive()
                //|| m_DisplayFilter.PassFilter(log.Properties.c_str(), log.Properties.c_str() + log.Properties.size())
                //|| m_DisplayFilter.PassFilter(log.Source.c_str(), log.Source.c_str() + log.Source.size())
                || m_DisplayFilter.PassFilter(log.Message.c_str(), log.Message.c_str() + log.Message.size() )
                
                ;
            if (!logPassesFilter) continue;

            ImGui::BeginGroup();

            if (log.Emphasis)
            {
                //TODO: implement font weights for imgui
            }

            std::string label;
            if (m_ShowTime)
            {
                struct tm localTime;
                localtime_s(&localTime, &log.Time);
                std::stringstream stream;
                stream << std::put_time(&localTime, m_TimeFormat.c_str());

                ImGui::Text("[%s] ", stream.str().c_str());
                ImGui::SameLine();
            }

            if (m_ShowLevel)
            {
                ImGui::Text("[");
                ImGui::SameLine();
                ImGui::TextColored(log.Color, m_CoreLogLevel[(int)log.Level].first.data());
                ImGui::SameLine();
                ImGui::Text("] ");
                ImGui::SameLine();
            }

            if (m_ShowLogger)
            {
                ImGui::Text(log.LoggerName.data());
                ImGui::SameLine();
                ImGui::Text(": ");
                ImGui::SameLine();
            }

            if (m_WrapText)
                ImGui::PushTextWrapPos(ImGui::GetCursorPos().x + ImGui::GetContentRegionAvail().x);

            ImGui::TextColored(log.Color, "%s", log.Message.c_str());
            if(m_WrapText)
                ImGui::PopTextWrapPos();
    
            ImGui::EndGroup();
        }

        if (!m_scrollLock && m_scrollToBottom)
        {
            ImGui::SetScrollHereY(1.0F);
        }
        m_scrollToBottom = false;

        ImGui::PopFont();
        ImGui::PopStyleVar();
        ImGui::EndChild();
        ImGui::End();
        return true;
    }

    void ConsoleWindow::RecieveLog(LogData& log)
    {
        std::unique_lock<std::shared_timed_mutex> lock(m_LogHistory_mutex);
        
        
        m_LogHistory.push_back(std::move(log));


        m_scrollToBottom = true;
    }

    void ConsoleWindow::ClearLogs()
    {
        std::unique_lock<std::shared_timed_mutex> lock(m_LogHistory_mutex);
        m_LogHistory.clear();
    }

}