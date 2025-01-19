#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <C:\Users\ronit\source\repos\minor\minor\imgui\imgui.h>
#include <C:\Users\ronit\source\repos\minor\minor\imgui\imgui_impl_glfw.h>
#include <C:\Users\ronit\source\repos\minor\minor\imgui\imgui_impl_opengl3.h>
#include <iostream>
#include <vector>
#include <cmath>
#include <climits>
#include <string>

// Constants
#define MAX_CITIES 100

// Data Structures
int total_cities = 0;                  // Total number of cities + HQs
int hq = 0;                            // Number of headquarters
float points[MAX_CITIES][2];           // Coordinates for cities and HQs
int graph[MAX_CITIES][MAX_CITIES];     // Adjacency matrix
int resources[MAX_CITIES][3];          // Resources at HQs (medicine, food, water)
int allocated_resources[MAX_CITIES][3];// Allocated resources for cities

// Function prototypes
void initGraph();
void renderInputs();
void renderVisualization();
void displayAdjacencyMatrix();
void calculateShortestPath();
void allocateResources();
void updateResources();
float randomFloat(float min, float max);
void resetState();

// Global UI state
bool show_adjacency_matrix = false;
bool show_shortest_path = false;
bool show_allocation = false;
bool show_update = false;
int start_node = 0, end_node = 0;
int selected_city = 0;

// Reset state
void resetState() {
    show_adjacency_matrix = false;
    show_shortest_path = false;
    show_allocation = false;
    show_update = false;
}

// Initialize graph with zero distances
void initGraph() {
    for (int i = 0; i < MAX_CITIES; i++) {
        for (int j = 0; j < MAX_CITIES; j++) {
            graph[i][j] = 0;
        }
    }
}

// Generate random float for visualization
float randomFloat(float min, float max) {
    return min + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / (max - min)));
}

void calculateShortestPath() {
    // Begin ImGui window for shortest path calculation
    ImGui::Begin("Shortest Path");

    // Input fields for start (HQ) and end (City)
    static int start_hq = 0;  // Using static to preserve values across frames
    static int end_city = 0;
    ImGui::InputInt("Start Headquarters (HQ)", &start_hq);
    ImGui::InputInt("End City", &end_city);

    static std::vector<int> path;     // Stores the shortest path
    static int total_distance = -1;  // Stores the shortest path distance
    static bool no_path = false;     // Indicates if no path exists

    // Check if "Calculate" button is pressed
    if (ImGui::Button("Calculate")) {
        // Validate inputs
        if (start_hq < 0 || start_hq >= hq || end_city < hq || end_city >= total_cities) {
            ImGui::Text("Invalid input. Start must be HQ, and end must be a City.");
        }
        else {
            // Initialize variables for Dijkstra's algorithm
            std::vector<int> dist(total_cities, INT_MAX); // Distances
            std::vector<bool> visited(total_cities, false); // Visited nodes
            std::vector<int> parent(total_cities, -1); // Parent nodes for path reconstruction
            dist[start_hq] = 0; // Distance to start node is 0

            // Dijkstra's algorithm
            for (int i = 0; i < total_cities; i++) {
                int u = -1;

                // Find the unvisited node with the smallest distance
                for (int j = 0; j < total_cities; j++) {
                    if (!visited[j] && (u == -1 || dist[j] < dist[u])) {
                        u = j;
                    }
                }

                // If no valid node is found, break
                if (u == -1 || dist[u] == INT_MAX) break;

                visited[u] = true;

                // Relax edges
                for (int v = 0; v < total_cities; v++) {
                    if (graph[u][v] > 0 && dist[u] + graph[u][v] < dist[v]) {
                        dist[v] = dist[u] + graph[u][v];
                        parent[v] = u;
                    }
                }
            }

            // Reset previous path and results
            path.clear();
            no_path = (dist[end_city] == INT_MAX);
            total_distance = no_path ? -1 : dist[end_city];

            // Construct the shortest path
            if (!no_path) {
                for (int v = end_city; v != -1; v = parent[v]) {
                    path.push_back(v);
                }
                std::reverse(path.begin(), path.end());
            }
        }
    }

    // Display results
    if (no_path) {
        ImGui::Text("No path exists between HQ %d and City %d.", start_hq, end_city - hq);
    }
    else if (total_distance != -1) {
        ImGui::Text("Shortest Path Distance: %d", total_distance);
        ImGui::Text("Path:");
        for (size_t i = 0; i < path.size(); i++) {
            std::string label = (path[i] < hq) ? "HQ " + std::to_string(path[i]) : "City " + std::to_string(path[i] - hq);
            ImGui::Text("%s%s", label.c_str(), (i < path.size() - 1) ? " -> " : "");
        }
    }

    // End ImGui window
    ImGui::End();
}



// Allocate resources to selected city
// Allocate resources to a city based on the nearest headquarters
void allocateResources() {
    ImGui::Begin("Allocate Resources");

    static int target_city = 0; // The city to allocate resources to
    static int selected_hq = -1; // The selected headquarters for allocation
    static int medicine = 0, food = 0, water = 0;
    static std::vector<int> nearest_hq_path; // Stores the path to the nearest HQ
    static int nearest_distance = -1; // Distance to the nearest HQ
    static bool allocation_successful = false;

    ImGui::InputInt("Target City", &target_city);
    if (target_city < hq) target_city = hq; // Ensure it's not an HQ
    if (target_city >= total_cities) target_city = total_cities - 1;

    if (ImGui::Button("Find Nearest HQ")) {
        allocation_successful = false;
        selected_hq = -1;
        nearest_hq_path.clear();
        nearest_distance = -1;

        // Calculate the nearest HQ using Dijkstra's algorithm
        std::vector<int> dist(total_cities, INT_MAX);
        std::vector<bool> visited(total_cities, false);
        std::vector<int> parent(total_cities, -1);

        dist[target_city] = 0;

        for (int i = 0; i < total_cities; i++) {
            int u = -1;

            // Find the nearest unvisited node
            for (int j = 0; j < total_cities; j++) {
                if (!visited[j] && (u == -1 || dist[j] < dist[u])) {
                    u = j;
                }
            }

            if (u == -1 || dist[u] == INT_MAX) break;

            visited[u] = true;

            // Relax edges
            for (int v = 0; v < total_cities; v++) {
                if (graph[u][v] > 0 && dist[u] + graph[u][v] < dist[v]) {
                    dist[v] = dist[u] + graph[u][v];
                    parent[v] = u;
                }
            }
        }

        // Find the nearest HQ
        for (int i = 0; i < hq; i++) {
            if (dist[i] < nearest_distance || nearest_distance == -1) {
                nearest_distance = dist[i];
                selected_hq = i;

                // Reconstruct the path
                nearest_hq_path.clear();
                for (int v = i; v != -1; v = parent[v]) {
                    nearest_hq_path.push_back(v);
                }
                std::reverse(nearest_hq_path.begin(), nearest_hq_path.end());
            }
        }

        if (selected_hq != -1) {
            ImGui::Text("Nearest HQ: HQ %d", selected_hq);
            ImGui::Text("Distance: %d", nearest_distance);
        }
        else {
            ImGui::Text("No HQ is reachable from this city.");
        }
    }

    if (selected_hq != -1) {
        ImGui::InputInt("Medicine", &medicine);
        ImGui::InputInt("Food", &food);
        ImGui::InputInt("Water", &water);

        if (ImGui::Button("Allocate")) {
            if (medicine <= resources[selected_hq][0] &&
                food <= resources[selected_hq][1] &&
                water <= resources[selected_hq][2]) {
                // Deduct resources from HQ
                resources[selected_hq][0] -= medicine;
                resources[selected_hq][1] -= food;
                resources[selected_hq][2] -= water;

                // Allocate resources to the city
                allocated_resources[target_city][0] += medicine;
                allocated_resources[target_city][1] += food;
                allocated_resources[target_city][2] += water;

                allocation_successful = true;
            }
            else {
                ImGui::Text("Insufficient resources in the selected HQ.");
            }
        }

        if (allocation_successful) {
            ImGui::Text("Resources allocated successfully!");
            // Show the HQ, distance, and path after successful allocation
            ImGui::Text("Resources provided by HQ %d", selected_hq);
            ImGui::Text("Distance: %d", nearest_distance);

            ImGui::Text("Path: ");
            for (int i = 0; i < nearest_hq_path.size(); i++) {
                ImGui::Text("%d", nearest_hq_path[i]);
                if (i < nearest_hq_path.size() - 1) {
                    ImGui::SameLine();
                    ImGui::Text("->");
                }
            }
        }
    }

    ImGui::End();
}


// Update resource availability at a specific headquarters
void updateResources() {
    ImGui::Begin("Update Resources");

    static int target_hq = 0;
    static int medicine = 0, food = 0, water = 0;
    static bool update_successful = false; // Tracks if the update was successful
    static float success_timer = 0.0f;    // Timer to control message visibility

    ImGui::InputInt("Headquarters to Update", &target_hq);
    if (target_hq < 0) target_hq = 0;
    if (target_hq >= hq) target_hq = hq - 1;

    ImGui::InputInt("Medicine", &medicine);
    ImGui::InputInt("Food", &food);
    ImGui::InputInt("Water", &water);

    if (ImGui::Button("Update")) {
        resources[target_hq][0] = std::max(0, medicine);
        resources[target_hq][1] = std::max(0, food);
        resources[target_hq][2] = std::max(0, water);

        update_successful = true; // Indicate success
        success_timer = 2.0f;     // Set a timer (2 seconds for message display)
    }

    // Display success message if update was successful
    if (update_successful) {
        ImGui::Text("Resources updated successfully!");
        success_timer -= ImGui::GetIO().DeltaTime; // Decrease timer
        if (success_timer <= 0.0f) {
            update_successful = false; // Reset success state
        }
    }

    ImGui::End();
}

// Display adjacency matrix
void displayAdjacencyMatrix() {
    ImGui::Begin("Adjacency Matrix");
    ImGui::Text("Graph Adjacency Matrix");

    for (int i = 0; i < total_cities; i++) {
        for (int j = 0; j < total_cities; j++) {
            // If the distance from i to j is set, make sure the reverse is also the same
            if (graph[i][j] != 0 && graph[j][i] == 0) {
                graph[j][i] = graph[i][j];  // Set the reverse direction distance
            }

            // Display the distance in the matrix
            ImGui::Text("%d ", graph[i][j]);
            if (j + 1 < total_cities) ImGui::SameLine();
        }
    }
    ImGui::End();
}

void renderInputs() {
    ImGui::Begin("Input Data");

    // Number of headquarters
    ImGui::InputInt("Number of Headquarters (HQ)", &hq);
    if (hq < 0) hq = 0;
    if (hq > MAX_CITIES) hq = MAX_CITIES;

    // Total cities (including HQs)
    ImGui::InputInt("Total Cities (including HQ)", &total_cities);
    if (total_cities < hq) total_cities = hq; // Ensure cities >= HQs
    if (total_cities > MAX_CITIES) total_cities = MAX_CITIES;

    // Input resources for each HQ
    if (ImGui::CollapsingHeader("Resources per HQ")) {
        for (int i = 0; i < hq; i++) {
            ImGui::Text("HQ %d", i + 1); // Display HQ number (1-based index for clarity)

            // Properly scope the labels for each HQ
            std::string medicine_label = "Medicine##" + std::to_string(i);
            std::string food_label = "Food##" + std::to_string(i);
            std::string water_label = "Water##" + std::to_string(i);

            ImGui::InputInt(medicine_label.c_str(), &resources[i][0]);
            ImGui::InputInt(food_label.c_str(), &resources[i][1]);
            ImGui::InputInt(water_label.c_str(), &resources[i][2]);

            // Ensure no negative resource values
            if (resources[i][0] < 0) resources[i][0] = 0;
            if (resources[i][1] < 0) resources[i][1] = 0;
            if (resources[i][2] < 0) resources[i][2] = 0;
        }
    }

    // Input adjacency matrix
    if (ImGui::CollapsingHeader("Adjacency Matrix (Distances)")) {
        for (int i = 0; i < total_cities; i++) {
            for (int j = i + 1; j < total_cities; j++) { // Start j from i + 1 to avoid duplicates
                std::string label_i = (i < hq) ? "HQ " + std::to_string(i) : "City " + std::to_string(i - hq);
                std::string label_j = (j < hq) ? "HQ " + std::to_string(j) : "City " + std::to_string(j - hq);

                ImGui::InputInt(("Distance " + label_i + " -> " + label_j).c_str(), &graph[i][j]);
                if (graph[i][j] < 0) graph[i][j] = 0; // Ensure no negative weights
                graph[j][i] = graph[i][j]; // Fill symmetric value
            }
        }
    }

    // Buttons for actions
    if (ImGui::Button("Visualize")) {
        resetState(); // Clear any previous state
    }
    ImGui::SameLine();
    if (ImGui::Button("Show Adjacency Matrix")) {
        show_adjacency_matrix = true;
    }
    ImGui::SameLine();
    if (ImGui::Button("Shortest Path")) {
        show_shortest_path = true;
    }
    ImGui::SameLine();
    if (ImGui::Button("Allocate Resources")) {
        show_allocation = true;
    }
    ImGui::SameLine();
    if (ImGui::Button("Update Resources")) {
        show_update = true;
    }

    ImGui::End();
}


void renderVisualization() {
    ImGui::Begin("Visualization");

    // Generate random positions for visualization
    for (int i = 0; i < total_cities; i++) {
        if (points[i][0] == 0 && points[i][1] == 0) { // Generate only if not set
            points[i][0] = randomFloat(-0.9f, 0.9f); // X-coordinate
            points[i][1] = randomFloat(-0.9f, 0.9f); // Y-coordinate
        }
    }

    // Draw cities and HQs
    ImDrawList* draw_list = ImGui::GetWindowDrawList();
    ImVec2 window_pos = ImGui::GetCursorScreenPos();
    ImVec2 window_size = ImGui::GetContentRegionAvail();
    ImVec2 center = ImVec2(window_pos.x + window_size.x / 2, window_pos.y + window_size.y / 2);

    for (int i = 0; i < total_cities; i++) {
        // Node position for visualization
        ImVec2 node_pos = ImVec2(center.x + points[i][0] * window_size.x / 2,
            center.y + points[i][1] * window_size.y / 2);

        // Draw connections (edges)
        for (int j = 0; j < total_cities; j++) {
            if (graph[i][j] > 0) { // Only draw if there is a connection
                ImVec2 target_pos = ImVec2(center.x + points[j][0] * window_size.x / 2,
                    center.y + points[j][1] * window_size.y / 2);
                draw_list->AddLine(node_pos, target_pos, IM_COL32(255, 255, 255, 255), 2.0f);
            }
        }

        // Determine color (blue for HQ, green for other cities)
        ImU32 color = (i < hq) ? IM_COL32(0, 0, 255, 255) : IM_COL32(0, 255, 0, 255);

        // Draw nodes
        draw_list->AddCircleFilled(node_pos, 10.0f, color);

        // Label nodes
        std::string label = (i < hq) ? "HQ " + std::to_string(i) : "City " + std::to_string(i - hq);
        draw_list->AddText(ImVec2(node_pos.x - 10.0f, node_pos.y - 20.0f), IM_COL32(255, 255, 255, 255), label.c_str());
    }

    ImGui::End();
}


int main() {
    // Initialize data
    initGraph();

    // Initialize GLFW
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return -1;
    }

    // Create window
    GLFWwindow* window = glfwCreateWindow(800, 600, "Disaster Management System", NULL, NULL);
    if (!window) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    // Initialize GLAD
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // Setup Dear ImGui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    (void)io;
    ImGui::StyleColorsDark();

    // Initialize ImGui backends
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // Render GUI
        renderInputs();
        renderVisualization();

        // Render additional functionalities
        if (show_adjacency_matrix) displayAdjacencyMatrix();
        if (show_shortest_path) calculateShortestPath();
        if (show_allocation) allocateResources();
        if (show_update) updateResources();

        // Rendering
        ImGui::Render();
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(window);
    }

    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
