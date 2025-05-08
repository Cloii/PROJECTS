//MIIIIIGOOOOOOOOO
#include "MainForm.h"

using namespace System;
using namespace System::Windows::Forms;

[STAThread] // This attribute is required for Windows Forms applications in C++/CLI
int main(array<System::String^>^ args) {
    System::Windows::Forms::Application::EnableVisualStyles();
    System::Windows::Forms::Application::SetCompatibleTextRenderingDefault(false);
    proj1::MainForm form;
    System::Windows::Forms::Application::Run(% form);
    return 0;
}
