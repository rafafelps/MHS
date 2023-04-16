#include <SFML/Graphics.hpp>
#include <iostream>
#include <cmath>
#include <vector>

#define PI 3.14159265

struct Engine {
    float mass;
    float k;
    float omega;
    float Xmax;
    float period;
    float phi;
    sf::Clock clock;
};


float pos(struct Engine* e);
float vel(struct Engine* e);
float acc(struct Engine* e);
float calcOmega(struct Engine* e);
float calcPeriod(struct Engine* e);
void setPeriod(struct Engine* e, float period);
void initSpring(std::vector<sf::RectangleShape*>* drawables);
void render(sf::RenderWindow* window, std::vector<sf::RectangleShape*>* drawables);

int main() {
    double aspect_ratio = 16.0 / 9;
    int width = 1280;
    int height = int(width / aspect_ratio);
    sf::RenderWindow window(sf::VideoMode(width, height), "Simple Harmonic Motion");
    std::vector<sf::RectangleShape*> drawables;

    struct Engine e;
    e.mass = 1;
    e.k = 1;
    setPeriod(&e, 10);
    e.Xmax = 50;
    e.phi = -PI/2;

    initSpring(&drawables);

    sf::RectangleShape line(sf::Vector2f(0.6f*width, 5));
    line.setOrigin(0.6f*width/2,2);
    line.setFillColor(sf::Color(255,0,0));
    line.setPosition(width/2, height/2);
    drawables.push_back(&line);

    double dt = 1.f/60.f; // Modify this to change physics rate.
    double accumulator = 0.f;
    sf::Clock clock;
    sf::Clock engineClock;
    bool drawn = false;
    unsigned int fps = 0;

    //circle.move(sf::Vector2f(380, 0));
    sf::Clock fpsClk;
    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();
        }

        accumulator += clock.getElapsedTime().asMicroseconds() / 1000000.f;
        clock.restart();
        if (accumulator >= dt) {
            // Physics and stuff
            float t = engineClock.getElapsedTime().asMilliseconds()/1000.f;
            //std::cout << pos(50, 4 * PI, -PI/2, t) << std::endl;
            //drawables[1]->setSize(sf::Vector2f(94, 481 + pos(&e)));
            

            accumulator = 0;
            drawn = false;
        }

        if (!drawn) {
            fps++;
            render(&window, &drawables);
            drawn = true;
        }

        if (fpsClk.getElapsedTime().asMilliseconds() >= 1000) {
            fpsClk.restart();
            std::cout << fps << std::endl;
            fps = 0;
        }
    }


    return 0;
}

float pos(struct Engine* e) {
    float t = e->clock.getElapsedTime().asMilliseconds() / 1000.f;
    return e->Xmax * cosf(e->omega * t + e->phi);
}

float vel(struct Engine* e) {
    float t = e->clock.getElapsedTime().asMilliseconds() / 1000.f;
    return - e->omega * e->Xmax * sinf(e->omega * t + e->phi);
}

float acc(struct Engine* e) {
    float t = e->clock.getElapsedTime().asMilliseconds() / 1000.f;
    return e->omega * e->omega * e->Xmax * cosf(e->omega * t + e->phi);
}

float calcOmega(struct Engine* e) {
    return sqrtf(e->k / e->mass);
}

float calcPeriod(struct Engine* e) {
    return 2 * PI * sqrtf(e->mass / e->k);
}

void setPeriod(struct Engine* e, float period) {
    e->period = period;
    e->omega = 2 * PI / period;
    e->k = e->omega * e->omega * e->mass;
}

void initSpring(std::vector<sf::RectangleShape*>* drawables) {
    sf::RectangleShape* ceiling = new sf::RectangleShape(sf::Vector2f(325, 23));
    ceiling->setOrigin(325/2-~(325&0x01), 23/2-~(23&0x01));
    ceiling->setFillColor(sf::Color(255, 247, 238, 220));
    ceiling->setPosition(997, 63);
    drawables->push_back(ceiling);

    sf::Texture* texture = new sf::Texture;
    texture->loadFromFile("spring.png");
    sf::RectangleShape* spring = new sf::RectangleShape(sf::Vector2f(94, 481));
    spring->setTexture(texture);
    spring->setOrigin(94/2-~(94&0x01), 0);
    spring->setPosition(997, 72);
    drawables->push_back(spring);

    sf::RectangleShape* box = new sf::RectangleShape(sf::Vector2f(134, 134));
    box->setFillColor(sf::Color(0, 0, 0));
    box->setOutlineThickness(5);
    box->setOutlineColor(sf::Color(255, 247, 238, 220));
    box->setOrigin(134/2-~(134&0x01), 134/2-~(134&0x01));
    box->setPosition(997, 625);
    drawables->push_back(box);
}

void render(sf::RenderWindow* window, std::vector<sf::RectangleShape*>* drawables) {
    window->clear();
    for (int i = 0; i < drawables->size(); i++) {
        window->draw(*(*drawables)[i]);
    }

    window->display();
}
