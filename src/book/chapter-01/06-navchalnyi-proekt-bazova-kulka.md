# 6. Навчальний проєкт: базова кулька

Починаємо з маленьких кроків. Після кожного кроку проєкт має компілюватися і запускатися. Якщо щось пішло не так, легше повернутися на один крок назад, ніж шукати помилку у великому файлі.

Працюємо в одному файлі `src/main.cpp`. Для першого графічного проєкту це нормально: головне зараз зрозуміти цикл програми, стан об'єкта і малювання кадру.

Далі проєкт буде рости поступово: від однієї кульки до поля, керування, кількох об'єктів і додаткових графічних ефектів. Не копіюйте великі готові приклади одразу: краще додавати одну зміну і одразу перевіряти результат.

## Крок 0. Вікно і порожній цикл

Почніть із мінімальної програми. Вона тільки відкриває вікно і дозволяє його закрити.

```cpp
#include <SFML/Graphics.hpp>
#include <optional>

int main() {
    sf::RenderWindow window(sf::VideoMode({800, 600}), "SFML lesson");
    window.setFramerateLimit(60);

    while (window.isOpen()) {
        while (const std::optional event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>()) {
                window.close();
            }
        }

        window.clear(sf::Color(30, 30, 30));
        window.display();
    }

    return 0;
}
```

Проміжний результат: відкривається темне вікно 800x600, кнопка закриття працює.

Спробуйте ще: змініть заголовок вікна або колір у `window.clear(...)`.

## Крок 1. Одна кулька

Додайте кульку перед головним циклом:

```cpp
sf::CircleShape ball(30.f);
ball.setPosition({100.f, 100.f});
ball.setFillColor(sf::Color::Green);
```

У частині малювання додайте:

```cpp
window.draw(ball);
```

Повний порядок малювання має бути таким:

```cpp
window.clear(sf::Color(30, 30, 30));
window.draw(ball);
window.display();
```

Проміжний результат: у вікні видно зелену кульку.

Спробуйте ще: змініть радіус, позицію або колір кульки.

## Крок 2. Стан кульки

Поки що позиція і колір заховані всередині `ball`. Для руху зручніше явно зберігати стан у змінних.

Перед створенням кульки додайте:

```cpp
float radius = 30.f;
sf::Vector2f position{100.f, 100.f};
sf::Vector2f velocity{160.f, 120.f};
sf::Color color = sf::Color::Green;
```

Створення кульки тепер можна записати так:

```cpp
sf::CircleShape ball(radius);
ball.setPosition(position);
ball.setFillColor(color);
```

Проміжний результат поки не зміниться. Але код готовий до руху: у нас уже є позиція, швидкість, радіус і колір.

Спробуйте ще: змініть `velocity` і передбачте, у який бік мала б рухатися кулька.

## Крок 3. Час кадру

Щоб рух не залежав від швидкості комп'ютера, додайте годинник перед головним циклом:

```cpp
sf::Clock clock;
```

На початку кожного кадру, вже всередині `while (window.isOpen())`, отримайте час кадру:

```cpp
float dt = clock.restart().asSeconds();
```

`dt` - це кількість секунд, яка минула від попереднього кадру. На 60 FPS це приблизно `0.016`.

Проміжний результат візуально ще не зміниться. Ми тільки підготували правильну основу для руху.

## Крок 4. Рух

У частині `Оновлення стану` додайте:

```cpp
position += velocity * dt;
ball.setPosition(position);
```

Тепер логіка кадру виглядає так:

```cpp
float dt = clock.restart().asSeconds();

// 1. Обробка подій.

// 2. Оновлення стану.
position += velocity * dt;
ball.setPosition(position);

// 3. Малювання кадру.
```

Проміжний результат: кулька рухається, але з часом вилітає за межі вікна.

Спробуйте ще: зробіть рух повільнішим або швидшим, змінивши `velocity`.

## Крок 5. Відбивання від меж вікна

Додайте сталі розміри вікна поруч зі змінними стану:

```cpp
const float windowWidth = 800.f;
const float windowHeight = 600.f;
```

Після оновлення позиції додайте перевірку меж:

```cpp
if (position.x <= 0.f) {
    position.x = 0.f;
    velocity.x = -velocity.x;
}

if (position.x + 2.f * radius >= windowWidth) {
    position.x = windowWidth - 2.f * radius;
    velocity.x = -velocity.x;
}

if (position.y <= 0.f) {
    position.y = 0.f;
    velocity.y = -velocity.y;
}

if (position.y + 2.f * radius >= windowHeight) {
    position.y = windowHeight - 2.f * radius;
    velocity.y = -velocity.y;
}

ball.setPosition(position);
```

У цьому варіанті `position` означає лівий верхній кут прямокутної області, у яку вписане коло. Тому правий край кульки має координату `position.x + 2.f * radius`, а нижній край - `position.y + 2.f * radius`.

SFML дозволяє змінити точку прив'язки об'єкта через `setOrigin(...)`. Наприклад, якщо поставити origin у центр кола, то `position` можна буде читати як координату центра:

```cpp
ball.setOrigin({radius, radius});
```

Тоді перевірки меж записувалися б інакше: лівий край був би `position.x - radius`, а правий - `position.x + radius`. У цьому проєкті залишаємо стандартну прив'язку до лівого верхнього кута, щоб усі наступні кроки використовували одну й ту саму модель координат.

Проміжний результат: кулька рухається і відбивається від країв вікна.

Спробуйте ще: змініть радіус кульки і перевірте, чи вона все одно правильно відбивається.

## Контрольна точка

Після цієї сторінки у вас має бути програма, де:

- відкривається вікно;
- у вікні є кулька;
- кулька рухається;
- кулька відбивається від країв.

Це перший робочий результат. Далі будемо додавати колір, поле, керування і лічильники.

:::details Повний код після кроку 5
Якщо ваша програма не компілюється або кулька поводиться не так, звірте свій `src/main.cpp` з цим варіантом.

```cpp
#include <SFML/Graphics.hpp>
#include <optional>

int main() {
    sf::RenderWindow window(sf::VideoMode({800, 600}), "SFML lesson");
    window.setFramerateLimit(60);

    const float windowWidth = 800.f;
    const float windowHeight = 600.f;

    float radius = 30.f;
    sf::Vector2f position{100.f, 100.f};
    sf::Vector2f velocity{160.f, 120.f};
    sf::Color color = sf::Color::Green;

    sf::CircleShape ball(radius);
    ball.setPosition(position);
    ball.setFillColor(color);

    sf::Clock clock;

    while (window.isOpen()) {
        float dt = clock.restart().asSeconds();

        while (const std::optional event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>()) {
                window.close();
            }
        }

        position += velocity * dt;

        if (position.x <= 0.f) {
            position.x = 0.f;
            velocity.x = -velocity.x;
        }

        if (position.x + 2.f * radius >= windowWidth) {
            position.x = windowWidth - 2.f * radius;
            velocity.x = -velocity.x;
        }

        if (position.y <= 0.f) {
            position.y = 0.f;
            velocity.y = -velocity.y;
        }

        if (position.y + 2.f * radius >= windowHeight) {
            position.y = windowHeight - 2.f * radius;
            velocity.y = -velocity.y;
        }

        ball.setPosition(position);

        window.clear(sf::Color(30, 30, 30));
        window.draw(ball);
        window.display();
    }

    return 0;
}
```
:::
