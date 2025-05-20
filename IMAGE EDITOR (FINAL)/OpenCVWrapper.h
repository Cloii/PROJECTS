#pragma once

#include <opencv2/opencv.hpp>
#include <msclr/marshal_cppstd.h>

using namespace System; // Add this to ensure IntPtr is accessible
using namespace System::Drawing;

// Simple OpenCV wrapper in C++/CLI
ref class OpenCVWrapper {
public:
    static cv::Mat BitmapToMat(System::Drawing::Bitmap^ bitmap) {
        System::Drawing::Imaging::BitmapData^ bmpData = bitmap->LockBits(
            System::Drawing::Rectangle(0, 0, bitmap->Width, bitmap->Height),
            System::Drawing::Imaging::ImageLockMode::ReadOnly,
            System::Drawing::Imaging::PixelFormat::Format32bppArgb);

        cv::Mat mat(bitmap->Height, bitmap->Width, CV_8UC4, bmpData->Scan0.ToPointer(), bmpData->Stride);
        cv::Mat result;
        mat.copyTo(result);
        bitmap->UnlockBits(bmpData);
        return result;
    }

    static System::Drawing::Bitmap^ MatToBitmap(cv::Mat mat) {
        System::Drawing::Bitmap^ bitmap = gcnew System::Drawing::Bitmap(mat.cols, mat.rows, mat.step, System::Drawing::Imaging::PixelFormat::Format32bppArgb, System::IntPtr(mat.data));
        return gcnew System::Drawing::Bitmap(bitmap); // Return a copy to avoid memory issues
    }
};