
#pragma once
#include "inuSensor.h"
#include "inuParams.h"
#include "inuDataPath.h"
#include "InuStreams.h"
#include "InuSensorExt.h"

using namespace InuDev;

namespace Inuchip
{
    class DepthParams
    {
    public:
        //hole
        int hole_on = 0;
        int hole_radius_min = 20;
        int hole_radous_max = 100;
        int hole_radius = 25;

        int hole_radius_sdk = 25;
        int hole_maxsize_sdk = 2500;
        int hole_maxheight_sdk = 300;

        //outlier
        int outlier_on = 0;
        int outlier_percent = 1;
        int outlier_percentmin = 1;
        int putlier_percentmax = 50;
        int outlier_dist = 45;
        int outlier_maxremove = 50;
        int outlier_distmin = 10;
        int outlier_distmax = 5000;

        int outlier_percent_sdk = 1;
        int outlier_dist_sdk = 45;

        //temporal1
        int temporal_on = 0;
        int temporal_all = 2;
        int temporal_buff = 6;
        int temporal_buff_min = 4;
        int temporal_buff_max = 15;

        //temporal2
        int temporal2_on = 0;
        int temporal2_alpha = 20;
        int temporal2_delta = 200;
        int temporal2_mode = 2;

        // static temporal sdk
        int static_temporal_on_sdk = 0;
        int static_temporal_all_sdk = 2;
        int static_temporal_buff_sdk = 6;
        // int temporal_buff_min_sdk = 4;
        // int temporal_buff_max_sdk = 15;

        // temporal sdk
        int temporal_stable_sdk = 50;  //16?
        int temporal_rapid_sdk = 500;  //96?

        //edge preserve
        int edgepreserve_on = 0;
        int edgepreserve_alpha = 50;
        int edgepreserve_delta = 20;

        // blob sdk
        int blob_maxsize_sdk = 2500;
        int blob_maxheight_sdk = 300;
        int blob_threshold_sdk = 0;
        int blob_sameblob_sdk = 1;
        int blob_modeblob_sdk = 4;

        // show depthvalue on opencv window
        int showDepthValue = 1;

        //adjust confidence
        unsigned int confidence = 0;

        //adjust depth scale
        unsigned int depth_scale = 1;

        bool need_restart = false;
        // depth color
        float scale = 20;
        int color_auto = 0;      //0:false 2:true

        // distance
        int depth_min_distance = 0;  //mm
        int depth_max_distance = 8000;  //mm
        std::string pcl_unit = "m";

        // pass through in depth
        int passthrough_on = 0;
        float passthrough_near = 40.0;      //mm
        float passthrough_far = 6000.0;     //mm
        // DPE
        // blob
        int dpe_blob_changed = 0;
        int dpe_Blob_on = 0;
        int dpe_BlobSize = 0;
        int dpe_BlobDiff = 0;
        // small
        int dpe_small_changed = 0;
        int dpe_small_on =0;
        // Plane
        int dpe_plane_changed = 0;

        int dpe_plane_on = 0;
        
        int dpe_plane_smoothMode = 0;
        int dpe_plane_grad_disp = 0;
        int dpe_plane_grad_min = 0;

        int dpe_plane_smooth_disp = 0;
        int dpe_plane_smooth_min = 0;
    };

    class InuDepthFilter 
    {
    public:
        InuDepthFilter(InuSensor *_inuSensor);

        ~InuDepthFilter();
        std::shared_ptr<InuDev::CDepthStream> depthStream;
        DepthParams *depthParams;

        bool setSDKTemporalFilter(int stable, int rapid);
        bool setSDKOutlierRemove(int percent, int dist);
        bool setSDKHoleFill(int radius);
        bool setSDKBlob(int mode, int maxsize, int maxheight, int threshold, int same);
        void setDistance(uint32_t min_dist,uint32_t max_dist);
        void setConfidence(uint8_t conf);
        void setDepthScale(uint8_t scale);

        bool _setSDKTemporalFilter();
        bool _setSDKOutlierRemove();
        bool _setSDKHoleFill();
        bool _setSDKBlob();
        bool _setDistAndScale();
        bool _setConfidence();
        bool _setSDKRuntimefilter();

    private:

        // std::shared_ptr<InuDev::CInuSensorExt> inuSensor;
        InuSensor *p_inuSensor;

    private:

        unsigned int confidence = 0;
        int32_t depth_min_distance = 0;
        int32_t depth_max_distance = 8000;
        uint8_t depth_scale = 0;

        // int temporal_stable_sdk = 50;  //16?
        // int temporal_rapid_sdk = 500;  //96?

        int blob_maxsize_sdk = 2500;
        int blob_maxheight_sdk = 300;
        int blob_threshold_sdk = 0;
        int blob_sameblob_sdk = 1;
        int blob_modeblob_sdk = 4;

    };
}