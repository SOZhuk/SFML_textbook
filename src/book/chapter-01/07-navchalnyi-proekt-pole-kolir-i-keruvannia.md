# 7. Навчальний проєкт: поле, колір і керування

Цей розділ продовжує попередній. Ми не переписуємо програму з нуля, а поступово додаємо нові можливості до вже робочої кульки. Після кожного кроку запускайте програму і перевіряйте саме ту зміну, яку щойно додали.

## Крок 6. Колір після удару

Мета цього кроку - зробити удар об межу помітним для користувача. Додайте зверху файлу:

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

Цей рядок запускає генератор випадкових чисел з різним початковим значенням під час кожного запуску програми.

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

Проміжний результат: кулька змінює колір один раз після удару об межу. Якщо кулька торкнулася кута і відбилася одразу по двох осях, прапорець усе одно обробляє це як один удар у цьому кадрі.

Спробуйте ще: зробіть не випадковий колір, а перемикання між двома кольорами.

## Крок 7. Поле менше за вікно

Мета цього кроку - відокремити ігрове поле від усього вікна. Кулька буде відбиватися не від країв вікна, а від прямокутної області всередині нього.

Перед циклом створіть поле:

```cpp
sf::FloatRect playArea({50.f, 50.f}, {700.f, 500.f});

sf::RectangleShape border(playArea.size);
border.setPosition(playArea.position);
border.setFillColor(sf::Color::Transparent);
border.setOutlineThickness(3.f);
border.setOutlineColor(sf::Color(220, 220, 220));
```

У перевірках меж замініть `0`, `windowWidth` і `windowHeight` на межі `playArea`. Пам'ятайте: `position` все ще означає лівий верхній кут кульки, а не її центр.

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

:::details Перевірка меж по осі y
Якщо не виходить перенести логіку з `x` на `y`, звіртеся з цим фрагментом.

```cpp
if (position.y <= playArea.position.y) {
    position.y = playArea.position.y;
    velocity.y = -velocity.y;
    bounced = true;
}

if (position.y + 2.f * radius >= playArea.position.y + playArea.size.y) {
    position.y = playArea.position.y + playArea.size.y - 2.f * radius;
    velocity.y = -velocity.y;
    bounced = true;
}
```
:::

У малюванні додайте межу поля перед кулькою:

```cpp
window.draw(border);
window.draw(ball);
```

Проміжний результат: у вікні видно прямокутне поле, а кулька відбивається від його меж.

Спробуйте ще: змініть розмір поля або колір рамки.

## Крок 8. Керування з клавіатури

Мета цього кроку - дати гравцю вплив на швидкість кульки. Для безперервного керування перевіряємо стан клавіш кожен кадр. Додайте після обробки подій:

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

Тут клавіатура змінює швидкість, а не позицію напряму. Тому кулька поводиться більш плавно: вона прискорюється і продовжує рухатися після відпускання клавіші.

Щоб кулька не розганялася безмежно, можна додати просте обмеження швидкості:

```cpp
float maxSpeed = 550.f;

if (velocity.x > maxSpeed) velocity.x = maxSpeed;
if (velocity.x < -maxSpeed) velocity.x = -maxSpeed;
if (velocity.y > maxSpeed) velocity.y = maxSpeed;
if (velocity.y < -maxSpeed) velocity.y = -maxSpeed;
```

Як додаткове завдання можна додати просте тертя, щоб кулька поступово сповільнювалася. Після обробки клавіатури і обмеження швидкості помножте швидкість на число трохи менше за `1`:

```cpp
velocity *= 0.99f;
```

Значення `0.99f` дає слабке сповільнення. Якщо взяти `0.95f`, кулька гальмуватиме помітно швидше. Це спрощена модель тертя: вона залежить від кількості кадрів, але для першого експерименту її достатньо.

Проміжний результат: стрілки впливають на напрям і швидкість руху.

Спробуйте ще: зробіть `accel` більшим або меншим.

## Крок 9. Лічильник відбивань

Мета цього кроку - зберігати простий ігровий показник. Додайте змінну перед головним циклом:

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

Найпростіший спосіб показати число без шрифтів - записати його в заголовок вікна. Додайте після оновлення лічильника:

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
        ball.setPosition(position);
        window.setTitle("Bounces: 0");
    }
}
```

Це саме події, а не стан клавіатури: дія виконується один раз у момент натискання.

## Крок 10. Текстовий лічильник у вікні

Мета цього кроку - перенести показник із заголовка вікна у саму сцену. Щоб показати текст прямо у вікні, потрібен файл шрифту. Завантажте [NotoSans-Regular.ttf](static/fonts/NotoSans-Regular.ttf) або весь [набір навчальних файлів](static/assets.zip), створіть у своєму C++ проєкті папку `assets` і покладіть шрифт туди.

:::warning Увага
Папка `assets` має бути там, звідки запускається `.exe`. Якщо шрифт не завантажується, спочатку перевірте робочу директорію запуску програми.
:::

```cpp
sf::Font font("assets/NotoSans-Regular.ttf");
sf::Text label(font, "Bounces: 0", 24);
label.setPosition({20.f, 20.f});
```

Після зміни лічильника оновлюйте текст:

```cpp
label.setString("Bounces: " + std::to_string(wallBounces));
```

У малюванні додайте текст після поля і кульки, щоб напис був поверх сцени:

```cpp
window.draw(border);
window.draw(ball);
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
