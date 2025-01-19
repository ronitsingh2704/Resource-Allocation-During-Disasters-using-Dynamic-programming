# Resource-Allocation-During-Disasters-using-Dynamic-programming
# Disaster Resource Allocation System

This project implements a graphical and interactive Disaster Resource Allocation System using **C++** and **OpenGL** with **Dear ImGui** for GUI elements. The system models the allocation of resources such as medicine, food, and water between multiple cities and headquarters (HQs), based on user inputs and predefined algorithms.

## Features

- **Graph Visualization**: Render cities, headquarters, and routes as an adjacency matrix.
- **Shortest Path Calculation**: Find the shortest path between a headquarters and a city using Dijkstra's algorithm.
- **Resource Allocation**: Allocate resources from the nearest HQ to a city while considering constraints.
- **Resource Update**: Update the available resources at a headquarters.
- **Dynamic Input**: Add cities, HQs, resources, and routes dynamically.
- **Interactive GUI**: Simple and intuitive user interface using Dear ImGui.

## Dependencies

Ensure the following libraries and tools are installed:

- **GLFW**: For handling window management.
- **GLAD**: For OpenGL function loading.
- **Dear ImGui**: For GUI implementation.
- **C++ Compiler**: Compatible with C++17 or later.

## Setup Instructions

1. **Clone the repository**:
   ```bash
   git clone https://github.com/ronit/Disaster-Resource-Allocation.git
   cd Disaster-Resource-Allocation
