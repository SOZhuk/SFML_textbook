# 9. Додаткові ідеї: індикатори, спрайти, анімація і фон

Цей розділ необов'язковий. Його задача - показати, у які боки можна розвивати проєкт після базової кульки. Тут можна брати одну ідею, пробувати її окремо, а не додавати все одразу.

Для цієї сторінки є публічний набір навчальних файлів: [assets.zip](static/assets.zip). Його можна розпакувати у свій C++ проєкт у папку `assets`.

Окремі файли також доступні напряму:

- [NotoSans-Regular.ttf](static/fonts/NotoSans-Regular.ttf) - шрифт для тексту;
- [player.png](static/sprites/player.png) - спрайт гравця із зеленим фоном;
- [enemy.png](static/sprites/enemy.png) - спрайт ворога із зеленим фоном;
- [bonus.png](static/sprites/bonus.png) - спрайт бонуса із зеленим фоном;
- [octopus01.png](static/animation/octopus01.png), [octopus02.png](static/animation/octopus02.png), [octopus03.png](static/animation/octopus03.png), [octopus04.png](static/animation/octopus04.png), [octopus05.png](static/animation/octopus05.png) - кадри анімації персонажа;
- [starsky.jpg](static/backgrounds/starsky.jpg) - фон зоряного неба;
- [credits.md](static/credits.md) - ліцензії і походження файлів.

Спрайти на цій сторінці не мають прозорого фону. Замість цього фон залитий спеціальним зеленим кольором. Це називають `key color`: під час завантаження ми кажемо програмі, що цей колір треба зробити прозорим.

## Ідея 1. Панель і візуальні індикатори

Якщо не хочете працювати зі шрифтами, інформацію можна показувати фігурами. Для панелі праворуч спочатку збільшіть вікно:

```cpp
sf::RenderWindow window(sf::VideoMode({1100, 600}), "SFML lesson");
```

Поле можна залишити всередині лівої частини вікна:

```cpp
sf::FloatRect playArea({50.f, 50.f}, {760.f, 500.f});
```

Потім зробіть бічну панель:

```cpp
sf::RectangleShape sidePanel({220.f, 500.f});
sidePanel.setPosition({850.f, 50.f});
sidePanel.setFillColor(sf::Color(50, 56, 70));
```

Намалюйте її до малювання кульок:

```cpp
window.draw(sidePanel);
```

Кількість кульок можна показати маленькими кружечками:

```cpp
for (int i = 0; i < static_cast<int>(balls.size()) && i < 12; ++i) {
    sf::CircleShape dot(7.f);
    dot.setPosition({875.f + (i % 4) * 22.f, 90.f + (i / 4) * 22.f});
    dot.setFillColor(sf::Color(120, 190, 255));
    window.draw(dot);
}
```

Кількість відбивань можна показати смужками:

```cpp
int bars = wallBounces;
if (bars > 20) bars = 20;

for (int i = 0; i < bars; ++i) {
    sf::RectangleShape bar({10.f, 20.f});
    bar.setPosition({875.f + i * 10.f, 180.f});
    bar.setFillColor(sf::Color(255, 210, 90));
    window.draw(bar);
}
```

Проміжний результат: праворуч з'являється панель з простими індикаторами.

## Ідея 2. Завантаження спрайта з key color

Завантажте [player.png](static/sprites/player.png) або розпакуйте [assets.zip](static/assets.zip). У своєму C++ проєкті покладіть файл у папку `assets`.

Для звичайного прозорого PNG вистачає `sf::Texture`. Але наші навчальні спрайти зроблені інакше: фон у них зелений. Тому спочатку завантажимо картинку як `sf::Image`, зробимо зелений фон прозорим, а потім створимо текстуру.

```cpp
sf::Image playerImage("assets/player.png");

sf::Color keyColor = playerImage.getPixel({0, 0});
playerImage.createMaskFromColor(keyColor);

sf::Texture playerTexture(playerImage);
sf::Sprite player(playerTexture);
```

Тут `getPixel({0, 0})` бере колір лівого верхнього пікселя. У наших файлах цей піксель належить фону, тому його можна використати як key color. `createMaskFromColor` знаходить пікселі такого самого кольору і робить їх прозорими.

Позицію спрайта можна оновлювати тією самою змінною `position`:

```cpp
player.setPosition(position);
window.draw(player);
```

Важливо: `sf::Sprite` не копіює текстуру всередину себе. Текстура `playerTexture` має існувати весь час, поки використовується спрайт.

Спробуйте ще: замініть кульку на спрайт тільки в малюванні, а логіку руху залиште стару. Це найпростіший спосіб перейти від геометричної фігури до картинки.

## Ідея 3. Спрайт-стрілочка повертається за напрямком руху

Якщо спрайт гравця схожий на стрілку, його можна повертати туди, куди зараз рухається об'єкт. Для цього нам потрібна швидкість `velocity`.

Додайте зверху файлу:

```cpp
#include <cmath>
```

Перед `main()` додайте:

```cpp
constexpr float PI = 3.1415926535f;
```

Після створення спрайта перенесіть його початок координат у центр картинки:

```cpp
sf::Vector2u playerSize = playerTexture.getSize();

player.setOrigin({
    static_cast<float>(playerSize.x) / 2.f,
    static_cast<float>(playerSize.y) / 2.f
});
```

За замовчуванням `position` у наших прикладах часто означає лівий верхній кут кульки. Спрайт зі встановленим центром зручніше ставити саме в центр об'єкта:

```cpp
sf::Vector2f playerCenter = position + sf::Vector2f(radius, radius);
player.setPosition(playerCenter);
```

Тепер можна порахувати кут за швидкістю:

```cpp
if (velocity.x != 0.f || velocity.y != 0.f) {
    float angle = std::atan2(velocity.y, velocity.x) * 180.f / PI;
    player.setRotation(sf::degrees(angle + 90.f));
}
```

`std::atan2(y, x)` повертає кут напряму в радіанах. Множення на `180 / PI` переводить його в градуси, бо `sf::degrees(...)` очікує градуси. Додавання `90.f` потрібне тому, що наш спрайт-стрілочка спочатку дивиться вгору. Якщо ваш спрайт спочатку дивиться вправо, додавання `90.f` треба прибрати.

У частині малювання тепер достатньо:

```cpp
window.draw(player);
```

Якщо ви вже перейшли на `std::vector<Ball>`, використовуйте швидкість і позицію першої кульки:

```cpp
sf::Vector2f v = balls[0].velocity;
sf::Vector2f center = balls[0].position + sf::Vector2f(balls[0].radius, balls[0].radius);

if (v.x != 0.f || v.y != 0.f) {
    float angle = std::atan2(v.y, v.x) * 180.f / PI;
    player.setRotation(sf::degrees(angle + 90.f));
}

player.setPosition(center);
window.draw(player);
```

Проміжний результат: замість кульки видно спрайт, який не просто рухається, а ще й повертається у напрямку руху.

## Ідея 4. Кадрова анімація персонажа

Кадрова анімація - це швидка заміна однієї картинки іншою. У нас є п'ять файлів: `octopus01.png`, `octopus02.png`, `octopus03.png`, `octopus04.png`, `octopus05.png`. Покладіть їх у папку `assets`.

Додайте зверху файлу:

```cpp
#include <array>
#include <cstddef>
#include <string>
```

Перед головним циклом створіть список файлів і масив текстур:

```cpp
std::array<std::string, 5> frameFiles{
    "assets/octopus01.png",
    "assets/octopus02.png",
    "assets/octopus03.png",
    "assets/octopus04.png",
    "assets/octopus05.png"
};

std::array<sf::Texture, 5> animationFrames;
```

Тепер завантажте всі кадри. Для кожного кадру беремо key color з лівого верхнього пікселя. У підготовлених файлах це зелений фон; для власних файлів такий прийом також корисний, якщо фон не точно `sf::Color::Green`.

```cpp
for (std::size_t i = 0; i < frameFiles.size(); ++i) {
    sf::Image frameImage(frameFiles[i]);
    sf::Color keyColor = frameImage.getPixel({0, 0});
    frameImage.createMaskFromColor(keyColor);

    if (!animationFrames[i].loadFromImage(frameImage)) {
        return 1;
    }
}
```

Створіть спрайт з першого кадру:

```cpp
sf::Sprite animatedPlayer(animationFrames[0]);
animatedPlayer.setOrigin({
    static_cast<float>(animationFrames[0].getSize().x) / 2.f,
    static_cast<float>(animationFrames[0].getSize().y) / 2.f
});
animatedPlayer.setScale({0.06f, 0.06f});
```

Кадри великі, тому `setScale({0.06f, 0.06f})` зменшує персонажа. Значення можна змінити: `0.04f` зробить його меншим, `0.08f` - більшим.

Для зміни кадрів потрібні лічильник і годинник:

```cpp
std::size_t currentFrame = 0;
sf::Clock animationClock;
float frameDuration = 0.12f;
```

У частині оновлення стану додайте:

```cpp
if (animationClock.getElapsedTime().asSeconds() >= frameDuration) {
    currentFrame = (currentFrame + 1) % animationFrames.size();
    animatedPlayer.setTexture(animationFrames[currentFrame], true);
    animationClock.restart();
}
```

Оператор `%` повертає залишок від ділення. Завдяки цьому після останнього кадру індекс знову стає `0`, і анімація повторюється по колу.

У частині малювання:

```cpp
sf::Vector2f playerCenter = position + sf::Vector2f(radius, radius);
animatedPlayer.setPosition(playerCenter);
window.draw(animatedPlayer);
```

Якщо треба поєднати анімацію і поворот, після `setPosition(...)` можна використати той самий код з `std::atan2(...)`:

```cpp
if (velocity.x != 0.f || velocity.y != 0.f) {
    float angle = std::atan2(velocity.y, velocity.x) * 180.f / PI;
    animatedPlayer.setRotation(sf::degrees(angle + 90.f));
}
```

Проміжний результат: персонаж рухається як раніше, але його зображення змінюється кілька разів на секунду.

## Ідея 5. Статичний фон

Завантажте [starsky.jpg](static/backgrounds/starsky.jpg) або розпакуйте [assets.zip](static/assets.zip). У своєму C++ проєкті покладіть файл у папку `assets`. Завантажте текстуру:

```cpp
sf::Texture backgroundTexture("assets/starsky.jpg");
sf::Sprite background(backgroundTexture);
```

Якщо картинка не збігається з розміром вікна, її можна масштабувати:

```cpp
sf::Vector2u textureSize = backgroundTexture.getSize();
background.setScale({
    800.f / static_cast<float>(textureSize.x),
    600.f / static_cast<float>(textureSize.y)
});
```

Малюйте фон першим:

```cpp
window.draw(background);
window.draw(ball);
```

Проміжний результат: замість однотонного кольору у вікні є фон.

## Ідея 6. Рух фону

Найпростіший варіант рухомого фону для SFML - намалювати дві копії фону поруч і повільно зсувати їх.

Перед циклом:

```cpp
sf::Sprite bg1(backgroundTexture);
sf::Sprite bg2(backgroundTexture);

sf::Vector2u textureSize = backgroundTexture.getSize();
sf::Vector2f backgroundScale{
    800.f / static_cast<float>(textureSize.x),
    600.f / static_cast<float>(textureSize.y)
};

bg1.setScale(backgroundScale);
bg2.setScale(backgroundScale);

float bgX = 0.f;
float bgSpeed = 60.f;
```

У частині оновлення:

```cpp
bgX -= bgSpeed * dt;

if (bgX <= -800.f) {
    bgX += 800.f;
}

bg1.setPosition({bgX, 0.f});
bg2.setPosition({bgX + 800.f, 0.f});
```

У частині малювання:

```cpp
window.draw(bg1);
window.draw(bg2);
window.draw(ball);
```

Цей приклад прокручує фон тільки горизонтально. Для сильніших: спробуйте змінювати `bgX` залежно від швидкості кульки або додати `bgY` для руху вгору-вниз.

## Що можна розвивати далі

- замінити кульку на спрайт гравця;
- повертати спрайт у напрямку руху;
- зробити кадрову анімацію з `octopus01.png` ... `octopus05.png`;
- додати фон і прокрутку фону;
- зробити бонуси й ворогів як окремі спрайти;
- додати простий постріл після натискання `Space`.

Головне правило лишається тим самим: одна нова ідея - один запуск - одна перевірка.
