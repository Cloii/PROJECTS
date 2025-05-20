#pragma once

namespace proj1 {

    using namespace System;
    using namespace System::ComponentModel;
    using namespace System::Collections;
    using namespace System::Windows::Forms;
    using namespace System::Data;
    using namespace System::Drawing;

    public ref class KernelInputForm : public System::Windows::Forms::Form
    {
    public:
        KernelInputForm(void)
        {
            InitializeComponent();
            InitializeDataGridView();
        }

        // Property to retrieve kernel values
        property array<float>^ KernelValues {
            array<float>^ get() {
                array<float>^ kernel = gcnew array<float>(9);
                int index = 0;
                for (int row = 0; row < 3; row++) {
                    for (int col = 0; col < 3; col++) {
                        String^ value = safe_cast<String^>(dataGridView1->Rows[row]->Cells[col]->Value);
                        float parsedValue;
                        float::TryParse(value, parsedValue);
                        kernel[index++] = parsedValue;
                    }
                }
                return kernel;
            }
        }

    protected:
        ~KernelInputForm()
        {
            if (components)
            {
                delete components;
            }
        }

    private:
        System::Windows::Forms::DataGridView^ dataGridView1;
        System::Windows::Forms::Button^ btnOK;
        System::Windows::Forms::Button^ btnCancel;
        System::ComponentModel::Container^ components;

#pragma region Windows Form Designer generated code
        void InitializeComponent(void)
        {
            this->dataGridView1 = (gcnew System::Windows::Forms::DataGridView());
            this->btnOK = (gcnew System::Windows::Forms::Button());
            this->btnCancel = (gcnew System::Windows::Forms::Button());
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->dataGridView1))->BeginInit();
            this->SuspendLayout();
            // 
            // dataGridView1
            // 
            this->dataGridView1->ColumnHeadersVisible = false;
            this->dataGridView1->RowHeadersVisible = false;
            this->dataGridView1->Location = System::Drawing::Point(12, 12);
            this->dataGridView1->Name = L"dataGridView1";
            this->dataGridView1->Size = System::Drawing::Size(150, 150);
            this->dataGridView1->TabIndex = 0;
            // 
            // btnOK
            // 
            this->btnOK->DialogResult = System::Windows::Forms::DialogResult::OK;
            this->btnOK->Location = System::Drawing::Point(12, 168);
            this->btnOK->Name = L"btnOK";
            this->btnOK->Size = System::Drawing::Size(75, 23);
            this->btnOK->TabIndex = 1;
            this->btnOK->Text = L"OK";
            this->btnOK->UseVisualStyleBackColor = true;
            // 
            // btnCancel
            // 
            this->btnCancel->DialogResult = System::Windows::Forms::DialogResult::Cancel;
            this->btnCancel->Location = System::Drawing::Point(93, 168);
            this->btnCancel->Name = L"btnCancel";
            this->btnCancel->Size = System::Drawing::Size(75, 23);
            this->btnCancel->TabIndex = 2;
            this->btnCancel->Text = L"Cancel";
            this->btnCancel->UseVisualStyleBackColor = true;
            // 
            // KernelInputForm
            // 
            this->AcceptButton = this->btnOK;
            this->AutoScaleDimensions = System::Drawing::SizeF(6, 13);
            this->AutoScaleMode = System::Windows::Forms::AutoScaleMode::Font;
            this->CancelButton = this->btnCancel;
            this->ClientSize = System::Drawing::Size(180, 203);
            this->Controls->Add(this->btnCancel);
            this->Controls->Add(this->btnOK);
            this->Controls->Add(this->dataGridView1);
            this->FormBorderStyle = System::Windows::Forms::FormBorderStyle::FixedDialog;
            this->MaximizeBox = false;
            this->MinimizeBox = false;
            this->Name = L"KernelInputForm";
            this->StartPosition = System::Windows::Forms::FormStartPosition::CenterParent;
            this->Text = L"Enter 3x3 Kernel";
            (cli::safe_cast<System::ComponentModel::ISupportInitialize^>(this->dataGridView1))->EndInit();
            this->ResumeLayout(false);
        }
#pragma endregion

    private:
        void InitializeDataGridView() {
            // Set up 3x3 grid
            dataGridView1->ColumnCount = 3;
            dataGridView1->RowCount = 3;

            // Configure columns and rows
            for (int i = 0; i < 3; i++) {
                dataGridView1->Columns[i]->Width = 50;
                dataGridView1->Rows[i]->Height = 50;
            }

            // Prevent adding new rows or columns
            dataGridView1->AllowUserToAddRows = false;
            dataGridView1->AllowUserToDeleteRows = false;

            // Prefill with default values (e.g., zero kernel)
            array<float>^ defaultKernel = {
                0, 0, 0,
                0, 1, 0,
                0, 0, 0 // Identity-like kernel
            };
            int index = 0;
            for (int row = 0; row < 3; row++) {
                for (int col = 0; col < 3; col++) {
                    dataGridView1->Rows[row]->Cells[col]->Value = defaultKernel[index++].ToString();
                }
            }

            // Center align text in cells
            dataGridView1->DefaultCellStyle->Alignment = DataGridViewContentAlignment::MiddleCenter;
        }
    };
}