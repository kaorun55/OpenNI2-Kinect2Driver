#include "Driver\OniDriverAPI.h"
#include "XnLib.h"
#include "XnHash.h"
#include "XnEvent.h"

#include <atlbase.h>
#include <Kinect.h>
#include <vector>
#include <iostream>


static const int        COLOR_WIDTH  = 1920;
static const int        COLOR_HEIGHT = 1080;
static const int        DEPTH_WIDTH  = 512;
static const int        DEPTH_HEIGHT = 424;

typedef struct  
{
	int refCount;
} KinectV2StreamFrameCookie;

class KinectV2Stream : public oni::driver::StreamBase
{
public:
	~KinectV2Stream()
	{
		stop();
	}

	OniStatus start()
	{
		xnOSCreateThread(threadFunc, this, &m_threadHandle);

		return ONI_STATUS_OK;
	}

	void stop()
	{
		m_running = false;
        xnOSWaitForThreadExit( m_threadHandle, 1000 );
	}

	virtual OniStatus SetVideoMode(OniVideoMode*) = 0;
	virtual OniStatus GetVideoMode(OniVideoMode* pVideoMode) = 0;

    virtual OniStatus GetHorizontalFov( float* pValue ) = 0;
    virtual OniStatus GetVerticalFov( float* pValue ) = 0;
    virtual OniStatus GetStride( int* pValue ) = 0;
    virtual OniStatus GetMaxValue( int* pValue ){ return ONI_STATUS_NOT_IMPLEMENTED; }
    virtual OniStatus GetMinValue( int* pValue ){ return ONI_STATUS_NOT_IMPLEMENTED; }

	OniStatus getProperty(int propertyId, void* data, int* pDataSize)
	{
		if (propertyId == ONI_STREAM_PROPERTY_VIDEO_MODE)
		{
			if (*pDataSize != sizeof(OniVideoMode))
			{
				printf("Unexpected size: %d != %d\n", *pDataSize, (int)sizeof(OniVideoMode));
				return ONI_STATUS_ERROR;
			}

			return GetVideoMode((OniVideoMode*)data);
		}
        else if ( propertyId == ONI_STREAM_PROPERTY_HORIZONTAL_FOV ){
            if ( *pDataSize != sizeof( float ) )
            {
                printf( "Unexpected size: %d != %d\n", *pDataSize, (int)sizeof( float ) );
                return ONI_STATUS_ERROR;
            }

            return GetHorizontalFov( (float*)data );
        }
        else if ( propertyId == ONI_STREAM_PROPERTY_VERTICAL_FOV ){
            if ( *pDataSize != sizeof( float ) )
            {
                printf( "Unexpected size: %d != %d\n", *pDataSize, (int)sizeof( float ) );
                return ONI_STATUS_ERROR;
            }

            return GetVerticalFov( (float*)data );
        }
        else if ( propertyId == ONI_STREAM_PROPERTY_STRIDE ){
            if ( *pDataSize != sizeof( int ) )
            {
                printf( "Unexpected size: %d != %d\n", *pDataSize, (int)sizeof( int ) );
                return ONI_STATUS_ERROR;
            }

            return GetStride( (int*)data );
        }
        else if ( propertyId == ONI_STREAM_PROPERTY_MAX_VALUE ){
            if ( *pDataSize != sizeof( float ) )
            {
                printf( "Unexpected size: %d != %d\n", *pDataSize, (int)sizeof( float ) );
                return ONI_STATUS_ERROR;
            }

            return GetVerticalFov( (float*)data );
        }
        else if ( propertyId == ONI_STREAM_PROPERTY_MIN_VALUE ){
            if ( *pDataSize != sizeof( float ) )
            {
                printf( "Unexpected size: %d != %d\n", *pDataSize, (int)sizeof( float ) );
                return ONI_STATUS_ERROR;
            }

            return GetVerticalFov( (float*)data );
        }
		
        return ONI_STATUS_NOT_IMPLEMENTED;
	}

	OniStatus setProperty(int propertyId, const void* data, int dataSize)
	{
		if (propertyId == ONI_STREAM_PROPERTY_VIDEO_MODE)
		{
			if (dataSize != sizeof(OniVideoMode))
			{
				printf("Unexpected size: %d != %d\n", dataSize, (int)sizeof(OniVideoMode));
				return ONI_STATUS_ERROR;
			}
			return SetVideoMode((OniVideoMode*)data);
		}

		return ONI_STATUS_NOT_IMPLEMENTED;
	}

	//OniDriverFrame* AcquireFrame()
	//{
	//	OniDriverFrame* pFrame = (OniDriverFrame*)xnOSCalloc(1, sizeof(OniDriverFrame));
	//	if (pFrame == NULL)
	//	{
	//		XN_ASSERT(FALSE);
	//		return NULL;
	//	}

	//	OniVideoMode mode;
	//	GetVideoMode( &mode );

	//	int dataSize = mode.resolutionX * mode.resolutionY * GetBytesPerPixel();
	//	pFrame->frame.data = xnOSMallocAligned(dataSize, XN_DEFAULT_MEM_ALIGN);
	//	if (pFrame->frame.data == NULL)
	//	{
	//		XN_ASSERT(FALSE);
	//		return NULL;
	//	}

	//    xnOSMemSet( pFrame->frame.data, 0,  dataSize );

	//	pFrame->pDriverCookie = xnOSMalloc(sizeof(KinectV2StreamFrameCookie));
	//	((KinectV2StreamFrameCookie*)pFrame->pDriverCookie)->refCount = 1;

	//	pFrame->frame.dataSize = dataSize;
	//	return pFrame;
	//}

	virtual void Mainloop() = 0;
	virtual int GetBytesPerPixel() = 0;

protected:
	// Thread
	static XN_THREAD_PROC threadFunc(XN_THREAD_PARAM pThreadParam)
	{
		KinectV2Stream* pStream = (KinectV2Stream*)pThreadParam;
		pStream->m_running = true;
		pStream->Mainloop();

		XN_THREAD_PROC_RETURN(XN_STATUS_OK);
	}



	volatile bool m_running;

	XN_THREAD_HANDLE m_threadHandle;

};

class KinectV2ColorStream : public KinectV2Stream
{
public:
	KinectV2ColorStream( CComPtr<IKinectSensor>& kinect )
		: KinectV2Stream()
		, kinect_( kinect )
	{
        configureColorNode();

		m_frameId = 1;
	}

	OniStatus SetVideoMode(OniVideoMode*) {return ONI_STATUS_NOT_IMPLEMENTED;}
	OniStatus GetVideoMode(OniVideoMode* pVideoMode)
	{
		pVideoMode->pixelFormat = ONI_PIXEL_FORMAT_RGB888;
		pVideoMode->fps = 30;
		pVideoMode->resolutionX = COLOR_WIDTH;
		pVideoMode->resolutionY = COLOR_HEIGHT;
		return ONI_STATUS_OK;
	}

    OniStatus GetHorizontalFov( float* pValue )
    {
        *pValue = horizontalFieldOfView;
        return ONI_STATUS_OK;
    }

    OniStatus GetVerticalFov( float* pValue )
    {
        *pValue = verticalFieldOfView;
        return ONI_STATUS_OK;
    }

    OniStatus GetStride( int* pValue )
    {
        *pValue = stride;
        return ONI_STATUS_OK;
    }


	virtual int GetBytesPerPixel() { return sizeof(OniRGB888Pixel); }

	void Mainloop()
    {
		m_running = true;

		while (m_running)
		{
            OniFrame* pFrame = getServices().acquireFrame();
            BuildFrame(pFrame);
            raiseNewFrame(pFrame);
            getServices().releaseFrame( pFrame );
        }
    }

private:

	void configureColorNode()
	{
        CComPtr<IColorFrameSource> colorFrameSource;

        auto hr = kinect_->Open();
        if (FAILED(hr)) {
            std::cerr << "IKinectSensor::Open() failed." << std::endl;
        }

        hr = kinect_->get_ColorFrameSource(&colorFrameSource);
        if (FAILED(hr)) {
            std::cerr << "IKinectSensor::get_ColorFrameSource() failed." << std::endl;
            return;
        }

        hr = colorFrameSource->OpenReader(&colorFrameReader_);
        if (FAILED(hr)) {
            std::cerr << "IColorFrameSource::OpenReader() failed." << std::endl;
            return;
        }

        colorRGBX_.resize( COLOR_WIDTH * COLOR_HEIGHT );

        CComPtr<IFrameDescription> frameDescription;
        hr = colorFrameSource->CreateFrameDescription( COLOR_FORMAT, &frameDescription );
        if ( FAILED( hr ) ) {
            std::cerr << "IColorFrameSource::get_FrameDescription() failed." << std::endl;
            return;
        }

        frameDescription->get_HorizontalFieldOfView( &horizontalFieldOfView );
        frameDescription->get_VerticalFieldOfView( &verticalFieldOfView );

        int width;
        frameDescription->get_Width( &width );
        stride = width * GetBytesPerPixel();
    }

	virtual int BuildFrame(OniFrame* pFrame)
	{
        update();

		pFrame->frameIndex = m_frameId;

		pFrame->videoMode.pixelFormat = ONI_PIXEL_FORMAT_RGB888;
		pFrame->videoMode.resolutionX = COLOR_WIDTH;
		pFrame->videoMode.resolutionY = COLOR_HEIGHT;
		pFrame->videoMode.fps = 30;

		pFrame->width = COLOR_WIDTH;
		pFrame->height = COLOR_HEIGHT;
		
        OniRGB888Pixel* pixel =  (OniRGB888Pixel*)pFrame->data;
        for ( int i = 0; i < colorRGBX_.size(); ++i ) {
            pixel[i].r = colorRGBX_[i].rgbRed;
            pixel[i].g = colorRGBX_[i].rgbGreen;
            pixel[i].b = colorRGBX_[i].rgbBlue;
        }

		pFrame->cropOriginX = pFrame->cropOriginY = 0;
		pFrame->croppingEnabled = FALSE;

		pFrame->sensorType = ONI_SENSOR_COLOR;
		pFrame->stride = COLOR_WIDTH*sizeof(OniRGB888Pixel);
		pFrame->timestamp = m_frameId*33000;

		++m_frameId;

		return 1;
	}

    bool update()
    {
        if ( !colorFrameReader_ ) {
            return false;
        }

        CComPtr<IColorFrame> colorFrame;
        HRESULT hr = colorFrameReader_->AcquireLatestFrame(&colorFrame);
        if (FAILED(hr)) {
            return false;
        }

        hr = colorFrame->CopyConvertedFrameDataToArray(colorRGBX_.size() * sizeof(RGBQUAD),
            reinterpret_cast<BYTE*>(&colorRGBX_[0]), COLOR_FORMAT );
        if (FAILED(hr)) {
            return false;
        }


        return true;
    }

	int m_frameId;


    CComPtr<IKinectSensor>          kinect_;
    CComPtr<IColorFrameReader>      colorFrameReader_;
    float horizontalFieldOfView;
    float verticalFieldOfView;
    int stride;

    std::vector<RGBQUAD> colorRGBX_;

    const ColorImageFormat COLOR_FORMAT = ColorImageFormat::ColorImageFormat_Bgra;
};


class KinectV2DepthStream : public KinectV2Stream
{
public:
	KinectV2DepthStream( CComPtr<IKinectSensor>& kinect )
		: KinectV2Stream()
		, kinect_( kinect )
        , bufferSize_( 0 )
        , buffer_( 0 )
	{
        configureDepthNode();

		m_frameId = 1;
	}

	OniStatus SetVideoMode(OniVideoMode*) {return ONI_STATUS_NOT_IMPLEMENTED;}
	OniStatus GetVideoMode(OniVideoMode* pVideoMode)
	{
        pVideoMode->pixelFormat = ONI_PIXEL_FORMAT_DEPTH_1_MM;
		pVideoMode->fps = 30;
		pVideoMode->resolutionX = DEPTH_WIDTH;
		pVideoMode->resolutionY = DEPTH_HEIGHT;
		return ONI_STATUS_OK;
	}

    OniStatus GetHorizontalFov( float* pValue )
    {
        *pValue = horizontalFieldOfView;
        return ONI_STATUS_OK;
    }

    OniStatus GetVerticalFov( float* pValue )
    {
        *pValue = verticalFieldOfView;
        return ONI_STATUS_OK;
    }

    OniStatus GetStride( int* pValue )
    {
        *pValue = stride;
        return ONI_STATUS_OK;
    }

    OniStatus GetMaxValue( int* pValue )
    {
        *pValue = depthMaxValue;
        return ONI_STATUS_OK;
    }

    OniStatus GetMinValue( int* pValue )
    {
        *pValue = depthMinValue;
        return ONI_STATUS_OK;
    }

	virtual int GetBytesPerPixel() { return sizeof(OniDepthPixel); }

	void Mainloop()
    {
		m_running = true;

		while (m_running)
		{
            OniFrame* pFrame = getServices().acquireFrame();
            BuildFrame(pFrame);
            raiseNewFrame(pFrame);
            getServices().releaseFrame( pFrame );
        }
    }

private:

	void configureDepthNode()
	{
        CComPtr<IDepthFrameSource> depthFrameSource;
        auto hr = kinect_->get_DepthFrameSource(&depthFrameSource);
        if (FAILED(hr)) {
            std::cerr << "IKinectSensor::get_DepthFrameSource() failed." << std::endl;
            return;
        }

        hr = depthFrameSource->OpenReader(&depthFrameReader_);
        if (FAILED(hr)) {
            std::cerr << "IDepthFrameSource::OpenReader() failed." << std::endl;
            return;
        }

        CComPtr<IFrameDescription> frameDescription;
        hr = depthFrameSource->get_FrameDescription( &frameDescription );
        if ( FAILED( hr ) ) {
            std::cerr << "IDepthFrameSource::get_FrameDescription() failed." << std::endl;
            return;
        }

        frameDescription->get_HorizontalFieldOfView( &horizontalFieldOfView );
        frameDescription->get_VerticalFieldOfView( &verticalFieldOfView );

        int width;
        frameDescription->get_Width( &width );
        stride = width * GetBytesPerPixel();


        depthFrameSource->get_DepthMaxReliableDistance( &depthMaxValue );
        depthFrameSource->get_DepthMinReliableDistance( &depthMinValue );
    }

	virtual int BuildFrame(OniFrame* pFrame)
	{
        update();

		pFrame->frameIndex = m_frameId;

		pFrame->videoMode.pixelFormat = ONI_PIXEL_FORMAT_DEPTH_1_MM;
		pFrame->videoMode.resolutionX = DEPTH_WIDTH;
		pFrame->videoMode.resolutionY = DEPTH_HEIGHT;
		pFrame->videoMode.fps = 30;

		pFrame->width = DEPTH_WIDTH;
		pFrame->height = DEPTH_HEIGHT;
		
        auto count = bufferSize_ * GetBytesPerPixel();
        if ( pFrame->data != 0 && depthBuffer_.size() != 0 && pFrame->dataSize == count ) {
            fprintf( stderr, "update()%d, %d, %d, %d\n", pFrame->data, &depthBuffer_[0], pFrame->dataSize,  count );
	        xnOSMemCopy( pFrame->data, &depthBuffer_[0],  count );
        }

		pFrame->cropOriginX = pFrame->cropOriginY = 0;
		pFrame->croppingEnabled = FALSE;

		pFrame->sensorType = ONI_SENSOR_DEPTH;
		pFrame->stride = DEPTH_WIDTH * GetBytesPerPixel();
		pFrame->timestamp = m_frameId*33000;

		++m_frameId;

		return 1;
	}

    bool update()
    {
        if (!depthFrameReader_) {
            return false;
        }

        CComPtr<IDepthFrame> depthFrame;
        HRESULT hr = depthFrameReader_->AcquireLatestFrame(&depthFrame);
        if (FAILED(hr)) {
            return false;
        }

        hr = depthFrame->AccessUnderlyingBuffer(&bufferSize_, &buffer_);            
        if (FAILED(hr)) {
            return false;
        }
        
        depthBuffer_.resize( bufferSize_ );
        std::copy( &buffer_[0], &buffer_[bufferSize_], depthBuffer_.begin() );

        return true;
    }

	int m_frameId;

    CComPtr<IKinectSensor>          kinect_;
    CComPtr<IDepthFrameReader>      depthFrameReader_;

    float horizontalFieldOfView;
    float verticalFieldOfView;
    int stride;
    UINT16 depthMinValue;
    UINT16 depthMaxValue;

    std::vector<UINT16> depthBuffer_;

    UINT bufferSize_;
    UINT16 *buffer_;
};


class KinectV2Device : public oni::driver::DeviceBase
{
public:
	KinectV2Device(oni::driver::DriverServices& driverServices, CComPtr<IKinectSensor>& kinect)
        : kinect_( kinect )
        , m_driverServices(driverServices)
	{
		m_numSensors = 2;

		m_sensors[0].pSupportedVideoModes = XN_NEW_ARR(OniVideoMode, 1);
		m_sensors[0].sensorType = ONI_SENSOR_DEPTH;
		m_sensors[0].numSupportedVideoModes = 1;
		m_sensors[0].pSupportedVideoModes[0].pixelFormat = ONI_PIXEL_FORMAT_DEPTH_1_MM;
		m_sensors[0].pSupportedVideoModes[0].fps = 30;
		m_sensors[0].pSupportedVideoModes[0].resolutionX = DEPTH_WIDTH;
        m_sensors[0].pSupportedVideoModes[0].resolutionY = DEPTH_HEIGHT;

		m_sensors[1].pSupportedVideoModes = XN_NEW_ARR(OniVideoMode, 1);
		m_sensors[1].sensorType = ONI_SENSOR_COLOR;
		m_sensors[1].numSupportedVideoModes = 1;
		m_sensors[1].pSupportedVideoModes[0].pixelFormat = ONI_PIXEL_FORMAT_RGB888;
		m_sensors[1].pSupportedVideoModes[0].fps = 30;
        m_sensors[1].pSupportedVideoModes[0].resolutionX = COLOR_WIDTH;
        m_sensors[1].pSupportedVideoModes[0].resolutionY = COLOR_HEIGHT;
	}
	OniDeviceInfo* GetInfo()
	{
		return m_pInfo;
	}

	OniStatus getSensorInfoList(OniSensorInfo** pSensors, int* numSensors)
	{
		*numSensors = m_numSensors;
		*pSensors = m_sensors;

		return ONI_STATUS_OK;
	}

	oni::driver::StreamBase* createStream(OniSensorType sensorType)
	{
		if (sensorType == ONI_SENSOR_COLOR) {
            return XN_NEW(KinectV2ColorStream, kinect_);
		}
        else  if (sensorType == ONI_SENSOR_DEPTH) {
            return XN_NEW(KinectV2DepthStream, kinect_);
		}

		return NULL;
	}

	void destroyStream(oni::driver::StreamBase* pStream)
	{
		XN_DELETE(pStream);
	}

	OniStatus  getProperty(int propertyId, void* data, int* pDataSize)
	{
		OniStatus rc = ONI_STATUS_OK;

		switch (propertyId)
		{
		case ONI_DEVICE_PROPERTY_DRIVER_VERSION:
			{
				if (*pDataSize == sizeof(OniVersion))
				{
					OniVersion* version = (OniVersion*)data;
					version->major = version->minor = version->maintenance = version->build = 2;
				}
				else
				{
					m_driverServices.errorLoggerAppend("Unexpected size: %d != %d\n", *pDataSize, sizeof(OniVersion));
					rc = ONI_STATUS_ERROR;
				}
			}
			break;
		default:
			m_driverServices.errorLoggerAppend("Unknown property: %d\n", propertyId);
			rc = ONI_STATUS_ERROR;
		}
		return rc;
	}
private:
	KinectV2Device(const KinectV2Device&);
	void operator=(const KinectV2Device&);

    CComPtr<IKinectSensor>	kinect_;

	OniDeviceInfo* m_pInfo;
	int m_numSensors;
	OniSensorInfo m_sensors[10];
	oni::driver::DriverServices& m_driverServices;
};

class Kinect2Driver : public oni::driver::DriverBase
{
public:
	Kinect2Driver(OniDriverServices* pDriverServices) : DriverBase(pDriverServices)
	{
	}

	virtual OniStatus initialize(
		oni::driver::DeviceConnectedCallback connectedCallback,
		oni::driver::DeviceDisconnectedCallback disconnectedCallback,
		oni::driver::DeviceStateChangedCallback deviceStateChangedCallback,
		void* pCookie)
	{
        ::OutputDebugStringA( "Initialize Kinect\n" );

		oni::driver::DriverBase::initialize(connectedCallback, disconnectedCallback, deviceStateChangedCallback, pCookie);

        // Open Kinect v2
        auto hr = ::GetDefaultKinectSensor( &kinect_ );
        if ( FAILED( hr ) ) {
			return ONI_STATUS_NO_DEVICE;
        }

		// Create device info
		OniDeviceInfo* pInfo = XN_NEW(OniDeviceInfo);
		xnOSStrCopy(pInfo->vendor, "Microsoft", ONI_MAX_STR);
		xnOSStrCopy(pInfo->name, "Kinect for Windows v2", ONI_MAX_STR);
		xnOSStrCopy(pInfo->uri, "Kinect for Windows v2", ONI_MAX_STR);

		// internal connect device
		deviceConnected(pInfo);
		deviceStateChanged(pInfo, 0);

		return ONI_STATUS_OK;
	}

	virtual oni::driver::DeviceBase* deviceOpen(const char* uri, const char* /*mode*/)
	{
        if ( !kinect_ ) {
            return 0;
        }

        ::OutputDebugStringA( "Open Kinect\n" );

        auto hr = kinect_->Open();
        if ( FAILED( hr ) ) {
            std::cerr << "IKinectSensor::Open() failed." << std::endl;
            return 0;
        }

        return XN_NEW( KinectV2Device, getServices(), kinect_ );
	}

	virtual void deviceClose(oni::driver::DeviceBase* pDevice)
	{
	}

	virtual OniStatus tryDevice(const char* )
	{
		return ONI_STATUS_ERROR;
	}



	void shutdown() {
	}

protected:

    CComPtr<IKinectSensor>	kinect_;
};

ONI_EXPORT_DRIVER(Kinect2Driver);
