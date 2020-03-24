#include "pch.h"
#include <winrt/Windows.Storage.FileProperties.h>
#include <winrt/Windows.Storage.Streams.h>
#include <winrt/Windows.Web.Http.Filters.h>
#include <winrt/Windows.Web.Http.Headers.h>
#include <winrt/Windows.Web.Http.h>
#include <future>
#include <sstream>
#include "unicode.h"
#include "promise_compat.h"

using namespace winrt;
using namespace Windows::UI;
using namespace Windows::UI::Composition;
using namespace Windows::UI::Xaml::Hosting;
using namespace Windows::Foundation;
using namespace Windows::Foundation::Numerics;

namespace winrt {
	using namespace winrt::Windows::Foundation;
	using namespace winrt::Windows::Storage;
}; // namespace winrt

winrt::fire_and_forget persistPreparedScriptAsync(
	const char* prepareTag // Optional tag. For e.g. eagerly evaluated vs lazy cache.
) {
	try {
		co_await winrt::resume_background();
		auto folder = winrt::ApplicationData::Current().LocalCacheFolder();
		auto fileName = winrt::to_hstring("url.bytecode");
		auto file = co_await folder.CreateFileAsync(fileName, winrt::CreationCollisionOption::ReplaceExisting);
	}
	catch (...) {
	}
}

void persistPreparedScript(
	const char* prepareTag // Optional tag. For e.g. eagerly evaluated vs lazy cache.
) noexcept {
	persistPreparedScriptAsync(prepareTag);
}

std::future<std::pair<std::string, bool>> DownloadFromAsync(const std::string& url) {
	winrt::Windows::Web::Http::Filters::HttpBaseProtocolFilter filter;
	filter.CacheControl().ReadBehavior(winrt::Windows::Web::Http::Filters::HttpCacheReadBehavior::NoCache);
	winrt::Windows::Web::Http::HttpClient httpClient(filter);
	winrt::Windows::Foundation::Uri uri(Microsoft::Common::Unicode::Utf8ToUtf16(url));

	co_await winrt::resume_background();

	winrt::Windows::Web::Http::HttpRequestMessage request(winrt::Windows::Web::Http::HttpMethod::Get(), uri);
	winrt::Windows::Web::Http::HttpResponseMessage response = co_await httpClient.SendRequestAsync(request);

	winrt::Windows::Storage::Streams::IBuffer buffer = co_await response.Content().ReadAsBufferAsync();
	auto reader = winrt::Windows::Storage::Streams::DataReader::FromBuffer(buffer);

	reader.UnicodeEncoding(winrt::Windows::Storage::Streams::UnicodeEncoding::Utf8);
	uint32_t len = reader.UnconsumedBufferLength();
	std::string result;
	if (len > 0 || response.IsSuccessStatusCode()) {
		std::vector<uint8_t> data(len);
		reader.ReadBytes(data);
		result = std::string(reinterpret_cast<char*>(data.data()), data.size());
	}
	else {
		std::ostringstream sstream;
		sstream << "HTTP Error " << static_cast<int>(response.StatusCode()) << " downloading " << url;
		result = sstream.str();
	}

	co_return std::make_pair(std::move(result), response.IsSuccessStatusCode());
}

void LaunchDevTools(const std::string& url) {
	DownloadFromAsync(url).get();
}

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
		windowsXamlManager_ = WindowsXamlManager::InitializeForCurrentThread();

		// This DesktopWindowXamlSource is the object that enables a non-UWP
		// desktop application to host UWP controls in any UI element that is
		// associated with a window handle (HWND).
		desktopSource_ = DesktopWindowXamlSource{};
		// Get handle to corewindow
		auto interop = desktopSource_.as<IDesktopWindowXamlSourceNative>();
		// Parent the DesktopWindowXamlSource object to current window
		check_hresult(interop->AttachToWindow(hWnd_));

		// Get the new child window's hwnd
		interop->get_WindowHandle(&hWndXamlIsland_);
		// Update the xaml island window size becuase initially is 0,0
		SetWindowPos(hWndXamlIsland_, 0, 0, 0, width, height, SWP_SHOWWINDOW);

		// Creating the Xaml content
		Windows::UI::Xaml::Controls::StackPanel xamlContainer;
		xamlContainer.Background(Windows::UI::Xaml::Media::SolidColorBrush{
			Windows::UI::Colors::LightGray() });

		Windows::UI::Xaml::Controls::TextBlock tb;
		tb.Text(L"Hello World from Xaml Islands!");
		tb.VerticalAlignment(Windows::UI::Xaml::VerticalAlignment::Center);
		tb.HorizontalAlignment(Windows::UI::Xaml::HorizontalAlignment::Center);
		tb.FontSize(48);

		xamlContainer.Children().Append(tb);
		xamlContainer.UpdateLayout();
		desktopSource_.Content(xamlContainer);
		// End XAML Island section

		ShowWindow(hWnd_, SW_SHOW);
		UpdateWindow(hWnd_);
	}

	void start() {
		persistPreparedScript(nullptr);

		// Message loop:
		MSG msg = {};
		while (GetMessage(&msg, NULL, 0, 0)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		// Close window
		DestroyWindow(hWnd_);
	}
	NativeWindow(const NativeWindow&) = delete;
	NativeWindow& operator=(const NativeWindow&) = delete;
	~NativeWindow() = default;

private:
	HWND hWnd_ = nullptr;
	// This Hwnd will be the window handler for the Xaml Island: A child window
	// that contains Xaml.
	HWND hWndXamlIsland_ = nullptr;
	WindowsXamlManager windowsXamlManager_ = nullptr;
	DesktopWindowXamlSource desktopSource_ = nullptr;
};

int CALLBACK WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance,
	_In_ LPSTR lpCmdLine, _In_ int nCmdShow) {
	NativeWindow nativeWindow{ hInstance };
	nativeWindow.start();
	return 0;
}
