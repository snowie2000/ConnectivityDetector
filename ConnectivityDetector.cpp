// ConnectivityDetector.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <windows.h>
#if defined(_DEBUG)
#    pragma comment(lib, "comsuppwd.lib")
#else
#    pragma comment(lib, "comsuppw.lib")
#endif

#include <comdef.h>
#include <comip.h>
#include <netlistmgr.h>

_COM_SMARTPTR_TYPEDEF(INetworkListManager, IID_INetworkListManager);

struct EventSink : INetworkListManagerEvents
{
    HRESULT QueryInterface(REFIID riid, LPVOID* ppvObj)
    {
        if (!ppvObj)
        {
            return E_POINTER;
        }
        *ppvObj = nullptr;
        if (riid == IID_IUnknown || riid == IID_INetworkListManagerEvents)
        {
            AddRef();
            *ppvObj = reinterpret_cast<void*>(this);
            return S_OK;
        }
        return E_NOINTERFACE;
    }

    ULONG AddRef() { return 1; }
    ULONG Release() { return 1; }

    HRESULT ConnectivityChanged(NLM_CONNECTIVITY newConnectivity)
    {
        if ((newConnectivity & (NLM_CONNECTIVITY_IPV4_INTERNET | NLM_CONNECTIVITY_IPV6_INTERNET)) != 0)
        {
            printf("Internet connection available\n");
        }
        else
        {
            printf("Internet connection not available\n");
        }
        return S_OK;
    }
    HRESULT NetworkConnectionPropertyChanged(GUID connectionId, NLM_CONNECTION_PROPERTY_CHANGE) { return S_OK; }
};

int main()
{
    // Initialize COM
    auto hr{ ::CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED) };

    // Instantiate INetworkListManager object
    INetworkListManagerPtr spManager{ nullptr };
    if SUCCEEDED(hr)
    {
        hr = spManager.CreateInstance(CLSID_NetworkListManager, nullptr, CLSCTX_ALL);
    }

    // Query for connection point container
    IConnectionPointContainerPtr spConnectionPoints{ nullptr };
    if SUCCEEDED(hr)
    {
        hr = spManager.QueryInterface(IID_PPV_ARGS(&spConnectionPoints));
    }

    // Find connection point for the interesting event
    IConnectionPointPtr spConnectionPoint{ __nullptr };
    if SUCCEEDED(hr)
    {
        hr = spConnectionPoints->FindConnectionPoint(IID_INetworkListManagerEvents, &spConnectionPoint);
    }

    // Construct event sink
    EventSink sink{};
    IUnknownPtr spSink{ nullptr };
    if (SUCCEEDED(hr))
    {
        hr = sink.QueryInterface(IID_IUnknown, reinterpret_cast<void**>(&spSink));
    }

    // And wire it up to the connection point
    DWORD cookie{ 0 };
    if SUCCEEDED(hr)
    {
        hr = spConnectionPoint->Advise(spSink, &cookie);
    }

    printf("I'm ready.\r\n");

    // At this point everything is set up to receive notifications
    MSG msg{};
    while (::GetMessageW(&msg, nullptr, 0, 0) > 0)
    {
        ::DispatchMessageW(&msg);
    }

    // Cleanup
    if (SUCCEEDED(hr))
    {
        hr = spConnectionPoint->Unadvise(cookie);
    }

    // Don't uninitialize COM since we have smart pointers that
    // get cleaned up only after leaving this scope.
}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
