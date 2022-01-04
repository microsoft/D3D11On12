# D3D11On12

D3D11On12 is a mapping layer, which maps graphics commands from D3D11 to D3D12. D3D11On12 is **not** an implementation of the D3D11 **API**, but is instead an implementation of the D3D11 usermode **DDI** (device driver interface). That means it is not a binary named d3d11.dll, but is named d3d11on12.dll.

When an application creates a D3D11 device, they may choose for it to be a D3D11On12 device, rather than a native D3D11 device (see `D3D11On12CreateDevice`). When this happens, d3d11on12.dll is loaded by the D3D11 runtime and initialized. When the application calls rendering commands, D3D11 will validate those commands, optionally including the D3D11 debug layer for further validation, and then will convert those commands to the D3D11 DDI and send it to D3D11On12, just like any D3D11 driver. D3D11On12 will take these commands and convert them into D3D12 API calls, which are further validated by the D3D12 runtime, optionally including the D3D12 debug layer, which are then converted to the D3D12 DDI and sent to the D3D12 driver.

Note that D3D11On12 is an *enlightened* D3D11 driver, and there are several places where it receives additional information compared to a traditional D3D11 driver, either to enable it to provide API-level information to D3D12 rather than driver-level information (as is the case for shaders), or to enable interop scenarios. When a D3D11 device is created with D3D11On12, the device will expose an `ID3D11On12Device` interface which enables applications to submit work to both the D3D11 API and the D3D12 API with lightweight sharing and synchronization.

For more details about D3D11On12, see:
* [D3D11On12 conceptual documentation](https://docs.microsoft.com/en-us/windows/win32/direct3d12/direct3d-11-on-12)
* [D3D11On12 reference documentation](https://docs.microsoft.com/en-us/windows/win32/api/d3d11on12/)
* D3D11On12 blog posts: [First](https://devblogs.microsoft.com/directx/direct3d-11-on-12-updates/) and [second](https://devblogs.microsoft.com/directx/coming-to-directx-12-d3d9on12-and-d3d11on12-resource-interop-apis/)

Make sure that you visit the [DirectX Landing Page](https://devblogs.microsoft.com/directx/landing-page/) for more resources for DirectX developers.

## How does it work?

The primary entrypoint to D3D11On12 is a custom version of the normal D3D11 driver [OpenAdapter10_2](https://docs.microsoft.com/en-us/windows-hardware/drivers/display/initializing-communication-with-the-direct3d-version-11-ddi) entrypoint, named `OpenAdapter_D3D11On12`, where D3D11 provides additional information to the mapping layer. In response to this, like a normal driver, D3D11On12 returns an adapter object, which exposes DDIs to create a device. The device is created like normal, but in addition to the normal DDI tables, it also exposes an `ID3D11On12DDIDevice` interface.

The device object internally uses an instance of the D3D12TranslationLayer immediate context, wrapped in a D3D12TranslationLayer batched context, to record commands. Similarly, most D3D11On12 objects are backed by an implementation from the D3D12TranslationLayer library. The code in this repository is largely a simple adaptor from the D3D11 DDI to the D3D12TranslationLayer library, where the real heavy lifting of converting to the D3D12 domain is done.

## Building

In order to build D3D11On12, the [WDK](https://docs.microsoft.com/en-us/windows-hardware/drivers/download-the-wdk) (Windows Driver Kit) must be installed, in order to provide `d3d10umddi.h` to D3D11On12, and in order to generate the D3D12TranslationLayer_WDK project, which hosts some code required to parse DXBC shaders and containers. The D3D12TranslationLayer and its subprojects, D3D12TranslationLayer_WDK and DXBCParser, will be fetched from GitHub when building with CMake if D3D12TranslationLayer_WDK isn't already included, such as by a parent CMakeLists.txt that has already entered that project. Assuming there was a top level `CMakeLists.txt` in a directory that included both D3D11On12 and D3D12TranslationLayer, you could achieve that like this:

```CMake
cmake_minimum_required(VERSION 3.14)
include(FetchContent)

FetchContent_Declare(
    d3d12translationlayer
    SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/D3D12TranslationLayer
)
FetchContent_MakeAvailable(d3d12translationlayer)

add_subdirectory(D3D11On12)
```

At the time of publishing, the D3D11On12 and D3D12TranslationLayer require **insider** versions of the SDK and WDK. Those can be found [here](https://www.microsoft.com/en-us/software-download/windowsinsiderpreviewWDK).

D3D11On12 requires C++17, and only supports building with MSVC at the moment.

### Troubleshoot building
A somewhat common issue when trying to build the mapping layers is that there are multiple Windows SDKs installed, but only a subset of them has a corresponding WDK also installed. You can specify which version of the SDK to use when generating the solution by adding the following to your cmake command: `-DCMAKE_SYSTEM_VERSION=<SDK_VERSION>`.

## Why open source?

The D3D11On12 mapping layer is included as an operating system component of Windows 10. Over the years and Windows 10 releases, it has grown in functionality, to the point where it is a complete and relatively performant implementation of a D3D11 driver. We are choosing to release the source to this component for two primary reasons:
1. To enable the community to contribute bugfixes and further performance improvements, which will improve the stability and performance of Windows 10. See [CONTRIBUTING](contributing.md).
2. To serve as an example of how to use the D3D12TranslationLayer library.

### What can you do with this?

There are minor differences between binaries built out of this repository and the versions that are included in the OS. To that end, shipping applications should not attempt to override the OS version of D3D11On12 with versions that they have built. We will not guarantee that newer versions of Windows will continue to support older versions of D3D11On12, since it is an OS component which may be revised together with D3D11. However, developers are welcome to override the Windows version of D3D11On12 for *local testing and experimentation*.

### Compatibility

When possible, we will attempt to maintain compatibility between D3D11 and D3D11On12. One should expect that the tip of D3D11On12's `master` branch should work nicely with the latest release of Windows 10. Support for configurations other than that are not guaranteed to work.

## Data Collection

The software may collect information about you and your use of the software and send it to Microsoft. Microsoft may use this information to provide services and improve our products and services. You may turn off the telemetry as described in the repository. There are also some features in the software that may enable you and Microsoft to collect data from users of your applications. If you use these features, you must comply with applicable law, including providing appropriate notices to users of your applications together with a copy of Microsoft's privacy statement. Our privacy statement is located at https://go.microsoft.com/fwlink/?LinkID=824704. You can learn more about data collection and use in the help documentation and our privacy statement. Your use of the software operates as your consent to these practices.

Note however that no data collection is performed when using your private builds.
