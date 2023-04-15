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
};

float pos(float Xmax, float omega, float phi, float t);
float vel(float Xmax, float omega, float phi, float t);
float acc(float Xmax, float omega, float phi, float t);
float calcOmega(float k, float mass);
float calcPeriod(float k, float mass);
void render(sf::RenderWindow* window, std::vector<sf::Drawable*>* drawables);

int main() {
    double aspect_ratio = 16.0 / 9;
    int width = 1280;
    int height = int(width / aspect_ratio);
    sf::RenderWindow window(sf::VideoMode(width, height), "Simple Harmonic Motion");

    Engine e;
    e.mass = 1;
    e.k = 1;
    float omega = 1;
    e.Xmax = 1;

    std::vector<sf::Drawable*> drawables;
    sf::CircleShape circle(20);
    circle.setFillColor(sf::Color(0, 148, 255));
    circle.setOrigin(19, 19);
    circle.setPosition(width/2, height/2);
    sf::RectangleShape line(sf::Vector2f(0.6f*width, 5));
    line.setOrigin(0.6f*width/2,2);
    line.setFillColor(sf::Color(255,0,0));
    line.setPosition(width/2, height/2);
    drawables.push_back(&circle);
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
            circle.setPosition(width/2 + pos(0.6f*width/2, 100, 90, t),height/2);
            //circle.setPosition(cosf(engineClock.getElapsedTime().asMilliseconds()/1000.f), 0);
            // pos(e.Xmax, e.omega, 0, engineClock.getElapsedTime().asMilliseconds()/1000.f)
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

float pos(float Xmax, float omega, float phi, float t) {
    return Xmax * cosf((omega * t + phi) * PI / 180.0);
}

float vel(float Xmax, float omega, float phi, float t) {
    return - omega * Xmax * sinf((omega * t + phi) * PI / 180.0);
}

float acc(float Xmax, float omega, float phi, float t) {
    return omega * omega * Xmax * cosf((omega * t + phi) * PI / 180.0);
}

float calcOmega(float k, float mass) {
    return sqrtf(k / mass);
}

float calcPeriod(float k, float mass) {
    return 2 * PI * sqrtf(mass / k);
}

void render(sf::RenderWindow* window, std::vector<sf::Drawable*>* drawables) {
    window->clear();
    for (int i = 0; i < 2; i++) {
        window->draw(*(*drawables)[i]);
    }

    window->display();
}
