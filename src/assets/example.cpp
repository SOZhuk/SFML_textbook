#include <SFML/Graphics.hpp>
#include <optional>
#include <cstdlib>
#include <ctime>
#include <vector>
#include <cmath>
#include <algorithm>

// ============================================================
// РЕФЕРЕНСНИЙ ФАЙЛ ДЛЯ ЗАНЯТТЯ З SFML
// ------------------------------------------------------------
// Цей файл показує КІНЦЕВУ демонстраційну версію проєкту,
// але всередині розбитий на великі блоки-коментарі, які
// відповідають етапам покрокового нарощування коду.
//
// Його можна:
// 1) просто запустити і показати фінальну мету уроку;
// 2) використовувати як шпаргалку для вчителя;
// 3) розбирати частинами, коментуючи кожен блок окремо.
//
// Логіка заняття:
// Крок 0  - вікно і цикл
// Крок 1  - одна кулька
// Крок 2  - стан об'єкта: позиція, швидкість, радіус, колір
// Крок 3  - час кадру dt
// Крок 4  - рух
// Крок 5  - відбивання від меж
// Крок 6  - зміна кольору при відбиванні
// Крок 7  - внутрішнє поле (стіни)
// Крок 8  - керування з клавіатури
// Крок 9+ - багато кульок, спавн мишкою, зіткнення
// ============================================================

namespace {

constexpr float PI = 3.1415926535f;

// ------------------------------------------------------------
// Допоміжна функція: випадковий м'який яскравий колір.
// Відповідає ідеї кроку 6: візуальний ефект після відбивання.
// ------------------------------------------------------------
sf::Color randomColor() {
    return sf::Color(
        static_cast<std::uint8_t>(50 + std::rand() % 206),
        static_cast<std::uint8_t>(50 + std::rand() % 206),
        static_cast<std::uint8_t>(50 + std::rand() % 206)
    );
}

float lengthSquared(const sf::Vector2f& v) {
    return v.x * v.x + v.y * v.y;
}

float length(const sf::Vector2f& v) {
    return std::sqrt(lengthSquared(v));
}

sf::Vector2f normalize(const sf::Vector2f& v) {
    float len = length(v);
    if (len < 1e-6f) return {1.f, 0.f};
    return v / len;
}

float dot(const sf::Vector2f& a, const sf::Vector2f& b) {
    return a.x * b.x + a.y * b.y;
}

// ------------------------------------------------------------
// Крок 9. Коли кульок стає багато, зручно мати окрему структуру.
// Тут ми зберігаємо стан кульки окремо від її намальованої форми.
// ------------------------------------------------------------
struct Ball {
    sf::Vector2f position;   // верхній лівий кут фігури
    sf::Vector2f velocity;   // пікселів за секунду
    float radius;
    sf::Color color;
    bool playerControlled;
    int wallBounces = 0;
};

sf::Vector2f centerOf(const Ball& b) {
    return b.position + sf::Vector2f(b.radius, b.radius);
}

sf::CircleShape makeShape(const Ball& b) {
    sf::CircleShape shape(b.radius);
    shape.setPosition(b.position);
    shape.setFillColor(b.color);
    return shape;
}

// ------------------------------------------------------------
// Крок 5 + Крок 7.
// Рух всередині прямокутного поля з відбиванням від стін.
// Повертає true, якщо було відбивання.
// ------------------------------------------------------------
bool bounceFromWalls(Ball& ball, const sf::FloatRect& playArea) {
    bool bounced = false;

    if (ball.position.x <= playArea.position.x) {
        ball.position.x = playArea.position.x;
        ball.velocity.x = -ball.velocity.x;
        bounced = true;
    }
    if (ball.position.x + 2.f * ball.radius >= playArea.position.x + playArea.size.x) {
        ball.position.x = playArea.position.x + playArea.size.x - 2.f * ball.radius;
        ball.velocity.x = -ball.velocity.x;
        bounced = true;
    }
    if (ball.position.y <= playArea.position.y) {
        ball.position.y = playArea.position.y;
        ball.velocity.y = -ball.velocity.y;
        bounced = true;
    }
    if (ball.position.y + 2.f * ball.radius >= playArea.position.y + playArea.size.y) {
        ball.position.y = playArea.position.y + playArea.size.y - 2.f * ball.radius;
        ball.velocity.y = -ball.velocity.y;
        bounced = true;
    }

    if (bounced) {
        ball.color = randomColor();
        ++ball.wallBounces;
    }

    return bounced;
}

// ------------------------------------------------------------
// Крок 11. Варіант трохи кращий, ніж просто міняти швидкості місцями:
// використовуємо наближення до більярдного відбивання для однакових мас.
//
// Ідея:
// 1) будуємо нормаль між центрами кульок;
// 2) дивимось, як швидкості проєктуються на нормаль;
// 3) міняємо нормальні компоненти місцями;
// 4) тангенціальні компоненти залишаємо.
// ------------------------------------------------------------
bool resolveBallCollision(Ball& a, Ball& b) {
    sf::Vector2f ca = centerOf(a);
    sf::Vector2f cb = centerOf(b);
    sf::Vector2f delta = cb - ca;

    float dist2 = lengthSquared(delta);
    float minDist = a.radius + b.radius;

    if (dist2 > minDist * minDist) {
        return false;
    }

    float dist = std::sqrt(std::max(dist2, 1e-8f));
    sf::Vector2f n                                                                                                                                                                                                       = (dist < 1e-6f) ? sf::Vector2f(1.f, 0.f) : delta / dist;

    // Розсовуємо кульки, якщо вони перекрилися.
    float overlap = minDist - dist;
    if (overlap > 0.f) {
        sf::Vector2f correction = n * (overlap * 0.5f + 0.01f);
        a.position -= correction;
        b.position += correction;
    }

    float va = dot(a.velocity, n);
    float vb = dot(b.velocity, n);

    // Якщо кульки вже розлітаються, то повторно не обробляємо удар.
    if (va - vb <= 0.f) {
        return false;
    }

    // Однакові маси: обмін нормальними компонентами.
    float newVa = vb;
    float newVb = va;

    a.velocity += (newVa - va) * n;
    b.velocity += (newVb - vb) * n;

    a.color = randomColor();
    b.color = randomColor();
    return true;
}

// ------------------------------------------------------------
// Створення нової некерованої кульки в точці кліку миші.
// Відповідає розширенню після базової частини.
// ------------------------------------------------------------
Ball makeRandomBall(const sf::Vector2f& spawnPosition) {
    float angle = static_cast<float>(std::rand()) / RAND_MAX * 2.f * PI;
    float speed = 180.f + static_cast<float>(std::rand() % 180);

    Ball b;
    b.radius = 18.f + static_cast<float>(std::rand() % 8);
    b.position = spawnPosition - sf::Vector2f(b.radius, b.radius);
    b.velocity = {std::cos(angle) * speed, std::sin(angle) * speed};
    b.color = randomColor();
    b.playerControlled = false;
    return b;
}

} // namespace

int main() {
    std::srand(static_cast<unsigned>(std::time(nullptr)));

    // ============================================================
    // Крок 0. Створення вікна і базового циклу застосунку.
    // ============================================================
    sf::RenderWindow window(sf::VideoMode({1100, 760}), "SFML lesson reference");
    window.setFramerateLimit(60);

    // ============================================================
    // Крок 3. Годинник для dt.
    // Він дозволяє зробити швидкість більш однаковою на різних ПК.
    // ============================================================
    sf::Clock clock;

    // ============================================================
    // Крок 7. Внутрішнє прямокутне поле, де літають кульки.
    // Праворуч лишаємо місце під майбутню інформаційну зону.
    // ============================================================
    sf::FloatRect playArea({50.f, 50.f}, {760.f, 620.f});

    sf::RectangleShape border(playArea.size);
    border.setPosition(playArea.position);
    border.setFillColor(sf::Color::Transparent);
    border.setOutlineThickness(3.f);
    border.setOutlineColor(sf::Color(220, 220, 220));

    sf::RectangleShape sidePanel({220.f, 620.f});
    sidePanel.setPosition({850.f, 50.f});
    sidePanel.setFillColor(sf::Color(50, 56, 70));
    sidePanel.setOutlineThickness(2.f);
    sidePanel.setOutlineColor(sf::Color(120, 130, 150));

    // ============================================================
    // Кроки 1-2. Спочатку була одна кулька.
    // У фінальній версії тримаємо всі кульки в vector<Ball>.
    // Перша кулька буде керованою.
    // ============================================================
    std::vector<Ball> balls;
    {
        Ball player;
        player.radius = 25.f;
        player.position = {playArea.position.x + 100.f, playArea.position.y + 80.f};
        player.velocity = {220.f, 160.f};
        player.color = sf::Color::Green;
        player.playerControlled = true;
        balls.push_back(player);
    }

    int totalWallBounces = 0;
    int totalBallCollisions = 0;

    // ============================================================
    // ГОЛОВНИЙ ЦИКЛ ЗАСТОСУНКУ
    // ============================================================
    while (window.isOpen()) {
        // --------------------------------------------------------
        // Крок 3. dt = час, що минув від попереднього кадру.
        // Захистимося від дуже великих значень при паузах/лагу.
        // --------------------------------------------------------
        float dt = clock.restart().asSeconds();
        dt = std::min(dt, 0.05f);

        // --------------------------------------------------------
        // 1. ОБРОБКА ПОДІЙ
        // Тут реагуємо на разові дії користувача.
        // --------------------------------------------------------
        while (const std::optional event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>()) {
                window.close();
            }

            if (const auto* key = event->getIf<sf::Event::KeyPressed>()) {
                if (key->code == sf::Keyboard::Key::Escape) {
                    window.close();
                }
                if (key->code == sf::Keyboard::Key::Space) {
                    balls[0].color = randomColor();
                }
                if (key->code == sf::Keyboard::Key::R) {
                    balls.clear();
                    Ball player;
                    player.radius = 25.f;
                    player.position = {playArea.position.x + 100.f, playArea.position.y + 80.f};
                    player.velocity = {220.f, 160.f};
                    player.color = sf::Color::Green;
                    player.playerControlled = true;
                    balls.push_back(player);
                    totalWallBounces = 0;
                    totalBallCollisions = 0;
                }
            }

            // ----------------------------------------------------
            // Розширення: клік мишкою створює нову кульку.
            // ----------------------------------------------------
            if (const auto* mouse = event->getIf<sf::Event::MouseButtonPressed>()) {
                if (mouse->button == sf::Mouse::Button::Left) {
                    sf::Vector2f p(static_cast<float>(mouse->position.x), static_cast<float>(mouse->position.y));
                    if (playArea.contains(p)) {
                        balls.push_back(makeRandomBall(p));
                    }
                }
            }
        }

        // --------------------------------------------------------
        // 2. ОНОВЛЕННЯ СТАНУ
        // --------------------------------------------------------

        // --------------------------------------------------------
        // Крок 8. Клавіатура впливає на швидкість керованої кульки.
        // Тут ми керуємо саме velocity, а не position.
        // --------------------------------------------------------
        {
            Ball& player = balls[0];
            float accel = 420.f;

            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left)) {
                player.velocity.x -= accel * dt;
            }
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right)) {
                player.velocity.x += accel * dt;
            }
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Up)) {
                player.velocity.y -= accel * dt;
            }
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Down)) {
                player.velocity.y += accel * dt;
            }

            float maxSpeed = 550.f;
            player.velocity.x = std::clamp(player.velocity.x, -maxSpeed, maxSpeed);
            player.velocity.y = std::clamp(player.velocity.y, -maxSpeed, maxSpeed);
        }

        // --------------------------------------------------------
        // Крок 4. Рух усіх кульок.
        // position += velocity * dt
        // --------------------------------------------------------
        for (Ball& ball : balls) {
            ball.position += ball.velocity * dt;
        }

        // --------------------------------------------------------
        // Крок 5 + 6 + 7. Відбивання від меж поля та зміна кольору.
        // --------------------------------------------------------
        for (Ball& ball : balls) {
            if (bounceFromWalls(ball, playArea)) {
                ++totalWallBounces;
            }
        }

        // --------------------------------------------------------
        // Крок 11. Зіткнення кульок.
        // --------------------------------------------------------
        for (std::size_t i = 0; i < balls.size(); ++i) {
            for (std::size_t j = i + 1; j < balls.size(); ++j) {
                if (resolveBallCollision(balls[i], balls[j])) {
                    ++totalBallCollisions;
                }
            }
        }

        // --------------------------------------------------------
        // 3. МАЛЮВАННЯ КАДРУ
        // clear -> draw -> display
        // --------------------------------------------------------
        window.clear(sf::Color(28, 30, 38));

        // Поле і бічна панель.
        window.draw(border);
        window.draw(sidePanel);

        // Декоративні індикатори замість тексту.
        // Це дозволяє обійтись без шрифтів у шаблоні.
        // 1) кількість кульок
        for (int i = 0; i < std::min<int>(static_cast<int>(balls.size()), 12); ++i) {
            sf::CircleShape dot(7.f);
            dot.setPosition({875.f + (i % 4) * 22.f, 90.f + (i / 4) * 22.f});
            dot.setFillColor(i == 0 ? sf::Color::Green : sf::Color(120, 190, 255));
            window.draw(dot);
        }

        // 2) лічильник відбивань від стін — смужки.
        {
            int bars = std::min(totalWallBounces, 20);
            for (int i = 0; i < bars; ++i) {
                sf::RectangleShape bar({10.f, 8.f + static_cast<float>((i % 5) * 4)});
                bar.setPosition({875.f + i * 10.f, 180.f});
                bar.setFillColor(sf::Color(255, 210, 90));
                window.draw(bar);
            }
        }

        // 3) лічильник зіткнень кульок — вертикальні смужки.
        {
            int bars = std::min(totalBallCollisions, 16);
            for (int i = 0; i < bars; ++i) {
                sf::RectangleShape bar({8.f, 30.f + static_cast<float>((i % 4) * 8)});
                bar.setPosition({875.f + i * 12.f, 250.f});
                bar.setFillColor(sf::Color(255, 120, 120));
                window.draw(bar);
            }
        }

        // 4) Вектор швидкості керованої кульки як індикатор.
        {
            const Ball& player = balls[0];
            sf::Vector2f v = player.velocity;
            float speed = length(v);
            sf::Vector2f dir = (speed < 1e-6f) ? sf::Vector2f(1.f, 0.f) : v / speed;

            sf::RectangleShape gauge({std::min(speed * 0.18f, 160.f), 12.f});
            gauge.setPosition({875.f, 340.f});
            gauge.setFillColor(sf::Color(120, 255, 160));
            window.draw(gauge);

            sf::RectangleShape directionArrow({55.f, 6.f});
            directionArrow.setOrigin({0.f, 3.f});
            directionArrow.setPosition({955.f, 410.f});
            directionArrow.setRotation(sf::degrees(std::atan2(dir.y, dir.x) * 180.f / PI));
            directionArrow.setFillColor(sf::Color(180, 220, 255));
            window.draw(directionArrow);

            sf::CircleShape tip(6.f);
            tip.setFillColor(sf::Color(180, 220, 255));
            tip.setPosition({955.f + dir.x * 55.f - 6.f, 410.f + dir.y * 55.f - 6.f});
            window.draw(tip);
        }

        // Усі кульки.
        for (const Ball& ball : balls) {
            window.draw(makeShape(ball));
        }

        window.display();
    }
}
