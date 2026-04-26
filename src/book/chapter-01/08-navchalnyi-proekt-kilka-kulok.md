# 8. Навчальний проєкт: кілька кульок

Цей розділ складніший. Тут з'являються `struct`, `std::vector`, цикли по об'єктах і прості зіткнення. Якщо ви впевнено зробили попередні кроки, переходьте далі. Якщо ні, поверніться до однієї кульки: далі ми будемо переносити ту саму логіку на багато об'єктів.

## Крок 11. Багато кульок

Мета цього кроку - замінити набір окремих змінних на один тип даних для кульки. Для кількох кульок зручно створити окремий тип:

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

Замість окремих змінних для однієї кульки створіть список. Почніть із двох об'єктів, щоб одразу побачити різницю:

```cpp
std::vector<Ball> balls;
balls.push_back({{100.f, 100.f}, {160.f, 120.f}, 30.f, sf::Color::Green});
balls.push_back({{300.f, 220.f}, {-140.f, 90.f}, 25.f, sf::Color::Blue});
```

Щоб намалювати `Ball`, перетворюємо його на `sf::CircleShape`. Функцію `makeShape` зручно розмістити перед `main()`:

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

Проміжний результат: у вікні є дві кульки, і кожна бере позицію, радіус та колір зі свого `Ball`.

Спробуйте ще: додайте третю кульку з іншою швидкістю.

## Крок 12. Однакова логіка для всіх кульок

Мета цього кроку - виконати стару логіку руху для кожної кульки окремо. Коли кульок багато, код руху і відбивання треба виконувати в циклі.

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

Проміжний результат: кілька кульок рухаються і відбиваються від меж поля. У цьому фрагменті кожна кулька має власні `position`, `velocity` і `radius`, тому однакові перевірки працюють для різних розмірів.

Спробуйте ще: зробіть так, щоб кожна кулька змінювала колір після відбивання.

## Крок 13. Додавання кульки кліком миші

Мета цього кроку - створювати нові об'єкти під час роботи програми. Додайте просту функцію створення кульки:

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

У циклі подій додайте обробку кліку. Координати миші приходять у пікселях вікна, тому перетворюємо їх на `sf::Vector2f`:

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

Проміжний результат: клік лівою кнопкою миші всередині поля додає нову кульку, а наступні кадри вже обробляють її разом з іншими елементами `balls`.

Запитання для перевірки: чи вся нова кулька точно всередині поля, якщо натиснути дуже близько до межі? Зараз ми перевіряємо тільки точку кліку, тобто центр кульки. Для сильніших: створіть `Ball newBall = makeBallAt(point)` і перевірте не лише `point`, а й краї нової кульки: `newBall.position.x`, `newBall.position.x + 2.f * newBall.radius`, і відповідні межі по `y`. Додавайте `newBall` у `balls` тільки після цієї перевірки.

Спробуйте ще: задавайте новій кульці випадкову швидкість або інший радіус.

## Крок 14. Прості зіткнення кульок

Мета цього кроку - навчити кульки приблизно реагувати одна на одну. Для початку зробимо просту перевірку: якщо відстань між центрами кульок менша за суму радіусів, вони торкнулися.

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
#include <cstddef>
#include <utility>
```

Після руху і відбивання від стін перевірте всі пари кульок. Внутрішній цикл починається з `i + 1`, щоб не порівнювати кульку саму з собою і не перевіряти одну пару двічі:

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

Проміжний результат: кульки приблизно реагують на зіткнення одна з одною: при дотику вони обмінюються швидкостями і змінюють колір.

Це спрощена модель. Вона може давати повторні удари, якщо кульки сильно перекрилися. Для сильніших: спробуйте розсувати кульки після зіткнення або використати нормаль між центрами, як у наближеній фізиці більярдних куль. Для базового проєкту достатньо, що зіткнення помітне і не потребує складної математики.

## Контрольна точка

Після цього розділу у вас є вже не одна кулька, а набір об'єктів. Вони рухаються за одними правилами, можуть додаватися мишкою і приблизно реагують на зіткнення.

Перед переходом далі перевірте три речі: програма працює з двома початковими кульками, клік додає нову кульку всередині поля, а зіткнення не зупиняють головний цикл.
