#include "pch.h"

using namespace winrt;
using namespace Windows::UI;
using namespace Windows::UI::Composition;
using namespace Windows::UI::Xaml::Hosting;
using namespace Windows::Foundation;
using namespace Windows::Foundation::Numerics;

BOOL CALLBACK UpdateChildSizeProc(HWND childHwnd, LPARAM lParam) {
  RECT rcClient;
  GetClientRect((HWND)lParam, &rcClient);
  SetWindowPos(childHwnd, 0, 0, 0, rcClient.right - rcClient.left,
               rcClient.bottom - rcClient.top, SWP_SHOWWINDOW);
  return FALSE;
}

LRESULT CALLBACK WindowProc(HWND hWnd, UINT messageCode, WPARAM wParam,
                            LPARAM lParam) {
  switch (messageCode) {
  case WM_PAINT:
    break;
  case WM_DESTROY:
    PostQuitMessage(0);
    break;
  case WM_SIZE:
    // Pass main window dimension to (XamlIsland) child windows
    EnumChildWindows(hWnd, UpdateChildSizeProc, (LPARAM)hWnd);
    break;
  default:
    return DefWindowProc(hWnd, messageCode, wParam, lParam);
    break;
  }
  return 0;
}

struct NativeWindow {
  NativeWindow(HINSTANCE hInstance) noexcept {
    // The main window class name.
    const wchar_t szWindowClass[] = L"WinDesk";
    WNDCLASSEX windowClass = {};
    windowClass.cbSize = sizeof(WNDCLASSEX);
    windowClass.lpfnWndProc = WindowProc;
    windowClass.hInstance = hInstance;
    windowClass.lpszClassName = szWindowClass;
    windowClass.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    windowClass.hIconSm = LoadIcon(windowClass.hInstance, IDI_APPLICATION);

    if (RegisterClassEx(&windowClass) == NULL) {
      MessageBox(NULL, L"Windows registration failed!", L"Error", NULL);
      return;
    }

    auto width = 720;
    auto height = 1280;
    hWnd_ =
        CreateWindow(szWindowClass, L"Win32 Desktop App",
                     WS_OVERLAPPEDWINDOW | WS_VISIBLE, CW_USEDEFAULT,
                     CW_USEDEFAULT, width, height, NULL, NULL, hInstance, NULL);
    if (hWnd_ == NULL) {
      MessageBox(NULL, L"Call to CreateWindow failed!", L"Error", NULL);
      return;
    }

    // XAML Island section
    // Initialize the Xaml Framework's corewindow for current thread
    // windowsXamlManager_ = WindowsXamlManager::InitializeForCurrentThread();

    // This DesktopWindowXamlSource is the object that enables a non-UWP
    // desktop application to host UWP controls in any UI element that is
    // associated with a window handle (HWND).
    // desktopSource_ = DesktopWindowXamlSource{};
    // Get handle to corewindow
    // auto interop = desktopSource_.as<IDesktopWindowXamlSourceNative>();
    // Parent the DesktopWindowXamlSource object to current window
    // check_hresult(interop->AttachToWindow(hWnd_));

    // Get the new child window's hwnd
    // interop->get_WindowHandle(&hWndXamlIsland_);
    // Update the xaml island window size becuase initially is 0,0
    // SetWindowPos(hWndXamlIsland_, 0, 0, 0, width, height, SWP_SHOWWINDOW);

    // Creating the Xaml content
    // Windows::UI::Xaml::Controls::StackPanel xamlContainer;
    // xamlContainer.Background(Windows::UI::Xaml::Media::SolidColorBrush{
        // Windows::UI::Colors::LightGray()});

    // Windows::UI::Xaml::Controls::TextBlock tb;
    // tb.Text(L"Hello World from Xaml Islands!");
    // tb.VerticalAlignment(Windows::UI::Xaml::VerticalAlignment::Center);
    // tb.HorizontalAlignment(Windows::UI::Xaml::HorizontalAlignment::Center);
    // tb.FontSize(48);

    // xamlContainer.Children().Append(tb);
    // xamlContainer.UpdateLayout();
    // desktopSource_.Content(xamlContainer);
    // End XAML Island section

    ShowWindow(hWnd_, SW_SHOW);
    UpdateWindow(hWnd_);
  }

  void start() {
    // Message loop:
    MSG msg = {};
    while (GetMessage(&msg, NULL, 0, 0)) {
      TranslateMessage(&msg);
      DispatchMessage(&msg);
    }
    // Close window
    DestroyWindow(hWnd_);
  }
  NativeWindow(const NativeWindow &) = delete;
  NativeWindow &operator=(const NativeWindow &) = delete;
  ~NativeWindow() = default;

private:
  HWND hWnd_ = nullptr;
  // This Hwnd will be the window handler for the Xaml Island: A child window
  // that contains Xaml.
  // HWND hWndXamlIsland_ = nullptr;
  // WindowsXamlManager windowsXamlManager_ = nullptr;
  // DesktopWindowXamlSource desktopSource_ = nullptr;
};

int CALLBACK WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPSTR lpCmdLine, _In_ int nCmdShow) {
  NativeWindow nativeWindow{hInstance};
  nativeWindow.start();
  return 0;
}
