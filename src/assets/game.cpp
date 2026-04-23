#include <SFML/Graphics.hpp>
#include <algorithm>
#include <cmath>
#include <ctime>
#include <filesystem>
#include <iostream>
#include <memory>
#include <optional>
#include <random>
#include <string>
#include <utility>
#include <vector>

// ============================================================
// game_sfml.cpp
// ------------------------------------------------------------
// Порт файлу game.py з pygame на SFML.
//
// Логіка збережена максимально близько до оригіналу:
// - є 2 варіанти гравця: кулька або спрайт-гуска;
// - є вороги, бонуси, nomad-кульки, "постріли";
// - вороги/бонуси з'являються по таймерах;
// - фон прокручується проти руху гравця;
// - є рахунок, рівень, життя, екран game over і перезапуск.
//
// Важливі зауваження:
// 1. Для компіляції потрібні ваші ресурси:
//      enemy.png
//      bonus.png
//      player.png
//      starsky.jpg
//      папка goose/ з кадрами гуски
// 2. Для тексту код намагається завантажити один з типових шрифтів.
//    Якщо шрифт не знайдеться, гра все одно працюватиме, але без тексту.
// 3. Колізія зі спрайтами тут теж наближена, як і в оригіналі pygame-версії:
//    використовується зменшений прямокутник/радіус, а не піксельна маска.
// ============================================================

namespace {

constexpr float PI = 3.1415926535f;
constexpr int PLAYER_VERSION = 2; // 1 - powerball, інакше goose image version

constexpr float MAX_VELOCITY = 10.0f;
constexpr unsigned WINDOW_WIDTH = 1200;
constexpr unsigned WINDOW_HEIGHT = 800;
constexpr float INSTRUCTION_AREA = 50.0f;
constexpr float BORDER_WIDTH = 10.0f;

constexpr float CURVED = 0.000f;
constexpr float GRAVITY = 0.01f;
constexpr float EPSILON = 0.00005f;
constexpr float BOUNCE_BRAKING = 0.001f;

const sf::Color COLOR_WHITE(255, 255, 255);
const sf::Color COLOR_BLUE(0, 0, 255);
const sf::Color COLOR_YELLOW(255, 255, 0);
const sf::Color COLOR_RED(255, 10, 10);
const sf::Color COLOR_CYAN(10, 255, 255);
const sf::Color COLOR_BLACK(0, 0, 0);
const sf::Color COLOR_ORANGE(255, 180, 0);
const sf::Color COLOR_GRAY(128, 128, 128);
const sf::Color COLOR_GREEN(10, 255, 10);

std::mt19937& rng() {
    static std::mt19937 gen(static_cast<unsigned>(std::time(nullptr)));
    return gen;
}

float randFloat(float l, float r) {
    std::uniform_real_distribution<float> dist(l, r);
    return dist(rng());
}

int randInt(int l, int r) {
    std::uniform_int_distribution<int> dist(l, r);
    return dist(rng());
}

sf::Color randomColour(float luminance = 72.0f) {
    while (true) {
        int r = randInt(0, 255);
        int b = randInt(0, 255);
        int g = static_cast<int>(std::round((luminance - 0.2126f * r - 0.0722f * b) / 0.7152f));
        if (0 <= g && g <= 255) {
            return sf::Color(static_cast<std::uint8_t>(r),
                             static_cast<std::uint8_t>(g),
                             static_cast<std::uint8_t>(b));
        }
    }
}

float lengthSq(const sf::Vector2f& v) {
    return v.x * v.x + v.y * v.y;
}

float length(const sf::Vector2f& v) {
    return std::sqrt(lengthSq(v));
}

float dot(const sf::Vector2f& a, const sf::Vector2f& b) {
    return a.x * b.x + a.y * b.y;
}

sf::Vector2f normalized(const sf::Vector2f& v) {
    float len = length(v);
    if (len < 1e-8f) return {1.f, 0.f};
    return v / len;
}

sf::FloatRect makePlayArea() {
    return {{BORDER_WIDTH, BORDER_WIDTH + INSTRUCTION_AREA},
            {WINDOW_WIDTH - 2.f * BORDER_WIDTH, WINDOW_HEIGHT - 2.f * BORDER_WIDTH - INSTRUCTION_AREA}};
}

std::optional<sf::Font> tryLoadFont() {
    std::vector<std::string> names = {
        "arial.ttf",
        "Arial.ttf",
        "DejaVuSans.ttf",
        "C:/Windows/Fonts/arial.ttf",
        "C:/Windows/Fonts/consola.ttf",
        "C:/Windows/Fonts/cour.ttf"
    };

    for (const auto& name : names) {
        sf::Font font;
        if (font.openFromFile(name)) {
            return font;
        }
    }
    return std::nullopt;
}

bool loadTexture(sf::Texture& texture, const std::string& filename) {
    const std::vector<std::filesystem::path> candidates = {
        filename,
        std::filesystem::path("python") / filename,
        std::filesystem::path("..") / filename,
        std::filesystem::path("..") / "python" / filename
    };

    auto found = std::find_if(candidates.begin(), candidates.end(), [](const std::filesystem::path& path) {
        return std::filesystem::exists(path);
    });

    if (found == candidates.end() || !texture.loadFromFile(found->string())) {
        std::cerr << "Cannot load texture: " << filename << '\n';
        return false;
    }
    texture.setSmooth(true);
    return true;
}

sf::Texture scaledTextureCopy(const sf::Texture& src, float factor) {
    sf::Image img = src.copyToImage();
    sf::Vector2u s = src.getSize();
    sf::Image scaled;
    scaled.resize({static_cast<unsigned>(std::max(1.0f, std::round(s.x * factor))),
                   static_cast<unsigned>(std::max(1.0f, std::round(s.y * factor)))}, sf::Color::Transparent);

    for (unsigned y = 0; y < scaled.getSize().y; ++y) {
        for (unsigned x = 0; x < scaled.getSize().x; ++x) {
            unsigned sx = std::min(s.x - 1, static_cast<unsigned>(x / factor));
            unsigned sy = std::min(s.y - 1, static_cast<unsigned>(y / factor));
            scaled.setPixel({x, y}, img.getPixel({sx, sy}));
        }
    }

    sf::Texture out;
    if (!out.loadFromImage(scaled)) {
        return {};
    }
    out.setSmooth(true);
    return out;
}

// ------------------------------------------------------------
// Базовий абстрактний об'єкт з лічильником часу існування.
// Відповідає AbstractObject з game.py.
// ------------------------------------------------------------
class AbstractObject {
public:
    virtual ~AbstractObject() = default;
    float getTime() const { return time_; }
    float incTime(float amount) {
        time_ += amount;
        return time_;
    }
protected:
    float time_ = 0.0f;
};

struct TailNode {
    sf::Vector2f position;
    sf::Color color;
};

// ------------------------------------------------------------
// PowerBall — базова фізична кулька з хвостом.
// Усе обчислюється відносно ЦЕНТРУ об'єкта.
// ------------------------------------------------------------
class PowerBall : public AbstractObject {
public:
    PowerBall(const sf::FloatRect& bounds,
              sf::Vector2f position = {0.f, 0.f},
              sf::Vector2f speed = {0.f, 0.f},
              float size = 20.f,
              sf::Color color = sf::Color::White,
              int tailLength = 10,
              bool bounce = true,
              bool changeColor = true)
        : bounds_(bounds),
          bounceFlag_(bounce),
          changeColor_(changeColor),
          color_(color),
          radius_(size / 2.f),
          position_(position),
          speed_(speed),
          tailLength_(std::max(0, tailLength)) {
        for (int i = 0; i < tailLength_; ++i) {
            tail_.push_back({position_, color_});
        }
    }

    virtual ~PowerBall() = default;

    virtual void update(float dt) {
        incTime(dt);

        for (int i = tailLength_ - 1; i >= 1; --i) {
            tail_[i] = tail_[i - 1];
            tail_[i].color = sf::Color(
                static_cast<std::uint8_t>(std::clamp(0.9f * tail_[i - 1].color.r + 0.1f * tail_[i - 1].color.g * randFloat(0.f, 1.f), 0.f, 255.f)),
                static_cast<std::uint8_t>(std::clamp(0.9f * tail_[i - 1].color.g + 0.1f * tail_[i - 1].color.b * randFloat(0.f, 1.f), 0.f, 255.f)),
                static_cast<std::uint8_t>(std::clamp(0.9f * tail_[i - 1].color.b + 0.1f * tail_[i - 1].color.r * randFloat(0.f, 1.f), 0.f, 255.f))
            );
        }
        if (!tail_.empty()) {
            tail_[0] = {position_, color_};
        }

        float tickScale = dt * 120.f;
        rotate(CURVED * tickScale);
        speed_ *= (1.0f - EPSILON * tickScale);
        speed_.y += GRAVITY * tickScale;
        position_ += speed_ * tickScale;

        if (bounceFlag_) {
            checkBounce();
        }
    }

    virtual void draw(sf::RenderTarget& target) const {
        for (auto it = tail_.rbegin(); it != tail_.rend(); ++it) {
            sf::CircleShape c(radius_);
            c.setFillColor(it->color);
            c.setOrigin({radius_, radius_});
            c.setPosition(it->position);
            target.draw(c);
        }

        sf::CircleShape body(radius_);
        body.setFillColor(color_);
        body.setOrigin({radius_, radius_});
        body.setPosition(position_);
        target.draw(body);
    }

    virtual sf::FloatRect collisionBox() const {
        return {position_ - sf::Vector2f(radius_, radius_), {2.f * radius_, 2.f * radius_}};
    }

    bool checkCollision(const PowerBall& other) const {
        return collisionBox().findIntersection(other.collisionBox()).has_value();
    }

    void elasticCollide(PowerBall& other) {
        sf::Vector2f a = other.position_ - position_;
        float r = length(a);
        if (r < 1e-6f) return;
        a /= r;

        sf::Vector2f v1p = a * dot(speed_, a);
        sf::Vector2f v2p = a * dot(other.speed_, a);
        if (dot(v1p, a) < 0.f && dot(v2p, a) < 0.f) return;

        setVelocityXY(speed_.x - v1p.x + v2p.x, speed_.y - v1p.y + v2p.y);
        other.setVelocityXY(other.speed_.x - v2p.x + v1p.x, other.speed_.y - v2p.y + v1p.y);
    }

    bool escaped() const {
        return (position_.y + radius_ < bounds_.position.y - 2.f * radius_ ||
                position_.x + radius_ < bounds_.position.x - 2.f * radius_ ||
                position_.y - radius_ > bounds_.position.y + bounds_.size.y + 2.f * radius_ ||
                position_.x - radius_ > bounds_.position.x + bounds_.size.x + 2.f * radius_);
    }

    void rotate(float angle) {
        float c = std::cos(angle);
        float s = std::sin(angle);
        speed_ = {speed_.x * c + speed_.y * s, speed_.y * c - speed_.x * s};
    }

    void accelerate(float fraction = 1.f, float absolute = 0.f) {
        float v = velocity() * fraction + absolute;
        if (v > MAX_VELOCITY) v = MAX_VELOCITY;
        if (v < 0.f) v = 0.0001f;
        setVelocity(v);
    }

    float velocity() const { return length(speed_); }

    void setVelocityXY(float vx, float vy) {
        speed_ = {vx, vy};
    }

    void setVelocity(float newVelocity) {
        float oldVelocity = velocity();
        if (oldVelocity < 1e-6f) {
            speed_ = {newVelocity, 0.f};
        } else {
            speed_ = speed_ / oldVelocity * newVelocity;
        }
    }

    const sf::Vector2f& position() const { return position_; }
    const sf::Vector2f& speed() const { return speed_; }
    float radius() const { return radius_; }

protected:
    bool checkBounce() {
        bool bounced = false;
        float left = bounds_.position.x + radius_;
        float right = bounds_.position.x + bounds_.size.x - radius_;
        float top = bounds_.position.y + radius_;
        float bottom = bounds_.position.y + bounds_.size.y - radius_;

        if (collisionBox().position.y + collisionBox().size.y >= bounds_.position.y + bounds_.size.y && speed_.y > 0.f) {
            position_.y = bottom;
            speed_.y *= -1.f;
            bounced = true;
        }
        if (collisionBox().position.x + collisionBox().size.x >= bounds_.position.x + bounds_.size.x && speed_.x > 0.f) {
            position_.x = right;
            speed_.x *= -1.f;
            bounced = true;
        }
        if (collisionBox().position.y <= bounds_.position.y && speed_.y < 0.f) {
            position_.y = top;
            speed_.y *= -1.f;
            bounced = true;
        }
        if (collisionBox().position.x <= bounds_.position.x && speed_.x < 0.f) {
            position_.x = left;
            speed_.x *= -1.f;
            bounced = true;
        }

        if (bounced) {
            bounce();
        }
        return bounced;
    }

    virtual void bounce() {
        speed_ *= (1.0f - BOUNCE_BRAKING);
        if (changeColor_) {
            color_ = randomColour(100.0f);
        }
    }

protected:
    sf::FloatRect bounds_;
    bool bounceFlag_ = true;
    bool changeColor_ = true;
    sf::Color color_ = sf::Color::White;
    float radius_ = 10.f;
    sf::Vector2f position_;
    sf::Vector2f speed_;
    int tailLength_ = 0;
    std::vector<TailNode> tail_;
};

// ------------------------------------------------------------
// PowerImage — спрайтовий об'єкт, який успадковує фізику руху,
// але малюється не як коло, а як зображення, повернуте за напрямом.
// ------------------------------------------------------------
class PowerImage : public PowerBall {
public:
    PowerImage(const sf::FloatRect& bounds,
               const sf::Texture* image,
               sf::Vector2f position,
               sf::Vector2f speed,
               bool bounce = true,
               bool changeColor = true,
               float defaultAngle = 0.f)
        : PowerBall(bounds, position, speed,
                    static_cast<float>(std::max(image ? image->getSize().x : 1u, image ? image->getSize().y : 1u)),
                    sf::Color::Black, 0, bounce, changeColor),
          image_(image),
          defaultAngle_(defaultAngle) {}

    void setImage(const sf::Texture* image) {
        image_ = image;
        if (image_) {
            radius_ = static_cast<float>(std::max(image_->getSize().x, image_->getSize().y)) / 2.f;
        }
    }

    float imageAngleRotate() const {
        return std::atan2(-speed_.y, speed_.x) * 180.f / PI + defaultAngle_;
    }

    void draw(sf::RenderTarget& target) const override {
        if (!image_) {
            PowerBall::draw(target);
            return;
        }

        sf::Sprite sprite(*image_);
        auto texSize = image_->getSize();
        sprite.setOrigin({texSize.x / 2.f, texSize.y / 2.f});
        sprite.setPosition(position_);
        sprite.setRotation(sf::degrees(imageAngleRotate()));
        target.draw(sprite);
    }

    sf::FloatRect collisionBox() const override {
        if (!image_) return PowerBall::collisionBox();
        auto texSize = image_->getSize();
        float w = static_cast<float>(texSize.x);
        float h = static_cast<float>(texSize.y);
        float cw = std::max(8.f, w * 2.f / 3.f);
        float ch = std::max(8.f, h * 2.f / 3.f);
        return {position_ - sf::Vector2f(cw / 2.f, ch / 2.f), {cw, ch}};
    }

private:
    const sf::Texture* image_ = nullptr;
    float defaultAngle_ = 0.f;
};

struct Resources {
    sf::Texture enemyTexture;
    sf::Texture bonusTexture;
    sf::Texture playerTexture;
    sf::Texture starTexture;
    std::vector<sf::Texture> gooseFrames;
    std::optional<sf::Font> font;

    bool load() {
        bool ok = true;
        ok &= loadTexture(enemyTexture, "enemy.png");
        ok &= loadTexture(bonusTexture, "bonus.png");
        ok &= loadTexture(playerTexture, "player.png");
        ok &= loadTexture(starTexture, "starsky.jpg");
        if (playerTexture.getSize().x != 0) {
            playerTexture = scaledTextureCopy(playerTexture, 0.5f);
        }
        font = tryLoadFont();

        // Масштаб player.png у pygame було scale_by(..., 0.5).
        // У SFML найпростіше не перепаковувати файл, а застосувати scale при малюванні.
        // Але для узгодженості колізій тут залишаємо текстуру як є, а масштаб задаємо геометрією руху.

        const std::vector<std::filesystem::path> gooseDirCandidates = {
            "goose",
            "Goose",
            std::filesystem::path("python") / "goose",
            std::filesystem::path("python") / "Goose",
            std::filesystem::path("..") / "goose",
            std::filesystem::path("..") / "Goose",
            std::filesystem::path("..") / "python" / "goose",
            std::filesystem::path("..") / "python" / "Goose"
        };
        auto gooseDir = std::find_if(gooseDirCandidates.begin(), gooseDirCandidates.end(), [](const std::filesystem::path& path) {
            return std::filesystem::exists(path) && std::filesystem::is_directory(path);
        });

        if (gooseDir != gooseDirCandidates.end()) {
            std::vector<std::filesystem::path> paths;
            for (const auto& entry : std::filesystem::directory_iterator(*gooseDir)) {
                if (entry.is_regular_file()) {
                    paths.push_back(entry.path());
                }
            }
            std::sort(paths.begin(), paths.end());
            for (const auto& path : paths) {
                sf::Texture t;
                if (t.loadFromFile(path.string())) {
                    gooseFrames.push_back(scaledTextureCopy(t, 0.5f));
                }
            }
        }

        if (gooseFrames.empty()) {
            std::cerr << "Goose frames not found in folder goose/\n";
        }

        return ok;
    }
};

struct GameState {
    bool repeatGame = true;
    bool playingFlag = true;
    int lives = 5;
    int points = 0;
    int level = 1;
    float enemyAppearDelay = 2.0f;
    float deltaX = 0.0f;
    float deltaY = 0.0f;
};

void drawInstructionBar(sf::RenderTarget& target, const sf::Font* font) {
    sf::RectangleShape topBar({static_cast<float>(WINDOW_WIDTH), INSTRUCTION_AREA});
    topBar.setPosition({0.f, 0.f});
    topBar.setFillColor(COLOR_BLUE);
    target.draw(topBar);

    if (font) {
        sf::Text text(*font,
                      "left, right: change direction. up, down: accelerate and decelerate, ctrl fire. esc - exit",
                      26);
        text.setFillColor(COLOR_WHITE);
        text.setPosition({50.f, 12.f});
        target.draw(text);
    }
}

void drawFrameBorder(sf::RenderTarget& target, const sf::FloatRect& frame) {
    sf::RectangleShape border(frame.size);
    border.setPosition(frame.position);
    border.setFillColor(COLOR_BLACK);
    border.setOutlineThickness(BORDER_WIDTH);
    border.setOutlineColor(COLOR_BLUE);
    target.draw(border);
}

// Малювання тла з прокруткою — аналог 4 blit-ів starsky у pygame.
void drawScrollingBackground(sf::RenderTarget& target,
                             const sf::Texture& starTexture,
                             const sf::FloatRect& frame,
                             float offsetX,
                             float offsetY) {
    sf::Sprite s1(starTexture), s2(starTexture), s3(starTexture), s4(starTexture);
    auto texSize = starTexture.getSize();
    if (texSize.x == 0 || texSize.y == 0) return;

    float scaleX = frame.size.x / texSize.x;
    float scaleY = frame.size.y / texSize.y;

    for (sf::Sprite* s : {&s1, &s2, &s3, &s4}) {
        s->setScale({scaleX, scaleY});
    }

    s1.setPosition({frame.position.x + offsetX - frame.size.x, frame.position.y + offsetY - frame.size.y});
    s2.setPosition({frame.position.x + offsetX,                  frame.position.y + offsetY - frame.size.y});
    s3.setPosition({frame.position.x + offsetX - frame.size.x, frame.position.y + offsetY});
    s4.setPosition({frame.position.x + offsetX,                  frame.position.y + offsetY});

    target.draw(s1);
    target.draw(s2);
    target.draw(s3);
    target.draw(s4);
}

PowerImage createEnemy(const sf::FloatRect& frame, const sf::Texture& texture) {
    sf::Vector2f speed(randFloat(-5.f, -2.f), randFloat(-1.f, 1.f));
    int halfH = static_cast<int>(texture.getSize().y / 2);
    float y = static_cast<float>(randInt(static_cast<int>(frame.position.y + 30.f),
                                         static_cast<int>(frame.position.y + frame.size.y - 20.f - halfH)));
    if (y > frame.position.y + frame.size.y * 2.f / 3.f && speed.y > 0.f) speed.y *= -1.f;
    if (y < frame.position.y + frame.size.y / 3.f && speed.y < 0.f) speed.y *= -1.f;

    return PowerImage(frame,
                      &texture,
                      sf::Vector2f(frame.position.x + frame.size.x - texture.getSize().x / 2.f, y),
                      speed,
                      false,
                      false,
                      180.f);
}

PowerImage createBonus(const sf::FloatRect& frame, const sf::Texture& texture) {
    sf::Vector2f speed(randFloat(-1.f, 1.f), randFloat(1.f, 3.f));
    int halfW = static_cast<int>(texture.getSize().x / 2);
    float x = static_cast<float>(randInt(static_cast<int>(frame.position.x + 40.f),
                                         static_cast<int>(frame.position.x + frame.size.x - 40.f - halfW)));
    if (x > frame.position.x + frame.size.x * 2.f / 3.f && speed.x > 0.f) speed.y *= -1.f;
    if (x < frame.position.x + frame.size.y / 3.f && speed.x < 0.f) speed.y *= -1.f;

    return PowerImage(frame,
                      &texture,
                      sf::Vector2f(x, frame.position.y + 1.f),
                      speed,
                      false,
                      false,
                      90.f);
}

PowerBall createNomad(const sf::FloatRect& frame, float spd = 1.f) {
    return PowerBall(frame,
                     sf::Vector2f(randFloat(frame.position.x + 20.f, frame.position.x + frame.size.x - 20.f),
                                  randFloat(frame.position.y + 20.f, frame.position.y + frame.size.y - 20.f)),
                     sf::Vector2f(randFloat(-spd, spd), randFloat(-spd, spd)),
                     20.f,
                     randomColour(200.0f),
                     40,
                     true,
                     false);
}

// Додає "постріл" — тобто нову nomad-кульку вздовж швидкості гравця.
void fire(PowerBall& player, std::vector<PowerBall>& nomads, const sf::FloatRect& frame) {
    if (player.velocity() < 2.f) return;
    if (nomads.size() > 5) return;

    sf::Vector2f dir = normalized(player.speed());
    sf::Vector2f spawn = player.position() + dir * (player.radius() * 2.f + 20.f);

    nomads.emplace_back(frame,
                        spawn,
                        player.speed(),
                        20.f,
                        randomColour(220.f),
                        static_cast<int>(player.velocity() * 5.f + 5.f),
                        true,
                        false);
    player.setVelocity(0.5f);
}

void score(GameState& state,
           std::vector<PowerBall>& nomads) {
    state.points += 1;
    if (state.points % 5 == 0) {
        nomads.clear();
        ++state.level;
        state.enemyAppearDelay *= 0.66f;
    }
    if (state.points % 8 == 0) {
        ++state.lives;
    }
}

void scoreBonus(GameState& state,
                std::vector<PowerBall>& nomads,
                int add) {
    int old = state.points;
    state.points += add;
    if (state.points / 5 > old / 5) {
        nomads.clear();
        ++state.level;
        state.enemyAppearDelay *= 0.66f;
    }
    if (state.points / 8 > old / 8) {
        ++state.lives;
    }
}

void drawHUD(sf::RenderTarget& target, const sf::Font* font, const GameState& state, const sf::FloatRect& frame) {
    if (!font) return;

    sf::Text hearts(*font, std::u32string(static_cast<std::size_t>(std::max(0, state.lives)), U'♥'), 54);
    hearts.setFillColor(COLOR_RED);
    hearts.setPosition(sf::Vector2f(frame.position.x + 10.f, frame.position.y + 10.f));
    target.draw(hearts);

    sf::Text scoreText(*font, "Score: " + std::to_string(state.points), 22);
    scoreText.setFillColor(COLOR_GREEN);
    scoreText.setPosition(sf::Vector2f(frame.position.x + frame.size.x - 150.f, frame.position.y + 20.f));
    target.draw(scoreText);

    sf::Text levelText(*font, "Level: " + std::to_string(state.level), 22);
    levelText.setFillColor(COLOR_YELLOW);
    levelText.setPosition(sf::Vector2f(frame.position.x + 20.f, frame.position.y + frame.size.y - 40.f));
    target.draw(levelText);
}

} // namespace

int main() {
    sf::RenderWindow window(sf::VideoMode({WINDOW_WIDTH, WINDOW_HEIGHT}), "game.py -> SFML");
    window.setFramerateLimit(120);

    const sf::FloatRect frame = makePlayArea();

    Resources res;
    res.load();
    sf::Font* fontPtr = res.font ? &*res.font : nullptr;

    bool repeatGame = true;

    while (window.isOpen() && repeatGame) {
        GameState state;

        // --------------------------------------------------------
        // Створення гравця. Логіка як у pygame-версії:
        // або PowerBall, або PowerImage (гуска / player.png).
        // --------------------------------------------------------
        std::unique_ptr<PowerBall> player;
        if (PLAYER_VERSION == 1 || !res.playerTexture.getSize().x) {
            player = std::make_unique<PowerBall>(
                frame,
                sf::Vector2f(randFloat(frame.position.x + 20.f, frame.position.x + frame.size.x - 20.f),
                             randFloat(frame.position.y + 20.f, frame.position.y + frame.size.y - 20.f)),
                sf::Vector2f(randFloat(-5.f, 5.f), randFloat(-5.f, 5.f)),
                20.f,
                COLOR_YELLOW,
                50,
                true,
                true
            );
        } else {
            player = std::make_unique<PowerImage>(
                frame,
                &res.playerTexture,
                sf::Vector2f(frame.position.x + frame.size.x / 4.f, frame.position.y + frame.size.y / 2.f),
                sf::Vector2f(randFloat(1.f, 3.f), randFloat(-1.f, 1.f)),
                true,
                true,
                0.f
            );
        }

        std::vector<PowerBall> nomads;
        for (int i = 0; i < 2; ++i) {
            nomads.push_back(createNomad(frame));
        }
        std::vector<PowerImage> enemies;
        std::vector<PowerImage> bonuses;

        sf::Clock dtClock;
        sf::Clock enemyClock;
        sf::Clock bonusClock;
        sf::Clock nomadClock;
        sf::Clock gooseClock;

        std::size_t gooseIndex = 0;

        // --------------------------------------------------------
        // Головний ігровий цикл одного запуску гри.
        // --------------------------------------------------------
        while (window.isOpen() && state.playingFlag && state.lives > 0) {
            float dt = dtClock.restart().asSeconds();
            dt = std::min(dt, 0.05f);

            while (const std::optional event = window.pollEvent()) {
                if (event->is<sf::Event::Closed>()) {
                    window.close();
                }
            }

            // Клавіатура: прямий аналог pygame.key.get_pressed().
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left)) {
                player->rotate(0.05f * dt * 120.f);
            }
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right)) {
                player->rotate(-0.05f * dt * 120.f);
            }
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Up)) {
                player->accelerate(0.95f, 0.4f);
            }
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Down)) {
                player->accelerate(0.9f);
            }
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Escape)) {
                state.playingFlag = false;
            }
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::LControl) ||
                sf::Keyboard::isKeyPressed(sf::Keyboard::Key::RControl) ||
                sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Space)) {
                fire(*player, nomads, frame);
            }

            // Таймери подій.
            if (enemyClock.getElapsedTime().asSeconds() >= state.enemyAppearDelay) {
                enemyClock.restart();
                if (res.enemyTexture.getSize().x) {
                    enemies.push_back(createEnemy(frame, res.enemyTexture));
                }
            }
            if (bonusClock.getElapsedTime().asSeconds() >= 2.5f) {
                bonusClock.restart();
                if (res.bonusTexture.getSize().x) {
                    bonuses.push_back(createBonus(frame, res.bonusTexture));
                }
            }
            if (nomadClock.getElapsedTime().asSeconds() >= 5.0f) {
                nomadClock.restart();
                if (!nomads.empty()) {
                    nomads.erase(nomads.begin());
                } else {
                    nomads.push_back(createNomad(frame, static_cast<float>(state.level)));
                }
            }
            if (gooseClock.getElapsedTime().asSeconds() >= 0.251f) {
                gooseClock.restart();
                if (PLAYER_VERSION != 1 && !res.gooseFrames.empty()) {
                    gooseIndex = (gooseIndex + 1) % res.gooseFrames.size();
                    if (auto* imagePlayer = dynamic_cast<PowerImage*>(player.get())) {
                        imagePlayer->setImage(&res.gooseFrames[gooseIndex]);
                    }
                }
            }

            // Прокрутка фону — майже пряма логіка з pygame.
            if (player->speed().x > 1.f) state.deltaX -= 1.f;
            if (player->speed().x < -1.f) state.deltaX += 1.f;
            if (player->speed().y < -1.f) state.deltaY += 1.f;
            if (player->speed().y > 1.f) state.deltaY -= 1.f;

            while (state.deltaY < 0.f) state.deltaY += frame.size.y;
            while (state.deltaX < 0.f) state.deltaX += frame.size.x;
            state.deltaX = std::fmod(state.deltaX, frame.size.x);
            state.deltaY = std::fmod(state.deltaY, frame.size.y);

            // Оновлення об'єктів.
            player->update(dt);
            for (auto& p : nomads) p.update(dt);
            for (auto& p : enemies) p.update(dt);
            for (auto& p : bonuses) p.update(dt);

            nomads.erase(std::remove_if(nomads.begin(), nomads.end(), [](const PowerBall& p) { return p.escaped(); }), nomads.end());
            enemies.erase(std::remove_if(enemies.begin(), enemies.end(), [](const PowerImage& p) { return p.escaped(); }), enemies.end());
            bonuses.erase(std::remove_if(bonuses.begin(), bonuses.end(), [](const PowerImage& p) { return p.escaped(); }), bonuses.end());

            // ----------------------------------------------------
            // Перевірка зіткнень — збережена логіка з game.py.
            // ----------------------------------------------------
            for (std::size_t i = 0; i < nomads.size();) {
                bool removedNomad = false;

                for (std::size_t j = 0; j < bonuses.size() && !removedNomad;) {
                    if (nomads[i].checkCollision(bonuses[j])) {
                        nomads[i].elasticCollide(bonuses[j]);
                        nomads.erase(nomads.begin() + static_cast<long long>(i));
                        removedNomad = true;
                    } else {
                        ++j;
                    }
                }
                if (removedNomad) continue;

                for (std::size_t j = 0; j < enemies.size() && !removedNomad;) {
                    if (nomads[i].checkCollision(enemies[j])) {
                        nomads.erase(nomads.begin() + static_cast<long long>(i));
                        enemies.erase(enemies.begin() + static_cast<long long>(j));
                        score(state, nomads);
                        removedNomad = true;
                    } else {
                        ++j;
                    }
                }
                if (removedNomad) continue;

                if (player->checkCollision(nomads[i])) {
                    player->elasticCollide(nomads[i]);
                    nomads.erase(nomads.begin() + static_cast<long long>(i));
                    continue;
                }

                ++i;
            }

            bool playerHit = false;
            for (auto& e : enemies) {
                if (e.checkCollision(*player)) {
                    player->elasticCollide(e);
                    enemies.clear();
                    nomads.clear();
                    bonuses.clear();
                    --state.lives;
                    playerHit = true;
                    break;
                }
            }
            (void)playerHit;

            for (std::size_t i = 0; i < bonuses.size();) {
                if (player->checkCollision(bonuses[i])) {
                    bonuses.erase(bonuses.begin() + static_cast<long long>(i));
                    if (!nomads.empty()) {
                        std::size_t cut = nomads.size() / 2;
                        nomads.erase(nomads.begin(), nomads.begin() + static_cast<long long>(cut));
                    }
                    scoreBonus(state, nomads, 2);
                } else {
                    ++i;
                }
            }

            // Малювання кадру.
            window.clear(COLOR_BLUE);
            drawInstructionBar(window, fontPtr);
            drawFrameBorder(window, frame);
            if (res.starTexture.getSize().x) {
                drawScrollingBackground(window, res.starTexture, frame, state.deltaX, state.deltaY);
            }

            player->draw(window);
            for (const auto& p : nomads) p.draw(window);
            for (const auto& p : enemies) p.draw(window);
            for (const auto& p : bonuses) p.draw(window);
            drawHUD(window, fontPtr, state, frame);

            window.display();
        }

        if (!window.isOpen()) break;

        if (state.lives == 0) {
            bool waiting = true;
            while (window.isOpen() && waiting) {
                while (const std::optional event = window.pollEvent()) {
                    if (event->is<sf::Event::Closed>()) {
                        window.close();
                    }
                }

                if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Escape)) {
                    waiting = false;
                    repeatGame = false;
                }
                if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Space)) {
                    waiting = false;
                    repeatGame = true;
                }

                window.clear(COLOR_BLUE);
                drawInstructionBar(window, fontPtr);
                drawFrameBorder(window, frame);
                if (res.starTexture.getSize().x) {
                    drawScrollingBackground(window, res.starTexture, frame, state.deltaX, state.deltaY);
                }

                if (fontPtr) {
                    sf::Text txt(*fontPtr, "G A M E   O V E R", 64);
                    txt.setFillColor(COLOR_RED);
                    sf::FloatRect r = txt.getLocalBounds();
                    txt.setPosition(sf::Vector2f(frame.position.x + frame.size.x / 2.f - r.size.x / 2.f,
                                                 frame.position.y + frame.size.y / 2.f - r.size.y / 2.f));
                    window.draw(txt);

                    sf::Text txt2(*fontPtr, "Press space to play again", 24);
                    txt2.setFillColor(COLOR_ORANGE);
                    sf::FloatRect r2 = txt2.getLocalBounds();
                    txt2.setPosition(sf::Vector2f(frame.position.x + frame.size.x / 2.f - r2.size.x / 2.f,
                                                  frame.position.y + frame.size.y / 2.f + 90.f));
                    window.draw(txt2);
                }

                window.display();
            }
        } else {
            repeatGame = false;
        }
    }

    return 0;
}
