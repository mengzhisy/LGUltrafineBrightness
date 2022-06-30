#include "helper.h"
#include "brightness.h"

class CMyEvents : public ISensorEvents {
    long m_cRef;
public:

    STDMETHODIMP QueryInterface(REFIID iid, void **ppv) override {
        if (ppv == NULL) {
            return E_POINTER;
        }
        if (iid == __uuidof(IUnknown)) {
            *ppv = static_cast<IUnknown *>(this);
        } else if (iid == __uuidof(ISensorEvents)) {
            *ppv = static_cast<ISensorEvents *>(this);
        } else {
            *ppv = NULL;
            return E_NOINTERFACE;
        }
        AddRef();
        return S_OK;
    }

    STDMETHODIMP_(ULONG) AddRef() override {
        return InterlockedIncrement(&m_cRef);
    }

    STDMETHODIMP_(ULONG) Release() {
        ULONG count = InterlockedDecrement(&m_cRef);
        if (count == 0) {
            delete this;
            return 0;
        }
        return count;
    }

    STDMETHODIMP OnEvent(
            ISensor *pSensor,
            REFGUID eventID,
            IPortableDeviceValues *pEventData) {
        HRESULT hr = S_OK;

        // Handle custom events here.

        return hr;
    }

    STDMETHODIMP OnDataUpdated(
            ISensor *pSensor,
            ISensorDataReport *pNewData) {
        HRESULT hr = S_OK;

        if (pSensor == NULL ||
            pNewData == NULL) {
            return E_POINTER;
        }

        // Declare and initialize the PROPVARIANT
        PROPVARIANT lightLevel;
        PropVariantInit(&lightLevel);

        // Get the sensor reading from the ISensorDataReport object
        hr = pNewData->GetSensorValue(
                SENSOR_DATA_TYPE_LIGHT_LEVEL_LUX,
                &lightLevel);

        if (SUCCEEDED(hr)) {
            if (lightLevel.vt == VT_R4) {
                // Extract the float value from the PROPVARIANT object
                float luxValue = lightLevel.fltVal;
                if (luxValue > 0) {
                    int brightness = int(luxValue / 2);
                    set_brightness(brightness);

                    // Normalize the light sensor data
                    double lightNormalized = ::log10(luxValue) / 5.0;

                    // Handle UI changes based on the normalized LUX data
                    // which ranges from 0.0 - 1.0 for a lux range of
                    // 0 lux to 100,000 lux.
                    printf("lux value=%.2f  brightness=%d\n", luxValue, brightness);
                }
            }
        }

        // Release the variant.
        PropVariantClear(&lightLevel);

        return hr;
    }

    STDMETHODIMP OnLeave(
            REFSENSOR_ID sensorID) {
        HRESULT hr = S_OK;

        // Peform any housekeeping tasks for the sensor that is leaving.
        // For example, if you have maintained a reference to the sensor,
        // release it now and set the pointer to NULL.

        return hr;
    }

    STDMETHODIMP OnStateChanged(
            ISensor *pSensor,
            SensorState state) {
        HRESULT hr = S_OK;

        if (NULL == pSensor) {
            return E_INVALIDARG;
        }


        if (state == SENSOR_STATE_READY) {
            wprintf_s(L"\nTime sensor is now ready.");
        } else if (state == SENSOR_STATE_ACCESS_DENIED) {
            wprintf_s(L"\nNo permission for the time sensor.\n");
            wprintf_s(L"Enable the sensor in the control panel.\n");
        }


        return hr;
    }
};

int main() {
    try {
        brightness_init();
        HRESULT hr = S_OK;

// Sensor interface pointers
        ISensorManager *pSensorManager = NULL;
        ISensorCollection *pSensorColl = NULL;
        ISensorDataReport *pReport = NULL;

        CoInitializeEx(NULL, COINIT_MULTITHREADED);

        hr = CoCreateInstance(CLSID_SensorManager,
                              nullptr,
                              CLSCTX_INPROC_SERVER,
                              IID_PPV_ARGS(&pSensorManager));

        if (FAILED(hr)) {
            std::exit(-10001);
        }

        // 获取光线传感器
        hr = pSensorManager->GetSensorsByType(SENSOR_TYPE_AMBIENT_LIGHT, &pSensorColl);
        if (FAILED(hr)) {
            std::exit(-10002);
        }

        ULONG sensorsCount = 0;

        // Verify that the collection contains
        // at least one sensor.
        hr = pSensorColl->GetCount(&sensorsCount);
        if (FAILED(hr)) {
            std::exit(-10003);
        }

        for (size_t i = 0; i < sensorsCount; i++) {
            ISensor *pSensor = nullptr;

            if (SUCCEEDED(hr)) {
                hr = pSensorColl->GetAt(i, &pSensor);
            }
            if (SUCCEEDED(hr)) {
                std::cout << "MANUFACTURER: "
                          << any_cast<string>(Sensor_GetProperty(pSensor, SENSOR_PROPERTY_MANUFACTURER)) << std::endl;
                std::cout << "MODEL: " << any_cast<string>(Sensor_GetProperty(pSensor, SENSOR_PROPERTY_MODEL))
                          << std::endl;
                std::cout << "DESCRIPTION: "
                          << any_cast<string>(Sensor_GetProperty(pSensor, SENSOR_PROPERTY_DESCRIPTION)) << std::endl;
                std::cout << "MIN_REPORT_INTERVAL: "
                          << any_cast<uint32_t>(Sensor_GetProperty(pSensor, SENSOR_PROPERTY_MIN_REPORT_INTERVAL))
                          << std::endl;
                std::cout << "FRIENDLY_NAME: "
                          << any_cast<string>(Sensor_GetProperty(pSensor, SENSOR_PROPERTY_FRIENDLY_NAME)) << std::endl;
            }

            if (SUCCEEDED(hr)) {
//                hr = Sensor_SetCurrentReportInterval(pSensor, 1000);
            }

            CMyEvents *pEventClass = NULL;
            ISensorEvents *pMyEvents = NULL;

            if (SUCCEEDED(hr)) {
                // Create an instance of the event class.
                pEventClass = new(std::nothrow) CMyEvents();
            }

            if (SUCCEEDED(hr)) {
                // Retrieve the pointer to the callback interface.
                hr = pEventClass->QueryInterface(IID_PPV_ARGS(&pMyEvents));
            }
            if (SUCCEEDED(hr)) {
                // Start receiving events.
                hr = pSensor->SetEventSink(pMyEvents);
            }
            if (SUCCEEDED(hr)) {
                Sleep(100000000);
            }

        }
    } catch (std::exception &e) {
        std::cerr << e.what() << std::endl;
        std::exit(-1);
    }

}