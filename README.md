# ✨ Files Sphere  

A fancy **file system** with a **game-inspired GUI**, running entirely inside the Windows **CMD**.  
Unlike typical console apps, this project delivers smooth **animations, transitions, and user interactions** using only a **basic graphics library** that works directly with pixels.  

---

## 🎮 Features  
- **Fancy GUI inside CMD** — looks like a mini-game, not a boring console app.  
- **Smooth Animations** — built by generating thousands of frames from custom videos using `ffmpeg`.  
- **GraphicalObject Class** — custom engine that animates objects & videos frame by frame with smooth transitions.  
- **User Interaction**  
  - Mouse support (coordinates + clicks)  
  - Keyboard input  
  - Disabled default CMD behavior to protect graphics.  
- **Modern Windows Integration**  
  - Friendly dialogs (file saving, opening, etc.)  
  - MessageBox pop-ups for user feedback.  
- **File Operations Made Fancy** ✨  
  - Create files  
  - Read files  
  - Delete files  
  - All with cool animations & effects.  

---

## 🛠️ Tech Stack  
- **Language**: C / C++  
- **Graphics**: Custom pixel-based graphics library  
- **Tools**: `ffmpeg` (for generating animation frames)  
- **Platform**: Windows CMD  

---

## 🚀 How It Works  
1. Videos → converted into frames with **ffmpeg**.  
2. Frames → animated via `GraphicalObject` class.  
3. CMD → turned into a game-like environment with mouse & keyboard support.  
4. User → manages files with style (create/read/delete).  

---

## 📸 Demo  
👉 Screenshots / GIFs coming soon.  

---

## 📁 File Structure

| Folder / File                       | Description                                      |
| ----------------------------------- | ------------------------------------------------ |
| `Codveda_9095efb7/`                 | Project-related intermediate/generated files     |
| `Debug/`                            | Debug build output                               |
| `Images/`                           | Contains project image assets                    |
| ├── `Animation/`                    | Images related to animations                     |
| ├── `Deleting/`                     | Images for delete operations                     |
| ├── `Entrance/`                     | Entrance screen images                           |
| ├── `Main_Menu/`                    | Main menu images                                 |
| ├── `Reading/`                      | Reading process images                           |
| ├── `Return/`                       | Return screen images                             |
| └── `Writting_the_file/`            | File writing related images                      |
| `x64/Debug/`                        | 64-bit debug build output                        |
| `.gitignore`                        | Ignored files configuration                      |
| `CC212SGL.dll / .h / .lib`          | External graphics library files                  |
| `Codveda_Simple_File_System.sln`    | Visual Studio solution file                      |
| `Codveda_Simple_File_System.vcxproj*` | Visual Studio project configuration files      |
| `FreeImage.dll / .h / .lib`         | FreeImage library for image processing           |
| `Task1.cpp`                         | Main C++ source file                             |


## ⚡ Getting Started  
```bash
# Clone the repository
git clone https://github.com/The-Mastermind-of-this-gen/Files_Sphere.git

# Build with Visual Studio or g++
# Example (if g++ works with your setup):
g++ main.cpp -o FilesSphere.exe
