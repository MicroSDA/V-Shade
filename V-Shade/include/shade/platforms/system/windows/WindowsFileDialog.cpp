#include "shade_pch.h"
#include "WindowsFileDialog.h"

//#include <commdlg.h>
#include <glfw/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>
#include "shade/core/application/Application.h"

#include <winnt.h>
#include <minwinbase.h>
#include <ShlObj.h>
#include <atlbase.h>

std::filesystem::path shade::WindowsFileDialog::OpenFile(const char* filter)
{
	OPENFILENAMEA ofn;
	CHAR szFile[260] = { 0 };
	CHAR currentDir[256] = { 0 };
	ZeroMemory(&ofn, sizeof(OPENFILENAME));
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = glfwGetWin32Window((GLFWwindow*)Application::GetWindow()->GetNativeWindow());
	ofn.lpstrFile = szFile;
	ofn.nMaxFile = sizeof(szFile);
	if (GetCurrentDirectoryA(256, currentDir))
		ofn.lpstrInitialDir = currentDir;
	ofn.lpstrFilter = filter;
	ofn.nFilterIndex = 1;
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;

	if (GetOpenFileNameA(&ofn) == TRUE)
	{
		auto workDir = std::filesystem::current_path();
		auto path = std::string(ofn.lpstrFile);
		auto pos = path.find(workDir.string());
		if (pos != std::string::npos)
		{
			path.erase(pos, workDir.string().size());
			std::replace(path.begin(), path.end(), '\\', '/');
			
		}
		//return "." + path;
		return path;
	}
	return std::filesystem::path();
}

std::filesystem::path shade::WindowsFileDialog::SaveFile(const char* filter)
{
	OPENFILENAMEA ofn;
	CHAR szFile[260] = { 0 };
	CHAR currentDir[256] = { 0 };
	ZeroMemory(&ofn, sizeof(OPENFILENAME));
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = glfwGetWin32Window((GLFWwindow*)Application::GetWindow()->GetNativeWindow());
	ofn.lpstrFile = szFile;
	ofn.nMaxFile = sizeof(szFile);
	if (GetCurrentDirectoryA(256, currentDir))
		ofn.lpstrInitialDir = currentDir;
	ofn.lpstrFilter = filter;
	ofn.nFilterIndex = 1;
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT | OFN_NOCHANGEDIR;

	// Sets the default extension by extracting it from the filter
	ofn.lpstrDefExt = strchr(filter, '\0') + 1;

	if (GetSaveFileNameA(&ofn) == TRUE)
	{
		auto workDir = std::filesystem::current_path();
		auto path = std::string(ofn.lpstrFile);
		auto pos = path.find(workDir.string());
		if (pos != std::string::npos)
		{
			path.erase(pos, workDir.string().size());
			std::replace(path.begin(), path.end(), '\\', '/');

		}
		return "." + path;

		return ofn.lpstrFile;
	}
	return std::filesystem::path();
}

std::filesystem::path shade::WindowsFileDialog::SelectFolder(const char* filter)
{
	// Create an instance of IFileOpenDialog.
	CComPtr<IFileOpenDialog> pFolderDlg;
	pFolderDlg.CoCreateInstance(CLSID_FileOpenDialog);

	// Set options for a filesystem folder picker dialog.
	FILEOPENDIALOGOPTIONS opt{};
	pFolderDlg->GetOptions(&opt);
	pFolderDlg->SetOptions(opt | FOS_PICKFOLDERS | FOS_PATHMUSTEXIST | FOS_FORCEFILESYSTEM);

	// Show the dialog modally.
	if (SUCCEEDED(pFolderDlg->Show(nullptr)))
	{
		// Get the path of the selected folder and output it to the console.
		CComPtr<IShellItem> pSelectedItem;
		pFolderDlg->GetResult(&pSelectedItem);

		CComHeapPtr<wchar_t> pPath;
		pSelectedItem->GetDisplayName(SIGDN_FILESYSPATH, &pPath);

		std::wstring wstring(pPath.m_pData);
		std::string path(wstring.begin(), wstring.end());

		auto workDir = std::filesystem::current_path();
		auto pos = path.find(workDir.string());

		if (pos != std::string::npos)
		{
			path.erase(pos, workDir.string().size());
			std::replace(path.begin(), path.end(), '\\', '/');
			path.erase(0, 1); // first '/'
		}
		return "./" + path + "/";
	}
	else
	{
		return std::filesystem::path();
	}
}
