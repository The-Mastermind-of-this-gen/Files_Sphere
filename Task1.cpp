#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

#include <sstream>
#include <shobjidl.h> 
#include <objbase.h>
#include<Windows.h>
#include <fstream>
#include<iostream>
#include<mmsystem.h>
#include<playsoundapi.h>
#include "CC212SGL.h"
#include <conio.h>

using namespace std;
#pragma comment(lib, "winmm.lib")
//Link to the Graphics .lib file
#pragma comment(lib, "CC212SGL.lib")
//Declare instance from the Graphics 
CC212SGL gr;

//Screen coordinates
long long Screen_X_Middle; 
long long Screen_Y_Middle; 


// it stops the interactions between the CMD and the mouse so graphics is not disturbed
void DisableQuickEdit() {
    HANDLE hStdin = GetStdHandle(STD_INPUT_HANDLE);

    DWORD prev_mode;
    GetConsoleMode(hStdin, &prev_mode);

    // Remove QuickEdit Mode and Insert Mode
    DWORD new_mode = prev_mode & ~ENABLE_QUICK_EDIT_MODE;
    new_mode &= ~ENABLE_INSERT_MODE;

    // Enable Extended Flags (required to make changes stick)
    new_mode |= ENABLE_EXTENDED_FLAGS;

    SetConsoleMode(hStdin, new_mode);
}

//hide the blinking text cursor (caret) in CMD to prevent graphics disturption
void HideCursor(bool ShowCursor = false)
{
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_CURSOR_INFO cursorInfo;

    GetConsoleCursorInfo(hConsole, &cursorInfo); // get current cursor info
    cursorInfo.bVisible = ShowCursor;                 // hide the cursor
    SetConsoleCursorInfo(hConsole, &cursorInfo); // apply changes
}

// Deleting function using system (Just moving to recycle bin I don't want a risk)
bool MoveFileToRecycleBin(const std::wstring& filePath) {
    SHFILEOPSTRUCTW fileOp = { 0 };

    wchar_t from[MAX_PATH] = { 0 };
    wcsncpy_s(from, filePath.c_str(), MAX_PATH - 1);
    from[filePath.size() + 1] = L'\0'; // double null

    fileOp.wFunc = FO_DELETE;
    fileOp.pFrom = from;
    fileOp.fFlags = FOF_ALLOWUNDO | FOF_NOCONFIRMATION | FOF_SILENT;

    return (SHFileOperationW(&fileOp) == 0);
}

// file selection
bool OpenFileDialog(char* filePath, DWORD size) {
    OPENFILENAMEA ofn;
    ZeroMemory(&ofn, sizeof(ofn)); // Initialize all fields to 0
    ofn.lStructSize = sizeof(ofn);
    ofn.lpstrFile = filePath;
    ofn.nMaxFile = size;
    ofn.lpstrFilter =
        "All Readable Files\0*.txt;*.pdf;*.doc;*.docx;*.xls;*.xlsx;*.csv\0"
        "Text Files\0*.txt\0"
        "PDF Files\0*.pdf\0"
        "Word Documents\0*.doc;*.docx\0"
        "Excel Files\0*.xls;*.xlsx\0"
        "CSV Files\0*.csv\0\0";
    ofn.nFilterIndex = 1;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

    return GetOpenFileNameA(&ofn);
}

//folder selection
bool OpenFolderDialog(char* folderPath, DWORD size)
{
     IFileDialog* pfd = nullptr;
    
     // Create the FileOpenDialog COM object
     HRESULT hr = CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_INPROC_SERVER,
         IID_PPV_ARGS(&pfd));
     if (FAILED(hr)) return false;
    
     // Set the dialog options
     DWORD options;
     pfd->GetOptions(&options);
     pfd->SetOptions(options | FOS_PICKFOLDERS);  // Folder picker mode
    
     // Show the dialog
     hr = pfd->Show(NULL);
     if (SUCCEEDED(hr)) {
         IShellItem* pItem;
         hr = pfd->GetResult(&pItem);
         if (SUCCEEDED(hr)) {
             PWSTR pszFilePath = nullptr;
             hr = pItem->GetDisplayName(SIGDN_FILESYSPATH, &pszFilePath);
             if (SUCCEEDED(hr)) {
                 // Convert wide string to ANSI
                 WideCharToMultiByte(CP_ACP, 0, pszFilePath, -1, folderPath, size, NULL, NULL);
                 CoTaskMemFree(pszFilePath);
                 pItem->Release();
                 pfd->Release();
                 return true;
             }
             pItem->Release();
         }
     }
     pfd->Release();
     return false;
}

string CreateFileAtPath(const string& folderPath, const string& fileName) {
    
    string fullPath = folderPath + "\\" + fileName+".txt";

    ofstream file(fullPath);  
    if (!file.is_open())
    {
        MessageBoxA(NULL, "Failed to create file!", "Error", MB_OK | MB_ICONERROR);
        return "";
    }
    file.close();

    MessageBoxA(NULL, ("File created at: " + fullPath).c_str(), "Successfully", MB_OK);
    return fullPath;
}

void waitFor(int start, int threshold)
{
	while (true)
	{
		float _t = (clock() / (float)CLOCKS_PER_SEC * 1000.0f - start / (float)CLOCKS_PER_SEC * 1000.0f);
		if (_t >= threshold)
			break;
	}
}

class GraphicalObject 
{
    int* Sprites;
    int TotalFramesCount = -1;
    int CurrentFrame = 0;

    int x = 0 , y = 0;

public:
    GraphicalObject(int sprites  = 1, int totalFramesCount = 1)
    {
        Sprites = new int[sprites];
        TotalFramesCount = totalFramesCount;
    }

    void loadFrames(const char path[],int imageWidth,int imageHight)
    {
        x = imageWidth / 2, y = imageHight / 2;
        if (TotalFramesCount > 1)
        {
            for (int i = 1;i <= TotalFramesCount;i++) 
            {
                char Final_Loc[200];//maximum path size
                char num[10];

                itoa(i, num, 10);// conversion from int to string ascii
            
                strcpy(Final_Loc, path);// Final_Loc = path
                strcat(Final_Loc, num); // Final_Loc += num
                strcat(Final_Loc, ".png");// Final_Loc += ".png"

                Sprites[i-1] = gr.loadImage(Final_Loc);
            }
        }
        else 
        {
            char Final_Loc[200];//maximum path size
            strcpy(Final_Loc, path);// Final_Loc = path
            strcat(Final_Loc, ".png");// Final_Loc += ".png"
            Sprites[0] = gr.loadImage(Final_Loc);
        }
    }
    void AnimateVid(int HorizontalShift = 0,int VirticalShift = 0,bool Reversed = false)
    {
        if (Reversed == false)
        {
            for (int i = 0;i < TotalFramesCount;i++)
            {
                int frame_start = clock();

                gr.beginDraw();

                gr.drawImage(Sprites[i], Screen_X_Middle - x + HorizontalShift, Screen_Y_Middle - y + VirticalShift, gr.generateFromRGB(0, 0, 0));

                gr.endDraw();

                waitFor(frame_start, 1.0 / 30 * 1000);
            }
        }
        else if (Reversed == true) 
        {
            for (int i = TotalFramesCount;i > 0;i--)
            {
                int frame_start = clock();

                gr.beginDraw();

                gr.drawImage(Sprites[i], Screen_X_Middle - x + HorizontalShift, Screen_Y_Middle - y + VirticalShift, gr.generateFromRGB(0, 0, 0));

                gr.endDraw();

                waitFor(frame_start, 1.0 / 30 * 1000);
            }
        }
    }
    void AnimateObject(int HorizontalShift = 0, int VirticalShift = 0,int R = 0,int G = 0,int B = 0)
    {
        if (TotalFramesCount > 1 && CurrentFrame != TotalFramesCount) 
        {
            gr.beginDraw();
            gr.drawImage(Sprites[CurrentFrame++], Screen_X_Middle - x + HorizontalShift, Screen_Y_Middle - y + VirticalShift, gr.generateFromRGB(R, G, B));
            gr.endDraw();
        }
        
        else if (TotalFramesCount > 1 && CurrentFrame == TotalFramesCount) 
        {
            CurrentFrame = 0;
            gr.beginDraw();
            gr.drawImage(Sprites[CurrentFrame++], Screen_X_Middle - x + HorizontalShift, Screen_Y_Middle - y + VirticalShift, gr.generateFromRGB(R, G, B));
            gr.endDraw();
        }
        
        else 
        {
            gr.drawImage(Sprites[0], Screen_X_Middle - x + HorizontalShift, Screen_Y_Middle - y + VirticalShift, gr.generateFromRGB(R, G, B));
        }
    }

};


//########### Prototypes ############
void DELETEING(GraphicalObject* DeletingAnimation, GraphicalObject* DeletingList);
void Reading(GraphicalObject* DeletingAnimation, GraphicalObject* DeletingList, GraphicalObject* ReadingAnimation, GraphicalObject* ReadingList);
void Creating(GraphicalObject* ReadingList, GraphicalObject* ReadingAnimation, GraphicalObject* WrittingAnimation, GraphicalObject* WrittingMenu, GraphicalObject* ReturnVid);
void WrittingFile(GraphicalObject* WrittingMenu,string fullpath, GraphicalObject* ReturnVid);
//###################################

void LoadingScreen(GraphicalObject* MenuList, GraphicalObject* DeletingAnimation,GraphicalObject* MenuBackground, GraphicalObject* DeletingList, GraphicalObject* ReadingAnimation, GraphicalObject* ReadingList, GraphicalObject* WrittingAnimation, GraphicalObject* WrittingMenu, GraphicalObject* ReturnVid)
{
    int Entrance = gr.loadImage("Images//Entrance//AfterLoading//loadingTransitionVid1.png");
    int LoadingRecangle = 0;
   
    GraphicalObject* FromLoadingTransition = new GraphicalObject(67,67);
    while (true) 
    {
        int frame_start = clock();
        gr.beginDraw();

        gr.drawImage(Entrance,(Screen_X_Middle - 1920/2), (Screen_Y_Middle - 1080 / 2),gr.generateFromRGB(0,0,0));
        
        gr.setDrawingColor(gr.generateFromRGB(20,10,8));
        gr.drawSolidRectangle(Screen_X_Middle-500,Screen_Y_Middle+500,1000,30);
        gr.setDrawingColor(gr.generateFromRGB(202,126,50));
        gr.drawSolidRectangle(Screen_X_Middle-490,Screen_Y_Middle+508, LoadingRecangle,14);
       
        gr.endDraw();

        if (LoadingRecangle >= 980)
            break;
        else if (LoadingRecangle == 20)
        {
            DeletingList->loadFrames("Images//Deleting//frame_63", 1920, 1080);
            MenuBackground->loadFrames("Images//Main_Menu//MenuListApearAnimate", 1980, 1080);
            ReadingList->loadFrames("Images//Reading//Reading62", 1980, 1080);
            WrittingMenu->loadFrames("Images//Writting_the_file//Frame59", 1980, 1080);
            LoadingRecangle += 10;
        }
        else if (LoadingRecangle == 250) 
        {
            FromLoadingTransition->loadFrames("Images//Entrance//AfterLoading//loadingTransitionVid",1920,1080);
            LoadingRecangle += 10;
        }
        else if (LoadingRecangle == 980/2) 
        {
            MenuList->loadFrames("Images//Main_Menu//MenuListApearAnimate", 1980, 1080);
            LoadingRecangle += 10;
        }
        else if (LoadingRecangle == 600)
        {
            WrittingAnimation->loadFrames("Images//Writting_the_file//Frame", 1980, 1080);
            LoadingRecangle += 10;
        }
        else if (LoadingRecangle == 740) 
        {
            DeletingAnimation->loadFrames("Images//Deleting//frame_", 1980, 1080);
            ReturnVid->loadFrames("Images//Return//Frame",1980,1080);
            LoadingRecangle += 10;
        }
        else if (LoadingRecangle == 900) 
        {
            ReadingAnimation->loadFrames("Images//Reading//Reading", 1980, 1080);
            LoadingRecangle += 10;
        }
        else 
            LoadingRecangle += 10;

        waitFor(frame_start, 1.0 / 30 * 1000);
    }

    FromLoadingTransition->AnimateVid();
}

int MainMenu(GraphicalObject* MenuList, GraphicalObject* DeletingAnimation, GraphicalObject* MenuBackground, GraphicalObject* DeletingList, GraphicalObject* ReadingAnimation, GraphicalObject* ReadingList, GraphicalObject* WrittingAnimation, GraphicalObject* WrittingMenu, GraphicalObject* ReturnVid)
{
    MenuList->AnimateVid(30,0);
    int choice = 1;
    //RGB Choice Regulation
    short RC, GC, BC, RR, GR, BR, RD, GD, BD,RE,GE,BE;

    while (true)
    {

        int frame_start = clock();
        
        gr.beginDraw();
        MenuBackground->AnimateObject(30, 0);
        
        if (choice == 1)
            RC = 255, GC = 255, BC = 255, RR = 0, GR = 0, BR = 0, RD = 0, GD = 0, BD =0,RE = 0,GE = 0,BE = 0;
        else if(choice == 2)
            RC = 0, GC = 0, BC = 0, RR = 255, GR = 255, BR = 255, RD = 0, GD = 0, BD = 0,RE = 0, GE = 0, BE = 0;
        else if(choice == 3)
            RC = 0, GC = 0, BC = 0, RR = 0, GR = 0, BR = 0, RD = 255, GD = 255, BD = 255, RE = 0, GE = 0, BE = 0;
        else if(choice == 4)
            RC = 0, GC = 0, BC = 0, RR = 0, GR = 0, BR = 0, RD = 0, GD = 0, BD = 0, RE = 255, GE = 50, BE = 50;
        
        //choice decide
        if(_kbhit()) 
        {
            int c = _getch();

            if (c == 224) { // special key (arrows, F keys, etc.)
                c = _getch(); // get the actual key code
                switch (c) {
                case 72: // Up arrow
                    if (choice > 1)
                        choice--;
                    break;
                case 80: // Down arrow
                    if (choice < 4)
                        choice++;
                    break;
                }
            }
            else if (c == 13) { // Enter key
                if (choice == 4) {
                    HWND hwnd = GetConsoleWindow();
                    PostMessage(hwnd, WM_CLOSE, 0, 0); // closes window
                }
                else if (choice == 3)
                {
                    DELETEING(DeletingAnimation, DeletingList);
                }
                else if (choice == 2)
                {
                    Reading(DeletingAnimation, DeletingList,ReadingAnimation,ReadingList);
                }
                else if (choice == 1)
                {
                    Creating(ReadingList,ReadingAnimation,WrittingAnimation,WrittingMenu, ReturnVid);
                }
            }
        }
        // Create A file
        gr.setFontSizeAndBoldness(50, 20000);
        gr.setDrawingColor(gr.generateFromRGB(RC, GC, BC));
        gr.drawText(Screen_X_Middle - 120, Screen_Y_Middle-150 , "Create a file");

        // Read a file
        gr.setDrawingColor(gr.generateFromRGB(RR, GR, BR));
        gr.drawText(Screen_X_Middle - 105, Screen_Y_Middle - 55, "Read a file");

        // Delete a file
        gr.setDrawingColor(gr.generateFromRGB(RD, GD, BD));
        gr.drawText(Screen_X_Middle - 115, Screen_Y_Middle + 45, "Delete a file");
        
        // Exit Program
        gr.setDrawingColor(gr.generateFromRGB(RE, GE, BE));
        gr.drawText(Screen_X_Middle - 42, Screen_Y_Middle + 140, "Exit");


        //My Signature
        gr.setFontSizeAndBoldness(40,20);
        gr.setDrawingColor(gr.generateFromRGB(255,255,255));
        gr.drawText(Screen_X_Middle - 212 ,Screen_Y_Middle*2- 140,"Created and developed By\n :Mohamed Hazem Tawfik");
        
        gr.endDraw();
        waitFor(frame_start, 1.0 / 30 * 1000);
    }
    return 0;
}

string Writting(int x = Screen_X_Middle, int y = Screen_Y_Middle,GraphicalObject* ReadingList = NULL,int KEY=0, string userInput = "")
{
    
    string shown;
    if (userInput.length() > 27)
        shown = userInput.substr(userInput.length() - 27);
    else
        shown = userInput;
    int CursorCounter = 0;

    while (true)
    {
        gr.beginDraw();
        if (KEY == 1) 
        {
            ReadingList->AnimateObject(30, 0);
            gr.setFontSizeAndBoldness(50, 20000);
            gr.setDrawingColor(gr.generateFromRGB(0, 0, 0));
            gr.drawText(Screen_X_Middle - 160, Screen_Y_Middle - 100, "Create your File");
            gr.setFontSizeAndBoldness(30, 50);
            gr.drawText(Screen_X_Middle - 385, Screen_Y_Middle - 10, "Enter File name here:");
            //inputs
            gr.setDrawingColor(gr.generateFromRGB(0, 0, 0));
            gr.drawSolidRectangle(Screen_X_Middle - 400, Screen_Y_Middle + 20, 800, 70);
            gr.setDrawingColor(gr.generateFromRGB(255, 255, 255));
            gr.drawSolidRectangle(Screen_X_Middle - 385, Screen_Y_Middle + 25, 770, 60);   
            gr.setDrawingColor(gr.generateFromRGB(96, 38, 16));
            gr.drawSolidRectangle(Screen_X_Middle + 270, Screen_Y_Middle + 25, 115, 60);
            gr.setDrawingColor(gr.generateFromRGB(255, 255, 255));
            gr.setFontSizeAndBoldness(50, 20000);
            gr.drawText(Screen_X_Middle + 290, Screen_Y_Middle + 30, ".txt *");
            //button
            gr.setDrawingColor(gr.generateFromRGB(96, 38, 16));
            gr.drawSolidCircle(Screen_X_Middle - 160, Screen_Y_Middle + 120, 60);
            gr.drawSolidCircle(Screen_X_Middle + 90, Screen_Y_Middle + 120, 60);
            gr.drawSolidRectangle(Screen_X_Middle - 130, Screen_Y_Middle + 120, 250, 60);
            gr.setDrawingColor(gr.generateFromRGB(255, 255, 255));
            gr.setFontSizeAndBoldness(35, 60);
            gr.drawText(Screen_X_Middle - 150, Screen_Y_Middle + 132, "Choose File Location");

        }

        int frame_start = clock();
        gr.setDrawingColor(gr.generateFromRGB(0, 0, 0));
        gr.setFontSizeAndBoldness(50,60);

        string mess = "";
        if (CursorCounter > 15 && CursorCounter <= 30) 
        {
            mess = shown;
        }
        else if (CursorCounter <= 15) 
        {
            mess = shown + '|';
        }
        else 
        {
            mess = shown ;
            CursorCounter = 0;
        }
        CursorCounter++;

        gr.drawText(x, y, mess.c_str());

        // Check for key press
        if (_kbhit())
        {
            char c = _getch();
            if (c == 8) // Backspace
            {
                if (!userInput.empty())
                {
                    userInput.pop_back();

                    // Keep only last 27 characters in shown
                    if (userInput.length() > 27)
                        shown = userInput.substr(userInput.length() - 27);
                    else
                        shown = userInput;
                }
            }
            else if (c == 13) // Enter
            {
                return userInput;
            }
            else
            {
                userInput += c;

                // Keep only last 27 characters in shown
                if (userInput.length() > 27)
                    shown = userInput.substr(userInput.length() - 27);
                else
                    shown = userInput;
            }
        }
        gr.setDrawingColor(gr.generateFromRGB(0, 0, 0));
        gr.drawText(Screen_X_Middle - 420, Screen_Y_Middle + 240, "Return to main menu");
        gr.drawText(Screen_X_Middle + 100, Screen_Y_Middle + 240,"Create the file");
        gr.endDraw();
        waitFor(frame_start, 1.0 / 30 * 1000);
        
    }
}

int main()
{
	gr.setup();
	gr.setFullScreenMode(); 
	system("CLS"); // Clear the cmd
    DisableQuickEdit();
    HideCursor();
    CoInitialize(NULL);
    //Screen center coordinates
    Screen_X_Middle = gr.getWindowWidth() / 2;
    Screen_Y_Middle = gr.getWindowHeight() / 2;

    //Objects Decleration
    GraphicalObject* MenuList = new GraphicalObject(67,67);
    GraphicalObject* DeletingAnimation = new GraphicalObject(63, 63);
    GraphicalObject* ReadingAnimation = new GraphicalObject(62, 62);
    GraphicalObject* WrittingAnimation = new GraphicalObject(59, 59);
    GraphicalObject* ReturnVid = new GraphicalObject(61, 61);
    GraphicalObject* MenuBackground = new GraphicalObject(1, 1);
    GraphicalObject* DeletingList = new GraphicalObject(1, 1);
    GraphicalObject* ReadingList = new GraphicalObject(1, 1);
    GraphicalObject* WrittingMenu = new GraphicalObject(1, 1);

    LoadingScreen(MenuList, DeletingAnimation, MenuBackground, DeletingList, ReadingAnimation,ReadingList,WrittingAnimation, WrittingMenu, ReturnVid);

	while (true)
	{
		int frame_start = clock();
        MainMenu(MenuList, DeletingAnimation,MenuBackground,DeletingList, ReadingAnimation,ReadingList,WrittingAnimation, WrittingMenu, ReturnVid);

		gr.beginDraw();

		gr.endDraw();

        //Time Control End 
		waitFor(frame_start, 1.0 / 30 * 1000);
	}
    CoUninitialize();
	return 0;
}

void DELETEING(GraphicalObject* DeletingAnimation, GraphicalObject* DeletingList)
{
    gr.endDraw();
    DeletingAnimation->AnimateVid(30, 0);
    gr.beginDraw();

    DeletingList->AnimateObject(0,0);
    gr.setFontSizeAndBoldness(50,20000);
    gr.setDrawingColor(gr.generateFromRGB(0, 0, 0));
    gr.drawText(Screen_X_Middle + 440, Screen_Y_Middle +80, "Deleting a File \n is in progress\n        ......");
    gr.endDraw();
    

    char filePath[MAX_PATH] = { 0 };
    if (OpenFileDialog(filePath, MAX_PATH))
    {
        std::wstring wsFilePath(filePath, filePath + strlen(filePath));
        if (MoveFileToRecycleBin(wsFilePath))
        {
                gr.beginDraw();

                DeletingList->AnimateObject(0,0);
                gr.setFontSizeAndBoldness(50, 20000);
                gr.setDrawingColor(gr.generateFromRGB(0, 0, 0));
                gr.drawText(Screen_X_Middle + 420, Screen_Y_Middle + 90, "  File is Deleted");
                gr.setDrawingColor(gr.generateFromRGB(0, 150, 0));
                gr.drawText(Screen_X_Middle + 430, Screen_Y_Middle + 130, "   successfully");
            
                gr.endDraw();
                Sleep(1100);
            
            
        }
        else
        {
            gr.beginDraw();

            DeletingList->AnimateObject(0, 0);
            gr.setFontSizeAndBoldness(50, 20000);
            gr.setDrawingColor(gr.generateFromRGB(150, 0, 0));
            gr.drawText(Screen_X_Middle + 420, Screen_Y_Middle + 90, "  Faild to delete\n    the file");

            gr.endDraw();
            Sleep(1100);
        }
    }
    else
    {
        gr.beginDraw();

        DeletingList->AnimateObject(0, 0);
        gr.setFontSizeAndBoldness(50, 20000);
        gr.setDrawingColor(gr.generateFromRGB(150, 0, 0));
        gr.drawText(Screen_X_Middle + 420, Screen_Y_Middle + 90, "   Faild to open\n        the file");

        gr.endDraw();
        Sleep(1100);
    }
    
    DeletingAnimation->AnimateVid(30, 0,true);
}

void Reading(GraphicalObject* DeletingAnimation, GraphicalObject* DeletingList, GraphicalObject* ReadingAnimation, GraphicalObject* ReadingList)
{

    gr.endDraw();
    DeletingAnimation->AnimateVid(30, 0);
    gr.beginDraw();

    DeletingList->AnimateObject(0, 0);
    gr.setFontSizeAndBoldness(50, 20000);
    gr.setDrawingColor(gr.generateFromRGB(0, 0, 0));
    gr.drawText(Screen_X_Middle + 425, Screen_Y_Middle + 80, "Opening the File \n   is in progress\n        ......");
    gr.endDraw();


    char filePath[MAX_PATH] = { 0 };
    if (OpenFileDialog(filePath, MAX_PATH))
    {
        DeletingAnimation->AnimateVid(30, 0, true);
        ReadingAnimation->AnimateVid(30,0);

        gr.beginDraw();

        ReadingList->AnimateObject(30, 0);
        gr.setFontSizeAndBoldness(50, 20000);
        gr.setDrawingColor(gr.generateFromRGB(0, 0, 0));
        gr.drawText(Screen_X_Middle - 335, Screen_Y_Middle - 70, "The File is now oppend for reading");
        gr.setDrawingColor(gr.generateFromRGB(0, 150, 0));
        gr.drawText(Screen_X_Middle - 150, Screen_Y_Middle, "       Enjoy! ;) ");

        gr.endDraw();

        if (ShellExecuteA(NULL, "open", filePath, NULL, NULL, SW_SHOWNORMAL));
        else
        {
            gr.beginDraw();

            ReadingList->AnimateObject(30, 0);
            gr.setFontSizeAndBoldness(50, 20000);
            gr.setDrawingColor(gr.generateFromRGB(150, 0, 0));
            gr.drawText(Screen_X_Middle + 420, Screen_Y_Middle + 90, "  Faild to read\n    the file");

            gr.endDraw();
            Sleep(1100);
            ReadingAnimation->AnimateVid(30, 0, true);
        }
        while (true) 
        {
            gr.beginDraw();

            ReadingList->AnimateObject(30, 0);
            gr.setFontSizeAndBoldness(50, 20000);
            gr.setDrawingColor(gr.generateFromRGB(0, 0, 0));
            gr.drawText(Screen_X_Middle - 335, Screen_Y_Middle - 70, "The File is now oppend for reading");
            gr.setDrawingColor(gr.generateFromRGB(0, 150, 0));
            gr.drawText(Screen_X_Middle -150, Screen_Y_Middle, "       Enjoy! ;) ");
            gr.setDrawingColor(gr.generateFromRGB(0, 0, 150));
            gr.drawText(Screen_X_Middle - 280, Screen_Y_Middle+200, "To return to main menu press");
            gr.drawText(Screen_X_Middle -150, Screen_Y_Middle+250, "       ENTER ");

            gr.endDraw();

            if (_kbhit())
            {
                int c = _getch();

                if (c == 13)
                {
                    ReadingAnimation->AnimateVid(30, 0, true);
                    return;
                }
                
            }
        }
    }
    else
    {
        gr.beginDraw();

        DeletingList->AnimateObject(0, 0);
        gr.setFontSizeAndBoldness(50, 20000);
        gr.setDrawingColor(gr.generateFromRGB(150, 0, 0));
        gr.drawText(Screen_X_Middle + 420, Screen_Y_Middle + 90, "   Faild to open\n        the file");

        gr.endDraw();
        Sleep(1100);
        DeletingAnimation->AnimateVid(30, 0, true); 
    }

}

void Creating(GraphicalObject* ReadingList, GraphicalObject* ReadingAnimation, GraphicalObject* WrittingAnimation, GraphicalObject* WrittingMenu, GraphicalObject* ReturnVid)
{
    ReadingAnimation->AnimateVid(30, 0);
    string fileName = "";
    char folderPath[MAX_PATH] = { 0 };
    int choice = 1;
    bool fileCreated = false;
    string fullpath;
    while (true) 
    {
        gr.beginDraw();
        ReadingList->AnimateObject(30,0);
        gr.setFontSizeAndBoldness(50, 20000);
        gr.setDrawingColor(gr.generateFromRGB(0, 0, 0));
        gr.drawText(Screen_X_Middle - 160, Screen_Y_Middle - 100, "Create your File");
        gr.setFontSizeAndBoldness(30, 50);
        gr.drawText(Screen_X_Middle - 385, Screen_Y_Middle - 10, "Enter File name here:");
        //inputs
        gr.setDrawingColor(gr.generateFromRGB(0,0,0));
        gr.drawSolidRectangle(Screen_X_Middle - 400, Screen_Y_Middle+20, 800, 70);
        gr.setDrawingColor(gr.generateFromRGB(255,255,255));
        gr.drawSolidRectangle(Screen_X_Middle - 385, Screen_Y_Middle+25, 770, 60);
        gr.setDrawingColor(gr.generateFromRGB(96, 38,16));
        gr.drawSolidRectangle(Screen_X_Middle +270, Screen_Y_Middle + 25, 115, 60);
        gr.setDrawingColor(gr.generateFromRGB(255, 255,255));
        gr.setFontSizeAndBoldness(50, 20000);
        gr.drawText(Screen_X_Middle +290, Screen_Y_Middle + 30, ".txt *");
        gr.setDrawingColor(gr.generateFromRGB(0, 0,0));
        string shown;
        if (fileName.length() > 27)
            shown = fileName.substr(fileName.length() - 27);
        else shown = fileName;
        gr.drawText(Screen_X_Middle - 380, Screen_Y_Middle + 30, shown.c_str());

        POINT cursorPos;
        GetCursorPos(&cursorPos);
        if ((GetAsyncKeyState(VK_LBUTTON) & 0x8000) && (cursorPos.x>Screen_X_Middle-400 && cursorPos.x < Screen_X_Middle + 400) && ((cursorPos.y > Screen_Y_Middle +20 && cursorPos.y < Screen_Y_Middle + 90)))
        {
            if(!fileCreated)
                fileName = Writting(Screen_X_Middle - 380, Screen_Y_Middle + 30,ReadingList,1,fileName);
        }
        else if ((GetAsyncKeyState(VK_LBUTTON) & 0x8000) && (cursorPos.x > Screen_X_Middle - 160 && cursorPos.x < Screen_X_Middle + 150) && ((cursorPos.y > Screen_Y_Middle + 120 && cursorPos.y < Screen_Y_Middle + 180))) 
        {
            if (!fileCreated) 
            {
                if (OpenFolderDialog(folderPath, MAX_PATH))
                {
                    MessageBoxA(NULL, folderPath, "Selected Folder", MB_OK);
                }
            }
        }
        else if ((cursorPos.x > Screen_X_Middle - 160 && cursorPos.x < Screen_X_Middle +150 ) && ((cursorPos.y > Screen_Y_Middle +120 && cursorPos.y < Screen_Y_Middle + 180)))
        {
            gr.setDrawingColor(gr.generateFromRGB(0,0,150));
        }
        else 
        {
            gr.setDrawingColor(gr.generateFromRGB(96, 38, 16));
        }
        gr.drawSolidCircle(Screen_X_Middle - 160, Screen_Y_Middle + 120,60);
        gr.drawSolidCircle(Screen_X_Middle + 90, Screen_Y_Middle + 120,60);
        gr.drawSolidRectangle(Screen_X_Middle - 130, Screen_Y_Middle + 120,250,60);
        gr.setDrawingColor(gr.generateFromRGB(255,255,255));
        gr.setFontSizeAndBoldness(35,60);
        gr.drawText(Screen_X_Middle - 150, Screen_Y_Middle + 132,"Choose File Location");
        
        int R1 = 0, R2 = 0;
        if (choice == 1)
        {
            R1 = 150;
            R2 = 0;
        }
        else if(choice == 2)
        {
            R2 = 150;
            R1 = 0;
        }

        //exiting
        string filecreation;
        !fileCreated ? filecreation = "Create the file" :filecreation = "Write the file";
        if (_kbhit())
        {
            int c = _getch();
            if (c == 224) { // special key (arrows, F keys, etc.)
                c = _getch(); // get the actual key code
                switch (c) 
                {
                    case 75: 
                        choice = 1;
                        break;
                    case 77: 
                        choice = 2;
                        break;
                }
            }
            else if (c == 13 && choice == 1)
            {

                ReadingAnimation->AnimateVid(30, 0,true);
                return;
            }
            else if (c == 13 && choice == 2 && !fileCreated)
            {
                if (fileName == "") 
                {
                    MessageBoxA(NULL,"Give the file a name first","Error ", MB_OK);
                }
                else if (folderPath == "") 
                {
                    MessageBoxA(NULL, "Choose a location for the file", "Error ", MB_OK);
                }
                else 
                {
                    fullpath = CreateFileAtPath(folderPath,fileName);
                    fileCreated = !fileCreated;
                }
            }
            else if (c == 13 && choice == 2 && fileCreated)
            {
                WrittingAnimation->AnimateVid(30,0);
                WrittingFile(WrittingMenu,fullpath, ReturnVid);
                break;
            }
            
        }
        gr.setFontSizeAndBoldness(50,50);
        gr.setDrawingColor(gr.generateFromRGB(R1,0,0));
        gr.drawText(Screen_X_Middle - 420, Screen_Y_Middle + 240,"Return to main menu");
        gr.setDrawingColor(gr.generateFromRGB(R2,0,0));
        gr.drawText(Screen_X_Middle + 100, Screen_Y_Middle + 240, filecreation.c_str());

        gr.endDraw();
    }

    return;
}

void WrittingFile(GraphicalObject* WrittingMenu,string fullpath,GraphicalObject* ReturnVid)
{
    string shown;
    int CursorCounter = 0;
    string userInput;

    while (1) 
    {
        gr.beginDraw();
        WrittingMenu->AnimateObject(30,0);
        gr.setFontSizeAndBoldness(50, 50);

        POINT cursorPos;
        GetCursorPos(&cursorPos);
        int G1 = 38, G2 = 38, B1 = 16, B2 = 16, R1 = 96, R2 = 96;
        if ((GetAsyncKeyState(VK_LBUTTON) & 0x8000) && (cursorPos.x > Screen_X_Middle - 790 && cursorPos.x < Screen_X_Middle - 500) && ((cursorPos.y > Screen_Y_Middle + 10 && cursorPos.y < Screen_Y_Middle + 100)))
        {
            FILE* file = fopen(fullpath.c_str(),"w");
            fwrite(userInput.c_str(), sizeof(char), userInput.size(), file);
            fclose(file);
            MessageBoxA(NULL, "The file is saved successfully","Status", MB_OK);
        }
        else if ((GetAsyncKeyState(VK_LBUTTON) & 0x8000) && (cursorPos.x > Screen_X_Middle - 790 && cursorPos.x < Screen_X_Middle - 500) && ((cursorPos.y > Screen_Y_Middle + 160 && cursorPos.y < Screen_Y_Middle + 250)))
        {
            ReturnVid->AnimateVid(30, 0);
            return;
        }
        else if ((cursorPos.x > Screen_X_Middle - 790 && cursorPos.x < Screen_X_Middle - 500) && ((cursorPos.y > Screen_Y_Middle + 10 && cursorPos.y < Screen_Y_Middle + 100)))
        {
            R1 = 199;
            G1 = 79;
            B1 = 33;
        }
        else if ((cursorPos.x > Screen_X_Middle - 790 && cursorPos.x < Screen_X_Middle - 500) && ((cursorPos.y > Screen_Y_Middle + 160 && cursorPos.y < Screen_Y_Middle + 250)))
        {
            R2 = 199;
            G2 = 79;
            B2 = 33;
        }

        
        gr.setDrawingColor(gr.generateFromRGB(R1, G1, B1));
        gr.drawSolidCircle(Screen_X_Middle - 790, Screen_Y_Middle +10, 90);
        gr.drawSolidCircle(Screen_X_Middle -590, Screen_Y_Middle +10, 90);
        gr.drawSolidRectangle(Screen_X_Middle - 745, Screen_Y_Middle+10 , 200, 90);
        
        gr.setDrawingColor(gr.generateFromRGB(R2, G2, B2));
        gr.drawSolidCircle(Screen_X_Middle - 790, Screen_Y_Middle +160, 90);
        gr.drawSolidCircle(Screen_X_Middle -590, Screen_Y_Middle +160, 90);
        gr.drawSolidRectangle(Screen_X_Middle - 745, Screen_Y_Middle+160 , 200, 90);


        gr.setDrawingColor(gr.generateFromRGB(255, 255, 255));
        gr.setFontSizeAndBoldness(50, 60);
        gr.drawText(Screen_X_Middle - 695, Screen_Y_Middle + 25, "Save");
        gr.drawText(Screen_X_Middle - 745, Screen_Y_Middle + 180, "Main Menu");


        int frame_start = clock();
        gr.setDrawingColor(gr.generateFromRGB(255, 255, 255));
        gr.setFontSizeAndBoldness(50, 60);


        string mess = "";
        if (CursorCounter > 15 && CursorCounter <= 30)
        {
            mess = shown;
        }
        else if (CursorCounter <= 15)
        {
            mess = shown + '|';
        }
        else
        {
            mess = shown;
            CursorCounter = 0;
        }
        CursorCounter++;

        gr.drawText(Screen_X_Middle+50, Screen_Y_Middle - 400, mess.c_str());

        // Check for key press
        if (_kbhit())
        {
            char c = _getch();
            if (c == 8) 
            {
                if (!userInput.empty()) 
                {
                    userInput.pop_back();
                    if(!shown.empty())
                        shown.pop_back();

                    // remove '\n' if it was just added for wrapping
                    if (!shown.empty() && shown.back() == '\n')
                        shown.pop_back();
                }
            }
            else if (c == 13)
            {
                userInput += '\n';
                shown += '\n';
            }
            else
            {
                userInput += c;
                shown += c;

                if (shown.size() % 40 == 0) {
                    bool hasEnter = false;
                    for (int i = 1; i <= 40 && i <= (int)shown.size(); i++) {
                        if (shown[shown.size() - i] == '\n') {
                            hasEnter = true;
                            break;
                        }
                    }
                    if (!hasEnter)
                        shown += '\n';
                }
            }
        }


        gr.endDraw();
        waitFor(frame_start, 1.0 / 30 * 1000);
    }
        return;
}