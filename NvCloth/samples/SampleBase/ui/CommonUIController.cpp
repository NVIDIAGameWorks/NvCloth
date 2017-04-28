/*
* Copyright (c) 2008-2017, NVIDIA CORPORATION.  All rights reserved.
*
* NVIDIA CORPORATION and its licensors retain all intellectual property
* and proprietary rights in and to this software, related documentation
* and any modifications thereto.  Any use, reproduction, disclosure or
* distribution of this software and related documentation without an express
* license agreement from NVIDIA CORPORATION is strictly prohibited.
*/

#include "CommonUIController.h"

#include "Renderer.h"
#include "SceneController.h"
#include "SampleController.h"
#include "SampleProfiler.h"

#include <imgui.h>
#include "imgui_impl_dx11.h"
#include "UIHelpers.h"

#include <cstdio>
#include <inttypes.h>


inline float memorySizeOutput(const char*& prefix, float value)
{
	for (prefix = "\0\0k\0M\0G\0T\0P\0E"; value >= 1024 && *prefix != 'E'; value /= 1024, prefix += 2);
	return value;
}

CommonUIController::CommonUIController()
{
}

HRESULT CommonUIController::DeviceCreated(ID3D11Device* pDevice)
{
	DeviceManager* manager = GetDeviceManager();
	ID3D11DeviceContext* pd3dDeviceContext;
	pDevice->GetImmediateContext(&pd3dDeviceContext);
	ImGui_ImplDX11_Init(manager->GetHWND(), pDevice, pd3dDeviceContext);
	
	ImGuiStyle& style = ImGui::GetStyle();
	style.WindowRounding = 8.0f;
	style.ScrollbarRounding = 8.0f;
	style.FrameRounding = 8.0f;
	//style.IndentSpacing = 20;
	int mainColor[3] = { 110, 110, 110 }; // previous green one { 50, 110, 30 }
	style.Colors[ImGuiCol_TitleBg]              = ImColor(mainColor[0], mainColor[1], mainColor[2], 62);
	style.Colors[ImGuiCol_TitleBgCollapsed]     = ImColor(mainColor[0], mainColor[1], mainColor[2], 52);
	style.Colors[ImGuiCol_TitleBgActive]        = ImColor(mainColor[0], mainColor[1], mainColor[2], 87);
	style.Colors[ImGuiCol_Header]               = ImColor(mainColor[0], mainColor[1], mainColor[2], 52);
	style.Colors[ImGuiCol_HeaderHovered]        = ImColor(mainColor[0], mainColor[1], mainColor[2], 92);
	style.Colors[ImGuiCol_HeaderActive]         = ImColor(mainColor[0], mainColor[1], mainColor[2], 72);
	style.Colors[ImGuiCol_ScrollbarBg]          = ImColor(mainColor[0], mainColor[1], mainColor[2], 12);
	style.Colors[ImGuiCol_ScrollbarGrab]        = ImColor(mainColor[0], mainColor[1], mainColor[2], 52);
	style.Colors[ImGuiCol_ScrollbarGrabHovered] = ImColor(mainColor[0], mainColor[1], mainColor[2], 92);
	style.Colors[ImGuiCol_ScrollbarGrabActive]  = ImColor(mainColor[0], mainColor[1], mainColor[2], 72);
	style.Colors[ImGuiCol_Button]               = ImColor(40, 100, 80, 30);
	style.Colors[ImGuiCol_ButtonHovered]        = ImColor(40, 100, 80, 100);
	style.Colors[ImGuiCol_ButtonActive]         = ImColor(40, 100, 80, 70);
	style.Colors[ImGuiCol_PopupBg]				= ImColor(10, 23, 18, 230);
	style.Colors[ImGuiCol_TextSelectedBg]		= ImColor(10, 23, 18, 180);
	style.Colors[ImGuiCol_FrameBg]				= ImColor(70, 70, 70, 30);
	style.Colors[ImGuiCol_FrameBgHovered]		= ImColor(70, 70, 70, 70);
	style.Colors[ImGuiCol_FrameBgActive]		= ImColor(70, 70, 70, 50);
	style.Colors[ImGuiCol_ComboBg]				= ImColor(20, 20, 20, 252);

	return S_OK;
}

void CommonUIController::DeviceDestroyed()
{
	ImGui_ImplDX11_Shutdown();
}

extern LRESULT ImGui_ImplDX11_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

LRESULT CommonUIController::MsgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	PX_UNUSED(hWnd);
	PX_UNUSED(wParam);
	PX_UNUSED(lParam);

	ImGui_ImplDX11_WndProcHandler(hWnd, uMsg, wParam, lParam);

	if (uMsg == WM_KEYDOWN && !ImGui::GetIO().WantCaptureKeyboard)
	{
		int iKeyPressed = static_cast<int>(wParam);
		switch (iKeyPressed)
		{
			case 'B':
			{
				getRenderer().setWireframeMode(!getRenderer().getWireframeMode());
				break;
			}
			case 'I':
			{
				//getBlastController().debugRenderMode = (BlastFamily::DebugRenderMode)(((int)getBlastController().debugRenderMode + 1) % BlastFamily::DebugRenderMode::DEBUG_RENDER_MODES_COUNT);
				break;
			}
			case VK_F5:
			{
				getRenderer().reloadShaders();
				break;
			}
			default:
				break;
		}
	}

	if (ImGui::GetIO().WantCaptureMouse)
		return 0;

	return 1;
}

void CommonUIController::Animate(double fElapsedTimeSeconds)
{
	m_dt = (float)fElapsedTimeSeconds;
}

void CommonUIController::Render(ID3D11Device*, ID3D11DeviceContext*, ID3D11RenderTargetView*, ID3D11DepthStencilView*)
{
	ImGui_ImplDX11_NewFrame();
	drawUI();
	ImGui::Render();
}

void CommonUIController::addDelayedCall(const char* title, const char* message, std::function<void()> func, float delay)
{
	DelayedCall call = { func, title, message, delay, delay };
	m_delayedCalls.emplace(call);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//													IMGUI UI
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


void CommonUIController::drawUI()
{
	const float padding = 8.0f;
	ImGui::SetNextWindowPos(ImVec2(padding, padding), ImGuiSetCond_Once/*ImGuiSetCond_FirstUseEver*/);
	ImGui::SetNextWindowSize(ImVec2(420, getRenderer().getScreenHeight() - 2 * padding), ImGuiSetCond_Once/*ImGuiSetCond_FirstUseEver*/);
	ImGui::SetNextWindowCollapsed(false, ImGuiSetCond_Once);
	ImGui::Begin("New Shiny UI", 0, ImGuiWindowFlags_NoTitleBar);
	{
		ImGui::PushItemWidth(ImGui::GetWindowSize().x * 0.5f);

		///////////////////////////////////////////////////////////////////////////////////////////
		// Scene
		///////////////////////////////////////////////////////////////////////////////////////////
		if (ImGui::CollapsingHeader("Scene"))
		{
			getSceneController().drawUI();
		}


		///////////////////////////////////////////////////////////////////////////////////////////
		// Stats
		///////////////////////////////////////////////////////////////////////////////////////////
		if (ImGui::CollapsingHeader("Stats"))
		{
			// FPS
			double averageTime = GetDeviceManager()->GetAverageFrameTime();
			float fps = (averageTime > 0) ? 1.0 / averageTime : 0.0;
			float frameMs = 1000.0f / fps;
			ImGui::Text("Frame Time %.3f ms (%.1f FPS)", frameMs, fps);

			static PlotLinesInstance<> fpsPlot;
			fpsPlot.plot("FPS", frameMs, "ms/frame", 0.0f, 100.0f);

			// Render stats
			ImGui::PushStyleColor(ImGuiCol_Text, ImColor(0xFF, 0x3B, 0xD8, 0xFF));
			ImGui::Text("Draw Calls (Opaque/Transparent): %d/%d", getRenderer().getVisibleOpaqueRenderablesCount(), getRenderer().getVisibleTransparentRenderablesCount());
			ImGui::PopStyleColor();

#if NV_PROFILE
			// Sample Profiler
			static bool s_showProfilerWindow = false;
			if (ImGui::Button("Code Profiler"))
			{
				s_showProfilerWindow = !s_showProfilerWindow;
			}
			if (s_showProfilerWindow)
			{
				drawCodeProfiler(&s_showProfilerWindow);
			}
#endif
		}

		///////////////////////////////////////////////////////////////////////////////////////////
		// Application
		///////////////////////////////////////////////////////////////////////////////////////////
		if (ImGui::CollapsingHeader("Application"))
		{
			// Reload Shaders
			if (ImGui::Button("Reload Shaders (F5)"))
			{
				getRenderer().reloadShaders();
			}

			/*// ImGui Test Window (just in case)
			static bool s_showTestWindow = false;
			if (ImGui::Button("ImGui Test Window"))
			{
				s_showTestWindow = !s_showTestWindow;
			}
			if (s_showTestWindow)
			{
				ImGui::ShowTestWindow();
			}*/
		}


		///////////////////////////////////////////////////////////////////////////////////////////
		// Debug Render
		///////////////////////////////////////////////////////////////////////////////////////////
		if (ImGui::CollapsingHeader("Debug Render"))
		{
			// WireFrame
			bool wireFrameEnabled = getRenderer().getWireframeMode();
			if (ImGui::Checkbox("WireFrame (B)", &wireFrameEnabled))
			{
				getRenderer().setWireframeMode(wireFrameEnabled);
			}

			// - - - - - - - -
			ImGui::Spacing();
		}


		///////////////////////////////////////////////////////////////////////////////////////////
		// Renderer
		///////////////////////////////////////////////////////////////////////////////////////////
		if (ImGui::CollapsingHeader("Renderer"))
		{
			getRenderer().drawUI();
		}


		///////////////////////////////////////////////////////////////////////////////////////////
		// Hints
		///////////////////////////////////////////////////////////////////////////////////////////
		if (ImGui::CollapsingHeader("Hints / Help"))
		{
			ImGui::BulletText("Rotate camera - RMB");
			ImGui::BulletText("Move camera - WASDQE(SHIFT)");
			//ImGui::BulletText("Play/Pause - P");
			ImGui::BulletText("Reload shaders - F5");
			ImGui::BulletText("Wireframe - O");
			//ImGui::BulletText("Restart - R");
		}

		ImGui::PopItemWidth();
	}
	ImGui::End();

	///////////////////////////////////////////////////////////////////////////////////////////
	// Mode Text
	///////////////////////////////////////////////////////////////////////////////////////////
	/*{
		ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4());
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0,0));

		const char* text = "CENTERED TEXT, REMOVE ME";
		ImVec2 size = ImGui::CalcTextSize(text);
		ImGui::SetNextWindowPos(ImVec2((getRenderer().getScreenWidth() - size.x) / 2, 0));
		ImGui::Begin("Mode Text", 0, ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar);
		ImGui::Text(text);
		ImGui::End();

		ImGui::PopStyleVar();
		ImGui::PopStyleColor();
	}*/

	///////////////////////////////////////////////////////////////////////////////////////////
	// FPS
	///////////////////////////////////////////////////////////////////////////////////////////
	{
		ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4());
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));

		double averageTime = GetDeviceManager()->GetAverageFrameTime();
		float fps = (averageTime > 0) ? 1.0 / averageTime : 0.0;
		static char buf[32];
		std::sprintf(buf, "%.1f FPS", fps);
		ImVec2 size = ImGui::CalcTextSize(buf);

		size.x += 20.0;
		ImGui::SetNextWindowSize(size);
		ImGui::SetNextWindowPos(ImVec2(getRenderer().getScreenWidth() - size.x, 0));
		ImGui::Begin("FPS", 0, ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar);
		ImGui::Text(buf);
		ImGui::End();

		ImGui::PopStyleVar();
		ImGui::PopStyleColor();
	}

	///////////////////////////////////////////////////////////////////////////////////////////
	// Loading overlay
	///////////////////////////////////////////////////////////////////////////////////////////
	if (!m_delayedCalls.empty())
	{
		DelayedCall& call = m_delayedCalls.front();
		if (call.delay > 0)
		{
			const int height = 50;
			const char* message = call.message;
			const float alpha =  PxClamp(lerp(0.0f, 1.0f, (call.delayTotal - call.delay) * 10.0f), 0.0f, 1.0f);

			ImGui::PushStyleColor(ImGuiCol_WindowBg, ImColor(0, 0, 0, 200));
			ImGui::PushStyleVar(ImGuiStyleVar_Alpha, alpha);
			ImGui::SetNextWindowPosCenter();
			ImVec2 size = ImGui::CalcTextSize(message);
			int width = std::max<float>(200, size.x) + 50;
			ImGui::SetNextWindowSize(ImVec2(width, height));
			ImGui::Begin(call.title, 0, ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar);
			ImGui::SetCursorPos(ImVec2((width - size.x) * 0.5f, (height - size.y) * 0.5f));
			ImGui::Text(message);
			ImGui::End();
			ImGui::PopStyleVar();
			ImGui::PopStyleColor();

			call.delay -= PxClamp(m_dt, 0.0f, 0.1f);
		}
		else
		{
			call.func();
			m_delayedCalls.pop();
		}
	}
}


void CommonUIController::drawCodeProfiler(bool* open)
{
	ImGuiWindowFlags window_flags = 0;
	const float padding = 8.0f;
	const float width = 550;
	const float height = 580;
	ImGui::SetNextWindowPos(ImVec2(getRenderer().getScreenWidth() - width - padding, padding), ImGuiSetCond_Once/*ImGuiSetCond_FirstUseEver*/);
	ImGui::SetNextWindowSize(ImVec2(width, height), ImGuiSetCond_Once);
	if (!ImGui::Begin("Code Profiler", open, window_flags))
	{
		// Early out if the window is collapsed, as an optimization.
		ImGui::End();
		return;
	}

	///////////////////////////////////////////////////////////////////////////////////////////
	// Control/Main Bar
	///////////////////////////////////////////////////////////////////////////////////////////
	{
		if (ImGui::Button("Reset"))
		{
			PROFILER_INIT();
		}
		ImGui::SameLine();
		if (ImGui::Button("Dump To File (profile.txt)"))
		{
			SampleProfilerDumpToFile("profile.txt");
		}
		ImGui::SameLine();
		ImGui::Text("Profiler overhead: %2.3f ms", SampleProfilerGetOverhead().count() * 0.001f);
	}

	///////////////////////////////////////////////////////////////////////////////////////////
	// Legend
	///////////////////////////////////////////////////////////////////////////////////////////
	{
		ImGui::Spacing(); ImGui::Separator(); ImGui::Spacing();
		ImGui::Text("Legend: name | calls | time | max time");
		ImGui::Spacing(); ImGui::Separator(); ImGui::Spacing();
	}

	///////////////////////////////////////////////////////////////////////////////////////////
	// Stats Tree
	///////////////////////////////////////////////////////////////////////////////////////////
	ImGui::SetNextTreeNodeOpen(true, ImGuiSetCond_Once);
	float plotMS = 0.0f;
	float plotMaxMS = 0.0f;
	const char* plotName = nullptr;
	if (ImGui::TreeNode("Root"))
	{
		auto treeIt = SampleProfilerCreateTreeIterator();
		if (treeIt)
		{
			uint32_t depth = 1;
			uint32_t openeDepth = 1;
			while (!treeIt->isDone())
			{
				const auto data = treeIt->data();

				while (data->depth < depth)
				{
					ImGui::TreePop();
					depth--;
				}

				const uint32_t maxLen = 30;
				auto hash = data->hash;
				static uint64_t selectedNodeHash = 0;
				if (selectedNodeHash == hash)
				{
					plotMS = data->time.count() * 0.001f;
					plotMaxMS = data->maxTime.count() * 0.001f;
					plotName = data->name;
				}
				if (ImGui::TreeNodeEx(data->name, data->hasChilds ? 0 : ImGuiTreeNodeFlags_Leaf, "%-*.*s | %d | %2.3f ms | %2.3f ms",
					maxLen, maxLen, data->name, data->calls, data->time.count() * 0.001f, data->maxTime.count() * 0.001f))
				{
					depth++;
					treeIt->next();
				}
				else
				{
					treeIt->next();
					while (!treeIt->isDone() && treeIt->data()->depth > depth)
						treeIt->next();
				}

				if (ImGui::IsItemClicked())
				{
					selectedNodeHash = hash;
				}
			}

			while (depth > 0)
			{
				ImGui::TreePop();
				depth--;
			}

			treeIt->release();
		}
		else
		{
			ImGui::Text("Profiler Is Broken. Begin/End Mismatch.");
		}
	}

	///////////////////////////////////////////////////////////////////////////////////////////
	// Selected Item Plot
	///////////////////////////////////////////////////////////////////////////////////////////
	{
		ImGui::Spacing(); ImGui::Separator(); ImGui::Spacing();
		if (plotName)
		{
			static PlotLinesInstance<> selectedNodePlot;
			selectedNodePlot.plot("", plotMS, plotName, 0.0f, plotMaxMS);
		}
		else
		{
			ImGui::Text("Select item to plot.");
		}
	}

	ImGui::End();
}