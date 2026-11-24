#include "inuDepthFilter.h"

using namespace std;

namespace Inuchip
{
    InuDepthFilter::InuDepthFilter(InuSensor *_inuSensor)
    {
        p_inuSensor = _inuSensor;
        depthParams = new DepthParams();
    }

    InuDepthFilter::~InuDepthFilter()
    {
         delete depthParams;
    }

    void InuDepthFilter::setDistance(uint32_t min_dist,uint32_t max_dist)
    {
        depthParams->depth_min_distance = min_dist;
        depthParams->depth_max_distance = max_dist;
    }

    void InuDepthFilter::setConfidence(uint8_t conf)
    {
        depthParams->confidence = conf;
    }

    void InuDepthFilter::setDepthScale(uint8_t scale)
    {
        depthParams->depth_scale = scale;
    }

    bool InuDepthFilter::setSDKTemporalFilter(int stable, int rapid)
    {
        depthParams->temporal_stable_sdk = stable;
        depthParams->temporal_rapid_sdk = rapid;
        return true;
    }

    bool InuDepthFilter::setSDKOutlierRemove(int percent, int dist)
    {
        depthParams->outlier_percent_sdk = percent;
        depthParams->outlier_dist_sdk = dist;

        return true;
    }

    bool InuDepthFilter::setSDKHoleFill(int radius)
    {
        depthParams->hole_radius_sdk = radius;

        return true;
    }

    bool InuDepthFilter::setSDKBlob(int mode, int maxsize, int maxheight, int threshold, int same)
    {
        depthParams->blob_maxsize_sdk = maxsize;
        depthParams->blob_maxheight_sdk = maxheight;
        depthParams->blob_threshold_sdk = threshold;
        depthParams->blob_sameblob_sdk = same;
        depthParams->blob_modeblob_sdk = mode;
        return true;
    }

    bool InuDepthFilter::_setSDKTemporalFilter( )
    {
        // can not set at runtime, must preset
        InuDev::CInuError retCode(0);
        CTemporalFilterParams temporalFilterParams;
        depthStream->GetTemporalFilterParams(temporalFilterParams);
        temporalFilterParams.StableThreshold = depthParams->temporal_stable_sdk;
        temporalFilterParams.RapidThreshold = depthParams->temporal_rapid_sdk;

        retCode = depthStream->SetTemporalFilterParams(temporalFilterParams);
        depthStream->GetTemporalFilterParams(temporalFilterParams);

        return true;
    }

    bool InuDepthFilter::_setSDKOutlierRemove( )
    {
        // do not use it
        InuDev::CInuError retCode(0);
        COutlierRemoveParams out;
        depthStream->GetOutlierRemoveParams(out);
        out.MaxPercent =depthParams->outlier_percent_sdk * 0.01f;
        out.MinDist = depthParams->outlier_dist_sdk;

        retCode = depthStream->SetOutlierRemoveParams(out);
        depthStream->GetOutlierRemoveParams(out);

        return true;
    }

    bool InuDepthFilter::_setSDKHoleFill( )
    {
        //run time
        InuDev::CInuError retCode(0);
        CHoleFillFilterParams holefill;
        depthStream->GetHoleFillParams(holefill);
        
        #ifdef _GE438_
		holefill.MaxSize = depthParams->hole_radius_sdk;
		holefill.MaxHeight = depthParams->hole_radius_sdk;
        #else
        holefill.MaxRadius = depthParams->hole_radius_sdk;
        #endif

        retCode = depthStream->SetHoleFillParams(holefill);
        depthStream->GetHoleFillParams(holefill);

        return true;
    }

    bool InuDepthFilter::_setSDKBlob( )
    {
        //run time
        InuDev::CInuError retCode(0);

        InuDev::CBlobFilterParams params;
        depthStream->GetBlobFilterParams(params);
        params.BlobMaxSize = depthParams->blob_maxsize_sdk;
        params.BlobMaxHeight = depthParams->blob_maxheight_sdk;
        params.BlobDisparityThreshold = depthParams->blob_threshold_sdk;
        params.BlobDispDiffSameBlob = depthParams->blob_sameblob_sdk;
        params.BlobMode = depthParams->blob_modeblob_sdk;
        retCode = depthStream->SetBlobFilterParams(params);
        depthStream->GetBlobFilterParams(params);

        return true;
    }

    bool InuDepthFilter::_setDistAndScale()
    {
        //dist is in m
        if(depthParams->depth_min_distance != depth_min_distance ||
            depthParams->depth_max_distance != depth_max_distance ||
            depthParams->depth_scale != depth_scale)
        {
            depth_min_distance = depthParams->depth_min_distance;
            depth_max_distance = depthParams->depth_max_distance;
            depth_scale = depthParams->depth_scale;
            //pcl_unit = depthParams->pcl_unit;

            InuDev::CInuError retCode(0);
            CDisparityParams disparityParams;
            depthStream->GetDisparityParams(disparityParams);
            disparityParams.MaxDistance = depth_max_distance;
            disparityParams.MinDistance = depth_min_distance;
            disparityParams.DepthScale = depth_scale;

            // disparityParams.PointCloudScale = 0.001f;
            // if(depthParams->pcl_unit == "m")
            // disparityParams.PointCloudScale = 1.0f;

            retCode = depthStream->SetDisparityParams(disparityParams);
        }
        return true;
    }

    bool InuDepthFilter::_setConfidence()
    {
        if(depthParams->confidence != confidence)
        {
            confidence = depthParams->confidence;

            p_inuSensor->inuSensor->SetSensorRegister(0x08010ae0, (confidence*2+1)<<24, 1, InuDev::eLeft);
            p_inuSensor->inuSensor->SetSensorRegister(0x080109e0, (confidence*2+1)<<24, 1, InuDev::eRight);
        }
        return true;
    }

    bool InuDepthFilter::_setSDKRuntimefilter()
    {
        // if(hole_radius_sdk != depthParams->hole_radius_sdk)
        // {
        //     setSDKHoleFill();
        //     hole_radius_sdk = depthParams->hole_radius_sdk;
        // }
        // //seems SDK outlier remove function is no effection.
        // if(outlier_percent_sdk != depthParams->outlier_percent_sdk||
        //     outlier_dist_sdk != depthParams->outlier_dist_sdk)
        // {
        //     setSDKOutlierRemove();
        //     outlier_percent_sdk = depthParams->outlier_percent_sdk;
        //     outlier_dist_sdk = depthParams->outlier_dist_sdk;
        // }
        // if(temporal_stable_sdk != depthParams->temporal_stable_sdk||
        //     temporal_rapid_sdk != depthParams->temporal_rapid_sdk)
        // {
        //     setSDKTemporalFileter();
        //     temporal_stable_sdk = depthParams->temporal_stable_sdk;
        //     temporal_rapid_sdk = depthParams->temporal_rapid_sdk;
        // }
        if(blob_maxsize_sdk != depthParams->blob_maxsize_sdk||
            blob_maxheight_sdk != depthParams->blob_maxheight_sdk||
            blob_threshold_sdk != depthParams->blob_threshold_sdk||
            blob_sameblob_sdk != depthParams->blob_sameblob_sdk||
            blob_modeblob_sdk != depthParams->blob_modeblob_sdk)
        {
            _setSDKBlob();
            blob_maxsize_sdk = depthParams->blob_maxsize_sdk;
            blob_maxheight_sdk = depthParams->blob_maxheight_sdk;
            blob_threshold_sdk = depthParams->blob_threshold_sdk;
            blob_sameblob_sdk = depthParams->blob_sameblob_sdk;
            blob_modeblob_sdk = depthParams->blob_modeblob_sdk;
        }
        return true;
    }
}
