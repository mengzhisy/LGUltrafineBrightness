//
// Created by M.z on 2022-06-30.
//

#pragma once

#include <Windows.h>
#include <initguid.h>
#include <propkeydef.h>
#include <iostream>
#include <propvarutil.h>
#include <functiondiscoverykeys.h>
#include <SensorsApi.h>
#include <sensors.h>
#include <any>
#include <stdexcept>
#include <string>
#include <vector>
#include <codecvt>

using namespace std;

template<class T>
void SafeRelease(T *&ppT) {
    if (ppT) {
        ppT->Release();
        ppT = NULL;
    }
}

void SafeRelease(BSTR &bstr) {
    if (bstr) {
        SysFreeString(bstr);
        bstr = nullptr;
    }
}

// convert a BSTR to a std::string.
std::string &BstrToStdString(const BSTR bstr, std::string &dst, int cp = CP_UTF8) {
    if (!bstr) {
        // define NULL functionality. I just clear the target.
        dst.clear();
        return dst;
    }

    // request content length in single-chars through a terminating
    //  nullchar in the BSTR. note: BSTR's support imbedded nullchars,
    //  so this will only convert through the first nullchar.
    int res = WideCharToMultiByte(cp, 0, bstr, -1, NULL, 0, NULL, NULL);
    if (res > 0) {
        dst.resize(res);
        WideCharToMultiByte(cp, 0, bstr, -1, &dst[0], res, NULL, NULL);
    } else {    // no content. clear target
        dst.clear();
    }
    return dst;
}


// conversion with temp.
std::string BstrToStdString(BSTR bstr, int cp = CP_UTF8) {
    std::string str;
    BstrToStdString(bstr, str, cp);
    return str;
}

std::string to_string(std::wstring& input) {
    std::wstring_convert<std::codecvt_utf8<wchar_t>> convert;
    return convert.to_bytes(input);
}

// https://docs.microsoft.com/en-us/windows/win32/api/propidlbase/ns-propidlbase-propvariant
std::any ToStdAny(PROPVARIANT &val) {
    switch (val.vt) {
        case VT_LPWSTR: {
            auto s = std::wstring(val.pwszVal);
            return to_string(s);
        }
        case VT_UI4: {
            return (uint32_t) val.ulVal;
        }
    }

    throw std::runtime_error("Unkown PROPVARIANT value type " + std::to_string(val.vt));
}

std::any Sensor_GetProperty(ISensor *sensor, REFPROPERTYKEY key) {
    PROPVARIANT result;
    if (FAILED(sensor->GetProperty(key, &result))) {
        throw std::runtime_error("sensor->GetProperty failed.");
    }
    return ToStdAny(result);
}

HRESULT Sensor_SetCurrentReportInterval(ISensor *pSensor, ULONG ulNewInterval) {

    HRESULT hr = S_OK;

    IPortableDeviceValues *pPropsToSet = NULL; // Input
    IPortableDeviceValues *pPropsReturn = NULL; // Output

    // Create the input object.
    hr = CoCreateInstance(__uuidof(PortableDeviceValues),
                          NULL,
                          CLSCTX_INPROC_SERVER,
                          IID_PPV_ARGS(&pPropsToSet));

    if (SUCCEEDED(hr)) {
        // Add the current report interval property.
        hr = pPropsToSet->SetUnsignedIntegerValue(SENSOR_PROPERTY_CURRENT_REPORT_INTERVAL, ulNewInterval);
    }

    if (SUCCEEDED(hr)) {
        // Only setting a single property, here.
        hr = pSensor->SetProperties(pPropsToSet, &pPropsReturn);
    }

    // Test for failure.
    if (hr == S_FALSE) {
        HRESULT hrError = S_OK;

        // Check results for failure.
        hr = pPropsReturn->GetErrorValue(SENSOR_PROPERTY_CURRENT_REPORT_INTERVAL, &hrError);

        if (SUCCEEDED(hr)) {
            // Print an error message.
            wprintf_s(L"\nSetting current report interval failed with error 0x%X\n", hrError);

            // Return the error code.
            hr = hrError;
        }
    } else if (hr == E_ACCESSDENIED) {
        // No permission. Take appropriate action.
    }

    SafeRelease(pPropsToSet);
    SafeRelease(pPropsReturn);

    return hr;
}

//void Sensor_SetReportInterval(ISensor *sensor, uint64_t interval) {
//    PROPVARIANT arg;
//    if (FAILED(sensor->Set)) {
//        throw std::runtime_error("sensor->GetProperty failed.");
//    }
//    return;
//}



