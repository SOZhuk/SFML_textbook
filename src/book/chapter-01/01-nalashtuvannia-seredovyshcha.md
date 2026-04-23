# 1. Налаштування середовища

Перед роботою з SFML перевіримо три речі:

1. чи доступний компілятор C++;
2. чи встановлена бібліотека SFML;
3. чи встановлений Visual Studio Code і рекомендовані розширення для C++;
4. чи компілюється і запускається мінімальна програма `Hello, world!`.

## 1. Перевірка компілятора

Відкрийте PowerShell, командний рядок (cmd) або термінал VS Code і виконайте:

```powershell
g++ --version
```

Очікуваний перший рядок:

```text
g++.exe (Rev13, Built by MSYS2 project) 15.2.0
```

Якщо версія збігається, компілятор уже готовий. Якщо команда `g++` не знайдена або версія інша, перевірте, який саме компілятор запускається:

```powershell
where g++
```

Для цього курсу потрібен компілятор із середовища MSYS2 UCRT64. Його шлях зазвичай такий:

```text
C:\msys64\ucrt64\bin
```

### Встановлення MSYS2 і toolchain

Якщо MSYS2 ще не встановлений, встановіть його:

```powershell
winget install --id MSYS2.MSYS2 -e
```

Після встановлення відкрийте з меню Start саме **MSYS2 UCRT64**, не звичайний MSYS2 Shell, або просто запустіть з командного рядку C:\msys64\ucrt64. У цьому вікні виконайте:

```bash
pacman -Syu
```

Якщо MSYS2 попросить закрити термінал після оновлення, закрийте його, знову відкрийте **MSYS2 UCRT64** і виконайте:

```bash
pacman -Syu
pacman -S --needed mingw-w64-ucrt-x86_64-toolchain
```

Після встановлення додайте шлях `C:\msys64\ucrt64\bin` до змінної середовища `Path`.

### Як додати шлях до Path у Windows

Готовий `.cmd` файл нижче може додати `C:\msys64\ucrt64\bin` у користувацький `Path` автоматично. Після цього все одно потрібно закрити і знову відкрити PowerShell, cmd або VS Code, бо вже запущені програми не перечитують змінні середовища.

Якщо автоматичне додавання не спрацювало, додайте шлях вручну:

1. Натисніть `Win`.
2. Знайдіть **Edit the system environment variables**.
3. Натисніть **Environment Variables...**.
4. У блоці **User variables** знайдіть змінну `Path`.
5. Натисніть **Edit**.
6. Натисніть **New**.
7. Додайте шлях `C:\msys64\ucrt64\bin`.
8. Натисніть **OK** у всіх відкритих вікнах.
9. Закрийте і знову відкрийте PowerShell або VS Code.
10. Знову перевірте:

```powershell
g++ --version
where g++
```

:::details Готовий .cmd файл для встановлення MSYS2, toolchain і SFML
Цей файл можна використати для швидкої підготовки нового комп'ютера. Він встановлює MSYS2, компіляторний набір UCRT64, бібліотеку SFML і додає шлях `C:\msys64\ucrt64\bin` у користувацький `Path`.

Збережіть текст у файл, наприклад `install-sfml-env.cmd`, і запустіть його. Після завершення закрийте і знову відкрийте PowerShell, cmd або VS Code.

```bat
@echo off
setlocal

winget install --id MSYS2.MSYS2 -e

set "MSYS2_ROOT=C:\msys64"
set "UCRT64_BIN=%MSYS2_ROOT%\ucrt64\bin"

if not exist "%MSYS2_ROOT%\usr\bin\bash.exe" (
    echo MSYS2 not found in %MSYS2_ROOT%
    exit /b 1
)

set "CHERE_INVOKING=1"
set "MSYSTEM=UCRT64"

"%MSYS2_ROOT%\usr\bin\bash.exe" -lc "pacman -Syu --noconfirm"
"%MSYS2_ROOT%\usr\bin\bash.exe" -lc "pacman -Syu --noconfirm"
"%MSYS2_ROOT%\usr\bin\bash.exe" -lc "pacman -S --needed --noconfirm mingw-w64-ucrt-x86_64-toolchain"
"%MSYS2_ROOT%\usr\bin\bash.exe" -lc "pacman -S --needed --noconfirm mingw-w64-ucrt-x86_64-sfml"

powershell -NoProfile -ExecutionPolicy Bypass -Command ^
  "$pathToAdd = '%UCRT64_BIN%';" ^
  "$current = [Environment]::GetEnvironmentVariable('Path', 'User');" ^
  "if ([string]::IsNullOrWhiteSpace($current)) { $newPath = $pathToAdd }" ^
  "elseif (($current -split ';') -notcontains $pathToAdd) { $newPath = $pathToAdd + ';' + $current }" ^
  "else { $newPath = $current }" ^
  "[Environment]::SetEnvironmentVariable('Path', $newPath, 'User')"

set "PATH=%UCRT64_BIN%;%PATH%"
where g++
g++ --version

echo.
echo Done.
echo Restart PowerShell, cmd, and VS Code so they can read the updated Path.
```

Якщо після першої команди `pacman -Syu` MSYS2 просить закрити вікно, закрийте його і запустіть `.cmd` файл ще раз.
:::

## 2. Встановлення бібліотеки SFML

Цей крок потрібен, якщо ви будете збирати SFML-проєкт локально через MSYS2 UCRT64 або через підготовлений викладачем шаблон, який очікує встановлену бібліотеку в системі.

Якщо ви створюєте проєкт через клон офіційного репозиторію `SFML/cmake-sfml-project` і CMake, окремо встановлювати SFML через `pacman` не обов'язково: CMake-шаблон може завантажити і підключити бібліотеку під час налаштування проєкту.

Для встановлення SFML відкрийте **MSYS2 UCRT64** і виконайте:

```bash
pacman -S --needed mingw-w64-ucrt-x86_64-sfml
```

Для повністю автоматичного встановлення можна використати:

```bash
pacman -S --needed --noconfirm mingw-w64-ucrt-x86_64-sfml
```

## 3. Перевірка Visual Studio Code

Знайдіть значок **Visual Studio Code** у меню Start або спробуйте запустити редактор із командного рядка:

```powershell
code
```

Якщо VS Code не встановлений, встановіть його через `winget`:

```powershell
winget install --id Microsoft.VisualStudioCode -e
```

Після встановлення закрийте і знову відкрийте PowerShell. Перевірте:

```powershell
code --version
```

### Рекомендоване розширення для C++

У VS Code встановіть **C/C++ Extension Pack** від Microsoft. Ідентифікатор розширення:

```text
ms-vscode.cpptools-extension-pack
```

Його можна встановити через вкладку Extensions або командою:

```powershell
code --install-extension ms-vscode.cpptools-extension-pack
```

Цей пакет додає підтримку C/C++, підказки коду, налагодження та інструменти, які знадобляться для роботи з проєктами C++.

## 4. Компіляція і запуск Hello World

Створіть окрему папку для перевірки, наприклад:

```powershell
mkdir cpp-check
cd cpp-check
code .
```

У VS Code створіть файл `main.cpp` із таким кодом:

```cpp
#include <iostream>

int main() {
    std::cout << "Hello, world!" << std::endl;
    return 0;
}
```

У терміналі VS Code скомпілюйте програму:

```powershell
g++ main.cpp -o main.exe
```

Запустіть її:

```powershell
.\main.exe
```

Очікуваний результат:

```text
Hello, world!
```

Якщо програма компілюється і запускається, середовище готове до роботи з навчальним проєктом SFML.
