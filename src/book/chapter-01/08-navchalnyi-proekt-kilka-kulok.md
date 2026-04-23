# 8. Навчальний проєкт: кілька кульок

Цей розділ складніший. Тут з'являються `struct`, `std::vector`, цикли по об'єктах і прості зіткнення. Якщо ви впевнено зробили попередні кроки, переходьте далі.

## Крок 11. Багато кульок

Для кількох кульок зручно створити окремий тип даних:

До цього стан однієї кульки зберігався в окремих змінних: `position`, `velocity`, `radius`, `color`. Коли кульок стає кілька, такий підхід швидко плутає код: довелося б створювати `position1`, `position2`, `velocity1`, `velocity2` і повторювати однакову логіку.

`struct Ball` збирає всі властивості однієї кульки в один об'єкт. А `std::vector<Ball>` зберігає список таких об'єктів. Після цього можна написати один цикл і виконати ті самі дії для кожної кульки: оновити позицію, перевірити межі, намалювати.

```cpp
struct Ball {
    sf::Vector2f position;
    sf::Vector2f velocity;
    float radius;
    sf::Color color;
};
```

Додайте:

```cpp
#include <vector>
```

Замість окремих змінних для однієї кульки створіть список:

```cpp
std::vector<Ball> balls;
balls.push_back({{100.f, 100.f}, {160.f, 120.f}, 30.f, sf::Color::Green});
balls.push_back({{300.f, 220.f}, {-140.f, 90.f}, 25.f, sf::Color::Blue});
```

Щоб намалювати `Ball`, перетворюємо його на `sf::CircleShape`:

```cpp
sf::CircleShape makeShape(const Ball& b) {
    sf::CircleShape shape(b.radius);
    shape.setPosition(b.position);
    shape.setFillColor(b.color);
    return shape;
}
```

Малювання всіх кульок:

```cpp
for (const Ball& ball : balls) {
    window.draw(makeShape(ball));
}
```

Проміжний результат: у вікні є дві кульки.

Спробуйте ще: додайте третю кульку з іншою швидкістю.

## Крок 12. Однакова логіка для всіх кульок

Коли кульок багато, код руху і відбивання треба виконувати в циклі.

```cpp
for (Ball& ball : balls) {
    ball.position += ball.velocity * dt;

    if (ball.position.x <= playArea.position.x) {
        ball.position.x = playArea.position.x;
        ball.velocity.x = -ball.velocity.x;
    }

    if (ball.position.x + 2.f * ball.radius >= playArea.position.x + playArea.size.x) {
        ball.position.x = playArea.position.x + playArea.size.x - 2.f * ball.radius;
        ball.velocity.x = -ball.velocity.x;
    }

    if (ball.position.y <= playArea.position.y) {
        ball.position.y = playArea.position.y;
        ball.velocity.y = -ball.velocity.y;
    }

    if (ball.position.y + 2.f * ball.radius >= playArea.position.y + playArea.size.y) {
        ball.position.y = playArea.position.y + playArea.size.y - 2.f * ball.radius;
        ball.velocity.y = -ball.velocity.y;
    }
}
```

Проміжний результат: кілька кульок рухаються і відбиваються від меж поля.

Спробуйте ще: зробіть так, щоб кожна кулька змінювала колір після відбивання.

## Крок 13. Додавання кульки кліком миші

Додайте просту функцію створення кульки:

```cpp
Ball makeBallAt(sf::Vector2f point) {
    Ball b;
    b.radius = 20.f;
    b.position = point - sf::Vector2f(b.radius, b.radius);
    b.velocity = {180.f, -120.f};
    b.color = randomColor();
    return b;
}
```

У циклі подій додайте обробку кліку:

```cpp
if (const auto* mouse = event->getIf<sf::Event::MouseButtonPressed>()) {
    if (mouse->button == sf::Mouse::Button::Left) {
        sf::Vector2f point{
            static_cast<float>(mouse->position.x),
            static_cast<float>(mouse->position.y)
        };

        if (playArea.contains(point)) {
            balls.push_back(makeBallAt(point));
        }
    }
}
```

Проміжний результат: клік лівою кнопкою миші всередині поля додає нову кульку.

Спробуйте ще: задавайте новій кульці випадкову швидкість або інший радіус.

## Крок 14. Прості зіткнення кульок

Для початку зробимо просту перевірку: якщо відстань між центрами кульок менша за суму радіусів, вони торкнулися.

```cpp
sf::Vector2f centerOf(const Ball& b) {
    return b.position + sf::Vector2f(b.radius, b.radius);
}

bool areTouching(const Ball& a, const Ball& b) {
    sf::Vector2f d = centerOf(b) - centerOf(a);
    float minDistance = a.radius + b.radius;
    return d.x * d.x + d.y * d.y <= minDistance * minDistance;
}
```

Додайте:

```cpp
#include <utility>
```

Після руху і відбивання від стін перевірте всі пари кульок:

```cpp
for (std::size_t i = 0; i < balls.size(); ++i) {
    for (std::size_t j = i + 1; j < balls.size(); ++j) {
        if (areTouching(balls[i], balls[j])) {
            std::swap(balls[i].velocity, balls[j].velocity);
            balls[i].color = randomColor();
            balls[j].color = randomColor();
        }
    }
}
```

Проміжний результат: кульки приблизно реагують на зіткнення одна з одною.

Це спрощена модель. Вона може давати повторні удари, якщо кульки сильно перекрилися. Для сильніших: спробуйте розсувати кульки після зіткнення або використати нормаль між центрами, як у наближеній фізиці більярдних куль.

## Контрольна точка

Після цього розділу у вас є вже не одна кулька, а набір об'єктів. Вони рухаються за одними правилами, можуть додаватися мишкою і приблизно реагують на зіткнення.
