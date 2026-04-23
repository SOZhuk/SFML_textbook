# 7. Навчальний проект: поле, колір і керування

Цей розділ продовжує попередній. Ми не переписуємо програму з нуля, а поступово додаємо нові можливості до вже робочої кульки.

## Крок 6. Колір після удару

Додайте зверху файлу:

```cpp
#include <cstdlib>
#include <ctime>
```

Перед `main()` додайте функцію:

```cpp
sf::Color randomColor() {
    return sf::Color(
        50 + std::rand() % 206,
        50 + std::rand() % 206,
        50 + std::rand() % 206
    );
}
```

На початку `main()` додайте:

```cpp
std::srand(static_cast<unsigned>(std::time(nullptr)));
```

Перед перевірками меж створіть прапорець:

```cpp
bool bounced = false;
```

У кожному місці, де швидкість змінюється на протилежну, додайте:

```cpp
bounced = true;
```

Після всіх перевірок меж додайте:

```cpp
if (bounced) {
    color = randomColor();
    ball.setFillColor(color);
}
```

Проміжний результат: кулька змінює колір після удару об межу.

Спробуйте ще: зробіть не випадковий колір, а перемикання між двома кольорами.

## Крок 7. Поле менше за вікно

Тепер зробимо внутрішнє поле. Кулька буде відбиватися не від країв вікна, а від прямокутної області всередині нього.

Перед циклом створіть поле:

```cpp
sf::FloatRect playArea({50.f, 50.f}, {700.f, 500.f});

sf::RectangleShape border(playArea.size);
border.setPosition(playArea.position);
border.setFillColor(sf::Color::Transparent);
border.setOutlineThickness(3.f);
border.setOutlineColor(sf::Color(220, 220, 220));
```

У перевірках меж замініть `0`, `windowWidth` і `windowHeight` на межі `playArea`:

```cpp
if (position.x <= playArea.position.x) {
    position.x = playArea.position.x;
    velocity.x = -velocity.x;
    bounced = true;
}

if (position.x + 2.f * radius >= playArea.position.x + playArea.size.x) {
    position.x = playArea.position.x + playArea.size.x - 2.f * radius;
    velocity.x = -velocity.x;
    bounced = true;
}
```

Так само зробіть для `y`.

У малюванні додайте межу поля перед кулькою:

```cpp
window.draw(border);
window.draw(ball);
```

Проміжний результат: у вікні видно прямокутне поле, а кулька відбивається від його меж.

Спробуйте ще: змініть розмір поля або колір рамки.

## Крок 8. Керування з клавіатури

Для безперервного керування перевіряємо стан клавіш кожен кадр. Додайте після обробки подій:

```cpp
float accel = 300.f;

if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left)) {
    velocity.x -= accel * dt;
}
if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right)) {
    velocity.x += accel * dt;
}
if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Up)) {
    velocity.y -= accel * dt;
}
if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Down)) {
    velocity.y += accel * dt;
}
```

Тут клавіатура змінює швидкість, а не позицію напряму. Тому кулька поводиться більш плавно: вона прискорюється і продовжує рухатися.

Щоб кулька не розганялася безмежно, можна додати просте обмеження швидкості:

```cpp
float maxSpeed = 550.f;

if (velocity.x > maxSpeed) velocity.x = maxSpeed;
if (velocity.x < -maxSpeed) velocity.x = -maxSpeed;
if (velocity.y > maxSpeed) velocity.y = maxSpeed;
if (velocity.y < -maxSpeed) velocity.y = -maxSpeed;
```

Проміжний результат: стрілки впливають на напрям і швидкість руху.

Спробуйте ще: зробіть `accel` більшим або меншим.

## Крок 9. Лічильник відбивань

Додайте змінну перед головним циклом:

```cpp
int wallBounces = 0;
```

Коли було відбивання, збільшуйте лічильник:

```cpp
if (bounced) {
    color = randomColor();
    ball.setFillColor(color);
    wallBounces++;
}
```

Найпростіший спосіб показати число без шрифтів - записати його в заголовок вікна. Додайте:

```cpp
window.setTitle("Bounces: " + std::to_string(wallBounces));
```

Для цього зверху файлу потрібен ще один include:

```cpp
#include <string>
```

Проміжний результат: у заголовку вікна видно кількість ударів об межі.

Спробуйте ще: показуйте в заголовку не тільки удари, а й координати кульки.

### Додаткові клавіші

У циклі подій можна додати прості разові команди. Наприклад, `Space` змінює колір, а `R` повертає кульку в початковий стан.

```cpp
if (const auto* key = event->getIf<sf::Event::KeyPressed>()) {
    if (key->code == sf::Keyboard::Key::Space) {
        color = randomColor();
        ball.setFillColor(color);
    }

    if (key->code == sf::Keyboard::Key::R) {
        position = {100.f, 100.f};
        velocity = {160.f, 120.f};
        wallBounces = 0;
    }
}
```

Це саме події, а не стан клавіатури: дія виконується один раз у момент натискання.

## Крок 10. Текстовий лічильник у вікні

Щоб показати текст прямо у вікні, потрібен файл шрифту. Завантажте [NotoSans-Regular.ttf](static/fonts/NotoSans-Regular.ttf) або весь [набір навчальних файлів](static/assets.zip), створіть у своєму C++ проєкті папку `assets` і покладіть шрифт туди.

```cpp
sf::Font font("assets/NotoSans-Regular.ttf");
sf::Text label(font, "Bounces: 0", 24);
label.setPosition({20.f, 20.f});
```

Після зміни лічильника оновлюйте текст:

```cpp
label.setString("Bounces: " + std::to_string(wallBounces));
```

У малюванні додайте:

```cpp
window.draw(label);
```

Проміжний результат: лічильник показаний у самому вікні.

Спробуйте ще: змініть розмір тексту, позицію або колір.

## Контрольна точка

Після цього розділу програма вже схожа на маленьку гру:

- кулька рухається в полі;
- відбивається від меж;
- змінює колір;
- керується клавіатурою;
- рахує відбивання.

Якщо далі стає складно, можна зупинитися тут: це вже хороший базовий результат.
