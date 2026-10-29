#include "../inc/UI/Window.h"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <GLFW/glfw3.h>
#include "../inc/third_party/Menlo-Regular.h"

void InitFonts() {
    ImGuiIO& io = ImGui::GetIO();

    // Настройки для кириллицы (если нужны)
    ImFontConfig config;
    config.FontDataOwnedByAtlas = false;

    const ImWchar* glyph_ranges = io.Fonts->GetGlyphRangesCyrillic();

    // Загружаем шрифт ИЗ ПАМЯТИ
    // ВАЖНО: ImGui по умолчанию НЕ копирует этот массив, а читает его напрямую.
    // Так как наш массив static const, он живет все время работы программы, что безопасно.
    ImFont* embedded_font = io.Fonts->AddFontFromMemoryTTF(
        (void*)menlo_regular, // Указатель на массив байт
        menlo_size,               // Размер массива
        14.0f,               // Размер шрифта в пикселях
        &config,             // Конфигурация
        glyph_ranges         // Диапазон символов
    );

    if (embedded_font == nullptr) {
        // Ошибка (например, массив данных поврежден)
    }
}

int main( int argc, char* argv[] ) 
{
    try {
        glfwInit();
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

        GLFWwindow* window = glfwCreateWindow(1024, 768, "Network Scanner", nullptr, nullptr);
        glfwMakeContextCurrent(window);
        glfwSwapInterval(1);

        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO();
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
        // Переходим на стандартную темную тему как на базовую базу
        ImGui::StyleColorsDark();

        ImGuiStyle& style = ImGui::GetStyle();

        // --- НАСТРОЙКА СКРУГЛЕНИЙ (ROUNDING) ---
        style.FrameRounding = 6.0f;     // Скругление кнопок, инпутов и полей ввода (то, что вам нужно)
        style.WindowRounding = 8.0f;    // Скругление углов у главного и дочерних окон
        style.ChildRounding = 6.0f;     // Скругление под-зон (child windows)
        style.PopupRounding = 6.0f;     // Скругление контекстных меню и всплывающих окон
        style.GrabRounding = 4.0f;      // Скругление ползунков (sliders)
        style.TabRounding = 4.0f;       // Скругление вкладок (tabs)
        style.ScrollbarRounding = 12.0f;// Делаем скроллбар круглым и аккуратным

        // --- НЕОБЯЗАТЕЛЬНО: Подстройка цветов для более современного "плоского" дизайна ---
        style.Colors[ImGuiCol_Button]        = ImVec4(0.20f, 0.25f, 0.35f, 1.00f); // Спокойный сине-серый
        style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.28f, 0.35f, 0.48f, 1.00f); // Чуть ярче при наведении
        style.Colors[ImGuiCol_ButtonActive]  = ImVec4(0.15f, 0.18f, 0.27f, 1.00f); // Темнее при клике

        // Настройки таблицы, чтобы они сочетались со стилем
        style.Colors[ImGuiCol_TableHeaderBg] = ImVec4(0.15f, 0.15f, 0.18f, 1.00f);
        style.Colors[ImGuiCol_TableRowBg]     = ImVec4(0.11f, 0.11f, 0.13f, 1.00f);
        style.Colors[ImGuiCol_TableRowBgAlt]  = ImVec4(0.14f, 0.14f, 0.16f, 1.00f);

        InitFonts();

        ImGui_ImplGlfw_InitForOpenGL(window, true);
        ImGui_ImplOpenGL3_Init("#version 150");

        asio::io_context io_context;
        auto work_guard = asio::make_work_guard(io_context);
        std::thread asio_thread([&io_context]() { io_context.run(); });
        Window app(io_context);

        while (!glfwWindowShouldClose(window)) {
            glfwPollEvents();

            ImGui_ImplOpenGL3_NewFrame();
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();

            app.draw();

            ImGui::Render();
            int display_w, display_h;
            glfwGetFramebufferSize(window, &display_w, &display_h);
            glViewport(0, 0, display_w, display_h);
            glClearColor(0.1f, 0.1f, 0.12f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT);
            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

            glfwSwapBuffers(window);
        }

        io_context.stop();
        if (asio_thread.joinable()) asio_thread.join();

        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();

        glfwDestroyWindow(window);
        glfwTerminate();
    } catch (const std::exception &e) {
        std::cerr << e.what() << std::endl;
        return 1;
    }
}