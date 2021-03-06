#include <igl/opengl/glfw/Viewer.h>
#include <igl/opengl/glfw/imgui/ImGuiMenu.h>
#include <igl/opengl/glfw/imgui/ImGuiHelpers.h>
#include <imgui/imgui.h>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "scene.h"

int main(int argc, char *argv[]){

    // Initialise the viewer
	igl::opengl::glfw::Viewer viewer;

	// Attach a menu plugin
	igl::opengl::glfw::imgui::ImGuiMenu menu;
	viewer.plugins.push_back(&menu);
    
    Scene scene(viewer);

	// Menu variable shared between two menus
    int eigenvector = 5;
    int iteration = 200;
    double lambda = 0.00000015;
    double noise = 0.5;
	double curvature_display_scale = 5;
	int compare = 0;
    static const char *models[]{"bunny.off","cow.off","cow_manifold.off","cow_manifold2.off","camel.off","dragon.off"};
    static int model_index = 0;
	
    scene.SetNumEigenvector(eigenvector);
    scene.SetIteration(iteration);
    scene.SetLambda(lambda);
    scene.SetNoise(noise);
	scene.SetCurvatureDisplayScale(curvature_display_scale);

    // Draw an optional panel for adjusting global variables
    menu.callback_draw_viewer_menu = [&]()
    {
        // Draw parent menu content
        menu.draw_viewer_menu();
        
        // Add a panel
        if (ImGui::CollapsingHeader("Utilities", ImGuiTreeNodeFlags_DefaultOpen))
        {
            if (ImGui::Combo("Model to Use", &model_index, models, IM_ARRAYSIZE(models))){
                scene.Initialise(models[model_index]);
            }

            if (ImGui::Button("Reset Scene", ImVec2(-1, 0))) {
                scene.Initialise(models[model_index]);
            }

			if (ImGui::InputDouble("Curvature Scale", &curvature_display_scale, 0, 0, "%.1f")) {
				scene.SetCurvatureDisplayScale(curvature_display_scale);
				scene.VisualiseCurvature();
			}
        }
    };
    
    // Draw a separate panel for performing tasks
    menu.callback_draw_custom_window = [&]()
    {
        ImGui::SetNextWindowPos(ImVec2(180.f * menu.menu_scaling(), 0), ImGuiSetCond_FirstUseEver);
        ImGui::SetNextWindowSize(ImVec2(448, 384), ImGuiSetCond_FirstUseEver);
        ImGui::Begin( "Operations", nullptr, ImGuiWindowFlags_NoSavedSettings );

        if (ImGui::CollapsingHeader("Curvature Discretisation", ImGuiTreeNodeFlags_DefaultOpen))
        {
            if (ImGui::Button("Uniform Mean Curvature", ImVec2(-1, 0))){
                scene.Discretisation(0);
            }

            if (ImGui::Button("Gaussian Curvature", ImVec2(-1, 0))){
                scene.Discretisation(1);
            }

            if (ImGui::Button("Non-Uniform Mean Curvature", ImVec2(-1, 0))){
                scene.Discretisation(2);
            }
        }

        if (ImGui::CollapsingHeader("Mesh Reconstruction", ImGuiTreeNodeFlags_DefaultOpen))
        {
            if(ImGui::InputInt("Number of Eigenvectors", &eigenvector)){
                scene.SetNumEigenvector(eigenvector);
            }

            if (ImGui::Button("Reconstruction", ImVec2(-1, 0))){
                scene.Reconstruction();
            }
        }

        if (ImGui::CollapsingHeader("Mesh Smoothing", ImGuiTreeNodeFlags_DefaultOpen))
        {

            if(ImGui::InputInt("Iteration", &iteration)){
                scene.SetIteration(iteration);
            }

            if(ImGui::InputDouble("Lambda", &lambda, 0, 0, "%.10f")){
                scene.SetLambda(lambda);
            }

            if (ImGui::Button("Explicit Smoothing", ImVec2(-1, 0))){
                scene.Smoothing(0);
				compare = 1;
            }

            if (ImGui::Button("Implicit Smoothing", ImVec2(-1, 0))){
                scene.Smoothing(1);
				compare = 1;
            }

            if(ImGui::InputDouble("Noise", &noise, 0, 0, "%.10f")){
                scene.SetNoise(noise);
            }

            if (ImGui::Button("Add Noise", ImVec2(-1, 0))) {
                scene.AddNoise();
				compare = 0;
            }

			if (ImGui::SliderInt("Unsmoothed/Smoothed", &compare, 0, 1))
			{
				scene.VisualiseComparison(compare);
			}
        }

        ImGui::End();
    };

    // Initialise the scene
	scene.Initialise(models[model_index]);

	// Call GUI
	viewer.launch();
}
