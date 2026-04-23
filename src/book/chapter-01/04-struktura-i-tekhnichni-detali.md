# 4. Структура і технічні деталі

Почнемо з мінімальної повної програми. Вона відкриває вікно, створює зелене коло, обробляє закриття вікна і кожен кадр перемальовує сцену.

```cpp
#include <SFML/Graphics.hpp>
#include <optional>

int main() {
    sf::RenderWindow window(sf::VideoMode({800, 600}), "SFML 3 works!");
    sf::CircleShape shape(100.f);
    shape.setFillColor(sf::Color::Green);

    while (window.isOpen()) {
        while (const std::optional event = window.pollEvent()) {
            // 1. Обробка подій.
            if (event->is<sf::Event::Closed>()) {
                window.close();
            }
        }

        // 2. Оновлення стану.

        // 3. Малювання кадру.
        window.clear();
        window.draw(shape);
        window.display();
    }
    return 0;
}
```

## Основні типи даних

У SFML багато значень передаються не окремими числами, а невеликими типами-обгортками. Простір імен бібліотеки - `sf`, тому більшість ідентифікаторів використовуються з префіксом `sf::`.

- `sf::Vector2u` - пара цілих беззнакових чисел. Зручно для розмірів: ширина і висота вікна, розмір текстури.
- `sf::Vector2i` - пара цілих чисел зі знаком. Часто використовується для позиції миші у пікселях.
- `sf::Vector2f` - пара чисел з плаваючою крапкою. Зручно для позиції, швидкості, переміщення і розмірів графічних об'єктів.
- `sf::Color` - колір. Наприклад, `sf::Color::Green`, `sf::Color::Black` або власний колір `sf::Color(30, 30, 30)`.
- `sf::Time` - проміжок часу. Його можна отримати з `sf::Clock` і перевести в секунди, мілісекунди або мікросекунди.
- `sf::Event` - подія від операційної системи: закриття вікна, натискання клавіші, рух миші тощо.

Приклади:

```cpp
sf::Vector2u windowSize{800, 600};
sf::Vector2i mousePixelPosition{120, 40};
sf::Vector2f position{100.f, 50.f};
sf::Vector2f velocity{80.f, 0.f};

position += velocity * 0.5f;
```

Тут `velocity` можна читати як швидкість у пікселях за секунду. Якщо помножити її на `0.5f`, отримаємо зміщення за половину секунди.

## Створення вікна

Рядок:

```cpp
sf::RenderWindow window(sf::VideoMode({800, 600}), "SFML 3 works!");
```

створює об'єкт вікна. `sf::RenderWindow` - це вікно, у яке можна малювати 2D-об'єкти.

Перший параметр - відеорежим:

```cpp
sf::VideoMode({800, 600})
```

У SFML 3 розмір передається як `sf::Vector2u`: `{800, 600}` означає ширину `800` і висоту `600` пікселів.

Другий параметр:

```cpp
"SFML 3 works!"
```

це заголовок вікна.

Коли створюється `sf::RenderWindow`, SFML просить операційну систему створити справжнє вікно, готує контекст для малювання і внутрішні ресурси, потрібні для показу кадрів. Після цього програма може перевіряти, чи вікно відкрите:

```cpp
while (window.isOpen()) {
    // головний цикл програми
}
```

## Створення фігури

Рядок:

```cpp
sf::CircleShape shape(100.f);
```

створює коло радіусом `100.f` пікселів. Насправді це об'єкт, який SFML уміє намалювати у вікні.

Колір задається так:

```cpp
shape.setFillColor(sf::Color::Green);
```

З фігурою можна виконувати інші дії:

```cpp
shape.setPosition({40.f, 30.f});
shape.setRadius(50.f);
shape.setFillColor(sf::Color(255, 120, 40));
shape.move({10.f, 0.f});
```

`setPosition` задає позицію, `setRadius` змінює радіус, `setFillColor` змінює колір, а `move` зміщує фігуру відносно поточної позиції.

## Цикл подій

Віконна програма має регулярно забирати події з черги операційної системи. У SFML 3 це робиться так:

```cpp
while (const std::optional event = window.pollEvent()) {
    if (event->is<sf::Event::Closed>()) {
        window.close();
    }
}
```

`pollEvent()` повертає `std::optional<sf::Event>`. Якщо подія є, ми отримуємо її і обробляємо. Якщо подій більше немає, внутрішній цикл завершується, і програма переходить до оновлення стану та малювання.

Важливо: коли `pollEvent()` повертає подію, ця подія забирається з внутрішньої FIFO-черги подій вікна. Тобто наступний виклик `pollEvent()` уже перевірятиме наступну подію. Якщо ви зчитали подію і нічого з нею не зробили, вона не повернеться назад у чергу - для вашої програми вона фактично пропущена.

`event->is<sf::Event::Closed>()` тільки перевіряє тип події. Це зручно для подій без додаткових даних, наприклад для закриття вікна.

Якщо подія має додаткові дані, використовуйте `getIf<T>()`.

### Приклади подій

Натискання клавіші:

```cpp
if (const auto* keyPressed = event->getIf<sf::Event::KeyPressed>()) {
    if (keyPressed->scancode == sf::Keyboard::Scan::Escape) {
        window.close();
    }
}
```

Клік миші:

```cpp
if (const auto* mousePressed = event->getIf<sf::Event::MouseButtonPressed>()) {
    if (mousePressed->button == sf::Mouse::Button::Left) {
        sf::Vector2i clickPosition = mousePressed->position;
    }
}
```

Рух миші всередині вікна:

```cpp
if (const auto* mouseMoved = event->getIf<sf::Event::MouseMoved>()) {
    sf::Vector2i mousePosition = mouseMoved->position;
}
```

Зміна розміру вікна:

```cpp
if (const auto* resized = event->getIf<sf::Event::Resized>()) {
    sf::Vector2u newSize = resized->size;
}
```

Події - це разові повідомлення. Вони відповідають на питання: "Що щойно сталося?"

## Перевірка поточного стану

Для безперервного руху часто краще перевіряти не подію, а поточний стан клавіатури або миші. Стан відповідає на питання: "Що натиснуто прямо зараз?"

Приклад для клавіатури:

```cpp
if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left)) {
    shape.move({-1.f, 0.f});
}

if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right)) {
    shape.move({1.f, 0.f});
}
```

Приклад для миші:

```cpp
if (sf::Mouse::isButtonPressed(sf::Mouse::Button::Left)) {
    sf::Vector2i mousePosition = sf::Mouse::getPosition(window);
}
```

Різниця важлива:

- подія `KeyPressed` виникає в момент натискання;
- стан `isKeyPressed` можна перевіряти кожен кадр, поки клавіша утримується.

## Малювання кадру

Кожен кадр у SFML зазвичай складається з трьох кроків:

```cpp
window.clear();
window.draw(shape);
window.display();
```

`window.clear()` очищає область малювання. Якщо колір не вказано, використовується чорний:

```cpp
window.clear(sf::Color::Black);
```

`window.draw(shape)` додає фігуру в поточний кадр. У цей момент ми не описуємо "намалюй один піксель", а передаємо SFML об'єкт, який треба відобразити.

`window.display()` показує готовий кадр на екрані. SFML не малює об'єкти одразу прямо у видиме вікно: команди `draw` готують кадр у прихованому буфері, а `display()` завершує кадр і робить цей буфер видимим.

Важливий порядок:

1. спочатку очистити старий кадр;
2. потім намалювати всі об'єкти;
3. в кінці показати результат.

Якщо забути `clear`, на екрані можуть залишатися сліди попередніх кадрів. Якщо забути `display`, підготовлений кадр не буде показаний.

## Питання для самоконтролю

1. Чому `pollEvent()` зазвичай викликають у внутрішньому циклі `while`, а не один раз за кадр?
2. Чим подія `KeyPressed` відрізняється від перевірки стану `sf::Keyboard::isKeyPressed(...)`?
3. Що станеться, якщо намалювати об'єкт через `window.draw(...)`, але забути викликати `window.display()`?
4. Чому порядок `clear` -> `draw` -> `display` важливий для кожного кадру?

## Час і однакова швидкість руху

Якщо рухати об'єкт на однакову кількість пікселів за кадр, швидкість буде залежати від комп'ютера. На швидкому комп'ютері кадрів буде більше, отже об'єкт рухатиметься швидше. На повільному - повільніше.

Краще прив'язувати оновлення стану до реального часу. Для цього SFML має `sf::Clock` і `sf::Time`.

```cpp
sf::Clock clock;
sf::Vector2f position{0.f, 80.f};
sf::Vector2f velocity{120.f, 0.f}; // пікселів за секунду

while (window.isOpen()) {
    // Обробка подій тут така сама, як у попередніх прикладах.

    sf::Time elapsed = clock.restart();
    float dt = elapsed.asSeconds();

    position += velocity * dt;
    shape.setPosition(position);

    window.clear();
    window.draw(shape);
    window.display();
}
```

`clock.restart()` повертає час, який минув від попереднього виклику `restart()`, і одразу запускає відлік заново.

`elapsed.asSeconds()` перетворює `sf::Time` у секунди як `float`. Якщо між кадрами минуло приблизно 16 мілісекунд, то:

```cpp
sf::Time elapsed = sf::milliseconds(16);
float seconds = elapsed.asSeconds(); // приблизно 0.016
auto ms = elapsed.asMilliseconds();  // 16
```

Якщо швидкість дорівнює `120` пікселів за секунду, то за `0.016` секунди об'єкт зміститься приблизно на:

```text
120 * 0.016 = 1.92 пікселя
```

Так рух лишається близьким за швидкістю на різних комп'ютерах.

Рендеринг кадру теж можна обмежувати, наприклад вертикальною синхронізацією або лімітом FPS. Але для логіки руху важливіше не кількість кадрів сама по собі, а те, щоб зміна стану використовувала реальний час між кадрами.

## Обмеження FPS

SFML дозволяє обмежити частоту кадрів після створення вікна:

```cpp
sf::RenderWindow window(sf::VideoMode({800, 600}), "SFML 3 works!");
window.setFramerateLimit(60);
```

`setFramerateLimit(60)` просить SFML не рендерити швидше ніж приблизно `60` кадрів за секунду. Це не абсолютно точний таймер, бо залежить від точності системного сну, але для навчального проєкту цього зазвичай достатньо.

Інший варіант - вертикальна синхронізація:

```cpp
window.setVerticalSyncEnabled(true);
```

Вона синхронізує показ кадрів із частотою оновлення монітора. Зазвичай обирають один підхід: або `setFramerateLimit`, або `setVerticalSyncEnabled`, а не обидва одночасно.

## Підрахунок поточного FPS

FPS - це кількість кадрів за секунду. Його можна порахувати вручну: рахуємо кадри, накопичуємо час і раз на секунду оновлюємо значення.

```cpp
#include <SFML/Graphics.hpp>
#include <optional>
#include <string>

int main() {
    sf::RenderWindow window(sf::VideoMode({800, 600}), "SFML 3 works!");
    window.setFramerateLimit(60);

    sf::CircleShape shape(100.f);
    shape.setFillColor(sf::Color::Green);

    sf::Clock fpsClock;
    unsigned int frameCount = 0;

    while (window.isOpen()) {
        while (const std::optional event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>()) {
                window.close();
            }
        }

        window.clear();
        window.draw(shape);
        window.display();

        frameCount++;

        sf::Time elapsed = fpsClock.getElapsedTime();
        if (elapsed >= sf::seconds(1.f)) {
            float fps = static_cast<float>(frameCount) / elapsed.asSeconds();
            window.setTitle("FPS: " + std::to_string(static_cast<int>(fps)));

            frameCount = 0;
            fpsClock.restart();
        }
    }

    return 0;
}
```

Тут `frameCount` збільшується після кожного `display()`, тобто після завершення кадру. Коли минула приблизно одна секунда, ми ділимо кількість кадрів на реальний час у секундах і отримуємо поточний FPS.

