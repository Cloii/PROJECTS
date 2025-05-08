#include "ShapeDetectionForm.h"
#include <windows.h>
#include <opencv2/opencv.hpp>
#include <opencv2/imgproc.hpp>
#include <algorithm>
#include <random>
#include <vector> 

namespace proj1 {
    using namespace System;
    using namespace System::ComponentModel;
    using namespace System::Collections;
    using namespace System::Windows::Forms;
    using namespace System::Data;
    using namespace System::Drawing;
    using namespace System::Drawing::Imaging;
    using namespace System::Collections::Generic;
    using namespace Microsoft::VisualBasic;

    // Helper function to convert Bitmap^ to cv::Mat
    cv::Mat BitmapToMat(Bitmap^ bitmap) {
        BitmapData^ bmpData = bitmap->LockBits(
            System::Drawing::Rectangle(0, 0, bitmap->Width, bitmap->Height),
            ImageLockMode::ReadOnly, PixelFormat::Format32bppArgb);
        cv::Mat mat(bitmap->Height, bitmap->Width, CV_8UC4);
        memcpy(mat.data, bmpData->Scan0.ToPointer(), bitmap->Height * bmpData->Stride);
        bitmap->UnlockBits(bmpData);
        cv::cvtColor(mat, mat, cv::COLOR_BGRA2BGR);
        return mat;
    }

    // Helper function to convert cv::Mat to Bitmap^
    Bitmap^ MatToBitmap(const cv::Mat& mat) {
        cv::Mat matBGR;
        if (mat.channels() == 1) {
            cv::cvtColor(mat, matBGR, cv::COLOR_GRAY2BGR);
        }
        else if (mat.channels() == 4) {
            cv::cvtColor(mat, matBGR, cv::COLOR_BGRA2BGR);
        }
        else {
            matBGR = mat.clone();
        }
        Bitmap^ bitmap = gcnew Bitmap(matBGR.cols, matBGR.rows, PixelFormat::Format24bppRgb);
        BitmapData^ bmpData = bitmap->LockBits(
            System::Drawing::Rectangle(0, 0, bitmap->Width, bitmap->Height),
            ImageLockMode::WriteOnly, PixelFormat::Format24bppRgb);
        for (int y = 0; y < matBGR.rows; y++) {
            memcpy(
                (unsigned char*)bmpData->Scan0.ToPointer() + y * bmpData->Stride,
                matBGR.ptr(y),
                matBGR.cols * 3);
        }
        bitmap->UnlockBits(bmpData);
        return bitmap;
    }

    public ref class MainForm : public System::Windows::Forms::Form
    {
    public:
        MainForm(void)
        {
            InitializeComponent();

            // Initialize trackbar properties
            trackBar1->Minimum = -255; trackBar1->Maximum = 255; trackBar1->Value = 0;
            trackBar1->TickFrequency = 50; trackBar1->LargeChange = 10; trackBar1->SmallChange = 1;
            trackBar1->Scroll += gcnew System::EventHandler(this, &MainForm::trackBar_Scroll);

            trackBar2->Minimum = -255; trackBar2->Maximum = 255; trackBar2->Value = 0;
            trackBar2->TickFrequency = 50; trackBar2->LargeChange = 10; trackBar2->SmallChange = 1;
            trackBar2->Scroll += gcnew System::EventHandler(this, &MainForm::trackBar_Scroll);

            trackBar3->Minimum = -255; trackBar3->Maximum = 255; trackBar3->Value = 0;
            trackBar3->TickFrequency = 50; trackBar3->LargeChange = 10; trackBar3->SmallChange = 1;
            trackBar3->Scroll += gcnew System::EventHandler(this, &MainForm::trackBar_Scroll);

            trackBar4->Minimum = -100; trackBar4->Maximum = 100; trackBar4->Value = 0;
            trackBar4->TickFrequency = 10; trackBar4->LargeChange = 5; trackBar4->SmallChange = 1;
            trackBar4->Scroll += gcnew System::EventHandler(this, &MainForm::trackBar_Scroll);

            trackBar5->Minimum = -100; trackBar5->Maximum = 100; trackBar5->Value = 0;
            trackBar5->TickFrequency = 10; trackBar5->LargeChange = 5; trackBar5->SmallChange = 1;
            trackBar5->Scroll += gcnew System::EventHandler(this, &MainForm::trackBar_Scroll);

            trackBar6->Minimum = -100; trackBar6->Maximum = 100; trackBar6->Value = 0;
            trackBar6->TickFrequency = 10; trackBar6->LargeChange = 5; trackBar6->SmallChange = 1;
            trackBar6->Scroll += gcnew System::EventHandler(this, &MainForm::trackBar_Scroll);

            trackBar7->Minimum = 0; trackBar7->Maximum = 100; trackBar7->Value = 100;
            trackBar7->TickFrequency = 10; trackBar7->LargeChange = 5; trackBar7->SmallChange = 1;
            trackBar7->Scroll += gcnew System::EventHandler(this, &MainForm::trackBar_Scroll);

            trackBar8->Minimum = 0; trackBar8->Maximum = 10; trackBar8->Value = 0;
            trackBar8->TickFrequency = 1; trackBar8->LargeChange = 2; trackBar8->SmallChange = 1;
            trackBar8->Scroll += gcnew System::EventHandler(this, &MainForm::trackBar_Scroll);

            trackBar9->Minimum = 0; trackBar9->Maximum = 255; trackBar9->Value = 128;
            trackBar9->TickFrequency = 25; trackBar9->LargeChange = 10; trackBar9->SmallChange = 1;
            trackBar9->Scroll += gcnew System::EventHandler(this, &MainForm::trackBar_Scroll);

            trackBar10->Minimum = 0; trackBar10->Maximum = 255; trackBar10->Value = 0;
            trackBar10->TickFrequency = 25; trackBar10->LargeChange = 10; trackBar10->SmallChange = 1;
            trackBar10->Scroll += gcnew System::EventHandler(this, &MainForm::trackBar_Scroll);

            trackBar11->Minimum = 0; trackBar11->Maximum = 255; trackBar11->Value = 255;
            trackBar11->TickFrequency = 25; trackBar11->LargeChange = 10; trackBar11->SmallChange = 1;
            trackBar11->Scroll += gcnew System::EventHandler(this, &MainForm::trackBar_Scroll);

            redHistogram = nullptr;
            greenHistogram = nullptr;
            blueHistogram = nullptr;
            lastFilterColor = Color::Empty;
            isThresholdActive = false;
            removeBackground = false;
            undoStack = gcnew System::Collections::Generic::Stack<ImageState^>();


            // Initialize debounceTimer
            debounceTimer = gcnew System::Windows::Forms::Timer();
            debounceTimer->Interval = 100; // 100ms debounce delay
            debounceTimer->Tick += gcnew System::EventHandler(this, &MainForm::debounceTimer_Tick);

            // Initialize state variables
            isThresholdActive = false; // Disable threshold by default
            removeBackground = false;
            lastFilterColor = Color::Empty;


          
        }

    protected:
        ~MainForm()
        {
            if (components) delete components;
            while (undoStack->Count > 0) delete undoStack->Pop();
            delete undoStack;
            if (pictureBox1->Image != nullptr) delete pictureBox1->Image;
            if (pictureBox2->Image != nullptr) delete pictureBox2->Image;
        }

    private:
        ref class ImageState {
        public:
            Bitmap^ image;
            int redAdjustment, greenAdjustment, blueAdjustment;
            int brightness, contrast, saturation, opacity, blurRadius, thresholdValue;
            int blackThreshold, whiteThreshold;
            Color filterColor;
            bool isThresholdActive;
            bool removeBackground;

            ImageState(Bitmap^ img, int r, int g, int b, int br, int co, int sa, int op, int bl, int th,
                int black, int white, Color fc, bool thresh, bool bg) {
                image = gcnew Bitmap(img);
                redAdjustment = r; greenAdjustment = g; blueAdjustment = b;
                brightness = br; contrast = co; saturation = sa; opacity = op;
                blurRadius = bl; thresholdValue = th;
                blackThreshold = black; whiteThreshold = white;
                filterColor = fc; isThresholdActive = thresh; removeBackground = bg;
            }

            ~ImageState() { delete image; }
        };

        ref struct Pixel {
            int r, g, b, a;
            int cluster;
        };

    private:
        System::Windows::Forms::Button^ button1;
        System::Windows::Forms::Button^ button2;
        System::Windows::Forms::Button^ button3;

        System::Windows::Forms::Button^ button5;
        System::Windows::Forms::Button^ button6;
        System::Windows::Forms::Button^ button7;
        System::Windows::Forms::Button^ button8;
        System::Windows::Forms::Button^ button9;
        System::Windows::Forms::Label^ label2;
        System::Windows::Forms::Label^ label3;
        System::Windows::Forms::PictureBox^ pictureBox1;
        System::Windows::Forms::PictureBox^ pictureBox2;
        System::Windows::Forms::TrackBar^ trackBar1;
        System::Windows::Forms::TrackBar^ trackBar2;
        System::Windows::Forms::TrackBar^ trackBar3;
        System::Windows::Forms::TrackBar^ trackBar4;
        System::Windows::Forms::TrackBar^ trackBar5;
        System::Windows::Forms::TrackBar^ trackBar6;
        System::Windows::Forms::TrackBar^ trackBar7;
        System::Windows::Forms::TrackBar^ trackBar8;
        System::Windows::Forms::TrackBar^ trackBar9;
        System::Windows::Forms::TrackBar^ trackBar10;
        System::Windows::Forms::TrackBar^ trackBar11;
        System::Windows::Forms::Label^ label4;
        System::Windows::Forms::Label^ label5;
        System::Windows::Forms::Label^ label6;
        System::Windows::Forms::Label^ label7;
        System::Windows::Forms::Label^ label8;
        System::Windows::Forms::Label^ label9;
        System::Windows::Forms::Label^ label10;
        System::Windows::Forms::Label^ label11;
        System::Windows::Forms::Label^ label12;
        System::Windows::Forms::Label^ thresholdValueLabel;
        System::Windows::Forms::Label^ blackValueLabel;
        System::Windows::Forms::Label^ whiteValueLabel;
        System::Windows::Forms::Label^ colorClustersLabel;
        System::Windows::Forms::GroupBox^ groupBoxControls;
        System::Windows::Forms::Button^ btnRGB;
        System::Windows::Forms::Button^ btnThreshold;
        System::Windows::Forms::Button^ btnEnhancements;
        System::Windows::Forms::Button^ btnFilters;
        System::Windows::Forms::Panel^ panelRGB;
        System::Windows::Forms::Panel^ panelThreshold;
        System::Windows::Forms::Panel^ panelEnhancements;
        System::Windows::Forms::Panel^ panelFilters;
        System::Windows::Forms::Button^ btnBlueGreen;
        System::Windows::Forms::Button^ btnViolet;
        System::Windows::Forms::Button^ btnBrown;
        System::Windows::Forms::Button^ btnKhaki;
        System::Windows::Forms::Button^ btnOrange;
        System::Windows::Forms::Button^ btnCyan;
        System::Windows::Forms::Button^ btnPink;
        System::Windows::Forms::Button^ btnTeal;
        System::Windows::Forms::Button^ btnLime;
        System::Windows::Forms::Button^ btnMagenta;
        System::Windows::Forms::PictureBox^ histogramBox;
        System::Windows::Forms::Label^ dominantColorLabel;
        System::ComponentModel::Container^ components;
        System::Windows::Forms::Button^ button10;
        System::Windows::Forms::Button^ button11;
        System::Windows::Forms::Panel^ panelOrientation;
        System::Windows::Forms::Button^ btnRotate90CW;
        System::Windows::Forms::Button^ btnRotate90CCW;
        System::Windows::Forms::Button^ btnRotate180;
        System::Windows::Forms::Button^ btnFlipHorizontal;
        System::Windows::Forms::Button^ btnFlipVertical;
        System::Windows::Forms::Button^ button12;
        System::Windows::Forms::Panel^ panelProjection;
        System::Windows::Forms::Button^ btnHorizontalProjection;
        System::Windows::Forms::Button^ btnVerticalProjection;
        System::Windows::Forms::Button^ button13;
        System::Windows::Forms::Timer^ debounceTimer;
        System::Windows::Forms::Button^ btnTranslate; 
        System::Windows::Forms::Panel^ panelConvolution;
        System::Windows::Forms::Button^ btnSmoothing;
        System::Windows::Forms::Button^ btnGaussianBlur;
        System::Windows::Forms::Button^ btnSharpen;
        System::Windows::Forms::Button^ btnMeanRemoval;
        System::Windows::Forms::Button^ btnEmboss;
        System::Windows::Forms::Button^ btnManualMath;
        Color lastFilterColor;
        bool isThresholdActive;
        bool removeBackground;
        System::Collections::Generic::Stack<ImageState^>^ undoStack;
        array<int>^ redHistogram;
        array<int>^ greenHistogram;
    private: System::Windows::Forms::Button^ button4;
private: System::Windows::Forms::Button^ button14;
       array<int>^ blueHistogram;

#pragma region Windows Form Designer generated code
           void InitializeComponent(void)
           {
               System::ComponentModel::ComponentResourceManager^ resources = (gcnew System::ComponentModel::ComponentResourceManager(MainForm::typeid));
               this->button1 = (gcnew System::Windows::Forms::Button());
               this->pictureBox1 = (gcnew System::Windows::Forms::PictureBox());
               this->pictureBox2 = (gcnew System::Windows::Forms::PictureBox());
               this->button2 = (gcnew System::Windows::Forms::Button());
               this->label2 = (gcnew System::Windows::Forms::Label());
               this->label3 = (gcnew System::Windows::Forms::Label());
               this->button3 = (gcnew System::Windows::Forms::Button());
               this->trackBar1 = (gcnew System::Windows::Forms::TrackBar());
               this->trackBar2 = (gcnew System::Windows::Forms::TrackBar());
               this->trackBar3 = (gcnew System::Windows::Forms::TrackBar());
               this->label4 = (gcnew System::Windows::Forms::Label());
               this->label5 = (gcnew System::Windows::Forms::Label());
               this->label6 = (gcnew System::Windows::Forms::Label());
               this->button5 = (gcnew System::Windows::Forms::Button());
               this->groupBoxControls = (gcnew System::Windows::Forms::GroupBox());
               this->button14 = (gcnew System::Windows::Forms::Button());
               this->button12 = (gcnew System::Windows::Forms::Button());
               this->button11 = (gcnew System::Windows::Forms::Button());
               this->btnThreshold = (gcnew System::Windows::Forms::Button());
               this->btnRGB = (gcnew System::Windows::Forms::Button());
               this->btnEnhancements = (gcnew System::Windows::Forms::Button());
               this->btnFilters = (gcnew System::Windows::Forms::Button());
               this->panelRGB = (gcnew System::Windows::Forms::Panel());
               this->panelThreshold = (gcnew System::Windows::Forms::Panel());
               this->trackBar9 = (gcnew System::Windows::Forms::TrackBar());
               this->trackBar10 = (gcnew System::Windows::Forms::TrackBar());
               this->trackBar11 = (gcnew System::Windows::Forms::TrackBar());
               this->label12 = (gcnew System::Windows::Forms::Label());
               this->thresholdValueLabel = (gcnew System::Windows::Forms::Label());
               this->blackValueLabel = (gcnew System::Windows::Forms::Label());
               this->whiteValueLabel = (gcnew System::Windows::Forms::Label());
               this->panelEnhancements = (gcnew System::Windows::Forms::Panel());
               this->trackBar4 = (gcnew System::Windows::Forms::TrackBar());
               this->trackBar5 = (gcnew System::Windows::Forms::TrackBar());
               this->trackBar6 = (gcnew System::Windows::Forms::TrackBar());
               this->trackBar7 = (gcnew System::Windows::Forms::TrackBar());
               this->trackBar8 = (gcnew System::Windows::Forms::TrackBar());
               this->label7 = (gcnew System::Windows::Forms::Label());
               this->label8 = (gcnew System::Windows::Forms::Label());
               this->label9 = (gcnew System::Windows::Forms::Label());
               this->label10 = (gcnew System::Windows::Forms::Label());
               this->label11 = (gcnew System::Windows::Forms::Label());
               this->panelFilters = (gcnew System::Windows::Forms::Panel());
               this->btnBlueGreen = (gcnew System::Windows::Forms::Button());
               this->btnViolet = (gcnew System::Windows::Forms::Button());
               this->btnBrown = (gcnew System::Windows::Forms::Button());
               this->btnKhaki = (gcnew System::Windows::Forms::Button());
               this->btnOrange = (gcnew System::Windows::Forms::Button());
               this->btnCyan = (gcnew System::Windows::Forms::Button());
               this->btnPink = (gcnew System::Windows::Forms::Button());
               this->btnTeal = (gcnew System::Windows::Forms::Button());
               this->btnLime = (gcnew System::Windows::Forms::Button());
               this->btnMagenta = (gcnew System::Windows::Forms::Button());
               this->panelOrientation = (gcnew System::Windows::Forms::Panel());
               this->btnRotate90CW = (gcnew System::Windows::Forms::Button());
               this->btnRotate90CCW = (gcnew System::Windows::Forms::Button());
               this->btnRotate180 = (gcnew System::Windows::Forms::Button());
               this->btnFlipHorizontal = (gcnew System::Windows::Forms::Button());
               this->btnFlipVertical = (gcnew System::Windows::Forms::Button());
               this->btnTranslate = (gcnew System::Windows::Forms::Button());
               this->panelProjection = (gcnew System::Windows::Forms::Panel());
               this->btnHorizontalProjection = (gcnew System::Windows::Forms::Button());
               this->btnVerticalProjection = (gcnew System::Windows::Forms::Button());
               this->panelConvolution = (gcnew System::Windows::Forms::Panel());
               this->btnSmoothing = (gcnew System::Windows::Forms::Button());
               this->btnGaussianBlur = (gcnew System::Windows::Forms::Button());
               this->btnSharpen = (gcnew System::Windows::Forms::Button());
               this->btnMeanRemoval = (gcnew System::Windows::Forms::Button());
               this->btnEmboss = (gcnew System::Windows::Forms::Button());
               this->btnManualMath = (gcnew System::Windows::Forms::Button());
               this->button7 = (gcnew System::Windows::Forms::Button());
               this->button8 = (gcnew System::Windows::Forms::Button());
               this->button9 = (gcnew System::Windows::Forms::Button());
               this->histogramBox = (gcnew System::Windows::Forms::PictureBox());
               this->dominantColorLabel = (gcnew System::Windows::Forms::Label());
               this->colorClustersLabel = (gcnew System::Windows::Forms::Label());
               this->button6 = (gcnew System::Windows::Forms::Button());
               this->button10 = (gcnew System::Windows::Forms::Button());
               this->button13 = (gcnew System::Windows::Forms::Button());
               this->button4 = (gcnew System::Windows::Forms::Button());
               (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->pictureBox1))->BeginInit();
               (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->pictureBox2))->BeginInit();
               (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->trackBar1))->BeginInit();
               (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->trackBar2))->BeginInit();
               (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->trackBar3))->BeginInit();
               this->groupBoxControls->SuspendLayout();
               this->panelRGB->SuspendLayout();
               this->panelThreshold->SuspendLayout();
               (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->trackBar9))->BeginInit();
               (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->trackBar10))->BeginInit();
               (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->trackBar11))->BeginInit();
               this->panelEnhancements->SuspendLayout();
               (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->trackBar4))->BeginInit();
               (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->trackBar5))->BeginInit();
               (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->trackBar6))->BeginInit();
               (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->trackBar7))->BeginInit();
               (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->trackBar8))->BeginInit();
               this->panelFilters->SuspendLayout();
               this->panelOrientation->SuspendLayout();
               this->panelProjection->SuspendLayout();
               this->panelConvolution->SuspendLayout();
               (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->histogramBox))->BeginInit();
               this->SuspendLayout();
               // 
               // button1
               // 
               this->button1->BackColor = System::Drawing::SystemColors::ButtonShadow;
               this->button1->ForeColor = System::Drawing::Color::Black;
               this->button1->Location = System::Drawing::Point(12, 12);
               this->button1->Name = L"button1";
               this->button1->Size = System::Drawing::Size(75, 23);
               this->button1->TabIndex = 0;
               this->button1->Text = L"Load Image";
               this->button1->UseVisualStyleBackColor = false;
               this->button1->Click += gcnew System::EventHandler(this, &MainForm::button1_Click);
               // 
               // pictureBox1
               // 
               this->pictureBox1->BackColor = System::Drawing::SystemColors::ButtonShadow;
               this->pictureBox1->Location = System::Drawing::Point(12, 41);
               this->pictureBox1->Name = L"pictureBox1";
               this->pictureBox1->Size = System::Drawing::Size(600, 400);
               this->pictureBox1->SizeMode = System::Windows::Forms::PictureBoxSizeMode::Zoom;
               this->pictureBox1->TabIndex = 1;
               this->pictureBox1->TabStop = false;
               this->pictureBox1->Click += gcnew System::EventHandler(this, &MainForm::pictureBox1_Click);
               // 
               // pictureBox2
               // 
               this->pictureBox2->BackColor = System::Drawing::SystemColors::ButtonShadow;
               this->pictureBox2->Location = System::Drawing::Point(621, 41);
               this->pictureBox2->Name = L"pictureBox2";
               this->pictureBox2->Size = System::Drawing::Size(600, 400);
               this->pictureBox2->SizeMode = System::Windows::Forms::PictureBoxSizeMode::Zoom;
               this->pictureBox2->TabIndex = 2;
               this->pictureBox2->TabStop = false;
               // 
               // button2
               // 
               this->button2->BackColor = System::Drawing::SystemColors::ButtonShadow;
               this->button2->ForeColor = System::Drawing::Color::Black;
               this->button2->Location = System::Drawing::Point(621, 12);
               this->button2->Name = L"button2";
               this->button2->Size = System::Drawing::Size(75, 23);
               this->button2->TabIndex = 3;
               this->button2->Text = L"Grayscale";
               this->button2->UseVisualStyleBackColor = false;
               this->button2->Click += gcnew System::EventHandler(this, &MainForm::button2_Click);
               // 
               // label2
               // 
               this->label2->AutoSize = true;
               this->label2->Location = System::Drawing::Point(12, 444);
               this->label2->Name = L"label2";
               this->label2->Size = System::Drawing::Size(74, 13);
               this->label2->TabIndex = 4;
               this->label2->Text = L"Original Image";
               this->label2->Click += gcnew System::EventHandler(this, &MainForm::label2_Click);
               // 
               // label3
               // 
               this->label3->AutoSize = true;
               this->label3->Location = System::Drawing::Point(618, 444);
               this->label3->Name = L"label3";
               this->label3->Size = System::Drawing::Size(69, 13);
               this->label3->TabIndex = 5;
               this->label3->Text = L"Edited Image";
               // 
               // button3
               // 
               this->button3->BackColor = System::Drawing::SystemColors::ButtonShadow;
               this->button3->ForeColor = System::Drawing::Color::Black;
               this->button3->Location = System::Drawing::Point(93, 12);
               this->button3->Name = L"button3";
               this->button3->Size = System::Drawing::Size(75, 23);
               this->button3->TabIndex = 6;
               this->button3->Text = L"Save Image";
               this->button3->UseVisualStyleBackColor = false;
               this->button3->Click += gcnew System::EventHandler(this, &MainForm::button3_Click);
               // 
               // trackBar1
               // 
               this->trackBar1->Location = System::Drawing::Point(6, 19);
               this->trackBar1->Name = L"trackBar1";
               this->trackBar1->Size = System::Drawing::Size(200, 45);
               this->trackBar1->TabIndex = 8;
               // 
               // trackBar2
               // 
               this->trackBar2->Location = System::Drawing::Point(6, 64);
               this->trackBar2->Name = L"trackBar2";
               this->trackBar2->Size = System::Drawing::Size(200, 45);
               this->trackBar2->TabIndex = 9;
               // 
               // trackBar3
               // 
               this->trackBar3->Location = System::Drawing::Point(6, 109);
               this->trackBar3->Name = L"trackBar3";
               this->trackBar3->Size = System::Drawing::Size(200, 45);
               this->trackBar3->TabIndex = 10;
               // 
               // label4
               // 
               this->label4->AutoSize = true;
               this->label4->Location = System::Drawing::Point(212, 29);
               this->label4->Name = L"label4";
               this->label4->Size = System::Drawing::Size(27, 13);
               this->label4->TabIndex = 11;
               this->label4->Text = L"Red";
               // 
               // label5
               // 
               this->label5->AutoSize = true;
               this->label5->Location = System::Drawing::Point(212, 74);
               this->label5->Name = L"label5";
               this->label5->Size = System::Drawing::Size(36, 13);
               this->label5->TabIndex = 12;
               this->label5->Text = L"Green";
               // 
               // label6
               // 
               this->label6->AutoSize = true;
               this->label6->Location = System::Drawing::Point(212, 119);
               this->label6->Name = L"label6";
               this->label6->Size = System::Drawing::Size(28, 13);
               this->label6->TabIndex = 13;
               this->label6->Text = L"Blue";
               // 
               // button5
               // 
               this->button5->BackColor = System::Drawing::SystemColors::ButtonShadow;
               this->button5->ForeColor = System::Drawing::Color::Black;
               this->button5->Location = System::Drawing::Point(255, 12);
               this->button5->Name = L"button5";
               this->button5->Size = System::Drawing::Size(75, 23);
               this->button5->TabIndex = 14;
               this->button5->Text = L"Undo";
               this->button5->UseVisualStyleBackColor = false;
               this->button5->Click += gcnew System::EventHandler(this, &MainForm::button5_Click);
               // 
               // groupBoxControls
               // 
               this->groupBoxControls->Controls->Add(this->button14);
               this->groupBoxControls->Controls->Add(this->button12);
               this->groupBoxControls->Controls->Add(this->button11);
               this->groupBoxControls->Controls->Add(this->btnThreshold);
               this->groupBoxControls->Controls->Add(this->btnRGB);
               this->groupBoxControls->Controls->Add(this->btnEnhancements);
               this->groupBoxControls->Controls->Add(this->btnFilters);
               this->groupBoxControls->Controls->Add(this->panelRGB);
               this->groupBoxControls->Controls->Add(this->panelThreshold);
               this->groupBoxControls->Controls->Add(this->panelEnhancements);
               this->groupBoxControls->Controls->Add(this->panelFilters);
               this->groupBoxControls->Controls->Add(this->panelOrientation);
               this->groupBoxControls->Controls->Add(this->panelProjection);
               this->groupBoxControls->Controls->Add(this->panelConvolution);
               this->groupBoxControls->Location = System::Drawing::Point(12, 460);
               this->groupBoxControls->Name = L"groupBoxControls";
               this->groupBoxControls->Size = System::Drawing::Size(600, 303);
               this->groupBoxControls->TabIndex = 15;
               this->groupBoxControls->TabStop = false;
               this->groupBoxControls->Text = L"Controls";
               // 
               // button14
               // 
               this->button14->BackColor = System::Drawing::SystemColors::ButtonShadow;
               this->button14->Location = System::Drawing::Point(500, 19);
               this->button14->Name = L"button14";
               this->button14->Size = System::Drawing::Size(94, 23);
               this->button14->TabIndex = 23;
               this->button14->Text = L"Convolution flter";
               this->button14->UseVisualStyleBackColor = false;
               this->button14->Click += gcnew System::EventHandler(this, &MainForm::button14_Click);
               // 
               // button12
               // 
               this->button12->BackColor = System::Drawing::SystemColors::ButtonShadow;
               this->button12->Location = System::Drawing::Point(341, 19);
               this->button12->Name = L"button12";
               this->button12->Size = System::Drawing::Size(75, 23);
               this->button12->TabIndex = 22;
               this->button12->Text = L"Projection";
               this->button12->UseVisualStyleBackColor = false;
               this->button12->Click += gcnew System::EventHandler(this, &MainForm::button12_Click);
               // 
               // button11
               // 
               this->button11->BackColor = System::Drawing::SystemColors::ButtonShadow;
               this->button11->Location = System::Drawing::Point(260, 19);
               this->button11->Name = L"button11";
               this->button11->Size = System::Drawing::Size(75, 23);
               this->button11->TabIndex = 21;
               this->button11->Text = L"Orientation";
               this->button11->UseVisualStyleBackColor = false;
               this->button11->Click += gcnew System::EventHandler(this, &MainForm::button11_Click);
               // 
               // btnThreshold
               // 
               this->btnThreshold->BackColor = System::Drawing::SystemColors::ButtonShadow;
               this->btnThreshold->Location = System::Drawing::Point(87, 19);
               this->btnThreshold->Name = L"btnThreshold";
               this->btnThreshold->Size = System::Drawing::Size(75, 23);
               this->btnThreshold->TabIndex = 20;
               this->btnThreshold->Text = L"Threshold";
               this->btnThreshold->UseVisualStyleBackColor = false;
               this->btnThreshold->Click += gcnew System::EventHandler(this, &MainForm::btnThreshold_Click);
               // 
               // btnRGB
               // 
               this->btnRGB->BackColor = System::Drawing::SystemColors::ButtonShadow;
               this->btnRGB->Location = System::Drawing::Point(6, 19);
               this->btnRGB->Name = L"btnRGB";
               this->btnRGB->Size = System::Drawing::Size(75, 23);
               this->btnRGB->TabIndex = 19;
               this->btnRGB->Text = L"RGB";
               this->btnRGB->UseVisualStyleBackColor = false;
               this->btnRGB->Click += gcnew System::EventHandler(this, &MainForm::btnRGB_Click);
               // 
               // btnEnhancements
               // 
               this->btnEnhancements->BackColor = System::Drawing::SystemColors::ButtonShadow;
               this->btnEnhancements->Location = System::Drawing::Point(168, 19);
               this->btnEnhancements->Name = L"btnEnhancements";
               this->btnEnhancements->Size = System::Drawing::Size(86, 23);
               this->btnEnhancements->TabIndex = 18;
               this->btnEnhancements->Text = L"Enhancements";
               this->btnEnhancements->UseVisualStyleBackColor = false;
               this->btnEnhancements->Click += gcnew System::EventHandler(this, &MainForm::btnEnhancements_Click);
               // 
               // btnFilters
               // 
               this->btnFilters->BackColor = System::Drawing::SystemColors::ButtonShadow;
               this->btnFilters->Location = System::Drawing::Point(422, 19);
               this->btnFilters->Name = L"btnFilters";
               this->btnFilters->Size = System::Drawing::Size(75, 23);
               this->btnFilters->TabIndex = 17;
               this->btnFilters->Text = L"Filters";
               this->btnFilters->UseVisualStyleBackColor = false;
               this->btnFilters->Click += gcnew System::EventHandler(this, &MainForm::btnFilters_Click);
               // 
               // panelRGB
               // 
               this->panelRGB->Controls->Add(this->trackBar1);
               this->panelRGB->Controls->Add(this->trackBar2);
               this->panelRGB->Controls->Add(this->trackBar3);
               this->panelRGB->Controls->Add(this->label4);
               this->panelRGB->Controls->Add(this->label5);
               this->panelRGB->Controls->Add(this->label6);
               this->panelRGB->Location = System::Drawing::Point(6, 45);
               this->panelRGB->Name = L"panelRGB";
               this->panelRGB->Size = System::Drawing::Size(580, 244);
               this->panelRGB->TabIndex = 16;
               this->panelRGB->Paint += gcnew System::Windows::Forms::PaintEventHandler(this, &MainForm::panelRGB_Paint);
               // 
               // panelThreshold
               // 
               this->panelThreshold->Controls->Add(this->trackBar9);
               this->panelThreshold->Controls->Add(this->trackBar10);
               this->panelThreshold->Controls->Add(this->trackBar11);
               this->panelThreshold->Controls->Add(this->label12);
               this->panelThreshold->Controls->Add(this->thresholdValueLabel);
               this->panelThreshold->Controls->Add(this->blackValueLabel);
               this->panelThreshold->Controls->Add(this->whiteValueLabel);
               this->panelThreshold->Location = System::Drawing::Point(20, 45);
               this->panelThreshold->Name = L"panelThreshold";
               this->panelThreshold->Size = System::Drawing::Size(566, 244);
               this->panelThreshold->TabIndex = 15;
               this->panelThreshold->Visible = false;
               // 
               // trackBar9
               // 
               this->trackBar9->Location = System::Drawing::Point(6, 19);
               this->trackBar9->Name = L"trackBar9";
               this->trackBar9->Size = System::Drawing::Size(200, 45);
               this->trackBar9->TabIndex = 8;
               // 
               // trackBar10
               // 
               this->trackBar10->Location = System::Drawing::Point(6, 64);
               this->trackBar10->Name = L"trackBar10";
               this->trackBar10->Size = System::Drawing::Size(200, 45);
               this->trackBar10->TabIndex = 9;
               // 
               // trackBar11
               // 
               this->trackBar11->Location = System::Drawing::Point(6, 109);
               this->trackBar11->Name = L"trackBar11";
               this->trackBar11->Size = System::Drawing::Size(200, 45);
               this->trackBar11->TabIndex = 10;
               // 
               // label12
               // 
               this->label12->AutoSize = true;
               this->label12->Location = System::Drawing::Point(212, 29);
               this->label12->Name = L"label12";
               this->label12->Size = System::Drawing::Size(54, 13);
               this->label12->TabIndex = 11;
               this->label12->Text = L"Threshold";
               // 
               // thresholdValueLabel
               // 
               this->thresholdValueLabel->AutoSize = true;
               this->thresholdValueLabel->Location = System::Drawing::Point(212, 49);
               this->thresholdValueLabel->Name = L"thresholdValueLabel";
               this->thresholdValueLabel->Size = System::Drawing::Size(46, 13);
               this->thresholdValueLabel->TabIndex = 12;
               this->thresholdValueLabel->Text = L"Value: 0";
               // 
               // blackValueLabel
               // 
               this->blackValueLabel->AutoSize = true;
               this->blackValueLabel->Location = System::Drawing::Point(212, 94);
               this->blackValueLabel->Name = L"blackValueLabel";
               this->blackValueLabel->Size = System::Drawing::Size(46, 13);
               this->blackValueLabel->TabIndex = 13;
               this->blackValueLabel->Text = L"Black: 0";
               // 
               // whiteValueLabel
               // 
               this->whiteValueLabel->AutoSize = true;
               this->whiteValueLabel->Location = System::Drawing::Point(212, 139);
               this->whiteValueLabel->Name = L"whiteValueLabel";
               this->whiteValueLabel->Size = System::Drawing::Size(47, 13);
               this->whiteValueLabel->TabIndex = 14;
               this->whiteValueLabel->Text = L"White: 0";
               // 
               // panelEnhancements
               // 
               this->panelEnhancements->Controls->Add(this->trackBar4);
               this->panelEnhancements->Controls->Add(this->trackBar5);
               this->panelEnhancements->Controls->Add(this->trackBar6);
               this->panelEnhancements->Controls->Add(this->trackBar7);
               this->panelEnhancements->Controls->Add(this->trackBar8);
               this->panelEnhancements->Controls->Add(this->label7);
               this->panelEnhancements->Controls->Add(this->label8);
               this->panelEnhancements->Controls->Add(this->label9);
               this->panelEnhancements->Controls->Add(this->label10);
               this->panelEnhancements->Controls->Add(this->label11);
               this->panelEnhancements->Location = System::Drawing::Point(20, 48);
               this->panelEnhancements->Name = L"panelEnhancements";
               this->panelEnhancements->Size = System::Drawing::Size(566, 244);
               this->panelEnhancements->TabIndex = 14;
               this->panelEnhancements->Visible = false;
               // 
               // trackBar4
               // 
               this->trackBar4->Location = System::Drawing::Point(6, 19);
               this->trackBar4->Name = L"trackBar4";
               this->trackBar4->Size = System::Drawing::Size(200, 45);
               this->trackBar4->TabIndex = 8;
               // 
               // trackBar5
               // 
               this->trackBar5->Location = System::Drawing::Point(6, 64);
               this->trackBar5->Name = L"trackBar5";
               this->trackBar5->Size = System::Drawing::Size(200, 45);
               this->trackBar5->TabIndex = 9;
               // 
               // trackBar6
               // 
               this->trackBar6->Location = System::Drawing::Point(6, 109);
               this->trackBar6->Name = L"trackBar6";
               this->trackBar6->Size = System::Drawing::Size(200, 45);
               this->trackBar6->TabIndex = 10;
               // 
               // trackBar7
               // 
               this->trackBar7->Location = System::Drawing::Point(6, 154);
               this->trackBar7->Name = L"trackBar7";
               this->trackBar7->Size = System::Drawing::Size(200, 45);
               this->trackBar7->TabIndex = 11;
               // 
               // trackBar8
               // 
               this->trackBar8->Location = System::Drawing::Point(6, 199);
               this->trackBar8->Name = L"trackBar8";
               this->trackBar8->Size = System::Drawing::Size(200, 45);
               this->trackBar8->TabIndex = 12;
               // 
               // label7
               // 
               this->label7->AutoSize = true;
               this->label7->Location = System::Drawing::Point(212, 29);
               this->label7->Name = L"label7";
               this->label7->Size = System::Drawing::Size(56, 13);
               this->label7->TabIndex = 13;
               this->label7->Text = L"Brightness";
               // 
               // label8
               // 
               this->label8->AutoSize = true;
               this->label8->Location = System::Drawing::Point(212, 74);
               this->label8->Name = L"label8";
               this->label8->Size = System::Drawing::Size(46, 13);
               this->label8->TabIndex = 14;
               this->label8->Text = L"Contrast";
               // 
               // label9
               // 
               this->label9->AutoSize = true;
               this->label9->Location = System::Drawing::Point(212, 119);
               this->label9->Name = L"label9";
               this->label9->Size = System::Drawing::Size(55, 13);
               this->label9->TabIndex = 15;
               this->label9->Text = L"Saturation";
               // 
               // label10
               // 
               this->label10->AutoSize = true;
               this->label10->Location = System::Drawing::Point(212, 164);
               this->label10->Name = L"label10";
               this->label10->Size = System::Drawing::Size(43, 13);
               this->label10->TabIndex = 16;
               this->label10->Text = L"Opacity";
               // 
               // label11
               // 
               this->label11->AutoSize = true;
               this->label11->Location = System::Drawing::Point(212, 209);
               this->label11->Name = L"label11";
               this->label11->Size = System::Drawing::Size(25, 13);
               this->label11->TabIndex = 17;
               this->label11->Text = L"Blur";
               // 
               // panelFilters
               // 
               this->panelFilters->Controls->Add(this->btnBlueGreen);
               this->panelFilters->Controls->Add(this->btnViolet);
               this->panelFilters->Controls->Add(this->btnBrown);
               this->panelFilters->Controls->Add(this->btnKhaki);
               this->panelFilters->Controls->Add(this->btnOrange);
               this->panelFilters->Controls->Add(this->btnCyan);
               this->panelFilters->Controls->Add(this->btnPink);
               this->panelFilters->Controls->Add(this->btnTeal);
               this->panelFilters->Controls->Add(this->btnLime);
               this->panelFilters->Controls->Add(this->btnMagenta);
               this->panelFilters->Location = System::Drawing::Point(20, 48);
               this->panelFilters->Name = L"panelFilters";
               this->panelFilters->Size = System::Drawing::Size(250, 160);
               this->panelFilters->TabIndex = 13;
               this->panelFilters->Visible = false;
               // 
               // btnBlueGreen
               // 
               this->btnBlueGreen->Location = System::Drawing::Point(6, 19);
               this->btnBlueGreen->Name = L"btnBlueGreen";
               this->btnBlueGreen->Size = System::Drawing::Size(75, 23);
               this->btnBlueGreen->TabIndex = 0;
               this->btnBlueGreen->Text = L"BlueGreen";
               this->btnBlueGreen->UseVisualStyleBackColor = true;
               this->btnBlueGreen->Click += gcnew System::EventHandler(this, &MainForm::btnBlueGreen_Click);
               // 
               // btnViolet
               // 
               this->btnViolet->Location = System::Drawing::Point(87, 19);
               this->btnViolet->Name = L"btnViolet";
               this->btnViolet->Size = System::Drawing::Size(75, 23);
               this->btnViolet->TabIndex = 1;
               this->btnViolet->Text = L"Violet";
               this->btnViolet->UseVisualStyleBackColor = true;
               this->btnViolet->Click += gcnew System::EventHandler(this, &MainForm::btnViolet_Click);
               // 
               // btnBrown
               // 
               this->btnBrown->Location = System::Drawing::Point(168, 19);
               this->btnBrown->Name = L"btnBrown";
               this->btnBrown->Size = System::Drawing::Size(75, 23);
               this->btnBrown->TabIndex = 2;
               this->btnBrown->Text = L"Brown";
               this->btnBrown->UseVisualStyleBackColor = true;
               this->btnBrown->Click += gcnew System::EventHandler(this, &MainForm::btnBrown_Click);
               // 
               // btnKhaki
               // 
               this->btnKhaki->Location = System::Drawing::Point(6, 48);
               this->btnKhaki->Name = L"btnKhaki";
               this->btnKhaki->Size = System::Drawing::Size(75, 23);
               this->btnKhaki->TabIndex = 3;
               this->btnKhaki->Text = L"Khaki";
               this->btnKhaki->UseVisualStyleBackColor = true;
               this->btnKhaki->Click += gcnew System::EventHandler(this, &MainForm::btnKhaki_Click);
               // 
               // btnOrange
               // 
               this->btnOrange->Location = System::Drawing::Point(87, 48);
               this->btnOrange->Name = L"btnOrange";
               this->btnOrange->Size = System::Drawing::Size(75, 23);
               this->btnOrange->TabIndex = 4;
               this->btnOrange->Text = L"Orange";
               this->btnOrange->UseVisualStyleBackColor = true;
               this->btnOrange->Click += gcnew System::EventHandler(this, &MainForm::btnOrange_Click);
               // 
               // btnCyan
               // 
               this->btnCyan->Location = System::Drawing::Point(168, 48);
               this->btnCyan->Name = L"btnCyan";
               this->btnCyan->Size = System::Drawing::Size(75, 23);
               this->btnCyan->TabIndex = 5;
               this->btnCyan->Text = L"Cyan";
               this->btnCyan->UseVisualStyleBackColor = true;
               this->btnCyan->Click += gcnew System::EventHandler(this, &MainForm::btnCyan_Click);
               // 
               // btnPink
               // 
               this->btnPink->Location = System::Drawing::Point(6, 77);
               this->btnPink->Name = L"btnPink";
               this->btnPink->Size = System::Drawing::Size(75, 23);
               this->btnPink->TabIndex = 6;
               this->btnPink->Text = L"Pink";
               this->btnPink->UseVisualStyleBackColor = true;
               this->btnPink->Click += gcnew System::EventHandler(this, &MainForm::btnPink_Click);
               // 
               // btnTeal
               // 
               this->btnTeal->Location = System::Drawing::Point(87, 77);
               this->btnTeal->Name = L"btnTeal";
               this->btnTeal->Size = System::Drawing::Size(75, 23);
               this->btnTeal->TabIndex = 7;
               this->btnTeal->Text = L"Teal";
               this->btnTeal->UseVisualStyleBackColor = true;
               this->btnTeal->Click += gcnew System::EventHandler(this, &MainForm::btnTeal_Click);
               // 
               // btnLime
               // 
               this->btnLime->Location = System::Drawing::Point(168, 77);
               this->btnLime->Name = L"btnLime";
               this->btnLime->Size = System::Drawing::Size(75, 23);
               this->btnLime->TabIndex = 8;
               this->btnLime->Text = L"Lime";
               this->btnLime->UseVisualStyleBackColor = true;
               this->btnLime->Click += gcnew System::EventHandler(this, &MainForm::btnLime_Click);
               // 
               // btnMagenta
               // 
               this->btnMagenta->Location = System::Drawing::Point(6, 106);
               this->btnMagenta->Name = L"btnMagenta";
               this->btnMagenta->Size = System::Drawing::Size(75, 23);
               this->btnMagenta->TabIndex = 9;
               this->btnMagenta->Text = L"Magenta";
               this->btnMagenta->UseVisualStyleBackColor = true;
               this->btnMagenta->Click += gcnew System::EventHandler(this, &MainForm::btnMagenta_Click);
               // 
               // panelOrientation
               // 
               this->panelOrientation->Controls->Add(this->btnRotate90CW);
               this->panelOrientation->Controls->Add(this->btnRotate90CCW);
               this->panelOrientation->Controls->Add(this->btnRotate180);
               this->panelOrientation->Controls->Add(this->btnFlipHorizontal);
               this->panelOrientation->Controls->Add(this->btnFlipVertical);
               this->panelOrientation->Controls->Add(this->btnTranslate);
               this->panelOrientation->Location = System::Drawing::Point(20, 48);
               this->panelOrientation->Name = L"panelOrientation";
               this->panelOrientation->Size = System::Drawing::Size(250, 160);
               this->panelOrientation->TabIndex = 12;
               this->panelOrientation->Visible = false;
               // 
               // btnRotate90CW
               // 
               this->btnRotate90CW->Location = System::Drawing::Point(6, 19);
               this->btnRotate90CW->Name = L"btnRotate90CW";
               this->btnRotate90CW->Size = System::Drawing::Size(75, 23);
               this->btnRotate90CW->TabIndex = 0;
               this->btnRotate90CW->Text = L"Rotate 90 CW";
               this->btnRotate90CW->UseVisualStyleBackColor = true;
               this->btnRotate90CW->Click += gcnew System::EventHandler(this, &MainForm::btnRotate90CW_Click);
               // 
               // btnRotate90CCW
               // 
               this->btnRotate90CCW->Location = System::Drawing::Point(87, 19);
               this->btnRotate90CCW->Name = L"btnRotate90CCW";
               this->btnRotate90CCW->Size = System::Drawing::Size(75, 23);
               this->btnRotate90CCW->TabIndex = 1;
               this->btnRotate90CCW->Text = L"Rotate 90 CCW";
               this->btnRotate90CCW->UseVisualStyleBackColor = true;
               this->btnRotate90CCW->Click += gcnew System::EventHandler(this, &MainForm::btnRotate90CCW_Click);
               // 
               // btnRotate180
               // 
               this->btnRotate180->Location = System::Drawing::Point(168, 19);
               this->btnRotate180->Name = L"btnRotate180";
               this->btnRotate180->Size = System::Drawing::Size(75, 23);
               this->btnRotate180->TabIndex = 2;
               this->btnRotate180->Text = L"Rotate 180";
               this->btnRotate180->UseVisualStyleBackColor = true;
               this->btnRotate180->Click += gcnew System::EventHandler(this, &MainForm::btnRotate180_Click);
               // 
               // btnFlipHorizontal
               // 
               this->btnFlipHorizontal->Location = System::Drawing::Point(6, 48);
               this->btnFlipHorizontal->Name = L"btnFlipHorizontal";
               this->btnFlipHorizontal->Size = System::Drawing::Size(75, 23);
               this->btnFlipHorizontal->TabIndex = 3;
               this->btnFlipHorizontal->Text = L"Flip Horizontal";
               this->btnFlipHorizontal->UseVisualStyleBackColor = true;
               this->btnFlipHorizontal->Click += gcnew System::EventHandler(this, &MainForm::btnFlipHorizontal_Click);
               // 
               // btnFlipVertical
               // 
               this->btnFlipVertical->Location = System::Drawing::Point(87, 48);
               this->btnFlipVertical->Name = L"btnFlipVertical";
               this->btnFlipVertical->Size = System::Drawing::Size(75, 23);
               this->btnFlipVertical->TabIndex = 4;
               this->btnFlipVertical->Text = L"Flip Vertical";
               this->btnFlipVertical->UseVisualStyleBackColor = true;
               this->btnFlipVertical->Click += gcnew System::EventHandler(this, &MainForm::btnFlipVertical_Click);
               // 
               // btnTranslate
               // 
               this->btnTranslate->Location = System::Drawing::Point(168, 48);
               this->btnTranslate->Name = L"btnTranslate";
               this->btnTranslate->Size = System::Drawing::Size(75, 23);
               this->btnTranslate->TabIndex = 5;
               this->btnTranslate->Text = L"Translate";
               this->btnTranslate->UseVisualStyleBackColor = true;
               this->btnTranslate->Click += gcnew System::EventHandler(this, &MainForm::btnTranslate_Click);
               // 
               // panelProjection
               // 
               this->panelProjection->Controls->Add(this->btnHorizontalProjection);
               this->panelProjection->Controls->Add(this->btnVerticalProjection);
               this->panelProjection->Location = System::Drawing::Point(20, 48);
               this->panelProjection->Name = L"panelProjection";
               this->panelProjection->Size = System::Drawing::Size(250, 160);
               this->panelProjection->TabIndex = 11;
               this->panelProjection->Visible = false;
               // 
               // btnHorizontalProjection
               // 
               this->btnHorizontalProjection->Location = System::Drawing::Point(6, 19);
               this->btnHorizontalProjection->Name = L"btnHorizontalProjection";
               this->btnHorizontalProjection->Size = System::Drawing::Size(100, 23);
               this->btnHorizontalProjection->TabIndex = 0;
               this->btnHorizontalProjection->Text = L"Horizontal Proj.";
               this->btnHorizontalProjection->UseVisualStyleBackColor = true;
               this->btnHorizontalProjection->Click += gcnew System::EventHandler(this, &MainForm::btnHorizontalProjection_Click);
               // 
               // btnVerticalProjection
               // 
               this->btnVerticalProjection->Location = System::Drawing::Point(112, 19);
               this->btnVerticalProjection->Name = L"btnVerticalProjection";
               this->btnVerticalProjection->Size = System::Drawing::Size(100, 23);
               this->btnVerticalProjection->TabIndex = 1;
               this->btnVerticalProjection->Text = L"Vertical Proj.";
               this->btnVerticalProjection->UseVisualStyleBackColor = true;
               this->btnVerticalProjection->Click += gcnew System::EventHandler(this, &MainForm::btnVerticalProjection_Click);
               // 
               // panelConvolution
               // 
               this->panelConvolution->Controls->Add(this->btnSmoothing);
               this->panelConvolution->Controls->Add(this->btnGaussianBlur);
               this->panelConvolution->Controls->Add(this->btnSharpen);
               this->panelConvolution->Controls->Add(this->btnMeanRemoval);
               this->panelConvolution->Controls->Add(this->btnEmboss);
               this->panelConvolution->Controls->Add(this->btnManualMath);
               this->panelConvolution->Location = System::Drawing::Point(20, 48);
               this->panelConvolution->Name = L"panelConvolution";
               this->panelConvolution->Size = System::Drawing::Size(250, 160);
               this->panelConvolution->TabIndex = 24;
               this->panelConvolution->Visible = false;
               // 
               // btnSmoothing
               // 
               this->btnSmoothing->Location = System::Drawing::Point(6, 19);
               this->btnSmoothing->Name = L"btnSmoothing";
               this->btnSmoothing->Size = System::Drawing::Size(75, 23);
               this->btnSmoothing->TabIndex = 0;
               this->btnSmoothing->Text = L"Smoothing";
               this->btnSmoothing->UseVisualStyleBackColor = true;
               this->btnSmoothing->Click += gcnew System::EventHandler(this, &MainForm::btnSmoothing_Click);
               // 
               // btnGaussianBlur
               // 
               this->btnGaussianBlur->Location = System::Drawing::Point(87, 19);
               this->btnGaussianBlur->Name = L"btnGaussianBlur";
               this->btnGaussianBlur->Size = System::Drawing::Size(75, 23);
               this->btnGaussianBlur->TabIndex = 1;
               this->btnGaussianBlur->Text = L"Gaussian Blur";
               this->btnGaussianBlur->UseVisualStyleBackColor = true;
               this->btnGaussianBlur->Click += gcnew System::EventHandler(this, &MainForm::btnGaussianBlur_Click);
               // 
               // btnSharpen
               // 
               this->btnSharpen->Location = System::Drawing::Point(168, 19);
               this->btnSharpen->Name = L"btnSharpen";
               this->btnSharpen->Size = System::Drawing::Size(75, 23);
               this->btnSharpen->TabIndex = 2;
               this->btnSharpen->Text = L"Sharpen";
               this->btnSharpen->UseVisualStyleBackColor = true;
               this->btnSharpen->Click += gcnew System::EventHandler(this, &MainForm::btnSharpen_Click);
               // 
               // btnMeanRemoval
               // 
               this->btnMeanRemoval->Location = System::Drawing::Point(6, 48);
               this->btnMeanRemoval->Name = L"btnMeanRemoval";
               this->btnMeanRemoval->Size = System::Drawing::Size(75, 23);
               this->btnMeanRemoval->TabIndex = 3;
               this->btnMeanRemoval->Text = L"Mean Removal";
               this->btnMeanRemoval->UseVisualStyleBackColor = true;
               this->btnMeanRemoval->Click += gcnew System::EventHandler(this, &MainForm::btnMeanRemoval_Click);
               // 
               // btnEmboss
               // 
               this->btnEmboss->Location = System::Drawing::Point(87, 48);
               this->btnEmboss->Name = L"btnEmboss";
               this->btnEmboss->Size = System::Drawing::Size(75, 23);
               this->btnEmboss->TabIndex = 4;
               this->btnEmboss->Text = L"Emboss";
               this->btnEmboss->UseVisualStyleBackColor = true;
               this->btnEmboss->Click += gcnew System::EventHandler(this, &MainForm::btnEmboss_Click);
               // 
               // btnManualMath
               // 
               this->btnManualMath->Location = System::Drawing::Point(168, 48);
               this->btnManualMath->Name = L"btnManualMath";
               this->btnManualMath->Size = System::Drawing::Size(75, 23);
               this->btnManualMath->TabIndex = 5;
               this->btnManualMath->Text = L"Manual Math";
               this->btnManualMath->UseVisualStyleBackColor = true;
               this->btnManualMath->Click += gcnew System::EventHandler(this, &MainForm::btnManualMath_Click);
               // 
               // button7
               // 
               this->button7->BackColor = System::Drawing::SystemColors::ButtonShadow;
               this->button7->ForeColor = System::Drawing::Color::Black;
               this->button7->Location = System::Drawing::Point(417, 12);
               this->button7->Name = L"button7";
               this->button7->Size = System::Drawing::Size(100, 23);
               this->button7->TabIndex = 16;
               this->button7->Text = L"Adaptive Thresh";
               this->button7->UseVisualStyleBackColor = false;
               this->button7->Click += gcnew System::EventHandler(this, &MainForm::btnAdaptive_Click);
               // 
               // button8
               // 
               this->button8->BackColor = System::Drawing::SystemColors::ButtonShadow;
               this->button8->ForeColor = System::Drawing::Color::Black;
               this->button8->Location = System::Drawing::Point(174, 12);
               this->button8->Name = L"button8";
               this->button8->Size = System::Drawing::Size(75, 23);
               this->button8->TabIndex = 17;
               this->button8->Text = L"Reset";
               this->button8->UseVisualStyleBackColor = false;
               this->button8->Click += gcnew System::EventHandler(this, &MainForm::button8_Click);
               // 
               // button9
               // 
               this->button9->BackColor = System::Drawing::SystemColors::ButtonShadow;
               this->button9->ForeColor = System::Drawing::Color::Black;
               this->button9->Location = System::Drawing::Point(702, 12);
               this->button9->Name = L"button9";
               this->button9->Size = System::Drawing::Size(100, 23);
               this->button9->TabIndex = 18;
               this->button9->Text = L"Remove BG";
               this->button9->UseVisualStyleBackColor = false;
               this->button9->Click += gcnew System::EventHandler(this, &MainForm::button9_Click);
               // 
               // histogramBox
               // 
               this->histogramBox->BackColor = System::Drawing::SystemColors::ButtonHighlight;
               this->histogramBox->Location = System::Drawing::Point(764, 460);
               this->histogramBox->Name = L"histogramBox";
               this->histogramBox->Size = System::Drawing::Size(286, 142);
               this->histogramBox->SizeMode = System::Windows::Forms::PictureBoxSizeMode::StretchImage;
               this->histogramBox->TabIndex = 19;
               this->histogramBox->TabStop = false;
               // 
               // dominantColorLabel
               // 
               this->dominantColorLabel->AutoSize = true;
               this->dominantColorLabel->Location = System::Drawing::Point(761, 605);
               this->dominantColorLabel->Name = L"dominantColorLabel";
               this->dominantColorLabel->Size = System::Drawing::Size(79, 13);
               this->dominantColorLabel->TabIndex = 20;
               this->dominantColorLabel->Text = L"Dominant Color";
               this->dominantColorLabel->Click += gcnew System::EventHandler(this, &MainForm::dominantColorLabel_Click);
               // 
               // colorClustersLabel
               // 
               this->colorClustersLabel->AutoSize = true;
               this->colorClustersLabel->Location = System::Drawing::Point(761, 621);
               this->colorClustersLabel->Name = L"colorClustersLabel";
               this->colorClustersLabel->Size = System::Drawing::Size(71, 13);
               this->colorClustersLabel->TabIndex = 21;
               this->colorClustersLabel->Text = L"Color Clusters";
               // 
               // button6
               // 
               this->button6->BackColor = System::Drawing::SystemColors::ButtonShadow;
               this->button6->ForeColor = System::Drawing::Color::Black;
               this->button6->Location = System::Drawing::Point(873, 12);
               this->button6->Name = L"button6";
               this->button6->Size = System::Drawing::Size(100, 23);
               this->button6->TabIndex = 22;
               this->button6->Text = L"Shape Detection";
               this->button6->UseVisualStyleBackColor = false;
               this->button6->Click += gcnew System::EventHandler(this, &MainForm::button6_Click);
               // 
               // button10
               // 
               this->button10->BackColor = System::Drawing::SystemColors::ActiveBorder;
               this->button10->Location = System::Drawing::Point(1073, 12);
               this->button10->Name = L"button10";
               this->button10->Size = System::Drawing::Size(80, 23);
               this->button10->TabIndex = 23;
               this->button10->Text = L"Segmentation";
               this->button10->UseVisualStyleBackColor = false;
               this->button10->Click += gcnew System::EventHandler(this, &MainForm::button10_Click);
               // 
               // button13
               // 
               this->button13->BackColor = System::Drawing::SystemColors::ActiveBorder;
               this->button13->Location = System::Drawing::Point(1159, 12);
               this->button13->Name = L"button13";
               this->button13->Size = System::Drawing::Size(75, 23);
               this->button13->TabIndex = 24;
               this->button13->Text = L"Convolution";
               this->button13->UseVisualStyleBackColor = false;
               this->button13->Click += gcnew System::EventHandler(this, &MainForm::button13_Click);
               // 
               // button4
               // 
               this->button4->BackColor = System::Drawing::SystemColors::ButtonShadow;
               this->button4->ForeColor = System::Drawing::Color::Black;
               this->button4->Location = System::Drawing::Point(523, 12);
               this->button4->Name = L"button4";
               this->button4->Size = System::Drawing::Size(75, 23);
               this->button4->TabIndex = 7;
               this->button4->Text = L"Binary";
               this->button4->UseVisualStyleBackColor = false;
               this->button4->Click += gcnew System::EventHandler(this, &MainForm::button4_Click);
               // 
               // MainForm
               // 
               this->AutoScaleDimensions = System::Drawing::SizeF(6, 13);
               this->AutoScaleMode = System::Windows::Forms::AutoScaleMode::Font;
               this->BackColor = System::Drawing::SystemColors::ControlDarkDark;
               this->ClientSize = System::Drawing::Size(1259, 786);
               this->Controls->Add(this->button13);
               this->Controls->Add(this->button10);
               this->Controls->Add(this->button6);
               this->Controls->Add(this->colorClustersLabel);
               this->Controls->Add(this->dominantColorLabel);
               this->Controls->Add(this->histogramBox);
               this->Controls->Add(this->button9);
               this->Controls->Add(this->button8);
               this->Controls->Add(this->button7);
               this->Controls->Add(this->groupBoxControls);
               this->Controls->Add(this->button5);
               this->Controls->Add(this->button4);
               this->Controls->Add(this->button3);
               this->Controls->Add(this->label3);
               this->Controls->Add(this->label2);
               this->Controls->Add(this->button2);
               this->Controls->Add(this->pictureBox2);
               this->Controls->Add(this->pictureBox1);
               this->Controls->Add(this->button1);
               this->Icon = (cli::safe_cast<System::Drawing::Icon^>(resources->GetObject(L"$this.Icon")));
               this->Name = L"MainForm";
               this->Text = L"IMAGE EDITOR";
               this->Load += gcnew System::EventHandler(this, &MainForm::MainForm_Load);
               (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->pictureBox1))->EndInit();
               (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->pictureBox2))->EndInit();
               (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->trackBar1))->EndInit();
               (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->trackBar2))->EndInit();
               (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->trackBar3))->EndInit();
               this->groupBoxControls->ResumeLayout(false);
               this->panelRGB->ResumeLayout(false);
               this->panelRGB->PerformLayout();
               this->panelThreshold->ResumeLayout(false);
               this->panelThreshold->PerformLayout();
               (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->trackBar9))->EndInit();
               (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->trackBar10))->EndInit();
               (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->trackBar11))->EndInit();
               this->panelEnhancements->ResumeLayout(false);
               this->panelEnhancements->PerformLayout();
               (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->trackBar4))->EndInit();
               (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->trackBar5))->EndInit();
               (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->trackBar6))->EndInit();
               (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->trackBar7))->EndInit();
               (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->trackBar8))->EndInit();
               this->panelFilters->ResumeLayout(false);
               this->panelOrientation->ResumeLayout(false);
               this->panelProjection->ResumeLayout(false);
               this->panelConvolution->ResumeLayout(false);
               (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->histogramBox))->EndInit();
               this->ResumeLayout(false);
               this->PerformLayout();

           }
#pragma endregion

    private:
        System::Void button1_Click(System::Object^ sender, System::EventArgs^ e) {
            OpenFileDialog^ openFileDialog = gcnew OpenFileDialog();
            openFileDialog->Filter = "Image Files|*.jpg;*.jpeg;*.png;*.bmp";
            if (openFileDialog->ShowDialog() == System::Windows::Forms::DialogResult::OK) {
                Bitmap^ bmp = gcnew Bitmap(openFileDialog->FileName);
                pictureBox1->Image = bmp;
                pictureBox2->Image = gcnew Bitmap(bmp);
                SaveState();
                CalculateHistogram(dynamic_cast<Bitmap^>(pictureBox2->Image));
                DrawHistogram();
                DetectDominantColorAndClusters();
            }
        }

        System::Void button2_Click(System::Object^ sender, System::EventArgs^ e) {
            if (pictureBox2->Image == nullptr) return;
            SaveState();
            Bitmap^ source = gcnew Bitmap(pictureBox2->Image);
            cv::Mat srcMat = BitmapToMat(source);
            cv::Mat grayMat;
            cv::cvtColor(srcMat, grayMat, cv::COLOR_BGR2GRAY);
            cv::Mat resultMat;
            cv::cvtColor(grayMat, resultMat, cv::COLOR_GRAY2BGR);
            pictureBox2->Image = MatToBitmap(resultMat);
            delete source;
            CalculateHistogram(dynamic_cast<Bitmap^>(pictureBox2->Image));
            DrawHistogram();
            DetectDominantColorAndClusters();
        }

        System::Void button3_Click(System::Object^ sender, System::EventArgs^ e) {
            if (pictureBox2->Image == nullptr) return;
            SaveFileDialog^ saveFileDialog = gcnew SaveFileDialog();
            saveFileDialog->Filter = "JPEG Image|*.jpg|PNG Image|*.png|Bitmap Image|*.bmp";
            if (saveFileDialog->ShowDialog() == System::Windows::Forms::DialogResult::OK) {
                pictureBox2->Image->Save(saveFileDialog->FileName);
            }
        }

        System::Void button4_Click(System::Object^ sender, System::EventArgs^ e) {
            if (pictureBox2->Image == nullptr) return;
            SaveState();
            isThresholdActive = true; // Activate threshold mode
            Console::WriteLine("Binary button clicked, applying threshold: {0}", trackBar9->Value); // Debug output
            ApplyImageAdjustments();
            thresholdValueLabel->Text = String::Format("Value: {0}", trackBar9->Value);
        }

        System::Void button5_Click(System::Object^ sender, System::EventArgs^ e) {
            if (undoStack->Count <= 1) return;
            delete undoStack->Pop();
            ImageState^ previousState = undoStack->Peek();
            pictureBox2->Image = gcnew Bitmap(previousState->image);
            trackBar1->Value = previousState->redAdjustment;
            trackBar2->Value = previousState->greenAdjustment;
            trackBar3->Value = previousState->blueAdjustment;
            trackBar4->Value = previousState->brightness;
            trackBar5->Value = previousState->contrast;
            trackBar6->Value = previousState->saturation;
            trackBar7->Value = previousState->opacity;
            trackBar8->Value = previousState->blurRadius;
            trackBar9->Value = previousState->thresholdValue;
            trackBar10->Value = previousState->blackThreshold;
            trackBar11->Value = previousState->whiteThreshold;
            lastFilterColor = previousState->filterColor;
            isThresholdActive = previousState->isThresholdActive;
            removeBackground = previousState->removeBackground;
            CalculateHistogram(dynamic_cast<Bitmap^>(pictureBox2->Image));
            DrawHistogram();
            DetectDominantColorAndClusters();
        }

        System::Void button6_Click(System::Object^ sender, System::EventArgs^ e) {
            if (pictureBox2->Image == nullptr) return;
            SaveState();
            ShapeDetectionForm^ shapeForm = gcnew ShapeDetectionForm(dynamic_cast<Bitmap^>(pictureBox2->Image));
            shapeForm->ShowDialog();
        }

        System::Void btnAdaptive_Click(System::Object^ sender, System::EventArgs^ e) {
            if (pictureBox2->Image == nullptr) return;
            SaveState();
            Bitmap^ source = gcnew Bitmap(pictureBox2->Image);
            cv::Mat srcMat = BitmapToMat(source);
            cv::Mat grayMat, threshMat;
            cv::cvtColor(srcMat, grayMat, cv::COLOR_BGR2GRAY);
            cv::adaptiveThreshold(grayMat, threshMat, 255, cv::ADAPTIVE_THRESH_GAUSSIAN_C, cv::THRESH_BINARY, 15, 2);
            cv::Mat resultMat;
            cv::cvtColor(threshMat, resultMat, cv::COLOR_GRAY2BGR);
            pictureBox2->Image = MatToBitmap(resultMat);
            delete source;
            CalculateHistogram(dynamic_cast<Bitmap^>(pictureBox2->Image));
            DrawHistogram();
            DetectDominantColorAndClusters();
        }

        System::Void button8_Click(System::Object^ sender, System::EventArgs^ e) {
            if (pictureBox1->Image == nullptr) return;
            while (undoStack->Count > 0) delete undoStack->Pop();
            pictureBox2->Image = gcnew Bitmap(pictureBox1->Image);
            trackBar1->Value = 0;
            trackBar2->Value = 0;
            trackBar3->Value = 0;
            trackBar4->Value = 0;
            trackBar5->Value = 0;
            trackBar6->Value = 0;
            trackBar7->Value = 100;
            trackBar8->Value = 0;
            trackBar9->Value = 128;
            trackBar10->Value = 0;
            trackBar11->Value = 255;
            lastFilterColor = Color::Empty;
            isThresholdActive = false;
            removeBackground = false;
            SaveState();
            CalculateHistogram(dynamic_cast<Bitmap^>(pictureBox2->Image));
            DrawHistogram();
            DetectDominantColorAndClusters();
        }

        System::Void button9_Click(System::Object^ sender, System::EventArgs^ e) {
            if (pictureBox2->Image == nullptr) return;
            SaveState();
            removeBackground = !removeBackground;
            ApplyImageAdjustments();
        }

        System::Void button10_Click(System::Object^ sender, System::EventArgs^ e) {
            if (pictureBox2->Image == nullptr) return;
            SaveState();
            Bitmap^ source = gcnew Bitmap(pictureBox2->Image);
            cv::Mat srcMat = BitmapToMat(source);
            cv::Mat labMat, reshaped;

            // Convert to Lab color space for better clustering
            cv::cvtColor(srcMat, labMat, cv::COLOR_BGR2Lab);

            // Reshape image for K-means
            reshaped = labMat.reshape(1, labMat.rows * labMat.cols);
            reshaped.convertTo(reshaped, CV_32F);

            // Set default number of clusters
            const int numClusters = 3;
            cv::Mat labels, centers;

            // Perform K-means clustering
            cv::kmeans(reshaped, numClusters, labels,
                cv::TermCriteria(cv::TermCriteria::EPS + cv::TermCriteria::COUNT, 20, 0.5),
                5, cv::KMEANS_PP_CENTERS, centers);

            // Convert centers to 8-bit unsigned
            centers.convertTo(centers, CV_8U);

            // Create segmented image
            cv::Mat kmeansMat(labMat.rows, labMat.cols, labMat.type());

            // Assign cluster centers to pixels
            for (int i = 0; i < labMat.rows * labMat.cols; i++) {
                int clusterIdx = labels.at<int>(i);
                int y = i / labMat.cols;
                int x = i % labMat.cols;
                kmeansMat.at<cv::Vec3b>(y, x) = cv::Vec3b(
                    centers.at<uchar>(clusterIdx, 0),
                    centers.at<uchar>(clusterIdx, 1),
                    centers.at<uchar>(clusterIdx, 2));
            }

            // Convert back to BGR for display
            cv::Mat resultMat;
            cv::cvtColor(kmeansMat, resultMat, cv::COLOR_Lab2BGR);

            // Update pictureBox2 with segmented image
            pictureBox2->Image = MatToBitmap(resultMat);

            // Clean up
            delete source;

            // Update histogram and color analysis
            CalculateHistogram(dynamic_cast<Bitmap^>(pictureBox2->Image));
            DrawHistogram();
            DetectDominantColorAndClusters();
        }

        System::Void button13_Click(System::Object^ sender, System::EventArgs^ e) {
            if (pictureBox2->Image == nullptr) return;
            SaveState();
            Bitmap^ source = gcnew Bitmap(pictureBox2->Image);
            cv::Mat srcMat = BitmapToMat(source);
            cv::Mat grayMat, sobelX, sobelY, sobelMat;

            // Convert to grayscale
            cv::cvtColor(srcMat, grayMat, cv::COLOR_BGR2GRAY);

            // Apply Sobel operators
            cv::Sobel(grayMat, sobelX, CV_32F, 1, 0, 3);
            cv::Sobel(grayMat, sobelY, CV_32F, 0, 1, 3);

            // Compute magnitude
            cv::Mat sobelXSquared, sobelYSquared;
            cv::pow(sobelX, 2, sobelXSquared);
            cv::pow(sobelY, 2, sobelYSquared);
            cv::Mat sobelSum;
            cv::add(sobelXSquared, sobelYSquared, sobelSum);
            cv::sqrt(sobelSum, sobelMat);

            // Normalize to 8-bit for display
            cv::normalize(sobelMat, sobelMat, 0, 255, cv::NORM_MINMAX, CV_8U);

            // Convert to BGR for display
            cv::Mat resultMat;
            cv::cvtColor(sobelMat, resultMat, cv::COLOR_GRAY2BGR);
            pictureBox2->Image = MatToBitmap(resultMat);
            delete source;
            CalculateHistogram(dynamic_cast<Bitmap^>(pictureBox2->Image));
            DrawHistogram();
            DetectDominantColorAndClusters();
        }

        void DetectDominantColorAndClusters() {
            if (pictureBox2->Image == nullptr) return;

            Bitmap^ source = gcnew Bitmap(pictureBox2->Image);
            cv::Mat srcMat = BitmapToMat(source);

            // Convert to Lab for better color clustering
            cv::Mat labMat;
            cv::cvtColor(srcMat, labMat, cv::COLOR_BGR2Lab);

            // Reshape for K-means
            cv::Mat reshaped = labMat.reshape(1, labMat.rows * labMat.cols);
            reshaped.convertTo(reshaped, CV_32F);

            // Perform K-means clustering
            int numClusters = 5;
            cv::Mat labels, centers;
            cv::kmeans(reshaped, numClusters, labels,
                cv::TermCriteria(cv::TermCriteria::EPS + cv::TermCriteria::COUNT, 20, 0.5),
                5, cv::KMEANS_PP_CENTERS, centers);

            // Count pixels in each cluster
            array<int>^ clusterCounts = gcnew array<int>(numClusters);
            for (int i = 0; i < numClusters; i++) clusterCounts[i] = 0;
            for (int i = 0; i < labels.rows; i++) {
                clusterCounts[labels.at<int>(i)]++;
            }

            // Find dominant cluster
            int maxCount = 0, dominantCluster = 0;
            for (int i = 0; i < numClusters; i++) {
                if (clusterCounts[i] > maxCount) {
                    maxCount = clusterCounts[i];
                    dominantCluster = i;
                }
            }

            // Convert dominant cluster center to BGR
            centers.convertTo(centers, CV_8U);
            cv::Mat dominantColorMat(1, 1, CV_8UC3);
            dominantColorMat.at<cv::Vec3b>(0, 0) = cv::Vec3b(
                centers.at<uchar>(dominantCluster, 0),
                centers.at<uchar>(dominantCluster, 1),
                centers.at<uchar>(dominantCluster, 2));
            cv::cvtColor(dominantColorMat, dominantColorMat, cv::COLOR_Lab2BGR);
            Color dominantColor = Color::FromArgb(
                dominantColorMat.at<cv::Vec3b>(0, 0)[2],
                dominantColorMat.at<cv::Vec3b>(0, 0)[1],
                dominantColorMat.at<cv::Vec3b>(0, 0)[0]);

            // Update labels
            dominantColorLabel->Text = String::Format("Dominant Color: R={0}, G={1}, B={2}",
                dominantColor.R, dominantColor.G, dominantColor.B);
            colorClustersLabel->Text = String::Format("Color Clusters: {0}", numClusters);

            delete source;
        }

        void CalculateHistogram(Bitmap^ image) {
            redHistogram = gcnew array<int>(256);
            greenHistogram = gcnew array<int>(256);
            blueHistogram = gcnew array<int>(256);
            for (int i = 0; i < 256; i++) {
                redHistogram[i] = 0;
                greenHistogram[i] = 0;
                blueHistogram[i] = 0;
            }

            BitmapData^ bmpData = image->LockBits(
                System::Drawing::Rectangle(0, 0, image->Width, image->Height),
                ImageLockMode::ReadOnly, PixelFormat::Format24bppRgb);
            unsigned char* ptr = (unsigned char*)bmpData->Scan0.ToPointer();
            for (int y = 0; y < image->Height; y++) {
                for (int x = 0; x < image->Width; x++) {
                    int index = y * bmpData->Stride + x * 3;
                    blueHistogram[ptr[index]]++;
                    greenHistogram[ptr[index + 1]]++;
                    redHistogram[ptr[index + 2]]++;
                }
            }
            image->UnlockBits(bmpData);
        }

        void DrawHistogram() {
            if (redHistogram == nullptr || greenHistogram == nullptr || blueHistogram == nullptr) return;

            int width = 256, height = 100;
            Bitmap^ histBitmap = gcnew Bitmap(width, height);
            Graphics^ g = Graphics::FromImage(histBitmap);
            g->Clear(Color::White);

            int maxVal = 0;
            for (int i = 0; i < 256; i++) {
                maxVal = Math::Max(maxVal, redHistogram[i]);
                maxVal = Math::Max(maxVal, greenHistogram[i]);
                maxVal = Math::Max(maxVal, blueHistogram[i]);
            }
            if (maxVal == 0) maxVal = 1;

            for (int i = 0; i < 256; i++) {
                int rHeight = (int)((double)redHistogram[i] / maxVal * height);
                int gHeight = (int)((double)greenHistogram[i] / maxVal * height);
                int bHeight = (int)((double)blueHistogram[i] / maxVal * height);
                g->DrawLine(Pens::Red, i, height, i, height - rHeight);
                g->DrawLine(Pens::Green, i, height, i, height - gHeight);
                g->DrawLine(Pens::Blue, i, height, i, height - bHeight);
            }

            histogramBox->Image = histBitmap;
            delete g;
        }

        
        System::Void ApplyImageAdjustments() {
            if (pictureBox2->Image == nullptr || pictureBox1->Image == nullptr) {
                System::Diagnostics::Debug::WriteLine("ApplyImageAdjustments: Source or output image is null, skipping.");
                return;
            }

            Bitmap^ source = gcnew Bitmap(pictureBox1->Image);
            Bitmap^ result = gcnew Bitmap(source->Width, source->Height);

            BitmapData^ srcData = source->LockBits(
                System::Drawing::Rectangle(0, 0, source->Width, source->Height),
                ImageLockMode::ReadOnly, PixelFormat::Format32bppArgb);
            BitmapData^ dstData = result->LockBits(
                System::Drawing::Rectangle(0, 0, result->Width, result->Height),
                ImageLockMode::WriteOnly, PixelFormat::Format32bppArgb);

            unsigned char* srcPtr = (unsigned char*)srcData->Scan0.ToPointer();
            unsigned char* dstPtr = (unsigned char*)dstData->Scan0.ToPointer();
            int stride = srcData->Stride;

            float brightness = trackBar4->Value / 100.0f;
            float contrast = 1.0f + trackBar5->Value / 100.0f;
            float saturation = 1.0f + trackBar6->Value / 100.0f;
            int opacity = trackBar7->Value;
            int blurRadius = trackBar8->Value;
            int thresholdValue = trackBar9->Value;
            int blackThreshold = trackBar10->Value;
            int whiteThreshold = trackBar11->Value;

            // Ensure black threshold is not greater than white threshold
            if (blackThreshold > whiteThreshold) {
                blackThreshold = whiteThreshold;
                trackBar10->Value = blackThreshold;
            }

            thresholdValueLabel->Text = "Value: " + thresholdValue;
            blackValueLabel->Text = "Black: " + blackThreshold;
            whiteValueLabel->Text = "White: " + whiteThreshold;

            for (int y = 0; y < source->Height; y++) {
                for (int x = 0; x < source->Width; x++) {
                    int offset = y * stride + x * 4;
                    int b = srcPtr[offset];
                    int g = srcPtr[offset + 1];
                    int r = srcPtr[offset + 2];
                    int a = srcPtr[offset + 3];

                    r += trackBar1->Value;
                    g += trackBar2->Value;
                    b += trackBar3->Value;

                    r = Math::Max(0, Math::Min(255, r));
                    g = Math::Max(0, Math::Min(255, g));
                    b = Math::Max(0, Math::Min(255, b));

                    float fr = r / 255.0f;
                    float fg = g / 255.0f;
                    float fb = b / 255.0f;

                    fr += brightness;
                    fg += brightness;
                    fb += brightness;

                    float avg = (fr + fg + fb) / 3.0f;
                    fr = avg + (fr - avg) * saturation;
                    fg = avg + (fg - avg) * saturation;
                    fb = avg + (fb - avg) * saturation;

                    fr = (fr - 0.5f) * contrast + 0.5f;
                    fg = (fg - 0.5f) * contrast + 0.5f;
                    fb = (fb - 0.5f) * contrast + 0.5f;

                    r = Math::Max(0, Math::Min(255, (int)(fr * 255)));
                    g = Math::Max(0, Math::Min(255, (int)(fg * 255)));
                    b = Math::Max(0, Math::Min(255, (int)(fb * 255)));

                    a = (a * opacity) / 100;

                    if (lastFilterColor != Color::Empty) {
                        r = (r + lastFilterColor.R) / 2;
                        g = (g + lastFilterColor.G) / 2;
                        b = (b + lastFilterColor.B) / 2;
                    }

                    if (isThresholdActive) {
                        int gray = (int)(r * 0.3 + g * 0.59 + b * 0.11);
                        if (gray >= thresholdValue) {
                            r = g = b = whiteThreshold;
                        }
                        else {
                            r = g = b = blackThreshold;
                        }
                    }

                    if (removeBackground) {
                        int gray = (int)(r * 0.3 + g * 0.59 + b * 0.11);
                        if (gray > 200) a = 0;
                    }

                    dstPtr[offset] = (unsigned char)b;
                    dstPtr[offset + 1] = (unsigned char)g;
                    dstPtr[offset + 2] = (unsigned char)r;
                    dstPtr[offset + 3] = (unsigned char)a;
                }
            }

            source->UnlockBits(srcData);
            result->UnlockBits(dstData);

            if (blurRadius > 0) {
                Bitmap^ blurred = gcnew Bitmap(result->Width, result->Height);
                BitmapData^ blurSrcData = result->LockBits(
                    System::Drawing::Rectangle(0, 0, result->Width, result->Height),
                    ImageLockMode::ReadOnly, PixelFormat::Format32bppArgb);
                BitmapData^ blurDstData = blurred->LockBits(
                    System::Drawing::Rectangle(0, 0, blurred->Width, blurred->Height),
                    ImageLockMode::WriteOnly, PixelFormat::Format32bppArgb);

                unsigned char* blurSrcPtr = (unsigned char*)blurSrcData->Scan0.ToPointer();
                unsigned char* blurDstPtr = (unsigned char*)blurDstData->Scan0.ToPointer();
                int blurStride = blurSrcData->Stride;

                for (int y = 0; y < result->Height; y++) {
                    for (int x = 0; x < result->Width; x++) {
                        int rSum = 0, gSum = 0, bSum = 0, aSum = 0, count = 0;
                        for (int dy = -blurRadius; dy <= blurRadius; dy++) {
                            for (int dx = -blurRadius; dx <= blurRadius; dx++) {
                                int nx = x + dx, ny = y + dy;
                                if (nx >= 0 && nx < result->Width && ny >= 0 && ny < result->Height) {
                                    int offset = ny * blurStride + nx * 4;
                                    bSum += blurSrcPtr[offset];
                                    gSum += blurSrcPtr[offset + 1];
                                    rSum += blurSrcPtr[offset + 2];
                                    aSum += blurSrcPtr[offset + 3];
                                    count++;
                                }
                            }
                        }
                        int offset = y * blurStride + x * 4;
                        blurDstPtr[offset] = (unsigned char)(bSum / count);
                        blurDstPtr[offset + 1] = (unsigned char)(gSum / count);
                        blurDstPtr[offset + 2] = (unsigned char)(rSum / count);
                        blurDstPtr[offset + 3] = (unsigned char)(aSum / count);
                    }
                }

                result->UnlockBits(blurSrcData);
                blurred->UnlockBits(blurDstData);

                // Dispose of the old result to free memory
                delete result;
                result = blurred;
            }

            // Dispose of the old image in pictureBox2 to prevent memory leaks
            if (pictureBox2->Image != nullptr) {
                Image^ oldImage = pictureBox2->Image;
                pictureBox2->Image = nullptr; // Detach the old image
                delete oldImage; // Dispose of the old image
            }

            pictureBox2->Image = result;
            delete source;

            CalculateHistogram(result);
            DrawHistogram();
            DetectDominantColorAndClusters();
        }

        System::Void trackBar_Scroll(System::Object^ sender, System::EventArgs^ e) {
            TrackBar^ trackBar = dynamic_cast<TrackBar^>(sender);

            // Activate threshold only when threshold trackbars are used
            if (trackBar == trackBar9 || trackBar == trackBar10 || trackBar == trackBar11) {
                if (!isThresholdActive) {
                    isThresholdActive = true; // Enable thresholding on first use
                }
            }

            // Safely handle debounceTimer
            if (debounceTimer != nullptr) {
                if (debounceTimer->Enabled) {
                    debounceTimer->Stop();
                }
                debounceTimer->Start();
            }
        }

        System::Void btnRGB_Click(System::Object^ sender, System::EventArgs^ e) {
            panelRGB->Visible = true;
            panelThreshold->Visible = false;
            panelEnhancements->Visible = false;
            panelFilters->Visible = false;
            panelOrientation->Visible = false;
            panelProjection->Visible = false;
            btnRGB->BackColor = System::Drawing::Color::DarkGray;
            btnThreshold->BackColor = System::Drawing::Color::Gray;
            btnEnhancements->BackColor = System::Drawing::Color::Gray;
            btnFilters->BackColor = System::Drawing::Color::Gray;
            button11->BackColor = System::Drawing::Color::Gray;
            button12->BackColor = System::Drawing::Color::Gray;
        }

        System::Void btnThreshold_Click(System::Object^ sender, System::EventArgs^ e) {
            panelRGB->Visible = false;
            panelThreshold->Visible = true;
            panelEnhancements->Visible = false;
            panelFilters->Visible = false;
            panelOrientation->Visible = false;
            panelProjection->Visible = false;
            btnRGB->BackColor = System::Drawing::Color::Gray;
            btnThreshold->BackColor = System::Drawing::Color::DarkGray;
            btnEnhancements->BackColor = System::Drawing::Color::Gray;
            btnFilters->BackColor = System::Drawing::Color::Gray;
            button11->BackColor = System::Drawing::Color::Gray;
            button12->BackColor = System::Drawing::Color::Gray;

            // Initialize trackbar values
            trackBar9->Value = 128; // Default threshold value
            trackBar10->Value = 0;  // Default black threshold
            trackBar11->Value = 255; // Default white threshold
            thresholdValueLabel->Text = "Value: " + trackBar9->Value;
            blackValueLabel->Text = "Black: " + trackBar10->Value;
            whiteValueLabel->Text = "White: " + trackBar11->Value;

            // Do not set isThresholdActive = true here; it will be set on trackbar use
            // Apply adjustments only if an image is loaded
            if (pictureBox2->Image != nullptr && pictureBox1->Image != nullptr) {
                ApplyImageAdjustments();
            }
        }

        System::Void btnEnhancements_Click(System::Object^ sender, System::EventArgs^ e) {
            panelRGB->Visible = false;
            panelThreshold->Visible = false;
            panelEnhancements->Visible = true;
            panelFilters->Visible = false;
            panelOrientation->Visible = false;
            panelProjection->Visible = false;
            btnRGB->BackColor = System::Drawing::Color::Gray;
            btnThreshold->BackColor = System::Drawing::Color::Gray;
            btnEnhancements->BackColor = System::Drawing::Color::DarkGray;
            btnFilters->BackColor = System::Drawing::Color::Gray;
            button11->BackColor = System::Drawing::Color::Gray;
            button12->BackColor = System::Drawing::Color::Gray;
        }

        System::Void btnFilters_Click(System::Object^ sender, System::EventArgs^ e) {
            panelRGB->Visible = false;
            panelThreshold->Visible = false;
            panelEnhancements->Visible = false;
            panelFilters->Visible = true;
            panelOrientation->Visible = false;
            panelProjection->Visible = false;
            btnRGB->BackColor = System::Drawing::Color::Gray;
            btnThreshold->BackColor = System::Drawing::Color::Gray;
            btnEnhancements->BackColor = System::Drawing::Color::Gray;
            btnFilters->BackColor = System::Drawing::Color::DarkGray;
            button11->BackColor = System::Drawing::Color::Gray;
            button12->BackColor = System::Drawing::Color::Gray;
        }

        System::Void button11_Click(System::Object^ sender, System::EventArgs^ e) {
            panelRGB->Visible = false;
            panelThreshold->Visible = false;
            panelEnhancements->Visible = false;
            panelFilters->Visible = false;
            panelOrientation->Visible = true;
            panelProjection->Visible = false;
            btnRGB->BackColor = System::Drawing::Color::Gray;
            btnThreshold->BackColor = System::Drawing::Color::Gray;
            btnEnhancements->BackColor = System::Drawing::Color::Gray;
            btnFilters->BackColor = System::Drawing::Color::Gray;
            button11->BackColor = System::Drawing::Color::DarkGray;
            button12->BackColor = System::Drawing::Color::Gray;
        }

        System::Void button12_Click(System::Object^ sender, System::EventArgs^ e) {
            panelRGB->Visible = false;
            panelThreshold->Visible = false;
            panelEnhancements->Visible = false;
            panelFilters->Visible = false;
            panelOrientation->Visible = false;
            panelProjection->Visible = true;
            btnRGB->BackColor = System::Drawing::Color::Gray;
            btnThreshold->BackColor = System::Drawing::Color::Gray;
            btnEnhancements->BackColor = System::Drawing::Color::Gray;
            btnFilters->BackColor = System::Drawing::Color::Gray;
            button11->BackColor = System::Drawing::Color::Gray;
            button12->BackColor = System::Drawing::Color::DarkGray;
        }

        System::Void btnBlueGreen_Click(System::Object^ sender, System::EventArgs^ e) {
            SaveState();
            lastFilterColor = Color::FromArgb(0, 128, 128);
            ApplyImageAdjustments();
        }

        System::Void btnViolet_Click(System::Object^ sender, System::EventArgs^ e) {
            SaveState();
            lastFilterColor = Color::FromArgb(128, 0, 128);
            ApplyImageAdjustments();
        }

        System::Void btnBrown_Click(System::Object^ sender, System::EventArgs^ e) {
            SaveState();
            lastFilterColor = Color::FromArgb(139, 69, 19);
            ApplyImageAdjustments();
        }

        System::Void btnKhaki_Click(System::Object^ sender, System::EventArgs^ e) {
            SaveState();
            lastFilterColor = Color::FromArgb(240, 230, 140);
            ApplyImageAdjustments();
        }

        System::Void btnOrange_Click(System::Object^ sender, System::EventArgs^ e) {
            SaveState();
            lastFilterColor = Color::FromArgb(255, 165, 0);
            ApplyImageAdjustments();
        }

        System::Void btnCyan_Click(System::Object^ sender, System::EventArgs^ e) {
            SaveState();
            lastFilterColor = Color::FromArgb(0, 255, 255);
            ApplyImageAdjustments();
        }

        System::Void btnPink_Click(System::Object^ sender, System::EventArgs^ e) {
            SaveState();
            lastFilterColor = Color::FromArgb(255, 192, 203);
            ApplyImageAdjustments();
        }

        System::Void btnTeal_Click(System::Object^ sender, System::EventArgs^ e) {
            SaveState();
            lastFilterColor = Color::FromArgb(0, 128, 128);
            ApplyImageAdjustments();
        }

        System::Void btnLime_Click(System::Object^ sender, System::EventArgs^ e) {
            SaveState();
            lastFilterColor = Color::FromArgb(0, 255, 0);
            ApplyImageAdjustments();
        }

        System::Void btnMagenta_Click(System::Object^ sender, System::EventArgs^ e) {
            SaveState();
            lastFilterColor = Color::FromArgb(255, 0, 255);
            ApplyImageAdjustments();
        }

        System::Void btnRotate90CW_Click(System::Object^ sender, System::EventArgs^ e) {
            if (pictureBox2->Image == nullptr) return;
            SaveState();
            Bitmap^ source = gcnew Bitmap(pictureBox2->Image);
            source->RotateFlip(RotateFlipType::Rotate90FlipNone);
            pictureBox2->Image = source;
            CalculateHistogram(dynamic_cast<Bitmap^>(pictureBox2->Image));
            DrawHistogram();
            DetectDominantColorAndClusters();
        }

        System::Void btnRotate90CCW_Click(System::Object^ sender, System::EventArgs^ e) {
            if (pictureBox2->Image == nullptr) return;
            SaveState();
            Bitmap^ source = gcnew Bitmap(pictureBox2->Image);
            source->RotateFlip(RotateFlipType::Rotate270FlipNone);
            pictureBox2->Image = source;
            CalculateHistogram(dynamic_cast<Bitmap^>(pictureBox2->Image));
            DrawHistogram();
            DetectDominantColorAndClusters();
        }

        System::Void btnRotate180_Click(System::Object^ sender, System::EventArgs^ e) {
            if (pictureBox2->Image == nullptr) return;
            SaveState();
            Bitmap^ source = gcnew Bitmap(pictureBox2->Image);
            source->RotateFlip(RotateFlipType::Rotate180FlipNone);
            pictureBox2->Image = source;
            CalculateHistogram(dynamic_cast<Bitmap^>(pictureBox2->Image));
            DrawHistogram();
            DetectDominantColorAndClusters();
        }

        System::Void btnFlipHorizontal_Click(System::Object^ sender, System::EventArgs^ e) {
            if (pictureBox2->Image == nullptr) return;
            SaveState();
            Bitmap^ source = gcnew Bitmap(pictureBox2->Image);
            source->RotateFlip(RotateFlipType::RotateNoneFlipX);
            pictureBox2->Image = source;
            CalculateHistogram(dynamic_cast<Bitmap^>(pictureBox2->Image));
            DrawHistogram();
            DetectDominantColorAndClusters();
        }

        System::Void btnFlipVertical_Click(System::Object^ sender, System::EventArgs^ e) {
            if (pictureBox2->Image == nullptr) return;
            SaveState();
            Bitmap^ source = gcnew Bitmap(pictureBox2->Image);
            source->RotateFlip(RotateFlipType::RotateNoneFlipY);
            pictureBox2->Image = source;
            CalculateHistogram(dynamic_cast<Bitmap^>(pictureBox2->Image));
            DrawHistogram();
            DetectDominantColorAndClusters();
        }

        System::Void btnHorizontalProjection_Click(System::Object^ sender, System::EventArgs^ e) {
            if (pictureBox2->Image == nullptr) return;
            SaveState();
            Bitmap^ source = gcnew Bitmap(pictureBox2->Image);
            cv::Mat srcMat = BitmapToMat(source);
            cv::Mat grayMat;
            cv::cvtColor(srcMat, grayMat, cv::COLOR_BGR2GRAY);

            std::vector<int> projection(grayMat.rows, 0);
            for (int y = 0; y < grayMat.rows; y++) {
                for (int x = 0; x < grayMat.cols; x++) {
                    projection[y] += 255 - grayMat.at<uchar>(y, x);
                }
            }

            int maxProj = *std::max_element(projection.begin(), projection.end());
            if (maxProj == 0) maxProj = 1;

            cv::Mat projMat(grayMat.rows, grayMat.cols, CV_8UC1, cv::Scalar(255));
            for (int y = 0; y < grayMat.rows; y++) {
                int width = (projection[y] * grayMat.cols) / maxProj;
                for (int x = 0; x < width; x++) {
                    projMat.at<uchar>(y, x) = 0;
                }
            }

            cv::Mat resultMat;
            cv::cvtColor(projMat, resultMat, cv::COLOR_GRAY2BGR);
            pictureBox2->Image = MatToBitmap(resultMat);
            delete source;
            CalculateHistogram(dynamic_cast<Bitmap^>(pictureBox2->Image));
            DrawHistogram();
            DetectDominantColorAndClusters();
        }

        System::Void btnVerticalProjection_Click(System::Object^ sender, System::EventArgs^ e) {
            if (pictureBox2->Image == nullptr) return;
            SaveState();
            Bitmap^ source = gcnew Bitmap(pictureBox2->Image);
            cv::Mat srcMat = BitmapToMat(source);
            cv::Mat grayMat;
            cv::cvtColor(srcMat, grayMat, cv::COLOR_BGR2GRAY);

            std::vector<int> projection(grayMat.cols, 0);
            for (int x = 0; x < grayMat.cols; x++) {
                for (int y = 0; y < grayMat.rows; y++) {
                    projection[x] += 255 - grayMat.at<uchar>(y, x);
                }
            }

            int maxProj = *std::max_element(projection.begin(), projection.end());
            if (maxProj == 0) maxProj = 1;

            cv::Mat projMat(grayMat.rows, grayMat.cols, CV_8UC1, cv::Scalar(255));
            for (int x = 0; x < grayMat.cols; x++) {
                int height = (projection[x] * grayMat.rows) / maxProj;
                for (int y = 0; y < height; y++) {
                    projMat.at<uchar>(grayMat.rows - 1 - y, x) = 0;
                }
            }

            cv::Mat resultMat;
            cv::cvtColor(projMat, resultMat, cv::COLOR_GRAY2BGR);
            pictureBox2->Image = MatToBitmap(resultMat);
            delete source;
            CalculateHistogram(dynamic_cast<Bitmap^>(pictureBox2->Image));
            DrawHistogram();
            DetectDominantColorAndClusters();
        }

        System::Void pictureBox1_Click(System::Object^ sender, System::EventArgs^ e) {
            // Optional: Handle click events on pictureBox1
        }

        System::Void label2_Click(System::Object^ sender, System::EventArgs^ e) {
            // Optional: Handle click events on label2
        }

        System::Void dominantColorLabel_Click(System::Object^ sender, System::EventArgs^ e) {
            // Optional: Handle click events on dominantColorLabel
        }

        void SaveState() {
            if (pictureBox2->Image == nullptr) return;

            // Limit undo stack size to prevent memory bloat
            if (undoStack->Count > 20) {
                delete undoStack->Pop();
            }

            // Create a smaller thumbnail for undo stack to save memory
            Bitmap^ source = dynamic_cast<Bitmap^>(pictureBox2->Image);
            Bitmap^ thumbnail = gcnew Bitmap(source, Math::Min(source->Width, 200), Math::Min(source->Height, 200));
            undoStack->Push(gcnew ImageState(
                thumbnail,
                trackBar1->Value, trackBar2->Value, trackBar3->Value,
                trackBar4->Value, trackBar5->Value, trackBar6->Value,
                trackBar7->Value, trackBar8->Value, trackBar9->Value,
                trackBar10->Value, trackBar11->Value,
                lastFilterColor, isThresholdActive, removeBackground));
        }
    private: System::Void MainForm_Load(System::Object^ sender, System::EventArgs^ e) {
    }

     System::Void debounceTimer_Tick(System::Object^ sender, System::EventArgs^ e) {
             // Stop the timer to prevent multiple executions  
               debounceTimer->Stop();

               // Apply image adjustments  
               ApplyImageAdjustments();
      }

    private: System::Void panelRGB_Paint(System::Object^ sender, System::Windows::Forms::PaintEventArgs^ e) {
    }


           System::Void btnTranslate_Click(System::Object^ sender, System::EventArgs^ e) {
               if (pictureBox2->Image == nullptr) return;

               // Prompt user for translation values
               String^ inputX = Microsoft::VisualBasic::Interaction::InputBox(
                   "Enter X translation (pixels, positive = right, negative = left):",
                   "X Translation", "0", -1, -1);
               String^ inputY = Microsoft::VisualBasic::Interaction::InputBox(
                   "Enter Y translation (pixels, positive = down, negative = up):",
                   "Y Translation", "0", -1, -1);

               float tx, ty;
               if (!float::TryParse(inputX, tx) || !float::TryParse(inputY, ty)) {
                   MessageBox::Show("Invalid translation values. Please enter numeric values.", "Error",
                       MessageBoxButtons::OK, MessageBoxIcon::Error);
                   return;
               }

               SaveState();

               // Convert Bitmap to cv::Mat
               Bitmap^ source = gcnew Bitmap(pictureBox2->Image);
               cv::Mat srcMat = BitmapToMat(source);

               // Create translation matrix
               cv::Mat translationMatrix = (cv::Mat_<float>(2, 3) << 1, 0, tx, 0, 1, ty);

               // Apply translation
               cv::Mat resultMat;
               cv::warpAffine(srcMat, resultMat, translationMatrix, srcMat.size(), cv::INTER_LINEAR, cv::BORDER_CONSTANT, cv::Scalar(255, 255, 255));

               // Convert back to Bitmap and update pictureBox2
               pictureBox2->Image = MatToBitmap(resultMat);

               // Clean up
               delete source;

               // Update histogram and color analysis
               CalculateHistogram(dynamic_cast<Bitmap^>(pictureBox2->Image));
               DrawHistogram();
               DetectDominantColorAndClusters();
           }


           System::Void button14_Click(System::Object^ sender, System::EventArgs^ e) {
               panelRGB->Visible = false;
               panelThreshold->Visible = false;
               panelEnhancements->Visible = false;
               panelFilters->Visible = false;
               panelOrientation->Visible = false;
               panelProjection->Visible = false;
               panelConvolution->Visible = true;
               btnRGB->BackColor = System::Drawing::Color::Gray;
               btnThreshold->BackColor = System::Drawing::Color::Gray;
               btnEnhancements->BackColor = System::Drawing::Color::Gray;
               btnFilters->BackColor = System::Drawing::Color::Gray;
               button11->BackColor = System::Drawing::Color::Gray;
               button12->BackColor = System::Drawing::Color::Gray;
               button14->BackColor = System::Drawing::Color::DarkGray;
           }
           void ApplyConvolutionFilter(const cv::Mat& kernel) {
               if (pictureBox2->Image == nullptr) return;

               SaveState();

               Bitmap^ source = gcnew Bitmap(pictureBox2->Image);
               cv::Mat srcMat = BitmapToMat(source);
               cv::Mat resultMat;

               // Apply convolution filter
               cv::filter2D(srcMat, resultMat, -1, kernel, cv::Point(-1, -1), 0, cv::BORDER_DEFAULT);

               pictureBox2->Image = MatToBitmap(resultMat);
               delete source;

               CalculateHistogram(dynamic_cast<Bitmap^>(pictureBox2->Image));
               DrawHistogram();
               DetectDominantColorAndClusters();
           }

           System::Void btnSmoothing_Click(System::Object^ sender, System::EventArgs^ e) {
               // Smoothing filter (average blur)
               float kernelData[9] = {
                   1.0f / 9, 1.0f / 9, 1.0f / 9,
                   1.0f / 9, 1.0f / 9, 1.0f / 9,
                   1.0f / 9, 1.0f / 9, 1.0f / 9
               };
               cv::Mat kernel(3, 3, CV_32F, kernelData);
               ApplyConvolutionFilter(kernel);
           }

           System::Void btnGaussianBlur_Click(System::Object^ sender, System::EventArgs^ e) {
               // Gaussian blur (simplified 3x3 kernel)
               float kernelData[9] = {
                   1.0f / 16, 2.0f / 16, 1.0f / 16,
                   2.0f / 16, 4.0f / 16, 2.0f / 16,
                   1.0f / 16, 2.0f / 16, 1.0f / 16
               };
               cv::Mat kernel(3, 3, CV_32F, kernelData);
               ApplyConvolutionFilter(kernel);
           }

           System::Void btnSharpen_Click(System::Object^ sender, System::EventArgs^ e) {
               // Sharpen filter
               float kernelData[9] = {
                   0, -1, 0,
                   -1, 5, -1,
                   0, -1, 0
               };
               cv::Mat kernel(3, 3, CV_32F, kernelData);
               ApplyConvolutionFilter(kernel);
           }

           System::Void btnMeanRemoval_Click(System::Object^ sender, System::EventArgs^ e) {
               // Mean removal (high-pass filter)
               float kernelData[9] = {
                   -1, -1, -1,
                   -1, 9, -1,
                   -1, -1, -1
               };
               cv::Mat kernel(3, 3, CV_32F, kernelData);
               ApplyConvolutionFilter(kernel);
           }

           System::Void btnEmboss_Click(System::Object^ sender, System::EventArgs^ e) {
               // Emboss filter
               float kernelData[9] = {
                   -2, -1, 0,
                   -1, 1, 1,
                   0, 1, 2
               };
               cv::Mat kernel(3, 3, CV_32F, kernelData);
               ApplyConvolutionFilter(kernel);
           }
           System::Void btnManualMath_Click(System::Object^ sender, System::EventArgs^ e) {
               if (pictureBox2->Image == nullptr) return;

               // Prompt user for 3x3 kernel values
               array<String^>^ prompts = {
                   "Kernel[0,0]:", "Kernel[0,1]:", "Kernel[0,2]:",
                   "Kernel[1,0]:", "Kernel[1,1]:", "Kernel[1,2]:",
                   "Kernel[2,0]:", "Kernel[2,1]:", "Kernel[2,2]:"
               };
               float kernelData[9];
               bool validInput = true;

               for (int i = 0; i < 9; i++) {
                   String^ input = Microsoft::VisualBasic::Interaction::InputBox(
                       prompts[i], "Enter Kernel Value", "0", -1, -1);
                   if (!float::TryParse(input, kernelData[i])) {
                       MessageBox::Show("Invalid kernel value. Please enter numeric values.", "Error",
                           MessageBoxButtons::OK, MessageBoxIcon::Error);
                       validInput = false;
                       break;
                   }
               }

               if (validInput) {
                   cv::Mat kernel(3, 3, CV_32F, kernelData);
                   ApplyConvolutionFilter(kernel);
               }
           }
};
}
