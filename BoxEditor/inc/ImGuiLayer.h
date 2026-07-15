
enum class MenuAction
{
    None,
    Exit,
    NewScene,
    OpenScene,
    SaveScene
};

struct GLFWwindow;


class ImGuiLayer
{
public:
    ImGuiLayer() = default;
    ~ImGuiLayer() = default;

    bool Initialize(GLFWwindow* window);
    void BeginFrame();


    // Draw all editor panels here
    void testwindow();
    MenuAction DrawMainMenu(); // dedicated panel classes once they grow.
    /*void DrawSceneViewport();
    void DrawObjectExplorer();
    void DrawInspector();*/


    // Docking control
    void SetEnableDocking(bool enabled);
    bool GetEnableDocking() const;
    void MainDockSpace(bool* p_open); // docking space

    void RenderImGui(); // finish ImGui frame and render
    void ImGuiShutdown();
 

private:
    bool m_initialized = false;
	bool m_enableDocking = false;
};