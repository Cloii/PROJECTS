#pragma once
#ifdef _DEBUG
#pragma comment(lib, "opencv_world4110d.lib")
#else
#pragma comment(lib, "opencv_world4110.lib")
#endif

#include <opencv2/opencv.hpp>
#include <opencv2/imgproc.hpp>

namespace proj1 {
    using namespace System;
    using namespace System::ComponentModel;
    using namespace System::Collections;
    using namespace System::Windows::Forms;
    using namespace System::Data;
    using namespace System::Drawing;
    using namespace System::Drawing::Imaging;
    using namespace System::Collections::Generic;
    using namespace System::Runtime::InteropServices;

    public ref class ShapeDetectionForm : public System::Windows::Forms::Form
    {
    public:
        ShapeDetectionForm(Bitmap^ image)
        {
            InitializeComponent();
            PostInitializeSetup(); // Call the new method to set AutoSize and Font
            if (image == nullptr) throw gcnew ArgumentNullException("image");

            originalImage = ResizeImageIfNeeded(image, 1000);
            pictureBox1->Image = gcnew Bitmap(originalImage);
            pictureBox1->SizeMode = PictureBoxSizeMode::Zoom;

            backgroundWorker = gcnew BackgroundWorker();
            backgroundWorker->WorkerReportsProgress = true;
            backgroundWorker->DoWork += gcnew DoWorkEventHandler(this, &ShapeDetectionForm::backgroundWorker_DoWork);
            backgroundWorker->RunWorkerCompleted += gcnew RunWorkerCompletedEventHandler(this, &ShapeDetectionForm::backgroundWorker_RunWorkerCompleted);

            circle = 0;
            triangle = 0;
            rectangles = 0;
            squares = 0;
            others = 0;
            shapeColors = gcnew Dictionary<int, Tuple<String^, Color>^>();
        }

        void SetImage(Bitmap^ image) {
            if (image == nullptr) throw gcnew ArgumentNullException("image");

            originalImage = ResizeImageIfNeeded(image, 1000);

            pictureBox1->Image = gcnew Bitmap(originalImage);
            pictureBox1->SizeMode = PictureBoxSizeMode::Zoom;

            processedImage = nullptr;

            label1->Text = L"Detected Shapes: N/A";
            colorTextBox->Text = L"Details: N/A";
            circle = 0;
            triangle = 0;
            rectangles = 0;
            squares = 0;
            others = 0;
            shapeColors->Clear();
        }

    protected:
        ~ShapeDetectionForm()
        {
            if (components) delete components;
        }

    private:
        PictureBox^ pictureBox1;
        Label^ label1;
        Button^ detectShapeButton;
        Button^ detectColorButton;
        TextBox^ colorTextBox;
        System::ComponentModel::Container^ components;
        Bitmap^ originalImage;
        Bitmap^ processedImage;
        String^ detectionResult;
        String^ colorInfoResult;
        BackgroundWorker^ backgroundWorker;
        int circle, triangle, rectangles, squares, others;
        Dictionary<int, Tuple<String^, Color>^>^ shapeColors;

        void InitializeComponent(void)
        {
            // Initialize components
            this->pictureBox1 = (gcnew System::Windows::Forms::PictureBox());
            this->label1 = (gcnew System::Windows::Forms::Label());
            this->detectShapeButton = (gcnew System::Windows::Forms::Button());
            this->detectColorButton = (gcnew System::Windows::Forms::Button());
            this->colorTextBox = (gcnew System::Windows::Forms::TextBox());
            this->components = (gcnew System::ComponentModel::Container());

            // Suspend layout to avoid rendering issues during setup
            this->SuspendLayout();

            // pictureBox1 setup
            this->pictureBox1->Location = System::Drawing::Point(12, 12);
            this->pictureBox1->Name = L"pictureBox1";
            this->pictureBox1->Size = System::Drawing::Size(260, 180);
            this->pictureBox1->SizeMode = System::Windows::Forms::PictureBoxSizeMode::Zoom;
            this->pictureBox1->TabIndex = 0;
            this->pictureBox1->TabStop = false;

            // label1 setup
            this->label1->Location = System::Drawing::Point(12, 200);
            this->label1->Name = L"label1";
            this->label1->Size = System::Drawing::Size(260, 60);
            this->label1->TabIndex = 1;
            this->label1->Text = L"Detected Shapes: N/A";

            // detectShapeButton setup
            this->detectShapeButton->Location = System::Drawing::Point(12, 270);
            this->detectShapeButton->Name = L"detectShapeButton";
            this->detectShapeButton->Size = System::Drawing::Size(120, 30);
            this->detectShapeButton->TabIndex = 2;
            this->detectShapeButton->Text = L"Detect Shapes";
            this->detectShapeButton->UseVisualStyleBackColor = true;
            this->detectShapeButton->Click += gcnew System::EventHandler(this, &ShapeDetectionForm::detectShapeButton_Click);

            // detectColorButton setup
            this->detectColorButton->Location = System::Drawing::Point(152, 270);
            this->detectColorButton->Name = L"detectColorButton";
            this->detectColorButton->Size = System::Drawing::Size(120, 30);
            this->detectColorButton->TabIndex = 3;
            this->detectColorButton->Text = L"Detect Colors";
            this->detectColorButton->UseVisualStyleBackColor = true;
            this->detectColorButton->Click += gcnew System::EventHandler(this, &ShapeDetectionForm::detectColorButton_Click);

            // colorTextBox setup
            this->colorTextBox->Location = System::Drawing::Point(12, 310);
            this->colorTextBox->Multiline = true;
            this->colorTextBox->Name = L"colorTextBox";
            this->colorTextBox->Size = System::Drawing::Size(260, 100);
            this->colorTextBox->TabIndex = 4;
            this->colorTextBox->Text = L"Details: N/A";
            this->colorTextBox->ReadOnly = true;

            // ShapeDetectionForm setup
            this->AutoScaleDimensions = System::Drawing::SizeF(6, 13);
            this->AutoScaleMode = System::Windows::Forms::AutoScaleMode::Font;
            this->ClientSize = System::Drawing::Size(284, 420); // Adjusted to fit all controls
            this->Controls->Add(this->colorTextBox);
            this->Controls->Add(this->detectColorButton);
            this->Controls->Add(this->detectShapeButton);
            this->Controls->Add(this->label1);
            this->Controls->Add(this->pictureBox1);
            this->Name = L"ShapeDetectionForm";
            this->Text = L"Shape Detection";
            this->Load += gcnew System::EventHandler(this, &ShapeDetectionForm::ShapeDetectionForm_Load);
            this->ResumeLayout(false);
            this->PerformLayout();
        }

        void PostInitializeSetup()
        {
            this->label1->AutoSize = true;
            this->label1->Font = gcnew System::Drawing::Font(L"Arial", 10.0f, System::Drawing::FontStyle::Bold);
        }

        Bitmap^ ResizeImageIfNeeded(Bitmap^ bitmap, int maxDimension) {
            if (bitmap == nullptr) throw gcnew ArgumentNullException("bitmap");
            if (bitmap->Width <= maxDimension && bitmap->Height <= maxDimension) {
                return gcnew Bitmap(bitmap);
            }

            int newWidth = bitmap->Width;
            int newHeight = bitmap->Height;
            float aspectRatio = static_cast<float>(bitmap->Width) / bitmap->Height;

            if (bitmap->Width > bitmap->Height) {
                newWidth = maxDimension;
                newHeight = static_cast<int>(maxDimension / aspectRatio);
            }
            else {
                newHeight = maxDimension;
                newWidth = static_cast<int>(maxDimension * aspectRatio);
            }

            return gcnew Bitmap(bitmap, newWidth, newHeight);
        }

        cv::Mat BitmapToMat(Bitmap^ bitmap) {
            if (bitmap == nullptr) throw gcnew Exception("Bitmap is null.");
            if (bitmap->Width <= 0 || bitmap->Height <= 0) throw gcnew Exception("Bitmap has invalid dimensions.");

            Bitmap^ compatibleBitmap = nullptr;
            try {
                if (bitmap->PixelFormat != PixelFormat::Format24bppRgb) {
                    compatibleBitmap = gcnew Bitmap(bitmap->Width, bitmap->Height, PixelFormat::Format24bppRgb);
                    Graphics^ g = Graphics::FromImage(compatibleBitmap);
                    g->DrawImage(bitmap, 0, 0, bitmap->Width, bitmap->Height);
                    delete g;
                }
                else {
                    compatibleBitmap = bitmap;
                }

                BitmapData^ bmpData = compatibleBitmap->LockBits(
                    Drawing::Rectangle(0, 0, compatibleBitmap->Width, compatibleBitmap->Height),
                    ImageLockMode::ReadOnly, PixelFormat::Format24bppRgb);

                if (bmpData == nullptr) {
                    if (compatibleBitmap != bitmap) delete compatibleBitmap;
                    throw gcnew Exception("Failed to lock bitmap data.");
                }

                int expectedStride = compatibleBitmap->Width * 3;
                if (Math::Abs(bmpData->Stride) < expectedStride) {
                    compatibleBitmap->UnlockBits(bmpData);
                    if (compatibleBitmap != bitmap) delete compatibleBitmap;
                    throw gcnew Exception("Bitmap stride is too small.");
                }

                cv::Mat mat(compatibleBitmap->Height, compatibleBitmap->Width, CV_8UC3, bmpData->Scan0.ToPointer(), bmpData->Stride);
                cv::Mat matCopy = mat.clone();
                compatibleBitmap->UnlockBits(bmpData);
                if (compatibleBitmap != bitmap) delete compatibleBitmap;
                cv::cvtColor(matCopy, matCopy, cv::COLOR_BGR2RGB);
                return matCopy;
            }
            catch (Exception^ ex) {
                if (compatibleBitmap != nullptr && compatibleBitmap != bitmap) delete compatibleBitmap;
                throw gcnew Exception("Error in BitmapToMat: " + ex->Message, ex);
            }
        }

        Bitmap^ MatToBitmap(cv::Mat mat) {
            if (mat.empty() || mat.data == nullptr) throw gcnew Exception("cv::Mat is empty.");

            cv::Mat matCopy = mat.isContinuous() ? mat : mat.clone();
            if (matCopy.channels() == 3) {
                cv::cvtColor(matCopy, matCopy, cv::COLOR_RGB2BGR);
            }
            else if (matCopy.channels() == 1) {
                cv::cvtColor(matCopy, matCopy, cv::COLOR_GRAY2BGR);
            }
            else {
                throw gcnew Exception("Unsupported number of channels.");
            }

            Bitmap^ bitmap = gcnew Bitmap(matCopy.cols, matCopy.rows, PixelFormat::Format24bppRgb);
            BitmapData^ bmpData = bitmap->LockBits(
                Drawing::Rectangle(0, 0, bitmap->Width, bitmap->Height),
                ImageLockMode::WriteOnly, PixelFormat::Format24bppRgb);

            unsigned char* dstData = static_cast<unsigned char*>(bmpData->Scan0.ToPointer());
            unsigned char* srcData = matCopy.data;
            for (int y = 0; y < matCopy.rows; y++) {
                memcpy(dstData + y * bmpData->Stride, srcData + y * matCopy.step, matCopy.cols * 3);
            }

            bitmap->UnlockBits(bmpData);
            return bitmap;
        }

        String^ GetColorName(Color c) {
            // Convert RGB to HSV
            int r = c.R;
            int g = c.G;
            int b = c.B;

            double maxVal = Math::Max(r, Math::Max(g, b)) / 255.0;
            double minVal = Math::Min(r, Math::Min(g, b)) / 255.0;
            double delta = maxVal - minVal;

            double h = 0;
            if (delta != 0) {
                if (maxVal == r / 255.0) {
                    h = 60 * fmod((g / 255.0 - b / 255.0) / delta, 6);
                }
                else if (maxVal == g / 255.0) {
                    h = 60 * ((b / 255.0 - r / 255.0) / delta + 2);
                }
                else if (maxVal == b / 255.0) {
                    h = 60 * ((r / 255.0 - g / 255.0) / delta + 4);
                }
            }
            if (h < 0) h += 360;

            double s = (maxVal == 0) ? 0 : delta / maxVal;
            double v = maxVal;

            // Scale hue to OpenCV's range (0-180)
            h /= 2;

            // Classify based on HSV with improved ranges
            if (s < 0.2 && v > 0.8) return "white"; // Very low saturation and high value = white
            if (s < 0.2) return "gray"; // Low saturation = gray
            if (v < 0.2) return "black"; // Very low value = black

            if ((h >= 0 && h < 15) || (h >= 165 && h <= 180)) return "red"; // Red range
            if (h >= 15 && h < 45) { // Expanded yellow range
                if (s > 0.3 && v > 0.3) return "yellow"; // Ensure vividness
            }
            if (h >= 45 && h < 90) return "green"; // Green range
            if (h >= 90 && h < 150) return "blue"; // Blue range

            return "gray"; // Default for undefined ranges
        }

        void DetectShapes(Bitmap^ image) {
            try {
                if (image == nullptr) throw gcnew Exception("Image is null.");

                circle = 0;
                triangle = 0;
                rectangles = 0;
                squares = 0;
                others = 0;
                shapeColors->Clear();

                cv::Mat src = BitmapToMat(image);
                cv::Mat gray, blurred, edges;

                // Preprocessing
                cv::cvtColor(src, gray, cv::COLOR_RGB2GRAY);
                cv::GaussianBlur(gray, blurred, cv::Size(3, 3), 1);
                cv::adaptiveThreshold(blurred, edges, 255, cv::ADAPTIVE_THRESH_GAUSSIAN_C, cv::THRESH_BINARY_INV, 7, 2);

                // Morphological operations to reduce noise
                cv::Mat kernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(3, 3));
                cv::dilate(edges, edges, kernel);
                cv::erode(edges, edges, kernel);

                std::vector<std::vector<cv::Point>> contours;
                std::vector<cv::Vec4i> hierarchy;
                cv::findContours(edges, contours, hierarchy, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

                cv::Mat drawing = src.clone();

                for (size_t i = 0; i < contours.size(); i++) {
                    if (hierarchy[i][3] != -1) continue;

                    std::vector<cv::Point> approx;
                    double epsilon = cv::arcLength(contours[i], true) * 0.03;
                    cv::approxPolyDP(contours[i], approx, epsilon, true);

                    double area = cv::contourArea(approx);
                    if (area < 100) continue;

                    cv::Moments m = cv::moments(approx);
                    if (m.m00 == 0) continue;
                    int centerX = static_cast<int>(m.m10 / m.m00);
                    int centerY = static_cast<int>(m.m01 / m.m00);

                    if (centerX < 0 || centerX >= src.cols || centerY < 0 || centerY >= src.rows) continue;

                    // Create a mask for the shape and calculate mean color
                    cv::Mat mask = cv::Mat::zeros(src.size(), CV_8UC1);
                    cv::drawContours(mask, std::vector<std::vector<cv::Point>>{approx}, 0, cv::Scalar(255), -1);
                    cv::Scalar meanBGR = cv::mean(src, mask);
                    Color shapeColor = Color::FromArgb(
                        static_cast<int>(meanBGR[2]), // R
                        static_cast<int>(meanBGR[1]), // G
                        static_cast<int>(meanBGR[0])  // B
                    );
                    String^ colorName = GetColorName(shapeColor);

                    // Assign drawing color
                    cv::Scalar drawColor;
                    if (colorName == "blue") drawColor = cv::Scalar(255, 0, 0);
                    else if (colorName == "red") drawColor = cv::Scalar(0, 0, 255);
                    else if (colorName == "yellow") drawColor = cv::Scalar(0, 255, 255);
                    else if (colorName == "green") drawColor = cv::Scalar(0, 255, 0);
                    else drawColor = cv::Scalar(128, 128, 128);

                    String^ shapeType = "Unknown";
                    if (approx.size() == 3) {
                        triangle++;
                        shapeType = "Triangle";
                        cv::drawContours(drawing, std::vector<std::vector<cv::Point>>{approx}, 0, drawColor, 2);
                        cv::putText(drawing, "T", cv::Point(centerX, centerY), cv::FONT_HERSHEY_SIMPLEX, 0.5, drawColor, 2);
                    }
                    else if (approx.size() == 4) {
                        cv::Rect rect = cv::boundingRect(approx);
                        double aspectRatio = static_cast<double>(rect.width) / rect.height;
                        if (aspectRatio >= 0.8 && aspectRatio <= 1.2) {
                            squares++;
                            shapeType = "Square";
                            cv::drawContours(drawing, std::vector<std::vector<cv::Point>>{approx}, 0, drawColor, 2);
                            cv::putText(drawing, "S", cv::Point(centerX, centerY), cv::FONT_HERSHEY_SIMPLEX, 0.5, drawColor, 2);
                        }
                        else {
                            rectangles++;
                            shapeType = "Rectangle";
                            cv::drawContours(drawing, std::vector<std::vector<cv::Point>>{approx}, 0, drawColor, 2);
                            cv::putText(drawing, "R", cv::Point(centerX, centerY), cv::FONT_HERSHEY_SIMPLEX, 0.5, drawColor, 2);
                        }
                    }
                    else {
                        double perimeter = cv::arcLength(approx, true);
                        double circularity = 4 * CV_PI * area / (perimeter * perimeter);
                        if (circularity > 0.8 && approx.size() >= 5) {
                            circle++;
                            shapeType = "Circle";
                            cv::drawContours(drawing, std::vector<std::vector<cv::Point>>{approx}, 0, drawColor, 2);
                            cv::putText(drawing, "C", cv::Point(centerX, centerY), cv::FONT_HERSHEY_SIMPLEX, 0.5, drawColor, 2);
                        }
                        else {
                            others++;
                            shapeType = "Other";
                            cv::drawContours(drawing, std::vector<std::vector<cv::Point>>{approx}, 0, drawColor, 2);
                            cv::putText(drawing, "O", cv::Point(centerX, centerY), cv::FONT_HERSHEY_SIMPLEX, 0.5, drawColor, 2);
                        }
                    }

                    shapeColors->Add(static_cast<int>(i), gcnew Tuple<String^, Color>(shapeType, shapeColor));
                }

                processedImage = MatToBitmap(drawing);

                int totalObjects = circle + triangle + squares + rectangles + others;
                detectionResult = String::Format(
                    L"Geometric Shape Quantity:\r\n" +
                    L"Number of Circles: {0}\r\n" +
                    L"Number of Triangles: {1}\r\n" +
                    L"Number of Squares: {2}\r\n" +
                    L"Number of Rectangles: {3}\r\n" +
                    L"Number of Others: {4}\r\n" +
                    L"TOTAL # of Objects: {5}",
                    circle, triangle, squares, rectangles, others, totalObjects
                );
            }
            catch (cv::Exception& ex) {
                throw gcnew Exception(gcnew String(ex.what()));
            }
        }

        void DetectAllColors() {
            try {
                if (originalImage == nullptr) throw gcnew Exception("Original image is null.");
                if (shapeColors->Count == 0) throw gcnew Exception("No shapes detected. Please detect shapes first.");

                Dictionary<String^, int>^ shapeColorCounts = gcnew Dictionary<String^, int>();
                shapeColorCounts->Add("blue", 0);
                shapeColorCounts->Add("red", 0);
                shapeColorCounts->Add("yellow", 0);
                shapeColorCounts->Add("green", 0);
                shapeColorCounts->Add("gray", 0);
                shapeColorCounts->Add("white", 0);
                shapeColorCounts->Add("black", 0);

                for each (auto shape in shapeColors) {
                    String^ colorName = GetColorName(shape.Value->Item2);
                    if (shapeColorCounts->ContainsKey(colorName)) {
                        shapeColorCounts[colorName] = shapeColorCounts[colorName] + 1;
                    }
                }

                colorInfoResult = String::Format(
                    L"Shape Color Counts:\r\n" +
                    L"Blue: {0}\r\n" +
                    L"Red: {1}\r\n" +
                    L"Yellow: {2}\r\n" +
                    L"Green: {3}\r\n" +
                    L"Gray: {4}\r\n" +
                    L"White: {5}\r\n" +
                    L"Black: {6}",
                    shapeColorCounts["blue"],
                    shapeColorCounts["red"],
                    shapeColorCounts["yellow"],
                    shapeColorCounts["green"],
                    shapeColorCounts["gray"],
                    shapeColorCounts["white"],
                    shapeColorCounts["black"]
                );
            }
            catch (Exception^ ex) {
                throw ex;
            }
        }

        void detectShapeButton_Click(Object^ sender, EventArgs^ e) {
            if (!backgroundWorker->IsBusy) {
                if (originalImage == nullptr) {
                    MessageBox::Show(L"Error: Original image is not loaded.");
                    return;
                }
                try {
                    if (originalImage->Width <= 0 || originalImage->Height <= 0) {
                        MessageBox::Show(L"Invalid image dimensions.", L"Error", MessageBoxButtons::OK, MessageBoxIcon::Error);
                        return;
                    }

                    Bitmap^ imageCopy = gcnew Bitmap(originalImage);
                    detectShapeButton->Enabled = false;
                    label1->Text = L"Processing...";
                    backgroundWorker->RunWorkerAsync(gcnew Tuple<Bitmap^, String^>(imageCopy, "shapes"));
                }
                catch (Exception^ ex) {
                    MessageBox::Show(L"Error: Invalid image data. " + ex->Message);
                }
            }
        }

        void detectColorButton_Click(Object^ sender, EventArgs^ e) {
            if (!backgroundWorker->IsBusy) {
                if (originalImage == nullptr) {
                    MessageBox::Show(L"Error: Original image is not loaded.");
                    return;
                }
                try {
                    if (originalImage->Width <= 0 || originalImage->Height <= 0) {
                        MessageBox::Show(L"Invalid image dimensions.", L"Error", MessageBoxButtons::OK, MessageBoxIcon::Error);
                        return;
                    }

                    Bitmap^ imageCopy = gcnew Bitmap(originalImage);
                    detectColorButton->Enabled = false;
                    colorTextBox->Text = L"Detecting details...";
                    backgroundWorker->RunWorkerAsync(gcnew Tuple<Bitmap^, String^>(imageCopy, "colors"));
                }
                catch (Exception^ ex) {
                    MessageBox::Show(L"Error: Invalid image data. " + ex->Message);
                }
            }
        }

        void backgroundWorker_DoWork(Object^ sender, DoWorkEventArgs^ e) {
            Tuple<Bitmap^, String^>^ args = safe_cast<Tuple<Bitmap^, String^>^>(e->Argument);
            Bitmap^ imageCopy = args->Item1;
            String^ task = args->Item2;

            try {
                if (task == "shapes") {
                    DetectShapes(imageCopy);
                }
                else if (task == "colors") {
                    DetectAllColors();
                }
                e->Result = task;
            }
            catch (Exception^ ex) {
                e->Result = task;
                throw ex;
            }
        }

        void backgroundWorker_RunWorkerCompleted(Object^ sender, RunWorkerCompletedEventArgs^ e) {
            if (e->Error != nullptr) {
                String^ task = safe_cast<String^>(e->Result);
                MessageBox::Show(L"Error: " + e->Error->Message);
                if (task == "shapes") {
                    detectShapeButton->Enabled = true;
                    label1->Text = L"Error occurred during shape detection.";
                }
                else if (task == "colors") {
                    detectColorButton->Enabled = true;
                    colorTextBox->Text = L"Error occurred during detail detection.";
                }
                return;
            }

            String^ task = safe_cast<String^>(e->Result);
            if (task == "shapes") {
                pictureBox1->Image = processedImage;
                label1->Text = detectionResult;
                colorTextBox->Text = detectionResult;
                detectShapeButton->Enabled = true;
            }
            else if (task == "colors") {
                colorTextBox->Text = colorInfoResult;
                detectColorButton->Enabled = true;
            }
        }

        System::Void ShapeDetectionForm_Load(System::Object^ sender, System::EventArgs^ e) {
            // Optional: Add any initialization logic needed when the form loads
        }
    };
}